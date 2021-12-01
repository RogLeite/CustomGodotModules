/* register_types.cpp */

#include "register_types.h"

#include "core/class_db.h"
#include "lua_controller.h"
#include "lua_controller_unit_tester.h"

void register_lua_controller_types () {
    ClassDB::register_class<LuaController>();
    ClassDB::register_class<LuaControllerUnitTester>();
}

void unregister_lua_controller_types () {
}