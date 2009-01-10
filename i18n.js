// $Id$

/**
 * Rewrite inputs to pass the language of the node currently being edited in the
 * path.
 *
 * This functionality ensures for example that node autocompletes get suggestions
 * for the node's language rather than the current interface language.
 */
Drupal.behaviors.i18n = function (context) {
  if (Drupal.settings && Drupal.settings.i18n) {
    $('form[id^=node-form]', context).find('input[value^=' + Drupal.settings.i18n.interface + ']').each(function () {
      $(this).val($(this).val().replace(Drupal.settings.i18n.interface, Drupal.settings.i18n.content));
    });
  }
};

