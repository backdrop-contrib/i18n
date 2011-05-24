<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */

/**
 * Implements hook_i18n_string_info().
 */
function i18n_field_i18n_string_info() {
  $groups['field'] = array(
    'title' => t('Fields'),
    'description' => t('Configurable fields descriptions, defaults, options, etc.'),
    'format' => FALSE, // This group doesn't have formatted strings
    'list' => TRUE, // This group can list all strings
  );
  return $groups;
}

/**
 * Implements hook_i18n_string_list().
 */
function i18n_field_i18n_string_list($group) {
  if ($group == 'field'  || $group == 'all') {
    $strings = array();
    foreach (field_read_fields() as $field) {
      if (!empty($field['settings']['allowed_values']) && i18n_field_type_info($field['type'], 'translate_options')) {
        $strings['field'][$field['field_name']]['#allowed_values'] = $field['settings']['allowed_values'];
      }
    }
    foreach (field_read_instances() as $instance) {
      $add = array(
        'label' => $instance['label'],
        'description' => $instance['description'],
      );
      // Only for some field types, like text field
      $field = field_info_field($instance['field_name']);
      if (isset($instance['default_value']) && i18n_field_type_info($field['type'], 'translate_default')) {
        $format = isset($instance['default_value'][0]['format']) ? $instance['default_value'][0]['format'] : NULL;
        $add['default_value'] = array(
          'string' => $instance['default_value'][0]['value'],
          'format' => $format,
        );
      }
      $strings['field'][$instance['field_name']][$instance['bundle']] = $add;
    }
    return $strings;
  }
}


/**
 * Implements hook_i18n_field_info().
 */
function i18n_field_i18n_field_info() {
  $info['text'] = $info['text_long'] = $info['text_with_summary'] = array(
    'translate_default' => 'i18n_field_translate_default',
  );
  $info['list_text'] = $info['list_boolean'] = $info['list_integer'] = $info['list_integer'] = array(
    'translate_options' => 'i18n_field_translate_allowed_values',
  );
  return $info;
}