<?php

$config = [
    'db_driver' => 'mysql',
    'db_host' => 'localhost',
    'db_name' => 'persistent',
    'db_username' => 'root',
    'db_password' => '',
];

try {
    $db = new PDO(
        "{$config['db_driver']}:host={$config['db_host']};dbname={$config['db_name']}",
        $config['db_username'],
        $config['db_password']
    );
    $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
}
catch (PDOException $e) {
    $error = $e->getMessage();
}

if (isset($db)) {
    echo 'Connected';
} elseif (isset($error)) {
    echo $error;
} else {
    echo 'Unknown error';
}