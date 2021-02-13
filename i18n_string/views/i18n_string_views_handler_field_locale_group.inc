<?php

/**
 * @file
 * Definition of i18n_string_views_handler_field_locale_group.
 */

/**
 * Field handler to translate a group into its readable form.
 *
 * @ingroup views_field_handlers
 */
class i18n_string_views_handler_field_locale_group extends views_handler_field {
  function render($values) {
    $groups = i18n_string_textgroup_options();
    // Sort the list.
    asort($groups);
    $value = $this->get_value($values);
    return isset($groups[$value]) ? $groups[$value] : '';
  }
}
