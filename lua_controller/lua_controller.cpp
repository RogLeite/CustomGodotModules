#include "lua_controller.h"

void LuaController::_bind_methods () {
    /* Valor default para argumento é a string vazia */
    ClassDB::bind_method(D_METHOD("set_lua_code", "code"), &LuaController::set_lua_code, DEFVAL(""));
    ClassDB::bind_method(D_METHOD("compile"), &LuaController::compile);
    ClassDB::bind_method(D_METHOD("run"), &LuaController::run);
}

void LuaController::set_lua_code (String code) {
    lua_code = std::move(code);
}

Error LuaController::compile () {

    /* Se a pré-condição da cadeia não foi atendida, não executa nenum código e retorna o erro */    
    if (lua_code.empty()) 
        return ERR_INVALID_DATA;

    ctx = LuaCpp::LuaContext();
 
    /* Attempts compilation of lua_code */
    try {
	    /* Gets const char* from Godot's String type */
        ctx.CompileString("default", lua_code.ascii().get_data());
    }
    catch(const std::logic_error& e) {
        return ERR_COMPILATION_FAILED;
    }

    return OK;
}

Error LuaController::run () {
    
    try {
	    ctx.Run("default");
	}
	catch (std::runtime_error& e) {
        return ERR_SCRIPT_FAILED;
    }
    return OK;
}

LuaController::LuaController () {
    /* The fllowing Godot's code convention, which didn't use initializer list in constructors */
    lua_code = "";
    ctx = LuaCpp::LuaContext();
}

LuaController::~LuaController() {
}