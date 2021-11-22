#include "lua_controller.h"

void LuaController::_bind_methods () {
    ClassDB::bind_method(D_METHOD("set_lua_code", "code"), &LuaController::set_lua_code, DEFVAL(""));
    ClassDB::bind_method(D_METHOD("compile"), &LuaController::compile);
    ClassDB::bind_method(D_METHOD("run"), &LuaController::run);
    ClassDB::bind_method(D_METHOD("clear_error_message"), &LuaController::clear_error_message);
    ClassDB::bind_method(D_METHOD("get_error_message"), &LuaController::get_error_message);
}

void LuaController::set_lua_code (String code) {
    lua_code = code;
    compilation_succeded = false;
}

Error LuaController::compile () { 
    /* Attempts compilation of lua_code */
    try {
	    /* Gets (const char*) from Godot's String type, and forces a recompilation */
        lua.CompileString("default", lua_code.ascii().get_data(), true);
    }
    catch(const std::logic_error& e) {
        compilation_succeded = false;
        error_message = String("[LOGIC ERROR] : ")+String(e.what());
        return ERR_COMPILATION_FAILED;
    }

    compilation_succeded = true;
    return OK;
}

Error LuaController::run () {
    if (!compilation_succeded) {
        error_message = "[RUNTIME ERROR] : No valid compiled code to execute";
        return ERR_INVALID_DATA;
    }
    
    try {
	    lua.Run("default");
	}
    catch (std::runtime_error& e) {
        error_message = String("[RUNTIME ERROR] : ")+String(e.what());
        return ERR_SCRIPT_FAILED;
    }

    return OK;
}

void LuaController::clear_error_message () {
    error_message = "";
}

String LuaController::get_error_message () {
    return error_message;
}

LuaController::LuaController () {
    /* This follows Godot's code convention, which didn't use initializer list */
    lua_code = "";
    lua = LuaCpp::LuaControllerContext();
    compilation_succeded = false;
    error_message = "";
}

LuaController::~LuaController() {
}