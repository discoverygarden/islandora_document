<?php

namespace Drupal\islandora_document\Form;

use Drupal\islandora\Form\ModuleHandlerAdminForm;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Render\RendererInterface;
use Drupal\Core\Url;
use Drupal\Core\Link;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Module administration form.
 */
class Admin extends ModuleHandlerAdminForm {
  /**
   * Renderer instance.
   *
   * @var Drupal\Core\Render\RendererInterface
   */
  protected $renderer;

  /**
   * {@inheritdoc}
   */
  public function __construct(RendererInterface $renderer) {
    $this->renderer = $renderer;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('renderer')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'islandora_document_admin';
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $config = $this->config('islandora_document.settings');

    $config->set('islandora_document_path_to_pdftotext', $form_state->getValue('islandora_document_path_to_pdftotext'));
    $config->set('islandora_document_allow_text_upload', $form_state->getValue('islandora_document_allow_text_upload'));
    $config->set('islandora_document_create_fulltext', $form_state->getValue('islandora_document_create_fulltext'));
    $config->set('islandora_document_thumbnail_width', $form_state->getValue('islandora_document_thumbnail_width'));
    $config->set('islandora_document_thumbnail_height', $form_state->getValue('islandora_document_thumbnail_height'));
    $config->set('islandora_document_preview_width', $form_state->getValue('islandora_document_preview_width'));
    $config->set('islandora_document_preview_height', $form_state->getValue('islandora_document_preview_height'));

    islandora_set_viewer_info('islandora_document_viewers', $form_state->getValue('islandora_document_viewers'));

    $config->save();
  }

