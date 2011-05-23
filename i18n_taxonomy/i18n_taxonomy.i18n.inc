<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */

/**
 * Implements hook i18n_object_info()
 */
function i18n_taxonomy_i18n_object_info() {
  $info['taxonomy_term'] = array(
    'title' => t('Taxonomy term'),
    'entity' => 'taxonomy_term',
    'key' => 'tid',
    'translation set' => array(
      'class' => 'i18n_taxonomy_translation_set',
      'table' => 'taxonomy_term_data',
      'field' => 'i18n_tsid',
      'parent' => 'taxonomy_vocabulary',
      'placeholder' => '%i18n_taxonomy_translation_set',
      'list path' => 'admin/structure/taxonomy/%taxonomy_vocabulary_machine_name/list/sets',
      'edit path' => 'admin/structure/taxonomy/%taxonomy_vocabulary_machine_name/list/sets/edit/%i18n_taxonomy_translation_set',
      'delete path' => 'admin/structure/taxonomy/%taxonomy_vocabulary_machine_name/list/sets/delete/%i18n_taxonomy_translation_set',
    ),
    'string translation' => array(
      'textgroup' => 'taxonomy',
      'type' => 'term',
      'properties' => array(
        'name' => t('Name'),
        'description' => array(
          'title' => t('Description'),
          'format' => 'format',
        ),
      ),
    )
  );
  $info['taxonomy_vocabulary'] = array(
    'title' => t('Vocabulary'),
    'entity' => 'taxonomy_vocabulary',
    'key' => 'vid',
    'string translation' => array(
      'textgroup' => 'taxonomy',
      'type' => 'vocabulary',
      'properties' => array(
        'name' => t('Name'),
        'description' => t('Description'),
      ),
    ),
    'translation container' => array(
      'name' => t('vocabulary'),
      'item type' => 'taxonomy_term',
      'item name' => t('terms'),
      'options' => array(I18N_MODE_NONE, I18N_MODE_LOCALIZE, I18N_MODE_TRANSLATE, I18N_MODE_LANGUAGE),
    ),
  );
  return $info;
}

/**
 * Implements hook_i18n_string_info()
 */
function i18n_taxonomy_i18n_string_info() {
  $groups['taxonomy'] = array(
    'title' => t('Taxonomy'),
    'description' => t('Vocabulary titles and term names for localizable vocabularies.'),
    'format' => FALSE, // This group doesn't have strings with format
    'list' => FALSE, // This group cannot list all strings
    'refresh callback' => 'i18n_taxonomy_i18n_string_refresh',
    'class' => 'i18n_taxonomy_textgroup',
  );
  return $groups;
}

/**
 * Implements hook_i18n_string_refresh()
 */
function i18n_taxonomy_i18n_string_refresh($group) {
  if ($group == 'taxonomy' || $group == 'all') {
    foreach (taxonomy_get_vocabularies() as $vid => $vocabulary) {
      if (empty($vocabulary->language)) {
        i18n_string_object_update('taxonomy_vocabulary', $vocabulary);
      }
      if (i18n_taxonomy_vocabulary_mode($vid) & I18N_MODE_LOCALIZE) {
        foreach (taxonomy_get_tree($vid, 0) as $term) {
          i18n_string_object_update('taxonomy_term', $term);
        }
      }
    }
    return TRUE; // Meaning it completed with no issues
  }
}
