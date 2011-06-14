<?php
/**
 * @file
 *   API for internationalization strings
 */

/**
 * String object that contains source and translations.
 * 
 * Note all database operations must go through textgroup object so we can switch storage at some point.
 */
class i18n_string_object {
  // Updated source string
  public $string;
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
      '%string' => ($string = $this->get_string()) ? $string : t('[empty string]'),
    );
  }
  /**
   * Set context properties
   */
  public function set_context($context) {
    $parts = is_array($context) ? $context : explode(':', $context);  
    $this->context = is_array($context) ? implode(':', $context) : $context;
    // Location will be the full string name
    $this->location = $this->textgroup . ':' . $this->context;
    $this->type = array_shift($parts);
    $this->objectid = $parts ? array_shift($parts) : '';
    $this->objectkey = (int)$this->objectid;      
    // Remaining elements glued again with ':'
    $this->property = $parts ? implode(':', $parts) : '';     
  }
  /**
   * Get source string
   */
  public function get_string() {
    if (isset($this->string)) {
      return $this->string;
    }
    elseif (isset($this->source)) {
      return $this->source;
    }
    else {
      return '';
    }
  }
  /**
   * Get translation to language from string object
   */
  public function get_translation($langcode) {
    if (!isset($this->translations[$langcode])) {
      $translation = $this->textgroup()->load_translation($this, $langcode);
      if ($translation && isset($translation->translation)) {
        $this->set_translation($translation, $langcode);
      }
      else {
        // No source, no translation
        $this->translations[$langcode] = FALSE;
      }      
    }
    // Which doesn't mean we've got a translation, only that we've got the result cached
    return $this->translations[$langcode];
  }
  /**
   * Set translation for language
   * 
   * @param $translation
   *   Translation object (from database) or string
   */
  public function set_translation($translation, $langcode = NULL) {
    if (is_object($translation)) {
      $langcode = $langcode ? $langcode : $translation->language;
      $string = isset($translation->translation) ? $translation->translation : FALSE;
      $this->set_properties($translation);
    }
    else {
      $string = $translation;
    }
    $this->translations[$langcode] = $string;
  }

  /**
   * Format the resulting translation or the default string applying callbacks
   *
   * There's a hidden variable, 'i18n_string_debug', that when set to TRUE will display additional info
   */
  public function format_translation($langcode, $options = array()) {
    $options += array('langcode' => $langcode, 'sanitize' => TRUE, 'cache' => FALSE, 'debug' => $this->textgroup()->debug);
    if ($translation = $this->get_translation($langcode)) {
      $string = $translation;
      if (!empty($this->format)) {
        $options += array('format' => $this->format);
      }
      if (isset($options['filter'])) {
        $string = call_user_func($options['filter'], $string);
      }
    }
    else {
      // Get default source string if no translation.
      $string = $this->get_string();
    }
    // Add debug information if enabled
    if ($options['debug']) {
      $info = array($this->textgroup, $this->context);
      if (!empty($this->format)) {
        $info[] = $this->format;
      }
      $options += array('suffix' => '');
      $options['suffix'] .= ' [' . implode(':', $info) . ']';
    }   
    return i18n_string_format($string, $options);
  }

  /**
   * Get source string provided a string object.
   *
   * @return
   *   String object if source exists.
   */
  public function get_source() {
    // If already searched and not found we don't have a source, 
    if (isset($this->lid) && !$this->lid) {
      return NULL;
    }
    elseif (!isset($this->lid) || !isset($this->source)) {
      // We may have lid from loading a translation but not loaded the source yet.
      if ($source = $this->textgroup()->load_source($this)) {
        $this->set_properties($source, FALSE);
        if (!isset($this->string)) {
          $this->string = $source->source;
        }
        return $this;
      }
      else {
        $this->lid = FALSE;
        return NULL;
      }
    }
    else {
      return $this;
    }
  }
  
  /**
   * Set properties from object or array
   * 
   * @param $properties
   *   Obejct or array of properties
   * @param $set_null
   *   Whether to set null properties too
   */
  public function set_properties($properties, $set_null = TRUE) {
    foreach ((array)$properties as $field => $value) {
      if (property_exists($this, $field) && ($set_null || isset($value))) {
        $this->$field = $value;
      }
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
  // Debug flag, set to true to print out more information.
  public $debug;
  // Cached or preloaded string objects
  public $strings;
  // Context to locale id map
  protected $string_map;
  /**
   * Class constructor
   */
  public function __construct($textgroup) {
    $this->textgroup = $textgroup;
    $this->debug = variable_get('i18n_string_debug', FALSE);
  }
  /**
   * Build string object
   * 
   * @param $context
   *   Context array or string
   * @param $string
   *   Current value for string source
   */
  public function build_string($context, $string = NULL) {
    // First try to locate string on cache
    $context = is_array($context) ? implode(':', $context) : $context;
    if ($cached = $this->cache_get($context)) {
      $i18nstring = $cached;
    }
    else {
      $i18nstring = new i18n_string_object();
      $i18nstring->textgroup = $this->textgroup;
      $i18nstring->set_context($context);
      $this->cache_set($context, $i18nstring);
    }
    $i18nstring->string = $string;
    return $i18nstring;
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
  protected function string_add($i18nstring, $options = array()) {
    $options += array('watchdog' => TRUE);
    // Default return status if nothing happens
    $status = -1;
    $source = NULL;
    $location = $i18nstring->location;
    // The string may not be allowed for translation depending on its format.
    if (!$this->string_check($i18nstring, $options)) {
      // The format may have changed and it's not allowed now, delete the source string
      return $this->string_remove($i18nstring, $options);
    }
    elseif ($source = $i18nstring->get_source()) {
      if ($source->source != $i18nstring->string || $source->location != $location) {
        $i18nstring->location = $location;
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
   * Check if string is ok for translation
   */
  protected static function string_check($i18nstring, $options = array()) {
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
   * Build query for i18n_string table
   */
  protected static function string_query($context, $multiple = FALSE) {
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
   * Remove string object.
   */
  public function string_remove($i18nstring, $options = array()) {
    $options += array('watchdog' => TRUE);
    if ($source = $i18nstring->get_source()) {
      db_delete('locales_target')->condition('lid', $source->lid)->execute();
      db_delete('i18n_string')->condition('lid', $source->lid)->execute();
      db_delete('locales_source')->condition('lid', $source->lid)->execute();
      $this->cache_set($source->context, NULL);
      if ($options['watchdog']) {
        watchdog('i18n_string', 'Deleted string %location for textgroup %textgroup: %string', $i18nstring->get_args());
      }
      return SAVED_DELETED;
    }
  }

  /**
   * Translate string object
   *
   * @param $i18nstring
   *   String object
   * @param $options
   *   Array with aditional options
   */
  protected function string_translate($i18nstring, $options = array()) {
    $langcode = isset($options['langcode']) ? $options['langcode'] : i18n_langcode();
    // Search for existing translation (result will be cached in this function call)
    $i18nstring->get_translation($langcode);
    return $i18nstring;
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
  protected function string_update($i18nstring, $options = array()) {
    if ($i18nstring->get_string()) {
      $status = $this->string_add($i18nstring, $options);
    }
    else {
      $status = $this->string_remove($i18nstring, $options);
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
   * Load string source from db
   */
  public static function load_source($i18nstring) {
    // Search the database using lid if we've got it or textgroup, context otherwise
    $query = db_select('locales_source', 's')->fields('s');
    $query->leftJoin('i18n_string', 'i', 's.lid = i.lid');
    $query->fields('i', array('format', 'objectid', 'type', 'property', 'objectindex'));
    if (!empty($i18nstring->lid)) {
      $query->condition('s.lid', $i18nstring->lid);
    }
    else {
      $query->condition('s.textgroup', $i18nstring->textgroup);
      $query->condition('s.context', $i18nstring->context);
    }
    // Speed up the query, we just need one row
    return $query->range(0, 1)->execute()->fetchObject();
  }

  /**
   * Load translation from db
   *
   * @todo Optimize when we've already got the source string
   */
  public static function load_translation($i18nstring, $langcode) {
    // Search the database using lid if we've got it or textgroup, context otherwise
    if (!empty($i18nstring->lid)) {
      // We've alreay got lid, we just need translation data
      $query = db_select('locales_target', 't');
      $query->condition('t.lid', $i18nstring->lid);
    }
    else {
      // Still don't have lid, load string properties too
      $query = db_select('i18n_string', 's')->fields('s');
      $query->leftJoin('locales_target', 't', 's.lid = t.lid');
      $query->condition('s.textgroup', $i18nstring->textgroup);
      $query->condition('s.context', $i18nstring->context);
    }
    // Add translation fields   
    $query->fields('t', array('translation', 'i18n_status'));
    $query->condition('t.language', $langcode);
    // Speed up the query, we just need one row
    $query->range(0, 1);
    return $query->execute()->fetchObject();
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
    if (!$string->get_source()) {
      // Create source string so we get an lid
      $this->save_source($string);
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
    if (isset($source->string)) {
      $source->source = $source->string;
    }
    if (empty($source->version)) {
      $source->version = 1;
    }
    return drupal_write_record('locales_source', $source, !empty($source->lid) ? 'lid' : array());
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
  public function context_remove($context, $string = NULL, $options = array()) {
    $options += array('messages' => $this->debug);
    $i18nstring = $this->build_string($context, $string);
    $status = $this->string_remove($i18nstring, $options);
    if ($options['messages'] && $status === SAVED_DELETED) {
      drupal_set_message(t('Deleted string %location for textgroup %textgroup: %string', $i18nstring->get_args()));
    }
    return $this;
  }

  /**
   * Translate source string
   */
  public function context_translate($context, $string, $options = array()) {
    $i18nstring = $this->build_string($context, $string);
    return $this->string_translate($i18nstring, $options);
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
  public function context_update($context, $string, $options = array()) {
    $options += array('format' => FALSE, 'messages' => $this->debug, 'watchdog' => TRUE);
    $i18nstring = $this->build_string($context, $string);
    $i18nstring->format = $options['format'];
    if (!$this->string_check($i18nstring, $options)) {
      $this->context_remove($context, $string, $options);
      $status = SAVED_DELETED;
    }
    else {
      $status = $this->string_update($i18nstring, $options);
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
   * Get multiple translations with the available key
   */
  public function multiple_get_translation($conditions, $property) {
    $translations = array();
    $langcode = !empty($conditions['language']) && !is_array($conditions['language']) ? $conditions['language'] : NULL;
    foreach ($this->multiple_load_translation($conditions) as $translation) {     
      // The string object may be already in cache
      $string = $this->build_string($translation->context);
      $string->set_properties($translation);
      // If this is a translation we set it there too
      if ($translation->language && $translation->translation) {
        $string->set_translation($translation);
      }
      elseif ($langcode) {
        $string->set_translation(FALSE, $langcode);
      }
      $translations[$translation->$property] = $string;
    }
    return $translations;
  }

  /**
   * Load multiple translations from db
   *
   * @todo Optimize when we've already got the source object
   */
  protected static function multiple_load_translation($conditions) {
    // We may be querying all translations at the same time or just one language.
    // The language field needs some special treatment though.
    $query = db_select('i18n_string', 's')->fields('s');
    $query->leftJoin('locales_target', 't', 's.lid = t.lid');
    $query->fields('t', array('translation', 'language', 'i18n_status'));
    foreach ($conditions as $field => $value) {
      if ($field == 'language') {
        $query->condition('t.language', $value);
      }
      else {
        $query->condition('s.' . $field, $value);
      }
    }
    return $query->execute()->fetchAll();
  }

  /**
   * Translate array of source strings
   * 
   * @param $context
   *   Context array with placeholders (*)
   * @param $strings
   *   Optional array of source strings indexed by the placeholder property
   *   
   * @return array
   *   Array of string objects (with translation) indexed by the placeholder field
   */
  public function multiple_translate($context, $strings = array(), $options = array()) {
    // First, build conditions and identify the variable field
    $conditions = $context = array_combine(array('type', 'objectid', 'property'), $context);
    $langcode = isset($options['langcode']) ? $options['langcode'] : i18n_langcode();
    // If we've got keyed source strings set the array of keys on the placeholder field
    // or if not, remove that condition so we search all strings with that keys.   
    foreach ($conditions as $field => $value) {
      if ($value === '*') {
        $property = $field;
        if ($strings) {
          $conditions[$field] = array_keys($strings);
        }
        else {
          unset($conditions[$field]);
        }
      }
    }
    // Now we'll add the language code to conditions and get the translations indexed by the property field
    $conditions['language'] = $langcode;
    $translations = $this->multiple_get_translation($conditions, $property);
    // Remap translations using property field
    foreach ($strings as $key => $source) {
      if (isset($translations[$key])) {
        $translations[$key]->string = $source;
      }
      else {
        // Not found any string for this property, create it to map in the response
        // But make sure we set this language's translation to FALSE so we don't search again
        $newcontext = $context;
        $newcontext[$property] = $key;
        $translations[$key] = $this->build_string($newcontext, $source);
      }
    }
    return $translations;
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
    $i18nstring = $this->build_string($context);
    if ($source = $i18nstring->get_source()) {
      $source->set_translation($translation, $langcode);
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
        foreach ($translations as $field => $i18nstring) {
          if (is_object($object)) {
            $object->$field = $i18nstring->format_translation($langcode, $options);
          }
          elseif (is_array($object)) {
            $object[$field] = $i18nstring->format_translation($langcode, $options);
          }
        }
      }
      return $this->translations[$langcode] = $object;
    }    
  }
}
