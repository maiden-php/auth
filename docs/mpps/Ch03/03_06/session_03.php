<?php
// initialize session
session_start();
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