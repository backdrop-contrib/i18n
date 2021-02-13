<?php

/**
 * @file
 * Definition of i18n_string_views_handler_filter_locale_group.
 */

/**
 * Filter by locale text group.
 *
 * @ingroup views_filter_handlers
 */
class i18n_string_views_handler_filter_locale_group extends views_handler_filter_in_operator {
  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = t('Group');
      $groups = i18n_string_textgroup_options();
      // Sort the list.
      asort($groups);
      $this->value_options = $groups;
    }
  }
}
