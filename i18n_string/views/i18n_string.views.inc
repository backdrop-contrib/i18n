<?php
/**
 * @file
 * Provides additional views data and handlers for i18n_string.
 */

/**
 * Implements hook_views_data_alter().
 */
function i18n_string_views_data_alter(&$data) {
  // Textgroup field and handlers, formerly provided by core, now by
  // i18n_string.
  $data['locales_source']['textgroup'] = array(
    'group' => t('Locale source'),
    'title' => t('Group'),
    'help' => t('The text group the translation is in.'),
    'field' => array(
      'handler' => 'i18n_string_views_handler_field_locale_group',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'i18n_string_views_handler_filter_locale_group',
    ),
    'argument' => array(
      'handler' => 'i18n_string_views_handler_argument_locale_group',
    ),
  );
}
