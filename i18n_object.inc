<?php
/**
 * Object wrapper
 */
class i18n_object_wrapper {
  protected $type;
  protected $object;

  /**
   * Class constructor
   */
  public function __construct($type, $object) {
    $this->type = $type;
    $this->object = $object;
  }

  
  /**
   * Get edit path for object
   */
  function get_edit_path() {
    if ($path = $this->get_info('edit path')) {
      return strtr($path, $this->get_placeholders());
    }
    else {
      return NULL;
    }
  }

  /**
   * Get field value from object/array
   */
  function get_field($field, $default = NULL) {
    return i18n_object_field($this->object, $field, $default);
  }

  /**
   * Get key value from object/array
   */
  function get_key($default = NULL) {
    if ($field = $this->get_info('key')) {
      return $this->get_field($field, $default);
    }
    else {
      return $default;
    }
  }

  /**
   * Get language code
   */
  public function get_langcode() {
    return i18n_object_langcode($this->object);
  }

  /**
   * Get real object or array
   */
  public function get_object() {
    return $this->object;
  }

  /**
   * Get menu placehoders for object
   */
  protected function get_placeholders() {
    $placeholders = $this->get_info('placeholders', array());
    foreach ($placeholders as $name => $field) {
      $placeholders[$name] = $this->get_field($field);
    }
    return $placeholders;
  }
  /**
   * Get title from item
   */
  public function get_title() {
    return entity_label($this->type, $this->object);
  }

  /**
   * Get link for item
   */
  public function get_path() {
    if ($uri = entity_uri($this->type, $this->object)) {
      return $uri['path'];
    }
  }

  /**
   * Menu access callback for mixed translation tab
   */
  function get_translate_access() {
    switch ($this->get_translate_mode()) {
      case I18N_MODE_TRANSLATE:
        return $this->translate_access();
      case I18N_MODE_LOCALIZE:
        return $this->localize_access();
      default:
        return FALSE;
    }
  }

  /**
   * Get translate or localize mode for object
   */
  function get_translate_mode() {
    return I18N_MODE_NONE;
  }  

  /**
   * Localize access
   */
  protected function translate_access() {
    return FALSE;
  }

  /**
   * Translate access (localize strings)
   */
  protected function localize_access() {
    return user_access('translate interface');
  }

  /**
   * Get object info
   */
  public function get_info($property) {
    return i18n_object_info($this->type, $property);
  }
}