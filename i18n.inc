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
 * Implementation of conf_url_rewrite
 * 
 * This is a conditional definition, just in case it is defined somewhere else
 */

if(!function_exists('custom_url_rewrite')) {
  function custom_url_rewrite($type, $path, $original) {
    return i18n_url_rewrite($type, $path, $original);
  }
}  

function i18n_url_rewrite($type, $path, $original){
   if ($type ='alias' && !i18n_get_lang_prefix($path) ){
     return $path ? i18n_get_lang() . '/'. $path : i18n_get_lang();
   } else {
     return $path;
   } 
} // End of i18n_url_rewrite

/**
 * Implementation of hook_db_rewrite_sql()
 */
function i18n_db_rewrite_sql($query, $primary_table, $primary_key){
  // Some exceptions for query rewrites
  if(strstr(request_uri(),'cron.php')) return;

  $alias = $primary_table;

  switch ($alias) {
    case 'n':
      $result = i18n_db_node_rewrite($query, $primary_table, $primary_key);
      break;
    case 't':
    case 'v':
      $result = i18n_db_taxonomy_rewrite($query, $primary_table, $primary_key);
      break;
  }

  return $result;
}

function i18n_db_node_rewrite($query, $primary_table, $primary_key){
  $alias = $primary_key{0};
  // When loading specific nodes, language conditions shouldn't apply  
  if (preg_match("/WHERE.* $alias.nid\s*=/", $query)) return;

  $mode = i18n_selection_mode();
  if($mode == 'off') return;
  
  if ($primary_table != 'n' && !strstr($query, '{node}')) {
    $result['join'] = 'INNER JOIN {node} n USING (nid)';
    $alias = 'n';
  } 
  switch($mode){
    case 'simple':
      $result['where'] = "$alias.language ='".i18n_get_lang()."' OR $alias.language = ''" ;
      break;
    case 'mixed':
      $result['where'] = "$alias.language ='".i18n_get_lang()."' OR $alias.language ='".i18n_default_language()."' OR $alias.language = ''" ;
      break;
    case 'strict':
      $result['where'] = "$alias.language ='".i18n_get_lang()."'" ;
      break;
    case 'default':
      $result['where'] = "$alias.language ='".i18n_default_language()."' OR $alias.language = ''" ;
      break;
      
  }

  return $result;
}

function i18n_db_taxonomy_rewrite($query, $primary_table, $primary_key){
  // Decide language depending on what we are doing
  $lang = i18n_get_lang(); // Default
  switch(arg(0)) {
    case 'node':
	    $edit = $_POST['edit'];
	    if( arg(1) == 'add'){
	      // Adding node
	      $lang = isset($edit['language']) ? $edit['language'] : i18n_get_lang();
	    } elseif( is_numeric(arg(1)) && arg(2) == 'edit') {
	      // Editing a node
	      if(is_array($edit)) {
	        $lang = isset($edit['language']) ? $edit['language'] : i18n_get_lang();
	      } else {
	        $lang = i18n_node_get_lang(arg(1), i18n_get_lang());
	      }
	    } else {
	      $lang = i18n_get_lang();
	    }
	    break;
    case 'translation':
      $edit = $_POST['edit'];
      $lang = isset($edit['language']) ? $edit['language'] : arg(3);
      break;
    case 'admin': 
		  if(arg(1) == 'taxonomy') {
		    // Taxonomy administration. Show all
		    $lang = NULL;
		  }
  }
  
  if($lang) {
    $alias = $primary_table;
    $result['where'] = $alias .".language ='$lang' or ". $alias .".language = ''" ;
    return $result;
  }
}

/**
 * Implementation of hook_exit
 */
function i18n_exit(){
  _i18n_variable_exit();
}

/**
 * Implementation of hook_form_alter
 * 
 * This is the place to add language fields to all forms
 */
