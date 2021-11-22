#include "lua_controller.h"

void LuaController::_bind_methods () {
    ClassDB::bind_method(D_METHOD("set_lua_code", "code"), &LuaController::set_lua_code, DEFVAL(""));
    ClassDB::bind_method(D_METHOD("prepare_callables"), &LuaController::prepare_callables);
    ClassDB::bind_method(D_METHOD("compile"), &LuaController::compile);
    ClassDB::bind_method(D_METHOD("run"), &LuaController::run);
    ClassDB::bind_method(D_METHOD("clear_error_message"), &LuaController::clear_error_message);
    ClassDB::bind_method(D_METHOD("get_error_message"), &LuaController::get_error_message);
    ClassDB::bind_method(D_METHOD("set_signals_to_register"), &LuaController::set_signals_to_register);
    ClassDB::bind_method(D_METHOD("get_signals_to_register"), &LuaController::get_signals_to_register);
    
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "signals_to_register", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE),
                "set_signals_to_register", "get_signals_to_register");
    
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
    List<MethodInfo> signal_list; 
	get_signal_list(&signal_list);

    callables.clear();

    // If there are no signals to register, then the work is done
    if (signals_to_register.empty())
        return;
    
    // For every signal in this object
    for (List<MethodInfo>::Element *E = signal_list.front(); E; E = E->next()) {
        // Only registers signals whose names are keys in the dictionary signals_to_register
        String signal_name(E->get().name);
        if (signals_to_register.has(signal_name))
            callables.push_back(std::make_shared<LuaCallable>(get_instance_id(), E->get()));
 	}
}

Error LuaController::run () {
    if (!compilation_succeded) {
        error_message = "[RUNTIME ERROR] : No valid compiled code to execute";
        return ERR_INVALID_DATA;
    }
    
    // Adds every LuaCallable from callables in the context as 
    // a variable named after name_in_lua
    for (auto &callable : callables) {
        const Variant &key = callable->get_signal_name();
        const char *name_in_lua = ((String)signals_to_register.get_valid(key)).ascii().get_data();
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

void LuaController::set_signals_to_register (const Dictionary& signals) {
    Array keys = signals.keys();
    // Checks if every key and value is of type STRING
    for (int i=0; i<keys.size(); i++) {
        const Variant &key = keys[i];
        ERR_FAIL_COND( key.get_type() != Variant::STRING);
        ERR_FAIL_COND( signals[key].get_type() != Variant::STRING);
    }
    signals_to_register = signals.duplicate();

    // Since signals_to_register changed, chances are that the callables need preparing
    prepare_callables();
}

Dictionary LuaController::get_signals_to_register () const {
    return signals_to_register.duplicate();
}

LuaController::LuaController () {
    // This follows Godot's code convention, which didn't use initializer list
    lua_code = "";
    lua = LuaCpp::LuaControllerContext();
    compilation_succeded = false;
    error_message = "";
    prepare_callables();
    signals_to_register = Dictionary();
    
    connect("script_changed", this, "prepare_callables");
}

LuaController::~LuaController() {
}