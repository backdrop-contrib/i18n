
(function ($) {

/**
 * Provide the summary information for the block settings vertical tab.
 */
Backdrop.behaviors.i18nSettingsSummary = {
  attach: function (context) {

    $('fieldset#edit-languages', context).backdropSetSummary(function (context) {
      var summary = '';
      if ($('.form-item-i18n-mode input[type=checkbox]:checked', context).val()) {
        summary += Backdrop.t('Translatable');
      }
      else {
        summary += Backdrop.t('Not translatable');
      }
      summary += ', ';
      if ($('.form-item-languages input[type=checkbox]:checked', context).val()) {
        summary += Backdrop.t('Restricted to certain languages');
      }
      else {
        summary += Backdrop.t('Not restricted');
      }
      return summary;
    });
  }
};

})(jQuery);