function i18n_form_alter($form_id, &$form) {
  drupal_set_message("i18n_form_alter id= $form_id");
  switch($form_id){
    case 'taxonomy_form_vocabulary': // Taxonomy vocabulary
      if(isset($form['vid'])) {
        $vocabulary = taxonomy_get_vocabulary($form['vid']['#value']);
      } 
	  $form['language'] = _i18n_language_select(isset($vocabulary) ? $vocabulary->language : i18n_get_lang(),t('This language will be set for all terms in this vocabulary')); 
      break;
    case 'taxonomy_form_term': // Taxonomy term
      if(isset($form['vid']) && is_numeric($form['vid'])) {
        $vocabulary = taxonomy_get_vocabulary($form['vid']);
      } 
	  $form['language'] = _i18n_language_select(isset($vocabulary) ? $vocabulary->language : i18n_get_lang());
      break;
    default:
    // Content type settings
	  if (isset($form['type']) && $form['type']['#value'] .'_node_settings' == $form_id) {
	    $form['workflow']['i18n_node_'. $form['type']['#value']] = array(
	      '#type' => 'radios',
	      '#title' => t('Multilingual support'),
	      '#default_value' => variable_get('i18n_node_'. $form['type']['#value'], 0),
	      '#options' => array(t('Disabled'), t('Enabled')),
	      '#description' => t('Enables language field and multilingual support for this content type.'),
	    );
	  }
  }    
}

/**
 * Implementation of hook_nodeapi
 */
function i18n_nodeapi(&$node, $op, $arg = 0) {
  if (variable_get("i18n_node_$node->type", 0)) {
    switch ($op) {
      case 'fields':
        return array('language');
        break;
        
      case 'form':
        $form['language'] = _i18n_language_select($node->language ? $node->language : ($node->nid ? i18n_get_lang() : ''), t('If you change the Language, you must click on <i>Preview</i> to get the right Categories &amp; Terms for that language.'));
        return $form;
    }
  }
}

/**
 * Helper function to create language selector
 */
function _i18n_language_select($value ='', $description ='', $weight = -20){
  return array(
	  '#type' => 'select',
	  '#title' => t('Language'),
	  '#default_value' => $value,
	  '#options' => array_merge(array('' => ''), i18n_supported_languages()),
	  '#description' => $description,
	  '#weight' => -20,
  );
}

/**
 * Implementation of hook_taxonomy
 * 
 * $edit parameter is an array, not an object !!
 */

function i18n_taxonomy($op, $type, $edit = NULL) {
  switch ("$type/$op") {
    case 'term/insert':
    case 'term/update':
        $language = isset($edit['language']) ? $edit['language'] : '';
        db_query("UPDATE {term_data} SET language='%s' WHERE tid=%d", $language, $edit['tid']);    
        break;
    case 'vocabulary/insert':    
    case 'vocabulary/update':
        $language = isset($edit['language']) ? $edit['language'] : '';
        db_query("UPDATE {vocabulary} SET language='%s' WHERE vid=%d", $language, $edit['vid']);    
        if ($language && $op == 'update') {
          db_query("UPDATE {term_data} SET language='%s' WHERE vid='%d'",$edit['language'],$edit['vid']);
          drupal_set_message(t('Reset language for all terms.'));
        }
        break;
  }
}

/**
 * More i18n API
 */

/**
 * This one expects to be called first from common.inc
 */
function i18n_get_lang() {
  static $i18n_language;
  //see if the language is already set.
  if ($i18n_language) {
    return $i18n_language;
  } else {
    return $i18n_language = _i18n_get_lang();
  }
}


function i18n_url($url, $lang, $query = NULL) {
  // Strip prefix if default language
  if ($url) {
    return url($lang.'/'.$url, $query);
  } else {
    return url($lang, $query);
  }
} 
function i18n_node_get_lang($nid, $default = '') {
  $lang = db_result(db_query('SELECT language FROM {node} WHERE nid=%d',$nid));
  return $lang ? $lang : $default ;
}

/**
 * Returns main language, two letter code
 */
function i18n_get_main_lang($lang = NULL){
  $lang = $lang ? $lang : i18n_get_lang();
  return substr($lang, 2);
}


/**
 * Language block
 * 
 * This is a simple language switcher which knows nothing about translations
 */
function i18n_block($op = 'list', $delta = 0) {
  if ($op == 'list') {
    $blocks[0]['info'] = t('Language switcher');
  }
  elseif($op == 'view') {
    $blocks['subject'] = t('Languages');
    $blocks['content'] = theme('item_list', i18n_get_links($_GET['q']));
  }

  return $blocks;
}

