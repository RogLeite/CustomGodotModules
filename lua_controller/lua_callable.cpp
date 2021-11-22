/**
 * @file lua_callable.cpp
 * @author Rodrigo Leite (you@domain.com)
 * @date 2021-11-22
 * 
 */
#include "lua_callable.h"
#include "core/error_macros.h"

int LuaCallable::Execute (LuaCpp::Engine::LuaState &L) {
    Object *obj = ObjectDB::get_instance(object_id);

    // Checks if the object instance still exists
    if (!obj)
        return 0;

    // Checks how many arguments the signal expects, and how many are in the stack 
    int expected_args_amount = signal_info.arguments.size();
    int top = lua_gettop(L);
    
    // Ignores the value at the base of the stack, it always is a userdatum
    if (expected_args_amount > top-1) {
        // If there are less values on the stack than expected, fill the remaining with "nil"
        int diff = expected_args_amount - top + 1; 
        for (int i = 0; i < diff; i++) 
            lua_pushnil(L);
    }

    // Instances a Variant for each argument on the stack
    std::vector<Variant> args{};
    for (int i=2; i<=expected_args_amount+1; i++) {
        int t = lua_type(L, i);
        switch (t) {
        case LUA_TNIL:
            args.push_back(Variant()); //Nil value
            break;
        case LUA_TSTRING:
            args.push_back(lua_tostring(L,i));
            break;
        case LUA_TNUMBER:
            args.push_back(lua_tonumber(L,i));
            break;
        case LUA_TBOOLEAN:
            args.push_back((bool)lua_toboolean(L,i));
            break;
        case LUA_TTABLE: // For now, ignores tables. But they could probably be translated to a Dictionary. Maybe converting to and from a string
        default:
            args.push_back(lua_typename(L, t));
            break;
        }
    }

    // Stores the pointer to each argument.
    std::vector<Variant*> p_args;
    for (Variant &arg : args)
        p_args.push_back(&arg);


    // The cast to (const Variant**) is needed by the overload resolution to find the correct method.
    Error err = obj->emit_signal(signal_info.name, (const Variant**)p_args.data(), p_args.size());
    if (err != OK) {
        // \todo [TODO] Send the error to the USER, not the PLAYER
        lua_pushstring(L, "Error ocurred on signal emition");
        return 1;
    }
    return 0;
}

String LuaCallable::get_signal_name () const {
    return signal_info.name;
}

LuaCallable::LuaCallable(ObjectID id, MethodInfo signal)
: object_id(id)
, signal_info(signal)
{
}

LuaCallable::~LuaCallable()
{
}
