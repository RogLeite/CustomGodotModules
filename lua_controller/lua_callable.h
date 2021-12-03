/**
 * @file lua_callable.h
 * @author Rodrigo Leite (you@domain.com)
 * @brief Calling this MetaObject inside a Lua context forward the call to the informed method in Godot
 * @version 0.3
 * @date 2021-11-22
 *
 */
#ifndef LUA_CALLABLE_H
#define LUA_CALLABLE_H

#include <LuaCpp.hpp>
#include "core/ustring.h"
#include "core/object.h"

#include <functional>

class LuaCallable : public LuaCpp::LuaMetaObject {
private:
    friend class LuaControllerUnitTester;
    ObjectID object_id;
    MethodInfo info;
    using ErrorHandler = std::function<void(Variant::CallError::Error, String)>;
    ErrorHandler handler;
public:
    /**
     * @brief Calls the method `info` of the Object represented by `object_id`
     */
    int Execute (LuaCpp::Engine::LuaState &L);
    /**
     * @brief Returns the method name, as stored in info
     */
    String get_method_name () const;

    /**
     * @brief Construct a new Meta Callable object
     * 
     * @param id Godot's representation of the instance of an object 
     * @param method Godot's description of the method
     * @param f Callback to be invoked if an error occurs when Execute() calls the method
     */
    LuaCallable (ObjectID id, MethodInfo method, ErrorHandler f);
    LuaCallable () = delete ;
    ~LuaCallable ();
};



#endif
