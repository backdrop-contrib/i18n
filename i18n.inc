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
  //drupal_set_message("type=$type path=$path original=$original");
  if ($type == 'alias' && !i18n_get_lang_prefix($path) ){
    return $path ? i18n_get_lang() . '/'. $path : i18n_get_lang();
  } else {
    return $path;
  } 
} 

/**
 * Implementation of hook_db_rewrite_sql()
 */
function i18n_db_rewrite_sql($query, $primary_table, $primary_key){
  // Some exceptions for query rewrites
  $mode = i18n_selection_mode();
  // drupal_set_message("i18n_db_rewrite mode=$mode query=$query");
  if($mode == 'off') return;
  
  switch ($primary_table) {
    case 'n':
      // Node queries
      return i18n_db_node_rewrite($query, $primary_table, $primary_key, $mode);
    case 't':
    case 'v':
      // Taxonomy queries
      return i18n_db_taxonomy_rewrite($query, $primary_table, $primary_key, $mode);
  }
}

function i18n_db_node_rewrite($query, $primary_table, $primary_key, $mode){
  // When loading specific nodes, language conditions shouldn't apply
  // TO-DO: Refine this regexp  
  if (preg_match("/WHERE.*$primary_table.nid\s*=\s*(\d|%d)/", $query)) return;
  
  $result['join'] = "LEFT JOIN {i18n_node} i18n ON $primary_table.nid = i18n.nid";
  $result['where'] = i18n_db_rewrite_where('i18n', $mode);
  
  return $result;
}

function i18n_db_taxonomy_rewrite($query, $primary_table, $primary_key, $mode){
  // When loading specific terms, vocabs, language conditions shouldn't apply
  // TO-DO: Refine this regexp  
  if (preg_match("/WHERE.* $primary_table\.tid\s*(=\s*\d|IN)/", $query)) return;
  
  $result['where'] = i18n_db_rewrite_where($primary_table, $mode);
 
  return $result;
}

