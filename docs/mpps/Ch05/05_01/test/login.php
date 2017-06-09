<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>Auto Login</title>
    <link href="css/styles.css" rel="stylesheet" type="text/css">
</head>

<body>
<h1>Persistent Login</h1>
<?php
if (isset($error)) {
    echo "<p>$error</p>";
}
?>
<form action="<?= $_SERVER['PHP_SELF']; ?>" method="post">
    <p>
        <label for="username">Username:</label>
        <input type="text" name="username" id="username">
    </p>
    <p>
        <label for="pwd">Password:</label>
        <input type="password" name="pwd" id="pwd">
    </p>
    <p>
        <input type="checkbox" name="remember" id="remember">
        <label for="remember">Remember me </label>
    </p>
    <p>
        <input type="submit" name="login" id="login" value="Log In">
    </p>
</form>
</body>
</html>