<?php

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'title' => t('User debt list'),
  'description' => t('The current debts for a user'),
  'single' => TRUE,
  'content_types' => array('list'),
  'required context' => new ctools_context_required(t('User'), 'user'),
  'category' => t('User'),
);

/**
 * Render the block
 */
function ding_debt_debts_content_type_render($subtype, $conf, $panel_args, $context) {
  // Adding the js to fix the duplicate ids issue. 
  // The weight is assigned to 5 in order to run the script after tableheader.js installs the sticky tables.
  drupal_add_js(drupal_get_path('module', 'ding_debt') . '/js/tableheader_duplicate_id_fix.js', array('weight' => 5));
  $preload_debts = &drupal_static('ids');
  $account = isset($context->data) ? $context->data : NULL;
  $preload_ids = array();
  $items = array();
  $debt_list = array();
  $rows_penalty = array();
  $rows_compensation = array();
  $row = array();
  $penalties_amount = 0;
  $compensation_amount = 0;
  
  $block = new stdClass();
  $block->module = 'ding_debt';
  $block->delta  = 'debts';
  $block->title = t('My debts');

  $debts = ding_provider_invoke_page('debt', 'list', $account);
  if (count($debts) == TRUE) {
    $header_penalties = array(
      array('data' => t('Fee date'), 'id' => 'feedate-penalties', 'class' => 'feedate'),
      array('data' => t('Material name'), 'id' => 'material-penalties', 'class' => 'feetitle'),
      array('data' => t('Amount'), 'id' => 'amount-penalties', 'class' => 'feeamount'),
    );

    $header_compensation = array(
      array('data' => t('Fee date'), 'id' => 'feedate-compensation' , 'class' => 'feedate'),
      array('data' => t('Material name'), 'id' => 'material-compensation' , 'class' => 'feetitle'),
      array('data' => t('Amount'), 'id' => 'amount-compensation', 'class' => 'feeamount'),
    );

    foreach ($debts as $debt) {
      
      if ( !$debt->invoice_number ) {
        $penalties_amount += $debt->amount;
        $penalties_amount -= $debt->amount_paid;
        $row = array(
          array('data' => format_date(strtotime(check_plain($debt->date)), 'custom', 'd/m/Y'), 'class' => 'feedate'),
          array('data' => $debt->display_name, 'class' => 'feetitle'),
          array('data' => number_format(($debt->amount - $debt->amount_paid), 2, ',', ' '), 'class' => 'feeamount'),
        );
        $rows_penalty[] = $row;      
        
      } 
      else {
        $compensation_amount += $debt->amount;
        $compensation_amount -= $debt->amount_paid;
        $row = array(
          array('data' => format_date(strtotime(check_plain($debt->date)), 'custom', 'd/m/Y'), 'class' => 'feedate'),
          array('data' => $debt->display_name, 'class' => 'feetitle'),
          array('data' => number_format(($debt->amount - $debt->amount_paid), 2, ',', ' ') . ' <span class="note">*</span>' , 'class' => 'feeamount'),
        );
        $rows_compensation[] = $row;      
      }
    }
    // Add total for penalties.
    $rows_penalty[] = array(
      array('data' => t('Total'), 'class' => 'feedate', 'headers' => 'feedate-penalties'),
      array('data' => '', 'class' => 'feetitle', 'headers' => 'material-penalties'),
      array('data' => number_format($penalties_amount, 2, ',', ' '), 'class' => 'feeamount', 'headers' => 'amount-penalties'),
    );
    // Add total for compensation.
    $rows_compensation[] = array(
      array('data' => t('Total'), 'class' => 'feedate', 'headers' => 'feedate-compensation'),
      array('data' => '', 'class' => 'feetitle', 'headers' => 'material-compensation'),
      array('data' => number_format($compensation_amount, 2, ',', ' '), 'class' => 'feeamount', 'headers' => 'amount-compensation'),
    );

    if ($compensation_amount > 0) {
      // add note
      $row = array(
        array('data' => '', 'class' => 'fee-invoice-note', 'headers' => 'feedate-compensation'),
        array('data' => t("* Invoiced fees can only be paid on your local library."), 'class' => 'fee-invoice-note', 'headers' => 'material-compensation'),
        array('data' => '', 'class' => 'fee-invoice-note', 'headers' => 'amount-compensation'),
      );
      $rows_compensation[] = $row;
    }
    
    $table_penalties = theme('table', array(
      'header' => $header_penalties,
      'rows' => $rows_penalty,
    ));
    
    $table_compensation = theme('table', array(
      'header' => $header_compensation,
      'rows' => $rows_compensation,
    ));
    
    // Criterion: only display "Pay" button on if there is penalty related with penalties.
    if ($penalties_amount > 0) {
      $build = ding_provider_get_form('ding_debt_debts_form', $debts);
    }

    $block->content = render($table_penalties) . render($build) . render($table_compensation);

  }
  else {
    $block->content = t('No debts');
  }
  return $block;
}

/**
 * Form for configuring the content type.
 *
 * Not currently used.
 */
function ding_debt_debts_content_type_edit_form($form, &$form_state) {
  return $form;
}

/**
 * Implements a drupal form, which implements a pay button and debts data
 */
function ding_debt_debts_form($form, &$form_state, $debts = array()) {

  $form['debt_data'] = array(
    '#type' => 'value',
    '#value' => $debts,
  );

  $form['pay'] = array(
    '#type' => 'submit',
    '#prefix' => '<div class="pay-button">',
    '#value' => t('Pay debts'),
    '#suffix' => '</div>',
  );

  return $form;
}

/**
 * Submit handler for the form.
 */
function ding_debt_debts_form_submit($form, &$form_state) {
  global $user;
  $amount = 0;
  $debts = array_filter($form_state['values']['debt_data']);
  foreach ($debts as $debt_id => $debt_data) {
    if ( !$form_state['values']['debt_data'][$debt_id]->invoice_number ) {
      $amount += $form_state['values']['debt_data'][$debt_id]->amount;
      $amount -= $form_state['values']['debt_data'][$debt_id]->amount_paid;
    } else {
      unset($debts[$debt_id]);
    }
  }

  if ($amount > 0) {
    $params = array('params' => array('debt_ids' => array_keys($debts),'debts'=>$debts));
    $url = ding_provider_invoke('payment', 'get_url', $amount, $params, 'ding_debt_payment_callback');
    if ($url) {
      $form_state['redirect'] = $url;
    }
  }
}

