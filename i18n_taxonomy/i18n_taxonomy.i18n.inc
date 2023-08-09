<?php
/**
 * @file
 * Internationalization (i18n) hooks.
 */

/**
 * Implements hook_i18n_object_info().
 */
function i18n_taxonomy_i18n_object_info() {
  $info['taxonomy_term'] = array(
    'title' => t('Taxonomy term'),
    'class' => 'i18n_taxonomy_term',
    'entity' => 'taxonomy_term',
    'key' => 'tid',
    'placeholders' => array(
      '%taxonomy_term' => 'tid',
    ),
    // Auto generate edit path.
    'edit path' => 'taxonomy/term/%taxonomy_term/edit',
    // Auto-generate translate tab.
    'translate tab' => 'taxonomy/term/%taxonomy_term/translate',
    'translation set' => TRUE,
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
    ),
  );
  $info['taxonomy_vocabulary'] = array(
    'title' => t('Vocabulary'),
    'entity' => 'taxonomy_vocabulary',
    'key' => 'machine_name',
    'placeholders' => array(
      '%taxonomy_vocabulary_machine_name' => 'machine_name',
    ),
    // Auto generate edit path.
    'edit path' => 'admin/structure/taxonomy/%taxonomy_vocabulary_machine_name/edit',
    // Auto-generate translate tab.
    'translate tab' => 'admin/structure/taxonomy/%taxonomy_vocabulary_machine_name/translate',
    // We can easily list all these objects.
    'list callback' => 'taxonomy_get_vocabularies',
    // Metadata for string translation.
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
      'options' => array(
        I18N_MODE_NONE,
        I18N_MODE_LOCALIZE,
        I18N_MODE_TRANSLATE,
        I18N_MODE_LANGUAGE,
      ),
    ),
  );
  return $info;
}

/**
 * Implements hook_i18n_translation_set_info().
 */
function i18n_taxonomy_i18n_translation_set_info() {
  $info['taxonomy_term'] = array(
    'title' => t('Taxonomy term'),
    'class' => 'i18n_taxonomy_translation_set',
    'table' => 'taxonomy_term_data',
    'field' => 'i18n_tsid',
    'parent' => 'taxonomy_vocabulary',
    'placeholder' => '%i18n_taxonomy_translation_set',
    'list path' => 'admin/structure/taxonomy/%taxonomy_vocabulary_machine_name/list/sets',
    'edit path' => 'admin/structure/taxonomy/%taxonomy_vocabulary_machine_name/list/sets/edit/%i18n_taxonomy_translation_set',
    'delete path' => 'admin/structure/taxonomy/%taxonomy_vocabulary_machine_name/list/sets/delete/%i18n_taxonomy_translation_set',
    'page callback' => 'i18n_taxonomy_term_translation_page',
  );
  return $info;
}

/**
 * Implements hook_i18n_string_info().
 */
function i18n_taxonomy_i18n_string_info() {
  $groups['taxonomy'] = array(
    'title' => t('Taxonomy'),
    'description' => t('Vocabulary titles and term names for localizable vocabularies.'),
    // This group doesn't have strings with format.
    'format' => FALSE,
    // This group cannot list all strings.
    'list' => FALSE,
    'refresh callback' => 'i18n_taxonomy_i18n_string_refresh',
  );
  return $groups;
}

/**
 * Implements hook_i18n_string_objects().
 *
 * @param string $type
 *   The type to load, taxonomy_term.
 * @return array
 *   Array of taxonomy terms or empty array.
 */
function i18n_taxonomy_i18n_string_objects($type) {
  if ($type == 'taxonomy_term') {
    $terms = array();
    $config_names = config_get_names_with_prefix('taxonomy.vocabulary.');
    foreach ($config_names as $config_name) {
      $config = config($config_name);
      if ($config->get('i18n_mode') == I18N_MODE_LOCALIZE) {
        $vocab_name = $config->get('machine_name');
        foreach (taxonomy_get_tree($vocab_name) as $term) {
          $terms[] = $term;
        }
      }
    }
    return $terms;
  }
}

/**
 * Callback for term translation tab.
 *
 * @param string $type
 *   Should be always 'taxonomy_term'.
 * @param object $term
 *   Taxonomy term object.
 */
function i18n_taxonomy_term_translation_page($type, $term) {
  module_load_include('admin.inc', 'i18n_taxonomy');
  $vocabulary = taxonomy_vocabulary_load($term->vocabulary);
  $translation_set = !empty($term->i18n_tsid) ? i18n_translation_set_load($term->i18n_tsid) : NULL;

  $translation_overview = i18n_taxonomy_translation_term_overview($term);

  $translation_term_form = backdrop_get_form('i18n_taxonomy_translation_term_form', $vocabulary, $translation_set, $term);

  return $translation_overview + $translation_term_form;
}
