<?php
/**
 * @file
 * The i18n_node module admin forms.
 */

/**
 * Additonal content option settings.
 */
function i18n_node_content_options_form($form, &$form_state) {
  $form['i18n_hide_translation_links'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hide content translation links'),
    '#description' => t('Hide the links to translations in content body and teasers. If you choose this option, switching language will only be available from the language switcher block.'),
    '#default_value' => FALSE,// TODO config_get
  );
  $form['i18n_node_default_language_none'] = array(
    '#type' => 'select',
    '#title' => t('Default language for content types with Multilingual support disabled.'),
    '#description' => t('Determines which language will be set for newly created content of types that don\'t have Multilingual support enabled.'),
    '#options' => array(
      0 => t('The site\'s default language (Default behaviour).'),
      1 => t('Language neutral (Recommended).'),
    ),
    '#default_value' => FALSE,// TODO config_get
  );
  return $form;
}

/**
 * Form items for settings per content type.
 */
function _i18n_node_content_type_settings() {
  $form['i18n_node_options'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Extended language options'),
    '#options' => array(
      'current' => t('Set current language as default for new content.'),
      'required' => t('Require language (Do not allow Language Neutral).'),
      'lock' => t('Lock language (Cannot be changed).'),
    ),
    '#default_value' => array(),// TODO config_get
  );
  $form['i18n_node_extended'] = array(
    '#type' => 'radios',
    '#title' => t('Extended language support'),
    '#options' => array(
      I18N_LANGUAGE_ENABLED => t('Normal - All enabled languages will be allowed.'),
      I18N_LANGUAGE_EXTENDED => t('Extended - All defined languages will be allowed.'),
      I18N_LANGUAGE_EXTENDED | I18N_LANGUAGE_HIDDEN => t('Extended, but not displayed - All defined languages will be allowed for input, but not displayed in links.'),
    ),
    '#default_value' => I18N_LANGUAGE_ENABLED,// TODO config_get
    '#description' => t('If enabled, all defined languages will be allowed for this content type in addition to only enabled ones. This is useful to have more languages for content than for the interface.'),
  );
  return $form;
}
