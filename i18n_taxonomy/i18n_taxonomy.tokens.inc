<?php
/**
 * @file
 * Builds placeholder replacement tokens for taxonomy terms and vocabularies.
 */

/**
 * Implements hook_token_info().
 */
function i18n_taxonomy_token_info() {

  // Taxonomy term related variables.
  $term['i18n-name'] = array(
    'name' => t("Name (localized)"),
    'description' => t("The name of the taxonomy term."),
  );
  $term['i18n-description'] = array(
    'name' => t("Description (localized)"),
    'description' => t("The optional description of the taxonomy term."),
  );

  // Taxonomy vocabulary related variables.
  $vocabulary['i18n-name'] = array(
    'name' => t("Name (localized)"),
    'description' => t("The name of the taxonomy vocabulary."),
  );
  $vocabulary['i18n-description'] = array(
    'name' => t("Description (localized)"),
    'description' => t("The optional description of the taxonomy vocabulary."),
  );

  // Chained tokens for taxonomies.
  $term['i18n-vocabulary'] = array(
    'name' => t("Vocabulary (localized)"),
    'description' => t("The vocabulary the taxonomy term belongs to."),
    'type' => 'vocabulary',
  );
  $term['i18n-parent'] = array(
    'name' => t("Parent term (localized)"),
    'description' => t("The parent term of the taxonomy term, if one exists."),
    'type' => 'term',
  );

  return array(
    'tokens' => array(
      'term' => $term,
      'vocabulary' => $vocabulary,
    ),
  );
}

/**
 * Implements hook_tokens().
 */
function i18n_taxonomy_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();
  $sanitize = !empty($options['sanitize']);
  $langcode = isset($options['language']) ? $options['language']->langcode : i18n_langcode();

  if ($type == 'term' && !empty($data['term'])) {
    $term = $data['term'];

    foreach ($tokens as $name => $original) {
      switch ($name) {

        case 'i18n-name':
          $name = i18n_taxonomy_term_name($term, $langcode);
          $replacements[$original] = $sanitize ? check_plain($name) : $name;
          break;

        case 'i18n-description':
          $context = "taxonomy:term:{$term->tid}:description";
          $params = array(
            'langcode' => $langcode,
            'format' => $term->format,
            'sanitize' => $sanitize,
            'cache' => TRUE,
          );
          $replacements[$original] = i18n_string_text($context, $term->description, $params);
          break;

        case 'i18n-vocabulary':
          $vocabulary = taxonomy_vocabulary_load($term->vocabulary);
          $replacements[$original] = check_plain(i18n_taxonomy_vocabulary_name($vocabulary, $langcode));
          break;

        case 'i18n-parent':
          if ($parents = taxonomy_term_load_parents($term->tid)) {
            $parent = array_pop($parents);
            $replacements[$original] = check_plain(i18n_taxonomy_term_name($parent, $langcode));
          }
          break;
      }

      // [term:i18n-vocabulary:*] chained tokens.
      if ($vocabulary_tokens = token_find_with_prefix($tokens, 'i18n-vocabulary')) {
        $vocabulary = taxonomy_vocabulary_load($term->vocabulary);
        $replacements += token_generate('vocabulary', $vocabulary_tokens, array('vocabulary' => $vocabulary), $options);
      }

      // [term:i18n-parent:*] chained tokens.
      if (($vocabulary_tokens = token_find_with_prefix($tokens, 'i18n-parent')) && $parents = taxonomy_term_load_parents($term->tid)) {
        $parent = array_pop($parents);
        $replacements += token_generate('term', $vocabulary_tokens, array('term' => $parent), $options);
      }
    }
  }

  elseif ($type == 'vocabulary' && !empty($data['vocabulary'])) {
    $vocabulary = $data['vocabulary'];

    foreach ($tokens as $name => $original) {
      switch ($name) {

        case 'i18n-name':
          $name = i18n_taxonomy_vocabulary_name($vocabulary, $langcode);
          $replacements[$original] = $sanitize ? check_plain($name) : $name;
          break;

        case 'i18n-description':
          if (i18n_object_langcode($vocabulary)) {
            $description = $vocabulary->description;
          }
          else {
            $context = "taxonomy:vocabulary:{$vocabulary->machine_name}:description";
            $description = i18n_string($context, $vocabulary->description, array('langcode' => $langcode));
          }
          $replacements[$original] = $sanitize ? filter_xss($description) : $description;
          break;
      }
    }
  }

  return $replacements;
}
