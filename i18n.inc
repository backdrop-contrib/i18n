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
  
  if ($primary_table != 'n' && !strstr($query, '{node}')) {
    $result['join'] = 'INNER JOIN {node} n USING (nid)';
    $alias = n;
  } 
  $result['where'] = $alias .".language ='".i18n_get_lang()."' or ". $alias .".language = ''" ;

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
    $blocks['content'] = theme('item_list', i18n_get_links());
  }

  return $blocks;
}

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
 * More i18n API
 */

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
    return $i18n_language = _i18n_get_lang();
  }
}

/**
 * Returns main language, two letter code
 */
function i18n_get_main_lang($lang = NULL){
  $lang = $lang ? $lang : i18n_get_lang();
  return substr($lang, 2);
}

/**
 * Function i18n_get_links
 * 
 * Returns an array of links for all languages, with or without names/flags
 */
function i18n_get_links($flags = 1, $names = 1, $separator = ' ') {
  $i18n_lang = i18n_get_lang();
  $languages = i18n_supported_languages();
  foreach ($languages as $lang => $langname) {
    $name = $names ? $langname: '' ; // Should be localized??
    $flag= $flags ? i18n_flag($lang, array('alt'=>$langname)) : '' ;
    $links[]= i18n_l("$flag$separator$name", $lang);
  }
  return $links;  
}

/**
 * Hook for rewriting URLs
 */
function i18n_url_rewrite($path, $mode ='outgoing') {
   if ($mode == 'outgoing' && !i18n_get_lang_prefix($path) ){
     return $path ? i18n_get_lang() . '/'. $path : i18n_get_lang();
   } else {
     return $path;
   }

}  // End of i18n_url_rewrite

/**
 * Links for different languages
 */

function i18n_l($text, $lang , $url = '' , $attributes = array(), $query = NULL) {
  global $i18n_langpath;
  // If !url get from original request
  if (!$url) {
    $url = _i18n_get_original_path();
  }
  // If url has lang_prefix, remove it
  i18n_get_lang_prefix($url, true);
  //are we looking at a node?
  if (preg_match("/^(node\/)([0-9]*)$/",$url,$matches)) {
    if ($nid = i18n_node_l($matches[2], $lang)) {
      $url = "node/$nid";
    }
  }
  elseif (preg_match("/^(taxonomy\/term\/)([^\/]*)$/",$url,$matches)) {//or at a taxonomy-listing?
    if ($str_tids = i18n_taxonomy_l($matches[2], $lang)) {
      $url = "taxonomy/term/$str_tids";
    }
  }
  
  return '<a href="'. i18n_url($url, $lang, $query) .'"'. drupal_attributes($attributes) .'>'. $text .'</a>';
}

function i18n_url($url, $lang, $query = NULL) {
  // Strip prefix if default language
  if ($url) {
    return url($lang.'/'.$url, $query);
  } else {
    return url($lang, $query);
  }
}

function i18n_flag($lang, $attribs = array()) {
  if ($path = variable_get('i18n_flags_path', 'modules/i18n/flags/*.png')) {
    $src = str_replace('*', $lang, $path);
    list($width, $height) = explode('x', variable_get('i18n_flags_size', '16x12'));
		$attribs = array_merge(array('class' => 'i18n-flag', 'width' => $width, 'height' => $height, 'alt' => 'Language Icon'), $attribs);
    $html = "<img src=\"$src\" ".drupal_attributes($attribs)." />";
    return $html;
  }
}

/**
 * Multilingual Nodes support
 */

// translation/node/id/lang
// translation/term/id

function i18n_translation_page() {
  $op = $_POST['op'] ? $_POST['op'] : arg(1);
  $edit = $_POST['edit'];
  
  switch ($op) {
    case 'node':
      print theme('page', i18n_translation_add(arg(2), arg(3)));
      break;
      
     case t('Preview'):
      $edit = node_validate($edit);
      print theme('page', node_preview($edit), t('Preview'));
      break;
      
    case t('Submit'):
      drupal_set_title(t('Submit'));
      print theme('page', node_submit($edit));
      break;
      
    case t('Delete'):
      print theme('page', node_delete($edit), t('Delete'));
      break;
      
    default:
      // print theme('page', node_page_default(), '');
  }      
}

function i18n_translation_add($nid, $lang) {
  $type = i18n_get_node_type($nid);
  return node_add($type);
}

/**
 * This is the callback for the tab 'translations' for nodes
 */
