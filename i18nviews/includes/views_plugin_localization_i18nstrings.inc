<?php
// $Id$
/**
 * @file
 * Contains the i18nstrings localization plugin.
 */

/**
 * Localization plugin to pass translatable strings through tt().
 *
 * @ingroup views_localization_plugins
 */
class views_plugin_localization_i18nstrings extends views_plugin_localization {

  /**
   * Translate a string.
   *
   * @param $string
   *   The string to be translated.
   * @param $keys
   *   An array of keys to identify the string. Generally constructed from
   *   view name, display_id, and a property, e.g., 'header'.
   */
  function translate($string, $keys = array()) {
    return i18nstrings($this->stringid($keys), $string);
  }

  /**
   * Save a string for translation.
   *
   * @param $string
   *   The string to be saved.
   * @param $keys
   *   An array of keys to identify the string. Generally constructed from
   *   view name, display_id, and a property, e.g., 'header'.
   */
  function save($string, $keys = array()) {
    i18nstrings_update($this->stringid($keys), $string);
    return TRUE;
  }

  /**
   * Delete a string.
   *
   * @param $string
   *   The string to be deleted.
   * @param $keys
   *   An array of keys to identify the string. Generally constructed from
   *   view name, display_id, and a property, e.g., 'header'.
   */
  function delete($string, $keys = array()) {
    i18nstrings_remove($this->stringid($keys), $string);
    return TRUE;
  }
  
  /**
   * Get string id for i18n
   *
   * @param $keys
   *   An array of keys to identify the string. Generally constructed from
   *   view name, display_id, and a property, e.g., 'header'.
   * @return $string
   *   String id for i18nstrins
   */
  function stringid($keys) {
    return 'views:' . implode(':', $keys);
  }
}

