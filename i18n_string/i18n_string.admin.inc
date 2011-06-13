<?php

/**
 * @file
 * Helper functions for string administration.
 */

// Load locale library
include_once './includes/locale.inc';

/**
 * Form callback. Refresh textgroups.
 */
function i18n_string_admin_refresh_form() {
  module_load_include('inc', 'i18n_string');
  // Select textgroup/s. Just the ones that have a 'refresh callback'
  $groups = array();
  foreach (i18n_string_group_info() as $name => $info) {
    $groups[$name] = $info['title'];
  }
  $form['groups'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Select text groups'),
    '#options' => $groups,
    '#description' => t('If a text group is no showing up here it means this feature is not implemented for it.'),
  );
  $form['delete'] = array(
    '#type' => 'checkbox',
    '#title' => t('Clean up left over strings.'),
    '#default_value' => TRUE,
  );
  $form['refresh'] = array(
    '#type' => 'submit',
    '#value' => t('Refresh strings'),
    '#suffix' => '<p>' . t('This will create all the missing strings for the selected text groups.') . '</p>',
  );
  // Get all languages, except default language.
  $languages = locale_language_list('name', TRUE);
  unset($languages[language_default('language')]);
  $form['languages'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Select languages'),
    '#options' => $languages,
  );
  $form['update'] = array(
    '#type' => 'submit',
    '#value' => t('Update translations'),
    '#suffix' => '<p>' . t('This will fetch all existing translations from the localization tables for the selected text groups and languages.') . '</p>',
  );
  return $form;
}

/**
 * Form submission.
 */
function i18n_string_admin_refresh_form_submit($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  $groups = array_filter($form_state['values']['groups']);
  $languages = array_filter($form_state['values']['languages']);
  $group_names = module_invoke_all('locale', 'groups');
  if ($op == t('Refresh strings') && $groups) {
    $batch = i18n_string_refresh_batch($groups, $form_state['values']['delete']);
    batch_set($batch);
  }
  elseif ($op == t('Update translations') && $groups && $languages) {
    $count = 0;
    foreach ($languages as $language) {
      $count += i18n_string_admin_update($language, $groups);
    }
    drupal_set_message(format_plural($count, '1 string has been updated.', '@count strings have been updated.'));
  }
}

/**
 * Update strings for language.
 */
function i18n_string_admin_update($language, $groups) {
  $query = db_select('locales_source', 'g')
    ->fields('g')
    ->fields('t', array('translation'))
    ->fields('i', array('format'))
    ->condition('t2.lid', NULL)
    ->condition('g.textgroup', $groups)
    ->condition('t.language', $language);
  $query->addField('t', 'lid', 'tlid');
  $query->join('locales_source', 's', 'g.source = s.source AND s.lid <> g.lid');
  $query->join('locales_target', 't', 's.lid = t.lid');
  $query->join('locales_target', 't2', 'g.lid = t2.lid');
  $query->join('i18n_string', 'i', 'i.lid = g.lid');
  $result = $query->execute()->fetchAll(PDO::FETCH_OBJ);

  $count = 0;
  foreach ($result as $string) {
    // Just update strings when no text format, otherwise it could be dangerous under some circumstances.
    if (empty($string->format) && !empty($string->translation)) {
      $count++;
      db_insert(locales_target)
      ->fields(array(
        'translation' => $string->translation,
        'lid' => $string->lid,
        'language' => $language,
      ))
      ->execute();
    }
  }
  return $count;
}

/**
 * Configure filters for string translation.
 *
 * This has serious security implications so this form needs the 'administer filters' permission
 */
function i18n_string_admin_settings() {
  module_load_include('inc', 'i18n_string');
  include_once './includes/locale.inc';
  // As the user has administer filters permissions we get a full list here
  foreach (filter_formats() as $fid => $format) {
    $format_list[$fid] = $format->name;
  }
  $form['i18n_string_allowed_formats'] = array(
    '#title' => t('Translatable text formats'),
    '#options' => $format_list,
    '#type' => 'checkboxes',
    '#default_value' => variable_get('i18n_string_allowed_formats', array(filter_fallback_format())),
    '#description' => t('Only the strings that have the text formats selected will be allowed by the translation system. All the others will be deleted next time the strings are refreshed.'),
  );
  $form['array_filter'] = array('#type' => 'value', '#value' => TRUE);
  return system_settings_form($form);
}

