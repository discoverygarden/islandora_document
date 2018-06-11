<?php

/**
 * @file
 * This file contains all admin functions.
 */

use Drupal\Core\Form\FormStateInterface;

/**
 * Function updates PDF to text URL div.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param Drupal\Core\Form\FormStateInterface $form_state
 *   The Drupal form state.
 */
function islandora_document_update_pdftotext_url_div(array $form, FormStateInterface $form_state) {
  return $form['islandora_document_url_fieldset']['wrapper'];
}
