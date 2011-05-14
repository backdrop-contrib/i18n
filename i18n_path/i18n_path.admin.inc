<?php
/**
 * @file
 * Administration pages for path translation.
 */

/**
 * Path overview page
 */
function i18n_path_admin_overview($type = NULL) {
  $type = 'path';
  $info = i18n_object_info($type);
  $language_list = language_list();

  // Build the sortable table header.
  $header['title'] = array('data' => t('Title'), 'field' => 't.title');
  $header['paths'] = t('Paths');
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
    $paths = array();
    foreach ($set->get_translations() as $language => $path) {
      if (!empty($path->path)) {
        $paths[$language] = $language_list[$language]->name . ': ' . l($path->path, $path->path, array('language' => $language_list[$language]));
      }
    }
    $options[$set->tsid] = array(
      'title' => check_plain($set->get_title()),
      'path' => theme('item_list', array('items' => $paths)),
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
 * Path add/edit form
 */
function i18n_path_admin_form($form, $form_state, $translation_set = NULL) {
  $form['translation_set'] = array('#type' => 'value', '#value' => $translation_set);
  if ($translation_set) {
    $paths = $translation_set->get_translations();
  }
  else {
    $paths = array();
  }
  $form['title'] = array(
    '#title' => t('Title'),
    '#type' => 'textfield',
    '#default_value' => $translation_set ? $translation_set->title : '',
    '#description' => t('Optional descriptive name for this set.'),
  );
  $form['translations'] = array(
    '#type' => 'fieldset',
    '#title' => t('Translations'),
    '#tree' => TRUE,
    '#description' => t('The path for this menu link. This can be an internal Drupal path such as %add-node or an external URL such as %drupal. Enter %front to link to the front page.', array('%front' => '<front>', '%add-node' => 'node/add', '%drupal' => 'http://drupal.org')),
  );
  foreach (i18n_language_list() as $langcode => $name) {
    $form['translations'][$langcode] = array(
      '#type' => 'textfield',
      '#title' => check_plain($name),
      '#default_value' => !empty($paths[$langcode]) ? $paths[$langcode]->path : '',
    );
  }

  $form['controls']['update'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#name' => 'save',
  );

  if ($translation_set) {
    $form['controls']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#name' => 'delete',
    );
  }
  return $form;
}

/**
 * Process form validation
 */
function i18n_path_admin_form_validate($form, &$form_state)  {
  if ($form_state['triggering_element']['#name'] == 'save') {
    $paths = &$form_state['values']['translations'];
    if ($paths = array_filter($paths)) {
      module_load_include('inc', 'menu', 'menu.admin');
      foreach ($paths as $language => &$link_path) {
        $link_path = i18n_prepare_normal_path($link_path, $language);
        $validation_form_state = array(
          'values' => array(
            'link_path' => $link_path,
          ),
        );
        menu_edit_item_validate(array(), $validation_form_state);
      }
    }
    else {
      form_set_error('paths', t('There are no path translations to save.'));
    }
  }
}

/**
 * Process form submission
 */
function i18n_path_admin_form_submit($form, &$form_state) {
  $translation_set = $form_state['values']['translation_set'];
  switch ($form_state['triggering_element']['#name']) {
    case 'save':
      $paths = array_filter($form_state['values']['translations']);
      $translation_set = $translation_set ? $translation_set : i18n_translation_set_create('path');
      $translation_set->title = '';
      $translations = $translation_set->get_translations();
      foreach ($paths as $lang => $path) {
        if (isset($translations[$lang])) {
          $translations[$lang]->path = $translation_set->translations[$lang]->path = $path;
        }
        else {
          $translation_set->translations[$lang] = $path;
        }
      }

      foreach (array_diff(array_keys($translation_set->translations), array_keys($paths)) as $language) {
        unset($translation_set->translations[$language]);
        unset($translations[$language]);
      }

      if (!empty($form_state['values']['title'])) {
        $translation_set->title = $form_state['values']['title'];
      }

      $translation_set->save(TRUE);
      drupal_set_message(t('The path translation has been saved.'));
      break;
    case 'delete':
      $destination = array();
      if (isset($_GET['destination'])) {
        $destination = drupal_get_destination();
        unset($_GET['destination']);
      }
      $form_state['redirect'] = array($translation_set->get_delete_path(), array('query' => $destination));
      return;
  }
  $form_state['redirect'] = 'admin/config/regional/i18n_translation/path';
}

/**
 * Save path translation set.
 */
function i18n_path_save_translations($paths, $tpid = NULL) {
  $paths = array_filter($paths);
  if (lock_acquire('i18n_path')) {
    if ($tpid) {
      db_delete('i18n_path')->condition('tpid', $tpid)->execute();
    }
    else {
      $tpid = 1 + (int)db_query('SELECT MAX(tpid) FROM {i18n_path}')->fetchField();
    }
    foreach ($paths as $langcode => $path) {
      db_insert('i18n_path')
        ->fields(array('tpid' => $tpid, 'language' => $langcode, 'path' => $path))
        ->execute();
    }
    lock_release('i18n_path');
    return $tpid;
  }
}

/**
 * Ask for confirmation of translation set deletion
 */
function i18n_path_delete_translation_confirm($form, &$form_state, $translation_set) {
  $form['#translation_set'] = $translation_set;
  $form['tsid'] = array(
    '#type' => 'value',
    '#value' => $translation_set->tsid,
  );
  if (empty($translation_set->title)) {
    $translation_set->title = t('Unknown');
  }
  return confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => $translation_set->title)),
    'admin/config/regional/i18n_translation/path',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Execute translation set deletion
 */
function i18n_path_delete_translation_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $translation_set = i18n_path_translation_set_load($form_state['values']['tsid']);
    $translation_set->delete(TRUE);
    drupal_set_message(t('The path translation has been deleted.'));
  }

  $form_state['redirect'] = 'admin/config/regional/i18n_translation/path';
}
