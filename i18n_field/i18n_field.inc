<?php
/**
 * @file
 * Field and field instance object handlers
 */

/**
 * Field object
 */
class i18n_field extends i18n_string_object_wrapper {
  /**
   * Get edit path for object
   */
  public function get_edit_path() {
    $info = entity_get_info($this->object['entity_type']);
    if (isset($info['bundles'][$this->object['bundle']]['admin'])) {
      // Extract path information from the bundle.
      $path = $info['bundles'][$this->object['bundle']]['admin']['real path'];
      return $path . '/fields/' . $this->object['field_name'];
    }
  }
  /**
   * Get translate path for object
   */
  public function get_translate_path($langcode = NULL) {
    return $this->get_edit_path() . '/translate/field' . ($langcode ? '/' . $$langcode : '');
  }  
  /**
   * Get string context
   */
  public function get_string_context() {
    return array($this->object['field_name'], '#field');
  }
  /**
   * Get translatable properties
   */
  protected function build_properties() {
    $properties = parent::build_properties();
    $object = $this->object;
    // For select fields field:field_name
    if (!empty($object['settings']['allowed_values']) && i18n_field_type_info($object['type'], 'translate_options')) {
      //return array('field', $field['field_name'], '#allowed_values');
      foreach ($object['settings']['allowed_values'] as $key => $value) {
        $properties[$this->get_textgroup()][$object['field_name']]['#allowed_values'][$key] = array(
          'title' => t('Option %name', array('%name' => $value)),
          'string' => $value,
        );
      }
    }
    return $properties;
  }
 
  /**
   * Context to be pre-loaded before translation.
   */
  protected function get_translate_context($langcode, $options) {
    return array(
      $this->object['field_name'],
      array('#field', '#allowed_values'),
    	'*'
    );
  }

  /**
   * Set field translation for object.
   * 
   * Mot often, this is a direct field set, but sometimes fields may have different formats.
   * 
   * @param $object
   *   A clone of the object or array. Field instance.
   */
  protected function translate_field(&$object, $i18nstring, $langcode, $options) {
    if ($i18nstring->objectid == '#allowed_values') {
      $object['settings']['#allowed_values'][$i18nstring->key] = $i18nstring->format_translation($langcode, $options);
    }
    else {
      parent::translate_field($object, $i18nstring, $langcode, $options);
    }
  }
}

/**
 * Field instance object
 */
class i18n_field_instance extends i18n_string_object_wrapper {
  /**
   * Get edit path for object
   */
  public function get_edit_path() {
    $info = entity_get_info($this->object['entity_type']);
    if (isset($info['bundles'][$this->object['bundle']]['admin'])) {
      // Extract path information from the bundle.
      $path = $info['bundles'][$this->object['bundle']]['admin']['real path'];
      return $path . '/fields/' . $this->object['field_name'];
    }
  }  
  /**
   * Get translate path for object
   */
  public function get_translate_path($langcode = NULL) {
    return $this->get_edit_path() . '/translate/instance' . ($langcode ? '/' . $$langcode : '');
  }  
  /**
   * Get string context
   */
  public function get_string_context() {
    return array($this->object['field_name'], $this->object['bundle']);
  }
  /**
   * Get translatable properties
   */
  protected function build_properties() {
    $properties = parent::build_properties();
    $object = $this->object;
    $field = field_info_field($object['field_name']);
    // Only for text field types
    if (isset($object['default_value']) && i18n_field_type_info($field['type'], 'translate_default')) {
      $format = isset($object['default_value'][0]['format']) ? $object['default_value'][0]['format'] : NULL;
      $properties[$this->get_textgroup()][$object['field_name']][$object['bundle']]['default_value']['string'] = $object['default_value'][0]['value'];
      $properties[$this->get_textgroup()][$object['field_name']][$object['bundle']]['default_value']['format'] = $format;
    }
    return $properties;
  }
  
  /**
   * Set field translation for object.
   * 
   * Mot often, this is a direct field set, but sometimes fields may have different formats.
   * 
   * @param $object
   *   A clone of the object or array. Field instance.
   */
  protected function translate_field(&$object, $i18nstring, $langcode, $options) {
    if ($i18nstring->property == 'default_value') {
      // Render string without applying format
      $object['default_value'][0]['value'] = $i18nstring->format_translation($langcode, array('sanitize' => FALSE) + $options);
    }
    else {
      parent::translate_field($object, $i18nstring, $langcode, $options);
    }
  }
  
  /**
   * Context to be pre-loaded before translation.
   */
  protected function get_translate_context($langcode, $options) {
    return array(
      $this->object['field_name'],
      array('#field', '#allowed_values', $this->object['bundle']),
    	'*'
    );   
  }
}