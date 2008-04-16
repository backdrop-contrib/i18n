// $Id$

README.txt
==========

********************************************************************
This is i18n package, development version , and works with Drupal 6.x
********************************************************************
WARNING: DO READ THE INSTALL FILE
********************************************************************
Updated documentation will be kept on-line at http://drupal.org/node/67817
********************************************************************

This is a collection of modules providing multilingual features.
These modules will build onto Drupal 6 core features enabling a full multilingual site

i18n: core i18n functionality
=============================
- Multilingual variables
- Extended language API for other modules

languageicons: language icons for translation links
============= 
This module will be spin-off from the main package

i18ntaxonomy: Taxonomy translation:
====================
You can create vocabularies and terms with or without language. 
- If you set language for a vocabulary/term, that term will just show up for pages in that language
- If you set language for a vocabulary, all the terms in that vocabulary will be assigned that language.
- When editing nodes, if you change the language for a node, you have to click on 'Preview' to have the right vocabularies/terms for that language. Otherwise, the language/taxonomy data for that node could be inconsistent.
  

About language dependent variables:
==================================
Some site-wide variables, like 'site_name', 'site_slogan', user e-mail contents... have language dependent content.
Since I don't like the solution of runing them through the localization system, because this means when you change the 'master' text, you have to re-translate it for every language, I've added this new feature which makes possible to have a list of variables -defined in the config file- which will be kept separated for each language.
This part is an add-on, and you can use it or not.

Known problems, compatibility issues
====================================
These modules should be compatible with all Drupal core modules.

Sample sites, using this module - e-mail me to be listed here
==========================================================
  http://www.reyero.net
  http://www.para.ro
  http://www.ctac.ca
  http://grasshopperarts.com
  http://funkycode.com
  http://www.newoceans.nl

Additional Support
=================
For support, please create a support request for this module's project: 
  http://drupal.org/project/i18n

If you need professional support, contact me by e-mail: freelance at reyero dot net

====================================================================
Jose A. Reyero, drupal at reyero dot net, http://www.reyero.net

Feedback is welcomed.