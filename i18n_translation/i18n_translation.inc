<?php
/**
 * @file
 * Internationalization (i18n) module - Translation set
 */
class i18n_translation_set {
  public $tsid = NULL;
  public $type;
  public $bundle = '';
  public $status = 0;
  public $master_id = 0;
  // It may optionally have a title
  public $title = '';
  // Translations indexed by language
  public $translations = NULL;
  
  /**
   * Constructor from object/array
   */
  public function __construct($translation = NULL) {
    if ($translation) {
      foreach ((array)$translation as $key => $value) {
        $this->$key = $value;
      }
    }
  }

  /**
   * Delete a translation set
   * 
   * @param $delete_translations
   *   Whether to unlink translations from the set. Detaults to TRUE.
   */
  public function delete($delete_translations = TRUE) {
    db_delete('i18n_translation_set')
      ->condition('tsid', $this->tsid)
      ->execute();
    if ($delete_translations) {
      $this->delete_translations();
    }
    $this->invoke_all('delete');
    $this->tsid = NULL;
  }

  /**
   * Invoke all modules
   */
  public function invoke_all($op) {
    module_invoke_all('i18n_translation_set_' . $op, $this);
    module_invoke_all('entity_' . $op, $this, 'i18n_translation');
  }

  /**
   * Create a new translation set
   * 
   * @param $save_translations
   *   Whether to update linked objects so they belong to this set.
   */
  public function insert($save_translations = TRUE) {
    $this->created = $this->changed = REQUEST_TIME;
    $status = drupal_write_record('i18n_translation_set', $this);
    if ($save_translations) {
      $this->save_translations();
    }
    $this->invoke_all('insert');
    return $status;
  }

  /**
   * Save translation set
   * 
   * @param $save_translations
   *   Whether to update linked objects so they belong to this set.
   */
  public function save($save_translations = TRUE) {
    $this->invoke_all('presave');
    return empty($this->tsid) ? $this->insert($save_translations) : $this->update($save_translations);
  }

  /**
   * Update a translation set
   * 
   * @param $update_translations
   *   Whether to update objects linked to this set.
   */
  public function update($update_translations = TRUE) {
    $this->changed = REQUEST_TIME;
    $status = drupal_write_record('i18n_translation_set', $this, 'tsid');
    if ($update_translations) {
      $this->clean_translations();
      $this->save_translations();
    }
    $this->invoke_all('update');
    return $status;
  }

  /**
   * Update a translation set or delete if empty.
   */
  public function update_delete() {
    if ($this->get_translations()) {
      return $this->update(TRUE);
    }
    else {
      return $this->delete(TRUE);
    }
  }

  /**
   * Clean all items in this translation set
   *
   * Unlink other items (not current translations from this translation set)
   */
  public function clean_translations() {
    if (($table = $this->get_table()) && ($field = $this->get_field())) {
      $query = db_update($table)
        ->fields(array($field => 0))
        ->condition($field, $this->tsid);
      if ($translations = $this->get_translations()) {
        $query->condition('language', array_keys($translations), 'NOT IN');
      }
      return $query->execute();
    }
  }

  /**
   * Save translations in this translation set
   */
  public function save_translations() {
    if (($table = $this->get_table()) && ($field = $this->get_field())) {
      if ($keys = $this->get_translations_keys()) {
        return db_update($table)
          ->fields(array($field => $this->tsid))
          ->condition($this->key_field(), $keys)
          ->execute();
      }
      else {
        return $this->delete_translations();
      }
    }
  }

  /**
   * Delete translations in this translation set
   *
   * It won't delete objects, just unlink them from translation set
   */
  public function delete_translations() {
    if (($table = $this->get_table()) && ($field = $this->get_field())) {
      return db_update($table)
        ->fields(array($field => 0))
        ->condition($field, $this->tsid)
        ->execute();
    }
  }

