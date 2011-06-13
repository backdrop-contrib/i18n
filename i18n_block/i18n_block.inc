<?php
/**
 * Blocks textgroup handler
 */

/**
 * Block object
 */
class i18n_block_object extends i18n_string_object_wrapper {
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
    if ($this->object->module == 'block' && !isset($this->object->body)) {
      $block = (object) block_custom_block_get($this->object->delta);
      $this->object->body = $block->body;
      $this->object->format = $block->format;
    }
    return parent::get_properties();
  }
  /**
   * Translation mode for object
   */
  public function get_translate_mode() {
    return !empty($this->object->i18n_mode) ? I18N_MODE_LOCALIZE : I18N_MODE_NONE;
  }
}