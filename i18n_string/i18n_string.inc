<?php
/**
 * @file
 *   API for internationalization strings
 */

/**
 * String object that may contain translations
 */
class i18n_string_object {
  // Properties from locale source
  public $lid;
  public $source;
  public $textgroup;
  public $location;  
  public $context;
  public $version;
  // Properties from i18n_tring
  public $type;
  public $objectid;
  public $property;
  public $objectkey;
  public $format;
  // Temporary language code and translation
  public $language;
  public $translation;
  // Array of translations to multiple languages
  public $translations;
  // Textgroup object
  protected $_textgroup;
  
  /**
   * Class constructor
   */
  public function __construct($data = NULL) {
    if ($data) {
      $this->set_properties($data);
    }
  }
  /**
   * Get message parameters from context and string.
   */
  public function get_args() {
    return array(
      '%location' => $this->location,
      '%textgroup' => $this->textgroup,
      '%string' => isset($this->source) ? $this->source : t('[empty string]'),
    );
  }
  /**
   * Get translation to language from string object
   */
  public function get_translation($langcode) {
    return isset($this->translations[$langcode]) ? $this->translations[$langcode] : NULL;
  }
  /**
   * Set translation for language
   */
  public function set_translation($langcode, $string) {
    $this->translations[$langcode] = $string;
  }
  /**
   * Set properties from object or array
   */
  public function set_properties($properties) {
    foreach ((array)$properties as $field => $value) {
      $this->$field = $value;
    }
    return $this;
  }
  /**
   * Access textgroup object
   */
  protected function textgroup() {
    if (!isset($this->_textgroup)) {
      $this->_textgroup = i18n_string_textgroup($this->textgroup);
    }
    return $this->_textgroup;
  }
}

/**
 * Textgroup handler for i18n_string API
 */
class i18n_string_textgroup_default {
  // Text group name
  public $textgroup;
  // Cached or preloaded translations
  public $strings;
  // Context to locale id map
  protected $string_map;
  // String formats
  protected $string_format;
  /**
   * Class constructor
   */
  public function __construct($textgroup) {
    $this->textgroup = $textgroup;
  }
  /**
   * Build string object
   */
  public function build_string($context, $source, $options = array()) {
    // First try to locate string on cache
    if (is_array($context)) {
      $parts = $context;
      $context = implode(':', $context);
    }
    else {
      $parts = explode(':', $context);
    }
    if ($cached = $this->cache_get($context)) {
      $string = $cached;
    }
    else {
      $string = new i18n_string_object();
      $string->context = $context;
      $string->textgroup = $this->textgroup;
      // Location will be the full string name
      $string->location = $this->textgroup . ':' . $string->context;
      $string->type = array_shift($parts);
      $string->objectid = $parts ? array_shift($parts) : '';
      $string->objectkey = (int)$string->objectid;      
      // Remaining elements glued again with ':'
      $string->property = $parts ? implode(':', $parts) : '';    
      $this->cache_set($context, $string);
    }    
    $string->source = $source;

    return $string;
  }
  /**
   * Add source string to the locale tables for translation.
   *
   * It will also add data into i18n_string table for faster retrieval and indexing of groups of strings.
   * Some string context doesn't have a numeric oid (I.e. content types), it will be set to zero.
   *
   * This function checks for already existing string without context for this textgroup and updates it accordingly.
   * It is intended for backwards compatibility, using already created strings.
   *
   * @param $i18nstring
   *   String object
   * @param $format
   *   Text format, for strings that will go through some filter
   * @return
   *   Update status.
   */
  protected function add_string($i18nstring, $options = array()) {
    $options += array('watchdog' => TRUE);
    // Default return status if nothing happens
    $status = -1;
    $source = NULL;
    // The string may not be allowed for translation depending on its format.
    if (!$this->check_string($i18nstring, $options)) {
      // The format may have changed and it's not allowed now, delete the source string
      return $this->remove_string($i18nstring, $options);
    }
    elseif ($source = $this->get_source($i18nstring)) {
      $i18nstring->lid = $source->lid;
      if ($source->source != $i18nstring->source || $source->location != $i18nstring->location) {
        // String has changed, mark translations for update
        $status = $this->save_source($i18nstring);
        db_update('locales_target')
          ->fields(array('i18n_status' => I18N_STRING_STATUS_UPDATE))
          ->condition('lid', $source->lid)
          ->execute();
      }
      elseif (empty($source->version)) {
        // When refreshing strings, we've done version = 0, update it
        $this->save_source($i18nstring);
      }
    }
    else {
      // We don't have the source object, create it
      $status = $this->save_source($i18nstring);
    }
    // Make sure we have i18n_string part, create or update
    // This will also create the source object if doesn't exist
    $this->save_string($i18nstring);

    if ($options['watchdog']) {
      switch ($status) {
        case SAVED_UPDATED:
          watchdog('i18n_string', 'Updated string %location for textgroup %textgroup: %string', $i18nstring->get_args());
          break;
        case SAVED_NEW:
          watchdog('i18n_string', 'Created string %location for text group %textgroup: %string', $i18nstring->get_args());
          break;
      }
    }
    return $status;
  }

