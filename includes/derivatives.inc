<?php

/**
 * @file
 * Contains all the functions required for adding derivatives for the PDF SP.
 */

use Drupal\Core\Logger\RfcLogLevel;
use Drupal\imagemagick\ImagemagickExecArguments;

use Drupal\islandora\MimeDetect;

/**
 * Gets the source of all derivatives as an unmanaged file.
 *
 * @param AbstractObject $object
 *   The object to generate/add the derivative datastreams to.
 * @param string $dsid
 *   The datastream identifier.
 *
 * @return string
 *   The file path to the unmanaged derivative source file.
 */
function islandora_document_get_derivative_source_as_unmanaged_file(AbstractObject $object, $dsid = 'OBJ') {
  $mime_detect = new MimeDetect();
  $ext = $mime_detect->getExtension($object[$dsid]->mimeType);
  $file_name = str_replace(':', '-', $object->id);
  $file_uri = file_create_filename("{$file_name}_$dsid.{$ext}", 'temporary://');
  if (isset($object[$dsid]) && $object[$dsid]->size != -1) {
    $object[$dsid]->getContent($file_uri);
  }
  return $file_uri;
}

/**
 * Creates a JPG derivative for the TN datastream.
 *
 * @param AbstractObject $object
 *   The object to add the derivative datastreams to.
 * @param bool $force
 *   Whether the derivative generation is being forced or not.
 *
 * @return array
 *   An array describing the outcome of the thumbnail creation.
 *
 * @see hook_islandora_derivative()
 */
function islandora_document_add_tn_derivative(AbstractObject $object, $force = FALSE) {
  $width = \Drupal::config('islandora_document.settings')->get('islandora_document_thumbnail_width');
  $height = \Drupal::config('islandora_document.settings')->get('islandora_document_thumbnail_height');
  $file_uri = islandora_document_get_derivative_source_as_unmanaged_file($object, 'PDF');
  $results = islandora_document_add_jpg_derivative($object, $file_uri, 'TN', $width, $height, $force);
  file_unmanaged_delete($file_uri);
  return $results;
}

/**
 * Creates a textual derivative for the FULL_TEXT datastream.
 *
 * @param AbstractObject $object
 *   The object to add the derivative datastreams to.
 * @param bool $force
 *   Whether the derivative generation is being forced or not.
 *
 * @return array
 *   An array describing the outcome of the FULL_TEXT datastream creation.
 *
 * @see hook_islandora_derivative()
 */
function islandora_document_add_fulltext_derivative(AbstractObject $object, $force = FALSE) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  if (!isset($object['FULL_TEXT']) || (isset($object['FULL_TEXT']) && $force === TRUE)) {
    if (!isset($object['PDF'])) {
      return islandora_document_no_obj_datastream($object->id);
    }
    $file_uri = islandora_document_get_derivative_source_as_unmanaged_file($object, 'PDF');
    $dsid = "FULL_TEXT";
    $source = $source = \Drupal::service("file_system")->realpath($file_uri);
    $executable = \Drupal::config('islandora_document.settings')->get('islandora_document_path_to_pdftotext');
    $temp = file_create_filename('fulltext.txt', 'temporary://');
    $derivative_file_uri = \Drupal::service("file_system")->realpath($temp);
    $command = "$executable $source $derivative_file_uri";
    exec($command, $exec_out, $return_code);
    file_unmanaged_delete($file_uri);
    $success = ($return_code === 0);

    if (!$success) {
      $success = [
        'success' => FALSE,
        'messages' => [
          [
            'message' => t('`%command` returned an error when creating %dsid derivative of PID %pid: %error'),
            'message_sub' => [
              '%command' => $command,
              '%dsid' => $dsid,
              '%pid' => $object->id,
              '%error' => implode("\n", $exec_out),
            ],
            'type' => 'dsm',
            'severity' => RfcLogLevel::ERROR,
          ],
        ],
      ];
    }
    else {
      // Before we add this datastream let's rip out non valid XML characters.
      $full_text = file_get_contents($derivative_file_uri);
      $sanitized_full_text = islandora_sanitize_input_for_valid_xml($full_text);
      file_put_contents($derivative_file_uri, $sanitized_full_text);
      $success = islandora_document_add_datastream($object, $dsid, $derivative_file_uri);
      if ($success === TRUE) {
        return [
          'success' => TRUE,
          'messages' => [
            [
              'message' => t('Created FULL_TEXT derivative.'),
              'type' => 'dsm',
            ],
          ],
        ];
      }
      else {
        return [
          'success' => FALSE,
          'messages' => [
            [
              'message' => t('Failed to create FULL_TEXT derivative for @pid. Error message: @error'),
              'message_sub' => [
                '@pid' => $object->id,
                '@error' => $success,
              ],
              'type' => 'watchdog',
              'severity' => RfcLogLevel::ERROR,
            ],
          ],
        ];
      }
    }
    file_unmanaged_delete($temp);
    return $success;
  }
}

