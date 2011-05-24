<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */

/**
 * Implements hook i18n_object_info()
 */
function i18n_block_i18n_object_info() {
  $info['block'] = array(
    'title' => t('Block'),
    'key' => array('module', 'delta'),
    'placeholders' => array(
    	'%module' => 'module',
      '%delta' => 'delta',
    ),
    'edit path' => 'admin/structure/block/manage/%module/%delta/configure',
    'string translation' => array(
      'textgroup' => 'blocks',
      'properties' => array(
        'title' => array(
          'title' => t('Title'),
          'empty' => '<none>',
        ),
        'body' => array(
          'title' => t('Body'),
          'format' => 'format',
        ),
      ),
      'translate path' => 'admin/structure/block/manage/%module/%delta/translate/%language',
    )
  );
  return $info;
}

/**
 * Implements hook_i18n_string_info().
 */
function i18n_block_i18n_string_info() {
  $groups['blocks'] = array(
    'title' => t('Blocks'),
    'description' => t('Configurable blocks titles and content.'),
    'format' => TRUE, // This group has strings with format (block body)
    'list' => TRUE, // This group can list all strings
    'class' => 'i18n_block_textgroup',
  );
  return $groups;
}

/**
 * Implements hook_i18n_string_list().
 */
function i18n_block_i18n_string_list($group) {
  if ($group == 'blocks' || $group == 'all') {
    $strings = array();
    $query = db_select('block', 'b')
      ->distinct()
      ->fields('b', array('module', 'delta', 'title', 'i18n_mode'))
      ->fields('bc', array('body', 'format'))
      ->condition('i18n_mode', I18N_MODE_LOCALIZE);
    $query->leftJoin('block_custom', 'bc', 'b.bid = bc.bid');
    $result = $query->execute()
      ->fetchAll(PDO::FETCH_OBJ);
    foreach ($result as $block) {
      // If the block has a custom title and no language it must be translated
      if ($block->title && $block->title != '<none>') {
        $strings['blocks'][$block->module][$block->delta]['title'] = $block->title;
      }
      // If the block has body and no language, must be a custom block (box)
      if ($block->body) {
        $strings['blocks'][$block->module][$block->delta]['body'] = array(
          'string' => $block->body,
          'format' => $block->format
        );
      }
    }
    return $strings;
  }
}

/**
 * Implements hook_i18n_object_strings().
 */
function i18n_block_i18n_object_strings($type, $object) {
  if ($type == 'block' && $object->i18n_mode & I18N_MODE_LOCALIZE) {
    $strings = array();
    // If the block has a custom title and no language it must be translated
    if ($object->title && $object->title != '<none>') {
      $strings['blocks'][$object->module][$object->delta]['title'] = $object->title;
    }
    if (!empty($object->body)) {
      $strings['blocks'][$object->module][$object->delta]['body'] = array(
        'string' => $object->body,
        'format' => $object->format
      );     
    }
    elseif ($object->module == 'block' && !isset($object->body)) {
      $custom_block = (object) block_custom_block_get($object->delta);
      $strings['blocks'][$object->module][$object->delta]['body'] = array(
      	'string' => $custom_block->body, 
      	'format' => $custom_block->format
      );
    }
    return $strings;
  }
}