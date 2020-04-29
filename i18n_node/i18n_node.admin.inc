<?php

/**
 * @file
 * The i18n_node module admin form.
 */

/**
 * Additonal content option settings.
 */
function i18n_node_content_options_form($form, &$form_state) {
  $form['hide_translation_links'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hide content translation links'),
    '#description' => t('Hide the links to translations in content body and teasers. If you choose this option, switching language will only be available from the language switcher block.'),
    '#default_value' => config_get('i18n.settings', 'i18n_node.hide_translation_links'),
  );
  $form['default_language_none'] = array(
    '#type' => 'select',
    '#title' => t('Default language for content types with Multilingual support disabled.'),
    '#description' => t("Determines which language will be set for newly created content of types that don't have Multilingual support enabled."),
    '#options' => array(
      0 => t("The site's default language (Default behaviour)."),
      1 => t('Language neutral (Recommended).'),
    ),
    '#default_value' => config_get('i18n.settings', 'i18n_node.default_language_none'),
  );
  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
    '#submit' => array('i18n_node_content_options_form_submit'),
  );

  return $form;
}

/**
 * Custom callback for i18n_node_content_options_form.
 */
function i18n_node_content_options_form_submit($form, &$form_state) {
  config_set('i18n.settings', 'i18n_node.hide_translation_links', $form_state['values']['hide_translation_links']);
  config_set('i18n.settings', 'i18n_node.default_language_none', $form_state['values']['default_language_none']);
  backdrop_set_message(t('The configuration options have been saved.'));
}
