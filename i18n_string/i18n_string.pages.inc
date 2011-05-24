<?php

/**
 * @file
 * Internationalization (i18n) package - translatable strings reusable admin UI.
 *
 * @author Jose A. Reyero, 2007
 */

/**
 * Generic translation interface for i18n_strings objects.
 */
function i18n_string_translate_page($form_meta, $langcode = NULL) {
  $form_meta += array(
    '#item_title_header' => t('Title'),
  );

  if (empty($langcode)) {
    drupal_set_title($form_meta['#page_title']);
    return i18n_string_translate_page_overview($form_meta);
  }
  else {
    $languages = language_list();
    drupal_set_title(t('Translate to @language', array('@language' => $languages[$langcode]->name)));
    return drupal_get_form('i18n_string_translate_page_form', $form_meta, $langcode);
  }
}

/**
 * Provide a core translation module like overview page for this object.
 */
function i18n_string_translate_page_overview($form_meta) {
  include_once DRUPAL_ROOT . '/includes/language.inc';

  $header = array(t('Language'), $form_meta['#item_title_header'], t('Status'), t('Operations'));
  $default_language = language_default();
  $rows = array();

  foreach (language_list() as $langcode => $language) {
    if ($langcode == $default_language->language) {
      $rows[] = array(
        $language->name . ' ' . t('(source)'),
        $form_meta['#item_title_default'],
        t('original'),
        l(t('edit'), $form_meta['#edit']),
      );
    }
    else {
      // Try to figure out if this item has any of its properties translated.
      $translated = FALSE;
      foreach ($form_meta['#items'] as $item) {
        $str = i18n_string($item['#string_key'], $item['#default_value'], array('langcode' => $langcode, 'sanitize' => FALSE));
        if ($str != $item['#default_value']) {
          $translated = TRUE;
          break;
        }
      }
      // Translate the item that was requested to be displayed as title.
      $item_title = i18n_string_translate($form_meta['#item_title_key'], $form_meta['#item_title_default'], array('langcode' => $langcode));
      $rows[] = array(
        $language->name,
        $item_title,
        $translated ? t('translated') : t('not translated'),
        l(t('translate'), $form_meta['#translate'] . '/' . $langcode),
      );
    }
  }

  $build['i18n_string_translation_overview'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
  );

  return $build;
}

/**
 * Form builder callback for in-place string translation.
 */
function i18n_string_translate_page_form($form, &$form_state, $form_meta, $langcode) {
  $formats = filter_formats();
  $form['langcode'] = array(
    '#type' => 'value',
    '#value' => $langcode,
  );
  $form['strings'] = array(
    // Use a tree, so we can access the values easily.
    '#tree' => TRUE,
  );
  foreach ($form_meta['#items'] as $item) {
    $disabled = FALSE;
    $description = '';
    list($textgroup, $context) = i18n_string_context($item['#string_key']);
    $source = db_select('i18n_string', 'i18ns')
      ->fields('i18ns')
      ->condition('textgroup', $textgroup)
      ->condition('context', implode(':', $context))
      ->execute()
      ->fetchObject();
    if ($source) {
      if ($source->format) {
        $format = $formats[$source->format];
        $disabled = !filter_access($format);
        if ($disabled) {
          $description = t('This string uses the %name text format. You are not allowed to translate or edit texts with this format.', array('%name' => $format->name));
        }
        else {
          $description = '<h5>' . t('Text format: @name', array('@name' => $format->name)) . '</h5>' . theme('filter_tips', array('tips' => _filter_tips($source->format, FALSE)));
        }
      }
    }
    $default_value = i18n_string($item['#string_key'], $item['#default_value'], array('langcode' => $langcode, 'sanitize' => FALSE));
    $form['strings'][implode(':', $item['#string_key'])] = array(
      '#title' => $item['#title'],
      '#type' => 'textarea',
      '#default_value' => $default_value,
      '#disabled' => $disabled,
      '#description' => $description,
      '#i18n_string_format' => !empty($source) ? $source->format : 0,
      '#rows' => min(ceil(str_word_count($default_value) / 12), 10),
    );
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save translation'),
  );
  $form['#validate'] = array(
    'i18n_string_translate_page_form_validate'
  );
  $form['#redirect'] = array(
    $form_meta['#translate'],
  );
  return $form;
}

/**
 * Validation submission callback for in-place string translation.
 */
function i18n_string_translate_page_form_validate($form, &$form_state) {
  foreach ($form_state['values']['strings'] as $key => $value) {
    // We don't need to validate disabled form fields because those are already
    // validated by the FormAPI.
    if (empty($form['strings'][$key]['#i18n_string_format'])) {
      i18n_string_validate_submission("strings][$key", $value);
    }
  }
}

/**
 * Form submission callback for in-place string translation.
 */
function i18n_string_translate_page_form_submit($form, &$form_state) {
  foreach ($form_state['values']['strings'] as $key => $value) {
    list($textgroup, $context) = i18n_string_context(explode(':', $key));
    i18n_string_textgroup($textgroup)->update_translation($context, $form_state['values']['langcode'], $value);
  }
  drupal_set_message(t('Translations saved.'));
  $form_state['redirect'] = $form['#redirect'];
}

/**
 * String submission validation callback.
 */
function i18n_string_validate_submission($formkey, $value) {
  // Validation based on locale_translate_edit_form_validate.
  if (!locale_string_is_safe($value)) {
    form_set_error($formkey, t('The submitted string contains disallowed HTML: %string', array('%string' => $value)));
    watchdog('locale', 'Attempted submission of a translation string with disallowed HTML: %string', array('%string' => $value), WATCHDOG_WARNING);
  }
}
