README.txt
==========

********************************************************************
  This is the new i18n module. Beta 1
********************************************************************
This is an 'advanced module', requires core and database patching and has some low level configuration options.
Thus, it is only intended for Drupal advanced users and administrators.
********************************************************************
Current version of this module works with Drupal 4.5.1.
May work also with 4.5.0, waiting for some feedback about this.

********************************************************************
WARNING: It is not backwards compatible with the old i18n module [4.5.0] 
WARNING: DO READ THE INSTALL FILE
WARNING: If updating from previous versions, see CHANGELOG.txt
********************************************************************

This module implements multilingual support as outlined in http://drupal.org/node/11051

It doesn't require anymore multiple language tables as previous versions

This module provides support for internationalization of Drupal sites:
    * Multilingual content, some basic translation interface, and links between translated versions
	  ** You can choose which node types to translate and a new 'language' field will show up when editing them
      ** Language can also be set for taxonomy terms
	* Translation of the user interface for registered and anonymous users (with locale module and the languages block enabled)
    * Detection of the brower language
    * Keeps the language settings accross consecutive requests using URL rewriting.
    * Provides a block for language selection and two theme functions: i18n_flags and i18n_links
    * Independent interface and content languages. 
	  ** This means you can have the interface -menus, etc..- in english while viewing a node in spanish
	  ** While some people doesn't like this, I consider it as an important feature which will be kept for future releases

To have a language selector on your page, you can use the block provided or these theme functions:

 theme("i18n_flags") -> Adds just a row with the flags
 theme("i18n_links",$flags,$names,$delim1,$delim2) -> Check documentation in the code for different options

About multilingual content:
=====================
Multilingual content means providing content translated to different languages or language specific content, which is not the same as interface translation. Interface translation is done through Drupal's localization system. 
This module supports:
  - Multilingual nodes
  - Multilingual taxonomy vocabularies and terms
  - Basic translation management for nodes and terms

While this works fine when viewing the main page and managing single nodes, many modules generate node listings which are not yet language aware.
This means you can have some blocks/pages listing nodes for all languages. 
All this issues will be addressed step by step, module by module, but it will take time.

** If you are a module developer and you want to make your module 'language aware', which is quite simple, you can take a look at patched 'node.module' or drop me an e-mail if you need help.

So far, I have not found incompatibilities with any module/node type. Please, let me know if you find any.

And yes, flexinode works with multiple languages :-)

About URL aliasing with language codes -requires path module
====================================
Incoming URL's are now translated following these steps:
1. First, a translation is searched for path with language code: 'en/mypage'
2. If not found, language code is removed, and a path translation is searched again: 'mypage'
Thus, you can define aliases with or without language codes in them

The 'Front page: Language dependent' option means that when the request is for the front page '/', a language prefix will be added before doing the path translation, and then -> step 1 above
This language code will be taken from browser if enabled 'Browser language detection', or will be the default otherwise.

To have aliases for a translated node/page, you have to define each of them. I.e.:
  en/mycustompath -> node/34 (which is suppossed to be the english version)
  es/mycustompath -> node/35 (which should be the spanish version)

About language dependent variables:
======================
Some site-wide variables, like 'site_name', 'site_slogan', user e-mail contents... have language dependent content.
Since I don't like the solution of runing them through the localization system, because this means when you change the 'master' text, you have to re-translate it for every language, I've added this new feature which makes possible to have a list of variables -defined in the config file- which will be kept separated for each language.
This part is an add-on, and you can use it or not. To setup which variables will be multilingual, see INSTALL.txt

About language dependent tables 
===============================
Language dependent tables are not needed anymore for multilingual content.
This feature is kept for backwards compatibility, experimentation and may be some use in the future.
* This can be used to have per-language data for modules not language-aware, like language statistics... you can experiment...let me know

About Synchronization  [old i18n module style] :
======================
This is not required anymore, and probably the 'basic translation interface' will be enough
However, I plan to implement some related options in the future, just to keep translations in sync

Samples: Sites using this module - e-mail me to be listed here
==========================================================
http://freelance.reyero.net, well, its mine, no merit :-)

Additional Support
==================
For support, please create a support request for this module's project: 
  http://drupal.org/project/i18n

If you need professional support, contact me by e-mail: freelance at reyero dot net

====================================================================
Jose A. Reyero, drupal at reyero dot net, http://freelance.reyero.net

Feedback is welcomed.