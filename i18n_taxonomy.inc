<?php
// $Id$

/**
 * Internationalization (i18n) module
 *
 * @author Jose A. Reyero, 2004
 * 
 * Multilingual Taxonomy support
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
    
    $tree = taxonomy_get_tree($vocabulary->vid);
    if ($tree) {
      $data = array();
       foreach ($tree as $term) {
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
  // debug($edit);
  $languages = i18n_supported_languages();
  foreach ($languages as $lang => $name) {
    $tid = $edit[$lang]['tid'];
    $old = $edit[$lang]['old'];
    if ($tid != $old) {
      $translations[$tid] = ($tid ? true : false);
      $translations[$old] = false;
    }
  }
  if (is_array($translations)) {
    // Save translations
    $trid = $edit['trid'];
    if ($trid =='new') {
      // New translation group
      $trid = db_next_id('{term_data}_trid');
    }
    // Remove old
    if (is_array($remove)) {
      // Remove old translations
    }
    foreach ($translations as $tid => $new) {
      $value = $new ? $trid : 'null';
      db_query('UPDATE {term_data} SET trid=%d WHERE tid=%d', $value, $tid);
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
  $trid= $edit['trid'];
  
  $languages = i18n_supported_languages();
  $selectlang = array_merge(array('' => ''), $languages);
  
  /* Vocabulary translations disabled for now
  if ($trid && $translations=i18n_term_get_translations($tid)) {
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
    $header = array(t('Language'), t('translated term'));
    $output .= form_group(t('term translations'), theme('table', $header, $rows));
  }
  */
  $output = form_select(t('Language'), 'language', $termlang, $selectlang);
  return $output;
}

function i18n_taxonomy_form_vocabulary($edit = array()) {
  $tid = $edit['tid'];
  $lang= $edit['language'];
  $trid= $edit['trid'];
  
  $languages = i18n_supported_languages();
  $selectlang = array_merge(array('' => ''), $languages);
  $output = form_select(t('Language'), 'language', $lang, $selectlang);
  return $output;
}

function i18n_taxonomy_where_sql($table, $type, $param = '') {
  //debug("$table $type $param");
  switch ($type) {
    case 'node':  // Param is nid
      $lang = i18n_node_get_lang($param, i18n_get_lang());
      return "AND $table.language in ('$lang', '')";
    case 'guess': // This is a difficult one :-). Guess from path
      return i18n_taxonomy_where_sql_guess($table);
    default:
      return "AND $table.language in ('". i18n_get_lang() . "','')";
  }
}

function i18n_taxonomy_where_sql_guess($table) {
  switch (arg(0)) {
    case 'node':
      if (is_numeric($nid = arg(1))) {
        $lang = i18n_node_get_lang($nid, i18n_get_lang());
      }
      else {
        $lang = i18n_get_lang();
      }
      break;
      
    case 'admin':
      // Get all languages
  }
  if ($lang) {
    return "AND $table.language in ('$lang','')";
  }
  else {
    return '';
  }
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
  $sql = 'select a.* from {term_data} a, {term_data} b where a.trid = b.trid and b.tid=%d';
  if ($getall) {
    $result = db_query($sql, $tid);
  }
  else {
    $result = db_query($sql. ' and a.tid != %d', $tid);
  }
  while ($data = db_fetch_object($result)) {
    $list[$data->language] = $data;
  }
  return $list;  
}

function i18n_taxonomy_get_translations($trid) {
  $result = db_query('SELECT * FROM {term_data} WHERE trid=%d',$trid);
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
    $andsql = ' AND trid > 0';
  }elseif ( $status =='untranslated' ) {
    $andsql = ' AND (trid = 0 or trid is null)';
  }

  $result = db_query("SELECT * FROM {term_data} WHERE vid=%d AND language='%s' " . $andsql, $vid, $lang);
  $list = array();
  while ($term = db_fetch_array($result)) {
     $list[] = $term;
  }
  return $list;  
}
/**
 * Taxonomy hook
 * $object parameter is an array, not an object !!
 */
 // $op = insert, update, delete
function i18n_taxonomy($op, $type, $edit) {
  if ($type == 'term') {
    switch ($op) {
      case 'insert':
      case 'update':      
        if (!$edit['language'] && $edit['trid']) {
          // Removed language, remove trid
          db_query('UPDATE {term_data} SET trid=0 WHERE tid=%d', $edit['tid']);
          drupal_set_message(t('Removed translation info from term'));
        }
        break;
      case 'form pre':
        return i18n_taxonomy_form_term($edit);
        break;  
      case 'delete':
    }
  } else {
    switch ($op) {
      case 'form pre':
        return i18n_taxonomy_form_vocabulary($edit);
        break;
    }
  }

} 
 
?>