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

    // This should never be true, because obj should be the owner of this here object
    // so I let the code execution continue 
    // if (!obj)
    //     return 0;

    // Checks how many arguments the method expects, and how many are in the stack 
    int expected_args_amount = info.arguments.size();
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
        case LUA_TSTRING:
            args.push_back(lua_tostring(L,i));
            break;
        case LUA_TNUMBER:
            args.push_back(lua_tonumber(L,i));
            break;
        case LUA_TBOOLEAN:
            args.push_back((bool)lua_toboolean(L,i));
            break;
        case LUA_TNIL:
        case LUA_TTABLE: // Ignores table. But they could probably be translated to a Dictionary.
        default:
            args.push_back(Variant()); //< Nil value
            break;
        }
    }

    // Stores the pointer to each argument.
    std::vector<Variant*> p_args;
    for (Variant &arg : args)
        p_args.push_back(&arg);

    Variant::CallError r_error;
    // The cast (const Variant**) is needed by the overload resolution to find the correct method.
    Variant result = obj->call(info.name, (const Variant**)p_args.data(), p_args.size(), r_error);
    if (r_error.error != Variant::CallError::CALL_OK) {
        String msg = Variant::get_call_error_text(obj, info.name, (const Variant**)p_args.data(), p_args.size(), r_error);
        handler(r_error.error, msg);
    }

    switch (result.get_type())
    {
    case Variant::STRING :
        lua_pushstring(L,((String)result).ascii().get_data());
        break;
    case Variant::INT :
    case Variant::REAL :
        lua_pushnumber(L, (lua_Number)result);
        break;
    case Variant::BOOL :
        lua_pushboolean(L, result ? 1 : 0);
        break;
    case Variant::DICTIONARY : // Could be converted to table, not implemented
	case Variant::ARRAY :      // Could be converted to table, not implemented
    case Variant::NIL :        // Same as default behaviour
    default:
        lua_pushnil(L);
        break;
    }

    // Allways returns a value, even if it is Nil
    return 1;
}

String LuaCallable::get_method_name () const {
    return info.name;
}

LuaCallable::LuaCallable(ObjectID id, MethodInfo method, ErrorHandler f)
: object_id(id)
, info(method)
, handler(f)
{
}

LuaCallable::~LuaCallable()
{
}
