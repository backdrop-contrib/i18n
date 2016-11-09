
(function ($) {

/**
 * Rewrite autocomplete inputs to pass the language of the node currently being
 * edited in the path.
 *
 * This functionality ensures node autocompletes get suggestions for the node's
 * language rather than the current interface language.
 */
Backdrop.behaviors.i18n = {
  attach: function (context) {
    if (Backdrop.settings && Backdrop.settings.i18n) {
      $('form[id^=node-form]', context).find('input.autocomplete[value^=' + Backdrop.settings.i18n.interface_path + ']').each(function () {
        $(this).val($(this).val().replace(Backdrop.settings.i18n.interface_path, Backdrop.settings.i18n.content_path));
      });
    }
  }
};

})(jQuery);