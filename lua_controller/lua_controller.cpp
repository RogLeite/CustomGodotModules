#include "lua_controller.h"

void LuaController::_bind_methods () {
    ClassDB::bind_method(D_METHOD("set_lua_code", "code"), &LuaController::set_lua_code, DEFVAL(""));
    ClassDB::bind_method(D_METHOD("compile"), &LuaController::compile);
    ClassDB::bind_method(D_METHOD("prepare_callables"), &LuaController::prepare_callables);
    ClassDB::bind_method(D_METHOD("run"), &LuaController::run);
    ClassDB::bind_method(D_METHOD("clear_error_message"), &LuaController::clear_error_message);
    ClassDB::bind_method(D_METHOD("get_error_message"), &LuaController::get_error_message);
    ClassDB::bind_method(D_METHOD("set_methods_to_register"), &LuaController::set_methods_to_register);
    ClassDB::bind_method(D_METHOD("get_methods_to_register"), &LuaController::get_methods_to_register);
    ClassDB::bind_method(D_METHOD("set_lua_core_libs", "flags"), &LuaController::set_lua_core_libs);
    ClassDB::bind_method(D_METHOD("get_lua_core_libs"), &LuaController::get_lua_core_libs);
    
    ClassDB::add_virtual_method(get_class_static(),
        MethodInfo("lua_error_handler",
            PropertyInfo(Variant::INT,"call_error_code",PROPERTY_HINT_ENUM,"CALL_OK,CALL_ERROR_INVALID_METHOD,CALL_ERROR_INVALID_ARGUMENT,CALL_ERROR_TOO_MANY_ARGUMENTS,CALL_ERROR_TOO_FEW_ARGUMENTS,CALL_ERROR_INSTANCE_IS_NULL"),
            PropertyInfo(Variant::STRING, "message")));
	
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "methods_to_register", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE),
                "set_methods_to_register", "get_methods_to_register");
            
    // Inspired by how Control's size flags are displayed
    ADD_GROUP("Core Libs", "lua_core_");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "lua_core_libraries", PROPERTY_HINT_FLAGS, "base,coroutine,table,io,os,string,utf8,math,debug,package"), "set_lua_core_libs", "get_lua_core_libs");
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


void LuaController::prepare_callables() {
    List<MethodInfo> method_list; 
	get_method_list(&method_list);

    callables.clear();

    // If there are no methods to register, then the work is done
    if (methods_to_register.empty())
        return;
    
    // For every method in this object
    for (List<MethodInfo>::Element *E = method_list.front(); E; E = E->next()) {
        // Only registers methods whose names are keys in the dictionary methods_to_register
        String method_name(E->get().name);
        if (methods_to_register.has(method_name))
            callables.push_back(
                std::make_shared<LuaCallable>(
                    get_instance_id(),
                    E->get(),
                    [=](Variant::CallError::Error err, String msg){
                        if (this->has_method("lua_error_handler"))
                            this->call("lua_error_handler", (int)err, msg);
                    }
                )
            );
 	}
}

Error LuaController::run () {
    if (!compilation_succeded) {
        error_message = "[RUNTIME ERROR] : No valid compiled code to execute";
        return ERR_INVALID_DATA;
    }
    
    // Adds every LuaCallable from callables in the context as a variable named after name_in_lua
    for (auto &callable : callables) {
        const Variant &key = callable->get_method_name();
        const char *name_in_lua = ((String)methods_to_register.get_valid(key)).ascii().get_data();
        lua.AddGlobalVariable(name_in_lua , callable);
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

void LuaController::set_methods_to_register (const Dictionary& methods) {
    Array keys = methods.keys();
    // Checks if every key and value is of type STRING
    for (int i=0; i<keys.size(); i++) {
        const Variant &key = keys[i];
        ERR_FAIL_COND( key.get_type() != Variant::STRING);
        ERR_FAIL_COND( methods[key].get_type() != Variant::STRING);
    }
    methods_to_register = methods.duplicate();

    // After methods_to_register changed, it's probable that callables need preparing
    prepare_callables();
}

Dictionary LuaController::get_methods_to_register () const {
    return methods_to_register.duplicate();
}

void LuaController::set_lua_core_libs (int flags) {
    if (lua_core_libraries == flags)
        return;
    lua_core_libraries = flags;
    // Sets the flags in the context
    lua.setLuaCoreLibraries(lua_core_libraries);
}

int LuaController::get_lua_core_libs () const {
    return lua_core_libraries;
}

LuaController::LuaController () {
    // This follows Godot's code convention, which didn't use initializer list
    lua_code = "";
    lua = LuaCpp::LuaControllerContext();
    compilation_succeded = false;
    error_message = "";
    prepare_callables();
    methods_to_register = Dictionary();
    lua_core_libraries = LuaCpp::LIB_ALL;
    
    connect("script_changed", this, "prepare_callables");
}

LuaController::~LuaController() {
}