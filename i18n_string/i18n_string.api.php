<?php

/**
 * @file
 * API documentation file for String translation module.
 */

/**
 * List text groups for string translation.
 * 
 * This information will be automatically produced later for hook_locale()
 */
function hook_i18n_string_info() {
  $groups['menu'] = array(
    'title' => t('Menu'),
    'description' => t('Translatable menu items: title and description.'),
    'format' => FALSE, // This group doesn't have strings with format
    'list' => TRUE, // This group can list all strings
  );
  return $groups;
}

/**
 * Provide list of translatable strings for text group.

 * A module can provide a list of translatable strings using hook_i18n_string_list() or
 * it can provide a list of objects using hook_i18n_string_objects() from which the string
 * list will be produced automatically.
 * 
 * @param $group
 *   Text group name.
 */
function hook_i18n_string_list($group) {
  if ($group == 'mygroup') {
    $strings['mygroup']['type1']['key1']['name'] = 'Name of object type1/key1';
    $strings['mygroup']['type1']['key1']['description'] = 'Description of object type1/key1';
    return $strings;
  }
}

/**
 * Alter string list for objects of text group.
 *
 * @see i18n_menu_i18n_string_list_menu_alter()
 * 
 * @param $strings
 *   Associative array with current string list indexed by textgroup, type, id, name
 * @param $type
 *   Object type ad defined on i18n_object_info()
 * @param $object
 *   Object defined on i18n_object_info()
 */
function hook_i18n_string_list_TEXTGROUP_alter(&$strings, $type = NULL, $object = NULL) {
  if ($type == 'menu_link' && $object) {
    if (isset($object['options']['attributes']['title'])) {
    	$strings['menu']['item'][$object['mlid']]['title']['string'] = $object['link_title']; 
      $strings['menu']['item'][$object['mlid']]['description']['string'] = $object['options']['attributes']['title'];
    }  
  }
}

/**
 * List objects to collect translatable strings.
 * 
 * A module can provide a list of translatable strings using hook_i18n_string_list() or
 * it can provide a list of objects using hook_i18n_string_objects() from which the string
 * list will be produced automatically.
 * 
 * @see i18n_object_info()
 * @see i18n_menu_i18n_string_objects()
 * @see i18n_string_i18n_string_list()
 * 
 * @param $type string
 *   Object type
 * @return $objects array
 *   Associative array of objects indexed by object id
 */
function hook_i18n_string_objects($type) {
  if ($type == 'menu_link') {
    // All menu items that have no language and are customized.
    return db_select('menu_links', 'm')
      ->fields('m')
      ->condition('language', LANGUAGE_NONE)
      ->condition('customized', 1)
      ->execute()
      ->fetchAllAssoc('mlid', PDO::FETCH_ASSOC);
  }
}