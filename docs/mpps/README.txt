IMPORTANT NOTE:

This course makes use of the PHP Password Hashing API (https://php.net/manual/en/book.password.php), which requires a minimum of PHP 5.5. If your server is running PHP 5.4, obtain the password_compat library from https://github.com/ircmaxell/password_compat, and include it in the locations indicated in Chapter 5.

The code will not work with PHP 5.3 or earlier.

In a testing environment, display_errors should be enabled. The best way is to edit the main PHP configuration file, php.ini. If you have difficulty editing php.ini, a simple way to enable display_errors on an Apache server is to add the following line to an .htaccess file in the server root:

php_flag display_errors on