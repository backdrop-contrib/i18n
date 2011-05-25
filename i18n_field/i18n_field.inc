<?php
/**
 * Blocks textgroup handler
 */
class i18n_field_textgroup extends i18n_string_default {
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
      case 'field':       
      case 'field_instance':
        $info = entity_get_info($object['entity_type']);
        if (isset($info['bundles'][$object['bundle']]['admin'])) {
          // Extract path information from the bundle.
          $path = $info['bundles'][$object['bundle']]['admin']['real path'];
          return $path . '/fields/' . $object['field_name'];
        } 
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
      case 'field':
        return $this->get_edit_path($type, $object) . '/translate/field' . ($language ? '/' . $language->language : '');
      case 'field_instance':
        return $this->get_edit_path($type, $object) . '/translate/instance' . ($language ? '/' . $language->language : '');
      default:
        return parent::get_translate_path($type, $object, $language);
    }
  }
  /**
   * Get base keys for translating this object
   */
  public function get_object_context($type, $object) {
    switch ($type) {
      case 'field':
        return array($object['field_name'], '#field');      
      case 'field_instance':
        return array($object['field_name'], $object['bundle']);
      default:
        return parent::get_object_context($type, $object);
    }
  }

  /**
   * Get object strings for translation
   */
  public function get_object_properties($type, $object) {
    $properties = parent::get_object_properties($type, $object);
    switch ($type) {
      case 'field':
        // For select fields field:field_name
        if (!empty($object['settings']['allowed_values']) && i18n_field_type_info($object['type'], 'translate_options')) {
          //return array('field', $field['field_name'], '#allowed_values');
          foreach ($object['settings']['allowed_values'] as $key => $value) {
            $properties[$this->textgroup][$object['field_name']]['#allowed_values'][$key] = array(
              'title' => t('Option %name', array('%name' => $value)),
              'string' => $value,
            );
          }
        }
        break;
      case 'field_instance':
        $field = field_info_field($object['field_name']);
        // Only for text field types
        if (isset($object['default_value']) && i18n_field_type_info($field['type'], 'translate_default')) {
          $format = isset($object['default_value'][0]['format']) ? $object['default_value'][0]['format'] : NULL;
          $properties[$this->textgroup][$object['field_name']][$object['bundle']]['default_value']['string'] = $object['default_value'][0]['value'];
          $properties[$this->textgroup][$object['field_name']][$object['bundle']]['default_value']['format'] = $format;
        }        
        break;
    }
    return $properties;
  }
}