/**
 * Function i18n_get_links
 * 
 * Returns an array of links for all languages, with or without names/flags
 */
function i18n_get_links($path = '', $names = 1) {
  $current = i18n_get_lang();
  foreach(i18n_supported_languages() as $lang => $name){
    $url = i18n_url($path, $lang);
    $name = $names ? $name: '' ; // Should be localized??
    $links[]= theme('i18n_link', $name, $url , $lang);
  }
  return $links;  
}


/**
 * Multilingual variables 
 */
function i18n_variable_init(){
  global $conf;
  global $i18n_conf;
  $lang = _i18n_get_lang();
  if($i18n_variables = variable_get('i18n_variables', '')){
    $i18n_conf = array();
    $variables = _i18n_variable_init();
    foreach($i18n_variables as $name){
      $i18n_conf[$name] = isset($variables[$name]) ? $variables[$name] : (isset($conf[$name]) ? $conf[$name] : '');
      // drupal_set_message("init $lang var $name = ".$i18n_conf[$name]);
    }
    $conf = array_merge($conf, $i18n_conf);
  }
}

function _i18n_variable_init(){  
  $lang = _i18n_get_lang();
  $variables = array();
  if ($cached = cache_get('variables:'.$lang)) {
    $variables = unserialize($cached->data);
  }
  else {
    $result = db_query("SELECT * FROM {i18n_variable} WHERE language='%s'", $lang);
    while ($variable = db_fetch_object($result)) {
      $variables[$variable->name] = unserialize($variable->value);
    }
    cache_set('variables:'.$lang, serialize($variables));
  }

  return $variables;    

}

function _i18n_variable_exit(){
  global $i18n_conf;
  global $conf;
  $lang = _i18n_get_lang();
  if($i18n_conf){
    $variables = array_diff_assoc($i18n_conf, $conf);
    if($variables){
      var_dump($variables);      
      foreach($variables as $name => $value){
        drupal_set_message("updated $name = $value");
        $i18n_conf[$name] = $conf[$name];
        db_query("DELETE FROM {i18n_variable} WHERE name='%s' AND language='%s'", $name, $lang );
        db_query("INSERT INTO {i18n_variable} (language, name, value) VALUES('%s', '%s', '%s')", $lang, $name, serialize($conf[$name]));
      }
      cache_set('variables:'.$lang, serialize($i18n_conf));
    }
  }
}
 
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

/**
 * @name Themeable functions
 * @{
 */
  
/**
 * Produces a language link
 */
function theme_i18n_link($text, $target, $lang, $separator = ' '){
  $output = '<span class="i18n-link">';
  $attributes = ($lang == i18n_get_lang()) ? array('class' => 'active') : NULL;
  $output .= l(theme('i18n_language_icon', $lang), $target, $attributes, NULL, NULL, FALSE, TRUE);
  $output .= $separator;
  $output .= l($text, $target, $attributes, NULL, NULL, FALSE, TRUE);
  $output .= '</span>';
  return $output;
}
 
function theme_i18n_language_icon($lang){
  if ($path = variable_get('i18n_icon_path', 'modules/i18n/flags/*.png')) {
    $languages = i18n_supported_languages();
    $src = str_replace('*', $lang, $path);
    list($width, $height) = explode('x', variable_get('i18n_flags_size', '16x12'));
	$attribs = array('class' => 'i18n-icon', 'width' => $width, 'height' => $height, 'alt' => $languages[$lang]);
    return "<img src=\"$src\" ".drupal_attributes($attribs)." />";
  }  
}


    
function theme_i18n_links($flags = 1, $names = 1, $delim1 = ' ' , $delim2 = ' ' ) {
  $links = i18n_get_links($flags, $names, $delim1);
  return implode($delim2, $links);
}

function theme_i18n_flags() {
  return theme_i18n_links(1, 0);
}

function theme_i18n_link_name_flag($lang, $path, $attributes = array()) {
  static $languages;
  if (!isset($languages)) {
    $languages = i18n_supported_languages();
  }
  return '<span class="i18n-link">'. l($languages[$lang], $path, $attributes, NULL, NULL, FALSE, TRUE) . '&nbsp;' . l(i18n_flag($lang) , $path, $attributes, NULL, NULL, FALSE, TRUE) .'</span>';
}

/* @} */
?>