function i18n_node_translation() {
  $op = $_POST['op'];
  $edit = $_POST['edit'];

  if (empty($op)) {
    $op = arg(2);
  }
  
  switch ($op) {
    case 'translation':
      $output = i18n_node_translation_status(arg(1));
      break;
  }
  print theme('page', $output);  
}

function i18n_node_translation_status($nid) {
  $languages = i18n_supported_languages();
  $translations = i18n_node_get_translations($nid, true);
  if (!$translations) {
    $node = node_load(array('nid' => $nid));
    $translations[$node->language]= $node;
  }
  foreach ($languages as $lang => $name) {
    if (array_key_exists($lang, $translations)) {
      $node = $translations[$lang];
      $rows[] = array($name, l($node->title, 'node/'.$node->nid), l(t('edit'), 'node/'.$node->nid.'/edit'));
      if ($node->nid == $nid) {
        drupal_set_title(t('Translations of ').$node->title);
      }
    }
    else {
      $rows[] = array($name, t('Not translated'), l(t('create translation'), 'translation/node/'.$nid.'/'.$lang));
  }
  }
  $header = array(t('Language'), t('Title'), t('Actions'));
  $output .= t('<h2>Translation Status</h2>') .'<br />';
  $output .= theme('table', $header, $rows);
  return $output;
}

/**
 * Hook nodeapi
 */
function i18n_nodeapi(&$node, $op, $arg = 0) {

  if (variable_get("i18n_node_$node->type", 0)) {
    switch ($op) {
      case 'fields':
        return array('language');
        break;
        
      case 'validate':
        break;
        
      case 'insert':
        if ( arg(0) == 'translation' ) {
          i18n_node_create_translation($node->nid, $node->language, $node->i18n_snid);
        }
        break;
  
      case 'delete':
        db_query('DELETE FROM {i18n_node} WHERE nid=%d', $node->nid);
        break;
        
      case 'update':
        // Check language changed?
        break;
        
      case 'form pre':
        $languages = i18n_supported_languages();
        if ($node->nid) {
          $translations = i18n_node_get_translations($node->nid);
        }
        elseif (arg(0) == 'translation') {
          // We are translating a node
          $snid = arg(2);
          $node->language = arg(3);
          $form = form_hidden('i18n_snid', $node->i18n_snid ? $node->i18n_snid : $snid);
          // Load the node to be translated  
          $trans = node_load(array('nid' => $snid));
          foreach ($trans as $field=>$value) {
            if ($field != 'nid' && empty($node->$field)) {
              $node->$field = $value;
            }
          }          
          $translations = i18n_node_get_translations($snid, true);

          if (!$translations) {
            $translations[$trans->language]= $trans;
          }
        }

        if ($translations) {
          foreach ($languages as $lang => $langname) {
            if ($trans = $translations[$lang]) {
              $rows[] = array($langname, l($trans->title, 'node/'.$trans->nid.'/edit'));
            }
            else {
              $langselect[$lang]= $langname;
            }
          }
        }
        else {
          $langselect = $languages;
        }
        
        $langselect = array_merge(array('' => ''), $langselect);
        $form .= form_select(t('Language'), 'language', $node->language ? $node->language : i18n_get_lang(), $langselect, t('If you change the Language, you must click on <i>Preview</i> to get the right Categories &amp; Terms for that language.'));
        if ($rows) {
           $header = array(t('Language'), t('Title'));
           $form .= form_group(t('translations'), theme('table', $header, $rows));
        }
    
        return $form;
        break;
    }
  }
}

function i18n_node_available_langs($node) {
  $languages = i18n_supported_languages();
  if ($node->nid) {
    $usedlangs = i18n_node_get_translations($node->nid);
  }
  elseif ($node->i18n_snid) {
    $usedlangs = i18n_node_get_translations($node->i18n_snid, true);
  }
  else {
    return $languages;
  }
  
  foreach ($languages as $lang=>$name) {
    if (!array_key_exists($lang, $usedlangs)) {
       $newlangs[$lang] = $name;
    }
  }
   
  return $newlangs;
    
}

function i18n_node_get_lang($nid, $default = '') {
  $lang = db_result(db_query('SELECT language FROM {node} WHERE nid=%d',$nid));
  return $lang ? $lang : $default ;
}

/**
	*	getall = true to get the also node itself
	*/

