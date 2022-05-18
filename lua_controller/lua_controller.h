/**
 * @file lua_controller.h
 * @author Rodrigo Leite (you@domain.com)
 * @brief LuaController can interpret a String as Lua code.
 * 
 * LuaController can interpret a String as Lua code. And, through the property method_to_register,
 * allows the user of the Node to choose which of LuaController's method can be invoked as a function
 * by the execution of the Lua code. The methods   
 * @version 1.1
 * @date 2021-11-19
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef LUA_CONTROLLER_H
#define LUA_CONTROLLER_H

#include "scene/main/node.h" /* Base Class for LuaController */
#include "core/ustring.h" /* To use Godot's String class */
#include "core/os/mutex.h" /* To use Godot's Mutex class */

#include <memory>
#include <vector>

#include <LuaCpp.hpp>
#include "LuaControllerContext.hpp"
#include "lua_callable.h"

class LuaController : public Node {
	GDCLASS(LuaController, Node);
    
    friend class LuaControllerUnitTester;

    /**
     * @brief String to be interpreted as Lua code
     */
    String lua_code;

    /**
     * @brief Context where lua_code is compiled and run
     */
    LuaCpp::LuaControllerContext lua;

    /**
     * @brief run() checks if this flag is true before trying to execute code
     */
    bool compilation_succeded;

    /**
     * @brief After LuaController's methods return a Error, they store a description of the error in error_message.
     */
    String error_message;

    /**
     * @brief Vector of objects invokable inside Lua
     * 
     * run() adds each of these LuaCallables in the LuaState before execution.
     * When invoked by the Lua script execution, a LuaCallable calls it's respective method from a Godot Object.
     */
    std::vector<std::shared_ptr<LuaCallable>> callables; 

    /**
     * @brief Dictionary of pairs {"method to register" : "name to register as"}
     * 
     * Each `key` from methods_to_register is the name of a method from this Object.
     * run() uses each value from methods_to_register to name the variable that represents the method `key` in the LuaControllerContext
     */
    Dictionary methods_to_register;

    /**
     * @brief Each bit is a flag that indicates if a specific core Lua library should be opened in the LuaControllerContext 
     */
    int lua_core_libraries;

    /**
     * @brief Maximum amount of lines executed
     * 
     * If the execution of the code exceedes max_lines lines executed, LuaController 
     * interrupts the execution and treats it as a TIMEOUT.
     * 
     * If max_lines is 0, the execution cannot timeout by line count.
     */
    int max_lines;
    
    /**
     * @brief Maximum amount of commands counted
     * 
     * If the execution of the code exceedes max_count commands counted, LuaController 
     * interrupts the execution and treats it as a TIMEOUT.
     * 
     * If max_count is 0, the execution cannot timeout by command count.
     */
    int max_count;
    /**
     * @brief The count of commands executed is incremented by 1 every
     * count_interval commands executed.
     * 
     * If count_interval is more than 1, a timeout by command count means that 
     * the true count of commands executed is max_count * count_interval.
     */
    int count_interval;

    /**
     * @brief When set to true forces execution of the script to stop. Default value is false
     * 
     * When is set to true, the execution of the script is interrupted as soon as possible, either:
     *  when the next line is entered, if only timeout by line count was defined;
     *  or before the execution of the next "nth" command, if the timeout by command count was defined.
     */
    bool force_stop;

    /**
     * @brief Protects the setget of force_stop
     */
    Mutex force_stop_lock;

    /**
     * @brief LuaCallable to get_force_stop that will be stored in the registry of LuaState
     */
    std::shared_ptr<LuaCallable> registry_get_force_stop;
    
protected:
    
    /**
     * @brief Binds a selection of methods and members on Godot's Class Database (ClassDB)
     */
    static void _bind_methods ();

public:

    /**
     * @brief Set the lua_code member
     * 
     * The String is not verified to see if it is valid and compilable lua code.
     */
    void set_lua_code (String code);

    /**
     * @brief Compiles the stored String as Lua code.
     * 
     * As a side-effect, compiles the code into the lua member. Any previous compiled code is lost.
     * 
     * @return Error OK if successfully compiled;
     * @return Error ERR_COMPILATION_FAILED if compilation failed somewhere;
     * 
     * @post 
     * If OK was returned, member lua now contains the correctly compiled Lua code
     * and run() will execute the code if called.
     * @post
     * If ERR_COMPILATION_FAILED was returned, compilation_succeded is set to false
     * and error_message contains the description of the error, prefixed with the 
     * string "[LOGIC ERROR] : "
     *           
     */
    Error compile ();

    /**
     * @brief Prepares a LuaCallable for each method from methods_to_register
     * 
     * For each method in methods_to_register, instances a LuaCallable and stores it in member callables. 
     * prepare_callables is connected to this object's "script_changed" signal.
     * prepare_callables needs to be called by the user if the object's methods have been changed.
     */
    void prepare_callables ();

    /**
     * 
     * @brief Executes the compiled Lua code
     * 
     * Every element from member callables is placed in the LuaControllerContext.
     * 
     * @return OK if the script ran successfully or was manualy interrupted with set_force_stop()
     * @return ERR_SCRIPT_FAILED if a runtime_error occured during the execution
     * @return ERR_TIMEOUT if the execution took to long to conclude
     * @return ERR_INVALID_DATA if `compilation_succeded` is false
     * @return FAILED if hook() failed to call get_force_stop
     * 
     * @post 
     * If ERR_INVALID_DATA was returned, error_message contains the description 
     * of the error, containing the string "[RUNTIME ERROR] : "
     * @post 
     * If ERR_TIMEOUT was returned, error_message contains the description 
     * of the error, containing the string "[TIMEOUT] : "
     * @post 
     * If ERR_SCRIPT_FAILED was returned, error_message contains the description 
     * of the error, containing the string "[RUNTIME ERROR] : "
     * @post 
     * If FAILED was returned, error_message contains the description 
     * of the error, containing the string "[FAILED CALL] : "
     */
    Error run ();

    /**
     * @brief Clears error_message.
     * 
     * Any stored error_message will not be recoverable after this.
     */
    void clear_error_message ();
    /**
     * @brief Get the error_message member
     */
    String get_error_message ();

    /**
     * @brief Getter and Setter methods for methods_to_register
     */
    void set_methods_to_register (const Dictionary& dictionary);
    Dictionary get_methods_to_register () const;

    /**
     * @brief Getter and Setter methods for lua_core_libraries
     */
    void set_lua_core_libs (int flags);
    int get_lua_core_libs () const;

    /**
     * @brief Getter and Setter methods for max_lines
     * 
     * The lower bound is 0, included. 
     * max_lines and max_count cannot both be 0 simultaneously 
     * 
     */
    void set_max_lines (int maximum);
    int get_max_lines () const;

    /**
     * @brief Getter and Setter methods for max_count
     * 
     * The lower bound is 0, included. 
     * max_lines and max_count cannot both be 0 simultaneously 
     */
    void set_max_count (int maximum);
    int get_max_count () const;

    /**
     * @brief Getter and Setter methods for count_interval
     * 
     * The lower bound is 1, included.
     */
    void set_count_interval (int interval);
    int get_count_interval () const;

    /**
     * @brief Getter and Setter methods for force_stop
     * 
     * force_stop_lock is internally so that no race condition occurs 
     * when these methods are called by different threads. force_stop_lock is 
     * used for both set and get, so they are mutualy exlusive with themselves
     * and each other.
     */
    void set_force_stop (bool force);
    bool get_force_stop () const;

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
