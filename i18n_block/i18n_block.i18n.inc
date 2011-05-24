<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */

/**
 * Implements hook_i18n_string_info().
 */
function i18n_block_i18n_string_info() {
  $groups['blocks'] = array(
    'title' => t('Blocks'),
    'description' => t('Configurable blocks titles and content.'),
    'format' => TRUE, // This group has strings with format (block body)
    'list' => TRUE, // This group can list all strings
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
