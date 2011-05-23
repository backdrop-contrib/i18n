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
    'string translation' => array(
      'textgroup' => 'menu',
      'type' => 'menu',
      'properties' => array(
        'title' => t('Title'),
        'description' => t('Description'),
      ),
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
      $strings['menu']['menu'][$name] = array(
        'title' => $menu['title'],
        'description' => $menu['description']
      );
    }
    $query = db_select('menu_links', 'm')
      ->fields('m')
      ->condition('language', LANGUAGE_NONE)
      ->condition('customized', 1);
    foreach ($query->execute()->fetchAll() as $link) {
      $options = unserialize($link->options);
      $strings['menu']['item'][$link->mlid]['title'] = $link->link_title;
      if (isset($options['attributes']['title'])) {
        $strings['menu']['item'][$link->mlid]['description'] = $options['attributes']['title'];
      }
    }
    return $strings;
  }
}