  /**
   * Set string object into cache
   */
  protected function cache_set($context, $string) {
    if ($string && !empty($this->strings[$context])) {
      // Merge translations
    }
    $this->strings[$context] = $string;
  }

  /**
   * Get translation from cache
   */
  protected function cache_get($context) {
    return isset($this->strings[$context]) ? $this->strings[$context] : NULL;
  }

  /**
   * Reset cache, needed for tests
   */
  public function cache_reset() {
    $this->strings = array();
    $this->string_format = array();
    $this->translations = array();
  }

  /**
   * Check if string is ok for translation
   */
  protected static function check_string($i18nstring, $options = array()) {
    $options += array('messages' => FALSE, 'watchdog' => TRUE);
    if (!empty($i18nstring->format) && !i18n_string_allowed_format($i18nstring->format)) {
      // This format is not allowed, so we remove the string, in this case we produce a warning
      drupal_set_message(t('The string %location for textgroup %textgroup is not allowed for translation because of its text format.', $i18nstring->get_args()), 'warning');
      return FALSE;
    }
    else {
      return TRUE;
    }
  }

  /**
   * Get source string provided a string object.
   *
   * @param $context
   *   Context array or string object.
   * @return
   *   String object if it exists.
   */
  public function get_source($i18nstring) {
    if (!is_object($i18nstring)) {
      $i18nstring = $this->build_string($i18nstring, NULL);
    }
    // Search the database using lid if we've got it or textgroup, context otherwise
    $query = db_select('locales_source', 's')->fields('s');
    $query->leftJoin('i18n_string', 'i', 's.lid = i.lid');
    $query->fields('i', array('format', 'objectid', 'type', 'property', 'objectindex'));
    if (!empty($i18nstring->lid)) {
      $query->condition('s.lid', $i18nstring->lid);
    }
    else {
      $query->condition('s.textgroup', $this->textgroup);
      $query->condition('s.context', $i18nstring->context);
    }
    // Speed up the query, we just need one row
    $source = $query->range(0, 1)->execute()->fetchObject();
    // Update cached map if we've got a context
    $this->string_map[$i18nstring->context] = $source ? $source->lid : FALSE;
    return $source ? new i18n_string_object($source) : FALSE;
  }

  /**
   * Get translation from the database. Full object with text format.
   *
   * This one doesn't return anything if we don't have the full i18n strings data there
   * to prevent missing data resulting in missing text formats
   */
  protected function get_translation($i18nstring, $langcode) {
    if (!isset($i18nstring->translations[$langcode])) {
      // Which doesn't mean we've got a translation, only that we've got the result cached
      $translation = $this->load_translation($i18nstring, $langcode);
      if ($translation) {
        $i18nstring
          ->set_properties($translation)
          ->set_translation($langcode, $translation->translation);
      }
      else {
        // No source, no translation
        $i18nstring->set_translation($langcode, FALSE);
      }
      
    }
    // Return the full object if we've got a translation
    return $i18nstring->get_translation($langcode) !== FALSE ? $i18nstring : NULL;
  }

  /**
   * Load translation from db
   *
   * @todo Optimize when we've already got the source string
   */
  protected static function load_translation($i18nstring, $langcode, $multiple = FALSE) {
    $query = self::query_string($i18nstring, $multiple);
    $query->leftJoin('locales_target', 't', 's.lid = t.lid');
    $query->fields('t', array('translation', 'i18n_status'));
    $query->condition('t.language', $langcode);
    // Speed up the query, we just need one row
    if (!$multiple) {
      $query->range(0, 1);
    }
    $translations = $query->execute()->fetchAll();

    foreach ($translations as $translation) {
      $translation->translation = isset($translation->translation) ? $translation->translation : FALSE;
    }
    if (!$multiple) {
      return reset($translations);
    }
    else {
      return $translations;
    }
  }

