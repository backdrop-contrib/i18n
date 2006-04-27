-- 
-- Table: 'i18n_node'
--

CREATE TABLE i18n_node (
  nid integer NOT NULL default '0', 
  trid integer NOT NULL default '0',
  language varchar(12) NOT NULL default '',
  status smallint NOT NULL default '0',
  PRIMARY KEY  (nid)
);

CREATE SEQUENCE i18n_node_trid_seq INCREMENT 1 START 1;


CREATE SEQUENCE term_data_trid_seq INCREMENT 1 START 1;

-- Add language and trid fields to term_data
ALTER TABLE term_data ADD language varchar(12);
UPDATE term_data SET language='';
ALTER TABLE term_data ALTER COLUMN language SET NOT NULL;
ALTER TABLE term_data ALTER COLUMN language SET DEFAULT '';

-- Add language field to vocabulary
ALTER TABLE vocabulary ADD language varchar(12);
UPDATE vocabulary SET language='';
ALTER TABLE term_data ALTER COLUMN language SET NOT NULL;
ALTER TABLE term_data ALTER COLUMN language SET DEFAULT '';
