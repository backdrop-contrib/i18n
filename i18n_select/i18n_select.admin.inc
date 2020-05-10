<?php

/**
 * @file
 * Forms for i18n select administration.
 */

/**
 * Admin settings form.
 */
function i18n_select_admin_settings() {
  $config = config_get('i18n_select.settings');
  $form['#config'] = 'i18n_select.settings';
  $form['types'] = array(
    '#type' => 'fieldset',
    '#title' => t('Content to filter by language'),
  );
  $form['types']['i18n_select_nodes'] = array(
    '#title' => t('Select nodes by language'),
    '#type' => 'checkbox',
    '#default_value' => isset($config['i18n_select_nodes']) ? $config['i18n_select_nodes'] : TRUE,
  );
  $form['types']['i18n_select_taxonomy'] = array(
    '#title' => t('Select taxonomy terms by language'),
    '#type' => 'boolean',
    '#default_value' => isset($config['i18n_select_taxonomy']) ? $config['i18n_select_taxonomy'] : TRUE,
    '#disabled' => !module_exists('i18n_taxonomy'),
  );

  $form['mode'] = array(
    '#type' => 'fieldset',
    '#title' => t('Content selection mode'),
  );
  $form['mode']['i18n_select_skip_tags'] = array(
    '#title' => t('Skip tags'),
    '#type' => 'textfield',
    '#default_value' => isset($config['i18n_select_skip_tags']) ? $config['i18n_select_skip_tags'] : 'views',
    '#description' => t('Skip queries with these tags. Enter a list of tags separated by commas.'),
  );

  // Enable for specific pages. This works pretty much like block visibility
  // Note this page requires 'administer site configuration'
  // so we don't need to check permissions.
  $form['pages'] = array(
    '#type' => 'fieldset',
    '#title' => t('Enable for specific pages'),
  );
  $form['pages']['i18n_select_page_mode'] = array(
    '#type' => 'select',
    '#title' => t('Select'),
    '#options' => i18n_select_option_list(),
    '#default_value' => isset($config['i18n_select_page_mode']) ? $config['i18n_select_page_mode'] : I18N_SELECT_PAGE_NOTLISTED,
  );
  $description = t("Specify pages by using their paths. Enter one path per line. The '*' character is a wildcard. Example paths are %blog for the blog page and %blog-wildcard for every personal blog. %front is the front page.", array(
    '%blog' => 'blog',
    '%blog-wildcard' => 'blog/*',
    '%front' => '<front>',
  ));
  $form['pages']['i18n_select_page_list'] = array(
    '#type' => 'textarea',
    '#title' => t('Pages'),
    '#default_value' => isset($config['i18n_select_page_list']) ? $config['i18n_select_page_list'] : 'admin/*',
    '#description' => $description,
  );
  $form['pages']['i18n_select_page_block'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable always for block content though it may be disabled for the page'),
    '#default_value' => isset($config['i18n_select_page_block']) ? $config['i18n_select_page_block'] : TRUE,
  );

  return system_settings_form($form);
}

/**
 * Options for page selection mode.
 */
function i18n_select_option_list() {
  $options = array(
    I18N_SELECT_PAGE_NOTLISTED => t('All pages except those listed'),
    I18N_SELECT_PAGE_LISTED => t('Only the listed pages'),
  );
  return $options;
}