/**
 * Creates a JPG derivative for the PREVIEW datastream.
 *
 * @param AbstractObject $object
 *   The object to add the derivative datastreams to.
 * @param bool $force
 *   Whether the derivative generation is being forced or not.
 *
 * @return array
 *   An array describing the outcome of the PREVIEW datastream creation.
 *
 * @see hook_islandora_derivative()
 */
function islandora_document_add_preview_derivative(AbstractObject $object, $force = FALSE) {
  $width = \Drupal::config('islandora_document.settings')->get('islandora_document_preview_width');
  $height = \Drupal::config('islandora_document.settings')->get('islandora_document_preview_height');
  $file_uri = islandora_document_get_derivative_source_as_unmanaged_file($object, 'PDF');
  $results = islandora_document_add_jpg_derivative($object, $file_uri, 'PREVIEW', $width, $height, $force);
  file_unmanaged_delete($file_uri);
  return $results;
}

/**
 * Creates a JPG derivative of the given file.
 *
 * @param AbstractObject $object
 *   The object to add the derivative datastreams to.
 * @param string $file_uri
 *   The URI to the PDF file the derivative will be generated from.
 * @param string $dsid
 *   The datastream ID to use for the derived datastream.
 * @param int $width
 *   The width to make the derived datastream.
 * @param int $height
 *   The height to make the derived datastream.
 * @param bool $force
 *   Whether the derivative generation is being forced or not.
 *
 * @return array
 *   An array describing the outcome of adding the JPG derivative.
 *
 * @see hook_islandora_derivative()
 */
function islandora_document_add_jpg_derivative(AbstractObject $object, $file_uri, $dsid, $width, $height, $force) {
  if (!isset($object[$dsid]) || (isset($object[$dsid]) && $force === TRUE)) {
    if (!isset($object['OBJ'])) {
      return islandora_document_no_obj_datastream($object->id);
    }
    if (isset($object['PDF']) && $object['PDF']->size != -1) {
      $derivative_file_uri = islandora_document_create_jpg_derivative($file_uri, $dsid, $width, $height);
    }
    else {
      $derivative_file_uri = drupal_get_path('module', 'islandora_document') . "/images/folder.png";
    }

    // Receive a valid file URI to add or an error message otherwise.
    if (!is_array($derivative_file_uri) && file_valid_uri($derivative_file_uri)) {
      $success = islandora_document_add_datastream($object, $dsid, $derivative_file_uri);
      if ($success === TRUE) {
        return [
          'success' => TRUE,
          'messages' => [
            [
              'message' => t('Created JPG derivative for @dsid (@pid).'),
              'message_sub' => [
                '@dsid' => $dsid,
                '@pid' => $object->id,
              ],
              'type' => 'dsm',
            ],
          ],
        ];
      }
      else {
        return [
          'success' => FALSE,
          'messages' => [
            [
              'message' => t('Failed to create JPG derivative for @dsid (@pid). Error message: @message'),
              'message_sub' => [
                '@dsid' => $dsid,
                '@pid' => $object->id,
                '@message' => $success,
              ],
              'type' => 'watchdog',
              'severity' => RfcLogLevel::ERROR,
            ],
          ],
        ];
      }
    }
    else {
      return $derivative_file_uri;
    }
  }
}

/**
 * Creates a jpg derivative from the given pdf file.
 *
 * @param string $file_uri
 *   The URI to the pdf file the derivative will be generated from.
 * @param string $dsid
 *   Used to identify the generated file.
 * @param int $width
 *   The width to make the derived datastream.
 * @param int $height
 *   The height to make the derived datastream.
 *
 * @return string|array
 *   A URI to the generated derivative if successful, error message otherwise.
 *
 * @see hook_islandora_derivative()
 */
function islandora_document_create_jpg_derivative($file_uri, $dsid, $width, $height) {
  $source = \Drupal::service("file_system")->realpath($file_uri);
  // Get the base name of the source file.
  $base = pathinfo($source, PATHINFO_FILENAME);
  $temp = file_create_filename("$base.$dsid.jpg", 'temporary://');
  $dest = \Drupal::service("file_system")->realpath($temp);
  $exec_manager = \Drupal::service('imagemagick.exec_manager');

  $args = new ImagemagickExecArguments($exec_manager);
  $args->setSourceLocalPath($source);
  $args->setDestinationLocalPath($dest);
  $args->setSourceFrames('[0]');
  $args->addArgument('-resize ' . $args->escapeShellArg("{$width}x{$height}"));
  $args->addArgument('-colorspace RGB');
  $args->addArgument('-flatten');

  $command = 'convert';
  \Drupal::moduleHandler()->alter('imagemagick_arguments', $args, $command);
  $output = '';
  $ret = -1;
  if ($exec_manager->execute($command, $args, $output, $ret) !== TRUE) {
    return [
      'success' => FALSE,
      'messages' => [
        [
          'message' => t('ImageMagick failed to create derivatives for @pid<br/>Error: @ret<br/>Command: @command<br/>Output: @output'),
          'message_sub' => [
            '@ret' => $ret,
            '@command' => $command,
            '@output' => $output,
          ],
          'type' => 'watchdog',
          'severity' => RfcLogLevel::ERROR,
        ],
      ],
    ];
  }
  return $temp;
}

