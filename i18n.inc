<?php
// $Id$

/**
 * Internationalization (i18n) module
 *
 * These are the functions to be defined only when module is enabled.
 * Otherwise they could mess up with the admin system
 *
 * @author Jose A. Reyero, 2004
 *
 */

/**
 * Implementation of hook_db_rewrite_sql()
 */
function i18n_db_rewrite_sql($query, $primary_table, $primary_key){
  $alias = $primary_key{0};
  switch ($alias) {
    case 'n':
      if ($primary_table != 'n') {
        $result['join'] = 'INNER JOIN {node} n USING (nid)';
      } // deliberate no break
    case 't': case 'v':
      $result['where'] = $alias .".language ='".i18n_get_lang()."' or ". $alias .".language = ''" ;
  }
  return $result;
}

/**
 * Implementation of hook_menu().
 */
function i18n_menu($may_cache) {
  $items = array();

  if ($may_cache) {
      $items[] = array(
        'path' => 'translation',
        'title' => t('translation'),
        'callback' => 'i18n_translation_page',
        'access' => user_access('administer nodes'),
        'type' => MENU_CALLBACK);
      
      $items[] = array(
        'path' => 'admin/taxonomy/i18n', 
        'title' => t('translation'),
        'callback' => 'i18n_taxonomy_admin',
        'access' => user_access('administer taxonomy'),
        'type' => MENU_LOCAL_TASK);

  }
  else {
    if (arg(0) == 'node' && is_numeric(arg(1)) && variable_get('i18n_node_'.i18n_get_node_type(arg(1)), 0)) {
      $access = user_access('administer nodes');
      $type = MENU_LOCAL_TASK;      
      $items[] = array(
        'path' => 'node/'. arg(1) .'/translation', 
        'title' => t('translation'),
        'callback' => 'i18n_node_translation',
        'access' => $access,
        'type' => $type);
    }
  }

  return $items;
} // End of i18n_menu
 
/**
 * Implementation of hook_link().
 */
function i18n_link($type, $node = NULL, $teaser = FALSE) {
  $links = array();
  if ($type == 'node' && !$teaser && variable_get('i18n_node_'.$node->type, 0)) {
    $languages = i18n_supported_languages();
    $translations = i18n_node_get_translations($node->nid);
    foreach ($translations as $lang => $trnode) {
        $links[]= theme('i18n_link_name_flag', $lang, 'node/'. $trnode->nid);
     }
  }
  return $links;
}

/**
 * Manage language dependent variables
 *
 * Requires a patch in bootstrap.inc
 */

function i18n_variable($name) {
  global $i18n_variables;
  if (is_array($i18n_variables) and in_array($name, $i18n_variables)) {
    return _i18n_get_lang().'_'.$name ;
  } else {
    return $name;
  }
}
 
/**
 * Rewriting urls
 */
function i18n_url_rewrite($path, $mode ='outgoing') {
   if ($mode == 'outgoing' && !i18n_get_lang_prefix($path)) {
      return i18n_get_lang() . '/'. $path;
   } else {
     return $path;
   }

}  // End of i18n_url_rewrite

/**
 * This one expects to be called first from common.inc
 * When first called, calls _i18n_init();
 */
function i18n_get_lang() {
  static $i18n_language;
  //see if the language is already set.
  if ($i18n_language) {
    return $i18n_language;
  } else {
    _i18n_init();
    return $i18n_language = _i18n_get_lang();
  }
}  // End of i18n_get_lang

/*
 * Second stage init, from common.inc
 * Does some additional includes
 */
function _i18n_init() {
  // Include parts, may be conditional include in the future
  include_once 'modules/i18n/i18n_node.inc';
  include_once 'modules/i18n/i18n_taxonomy.inc';  
}

/**
 * Language block
 */
function i18n_block($op = 'list', $delta = 0) {
  if ($op == 'list') {
    $blocks[0]['info'] = t('Languages');
  }
  else {
    $blocks['subject'] = t('Languages');
    $content = '<table><tr><td>' ;
    $content .=  theme('i18n_links', 1, 1, '</td><td>', '</td></tr><tr><td>' );
    $content .= '</td></tr></table>' ;
    $blocks['content'] =$content;
  }

  return $blocks;
}


/**
 * @name Theme functions
 * @{
 */
 
/**
  * Returns language links with optional flags
  *
  * @param $flags an integer, 1 to use language flags
  * @param $names an integer, 1 to use language names
  * @param $delim1 delimiter to place between language name and flag
  * @param $delim2 delimiter to place between different languages
  *
  * @return a string containing the @a links output.
  */
  
function theme_i18n_links($flags = 1, $names = 1, $delim1 = ' ' , $delim2 = ' ' ) {
  $i18n_lang = i18n_get_lang();
  $languages = i18n_supported_languages();
  foreach ($languages as $lang => $langname) {
    $name = $names ? t($langname): '' ; // Should be localized??
    $flag= $flags ? i18n_flag($lang) : '' ;
    if ($lang == $i18n_lang) {
      $links[] = "<strong>$name</strong>$delim1$flag";
    }
    else {
      $links[] = i18n_l($name, $lang).$delim1.i18n_l($flag, $lang);
    }
  }
  $output =implode($delim2, $links);
  return $output;  
}

function theme_i18n_flags() {
  return theme_i18n_links(1, 0);
}

function theme_i18n_link_name_flag($lang, $path, $attributes = array()) {
  static $languages;
  if (!isset($languages)) {
    $languages = i18n_supported_languages();
  }
  return '<span class="i18n-link">'. l(t($languages[$lang]), $path, $attributes) . '&nbsp;' . l(i18n_flag($lang) , $path, $attributes) .'</span>';
}


/*
 * Creates links for different languages
 */
 
function i18n_l($text, $lang , $url = '' , $attributes = array(), $query = NULL) {
  global $i18n_langpath;
  // If !url get from original request
  if (!$url) {
    $url = i18n_get_original_path();
  }
    // If url has lang_prefix, remove it
  if (i18n_get_lang_prefix($url)) {
    $url = substr($url, 3);
  }
 // $result= l($text, $url, $attributes, $query);
  return '<a href="'. i18n_url($url, $lang, $query) .'"'. drupal_attributes($attributes) .'>'. $text .'</a>';
}

function i18n_url($url, $lang, $query = NULL) {
  if ($url) {
    return url($lang.'/'.$url, $query);
  } else {
    return url($lang,$query);
  }
}

function i18n_flag($lang, $attribs = array()) {
  if ($path = variable_get('i18n_flags', '<img class="i18n-flag" src="modules/i18n/flags/*.png" width="16" height="12"/>')) {
    $flag = str_replace('*' , $lang, $path);
    if(!empty($attribs)){
      $flag = str_replace('<','< '.drupal_attributes($attribs).' ',$flag);
    }
		return $flag;
  }
}
/* @} */

?>