<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */

/**
 * Implements hook i18n_object_info()
 */
function i18n_menu_i18n_object_info() {
  $info['menu'] = array(
    'title' => t('Menu'),
    'key' => 'menu_name',
    'base path' => 'admin/structure/menu/manage',
    'placeholders' => array(
      '%menu' => 'menu_name',
    ),
    'edit path' => 'admin/structure/menu/manage/%menu/edit',
    // Auto-generate translate tab.
    'translate tab' => 'admin/structure/menu/manage/%menu/translate',
    'string translation' => array(
      'textgroup' => 'menu',
      'type' => 'menu',
      'properties' => array(
        'title' => t('Title'),
        'description' => t('Description'),
      ),
      'translate path' => 'admin/structure/menu/manage/%menu/translate/%language',
    ),
    'translation container' => array(
      'name' => t('menu'),
      'item type' => 'menu_link',
      'item name' => t('menu items'),
      'options' => array(I18N_MODE_NONE, I18N_MODE_MULTIPLE, I18N_MODE_LANGUAGE),
    ),
  );
  $info['menu_link'] = array(
    'title' => t('Menu link'),
    'key' => 'mlid',
    'base path' => 'admin/structure/menu/item',
    'edit path' => 'admin/structure/menu/item/%menu_link/edit',
    // Auto-generate translate tab
    'translate tab' => 'admin/structure/menu/item/%menu_link/translate',
    'placeholders' => array(
    	'%menu_link' => 'mlid',
      '%menu' => 'menu_name',
    ),
    'string translation' => array(
      'textgroup' => 'menu',
      'type' => 'item',
      'properties' => array(
        'title' => t('Title'),
        'description' => t('Description'),
      ),
      'translate path' => 'admin/structure/menu/item/%menu_link/translate/%language',
    ),
    'translation set' => array(
      'class' => 'i18n_menu_link_translation_set',
      'table' => 'menu_links',
      'field' => 'i18n_tsid',
      'parent' => 'menu',
      'placeholder' => '%i18n_menu_translation',
      'list path' => 'admin/structure/menu/manage/translation',
      'edit path' => 'admin/structure/menu/manage/translation/edit/%i18n_menu_translation',
      'delete path' => 'admin/structure/menu/manage/translation/delete/%i18n_menu_translation',
      'page callback' => 'i18n_menu_item_translation_page',
    ),
  );
  return $info;
}

/**
 * Implements hook_i18n_string_info()
 */
function i18n_menu_i18n_string_info() {
  $groups['menu'] = array(
    'title' => t('Menu'),
    'description' => t('Translatable menu items: title and description.'),
    'format' => FALSE, // This group doesn't have strings with format
    'list' => TRUE, // This group can list all strings
  );
  return $groups;
}

/**
 * Implements hook_i18n_string_list()
 */
function i18n_menu_i18n_string_list($group) {
  if ($group == 'menu' || $group == 'all') {
    $strings = array();
    // Add all menus
    foreach (menu_load_all() as $name => $menu) {
      $strings['menu']['menu'][$name] = i18n_string_object_strings('menu', $menu);
    }
    $query = db_select('menu_links', 'm')
      ->fields('m')
      ->condition('language', LANGUAGE_NONE)
      ->condition('customized', 1);
    foreach ($query->execute()->fetchAll() as $link) {
      $link->options = unserialize($link->options);
      $strings['menu']['item'][$link->mlid] = i18n_string_object_strings('menu_link', (array)$link);
    }
    return $strings;
  }
}

/**
 * Callback for menu item translation tab.
 */
function i18n_menu_item_translation_page($type, $item) {
  module_load_include('admin.inc', 'i18n_menu');
  // If the item has a language code, we can only support translation sets.
  $translation_set = !empty($item['i18n_tsid']) ? i18n_translation_set_load($item['i18n_tsid']) : NULL;
  $build['overview'] = i18n_menu_translation_item_overview($item, $translation_set);
  $build['translation_form'] = drupal_get_form('i18n_menu_translation_form', NULL, $item, $translation_set);
  return $build;
}

/**
 * Implements hook_i18n_string_list_TEXTGROUP_alter()
 */
function i18n_menu_i18n_string_list_menu_alter(&$strings, $type = NULL, $object = NULL) {
  if ($type == 'menu_link' && $object) {
    if (isset($object['options']['attributes']['title'])) {
    	$strings['menu']['item'][$object['mlid']]['title']['string'] = $object['link_title']; 
      $strings['menu']['item'][$object['mlid']]['description']['string'] = $object['options']['attributes']['title'];
    }  
  }
}