  /**
   * Get string source object
   *
   * @param $context
   *   Context string or object.
   *
   * @return
   *   - Translation string as object if found.
   *   - FALSE if no translation
   *
   */
  protected static function get_string($context, $multiple = FALSE) {
    return self::query_string($context, $multiple)->execute()->fetchObject();
  }

  /**
   * Build query for i18n_string table
   */
  protected static function query_string($context, $multiple = FALSE) {
    // Search the database using lid if we've got it or textgroup, context otherwise
    $query = db_select('i18n_string', 's')->fields('s');
    if (!empty($context->lid)) {
      $query->condition('s.lid', $context->lid);
    }
    else {
      $query->condition('s.textgroup', $context->textgroup);
      if (!$multiple) {
        $query->condition('s.context', $context->context);
      }
      else {
        // Query multiple strings
        foreach (array('type', 'objectid', 'property') as $field) {
          if (!empty($context->$field)) {
            $query->condition('s.' . $field, $context->$field);
          }
        }
      }
    }
    return $query;
  }

  /**
   * Remove source and translations for user defined string.
   *
   * Though for most strings the 'name' or 'string id' uniquely identifies that string,
   * there are some exceptions (like profile categories) for which we need to use the
   * source string itself as a search key.
   *
   * @param $context
   *   Textgroup and location glued with ':'.
   * @param $string
   *   Optional source string (string in default language).
   */
  public function remove($context, $string = NULL, $options = array()) {
    $options += array('messages' => FALSE);
    $i18nstring = $this->build_string($context, $string, $options);
    $status = $this->remove_string($i18nstring, $options);
    if ($options['messages'] && $status === SAVED_DELETED) {
      drupal_set_message(t('Deleted string %location for textgroup %textgroup: %string', $i18nstring->get_args()));
    }
    return $this;
  }

  /**
   * Remove string object.
   */
  public function remove_string($i18nstring, $options = array()) {
    $options += array('watchdog' => TRUE);
    if ($source = $this->get_source($i18nstring)) {
      db_delete('locales_target')->condition('lid', $source->lid)->execute();
      db_delete('i18n_string')->condition('lid', $source->lid)->execute();
      db_delete('locales_source')->condition('lid', $source->lid)->execute();
      if ($options['watchdog']) {
        watchdog('i18n_string', 'Deleted string %location for textgroup %textgroup: %string', $i18nstring->get_args());
      }
      return SAVED_DELETED;
    }
  }
  /**
   * Save / update string object
   *
   * There seems to be a race condition sometimes so skip errors, #277711
   *
   * @param $string
   *   Full string object to be saved
   * @param $source
   *   Source string object
   */
  protected function save_string($string, $update = FALSE) {
    if (empty($string->lid)) {
      if ($source = $this->get_source($string)) {
        $string->lid = $source->lid;
      }
      else {
        // Create source string so we get an lid
        $this->save_source($string);
      }
    }
    if (!isset($string->objectkey)) {
      $string->objectkey = (int)$string->objectid;
    }
    if (!isset($string->format)) {
      $string->format = '';
    }
    $status = db_merge('i18n_string')
      ->key(array('lid' => $string->lid))
      ->fields(array(
          'textgroup' => $string->textgroup,
          'context' => $string->context,
          'objectid' => $string->objectid,
          'type' => $string->type,
          'property' => $string->property,
          'objectindex' => $string->objectkey,
          'format' => $string->format,
      ))
      ->execute();
    return $status;
  }

  /**
   * Save translation to the db
   *
   * @param $string
   *   Full string object with translation data (language, translation)
   */
  protected function save_translation($string, $langcode) {
    db_merge('locales_target')
      ->key(array('lid' => $string->lid, 'language' => $langcode))
      ->fields(array('translation' => $string->get_translation($langcode)))
      ->execute();
  }

