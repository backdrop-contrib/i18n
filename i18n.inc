<?php
// $Id$

/**
 * Conditional definition of conf_url_rewrite
 *
 */

function conf_url_rewrite($path, $mode = 'incoming') {
  if(variable_get('i18n_keep','')=='url' || variable_get('i18n_content',0)){
    return i18n_url_rewrite($path,$mode);
  } else {
    return $path;
  }
} 

function i18n_url_rewrite($path, $mode = 'incoming') {
  global $i18n_langpath;
  if ($mode == 'incoming') { // URL coming from a client
    if($i18n_langpath=i18n_get_lang_prefix($path)){
      // Remove language from path and try to find alias for new path
      $path=substr($path,3);
      return drupal_get_normal_path($path);
    } else {
      return $path;
    }
  }
  else { // URL going out to a client
    if($i18n_langpath){
      return "$i18n_langpath/$path";
    } else {
      return $path;
    }
  }
}

// Get language from path, but only if it is in the $languages array
function i18n_get_lang_prefix($path){
  global $languages;
  $split=split("/",$path);
  $maybelang=$split[0];
  if($languages[$maybelang]){
    return $maybelang;
  }
}

// This is just an experiment to keep tables in sync. 
// Needs some patching to core.
function i18n_db_sync($query){
  global $languages,$i18n_lang;
  global $i18n_db_sync,$db_prefix_i18n_sync;
  if (preg_match("/^(insert into|delete from)[ ]+\{([\w]+)\}.*/i",$query,$matches) && $db_prefix_i18n_sync[$matches[2]]){
    debug("SYNC:".$sql);
    $i18n_db_sync=false;
    foreach($languages as $lang => $desc){
      i18n_set_db_prefix($lang);
      $result[$lang]=_db_query(db_prefix($args));
    }
    i18n_set_db_prefix($locale);
    $i18n_db_sync=true;
  } 
  elseif(preg_match("/^REPLACE INTO \{sequences\}.*/i",$sql)){
  }
}
?>