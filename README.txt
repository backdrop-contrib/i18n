README.txt
==========
Warning: This is a beta version and all the combinations of the module settings have not been tested yet
Warning: Some features are experimental
Warning: DO READ THE INSTALL FILE

This module provides support for internationalization of Drupal sites in various ways:

    * Translation of the user interface for registered and anonymous users
    * Basic multi-language using URL Aliasing, combined with path module. See below.
    * [EXPERIMENTAL, see INSTALL file] Advanced multi-language using duplicated tables. Selected tables will be made language dependent, having a different table for each language.
    * Detection of the brower language
    * Keeps the language settings accross consecutive requests, using a number of methods: URL rewriting, sessions, cookies
    * Provides a block for language selection and two theme functions: i18n_flags and i18n_links
	* Language dependent variables -one more patch required-
You can try different settings to have only content translation, interface translation or both. 

For url rewriting you need to have the file i18n.inc in the includes folder and add the following line to your configuration file:

  			include 'includes/i18n.inc';

To have a language selector on your page, you can use the block provided or these theme functions:

 theme("i18n_flags") -> Adds just a row with the flags
 theme("i18n_links",$flags,$names,$delim1,$delim2) -> Check documentation in the code for different options

About multilingual content:
=====================
  Multilingual content means providing content translated to different languages or language specific content, which is not the same as interface translation. Interface translation is done through Drupal's localization system. 
  This module currently provides two options to build multi-language sites:
  
  
  1. With URL aliasing - using path module's Custom URLs and i18n's URL rewriting feature
  ---------------------
  You have to create a Custom URL for each language version of a node:
    I.e. 'en/mypage', 'es/mypage' should be English and Spanish versions of the same page
  	The link to display this page will be only 'mypage'. The language code will be added automatically.

  2. With duplicated tables [Only in HEAD CVS version of the module]
  ------------------------
    You can keep separated tables -the ones you choose- for each language. 
    You have to create the tables manually and add them to $db_prefix_i18n. See INSTALL file.
    * This feature uses db prefixing (An idea taken from Walkah's translate_node module) to keep separated tables for each language.

About Synchronization  [EXPERIMENTAL FEATURE] :
======================
  Some synchronization is needed when keeping separated data for each language.
  This is about keeping object ids -for nodes, vocabularies, terms- in sync between different languages. This is neccesary in order to provide the translated version automatically when the user is viewing a page an selects a different language.
  There are options for automatic synchronization of nodes and taxonomy. This creates/deletes nodes/vocabularies/terms for each language when they are created for *any* language, thus keeping id's in sync.
  * Authomatic synchronization is not garanteed to work for all node types. Please, do some testing before trying this module with other contributed modules and create a bug or a feature request...
  ** When using language dependent tables other than node tables or taxonomy tables, some manual synchronization may be needed. 
  *** The automatic synchronization ACTUALLY DELETES NODES and TAXONOMY DATA. Take care.

About language dependent variables [NEW] :
======================
Some site-wide variables, like 'site_name', 'site_slogan', user e-mail contents... have language dependent content.
Since I don't like the solution of runing them through the localization system, because this means when you change the 'master' text, you have to re-translate it for every language, I've added this new feature which makes possible to have a list of variables -defined in the config file- which will be kept separated for each language.
This part is an add-on, and you can use it or not.

Notes
=====
  This module is currently under development and I am still figuring out how some combinations of language dependent/independent tables may/should work. There could be some issues with moderation, search, feeds, etc... If you want to try different settings or have any idea about it, please let me know.
  Some advanced features are quite complex and require database modifications and an overall Drupal knowledge. They are not intended for novice users.
  
Jose A. Reyero, drupal at reyero dot net

Feedback will be welcomed, but for support, please create an 'issue' of type 'support request' for the project -Internationalization-.
 
