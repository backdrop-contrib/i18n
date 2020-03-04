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
