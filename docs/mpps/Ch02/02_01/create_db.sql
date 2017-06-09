CREATE DATABASE IF NOT EXISTS persistent COLLATE utf8_general_ci;

CREATE USER 'sess_admin'@'localhost' IDENTIFIED BY 'secret';

GRANT SELECT, INSERT, UPDATE, DELETE ON persistent.* TO 'sess_admin'@'localhost';