/**
 * Adds the file as a datastream to the object using the DSID to identify it.
 *
 * @param AbstractObject $object
 *   The object to add the datastream to.
 * @param string $datastream_id
 *   The datastream id of the added datastream.
 * @param string $file_uri
 *   A URI to the file containing the content for the datastream.
 *
 * @return bool|string
 *   TRUE if successfully added/modified the datastream, error message if not.
 */
function islandora_document_add_datastream(AbstractObject $object, $datastream_id, $file_uri) {
  $return = TRUE;
  try {
    $ingest = !isset($object[$datastream_id]);
    $mime_detector = new MimeDetect();
    if ($ingest) {
      $ds = $object->constructDatastream($datastream_id, "M");
      $ds->label = $datastream_id;
    }
    else {
      $ds = $object[$datastream_id];
    }
    if ($ds->mimetype != $mime_detector->getMimetype($file_uri)) {
      $ds->mimetype = $mime_detector->getMimetype($file_uri);
    }
    $ds->setContentFromFile(\Drupal::service("file_system")->realpath($file_uri), FALSE);
    if ($ingest) {
      $object->ingestDatastream($ds);
    }
  }
  catch (exception $e) {
    $return = $e->getMessage();
  }
  file_unmanaged_delete($file_uri);
  return $return;
}

/**
 * Helper function declaring that no OBJ datastream is present.
 *
 * @param string $pid
 *   The pid of the ingested object.
 *
 * @return array
 *   An array describing the failure of the operation..
 *
 * @see hook_islandora_derivative()
 */
function islandora_document_no_obj_datastream($pid) {
  return [
    'success' => FALSE,
    'messages' => [
      [
        'message' => t('Could not create derivatives for %s. No file was uploaded.'),
        'message_sub' => ['%s' => $pid],
        'type' => 'watchdog',
        'severity' => RfcLogLevel::ERROR,
      ],
    ],
  ];
}

/**
 * Creates a PDF datastream.
 *
 * @param AbstractObject $object
 *   The object to add the derivative datastreams to.
 * @param bool $force
 *   Whether the derivative generation is being forced or not.
 *
 * @return array
 *   An array describing the outcome of the thumbnail creation.
 *
 * @see hook_islandora_derivative()
 */
function islandora_document_add_pdf_derivative(AbstractObject $object, $force = FALSE) {
  module_load_include('inc', 'islandora_jodconverter', 'includes/utilities');
  if (!isset($object['PDF']) || (isset($object['PDF']) && $force === TRUE)) {
    if (!isset($object['OBJ'])) {
      return islandora_document_no_obj_datastream($object->id);
    }

    $file_uri = islandora_document_get_derivative_source_as_unmanaged_file($object);
    $dsid = "PDF";
    $from = \Drupal::service("file_system")->realpath($file_uri);
    $derivative_file_uri = \Drupal::service("file_system")->realpath(file_create_filename('pdffile.pdf', 'temporary://'));
    if ($object['OBJ']->mimetype == 'application/pdf') {
      $success = islandora_document_add_datastream($object, $dsid, $from);
    }
    else {
      $success = islandora_jodconverter_convert($from, $derivative_file_uri);
      file_unmanaged_delete($file_uri);
      if ($success && file_exists($derivative_file_uri)) {
        $success = islandora_document_add_datastream($object, $dsid, $derivative_file_uri);
      }
      else {
        $success = FALSE;
      }
    }
    if ($success === TRUE) {
      return [
        'success' => TRUE,
        'messages' => [
          [
            'message' => t('Created PDF derivative.'),
            'type' => 'dsm',
          ],
        ],
      ];
    }
    else {
      return [
        'success' => FALSE,
        'messages' => [
          [
            'message' => t('Failed to create PDF derivative for @pid. Error message: @error'),
            'message_sub' => [
              '@pid' => $object->id,
              '@error' => $success,
            ],
            'type' => 'watchdog',
            'severity' => RfcLogLevel::ERROR,
          ],
        ],
      ];
    }
  }
  return [
    'success' => TRUE,
    'messages' => [
      [
        'message' => t('PDF derivative for @pid already exists.'),
        'type' => 'dsm',
      ],
    ],
  ];
}