  /**
   * Save source string (create / update)
   */
  protected static function save_source($source) {
    if (empty($source->version)) {
      $source->version = 1;
    }
    return drupal_write_record('locales_source', $source, !empty($source->lid) ? 'lid' : array());
  }
  /**
   * Translate source string
   */
  public function translate($context, $string, $options = array()) {
    $i18nstring = $this->build_string($context, $string, $options);
    return $this->translate_string($i18nstring, $options);
  }
  /**
   * Translate array of source strings
   */
  public function multiple_translate($context, $strings, $options = array()) {
    $i18nstring = $this->build_string($context, NULL, $options);
    // Set the array of keys on the placeholder field
    foreach (array('type', 'objectid', 'property') as $field) {
      if ($i18nstring->$field === '*') {
        $i18nstring->$field = array_keys($strings);
        $property = $field;
      }
    }
    $langcode = isset($options['langcode']) ? $options['langcode'] : i18n_langcode();
    $translations = $this->multiple_get_translation($i18nstring, $langcode);
    // Remap translations using index field
    $result = $strings;
    foreach ($translations as $translation) {
      $index = $translation->$property;
      $translation->source = is_array($strings[$index]) ? $strings[$index]['string'] : $strings[$index];
      $result[$index] = $translation;
      unset($strings[$index]);
    }
    // Fill in remaining strings for consistency, l10n_client, etc..
    foreach ($strings as $index => $string) {
      $translation = clone $i18nstring;
      $translation->$property = $index;
      $translation->source = is_array($strings[$index]) ? $strings[$index]['string'] : $strings[$index];
      $result[$index] = $translation;
    }
    return $result;
  }
  /**
   * Get multiple translations with the available key
   */
  public function multiple_get_translation($i18nstring, $langcode) {
    $translations = array();
    foreach ($this->load_translation($i18nstring, $langcode, TRUE) as $index => $translation) {     
      $string = new i18n_string_object($translation);
      $string->set_translation($langcode, $translation->translation);
      if ($translation->translation !== FALSE) {
        $translations[$langcode] = $string;
      } 
      $this->cache_set($string->context, $string);
    }
    return $translations;
  }

  /**
   * Translate string object
   *
   * @param $i18nstring
   *   String object
   * @param $options
   *   Array with aditional options
   */
  protected function translate_string($i18nstring, $options = array()) {
    $langcode = isset($options['langcode']) ? $options['langcode'] : i18n_langcode();
    // Search for existing translation (result will be cached in this function call)
    if ($translation = $this->get_translation($i18nstring, $langcode)) {
      return $translation;
    }
    else {
      return $i18nstring;
    }
  }

  /**
   * Update / create translation source for user defined strings.
   *
   * @param $name
   *   Textgroup and location glued with ':'.
   * @param $string
   *   Source string in default language. Default language may or may not be English.
   * @param $options
   *   Array with additional options:
   *   - 'format', String format if the string has text format
   *   - 'messages', Whether to print out status messages
   */
  public function update($context, $string, $options = array()) {
    $options += array('format' => FALSE, 'messages' => FALSE, 'watchdog' => TRUE);
    $i18nstring = $this->build_string($context, $string, $options);
    $i18nstring->format = $options['format'];
    if (!$this->check_string($i18nstring, $options)) {
      $this->remove($context, $string, $options);
      $status = SAVED_DELETED;
    }
    else {
      $status = $this->update_string($i18nstring, $options);
    }

    if ($options['messages']) {
      switch ($status) {
        case SAVED_UPDATED:
          drupal_set_message(t('Updated string %location for textgroup %textgroup: %string', $i18nstring->get_args()));
          break;
        case SAVED_NEW:
          drupal_set_message(t('Created string %location for text group %textgroup: %string', $i18nstring->get_args()));
          break;
      }
    }
    return $this;
  }
  /**
   * Update / create / remove string.
   *
   * @param $name
   *   String context.
   * @pram $string
   *   New value of string for update/create. May be empty for removing.
   * @param $format
   *   Text format, that must have been checked against allowed formats for translation
   * @return status
   *   SAVED_UPDATED | SAVED_NEW | SAVED_DELETED
   */
  protected function update_string($i18nstring, $options = array()) {
    if (!empty($i18nstring->source)) {
      $status = $this->add_string($i18nstring, $options);
    }
    else {
      $status = $this->remove_string($i18nstring, $options);
    }
    return $status;
  }
  /**
   * Update string translation, only if source exists.
   * 
   * @param $context
   *   String context as array
   * @param $langcode
   *   Language code to create the translation for
   * @param $translation
   *   String translation for this language
   */
  function update_translation($context, $langcode, $translation) {
    if ($source = $this->get_source($context)) {
      $source->set_translation($langcode, $translation);
      $this->save_translation($source, $langcode);
      return $source;
    }
  }


  /**
   * Recheck strings after update
   */
  public function update_check() {
    // Find strings in locales_source that have no data in i18n_string
    $query = db_select('locales_source', 'l')
    ->fields('l')
    ->condition('l.textgroup', $this->textgroup);
    $alias = $query->leftJoin('i18n_string', 's', 'l.lid = s.lid');
    $query->condition('s.lid', NULL);
    foreach ($query->execute()->fetchAll() as $string) {
      list($string->type, $string->objectid, $string->property) = explode(':', $string->context);
      $this->save_string($string);
    }
  }

}

/**
 * String object wrapper
 */
