/**
 * @file lua_callable.h
 * @author Rodrigo Leite (you@domain.com)
 * @brief Calling this MetaObject inside the Lua context emits the informed signal in Godot
 * @version 0.1
 * @date 2021-11-22
 *
 */
#ifndef LUA_CALLABLE_H
#define LUA_CALLABLE_H

#include <LuaCpp.hpp>
#include "core/ustring.h"
#include "core/object.h"

class LuaCallable : public LuaCpp::LuaMetaObject {
private:
    ObjectID object_id;
    MethodInfo signal_info;
public:
    /**
     * @brief Emits the informed signal of the Object represented by object_id
     * 
     * Emits from the Object represented by object_id the signal described by signal_info
     */
    int Execute (LuaCpp::Engine::LuaState &L);
    /**
     * @brief Returns the signal name, as stored in signal_info
     */
    String get_signal_name () const;

    /**
     * @brief Construct a new Meta Callable object
     * 
     * @param id Godot's representation of the instance of an object 
     * @param signal Godot's description of the signal
     */
    LuaCallable (ObjectID id, MethodInfo signal);
    LuaCallable () = delete ;
    ~LuaCallable ();
};



#endif
