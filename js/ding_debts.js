 (function($) {
  Drupal.behaviors.dingDebtTableHeaderFix = {
    attach: function(context, settings) {
      // Changing the ids of the header elements, of the sticky-header tables, in the debt_page to avoid duplicates.
      var mainElement = '.sticky-header';
      $(mainElement).each(function() {
        $(this).find('th').each(function() {
          $(this).attr('id', $(this).attr('id') + '-sticky');
        });
      });
    }
  };   
})(jQuery);
