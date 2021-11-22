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
	RunWithEnvironment(name, globalEnvironment);
}

void LuaControllerContext::RunWithEnvironment(const std::string &name, const LuaEnvironment &env) {
	std::unique_ptr<Engine::LuaState> L = newStateFor(name);
	
	for(const auto &var : env) {
		((std::shared_ptr<Engine::LuaType>) var.second)->PushGlobal(*L, var.first);
	}

	int res = lua_pcall(*L, 0, LUA_MULTRET, 0);
	if (res != LUA_OK ) {
		throw std::runtime_error(lua_tostring(*L,1));
	}

	for(const auto &var : env) {
		((std::shared_ptr<Engine::LuaType>) var.second)->PopGlobal(*L);
	}

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

void LuaControllerContext::setLuaCoreLibraries (int flags) {
	lua_core_libraries = flags;
}

int LuaControllerContext::getLuaCoreLibraries () const {
	return lua_core_libraries;
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
	
} /* namespace LuaCpp */
