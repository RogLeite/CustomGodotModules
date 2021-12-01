/**
 * @file lua_controller_unit_tester.h
 * @author Rodrigo Leite (you@domain.com)
 * @brief LuaControllerUnitTester's methods run unit tests for each class in the LuaController module. 
 * @version 0.1
 * @date 2021-11-30
 * 
 * @copyright Copyright (c) 2021
 * 
 */


#ifndef LUA_CONTROLLER_UNIT_TESTER_H
#define LUA_CONTROLLER_UNIT_TESTER_H

#include "scene/main/node.h"
#include "core/class_db.h"


class LuaControllerUnitTester : public Node {
	GDCLASS(LuaControllerUnitTester, Node);

    /**
     * _true(), _not(), _and() are simple methods, to help unit test 
     * function calls from inside the execution of Lua code.
     */

    /**
     * @brief Returns true
     */
    Variant _true ();
    /**
     * @brief Returns !arg
     * Safe to call with fewer arguments
     */
    Variant _not (Variant arg);
    /**
     * @brief Returns !arg
     * Unsafe to call with fewer arguments
     */
    Variant _not_unsafe (bool arg);
    /**
     * @brief Returns arg1 && arg2
     * Safe to call with fewer arguments
     */
    Variant _and (Variant arg1, Variant arg2);

    /**
     * @brief Run unit tests for class LuaCallable.
     * 
     * @return 
     * true if all tests succeeded. 
     * @return 
     * Array of String if any test failed. Each String in the Array decribes one failed Assertion.
     */
    Variant _lua_callable ();

    /**
     * @brief Run unit tests for class LuaControllerContext.
     * 
     * @return 
     * true if all tests succeeded. 
     * @return 
     * Array of String if any test failed. Each String in the Array decribes one failed Assertion.
     */
    Variant _lua_controller_context ();

    /**
     * @brief Run unit tests for class LuaController.
     * 
     * @return 
     * true if all tests succeeded. 
     * @return 
     * Array of String if any test failed. Each String in the Array decribes one failed Assertion.
     */
    Variant _lua_controller ();

protected:    
    /**
     * @brief Binds a selection of methods and members on Godot's Class Database (ClassDB)
     */
    static void _bind_methods ();

public:
    /**
     * @brief Construct a new LuaControllerUnitTester object
     */
    LuaControllerUnitTester ();
    /**
     * @brief Destroy the LuaControllerUnitTester object
     */
    ~LuaControllerUnitTester ();
};

#endif