  /**
   * {@inheritdoc}
   */
  protected function getEditableConfigNames() {
    return ['islandora_document.settings'];
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $form_state->loadInclude('islandora_document', 'inc', 'includes/admin.form');
    $config = $this->config('islandora_document.settings');
    if (NULL !== $form_state->getValue('islandora_document_path_to_pdftotext')) {
      $islandora_document_path_to_pdftotext = $form_state->getValue('islandora_document_path_to_pdftotext');
    }
    else {
      $islandora_document_path_to_pdftotext = $config->get('islandora_document_path_to_pdftotext');
    }
    exec($islandora_document_path_to_pdftotext, $output, $return_value);
    $image = [
      '#theme' => 'image',
      '#uri' => Url::fromUri('base:core/misc/icons/73b355/check.svg')->toString(),
    ];
    $confirmation_message = $this->renderer->render($image)
      . $this->t('pdftotext executable found at @url', ['@url' => $islandora_document_path_to_pdftotext]);

    if ($return_value != 99) {
      $image = [
        '#theme' => 'image',
        '#uri' => Url::fromUri('base:core/misc/icons/e32700/error.svg')->toString(),
      ];
      $confirmation_message = $this->renderer->render($image)
        . $this->t('Unable to find pdftotext executable at @url', ['@url' => $islandora_document_path_to_pdftotext]);
    }
    // @Todo: add a configuration to choose which Derivatives to build by call the
    // jodconverter's get format function.
    $form = [];
    // AJAX wrapper for url checking.
    $form['islandora_document_url_fieldset'] = [
      '#type' => 'fieldset',
      '#title' => $this->t('TEXT'),
    ];
    $form['islandora_document_url_fieldset']['islandora_document_allow_text_upload'] = [
      '#type' => 'checkbox',
      '#title' => $this->t('Allow users to upload .txt files with PDFs'),
      '#description' => $this->t('Uploaded text files are appended to PDFs as FULL_TEXT datastreams and are indexed into Solr.'),
      '#default_value' => $config->get('islandora_document_allow_text_upload'),
    ];
    $form['islandora_document_url_fieldset']['islandora_document_create_fulltext'] = [
      '#type' => 'checkbox',
      '#title' => $this->t('Extract text from PDFs using pdftotext'),
      '#description' => $this->t("Extracted text is stored in the FULL_TEXT datastreams and is indexed into Solr. If a text file is uploaded it will be used instead of the extracted text. </br><strong>Note:</strong> PDFs that contain visible text do not necessarily contain text (e.g. images scanned and saved as PDFs). Consider converting text-filled images with no text streams to TIFFs and using the @book with @ocr enabled.",
        [
          '@book' => Link::fromTextAndUrl($this->t('Book Solution Pack'), Url::fromUri('https://wiki.duraspace.org/display/ISLANDORA711/Book+Solution+Pack'))->toString(),
          '@ocr' => Link::fromTextAndUrl($this->t('OCR'), Url::fromUri('https://wiki.duraspace.org/display/ISLANDORA711/Islandora+OCR'))->toString(),
        ]
      ),
      '#default_value' => $config->get('islandora_document_create_fulltext'),
    ];
    $form['islandora_document_url_fieldset']['wrapper'] = [
      '#prefix' => '<div id="islandora-url">',
      '#suffix' => '</div>',
      '#type' => 'markup',
    ];
    $form['islandora_document_url_fieldset']['wrapper']['islandora_document_path_to_pdftotext'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Path to pdftotext executable'),
      '#default_value' => $islandora_document_path_to_pdftotext,
      '#description' => $confirmation_message,
      '#ajax' => [
        'callback' => 'islandora_document_update_pdftotext_url_div',
        'wrapper' => 'islandora-url',
        'effect' => 'fade',
        'event' => 'blur',
        'disable-refocus' => TRUE,
        'progress' => ['type' => 'throbber'],
      ],
      '#states' => [
        'visible' => [
          ':input[name="islandora_document_create_fulltext"]' => ['checked' => TRUE],
        ],
      ],
    ];
    $form['islandora_document_thumbnail_fieldset'] = [
      '#type' => 'fieldset',
      '#title' => $this->t('Thumbnail'),
      '#description' => $this->t('Settings for creating PDF thumbnail derivatives'),
    ];
    $form['islandora_document_thumbnail_fieldset']['islandora_document_thumbnail_width'] = [
      '#type' => 'number',
      '#title' => $this->t('Width'),
      '#description' => $this->t('The width of the thumbnail in pixels.'),
      '#default_value' => $config->get('islandora_document_thumbnail_width'),
      '#size' => 5,
    ];
    $form['islandora_document_thumbnail_fieldset']['islandora_document_thumbnail_height'] = [
      '#type' => 'number',
      '#title' => $this->t('Height'),
      '#description' => $this->t('The height of the thumbnail in pixels.'),
      '#default_value' => $config->get('islandora_document_thumbnail_height'),
      '#size' => 5,
    ];
    $form['islandora_document_preview_fieldset'] = [
      '#type' => 'fieldset',
      '#title' => $this->t('Preview image'),
      '#description' => $this->t('Settings for creating PDF preview image derivatives'),
    ];
    $form['islandora_document_preview_fieldset']['islandora_document_preview_width'] = [
      '#type' => 'number',
      '#title' => $this->t('Max width'),
      '#description' => $this->t('The maximum width of the preview in pixels.'),
      '#default_value' => $config->get('islandora_document_preview_width'),
      '#size' => 5,
    ];
    $form['islandora_document_preview_fieldset']['islandora_document_preview_height'] = [
      '#type' => 'number',
      '#title' => $this->t('Max height'),
      '#description' => $this->t('The maximum height of the preview in pixels.'),
      '#default_value' => $config->get('islandora_document_preview_height'),
      '#size' => 5,
    ];
    $form_state->loadInclude('islandora', 'inc', 'includes/solution_packs');
    $form += islandora_viewers_form('islandora_document_viewers', 'application/pdf');
    $form['submit'] = [
      '#type' => 'submit',
      '#value' => $this->t('Submit'),
    ];
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function validateForm(array &$form, FormStateInterface $form_state) {
    if (FALSE !== $form_state->getValue('islandora_document_create_fulltext')) {
      $islandora_document_path_to_pdftotext = $form_state->getValue('islandora_document_path_to_pdftotext');
      exec($islandora_document_path_to_pdftotext, $output, $return_value);
      if ($return_value != 99) {
        return $form_state->setError($form['islandora_document_url_fieldset']['wrapper']['islandora_document_path_to_pdftotext'], $this->t('Cannot extract text from PDF without a valid path to pdftotext.'));
      }
    }
  }

}
