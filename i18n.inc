<?php
// $Id$

/**
 * Conditional definition of conf_url_rewrite
 *
 */

function conf_url_rewrite($path, $mode = 'incoming') {
		if(variable_get('i18n_keep','')=='url'){
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
?>