function i18n_node_get_translations($nid, $getall = false) {
  $sql = 'SELECT n.nid, n.language, n.title, a.trid FROM {node} n INNER JOIN {i18n_node} a ON n.nid = a.nid INNER JOIN {i18n_node} b ON a.trid = b.trid AND b.nid =%d';
  if ($getall) {
    $result = db_query($sql, $nid);
  }
  else {
    $result = db_query($sql . ' WHERE n.nid != %d', $nid, $nid);
  }
  $items = array();
  while ($node = db_fetch_object($result)) {
      $items[$node->language] = $node;
  }
  return $items;
}

/* returns an url-part, pointing to the translated node, if exists */
function i18n_node_l($nid, $lang) {
  $sql = 'SELECT n.nid FROM {node} n INNER JOIN {i18n_node} a ON n.nid = a.nid INNER JOIN {i18n_node} b ON a.trid = b.trid AND b.nid =%d WHERE n.nid != %d AND n.language = \'%s\'';
  $result = db_fetch_object(db_query($sql, $nid, $nid, $lang));
  return $result->nid;
}

// snid = source nid
function i18n_node_create_translation($nid, $lang, $snid) {
  $trid = db_result(db_query('SELECT trid FROM {i18n_node} WHERE nid=%d', $snid));
  if (!$trid) {
    $trid = db_next_id('{i18n_node}_trid');
    db_query('INSERT INTO {i18n_node} (trid, nid) VALUES (%d, %d)', $trid, $snid);
  }
  db_query('INSERT INTO {i18n_node}(trid, nid) VALUES (%d, %d)', $trid, $nid);
}

function i18n_get_node_type($nid) {
  return db_result(db_query('SELECT type FROM {node} WHERE nid=%d', $nid));
}

/**
 * Multilingual Taxonomy
 *
 */
 
/**
 * This is the callback for taxonomy translations
 * 
 * Gets the urls:
 * 		admin/taxonomy/i18n/term/xx
 * 		admin/taxonomy/i18n/term/new/xx
 */

function i18n_taxonomy_admin() {
  $op = $_POST['op'] ? $_POST['op'] : arg(3);
  $edit = $_POST['edit'];
  switch ($op) {
     case 'term':
      print theme('page', i18n_taxonomy_term_form(arg(4), $edit), t('Term Translations'));
      break;
      
     case t('Submit'):
      drupal_set_title(t('Submit'));
      i18n_taxonomy_term_save($edit);
      print theme('page', i18n_taxonomy_overview(), t("Term Translations"));
      break;
      
    case t('Delete'):
      //print theme('page', node_delete($edit), t('Delete'));
      break;
      
    default:
      print theme('page', i18n_taxonomy_overview(), t("Term Translations"));
  }      
}  
    
/**
 * Generate a tabular listing of translations for vocabularies.
 */

function i18n_taxonomy_overview() {
  $languages = i18n_supported_languages();
  $header = array_merge($languages, array(''));
  $vocabularies = taxonomy_get_vocabularies();
  //rows[] = array();
  foreach ($vocabularies as $vocabulary) {
    $links = array();
    $types = array();
 
    $rows[] = array(array('data' => $vocabulary->name, 'colspan' =>  count($languages), 'align' => 'center'), 
        l(t('new translation'), 'admin/taxonomy/i18n/term/new/'.$vocabulary->vid));
    // Get terms/translations for this vocab 
    $result = db_query('SELECT t.*,i.trid FROM {term_data} t LEFT JOIN {i18n_taxonomy_term} i USING(tid) WHERE t.vid=%d',$vocabulary->vid);
    $list = array();
    while ($data = db_fetch_object($result)) {
      $list[] = $data;
    }
    if (!empty($list)) {
      $data = array();
       foreach ($list as $term) {
        if ($term->language) {
           if ($term->trid) {
             if (!array_key_exists($term->trid, $data)) {
               $data[$term->trid] = array();
             }
             $data[$term->trid][$term->language]= $term;
           }
           else {
             $untranslated[$term->language][] = $term;
           }
        }
      }
      // Reorder data for rows and languages
      foreach ($data as $trid => $terms) {
        $thisrow = array();
        foreach ($languages as $lang => $name) {
          if (array_key_exists($lang, $terms)) {
            $thisrow[] = $terms[$lang]->name;
           }
           else {
            $thisrow[] = '--';
           }
        }
        $thisrow[] = l(t('edit translations'), 'admin/taxonomy/i18n/term/'.$trid);
        $rows[] = $thisrow;
      }
    }
  }

  return theme('table', $header, $rows);
}

