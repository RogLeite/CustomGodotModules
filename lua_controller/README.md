# LuaController
This Godot module provides the Node `LuaController`. This Node executes a String as a Lua script, and user of `LuaController` can define methods to be invoked by the Lua script, so changes can occur in the game's Scene.

## How to compile, install, and execute Lua, LuaCpp, and Godot on Linux

### Lua5.3
 - Download from http://www.lua.org/ftp/ the source for Lua5.3.6 and uncompress it.
 - In `src/Makefile` define the variable "MYCFLAGS" as "PIC":
```
MYCFLAGS = -fPIC
```
 - In Lua's root directory call:
 ```
 make
 ```
 - Then install the library by calling:
 ```
 make install
 ```
 - With that, the library Lua5.3.6 should be installed with the name `lua` in the directory `/usr/local/lib`.
 - If any problems or compilation errors occur, check lua's [official installation instructions](https://www.lua.org/manual/5.3/readme.html). There they explain the possible need for the `readline` package, and how to install Lua locally.

### LuaCpp 
 - Download the source code from jordanvrtanoski's repository [https://github.com/jordanvrtanoski/luacpp](https://github.com/jordanvrtanoski/luacpp). Uncompress it.
 - For it's tests and documentation, LuaCpp uses "valgrind" and "doxygen". If needed, install them with a package manager.
 - To compile and install, follow the `README.md` like this:
 ```
 mkdir build
 cd build
 cmake ../Source
 make -j 4
 make install
 ```
 - If you want to use your local Lua installation, during the "cmake" step, use the following command (`<install>` is the absolute path to the directory lua was installed):
 ```
 cmake ../Source -D LUA_INCLUDE_DIR=<install>/include -D LUA_LIBRARIES=<install>/lib/liblua.a
 ```
 - Finally, for Godot's compilation to find the installation of LuaCpp (`libluacpp.so`), run (may need sudo privilege):
 ```
 ldconfig
 ```
 ### Godot's compilation + LuaController
 - Download the source code for the module LuaController from the [repository](https://github.com/RogLeite/CustomGodotModules)
 - Download Godot's  source code from the [repository](https://github.com/godotengine/godot)
 - IMPORTANT: Switch to branch `3.4`:
 ```
 git checkout 3.4
 ```
 - Follow the steps and recomendations in Godot's official documentation for your specific linux distribution: [https://docs.godotengine.org/en/3.4/development/compiling/compiling_for_x11.html](https://docs.godotengine.org/en/3.4/development/compiling/compiling_for_x11.html)
 - To compile the module LuaController, go to Godot's root directory. Run the following scons command:
 ```
 scons custom_modules=”path-to/CustomGodotModules” p=x11 target=debug
 ```
 - Before launching Godot, inform it where the binary for the module is:
 ```
 export _LIBRARY_PATH=”<path-to-godot-root>/bin/”
 ```
 - Then run Godot's binary:
 ```
 ./bin/godot*
 ```

### Run the automated tests for LuaController
 - Go to the directory of the tester project
 ```
 cd lua_controller/test
 ```
 - Give execution permission to `runtest.sh`
 ```
 chmod u+x runtest.sh
 ```
 - Set the environment variable indicating where Godot's binary is:
 ```
 export GODOT_BIN="path-to-godot-root/bin/godot<fill-file-name>" 
 ```
 - Run all test suites from the command-line:
 ```
 ./runtest.sh -a test/ 
 ```
 
 ## Build the documentation
 To build the documentation into the `docs` directory, run doxygen:
 ```
 doxygen Doxyfile
 ```