/**
 * @file LuaControllerContext.cpp
 * @author Rodrigo Leite (you@domain.com)
 * @brief Adaptation of the LuaController class from Jordan Vrtanoski's "LuaCpp" library
 * @date 2021-11-19
 */

/*
   For this considerable reuse of code from LuaContext.cpp:

   MIT License

   Copyright (c) 2021 Jordan Vrtanoski

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   */


#include <stdexcept>
#include <iostream>
#include <functional>

#include <string>
#include <sstream>

#include "LuaControllerContext.hpp"

namespace LuaCpp {

std::unique_ptr<Engine::LuaState> LuaControllerContext::newState() {
	return newState(globalEnvironment);
}

std::unique_ptr<Engine::LuaState> LuaControllerContext::newState(const LuaEnvironment &env) {
	std::unique_ptr<Engine::LuaState> L = std::make_unique<Engine::LuaState>();
	
	openLibs(*L, getLuaCoreLibraries());
	
	for(const auto &lib : libraries ) {
		((std::shared_ptr<Registry::LuaLibrary>) lib.second)->RegisterFunctions(*L);
	}
	for(const auto &var : env) {
		((std::shared_ptr<Engine::LuaType>) var.second)->PushGlobal(*L, var.first);
	}
	// std::cout << "em newState, começa loop sobre registryVariables\n";
	for(const auto &var : registryVariables) {
		// std::cout << "em newState, empurra a variável de registro: " << var.first<< std::endl;
		((std::shared_ptr<Engine::LuaType>) var.second)->PushValue(*L);
		std::string name = var.first;
		// std::cout << "em newState, lua_set_field pra variável de registro: " << var.first<< std::endl;
		lua_setfield(*L, LUA_REGISTRYINDEX, name.c_str());
		lua_pop(*L, 1);
	}
	lua_pushstring(*L, std::string(Version).c_str());
	lua_setglobal(*L, "_luacppversion");

	return L;
}

std::unique_ptr<Engine::LuaState> LuaControllerContext::newStateFor(const std::string &name) {
	return newStateFor(name, globalEnvironment);
}

std::unique_ptr<Engine::LuaState> LuaControllerContext::newStateFor(const std::string &name, const LuaEnvironment &env) {
	if (registry.Exists(name)) {
		std::unique_ptr<Registry::LuaCodeSnippet> cs = registry.getByName(name);
		std::unique_ptr<Engine::LuaState> L = newState(env);
		cs->UploadCode(*L);
		return L;	
	}	
	throw std::runtime_error("Error: The code snipped not found ...");
}

void LuaControllerContext::CompileString(const std::string &name, const std::string &code) {
	registry.CompileAndAddString(name, code);
}

void LuaControllerContext::CompileString(const std::string &name, const std::string &code, bool recompile) {
	registry.CompileAndAddString(name, code, recompile);
}


void LuaControllerContext::CompileFile(const std::string &name, const std::string &fname) {
	registry.CompileAndAddFile(name,fname);
}

void LuaControllerContext::CompileFile(const std::string &name, const std::string &fname, bool recompile) {
	registry.CompileAndAddFile(name,fname, recompile);
}


void LuaControllerContext::CompileStringAndRun(const std::string &code) {
	registry.CompileAndAddString("default", code, true);
	Run("default");
}

void LuaControllerContext::CompileFileAndRun(const std::string &code) {
	registry.CompileAndAddFile("default", code, true);
	Run("default");
}

void LuaControllerContext::Run(const std::string &name) {
	// std::cout << "Entering LuaControllerContext::Run()\n";
	RunWithEnvironment(name, globalEnvironment);
	// std::cout << "Leaving LuaControllerContext::Run()\n";
}

void LuaControllerContext::RunWithEnvironment(const std::string &name, const LuaEnvironment &env) {
	std::unique_ptr<Engine::LuaState> L = newStateFor(name);
	
	for(const auto &var : env) {
		((std::shared_ptr<Engine::LuaType>) var.second)->PushGlobal(*L, var.first);
	}

	int mask = 0;
	mask |= (max_lines != 0 ? LUA_MASKLINE : 0); // Checks if counts lines
	mask |= (max_count != 0 ? LUA_MASKCOUNT : 0); // Checks if counts commands

	// std::cout << "hook mask is defined as: " << mask << std::endl;
		
	// std::cout << "call to lua_sethook()\n";
	lua_sethook(*L, hook, mask, count_interval); 
	// std::cout << "call to setTimeoutInfo(0, "<<max_lines<<", 0, "<<max_count<<")\n";
	setTimeoutInfo(*L, 0, max_lines, 0, max_count);

	// std::cout << "RunWithEnvironment: logo antes da lua_pcall\n";
    int res = lua_pcall(*L, 0, LUA_MULTRET, 0);
	// std::cout << "RunWithEnvironment: logo depois da lua_pcall, res = "<<res<<std::endl;

	int lines, max_l, count, max_c;
	getTimeoutInfo(*L, lines, max_l, count, max_c);
	// std::cout << "Total lines: " << lines << " with max = "<< max_l <<"\nTotal count: " << count << " with max = "<< max_c << std::endl;

	if (res != LUA_OK ) {
		// std::cout << "Em RunWithEnvironment will call lua_tostring(L,-1)\n";
		std::string message( lua_tostring(*L, -1) );
		// std::cout << "Em RunWithEnvironment called lua_tostring(L,-1)\n";
		if ( message.find("[TIMEOUT]") == std::string::npos
			&& message.find("[FAILED CALL]") == std::string::npos
			&& message.find("[INTERRUPTED]") == std::string::npos)
			// If error message is not of a TIMEOUT, FAILED CALL, or INTERRUPTED,
			//  assembles a runtime_error message
			message = std::string("[RUNTIME ERROR] : ") + message;
		// std::cout << "Em RunWithEnvironment will throw the message:"<<message<<std::endl;
		throw std::runtime_error(message);
	}
	// std::cout << "Poping global variables\n";
	for(const auto &var : env) {
		((std::shared_ptr<Engine::LuaType>) var.second)->PopGlobal(*L);
	}
	// std::cout << "Poping registry variables\n";
	for(const auto &var : registryVariables) {
		std::string name = var.first;
		// std::cout << "\tPoping registry variable named: "<<name<<std::endl;
		lua_getfield(*L, LUA_REGISTRYINDEX, name.c_str());
		((std::shared_ptr<Engine::LuaType>) var.second)->PopValue(*L);
		lua_pop(*L,1);
	}
	// std::cout << "Leaving RunWithEnvironment\n";
	return;
}
		
void LuaControllerContext::AddLibrary(std::shared_ptr<Registry::LuaLibrary> &library) {
	libraries[library->getName()] = std::move(library);
}

void LuaControllerContext::AddGlobalVariable(const std::string &name, std::shared_ptr<Engine::LuaType> var) {
	globalEnvironment[name] = std::move(var);
}

std::shared_ptr<Engine::LuaType> &LuaControllerContext::getGlobalVariable(const std::string &name) {
	return globalEnvironment[name];
}

void LuaControllerContext::AddRegistryVariable(const std::string &name, std::shared_ptr<Engine::LuaType> var) {
	registryVariables[name] = std::move(var);
}

std::shared_ptr<Engine::LuaType> &LuaControllerContext::getRegistryVariable(const std::string &name) {
	return registryVariables[name];
}

void LuaControllerContext::setLuaCoreLibraries (int flags) {
	lua_core_libraries = flags;
}

int LuaControllerContext::getLuaCoreLibraries () const {
	return lua_core_libraries;
}

void LuaControllerContext::setMaxLines (int maximum) {
	max_lines = maximum;
}

void LuaControllerContext::setMaxCount (int maximum) {
	max_count = maximum;
}

void LuaControllerContext::setCountInterval (int interval) {
	count_interval = interval;
}

/* Serviu de referência luaL_openlibs em linit.c, mas este código é só muito mais feio e hard-coded */
void openLibs (Engine::LuaState &L, int lib_flags) {
	if (lib_flags == LIB_NONE)
		return;
	
	if (lib_flags == LIB_ALL) {
		luaL_openlibs(L);
		return;
	}
	if ( lib_flags & LIB_BASE) {
		luaL_requiref(L, "_G", luaopen_base, 1);
		lua_pop(L, 1);
	}
	if ( lib_flags & LIB_COROUTINE) {
		luaL_requiref(L, "coroutine", luaopen_coroutine, 1);
		lua_pop(L, 1);
	}
	if ( lib_flags & LIB_TABLE) {
		luaL_requiref(L, "table", luaopen_table, 1);
		lua_pop(L, 1);
	}
	if ( lib_flags & LIB_IO) {
		luaL_requiref(L, "io", luaopen_io, 1);
		lua_pop(L, 1);
	}
	if ( lib_flags & LIB_OS) {
		luaL_requiref(L, "os", luaopen_os, 1);
		lua_pop(L, 1);
	}
	if ( lib_flags & LIB_STRING) {
		luaL_requiref(L, "string", luaopen_string, 1);
		lua_pop(L, 1);
	}
	if ( lib_flags & LIB_UTF8) {
		luaL_requiref(L, "utf8", luaopen_utf8, 1);
		lua_pop(L, 1);
	}
	if ( lib_flags & LIB_MATH) {
		luaL_requiref(L, "math", luaopen_math, 1);
		lua_pop(L, 1);
	}
	if ( lib_flags & LIB_DEBUG) {
		luaL_requiref(L, "debug", luaopen_debug, 1);
		lua_pop(L, 1);
	}
	if ( lib_flags & LIB_PACKAGE) {
		luaL_requiref(L, "package", luaopen_package, 1);
		lua_pop(L, 1);
	}
}
	
void hook (lua_State *L, lua_Debug *ar) {

	int lines, max_lines, count, max_count;
    getTimeoutInfo(L, lines, max_lines, count, max_count);
    

	// std::cout << "Hook called with ar->event == ";
	switch (ar->event)
	{
	case LUA_HOOKCOUNT:
		// std::cout << "LUA_HOOKCOUNT\n";
		count += 1;
		break;
	case LUA_HOOKLINE:
		// std::cout << "LUA_HOOKLINE\n";
		lines += 1;
		break;
	default:
		// std::cout << "default case\n";
		break;
	}

	bool cond_lines = max_lines > 0 && lines >= max_lines;
	bool cond_count = max_count > 0 && count >= max_count;

	if ( cond_lines ) {
		// std::cout << "cond_lines was satisfied, jumping...\n";
		luaL_error(L, "[TIMEOUT] : Exceeded line count. Counted %d, limit was %d", lines, max_lines);
	}
	if ( cond_count ) {
		// std::cout << "cond_count was satisfied, jumping...\n";
		luaL_error(L, "[TIMEOUT] : Exceeded command count. Counted %d, limit was %d", count, max_count);
	}

	// std::cout << "no condition was satisfied, won't throw error.\n";
    setTimeoutInfo(L, lines, max_lines, count, max_count);

	// std::cout << "No Hook, logo antes de lua_get_field pra callback de force_stop\n";
	// Check if ForceStop was set to true
    lua_getfield(L, LUA_REGISTRYINDEX, "lua_controller_get_force_stop");
	// std::cout << "No Hook, logo antes do lua_pcall pra callback de force_stop\n";
	int res = lua_pcall(L, 0, 1, 0);
	// std::cout << "No Hook, logo depois do lua_pcall pra callback de force_stop\n";
	if (res != LUA_OK) {
		// Error when calling get_force_stop
		// std::cout << "No Hook, no erro quando chama get_force_stop, logo antes de fazer lua_tostring()\n";
		std::string message = lua_tostring(L,-1);
		// std::cout << "No Hook, no erro quando chama get_force_stop, logo antes de lançar luaL_error()\n";
		luaL_error(L, "[FAILED CALL] : Couldn't call function 'lua_controller_get_force_stop' from the registry. Error returned by pcall : %s", message.c_str());
	}
	// std::cout << "No Hook, logo antes do lua_toboolean()\n";
	bool force_stop = lua_toboolean(L,-1);
	// std::cout << "No Hook, valor recuperado de lua_toboolean() = "<<force_stop<<std::endl;
	lua_pop(L, 1);
	if (force_stop) {
		// std::cout << "No Hook, quando force_stop é true, logo antes de lançar luaL_error()\n";
		luaL_error(L, "[INTERRUPTED] : Execution of the script was manualy interrupted");
	}
}


void setTimeoutInfo (lua_State *L, int lines, int max_lines, int count, int max_count) {
	// std::cout << "entrou em setuptimeout\n";

	lua_pushinteger(L, lines); // lines
	// std::cout << "empurrou lines\n";
	lua_setfield(L, LUA_REGISTRYINDEX, "lua_controller_timeout_lines");
	// std::cout << "setfield para lines\n";

	lua_pushinteger(L, max_lines); // lines
	// std::cout << "empurrou max_lines\n";
	lua_setfield(L, LUA_REGISTRYINDEX, "lua_controller_timeout_max_lines");
	// std::cout << "setfield para max_lines\n";

	lua_pushinteger(L, count); // count
	// std::cout << "empurrou count\n";
	lua_setfield(L, LUA_REGISTRYINDEX, "lua_controller_timeout_count");
	// std::cout << "setfield para count\n";

	lua_pushinteger(L, max_count); // max_count
	// std::cout << "empurrou max_count\n";
	lua_setfield(L, LUA_REGISTRYINDEX, "lua_controller_timeout_max_count");
	// std::cout << "setfield para max_count\n";

	// std::cout << "saindo de setuptimeout\n";
}

void getTimeoutInfo (lua_State *L, int &lines, int &max_lines, int &count, int &max_count) {
	int pops = 0;
	// std::cout << "Entrando em gettimeoutinfo\n";
    
    lua_getfield(L, LUA_REGISTRYINDEX, "lua_controller_timeout_lines");
    // std::cout << "getfield para lines\n";
    lines = lua_tointeger(L, -1);
	pops++;
    // std::cout << "tointeger para lines\n";

    lua_getfield(L, LUA_REGISTRYINDEX, "lua_controller_timeout_max_lines");
    // std::cout << "getfield para max_lines\n";
    max_lines = lua_tointeger(L, -1);
	pops++;
    // std::cout << "tointeger para max_lines\n";
    
    lua_getfield(L, LUA_REGISTRYINDEX, "lua_controller_timeout_count");
    // std::cout << "getfield para count\n";
    count = lua_tointeger(L, -1);
	pops++;
    // std::cout << "tointeger para count\n";
    
    lua_getfield(L, LUA_REGISTRYINDEX, "lua_controller_timeout_max_count");
    // std::cout << "getfield para max_count\n";
    max_count = lua_tointeger(L, -1);
	pops++;
    // std::cout << "tointeger para max_count\n";

    // pops every integer pushed
    lua_pop(L, pops);
}

} /* namespace LuaCpp */
