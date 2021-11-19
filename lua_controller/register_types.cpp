/* register_types.cpp */

#include "register_types.h"

#include "core/class_db.h"
#include "lua_controller.h"

void register_lua_controller_types () {
    ClassDB::register_class<LuaController>();
}

void unregister_lua_controller_types () {
}