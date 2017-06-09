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
<p><a href="session_01.php">Go to page 1</a></p>
</body>
</html>