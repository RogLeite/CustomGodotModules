/**
 * @file lua_controller.h
 * @author Rodrigo Leite (you@domain.com)
 * @brief 
 * @version 0.2
 * @date 2021-11-19
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef LUA_CONTROLLER_H
#define LUA_CONTROLLER_H

#include "scene/main/node.h" /* Base Class for LuaController */
#include "core/ustring.h" /* To use Godot's String class */

#include <memory>
#include <vector>

#include <LuaCpp.hpp>
#include "LuaControllerContext.hpp"
#include "lua_callable.h"

class LuaController : public Node {
	GDCLASS(LuaController, Node);
    
    /**
     * @brief String to be interpreted as Lua code
     */
    String lua_code;

    /**
     * @brief Context where lua_code is compiled and run
     */
    LuaCpp::LuaControllerContext lua;

    /**
     * @brief Flag used by run() to decide if it tries to execute code
     */
    bool compilation_succeded;

    /**
     * @brief Stores the error message describing the last Error code returned by a method.
     */
    String error_message;

    /**
     * @brief Vector of objects callable from Lua
     * 
     * When called during the execution of Lua code, they emit a Godot signal.
     */
    std::vector<std::shared_ptr<LuaCallable>> callables; 

    /**
     * @brief Dictionary of pairs {"signal to register" : "name to register as"}
     * 
     * The keys of this dictionary are which of this object's signals will be registered
     * as LuaCallables inside a Context, and they're respective values are how they will 
     * be named in the context (i.e. what variable the player will call)
     */
    Dictionary signals_to_register;
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
     * As a side-effect, resets lua and uses it to compile the code.
     * 
     * @return Error OK if successfully compiled;
     * @return Error ERR_COMPILATION_FAILED if compilation failed somewhere;
     * 
     * @post 
     * If OK was returned, lua now contains the correctly compiled Lua code
     * and run() will execute the code if called.
     * @post
     * If ERR_COMPILATION_FAILED wes returned, compilation_succeded will be set to false
     * and error_message now contains the description of the error, prefixed with the 
     * string "[LOGIC ERROR] : "
     *           
     */
    Error compile ();

    /**
     * @brief Prepares a LuaCallable for each signal from signals_to_register
     * 
     * This needs to be called if the object's signals have changed.
     * This method is connected to this objects "script_changed" signal.
     * For each signal in signals_to_register, instances a LuaCallable and stores it in callables. 
     */
    void prepare_callables ();

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
     * @return ERR_INVALID_DATA `compilation_succeded` is false
     * 
     * @post 
     * If ERR_INVALID_DATA was returned, error_message now contains the description 
     * of the error, prefixed with the string "[RUNTIME ERROR] : "
     * @post 
     * If ERR_SCRIPT_FAILED was returned, error_message now contains the description 
     * of the error, prefixed with the string "[RUNTIME ERROR] : "
     */
    Error run ();

    /**
     * @brief Clears error_message.
     * Any stored error_message will not be recoverable after this.
     */
    void clear_error_message ();
    /**
     * @brief Get the error_message member
     * 
     * @return String 
     */
    String get_error_message ();

    /**
     * @brief Getter and Setter methods for signals_to_register
     */
    void set_signals_to_register (const Dictionary& dictionary);
    Dictionary get_signals_to_register () const;

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
