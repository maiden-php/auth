<?php

namespace Maiden\Auth;

interface AuthInterface
{
    function login(array $data);

    function logout();

    function register(array $data);

    function changePassword();

    // sends an email to the user
    function forgotPassword();

    function getAuthUser();

    function isLoggedIn();
}