function i18n_taxonomy_term_save($edit) {
  $languages = i18n_supported_languages();
  $translations = array();
  foreach ($languages as $lang => $name) {
    $tid = $edit[$lang]['tid'];
    $old = $edit[$lang]['old'];
    if ($tid) {
      $translations[] = $tid;
    }
  }
  $trid = $edit['trid'];
  // Remove old translations
  if (is_numeric($trid)){
    db_query('DELETE FROM {i18n_taxonomy_term} WHERE trid=%d',$trid);  
  }
  // Save new translations
  if (!empty($translations)) {
    // Save translations
    if ($trid =='new') {
      // New translation group
      $trid = db_next_id('{i18n_taxonomy_term}_trid');
    } 
    foreach ($translations as $tid ) {
      //$value = $new ? $trid : 'null';
      //db_query('UPDATE {i18n_taxonomy_term} SET trid=%d WHERE tid=%d', $value, $tid);
      db_query('INSERT INTO {i18n_taxonomy_term}(trid,tid) VALUES(%d,%d)',$trid,$tid);

    }
    drupal_set_message(t('Term translations have been updated'));
  }

}
function i18n_taxonomy_term_form($trid) {
  $languages = i18n_supported_languages();
  if ($trid == 'new') {
    $translations = array();
    $vid = arg(5);
  }
  else {
    $translations =i18n_taxonomy_get_translations($trid);
    $vid = $translations[key($translations)]->vid;
  } 
  $vocabulary = taxonomy_get_vocabulary($vid);
  
  // List of terms for languages
  foreach ($languages as $lang => $langname) {
    if ($trans = $translations[$lang]) {
      $field1 = l($trans->name, 'admin/taxonomy/edit/term/'.$trans->tid);
      $tidtrans = $trans->tid;
     }
     else {
      $field1 = "--";
      $tidtrans = 0;
    }
    // Available terms for this language
    $list = i18n_array2list(i18n_vocabulary_get_terms($vid, $lang, 'all'), 'tid', 'name');
    $list[0] ='';
    $field2 = form_select('', 
          "$lang][tid", 
          $tidtrans, 
          $list);
    $field2 .= form_hidden("$lang][old", $tidtrans);
    $rows[] = array($langname, $field1, $field2);
  }
  $header = array(t('Language'), t('current translation'), t('change translation'));
  // Prepares html form
  $output = form_item(t("Vocabulary"), $vocabulary->name);
  $output .= form_hidden('trid', $trid);
  $output .= theme('table', $header, $rows);
  $output .= form_submit(t('Submit'));
  return form($output);
}

/**
 * Adds language and translation fields to term edition form
 * 
 * This is a hook for taxonomy module
 */

function i18n_taxonomy_form_term($edit = array()) {
  $tid = $edit['tid'];
  $termlang= $edit['language'];
  $vid = isset($edit['vid']) ? $edit['vid'] : arg(4);
  // $trid= $edit['trid'];
  
  $languages = i18n_supported_languages();
  $selectlang = array_merge(array('' => ''), $languages);

  $vocabulary = taxonomy_get_vocabulary($vid);
  // If vocabulary has language, this should be the one for the term
  if ($vocabulary->language) {
    $edit['language'] = $vocabulary->language;
    $output = form_item(t('Language'),$languages[$vocabulary->language],t('The vocabulary already has a language.'));
  } 
  elseif ($termlang && $translations=i18n_term_get_translations($tid)) {
    // Term has translations. Not all languages are valid
    $selectlang = array('' => '');
    foreach ($languages as $lang => $langname) {
      if ($trans = $translations[$lang]) {
        $rows[] = array($langname, l($trans->name, 'admin/taxonomy/edit/term/'.$trans->tid));  
      }
      else {
        $selectlang[$lang] = $langname;
      }
    }
    $output = form_select(t('Language'), 'language', $termlang, $selectlang);

    $header = array(t('Language'), t('translated term'));
    $output .= form_group(t('term translations'), theme('table', $header, $rows));
    
  } else {
    $output = form_select(t('Language'), 'language', $termlang, $selectlang);
  }
  return $output;
}

function i18n_taxonomy_form_vocabulary($edit = array()) {
  $tid = $edit['tid'];
  $lang= $edit['language'];
  $trid= $edit['trid'];
  
  $languages = i18n_supported_languages();
  $selectlang = array_merge(array('' => ''), $languages);
  $output = form_select(t('Language'), 'language', $lang, $selectlang, t('This language will be set for all terms in this vocabulary'));
  return $output;
}