/**
 * Refresh all user defined strings for a given text group.
 *
 * @param $group
 *   Text group to refresh
 * @param $delete
 *   Optional, delete existing (but not refresed, strings and translations)
 * @return Boolean
 *   True if the strings have been refreshed successfully. False otherwise.
 */
function i18n_string_refresh_group($group, $delete = FALSE) {
  $result = FALSE;
  
  // Compile all strings for this group
  if ($strings = i18n_string_group_string_list($group)) {
    i18n_string_refresh_string_list($strings);
    $result = TRUE;
  }
  // Invoke refresh hook
  $result = $result || module_invoke_all('i18n_string_refresh', $group);

  // Now delete all source strings that were not refreshed (they don't have a row in i18n_string)
  if ($result && $delete) {
    i18n_string_refresh_cleanup($group);
  }
  return $result;
}

/**
 * Clean up left over strings for text group
 */
function i18n_string_refresh_cleanup($group) {
  $lids = db_select('locales_source', 's')
    ->fields('s', array('lid'))
    ->condition('textgroup', $group)
    ->condition('version', 0)
    ->execute()
    ->fetchCol();
  if ($lids) {
    drupal_set_message(t('Performing cleanup for text group %textgroup, deleting @count left over strings.', array('%textgroup' => $group, '@count' => count($lids))));
    db_delete('locales_target')->condition('lid', $lids)->execute();
    db_delete('locales_source')->condition('lid', $lids)->execute();
    db_delete('i18n_string')->condition('lid', $lids)->execute();
    return count($lids);
  }
  else {
    return 0;
  }
}

/**
 * Prepare group for refreshing, reset version, count strings
 */
function i18n_string_refresh_reset($group) {
  // Mark data on locales_source setting version = 0
  db_update('locales_source')
    ->fields(array('version' => 0))
    ->condition('textgroup', $group)
    ->execute();
  return (int)db_query("SELECT COUNT(*) FROM {locales_source} WHERE textgroup = :textgroup", array(':textgroup' => $group))->fetchField();
}

/**
 * Refresh string list
 */
function i18n_string_refresh_string_list($strings) {
  $count = 0;
  foreach ($strings as $textgroup => $group_strings) {
    foreach ($group_strings as $type => $type_strings) {
      foreach ($type_strings as $id => $object_strings) {
        foreach ($object_strings as $key => $string) {
          if (is_array($string)) {
            $format = isset($string['format']) ? $string['format'] : NULL;
            $string = $string['string'];
          }
          else {
            $format = NULL;
          }
          i18n_string_update(array($textgroup, $type, $id, $key), $string, array('format' => $format));
          $count++;
        }
      }
    }
  }
  return $count;
}

/**
 * Create batch for refreshing strings
 *
 * @param $groups
 *   Array of text groups to refresh
 * @param $delete
 *   Optional, delete existing (but not refresed, strings and translations)
 */
function i18n_string_refresh_batch($groups, $delete = FALSE) {
  $operations = array();
  foreach ($groups as $group) {
    $operations[] = array('_i18n_string_batch_refresh_prepare', array($group));
    // First try to find string list
    $operations[] = array('_i18n_string_batch_refresh_list', array($group));
    // Then invoke refresh callback
    $operations[] = array('_i18n_string_batch_refresh_callback', array($group));
    if ($delete) {
      $operations[] = array('_i18n_string_batch_refresh_cleanup', array($group));
    }
    // Output group summary
    $operations[] = array('_i18n_string_batch_refresh_summary', array($group));
  }
  $batch = array(
    'operations'    => $operations,
    'title'         => t('Refreshing user defined strings'),
    'init_message'  => t('Starting string refresh'),
    'error_message' => t('Error refreshing user defined strings'),
    'file' => drupal_get_path('module', 'i18n_string') . '/i18n_string.admin.inc',
  );
  return $batch;
}

/**
 * Refresh strings for enabled modules
 */
function i18n_string_refresh_enabled_modules($modules) {
  // Check if any of the modules has strings to update
  $count = 0;
  foreach ($modules as $module) {
    if ($strings = i18n_string_module_string_list($module)) {
      $count += i18n_string_refresh_string_list($strings);
     
    }
    // Call module refresh if exists
    module_invoke($module, 'i18n_string_refresh', 'all');
  }
  if ($count) {
    drupal_set_message(format_plural($count, 'Refresed one string for enabled modules.', 'Refreshed @count strings for the enabled modules.'));
  }
}

