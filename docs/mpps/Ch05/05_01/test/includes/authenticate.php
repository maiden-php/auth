<?php
require_once __DIR__ . '/init.php';

use Foundationphp\Sessions\AutoLogin;

if (isset($_SESSION['authenticated']) || isset($_SESSION['lynda_auth'])) {
   // we're OK
} else {
    $autologin = new AutoLogin($db);
    $autologin->checkCredentials();
    if (!isset($_SESSION['lynda_auth'])) {
        header('Location: login.php');
        exit;
    }
}