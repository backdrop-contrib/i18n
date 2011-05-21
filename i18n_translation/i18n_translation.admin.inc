<?php

/**
 * @file
 * Internationalization (i18n) module. Translation sets admin
 */

/**
 * Overview page
 */
function i18n_translation_admin_overview($type) {
  $info = i18n_object_info($type);

  // Build the sortable table header.
  $header['title'] = array('data' => t('Title'), 'field' => 't.title');
  $header['items'] = t('Items');
  $header['operations'] = array('data' => t('Operations'));

  $query = db_select('i18n_translation_set', 't')->extend('PagerDefault')->extend('TableSort');
  $query->condition('t.type', $type);

  $tsids = $query
    ->fields('t', array('tsid'))
    ->limit(50)
    ->orderByHeader($header)
    ->execute()
    ->fetchCol();

  $translations = $tsids ? entity_load('i18n_translation', $tsids) : array();
  $destination = drupal_get_destination();
  $options = array();
  foreach ($translations as $set) {
    $items = array();
    foreach ($set->item_list() as $language => $item) {
      $items[$language] = i18n_language_name($language) . ': ' . $item;
    }
    $options[$set->tsid] = array(
      'title' => check_plain($set->get_title()),
      'path' => theme('item_list', array('items' => $items)),
      'operations' => '',
    );
    // Build a list of all the accessible operations for the current set.
    $operations = array();
    if ($path = $set->get_edit_path()) {
      $operations['edit'] = array(
        'title' => t('edit'),
        'href' => $path,
        'query' => $destination,
      );
    }
    if ($path = $set->get_delete_path()) {
      $operations['delete'] = array(
        'title' => t('delete'),
        'href' => $path,
        'query' => $destination,
      );
    }
    if (count($operations) > 1) {
      // Render an unordered list of operations links.
      $options[$set->tsid]['operations'] = array(
        'data' => array(
          '#theme' => 'links__node_operations',
          '#links' => $operations,
          '#attributes' => array('class' => array('links', 'inline')),
        ),
      );
    }
    elseif (!empty($operations)) {
      // Render the first and only operation as a link.
      $link = reset($operations);
      $options[$set->tsid]['operations'] = array(
        'data' => array(
          '#type' => 'link',
          '#title' => $link['title'],
          '#href' => $link['href'],
          '#options' => array('query' => $link['query']),
        ),
      );
    }
  }

  $form = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $options,
    '#empty' => t('No path translations set available.')
  );

  return $form;  
}

/**
 * Admin form
 */
function i18n_translation_admin_form($form, &$form_state, $type = NULL, $tsids = NULL) {
  // Build the sortable table header.
  $header['title'] = array('data' => t('Title'), 'field' => 't.title');
  if ($type) {
    $header['type'] = array('data' => t('Type'), 'field' => 't.type');
  }
  $header['created'] = array('data' => t('Created'), 'field' => 't.created');
  $header['changed'] = array('data' => t('Updated'), 'field' => 't.changed', 'sort' => 'desc');
  $header['operations'] = array('data' => t('Operations'));

  $query = db_select('i18n_translation_set', 't')->extend('PagerDefault')->extend('TableSort');
  if ($type) {
    $query->condition('t.type', $type);
  }
  if ($tsids) {
    $query->condition('t.tsid', $tsids);
  }
  $tsids = $query
    ->fields('t', array('tsid'))
    ->limit(50)
    ->orderByHeader($header)
    ->execute()
    ->fetchCol();
  $translations = $tsids ? entity_load('i18n_translation', $tsids) : array();
  $destination = drupal_get_destination();
  $rows = array();
  foreach ($translations as $set) {
    $info = i18n_object_info($set->type);
    $rows[$set->tsid] = array(
      'title' => check_plain($set->get_title()),
      'type' => isset($info['title']) ? $info['title'] : t('Unknown'),
      'created' => format_date($set->created, 'short'),
      'changed' => format_date($set->changed, 'short'),
      'operations' => '',
    );
    if (!$type) {
      unset($rows[$set->tsid]['type']);
    }
    // Build a list of all the accessible operations for the current set.
    $operations = array();
    if ($path = $set->get_edit_path()) {
      $operations['edit'] = array(
        'title' => t('edit'),
        'href' => $path,
        'query' => $destination,
      );
    }
    if (count($operations) > 1) {
      // Render an unordered list of operations links.
      $rows[$set->tsid]['operations'] = array(
        'data' => array(
          '#theme' => 'links__node_operations',
          '#links' => $operations,
          '#attributes' => array('class' => array('links', 'inline')),
        ),
      );
    }
    elseif (!empty($operations)) {
      // Render the first and only operation as a link.
      $link = reset($operations);
      $rows[$set->tsid]['operations'] = array(
        'data' => array(
          '#type' => 'link',
          '#title' => $link['title'],
          '#href' => $link['href'],
          '#options' => array('query' => $link['query']),
        ),
      );
    }
  }
  $form['translations'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No translation sets available.'),
  );
  return $form;
}