<?php
// $Id$

/**
 * Internationalization (i18n) module
 *
 * @author Jose A. Reyero, 2004
 * 
 * Multilingual Nodes support
 */

/**
 * Implementation of hook_node_rewrite_sql()
 */ 
function i18n_node_rewrite_sql($hint,$node_alias){
  $result = array();
  if ($hint & NODE_REWRITE_SQL_RELATED_LIST) {
    $result['join'] = "INNER JOIN {node} USING(nid)";
    $result['where'] = "{node}.language='".i18n_get_lang()."'";
  } elseif ( $hint & NODE_REWRITE_SQL_SEARCH) {
    
  } else {
    $result['where'] = "$node_alias.language ='".i18n_get_lang()."'" ;
  }
  return $result;
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
        $form .= form_select(t('Language'), 'language', $node->language ? $node->language : i18n_get_lang(), $langselect);
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
  //debug($items);
  return $items;
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

function i18n_node_where_sql() {
  return " AND ( n.language='" . i18n_get_lang(). "' OR n.language='')";
}

function i18n_get_node_type($nid) {
  return db_result(db_query('SELECT type FROM {node} WHERE nid=%d', $nid));
}

?>