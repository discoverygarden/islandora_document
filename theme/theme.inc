<?php

/**
 * @file
 * This file contains all theme and preprocess functions.
 */

use Drupal\Core\Link;
use Drupal\Core\Url;

use Drupal\islandora\DublinCore;

/**
 * Implements template_preprocess_HOOK().
 *
 * Prepares variables for islandora-pdf templates.
 *
 * Default template: islandora-pdf.tpl.php.
 *
 * @param array $variables
 *   An associative array containing:
 *   - islandora_object: An AbstractObject from Islandora.
 */
function template_preprocess_islandora_document(array &$variables) {
  $islandora_object = $variables['islandora_object'];
  module_load_include('inc', 'islandora', 'includes/datastream');
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/metadata');

  // We should eventually remove the DC object and dc_array code as it only
  // exists to not break legacy implementations.
  try {
    if (isset($islandora_object['DC']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['DC'])) {
      $dc = $islandora_object['DC']->content;
      $dc_object = DublinCore::importFromXMLString($dc);
    }
  }
  catch (Exception $e) {
    drupal_set_message(t('Error retrieving object %s %t', ['%s' => $islandora_object->id, '%t' => $e->getMessage()]), 'error', FALSE);
  }
  $variables['islandora_dublin_core'] = isset($dc_object) ? $dc_object : NULL;
  $variables['dc_array'] = isset($dc_object) ? $dc_object->asArray() : [];
  $variables['islandora_object_label'] = $islandora_object->label;
  $variables['theme_hook_suggestions'][] = 'islandora_document__' . str_replace(':', '_', $islandora_object->id);
  $variables['parent_collections'] = islandora_get_parents_from_rels_ext($islandora_object);
  $variables['metadata'] = islandora_retrieve_metadata_markup($islandora_object);
  $variables['description'] = islandora_retrieve_description_markup($islandora_object);

  module_load_include('inc', 'islandora', 'includes/solution_packs');
  $viewer = islandora_get_viewer([], 'islandora_pdf_viewers', $islandora_object);

  if ($viewer) {
    $variables['islandora_content'] = $viewer;
  }
  else {
    // Full size url.
    if (isset($islandora_object['PDF'])) {
      $full_size_url = Url::fromRoute('islandora.view_datastream_view', ['object' => $islandora_object->id, 'datastream' => 'PDF'], ['absolute' => TRUE]);

      $variables['islandora_full_url'] = $full_size_url->toString();
    }
    // Thumbnail.
    if (isset($islandora_object['TN'])) {
      $thumbnail_size_url = Url::fromRoute('islandora.view_datastream_view', ['object' => $islandora_object->id, 'datastream' => 'TN']);

      $params = [
        '#title' => $islandora_object->label,
        '#theme' => 'image',
        '#path' => $thumbnail_size_url->toString(),
      ];
      $variables['islandora_thumbnail_img'] = \Drupal::service("renderer")->render($params);

    }
    // Preview image + link.
    if (isset($islandora_object['PREVIEW'])) {
      $preview_url = Url::fromRoute('islandora.view_datastream_view', ['object' => $islandora_object->id, 'datastream' => 'PREVIEW']);

      $params = [
        '#title' => $islandora_object->label,
        '#theme' => 'image',
        '#path' => $preview_url->toString(),
      ];
      $variables['islandora_preview_img'] = \Drupal::service("renderer")->render($params);;

      $variables['islandora_content'] = Link::fromTextAndUrl($variables['islandora_preview_img'], $full_size_url);

      // Sanitize this object name a bit and provide download link.
      $sanitized_label = preg_replace('/[^A-Za-z0-9_\-]|\.pdf$/', '_', $islandora_object->label);
      $download_url = Url::fromRoute('islandora.download_datastream', [
        'object' => $islandora_object->id,
        'datastream' => 'OBJ',
        'filename' => "{$sanitized_label}.pdf",
      ], ['attributes' => ['class' => ['islandora-pdf-link']]]);
      $download_text = t("Download document");
      $variables['islandora_download_link'] = Link::fromTextAndUrl($download_text, $download_url)->toString();
    }
  }
}