/* Returns an url for the translated taxonomy-page, if exists */
function i18n_taxonomy_l($str_tids, $lang) {
  if (preg_match('/^([0-9]+[+ ])+[0-9]+$/', $str_tids)) {
    $separator = '+';
    // The '+' character in a query string may be parsed as ' '.
    $tids = preg_split('/[+ ]/', $str_tids);
  }
  else if (preg_match('/^([0-9]+,)*[0-9]+$/', $str_tids)) {
    $separator = ',';
    $tids = explode(',', $str_tids);
  }
  else {
    return;
  }
  
  $translated_tids = array();
  foreach ($tids as $tid) {
    if ($translated_tid = i18n_term_get_translations($tid)) {
      $translated_tids[] = $translated_tid[$lang]->tid;
    }
  }
  
  return implode($separator, $translated_tids);
}

/**
 * Converts a list of arrays to an array of the form keyfield => namefield
 */
 
function i18n_array2list($data, $keyfield, $namefield = 'name') {
  foreach ($data as $key => $value) {
    if (is_array($data)) {
      $list[$value[$keyfield]] = $value[$namefield];
    }
    else {
      $list[$value->$keyfield] = $value->$namefield;
    }
  }
  return $list;
}

// returns array lang > term
function i18n_term_get_translations($tid, $getall = false) {
  $sql = 'SELECT t.* FROM {term_data} t INNER JOIN {i18n_taxonomy_term} i USING(tid) INNER JOIN {i18n_taxonomy_term} i2 ON  i.trid = i2.trid WHERE i2.tid=%d';
  if ($getall) {
    $result = db_query($sql, $tid);
  }
  else {
    $result = db_query($sql. ' AND t.tid != %d',$tid, $tid);
  }
  while ($data = db_fetch_object($result)) {
    $list[$data->language] = $data;
  }
  return $list;  
}

function i18n_taxonomy_get_translations($trid) {
  $result = db_query('SELECT t.*,i.trid FROM {term_data} t JOIN {i18n_taxonomy_term} i USING(tid) WHERE i.trid=%d',$trid);
  $list = array();
  while ($data = db_fetch_object($result)) {
    $list[$data->language] = $data;
  }
  return $list;     
}

function i18n_db_get_arrays($sql) {
  $result = db_query($sql);
  $list = array();
  while ($data = db_fetch_array($result)) {
    $list[] = $data;
  }
  return $list;  
}

function i18n_db_get_objects($sql) {
  $result = db_query($sql);
  $list = array();
  while ($data = db_fetch_object($result)) {
    $list[] = $data;
  }
  return $list;    
}
// returns array lang > term
function i18n_vocabulary_get_terms($vid, $lang, $status = 'all') {
  if ($status=='translated') {
    $andsql = ' AND i.trid > 0';
  }elseif ( $status =='untranslated' ) {
    $andsql = ' AND (i.trid = 0 or i.trid is null)';
  }

  $result = db_query("SELECT t.*, i.trid FROM {term_data} t LEFT JOIN {i18n_taxonomy_term} i USING(tid) WHERE t.vid=%d AND t.language='%s' " . $andsql, $vid, $lang);
  $list = array();
  while ($term = db_fetch_array($result)) {
     $list[] = $term;
  }
  return $list;  
}
/**
 * Taxonomy hook
 * $edit parameter is an array, not an object !!
 */
 // $op = insert, update, delete
function i18n_taxonomy($op, $type, $edit) {
  switch ("$type/$op") {
    case 'term/insert':
    case 'term/update':      
	      if (!$edit['language']) {
	        // Removed language, remove trid
	        db_query('DELETE FROM {i18n_taxonomy_term} WHERE tid=%d', $edit['tid']);
	        drupal_set_message(t('Removed translation info from term'));
	      }
        break;
    case 'term/form pre':
        return i18n_taxonomy_form_term($edit);
        break;  
    case 'term/delete':
	      db_query('DELETE FROM {i18n_taxonomy_term} WHERE tid=%d', $edit['tid']);
        break;
    case 'vocabulary/form pre':
        return i18n_taxonomy_form_vocabulary($edit);
        break;
    case 'vocabulary/update':
        if (isset($edit['language'])) {
          db_query("UPDATE {term_data} SET language='%s' WHERE vid='%d'",$edit['language'],$edit['vid']);
        }
        break;
  }
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