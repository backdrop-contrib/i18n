<?php
// $Id$

/**
 * conf_url_rewrite
 *
 * Use this function to keep language info in path
 */

function conf_url_rewrite($path, $mode = 'incoming') {
  static $incoming;
  if ( variable_get('i18n_keep','')=='url' || variable_get('i18n_content',0)) {
    if ($mode == 'outgoing') {// URL going out to a client
      return i18n_url_rewrite_out($path);
    }
    elseif(!$incoming) { 
      // URL coming from a client; Incoming urls are only proccessed the first time
      $incoming = true;
      return i18n_url_rewrite_in($path);
    }
    else {
      return $path;
    } 
  } 
  else {
    return $path;
  }
} 

/**
 * Gets language from path and tries to get translated path
 */
function i18n_url_rewrite_in($path) {
  global $i18n_langpath;
  if (function_exists('debug')) debug($path);
  if($i18n_langpath=i18n_get_lang_prefix($path)){
    // Remove language from path and try to find alias for new path
    $path=substr($path,3);
    if($trpath =i18n_get_normal_path($path) ){
      return $trpath;
    }
    else {
      return $path;
    }
  } else {
    return $path;
  }

}

function i18n_url_rewrite_out($path) {
  global $i18n_langpath;  
  // When creating a node, path="node/view/", do not set language path (?)
  if($path=='node/view/'){
    return null;
  // Only adds langpath when it already exists
  }
  elseif($i18n_langpath){
    return "$i18n_langpath/$path";
  } 
  else {
    return $path;
  }
}

/**
 * Get language code from path.
 *
 * It doesn't check anymore for valid languages
 */

function i18n_get_lang_prefix($path){
  if(preg_match("/^\w\w($|\/.*)/",$path) ){
    return substr($path,0,2);
  }
}

/**
 * Adaptation of 'common.inc::drupal_get_normal_path'
 * For not to call conf_url_rewrite again
 */

function i18n_get_normal_path($path) {
  if (($map = drupal_get_path_map()) && isset($map[$path])) {
    return $map[$path];
  }
}

?>