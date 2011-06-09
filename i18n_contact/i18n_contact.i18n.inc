<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */

/**
 * Implements hook_i18n_string_info().
 */
function i18n_contact_i18n_string_info() {
  $groups['contact'] = array(
    'title' => t('Contact forms'),
    'description' => t('Configurable contact form categories.'),
    'format' => FALSE,
    'list' => TRUE,
  );
  return $groups;
}

/**
 * Implements hook_i18n_object_info().
 */
function i18n_contact_i18n_object_info() {
  $info['contact'] = array(
    'title' => t('Contact category'),
    'key' => 'cid',
    'placeholders' => array(
      '%contact' => 'cid',
    ),
    'edit path' => 'admin/structure/contact/edit/%contact',
    'translate path' => 'admin/structure/contact/edit/%contact/translate',
    'string translation' => array(
      'textgroup' => 'contact',
      'type' => 'category',
      'properties' => array(
        'category' => t('Category'),
        'reply' => t('Auto-reply'),
      ),
      'translate path' => 'admin/structure/contact/edit/%contact/translate/%language',
    )
  );
  return $info;
}

/**
 * Implements hook_i18n_string_list().
 */
function i18n_contact_i18n_string_list($group) {
  if ($group == 'contact' || $group == 'all') {
    $strings = array();
    $query = db_select('contact', 'c')->fields('c');
    $result = $query->execute()->fetchAll();
    foreach ($result as $contact) {
      $strings['contact']['category'][$contact->cid]['category'] = $contact->category;
      if (!empty($contact->reply)) {
        $strings['contact']['category'][$contact->cid]['reply'] = $contact->reply;
      }
    }
    return $strings;
  }
}