class i18n_string_object_wrapper extends i18n_object_wrapper {
  // Text group object
  protected $textgroup;
  // Object translations
  protected $translations;
  
  /**
   * Get object strings for translation
   * 
 	 * This will return a simple array of arrays, indexed by string key.
   * Each item will have the same elements as in i18n_string_object_properties() and
   * - 'name', full string name array (textgroup, object type, object id, string key)
   */
  public function get_strings($include_empty = FALSE) {
    $strings = array();
    foreach ($this->get_properties() as $textgroup => $textgroup_list) {
      foreach ($textgroup_list as $type => $type_list) {
        foreach ($type_list as $object_id => $object_list) {
          foreach ($object_list as $key => $string) {
            if ($include_empty || !empty($string['string'])) {
              $strings[$key] = $string + array(
                'name' => array($textgroup, $type, $object_id, $key),
              );
            }
          }
        }
      }
    }
    return $strings;
  }
  /**
   * Get object translatable properties
   * 
   * This will return a big array indexed by textgroup, object type, object id and string key.
   * Each element is an array with string information, and may have these properties:
   * - 'string', the string itself, will be NULL if the object doesn't have that string
   * - 'format', string format when needed
   * - 'title', string readable name
   */
  public function get_properties() {
    list($string_type, $object_id) = $this->get_string_context(); 
    $object_keys = array(
      $this->get_textgroup(),
      $string_type,
      $object_id,
    );
    $strings = array();   
    foreach ($this->get_string_info('properties') as $field => $info) {
      $info = is_array($info) ? $info : array('title' => $info);
      $field_name = isset($info['field']) ? $info['field'] : $field;
      $value = $this->get_field($field_name);
      $strings[$this->get_textgroup()][$string_type][$object_id][$field] = array(
        'string' => is_array($value) || isset($info['empty']) && $value === $info['empty'] ? NULL : $value,
        'title' => $info['title'],
        'format' => isset($info['format']) ? $this->get_field($info['format']) : NULL,
        'name' => array_merge($object_keys, array($field)),
      );
    }
    // Call hook_i18n_string_list_TEXTGROUP_alter(), last chance for modules
    drupal_alter('i18n_string_list_' . $this->get_textgroup(), $strings, $this->type, $this->object);
    return $strings;
  }
  /**
   * Get object info
   */
  public function get_string_info($property) {
    return i18n_string_object_info($this->type, $property);
  }
  /**
   * Get string context
   */
  public function get_string_context() {
    return array($this->get_string_info('type'), $this->get_key());
  }
  /**
   * Get translate path for object
   *
   * @param $langcode
   * 	 Language code if we want ti for a specific language
   */
  public function get_translate_path($langcode = NULL) {
    $replacements = array('%language' => $langcode ? $langcode : '');
    if ($path = $this->get_string_info('translate path')) {
      return $this->path_replace($path, $replacements);
    }
    elseif ($path = $this->get_info('translate tab')) {
      // If we've got a translate tab path, we just add language to it
      return $this->path_replace($path . '/%language', $replacements);
    }
  }

  /**
   * Translation mode for object
   */
  public function get_translate_mode() {
    return !$this->get_langcode() ? I18N_MODE_LOCALIZE : I18N_MODE_NONE;
  }
  /**
   * Get textgroup name
   */
  public function get_textgroup() {
    return $this->get_string_info('textgroup');
  }
  /**
   * Get textgroup object
   */
  protected function textgroup() {
    if (!isset($this->textgroup)) {
      $this->textgroup = i18n_string_textgroup($this->get_textgroup());
    }
    return $this->textgroup;
  }

  /**
   * Translate object properties
   */
  public function translate($options = array()) {
    $options['langcode'] = $langcode = isset($options['langcode']) ? $options['langcode'] : i18n_langcode();
    // We may have it already translated. As objects are statically cached, translations are too.
    if (isset($this->translations[$langcode])) {
      return $this->translations[$langcode];
    }
    else {
      $object = is_object($this->object) ? clone $this->object : $this->object;
      if ($strings = $this->get_strings()) {
        $context = $this->get_string_context();
        $context[] = '*';
        $translations = $this->textgroup()->multiple_translate($context, $strings, $options);
        // Clone real objects, mark translated...
        foreach ($translations as $field => $value) {
          if (is_object($object)) {
            $object->$field = i18n_string_format($value, $options);
          }
          elseif (is_array($object)) {
            $object[$field] = i18n_string_format($value, $options);
          }
        }
      }
      return $this->translations[$langcode] = $object;
    }    
  }
}
