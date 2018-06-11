<?php

namespace Drupal\islandora_document\Form;

use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Entity\EntityStorageInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Upload form when ingesting document objects.
 */
class DocumentUpload extends FormBase {
  protected $fileEntityStorage;

  /**
   * Constructor.
   */
  public function __construct(EntityStorageInterface $file_entity_storage) {
    $this->fileEntityStorage = $file_entity_storage;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('entity_type.manager')->getStorage('file')
    );
  }

  /**
   * Defines a file upload form for uploading documents.
   *
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'islandora_document_upload_form';
  }

  /**
   * Submit handler, adds uploaded file to ingest object.
   *
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
    $form_state->loadInclude('islandora_jodconverter', 'inc', 'includes/utilities');
    $pdf = islandora_jodconverter_get_format('pdf');
    $pdf['from'][] = 'pdf';
    $extensions = [implode(' ', $pdf['from'])];
    $form = [];
    $form['file'] = [
      '#title' => $this->t('Document File'),
      '#type' => 'managed_file',
      '#required' => TRUE,
      '#description' => $this->t('Select file to upload.<br/>Files must be less than <strong>@size MB.</strong><br/>Allowed file types: <strong>@ext.</strong>', ['@size' => $upload_size, '@ext' => $extensions[0]]),
      '#default_value' => $form_state->getValue('files'),
      '#upload_location' => 'temporary://',
      '#upload_validators' => [
        'file_validate_extensions' => $extensions,
        // Assume it's specified in MB.
        'file_validate_size' => [$upload_size * 1024 * 1024],
      ],
    ];
    if ($this->config('islandora_document.settings')->get('islandora_document_allow_text_upload')) {
      $form['islandora_document_text_upload'] = [
        '#type' => 'checkbox',
        '#title' => $this->t("Add text file to this upload?"),
      ];
      // Wrapper work-around for a Drupal bug affecting visible state.
      $form['text_section'] = [
        '#type' => 'item',
        '#states' => [
          'visible' => ['#edit-islandora-document-text-upload' => ['checked' => TRUE]],
        ],
      ];
      $form['text_section']['text'] = [
        '#title' => $this->t('Document text'),
        '#type' => 'managed_file',
        '#required' => FALSE,
        '#description' => $this->t('Select text file to upload.<br/>Files must be less than <strong>@size MB.</strong><br/>Allowed file types: <strong>@ext.</strong><br />This file is optional.', ['@size' => $upload_size, '@ext' => 'txt']),
        '#default_value' => $form_state->getValue('files'),
        '#upload_location' => 'temporary://',
        '#upload_validators' => [
          'file_validate_extensions' => ['txt'],
          // Assume it's specified in MB.
          'file_validate_size' => [$upload_size * 1024 * 1024],
        ],
      ];
    }
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $object = islandora_ingest_form_get_object($form_state);
    if ($form_state->getValue('file')) {
      if (empty($object['OBJ'])) {
        $ds = $object->constructDatastream('OBJ', 'M');
        $object->ingestDatastream($ds);
      }
      else {
        $ds = $object['OBJ'];
      }
      $file = $this->fileEntityStorage->load(reset($form_state->getValue('file')));
      $ds->setContentFromFile($file->getFileUri(), FALSE);
      $ds->label = $file->getFilename();
      $ds->mimetype = $file->getMimeType();
    }
    if ($form_state->getValue('text') && $form_state->getValue('text') > 0) {
      if (empty($object['FULL_TEXT'])) {
        $ds = $object->constructDatastream('FULL_TEXT', 'M');
        $object->ingestDatastream($ds);
      }
      else {
        $ds = $object['FULL_TEXT'];
      }
      $text_file = $this->fileEntityStorage->load(reset($form_state->getValue('text')));
      $ds->setContentFromFile($text_file->getFileUri(), FALSE);
      $ds->label = $text_file->getFilename();
      $ds->mimetype = $text_file->getMimeType();
    }
  }

}
