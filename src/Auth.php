<?php

namespace Maiden\Auth;

class Auth implements AuthInterface
{
    /**
     * Auth constructor.
     */
    function __construct()
    {

    }

    /**
     * @param array $data
     * @return mixed
     */
    function login(array $data)
    {
        // TODO: Implement login() method.
    }

    /**
     * @return mixed
     */
    function logout()
    {
        // TODO: Implement logout() method.
    }

    /**
     * @param array $data
     * @return mixed
     */
    function register(array $data)
    {
        // TODO: Implement register() method.
    }

    /**
     * @return mixed
     */
    function changePassword()
    {
        // TODO: Implement changePassword() method.
    }

    /**
     * @return mixed
     */
    function forgotPassword()
    {
        // TODO: Implement forgotPassword() method.
    }

    /**
     * @return mixed
     */
    function getAuthUser()
    {
        // TODO: Implement getAuthUser() method.
    }

    /**
     * @return mixed
     */
    function isLoggedIn(): bool
    {
        return true;
    }
}