function i18n_db_rewrite_where($alias, $mode){
  switch($mode){
    case 'simple':
      return "$alias.language ='".i18n_get_lang()."' OR $alias.language =''" ;
    case 'mixed':
      return "$alias.language ='".i18n_get_lang()."' OR $alias.language ='".i18n_default_language()."' OR $alias.language =''" ;
    case 'strict':
      return "$alias.language ='".i18n_get_lang()."'" ;
    case 'node':
    case 'translation':
      return "$alias.language ='".i18n_selection_mode('params')."' OR $alias.language =''" ;
    case 'default':
      return "$alias.language ='".i18n_default_language()."' OR $alias.language =''" ;
    case 'custom':
      return str_replace('%alias',$alias, i18n_selection_mode('params'));
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
 * Alan: - changed to test in case translation_form_alter (or another module/mechanism) has already set language
 *       - translation module may reduce language selection options in case there already exist translations
 */
function i18n_form_alter($form_id, &$form) {
  //drupal_set_message("i18n_form_alter form_id=$form_id ");
  switch($form_id){
    case 'taxonomy_overview_vocabularies':
      $vocabularies = taxonomy_get_vocabularies();
      $languages = i18n_supported_languages();
      foreach ($vocabularies as $vocabulary) {
        if($vocabulary->language) $form[$vocabulary->vid]['type']['#value'] = $form[$vocabulary->vid]['type']['#value'].'&nbsp('.$languages[$vocabulary->language].')';
      }
    	break;
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
	    // Node edit form
		  if (isset($form['type']) && $form['type']['#value'] .'_node_form' == $form_id){
	      // Language field
	      if(variable_get('i18n_node_'.$form['type']['#value'], 0) && !isset($form['i18n']['language'])) {
          // Language field
          $form['i18n'] = array('#type' => 'fieldset', '#title' => t('Language'), '#collapsible' => TRUE, '#collapsed' => FALSE, '#weight' => -4);
          // Language will default to current only when creating a node
	        $language = isset($form['#node']->language) ? $form['#node']->language : (arg(1)=='add' ? i18n_get_lang() : '');
	        $form['i18n']['language'] = _i18n_language_select($language, t('If you change the Language, you must click on <i>Preview</i> to get the right Categories &amp; Terms for that language.'), -4);
	      }
	      // Correction for lang/node/nid aliases generated by path module
				// if($form['#node']->path && $form['#node']->path == i18n_get_lang().'/node/'.$form['#node']->nid){
	      if($form['#node']->path) {
	        $alias = drupal_lookup_path('alias', 'node/'.$form['#node']->nid);
	        if($alias && $alias != 'node/'.$form['#node']->nid){
	          $form['#node']->path = $alias;
	        } else {
	          unset($form['#node']->path);
	        }
	      }
        // Some language values for node forms
        // To-do: addapt for translations too
        /*
        if($language && $form['#node']->type == 'book') {
          i18n_selection_mode('custom', "%alias.language ='$language' OR %alias.language IS NULL" );
          $form['parent']['#options'] = book_toc($form['#node']->nid);
          i18n_selection_mode('reset');
        }
        */

	  }
    
  }    
}

/**
 * Implementation of hook_nodeapi
 * Updated for new table i18n_node
 */
function i18n_nodeapi(&$node, $op, $teaser = NULL, $page = NULL) {
  if (variable_get("i18n_node_$node->type", 0)) {
	  switch ($op) {
      case 'load':
        return db_fetch_array(db_query("SELECT trid, language FROM {i18n_node} WHERE nid=%d", $node->nid));
      case 'insert':
      case 'update':
        db_query("DELETE FROM {i18n_node} WHERE nid=%d",$node->nid);
        if($node->language){
          db_query("INSERT INTO {i18n_node} (nid, trid, language) VALUES(%d, '%d', '%s')", $node->nid, $node->trid, $node->language);
        }
        break;
      case 'delete': 
        db_query('DELETE FROM {i18n_node} WHERE nid=%d', $node->nid);
        break;
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
	  '#weight' => $weight,
  );
}

/**
 * Implementation of hook_taxonomy
 * 
 * $edit parameter may be an array or an object !!
 */
function i18n_taxonomy($op, $type, $edit = NULL) {
  $edit = (array)$edit;
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
	      db_query("UPDATE {term_data} t SET t.language='%s' WHERE t.vid=%d", $edit['language'], $edit['vid']);
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

/**
 * Produces i18n paths, with language prefix
 * If path is empty or site frontpage, path = 'lang'
 * Check for frontpage and search for alias before adding language
 */
function i18n_path($path, $lang) {
  if (!$path || $path == i18n_frontpage($lang)) {
    return $lang;
  } elseif($alias = drupal_lookup_path('alias', $path)) {
	  if($prefix = i18n_get_lang_prefix($alias)) {
      // This alias will be valid only if it has the same language
	    return ($prefix == $lang) ? $alias : $lang.'/'.$path;
	  } else { // Alias without language prefix
	    return $lang.'/'.$alias;
	  }
  } else { // Alias for language path will be searched later
    return $lang.'/'.$path;  
  } 
}

function i18n_node_get_lang($nid, $default = '') {
  $lang = db_result(db_query('SELECT language FROM {i18n_node} WHERE nid=%d',$nid));
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
function i18n_get_links($path = '') {
  if($path == i18n_frontpage()) {
    $path = '';
  }
  foreach(i18n_supported_languages() as $lang => $name){
    $links[$lang]= theme('i18n_link', $name, i18n_path($path, $lang), $lang);
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
      foreach($variables as $name => $value){
        $i18n_conf[$name] = $conf[$name];
        db_query("DELETE FROM {i18n_variable} WHERE name='%s' AND language='%s'", $name, $lang );
        db_query("INSERT INTO {i18n_variable} (language, name, value) VALUES('%s', '%s', '%s')", $lang, $name, serialize($conf[$name]));
      }
      cache_set('variables:'.$lang, serialize($i18n_conf));
    }
  }
}
 
/**
 * @name Themeable functions
 * @{
 */
  
/**
 * Produces a language link with the right flag
 */
function theme_i18n_link($text, $target, $lang, $separator='&nbsp;'){
  $output = '<span class="i18n-link">';
  $attributes = ($lang == i18n_get_lang()) ? array('class' => 'active') : NULL;
  $output .= l(theme('i18n_language_icon', $lang), $target, $attributes, NULL, NULL, FALSE, TRUE);
  $output .= $separator;
  $output .= l($text, $target, $attributes, NULL, NULL, FALSE, TRUE);
  $output .= '</span>';
  return $output;
}
 
function theme_i18n_language_icon($lang){
  if ($path = variable_get('i18n_icon_path', drupal_get_path('module', 'i18n').'/flags/*.png')) {
    $languages = i18n_supported_languages();
    $src = base_path().str_replace('*', $lang, $path);
    list($width, $height) = explode('x', variable_get('i18n_icon_size', '16x12'));
    $attribs = array('class' => 'i18n-icon', 'width' => $width, 'height' => $height, 'alt' => $languages[$lang]);
    return "<img src=\"$src\" ".drupal_attributes($attribs)." />";
  }  
}

/* @} */
?>