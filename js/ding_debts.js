(function($) {
  Drupal.behaviors.dingDebtTableHeaderFix = {
    attach: function(context, settings) {
      // Changing the ids of the header elements, of the sticky-header tables, in the debt_page to avoid duplicates.
      var mainElement = '.sticky-header';
      var i = setInterval(function () {
        // Check if sticky headers are inserted.
        if ($(mainElement).length) {
          // Release setInterval after it has entered once.
          clearInterval(i);
          $(mainElement).each(function() {
            $(this).find('th').each(function() {
              $(this).attr('id', $(this).attr('id') + '-sticky');
            });
          });
        }
      }, 100);
    }
  };
})(jQuery);
