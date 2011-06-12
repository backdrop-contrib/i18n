<?php
/**
 * Blocks textgroup handler
 */

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