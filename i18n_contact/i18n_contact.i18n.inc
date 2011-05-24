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
