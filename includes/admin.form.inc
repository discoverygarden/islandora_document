<?php

/**
 * @file
 * This file contains all admin functions.
 */

use Drupal\Core\Form\FormStateInterface;

/**
 * Admin form validate function.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param Drupal\Core\Form\FormStateInterface $form_state
 *   The Drupal form state.
 */
function islandora_document_admin_validate(array $form, FormStateInterface &$form_state) {
  if (NULL !== $form_state->getValue('islandora_document_create_fulltext')) {
    $islandora_document_path_to_pdftotext = $form_state->getValue('islandora_document_path_to_pdftotext');
    exec($islandora_document_path_to_pdftotext, $output, $return_value);
    if ($return_value != 99) {
      form_set_error('', $this->t('Cannot extract text from PDF without a valid path to pdftotext.'));
    }
  }
}

/**
 * Function updates PDF to text URL div.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param Drupal\Core\Form\FormStateInterface $form_state
 *   The Drupal form state.
 */
function islandora_document_update_pdftotext_url_div(array $form, FormStateInterface $form_state) {
  unset($form_state['submit_handlers']);
  $form_state['rebuild'] = TRUE;

  return $form['islandora_document_url_fieldset']['wrapper'];
}
