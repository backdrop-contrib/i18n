README.txt
==========

********************************************************************
  This is i18n module, version 4.6, and works with Drupal 4.6.x
********************************************************************
WARNING: It is not 100% backwards compatible with the old i18n module [4.5.x] 
WARNING: DO READ THE INSTALL FILE
********************************************************************

This module implements multilingual support as outlined in http://drupal.org/node/11051
Some more info about this module will be available here: http://reyero.net/en/drupal/i18n

It doesn't require anymore multiple language tables as previous versions

This module provides support for internationalization of Drupal sites:
    * Multilingual content, some basic translation interface, and links between translated versions
    * Translation of the user interface for registered and anonymous users (with locale)
    * Detection of the brower language
    * Keeps the language settings accross consecutive requests using URL rewriting.
    * Provides a block for language selection and two theme functions: i18n_flags and i18n_links

To have a language selector on your page, you can use the block provided or these theme functions:

 theme("i18n_flags") -> Adds just a row with the flags
 theme("i18n_links",$flags,$names,$delim1,$delim2) -> Check documentation in the code for different options

Multilingual content:
=====================
Multilingual content means providing content translated to different languages or language specific content, which is not the same as interface translation. Interface translation is done through Drupal's localization system. 
This module supports:
  - Multilingual nodes
  - Multilingual taxonomy vocabularies and terms
  - Basic translation management for nodes and terms

When you navigate the site using multiple languages, the pages will just show terms and nodes for the chosen language plus the ones that haven't a definde language. 
When editing a node, you must click on 'Preview' after changing language for the right vocabularies and terms to be shown.

The multi language support is expected to work for all node types, and node listings in Drupal 4.6!!

So far, I have not found incompatibilities with any other module. Please, let me know if you find any.

And yes, flexinode works with multiple languages :-)

Taxonomy translation:
====================
You can create vocabularies and terms with or without language. 
- If you set language for a vocabulary/term, that term will just show up for pages in that language
- If you set language for a vocabulary, all the terms in that vocabulary will be assigned that language.
- When editing nodes, if you change the language for a node, you have to click on 'Preview' to have the right vocabularies/terms for that language. Otherwise, the language/taxonomy data for that node could be inconsistent.
  
About URL aliasing with language codes -requires path module
====================================
Incoming URL's are now translated following these steps:
1. First, a translation is searched for path with language code: 'en/mypage'
2. If not found, language code is removed, and path translation is searched again: 'mypage'
Thus, you can define aliases with or without language codes in them

The 'Front page: Language dependent' option means that when the request is for the front page '/', a language prefix will be added before doing the path translation, and then -> step 1 above
This language code will be taken from browser if enabled 'Browser language detection, or will be the default otherwise.

To have aliases for a translated node/page, you have to define each of them. I.e.:
  en/mycustompath -> node/34 (which is suppossed to be the english version)
  es/mycustompath -> node/35 (which should be the spanish version)

For outgoing URL's, the language code will be added authomatically.

About language dependent variables:
======================
Some site-wide variables, like 'site_name', 'site_slogan', user e-mail contents... have language dependent content.
Since I don't like the solution of runing them through the localization system, because this means when you change the 'master' text, you have to re-translate it for every language, I've added this new feature which makes possible to have a list of variables -defined in the config file- which will be kept separated for each language.
This part is an add-on, and you can use it or not.

About language dependent tables 
===============================
Language dependent tables are not needed anymore for multilingual content.
This is kept for backwards compatibility, experimentation and may be some use in the future.
* This can be used to have per-language data for modules not language-aware, like language statistics... you can experiment...

Known problems, compatibility
=============================
- Taxonomy patch not compatible with taxonomy_access patch. See http://drupal.org/node/22834

Sample sites, using this module - e-mail me to be listed here
==========================================================
  http://www.reyero.net

Additional Support
=================
For support, please create a support request for this module's project: 
  http://drupal.org/project/i18n

If you need professional support, contact me by e-mail: freelance at reyero dot net

====================================================================
Jose A. Reyero, drupal at reyero dot net, http://www.reyero.net

Feedback is welcomed.