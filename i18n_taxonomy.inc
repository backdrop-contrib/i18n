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
  // debug($edit);
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
 * $object parameter is an array, not an object !!
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
 
?>