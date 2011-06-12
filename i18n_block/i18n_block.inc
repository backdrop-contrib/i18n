<?php
/**
 * Blocks textgroup handler
 */
class i18n_block_textgroup extends i18n_string_default {
  /**
   * Get edit path for object
   *
   * @param $type
   * 	 Object type in this text group, like 'vocabulary', 'term', etc..
   * @param $object
   *   The Drupal object itself
   */
  public function get_edit_path($type, $object) {
    if ($type == 'block') {
      return 'admin/structure/block/manage/' . $object->module . '/' . $object->delta . '/configure';
    }
    else {
      return parent::get_edit_path($type, $object);
    }
  }
  /**
   * Get translate path for object
   *
   * @param $type
   * 	 Object type in this text group, like 'vocabulary', 'term', etc..
   * @param $object
   *   The Drupal object itself
   */
  public function get_translate_path($type, $object, $language = NULL) {
    if ($type == 'block') {
      return 'admin/structure/block/manage/' . $object->module . '/' . $object->delta . '/translate' . ($language ? '/' . $language->language : '');
    }
    else {
      return parent::get_translate_path($type, $object, $language);
    }
  }
  /**
   * Get base keys for translating this object
   */
  public function get_object_context($type, $object) {
    if ($type == 'block') {
      return array($object->module, $object->delta);
    }
    else {
      return parent::get_object_context($type, $object);
    }
  }

  /**
   * Get object strings for translation
   */
  public function get_object_properties($object_type, $object) {
    if ($object_type == 'block' && $object->i18n_mode & I18N_MODE_LOCALIZE) {
      if ($object->module == 'block' && !isset($object->body)) {
        $block = (object) block_custom_block_get($object->delta);
        $object->body = $block->body;
        $object->format = $block->format;
      }
    }
    return parent::get_object_properties($object_type, $object);
  }
}

/**
 * Block object
 */
class i18n_block_object extends i18n_string_object {
  /**
   * Get base path for object
   */
  protected function get_base_path() {
    return 'admin/structure/block/manage/' . $this->object->module . '/' . $this->object->delta;
  }
  /**
   * Get edit path for object
   *
   * @param $type
   * 	 Object type in this text group, like 'vocabulary', 'term', etc..
   * @param $object
   *   The Drupal object itself
   */
  public function get_edit_path() {
     return $this->get_base_path() . '/configure';
  }
  /**
   * Get translate path for object
   *
   * @param $type
   * 	 Object type in this text group, like 'vocabulary', 'term', etc..
   * @param $object
   *   The Drupal object itself
   */
  public function get_translate_path($language = NULL) {
    return $this->get_base_path() . '/translate' . ($language ? '/' . $language->language : '');
  }
  /**
   * Get base keys for translating this object
   */
  public function get_string_context() {
    return array($this->object->module, $this->object->delta);
  }

  /**
   * Get object strings for translation
   */
  public function get_properties() {
    if ($this->object->i18n_mode & I18N_MODE_LOCALIZE) {
      if ($this->object->module == 'block' && !isset($this->object->body)) {
        $block = (object) block_custom_block_get($this->object->delta);
        $this->object->body = $block->body;
        $this->object->format = $block->format;
      }
    }
    return parent::get_properties();
  }
}