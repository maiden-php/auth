<?php
use Foundationphp\Sessions\MysqlSessionHandler;

require_once './includes/db_connect.php';
require_once './Foundationphp/Sessions/MysqlSessionHandler.php';

$handler = new MysqlSessionHandler($db);
session_set_save_handler($handler);

// initialize session
session_start();

if (isset($_POST['logout'])) {
    $_SESSION = [];
    $params = session_get_cookie_params();
    setcookie(session_name(), '', time() - 86400, $params['path'],
        $params['domain'], $params['secure'], $params['httponly']);
    session_destroy();
    header('Location: session_01.php');
    exit;
}


?>
<!doctype html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <title>Session Test</title>
</head>
<body>
<p>Hello<?php
if (isset($_SESSION['first_name'])) {
    echo ' again, ' . $_SESSION['first_name'];
} else {
    echo ', stranger';
}
    ?>.</p>
<form method="post" action="<?= $_SERVER['PHP_SELF']; ?>">
    <p><input type="submit" name="logout" value="Log Out"></p>
</form>
</body>
</html>