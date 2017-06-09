<?php
use Foundationphp\Sessions\PersistentSessionHandler;

require_once __DIR__ . '/Psr4AutoloaderClass.php';
require_once __DIR__ . '/db_connect.php';

$loader = new Psr4AutoloaderClass();
$loader->register();
$loader->addNamespace('Foundationphp', __DIR__ . '/../../Foundationphp');

$handler = new PersistentSessionHandler($db);
session_set_save_handler($handler);
session_start();
$_SESSION['active'] = time();