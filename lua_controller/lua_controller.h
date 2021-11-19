/**
 * @file lua_controller.h
 * @author Rodrigo Leite (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2021-11-19
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef LUA_CONTROLLER_H
#define LUA_CONTROLLER_H

#include "scene/main/node.h" /* Base Class for LuaController */
#include "core/ustring.h" /* To use Godot's String class */

#include <LuaCpp.hpp>

class LuaController : public Node {
	GDCLASS(LuaController, Node);
    
    /**
     * @brief String to be interpreted as Lua code
     */
    String lua_code;

    /**
     * @brief Context where lua_code is compiled and run
     */
    LuaCpp::LuaContext ctx;

    /**
     * @brief Flag used by run() to decide if it tries to execute code
     */
    Error compilation_succeded;

protected:
    /**
     * @brief Binds a selection of methods and members on Godot's Class Database (ClassDB)
     */
    static void _bind_methods ();

public:

    /**
     * @brief Set the lua_code member
     * 
     * @param code String to be stored
     * 
     * @details Sets lua_code as code. If code is an empty String, future calls to 
     * LuaController's other methods treat lua_code as undefined which, in general,
     * means they will do nothing and return an Error.
     */
    void set_lua_code (String code);

    /**
     * @brief Compiles the stored String as Lua code.
     * 
     * Checks if the last call made to set_lua_code had a non-empty string as parameter.
     * As a side-effect, resets ctx and uses it to compile the code.
     * 
     * @return Error OK if successfully compiled;
     * @return Error ERR_COMPILATION_FAILED if compilation failed somewhere;
     * @return Error ERR_INVALID_DATA if lua_code is empty
     * 
     * @post If OK was returned, ctx now contains the correctly compiled Lua code
     * and run() will execute the code if called.
     * 
     * @post
     * If ERR_COMPILATION_FAILED or ERR_INVALID_DATA were returned, compilation_succeded will
     * be set to false
     *           
     */
    Error compile ();

    /**
     * 
     * @brief Executes the compiled Lua code
     * 
     * @return Error 
     * 
     * @details description
     * 
     * @return OK if the script ran successfully;
     * @return ERR_SCRIPT_FAILED if there was a runtime error in the execution;
     * @return \todo [TODO] ERR_TIMEOUT if the execution took to long to conclude
     * @return \todo [TODO] ERR_INVALID_DATA `compilation_succeded` is false
     */
    Error run ();

    /**
     * @brief Construct a new LuaController object
     */
    LuaController ();
    /**
     * @brief Destroy the LuaController object
     */
    ~LuaController ();

};
#endif