/**
 * Purge uninstalled modules.
 */
function i18n_string_refresh_uninstalled_modules($modules) {
  foreach ($modules as $module) {
    // If the module defines any textgroup, purge all strings.
    module_load_include('i18n.inc', $module);
    if ($string_info = module_invoke($module, 'i18n_string_info')) {
      foreach ($string_info as $group => $group_info) {
        i18n_string_refresh_reset($group);
        i18n_string_refresh_cleanup($group);
      }
    }
  }
}

/**
 * Prepare group for refreshing
 */
function _i18n_string_batch_refresh_prepare($group, &$context) {
  $context['results'][$group] = array(
    'count' => i18n_string_refresh_reset($group),
    'result' => FALSE,
  );
}

/**
 * Batch operation: Refresh string list for group
 */
function _i18n_string_batch_refresh_list($group, &$context) {
  $count = 0;
  if ($strings = i18n_string_group_string_list($group)) {
    $count = i18n_string_refresh_string_list($strings);
    $context['results'][$group]['result'] = TRUE;
  }
  $context['results'][$group]['refresh'] = $count;
}

/**
 * Batch operation: Invoke i18n_string_refresh
 */
function _i18n_string_batch_refresh_callback($group, &$context) {
  $result = module_invoke_all('i18n_string_refresh', $group);
  $count = $result ? array_sum($result) : 0;
  $context['results'][$group]['refresh'] += $count;
  if ($count) {
    $context['results'][$group]['result'] = TRUE;
  }
}

/**
 * Batch callback, delete old strings
 */
function _i18n_string_batch_refresh_cleanup($group, &$context) {
  if (!empty($context[$group]['result'])) {
    $context['results'][$group]['deleted'] = i18n_string_refresh_cleanup($group);
  }
}

/**
 * Batch operations: Print refresh summary for group
 */
function _i18n_string_batch_refresh_summary($group, &$context) {
  if ($context['results'][$group]['result']) {
    drupal_set_message(t("Successfully refreshed @count strings for %group", array('@count' => $context['results'][$group]['refresh'], '%group' => i18n_string_group_info($group, 'title'))));
    if (!empty($context['results'][$group]['deleted'])) {
      drupal_set_message(t('Deleted @count left over strings.', array('@count' => $context['results'][$group]['deleted'])));
    }
  }
  else {
    drupal_set_message(t("Cannot refresh strings for %group.", array('%group' => i18n_string_group_info($group, 'title'))), 'warning');
  }
}

/**
 * Get all strings for a text group
 */
function i18n_string_group_string_list($group) {
  // Add strings provided by all modules on hook_string_list().
  // Note that i18n_string module itself will also collect all strings for this group's objects
  $strings = module_invoke_all('i18n_string_list', $group);
  // Invoke hook_i18n_string_list_TEXTGROUP_alter()
  drupal_alter('i18n_string_list_' . $group, $strings);
  return $strings;
}

/**
 * Get all strings from a module.
 */
function i18n_string_module_string_list($module) {
  $strings = array();
  // Try loading i18n.inc for the module
  module_load_include('i18n.inc', $module);
  // If the module defines any textgroup, get all strings for this group
  if ($groups = module_invoke($module, 'i18n_string_info')) {
    foreach ($groups as $group) {
      $strings = array_merge_recursive($strings, i18n_string_group_string_list($group));
    }   
  }
  else {
    $groups = array();
  }
  // The module may implement i18n_string_list()
  if ($string_list = module_invoke($module, 'i18n_string_list', 'all')) {
    foreach ($string_list as $group => $group_strings) {
      if (!in_array($group, $groups)) {
        $strings[$group] = $group_strings;
      }
    }
  }
  // If the module defines any object that has strings in another textgroup
  if ($object_types = module_invoke($module, 'i18n_object_info')) {
    foreach ($object_types as $type => $type_info) {
      if (($group = i18n_string_object_info($type, 'textgroup')) && !in_array($group, $groups)) {
        if ($group_strings = i18n_string_object_type_string_list($type)) {
          $strings = array_merge_recursive($strings, $group_strings);
        }
      }
    }
  }
  return $strings;
}