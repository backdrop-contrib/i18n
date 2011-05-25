<?php
/**
 * @file
 * Internationalization (i18n) module - Translation set
 */
class i18n_taxonomy_translation_set extends i18n_translation_set {
  /**
   * Load all path translations
   */
  public function load_translations() {
    return i18n_translation_set_index(taxonomy_term_load_multiple(array(), array('i18n_tsid' => $this->tsid)));
  }
  /**
   * Get placeholder values for path replacement
   */
  function get_path_placeholders($op = 'list') {
    $values = parent::get_path_placeholders($op);
    if (!empty($this->bundle)) {
      $values['%taxonomy_vocabulary_machine_name'] = $this->bundle; 
    }
    return $values;
  }

  /**
   * Access to object translation. This should check object properties and permissions
   */
  public static function translate_access($term) {
    return taxonomy_term_edit_access($term) && user_access('translate interface') && self::translate_mode($term) & I18N_MODE_TRANSLATE;
  }
  /**
   * Translation mode for object
   */
  public static function translate_mode($term) {
    $mode = i18n_taxonomy_vocabulary_mode($term->vid);
    if (i18n_object_langcode($term)) {
      return $mode & I18N_MODE_TRANSLATE;
    }
    else {
      return $mode & I18N_MODE_LOCALIZE;
    }
  }
}

/**
 * Taxonomy textgroup handler
 */
class i18n_taxonomy_textgroup extends i18n_string_default {
  /**
   * Get edit path for object
   *
   * @param $type
   * 	 Object type in this text group, like 'vocabulary', 'term', etc..
   * @param $object
   *   The Drupal object itself
   */
  public function get_edit_path($type, $object) {
    switch ($type) {
      case 'taxonomy_term':
        return 'taxonomy/term/' . $object->tid . '/edit';
      case 'taxonomy_vocabulary':
        return 'admin/structure/taxonomy/' . $object->machine_name . '/edit';
      default:
        return parent::get_edit_path($type, $object);
    }
  }
  /**
   * Get translate path for object
   *
   * @param $type
   * 	 Object type in this text group, like 'vocabulary', 'term', etc..
   * @param $object
   *   The Drupal object itself
   */
  public function get_translate_path($type, $object, $language = NULL) {
    switch ($type) {
      case 'taxonomy_term':
        return 'taxonomy/term/' . $object->tid . '/translate' . ($language ? '/' . $language->language : '');
      case 'taxonomy_vocabulary':
        return 'admin/structure/taxonomy/' . $object->machine_name . '/translate' . ($language ? '/' . $language->language : '');
      default:
        return parent::get_translate_path($type, $object, $language);
    }
  }
}