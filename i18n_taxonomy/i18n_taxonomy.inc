<?php
/**
 * @file
 * Internationalization (i18n) module - Translation set
 */
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

/**
 * Taxonomy textgroup handler
 */
class i18n_taxonomy_term extends i18n_string_object {
  /**
   * Get edit path for object
   */
  public function get_edit_path() {
     return 'taxonomy/term/' . $this->object->tid . '/edit';
  }
  /**
   * Get translate path for object
   */
  public function get_translate_path($language = NULL) {
    return 'taxonomy/term/' . $this->object->tid . '/translate' . ($language ? '/' . $language->language : '');
  }
  /**
   * Translation mode for object
   */
  protected function translate_mode() {
    $mode = i18n_taxonomy_vocabulary_mode($this->object->vid);
    if ($this->get_langcode()) {
      return $mode & I18N_MODE_TRANSLATE;
    }
    else {
      return $mode & I18N_MODE_LOCALIZE;
    }
  }
  /**
   * Access to object translation. This should check object properties and permissions
   */
  protected function translate_access() {
    return taxonomy_term_edit_access($this->object) && $this->get_langcode() && user_access('translate interface');
  }
}

/**
 * Taxonomy textgroup handler
 */
class i18n_taxonomy_vocabulary extends i18n_string_object {
  /**
   * Get edit path for object
   */
  public function get_edit_path() {
    return 'admin/structure/taxonomy/' . $this->object->machine_name . '/edit';
  }
  /**
   * Get translate path for object
   */
  public function get_translate_path($language = NULL) {
    return 'admin/structure/taxonomy/' . $this->object->machine_name . '/translate' . ($language ? '/' . $language->language : '');
  }
}