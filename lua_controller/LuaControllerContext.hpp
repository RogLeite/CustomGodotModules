/**
 * @file LuaControllerContext.hpp
 * @author Rodrigo Leite (you@domain.com)
 * @brief Adaptation of the LuaController class from Jordan Vrtanoski's "LuaCpp" library
 * @version 1.0
 * @date 2021-11-19
 */

/*
   For this considerable reuse of code from LuaContext.hpp:

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


#ifndef LUACPP_LUACONTROLLERCONTEXT_HPP
#define LUACPP_LUACONTROLLERCONTEXT_HPP

#include <memory>

#include <LuaCpp.hpp>

// Forward declaration necessary for friend declaration
class LuaControllerUnitTester;

namespace LuaCpp {

	enum CORE_LIBS_FLAGS {
		LIB_NONE = 0,
		LIB_BASE = 1,
		LIB_COROUTINE = 2,
		LIB_TABLE = 4,
		LIB_IO = 8,
		LIB_OS = 16,
		LIB_STRING = 32,
		LIB_UTF8 = 64,
		LIB_MATH = 128,
		LIB_DEBUG = 256,
		LIB_PACKAGE = 512,
		LIB_ALL = 1023, //< the combination of all libraries
    };

	class LuaControllerContext {
	private:
    	friend class ::LuaControllerUnitTester;
		/**
		 * @brief Name of the context
		 */
		std::string name;

		/**
		 * @brief Repository that will keep the code snippets
		 * 
		 * @see LuaControllerContext::Registry::LuaRegistry
		 */
		Registry::LuaRegistry registry;

		/**
		 * Custom `C` libraries for the session
		 */
		std::map<std::string, std::shared_ptr<Registry::LuaLibrary>> libraries;

		/**
		 * Each bit is a flag that indicates if a specific core Lua library should be opened in LuaState
		 */
		int lua_core_libraries;

		/**
		 *	The values to load onto the context as global
		 */
		LuaEnvironment globalEnvironment;
		/**
		 * @brief The count of commands executed is incremented by 1 every
		 * count_interval commands executed.
		 */
		int count_interval;

		/**
		 * @brief Maximum amount of lines executed
		 * 
		 * If max_lines is 0, the execution cannot timeout by line count.
		 */
		int max_lines;
		/**
		 * @brief Maximum amount of commands counted
		 * 
		 * If max_count is 0, the execution cannot timeout by command count.
		 */
		int max_count;

	public:

		/**
		 * @brief Constructs and empty context
		 *
		 * @details
		 * Creates empty Lua cotext. This is the main entry point
		 * for the communication with the Lua virtual machine
		 * from the high level APIs.
		 */
		LuaControllerContext()
		 : registry()
		 , libraries()
		 , lua_core_libraries(LIB_ALL)
		 , globalEnvironment()
		 , count_interval(10)
		 , max_lines(1e5)
		 , max_count(1e6)
		 {};
		~LuaControllerContext() {};

		/**
		 * @brief Creates new Lua execution state from the context
		 *
		 * @details
		 * Creates new Lua execution state from the context. The
		 * state will be loaded with the standard libraries 
		 * and the libraries that are registered in the context
		 *
		 * The globalEnvironment variables will also be loaded to the context.
		 *
		 * @return Pointer to the LuaState object holding the pointer of the lua_State
		 */
		std::unique_ptr<Engine::LuaState> newState();

		/**
		 * @brief Creates new Lua execution state from the context
		 *
		 * @details
		 * Creates new Lua execution state from the context. The
		 * state will be loaded with the standard libraries 
		 * and the libraries that are registered in the context
		 *
		 * The provided environment will be loaded instead of the 
		 * global environment.
		 *
		 * @param env Variables from this environment will be loaded as global 
		 * variables on the state
		 * 
		 * @return Pointer to the LuaState object holding the pointer of the lua_State
		 */
		std::unique_ptr<Engine::LuaState> newState(const LuaEnvironment &env);

		/**
		 * @brief Creates new Lua execution state from the context and loads a snippet
		 *
		 * @details
		 * Creates new Lua execution state from the context. The
		 * state will be loaded with the standard libraries 
		 * and the libraries that are registered in the context. In addition to the
		 * libraries, the context will be loaded with the selected snippet.
		 * The snippet will be on top of the lua stack (position 1) and is ready
		 * for execution, or definition of a global name
		 *
		 * The variables defined in the globalEnvironment will be loaded on the
		 * state.
		 *
		 * If the name is not found, the method will throw exception
		 *
		 * @param name Name of the snippet to be loaded
		 *
		 * @return Pointer to the LuaState object holding the pointer of the lua_State
		 */
	        std::unique_ptr<Engine::LuaState> newStateFor(const std::string &name);
		
		/**
		 * @brief Creates new Lua execution state from the context and loads a snippet
		 *
		 * @details
		 * Creates new Lua execution state from the context. The
		 * state will be loaded with the standard libraries 
		 * and the libraries that are registered in the context. In addition to the
		 * libraries, the context will be loaded with the selected snippet.
		 * The snippet will be on top of the lua stack (position 1) and is ready
		 * for execution, or definition of a global name
		 *
		 * The varibles provided with the `LuaEnvironment` will also be loaded on
		 * the state as a global variables.
		 *
		 * If the name is not found, the method will throw exception
		 *
		 * @param name Name of the snippet to be loaded
		 * @param env Variables from this environment will be loaded as global 
		 * variables on the state
		 *
		 * @return Pointer to the LuaState object holding the pointer of the lua_State
		 */
	        std::unique_ptr<Engine::LuaState> newStateFor(const std::string &name, const LuaEnvironment &env);

		/**
		 * @brief Compiles a string containing Lua code and adds it to the repository
		 *
		 * @details
		 * Compiles a string containing Lua code and adds the compiled binary to the
		 * repository as a LuaCodeSnippet. The code is registered in Lua engine and in 
		 * the repository under the same name. If the compilation fails, the function
		 * will throw an `std::logic_error` with the error code received from Lua engin
		 *
		 * @param name Name under which the snippet is registered in the repository
		 * @param code A valid Lua code that will be compiled
		 */
		void CompileString(const std::string &name, const std::string &code);
		
		/**
		 * @brief Compiles a string containing Lua code and adds it to the repository
		 *
		 * @details
		 * Compiles a string containing Lua code and adds the compiled binary to the
		 * repository as a LuaCodeSnippet. The code is registered in Lua engine and in 
		 * the repository under the same name. If the compilation fails, the function
		 * will throw an `std::logic_error` with the error code received from Lua engin
		 *
		 * @param name Name under which the snippet is registered in the repository
		 * @param code A valid Lua code that will be compiled
		 * @param recompile if true, the new version of the code will be active
		 */
		void CompileString(const std::string &name, const std::string &code, bool recompile);

		/**
		 * @brief Compiles a fle containing Lua code and adds it to the registry
		 *
		 * @details
		 * Compiles a file containing Lua code and adds the compiled binary to the
		 * repository as a LuaCodeSnippet. The code is registered in Lua engine and in 
		 * the repository under the same name. If the compilation fails, the function
		 * will throw an `std::logic_error` with the error code received from Lua engin
		 *
		 * @param name Name under which the snippet is registered in the registry
		 * @param fname path to the file where the code is stored
		 */
		void CompileFile(const std::string &name, const std::string &fname);

		/**
		 * @brief Compiles a fle containing Lua code and adds it to the registry
		 *
		 * @details
		 * Compiles a file containing Lua code and adds the compiled binary to the
		 * repository as a LuaCodeSnippet. The code is registered in Lua engine and in 
		 * the repository under the same name. If the compilation fails, the function
		 * will throw an `std::logic_error` with the error code received from Lua engin
		 *
		 * @param name Name under which the snippet is registered in the registry
		 * @param fname path to the file where the code is stored
		 * @param recompile if set to true, the new code will replace the old in the registry
		 */
		void CompileFile(const std::string &name, const std::string &fname, bool recompile);

		/**
		 * @brief Compiles a code snippet and runs
		 *
		 * @details
		 * The code sneippet will be compiled under the name `default` and
		 * will be executed in the context.
		 *
		 * This is the highest level function and is ment for prototyping
		 * use since the code is recompiled with each call. For use in
		 * project, use `Run` instead
		 *
		 * @see Run(std::string name)
		 *
		 * @param code Code to run
		 */
		void CompileStringAndRun(const std::string &code);

		/**
		 * @brief Compiles the file and runs
		 *
		 * @details
		 * The file will be compiled under the name `default` and
		 * will be executed in the context.
		 *
		 * This is the highest level function and is ment for prototyping
		 * use since the code is recompiled with each call. For use in
		 * project, use `Run` instead
		 *
		 * @see Run(std::string name)
		 *
		 * @param code Code to run
		 */
		void CompileFileAndRun(const std::string &code);
		
		/**
		 * @brief Run a code snippet
		 *
		 * @details
		 * Run a snippet that was previously compiled and stored in the registry
		 *
		 * @param name Name under which the snippet is registered
		 */
		void Run(const std::string &name);

		/**
		 * @brief Run a code snippet with a given `lua` global table
		 *
		 * @details
		 * Run a snippet that was previously compiled and stored in the registry
		 * with a given environment (lua global variables)
		 * 
		 * Sets lua_Hook for LUA_HOOKCOUNT and LUA_HOOKLINE, if max_count or max_lines are defined
		 * The hook serves as a timeout for the execution.
		 *
		 * @param name Name under which the snippet is registered
		 * @param env Variables from this environment will be loaded as global 
		 * variables on the state
		 */
		void RunWithEnvironment(const std::string &name, const LuaEnvironment &env);

		/**
		* @brief Add a `C` library to the context
		*
		* @details
		* Adds a `C` library to the context. The library will be loaded whenever a
		* new state is created fron the context.
		*
		* @param library The library containing `C` functions
		*/
		void AddLibrary(std::shared_ptr<Registry::LuaLibrary> &library);

		/**
		 * @brief Add a global variable
		 *
		 * @details
		 * Add global variable. The variables will be populated in
		 * the LuaState before the state is returned from the 
		 * newState() and newStateFor() methods.
		 *
		 * The variables will be accessible from the lua enging
		 * under the registered name.
		 *
		 * @param name name of the global variable
		 * @param var the variable
		 */
		void AddGlobalVariable(const std::string &name, std::shared_ptr<Engine::LuaType> var);

		/**
		 * @brief Retrurns the shared pointer to a global variable
		 *
		 * @details
		 * Returns a shared pointer to the global variable. The variable
		 * should be reinterpreted as the proper type.
		 *
		 * @param name Name of the global variable
		 *
		 * @returns
		 * The shared pointer of the global variable
		 */
		std::shared_ptr<Engine::LuaType> &getGlobalVariable(const std::string &name);

		/**
		 * @brief Set the lua_core_libraries flags
		 * 
		 * @param flags 
		 */
		void setLuaCoreLibraries (int flags);
		
		/**
		 * @brief Get the lua_core_libraries flags
		 */
		int getLuaCoreLibraries () const;

		/**
    	 * @brief Maximum amount of lines executed
		 * 
		 * @param maximum 
		 */
		void setMaxLines (int maximum);

		/**
  	   * @brief Maximum amount of commands counted
		 * 
		 * @param maximum 
		 */
		void setMaxCount (int maximum);

		/**
		 * @brief The count of commands executed is incremented by 1 every
		 * count_interval commands executed.
		 * 
		 * @param interval 
		 */
		void setCountInterval (int interval);
		
	};

	/**
	 * @brief Opens the chosen core Lua libraries inside the passed LuaState
	 * 
	 * @param L The state where the libraries will be opened
	 * @param lib_flags Bitwise OR of the flags from CORE_LIBS_FLAGS
	 */
	void openLibs (Engine::LuaState &L, int lib_flags);


	/**
	 * @brief The lua_Hook to be set on RunWhitEnvironment()
	 * 
	 * hook() increments counters (which ones depend on the LUA_MASK used) and calls luaL_error on L
	 * if any one of them exceedes they're limit. If a limit is set to 0, it's counting is disabled.
	 * 
	 * @param L 
	 * @param ar 
	 */
	void hook (lua_State *L, lua_Debug *ar);
	
	void setTimeoutInfo (lua_State *L, int lines, int max_lines, int count, int max_count);

	void getTimeoutInfo (lua_State *L, int &lines, int &max_lines, int &count, int &max_count);
}

/**
 * @example example_helloworld.cpp
 */
#endif // LUACPP_LUACONTROLLERCONTEXT_HPP
