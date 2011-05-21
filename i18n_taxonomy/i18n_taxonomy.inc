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
  public function get_translate_path($type, $object) {
    switch ($type) {
      case 'taxonomy_term':
        return 'taxonomy/term/' . $object->tid . '/translate';
      case 'taxonomy_vocabulary':
        return 'admin/structure/taxonomy/' . $object->machine_name . '/translate';
    }
  }
}