  /**
   * Get translations, indexed by language
   */
  public function get_translations() {
    if (!isset($this->translations)) {
      $this->translations = $this->load_translations();
    }
    return $this->translations;
  }
  /**
   * Get item for language
   */
  public function get_item($langcode) {
    if (($translations = $this->get_translations()) && isset($translations[$langcode])) {
      return $translations[$langcode];
    }
    else {
      return NULL;
    }
  }
  /**
   * Get translations keys, indexed by language
   */
  public function get_translations_keys() {
    $keys = array();
    foreach ($this->get_translations() as $lang => $item) {
      if ($id = i18n_object_field($item, $this->key_field())) {
        $keys[$lang] = $id;
      }
    }
    return array_filter($keys);
  }
  /**
   * Get edit path for this translation set
   */
  public function get_edit_path() {
    if ($path = $this->get_info('edit path')) {
      return strtr($path, $this->get_path_placeholders('delete'));
    }
    else {
      return '';
    }
  }
  /**
   * Get operations as renderable links
   */
  public function get_operations() {
    $destination = drupal_get_destination();
    $operations = array();
    if ($path = $this->get_edit_path()) {
      $operations['edit'] = array(
        'title' => t('edit'),
        'href' => $path,
        'query' => $destination,
      );
    }
    if ($path = $this->get_delete_path()) {
      $operations['delete'] = array(
        'title' => t('delete'),
        'href' => $path,
        'query' => $destination,
      );
    }
    return $operations;  
  }
  /**
   * Get items as renderable links
   */
  public function get_links() {
    $language_list = language_list();
    $items = array();
    foreach ($this->get_translations() as $langcode => $item) {
      $title = $this->item_title($item);
      $path = $this->item_path($item);
      $language = isset($language_list[$langcode]) ? $language_list[$langcode] : NULL;
      $items[$langcode] = array(
        'title' => $title,
        'href' => $path ? $path : NULL,
        'language' => $language,
      );
      if ($language && function_exists('languageicons_link_add')) {
        languageicons_link_add($items[$langcode]);
      }
    }
    return $items;    
  }
  /**
   * Get overview (list) path for this translation set
   */
  public function get_list_path() {
    return $this->get_info('list path');
  }
  /**
   * Get delete path for this translation set
   */
  public function get_delete_path() {
    if ($path = $this->get_info('delete path')) {
      return strtr($path, $this->get_path_placeholders('delete'));
    }
    else {
      return '';
    }
  }
  /**
   * Get placeholder values for path replacement
   */
  function get_path_placeholders($op = 'list') {
    $values['%operation'] = $op;
    $values['%type'] = $this->type;
    $values['%i18n_translation_set'] = $this->tsid;
    if ($placeholder = $this->get_info('placeholder')) {
      $values[$placeholder] = $this->tsid;
    }
    return $values;
  }
  
  /**
   * Get field on translations table that stores the translation set id (tsid)
   */
  protected function get_field() {
    return $this->get_info('field');
  }

  /**
   * Get property from translation set info
   */
  public function get_info($property, $default = NULL) {
    $info = i18n_translation_set_info($this->type);
    return $info && isset($info[$property]) ? $info[$property] : $default;
  }

  /**
   * Get table name, where translation items are stored.
   */
  protected function get_table() {
    return $this->get_info('table');
  }

  /**
   * Get title for this set
   */
  public function get_title() {
    if (!empty($this->title)) {
      return $this->title;
    }
    elseif ($translations = $this->get_translations()) {
      foreach ($translations as $item) {
        $names[] = $this->item_title($item);
      }
      return implode(' / ', $names);
    }
    else {
      return t('Undefined');
    }
  }

  /**
   * Get title from item
   */
  protected function item_title($item) {
    return entity_label($this->type, $item);
  }

  /**
   * Get link for item
   */
  protected function item_path($item) {
    if ($uri = entity_uri($this->type, $item)) {
      return $uri['path'];
    }
  }

  /**
   * Get item list
   */
  public function item_list() {
    $language_list = language_list();
    $items = array();
    foreach ($this->get_translations() as $langcode => $item) {
      $title = $this->item_title($item);
      $path = $this->item_path($item);
      if ($title && $path) {
        $options = isset($language_list[$langcode]) ? array('language' => $language_list[$langcode]) : array();
        $items[$langcode] = l($title, $path, $options);
      }
      elseif ($title) {
        $items[$langcode] = check_plain($title);
      }
    }
    return $items;
  }

  /**
   * Add translations
   */
  public function add_translations($translations) {
    array_map(array($this, 'add_item'), $translations);
    return $this;
  }

  /**
   * Add translation item
   */
  public function add_item($item, $langcode = NULL) {
    $item = $this->check_item($item);
    $langcode = $langcode ? $langcode : i18n_object_langcode($item);
    if ($item && $langcode) {
      $this->get_translations();
      $this->translations[$langcode] = $item;
    }
    return $this;
  }

  /**
   * Remove item / language from translation set
   * 
   * @param $item
   *   Item to remove from this translation set, it must have a language property.
   */
  public function remove_item($item) {
    return $this->remove_language(i18n_object_langcode($item));
  }

  /**
   * Remove language from translation set.
   * 
   * @param $langcode
   *   Language to remove from this translation set.
   */
  public function remove_language($langcode) {
    $this->get_translations();
    if ($langcode && isset($this->translations[$langcode])) {
      unset($this->translations[$langcode]);
    }    
  }

  /**
   * Check item has translation set property
   */
  public function check_item($item) {
    $field = $this->get_field();
    if (is_object($item)) {
      $item->$field = $this->tsid;
    }
    elseif (is_array($item)) {
      $item[$field] = $this->tsid;
    }
    return $item;
  }
  /**
   * Load all translations as objects indexed by language
   */
  public function load_translations() {
    if (($table = $this->get_table()) && ($field = $this->get_field())) {
      return db_select($table, 't')
        ->fields('t')
        ->condition('t.' . $field, $this->tsid)
        ->execute()
        ->fetchAllAssoc('language');
    }
    else {
      return array();
    }
  }

  /**
   * Get key field for this translation items
   */
  protected function key_field() {
    $info = i18n_object_info($this->type);
    return $info['key'];
  }
  
  /**
   * Access to object translation. This should check object properties and permissions
   */
  public static function translate_access($object) {
    return FALSE;
  }
  /**
   * Translation mode for object
   */
  public static function translate_mode($object) {
    return I18N_MODE_NONE;
  }
}