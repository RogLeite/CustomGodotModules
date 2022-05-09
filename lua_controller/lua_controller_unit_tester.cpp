#include "lua_controller_unit_tester.h"

#include <map>
#include <stdexcept>
#include <memory>

#include "core/array.h"

#include <LuaCpp.hpp>
#include "lua_callable.h"
#include "LuaControllerContext.hpp"
#include "lua_controller.h"

/**
 * @brief A Suite collects the error messages, stores the name of the suite, and counts the tests
 * All these variables so the error messages can be very explicit.
 */
#define START_SUITE(func) \
    Array errors;\
    String func_name(func);\
    int test_count = 0;

/**
 * @brief NEW_TEST starts a new test
 * 
 * A test has it's own number and error message.
 */
#define NEW_TEST(test_name) \
    test_count += 1;\
    String error_msg(vformat("[FAILURE] %s @ TEST %d \'%s\' : ", func_name, test_count, test_name));/*  \
    print_line(vformat("Starting test number %d : \"%s\"", test_count, test_name)); */

/**
 * @brief 
 * UNIT_ASSERT follows the convention from Godot's asserts: if condition is true, then it's an error
 * 
 * On a failed assert, the error message stores is a concatenation of the test's error message and
 * the description passed to UNIT_ASSERT 
 */
#define UNIT_ASSERT(cond, msg) \
    if ( ( cond ) ) { errors.push_back(error_msg + String(msg)); };

/**
 * @brief END_SUITE returns the error array, if there are any. If not, returns true
 */
#define END_SUITE \
    return errors; 

/**
 * @brief Helper to test LuaCpp::openLibs()
 * 
 * UNIT_CHECK_LIB is a helper to check if each individual library can be opened correctly on a LuaState
 */
#define UNIT_CHECK_LIB(flag, var_name, message) \
        openLibs(L, flag); \
        lua_getglobal(L, var_name); \
        UNIT_ASSERT(lua_isnil(L, -1), message); \
        lua_pop(L, 1);

/**
 * @brief arg_or_default helps keep methods safe when called with fewer arguments than expected
 */
namespace {        
    inline Variant arg_or_default(Variant arg, Variant value) {
        return arg.get_type() == Variant::NIL ? value : arg;
    }
}

void LuaControllerUnitTester::_bind_methods () {
    ClassDB::bind_method(D_METHOD("_true"), &LuaControllerUnitTester::_true);
    ClassDB::bind_method(D_METHOD("_not"), &LuaControllerUnitTester::_not);
    ClassDB::bind_method(D_METHOD("_not_unsafe"), &LuaControllerUnitTester::_not_unsafe);
    ClassDB::bind_method(D_METHOD("_and"), &LuaControllerUnitTester::_and);
    ClassDB::bind_method(D_METHOD("_get_force_stop_false"), &LuaControllerUnitTester::_get_force_stop_false);
    ClassDB::bind_method(D_METHOD("lua_callable"), &LuaControllerUnitTester::_lua_callable);
    ClassDB::bind_method(D_METHOD("lua_controller_context"), &LuaControllerUnitTester::_lua_controller_context);
    ClassDB::bind_method(D_METHOD("lua_controller"), &LuaControllerUnitTester::_lua_controller);
}


Variant LuaControllerUnitTester::_true () {
    return true;
}
Variant LuaControllerUnitTester::_not (Variant arg) {
    return !arg_or_default(arg, false);
}
Variant LuaControllerUnitTester::_not_unsafe (bool arg) {
    return !arg;
}
Variant LuaControllerUnitTester::_and (Variant arg1, Variant arg2) {
    return arg_or_default(arg1, false) && arg_or_default(arg2, false);
}

Variant LuaControllerUnitTester::_get_force_stop_false () {
    return false;
}

Array LuaControllerUnitTester::_lua_callable () {
    
    // For ease of access
    List<MethodInfo> method_list; 
    get_method_list(&method_list);
    std::map<String, MethodInfo> methods{};
    for (List<MethodInfo>::Element *E = method_list.front(); E; E = E->next()) {
        methods[E->get().name] = E->get();
    }

    START_SUITE("lua_callable");

    {
        NEW_TEST("Stores ObjectID correctly");
        LuaCallable o(get_instance_id(), MethodInfo(),
                        [=](Variant::CallError::Error err, String msg){} );
        UNIT_ASSERT( o.object_id != get_instance_id(), "Incorrect Object_ID stored");
    }
    {
        NEW_TEST("Stores MethodInfo correctly");
     
        //Stores a empty MethodInfo
        MethodInfo empty = MethodInfo();
        LuaCallable o1(get_instance_id(), empty,
                        [=](Variant::CallError::Error err, String msg){} );
        UNIT_ASSERT( !(o1.info == empty) , "Stored MethodInfo() incorrectly");
        
        // Stores _true's MethodInfo 
        LuaCallable o2(get_instance_id(), methods["_true"] ,
                        [=](Variant::CallError::Error err, String msg){} );

        UNIT_ASSERT( !(o2.info == methods["_true"]), "Stored MethodInfo is incorrect");
    }
    {
        NEW_TEST("Tests get_method_name");
        LuaCallable o(get_instance_id(), MethodInfo("NameOfMethod"),
                        [=](Variant::CallError::Error err, String msg){} );
        UNIT_ASSERT( o.get_method_name().casecmp_to("NameOfMethod") != 0, "String received is not identical to stored");
    }
    {
        NEW_TEST("Stores ErrorHandler correctly");
        bool handler_called = false;
        LuaCallable o(get_instance_id(), MethodInfo("NameOfMethod"),
                        [=, &handler_called](Variant::CallError::Error err, String msg) {handler_called = true;} );
        o.handler(Variant::CallError::CALL_OK, "Call ok");
        UNIT_ASSERT(!handler_called, "ErrorHandler was not stored correctly");
    }
    {
        NEW_TEST("Execute() works with correct method call");
        // Test for method _true 
        {
            LuaCallable o(get_instance_id(), methods["_true"],
                [=](Variant::CallError::Error err, String msg){});
            
            LuaCpp::Engine::LuaState L;
            // Pushes a trash value on the stack because LuaCallable ignores the first element
            lua_pushstring(L, "Trash string");
            o.Execute(L);
            //Expects a boolean value of true on the stack
            UNIT_ASSERT(!lua_isboolean(L, -1), "Execute() for method _true didn't return a boolean");
            UNIT_ASSERT((bool)lua_toboolean(L, -1) != true, "Execute() for method _true didn't the correct boolean value (expected true)");
        }
        // Test for method _not
        {
            LuaCallable o(get_instance_id(), methods["_not"],
                [=](Variant::CallError::Error err, String msg){});
            
            LuaCpp::Engine::LuaState L;
            // Pushes a trash value on the stack because LuaCallable ignores the first element
            lua_pushstring(L, "Trash string");
            // Pushes first argument
            lua_pushboolean(L, 0);
            o.Execute(L);
            //Expects a boolean value of true on the stack
            UNIT_ASSERT(!lua_isboolean(L, -1), "Execute() for method _not didn't return a boolean");
            UNIT_ASSERT((bool)lua_toboolean(L, -1) != true, "Execute() for method _not didn't the correct boolean value (expected true)");
        }
        // Test for method _and
        {
            LuaCallable o(get_instance_id(), methods["_and"],
                [=](Variant::CallError::Error err, String msg){});
            
            LuaCpp::Engine::LuaState L;
            // Pushes a trash value on the stack because LuaCallable ignores the first element
            lua_pushstring(L, "Trash string");
            // Pushes first argument
            lua_pushboolean(L, 1);
            // Pushes second argument
            lua_pushboolean(L, 1);
            o.Execute(L);
            //Expects a boolean value of true on the stack
            UNIT_ASSERT(!lua_isboolean(L, -1), "Execute() for method _and didn't return a boolean");
            UNIT_ASSERT((bool)lua_toboolean(L, -1) != true, "Execute() for method _and didn't the correct boolean value (expected true)");
        }
    }
    {
        NEW_TEST("Test Execute() with too many arguments (should not raise error)");
        // Is set to true by the ErrorHandler
        bool handler_called = false;
        // Indicates if the error was the one tested for
        bool was_corresponding_error = false;
        // Testing with method _true
        LuaCallable o(get_instance_id(), methods["_true"],
            [=, &handler_called, &was_corresponding_error](Variant::CallError::Error err, String msg) {
                handler_called = true;
                if (err == Variant::CallError::CALL_ERROR_TOO_MANY_ARGUMENTS) 
                    was_corresponding_error = true;
            });
        
        LuaCpp::Engine::LuaState L;
        // Pushes a trash value on the stack because LuaCallable ignores the first element
        lua_pushstring(L, "Trash string");
        // Pushes first argument, should be ignored
        lua_pushboolean(L, 0);
        o.Execute(L);
        //Expects a boolean value of true on the stack
        UNIT_ASSERT( handler_called, "ErrorHandler was called from Execute()" );
        UNIT_ASSERT( was_corresponding_error, "ErrorHandler received CALL_ERROR_TOO_MANY_ARGUMENTS from Execute()" );
        UNIT_ASSERT(!lua_isboolean(L, -1), "Execute() for method _not didn't return a boolean");
        UNIT_ASSERT((bool)lua_toboolean(L, -1) != true, "Execute() for method _not didn't the correct boolean value (expected true)");
    }
    {
        NEW_TEST("Test Execute() with too few arguments (should not raise error)");
        // Is set to true by the ErrorHandler
        bool handler_called = false;
        // Indicates if the error was the one tested for
        bool was_corresponding_error = false;
        // Testing with method _not
        LuaCallable o(get_instance_id(), methods["_not"],
            [=, &handler_called, &was_corresponding_error](Variant::CallError::Error err, String msg) {
                handler_called = true;
                if (err == Variant::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS) 
                    was_corresponding_error = true;
            });
        
        LuaCpp::Engine::LuaState L;
        // Pushes a trash value on the stack because LuaCallable ignores the first element
        lua_pushstring(L, "Trash string");
        // Pushes first argument, should be ignored
        lua_pushboolean(L, 0);
        o.Execute(L);
        //Expects a boolean value of true on the stack
        UNIT_ASSERT( handler_called, "ErrorHandler was called from Execute()" );
        UNIT_ASSERT( was_corresponding_error, "ErrorHandler received CALL_ERROR_TOO_FEW_ARGUMENTS from Execute()" );
        UNIT_ASSERT(!lua_isboolean(L, -1), "Execute() for method _not didn't return a boolean");
        UNIT_ASSERT((bool)lua_toboolean(L, -1) != true, "Execute() for method _not didn't the correct boolean value (expected true)");
    }
    {
        NEW_TEST("Test Execute() safe function and invalid argument (should not raise error)");
        // Is set to true by the ErrorHandler
        bool handler_called = false;
        // Indicates if the error was the one tested for
        bool was_corresponding_error = false;
        // Testing with method _true
        LuaCallable o(get_instance_id(), methods["_not"],
            [=, &handler_called, &was_corresponding_error](Variant::CallError::Error err, String msg) {
                handler_called = true;
                if (err == Variant::CallError::CALL_ERROR_INVALID_ARGUMENT) 
                    was_corresponding_error = true;
            });
        
        LuaCpp::Engine::LuaState L;
        // Pushes a trash value on the stack because LuaCallable ignores the first element
        lua_pushstring(L, "Trash string");
        o.Execute(L);
        //Expects a boolean value of true on the stack
        UNIT_ASSERT( handler_called, "ErrorHandler was called from Execute()" );
        UNIT_ASSERT( was_corresponding_error, "ErrorHandler received CALL_ERROR_INVALID_ARGUMENT from Execute()" );
        UNIT_ASSERT(!lua_isboolean(L, -1), "Execute() for method _not didn't return a boolean");
        UNIT_ASSERT((bool)lua_toboolean(L, -1) != true, "Execute() for method _not didn't the correct boolean value (expected true)");
    }
    {
        NEW_TEST("Test Execute() unsafe function and invalid argument (should handle error)");
        // Is set to true by the ErrorHandler
        bool handler_called = false;
        // Indicates if the error was the one tested for
        bool was_corresponding_error = false;
        // Testing with method _not_unsafe
        LuaCallable o(get_instance_id(), methods["_not_unsafe"],
            [=, &handler_called, &was_corresponding_error](Variant::CallError::Error err, String msg) {
                handler_called = true;
                if (err == Variant::CallError::CALL_ERROR_INVALID_ARGUMENT) 
                    was_corresponding_error = true;
            });
        
        LuaCpp::Engine::LuaState L;
        // Pushes a trash value on the stack because LuaCallable ignores the first element
        lua_pushstring(L, "Trash string");
        o.Execute(L);
        
        UNIT_ASSERT( !handler_called, "ErrorHandler wasn't called from Execute()" );
        UNIT_ASSERT( !was_corresponding_error, "ErrorHandler received from Execute() incorrect CALL_ERROR" );
    }
    {
        NEW_TEST("Test Execute() with invalid method (should handle error)");
        // Is set to true by the ErrorHandler
        bool handler_called = false;
        // Indicates if the error was the one tested for
        bool was_corresponding_error = false;
        // Testing with non-existent method
        LuaCallable o(get_instance_id(), MethodInfo("InvalidMethod"),
            [=, &handler_called, &was_corresponding_error](Variant::CallError::Error err, String msg) {
                handler_called = true;
                if (err == Variant::CallError::CALL_ERROR_INVALID_METHOD) 
                    was_corresponding_error = true;
            });
        
        LuaCpp::Engine::LuaState L;
        // Pushes a trash value on the stack because LuaCallable ignores the first element
        lua_pushstring(L, "Trash string");
        o.Execute(L);

        UNIT_ASSERT( !handler_called, "ErrorHandler wasn't called from Execute()" );
        UNIT_ASSERT( !was_corresponding_error, "ErrorHandler received from Execute() incorrect CALL_ERROR" );
    }

    END_SUITE;

}

Array LuaControllerUnitTester::_lua_controller_context () {

    using namespace LuaCpp;

    // For ease of access
    List<MethodInfo> method_list; 
    get_method_list(&method_list);
    std::map<String, MethodInfo> methods{};
    for (List<MethodInfo>::Element *E = method_list.front(); E; E = E->next()) {
        methods[E->get().name] = E->get();
    }

    START_SUITE("lua_controller_context");

    {
        NEW_TEST("Test default value for lua_core_libraries");
        LuaControllerContext ctx;
        UNIT_ASSERT( ctx.lua_core_libraries != LIB_ALL, "LuaControllerContext didn't initialize with all core libraries enabled" );
    }
    {
        NEW_TEST("Test setLuaCoreLibraries");
        LuaControllerContext ctx;
        ctx.setLuaCoreLibraries(LIB_BASE | LIB_STRING);
        UNIT_ASSERT(ctx.lua_core_libraries != 33, "Didn't set the correct bits");
    }
    {
        NEW_TEST("Test getLuaCoreLibraries");
        LuaControllerContext ctx;
        ctx.lua_core_libraries = 192;
        int value = ctx.getLuaCoreLibraries();
        UNIT_ASSERT( value != 192, vformat("Didn't get the correct value. Expected 192, got %d", value));
    }
    {
        NEW_TEST("Test LuaCpp::openLibs()");
        Engine::LuaState L;
        // Checks if each individual library can be opened
        UNIT_CHECK_LIB(LIB_BASE, "print", "lib base wasn't opened correctly");
        UNIT_CHECK_LIB(LIB_COROUTINE, "coroutine", "lib coroutine wasn't opened correctly");
        UNIT_CHECK_LIB(LIB_TABLE, "table", "lib table wasn't opened correctly");
        UNIT_CHECK_LIB(LIB_IO, "io", "lib io wasn't opened correctly");
        UNIT_CHECK_LIB(LIB_OS, "os", "lib os wasn't opened correctly");
        UNIT_CHECK_LIB(LIB_STRING, "string", "lib string wasn't opened correctly");
        UNIT_CHECK_LIB(LIB_UTF8, "utf8", "lib utf8 wasn't opened correctly");
        UNIT_CHECK_LIB(LIB_MATH, "math", "lib math wasn't opened correctly");
        UNIT_CHECK_LIB(LIB_DEBUG, "debug", "lib debug wasn't opened correctly");
        UNIT_CHECK_LIB(LIB_PACKAGE, "package", "lib package wasn't opened correctly");

        //Checks if LIB_NONE opens any library
        Engine::LuaState L2;
        openLibs(L2, LIB_NONE);
        lua_getglobal(L2, "print");
        UNIT_ASSERT(!lua_isnil(L2, -1), "openLibs, when called with LIB_NONE flag, opened base library");
        lua_pop(L2, 1);
        
        //Checks if LIB_ALL opens any library
        Engine::LuaState L3;
        openLibs(L3, LIB_ALL);
        lua_getglobal(L3, "print");
        UNIT_ASSERT(lua_isnil(L3, -1), "openLibs, when called with LIB_ALL flag, didn't open base library");
        lua_pop(L3, 1);
    }
    {
        NEW_TEST("Test set Timeout Limits");
        LuaControllerContext ctx;
        UNIT_ASSERT( ctx.max_lines != 1e5, "Incorrect max_lines initial value");
        UNIT_ASSERT( ctx.max_count != 1e6, "Incorrect max_count initial value");
        UNIT_ASSERT( ctx.count_interval != 10, "Incorrect count_interval initial value");

        // No limits are imposed to the values
        ctx.setMaxLines(-5);
        ctx.setMaxCount(-5);
        ctx.setCountInterval(-5);
        UNIT_ASSERT( ctx.max_lines != -5, "setMaxLines didn't set value to -5");
        UNIT_ASSERT( ctx.max_count != -5, "setMaxCount didn't set value to -5");
        UNIT_ASSERT( ctx.count_interval != -5, "setCountInterval didn't set value to -5");
    }
    {
        NEW_TEST("Test a script timeout");
        LuaControllerContext ctx;
        ctx.setLuaCoreLibraries(LIB_NONE);
        ctx.setMaxLines(0);
        ctx.setMaxCount(3);
        ctx.setCountInterval(1);
        ctx.CompileString("default", "a=1;a=a+1;a=a+1;a=a+1;", true);

        // Adds dummy callbacks to `force_stop`
        std::shared_ptr<LuaCallable> registry_get_force_stop = std::make_shared<LuaCallable>(
                    get_instance_id(),
                    methods["_get_force_stop_false"],
                    [=](Variant::CallError::Error err, String msg){});
        ctx.AddRegistryVariable(std::string("lua_controller_get_force_stop"), registry_get_force_stop);

        bool error_occured = false;
        try {
            ctx.Run("default");
        }
        catch (std::runtime_error& e) {
            // print_line("\tRuntime error in first run");
            error_occured = true;
            String what = e.what();
            // Chooses the error value given the received message
            UNIT_ASSERT(!what.begins_with("[TIMEOUT] : Exceeded command count"), vformat("Error message for count timeout is incorrect. Was \"%s\"", what));
        }
        UNIT_ASSERT(!error_occured, "No Error was thrown for count timeout");
        
        // print_line("\tEnd of first run");

        ctx.setMaxLines(2);
        ctx.setMaxCount(0);
        ctx.CompileString("default", "a=1\na=a+1\na=a+1\na=a+1;", true);
        
        ctx.AddRegistryVariable(std::string("lua_controller_get_force_stop"), registry_get_force_stop);

        error_occured = false;
        try {
            ctx.Run("default");
        }
        catch (std::runtime_error& e) {
            // print_line("\tRuntime error in second run");
            error_occured = true;
            String what = e.what();
            // Chooses the error value given the received message
            UNIT_ASSERT(!what.begins_with("[TIMEOUT] : Exceeded line count"), vformat("Error message for line timeout is incorrect. Was \"%s\"", what));
        }
        UNIT_ASSERT(!error_occured, "No Error was thrown for line tieout");

        // print_line("\tEnd of second run");
    }
    {
        NEW_TEST("Test set/getTimeoutInfo");
        Engine::LuaState L4;
        setTimeoutInfo(L4, 1,2,3,4);
        int v1,v2,v3,v4;
        getTimeoutInfo(L4,v1,v2,v3,v4);
        UNIT_ASSERT(v1 != 1, "Timeout info for lines is incorrect");
        UNIT_ASSERT(v2 != 2, "Timeout info for max_lines is incorrect");
        UNIT_ASSERT(v3 != 3, "Timeout info for count is incorrect");
        UNIT_ASSERT(v4 != 4, "Timeout info for max_count is incorrect");
    }
    {
        NEW_TEST("Test registryVariables initialization");
        LuaControllerContext ctx;
        UNIT_ASSERT(!ctx.registryVariables.empty(), "registryVariables was not initialized empty");
    }
    {
        NEW_TEST("Test addRegistryVariable");
        LuaControllerContext ctx;
        std::string name = "FirstVar";
        ctx.AddRegistryVariable(name, std::make_shared<LuaCpp::Engine::LuaTNumber>(1.1));
        UNIT_ASSERT(ctx.registryVariables.empty(), "Variable not registered in registryVariables");
        UNIT_ASSERT(ctx.registryVariables.find(name) == ctx.registryVariables.end(), "Cannot find the key of a registered variable")
    }
    END_SUITE;

}

Array LuaControllerUnitTester::_lua_controller () {

    START_SUITE("lua_controller");
    
    {
        NEW_TEST("Test default value for lua_core_libraries");
        LuaController control;
        UNIT_ASSERT( control.lua_core_libraries != LuaCpp::LIB_ALL, "LuaController didn't initialize with all core libraries enabled" );
        UNIT_ASSERT( control.lua.lua_core_libraries != LuaCpp::LIB_ALL, "LuaControllerContext didn't initialize with all core libraries enabled" );
    }
    {
        NEW_TEST("Test set_lua_core_libs()");
        LuaController control;
        control.set_lua_core_libs(LuaCpp::LIB_BASE | LuaCpp::LIB_STRING);
        UNIT_ASSERT(control.lua_core_libraries != 33, "Didn't set the correct bits on LuaController");
        UNIT_ASSERT(control.lua.lua_core_libraries != 33, "LuaController didn't forward the \"set\" to it's LuaControllerContext");
    }
    {
        NEW_TEST("Test get_lua_core_libs()");
        LuaController control;
        control.lua_core_libraries = 192;
        int value = control.get_lua_core_libs();
        UNIT_ASSERT( value != 192, vformat("Didn't get the correct value. Expected 192, got %d", value));
    }
    {
        NEW_TEST("Test default value for methods_to_register");
        LuaController control;
        UNIT_ASSERT( control.methods_to_register.size() != 0, "LuaController didn't initialize with empty methods_to_register" );
    }
    {
        NEW_TEST("Test set_methods_to_register() with valid Dictionary");
        LuaController control;
        Dictionary dic;
        dic["name1"] = "lua_name1";
        dic["name2"] = "lua_name2";
        control.set_methods_to_register(dic);
        UNIT_ASSERT( control.methods_to_register.size() != 2, "set_methods_to_register didn't set the dictionary properly" );
    }
    {
        NEW_TEST("Test set_methods_to_register() with invalid Dictionary");
        LuaController control;
        Dictionary dic1;
        dic1[5] = "lua_name1";
        dic1["name2"] = "lua_name2";
        control.set_methods_to_register(dic1);
        UNIT_ASSERT( control.methods_to_register.size() != 0, "set_methods_to_register stored Dictionary with invalid key" );
        Dictionary dic2;
        dic2["name1"] = 5;
        dic2["name2"] = "lua_name2";
        control.set_methods_to_register(dic2);
        UNIT_ASSERT( control.methods_to_register.size() != 0, "set_methods_to_register stored Dictionary with invalid value" );
    }
    {
        NEW_TEST("Test get_methods_to_register()");
        LuaController control;
        Dictionary dic;
        dic["name1"] = "lua_name1";
        dic["name2"] = "lua_name2";
        control.set_methods_to_register(dic);
        Dictionary got = control.get_methods_to_register();
        UNIT_ASSERT( got.size() == 0, "get_methods_to_register returned empty Dictionary" );
        UNIT_ASSERT( ((String)got.get_valid("name1")).casecmp_to("lua_name1") != 0, "Value 'lua_name1' wasn't received in key 'name1'");
    }
    {
        NEW_TEST("Test initial timeout values");
        LuaController control;
        UNIT_ASSERT( control.max_lines != 1e5, "Incorrect max_lines initial value");
        UNIT_ASSERT( control.max_count != 1e6, "Incorrect max_count initial value");
        UNIT_ASSERT( control.count_interval != 10, "Incorrect count_interval initial value");
    }
    {
        NEW_TEST("Test setget of timeout values out of bounds");
        LuaController control;
        
        // Tries to set outside of bounds
        control.set_max_lines(-1);
        UNIT_ASSERT( control.max_lines == -1, "set_max_lines set value to -1" );
        control.set_max_count(-1);
        UNIT_ASSERT( control.max_count == -1, "set_max_count set value to -1" );
        control.set_count_interval(0);
        UNIT_ASSERT( control.count_interval == 0, "set_count_interval set value to 0" );
    }
    {
        NEW_TEST("Test setget of timeout values in bounds");
        LuaController control;

        control.set_count_interval(5);
        UNIT_ASSERT( control.count_interval != 5, "set_count_interval didn't set value" );

        // Tries to set both values to 0
        control.set_max_lines(0);
        UNIT_ASSERT( control.max_lines != 0, "set_max_lines didn't set value" );
        control.set_max_count(0);
        UNIT_ASSERT( control.max_count == 0 && control.max_lines == 0 , "set_max_count set value to 0 when max_lines was 0" );

        control.set_max_count(10);
        UNIT_ASSERT( control.max_count != 10, "set_max_count didn't set value" );
        
        control.set_max_lines(10);
        UNIT_ASSERT( control.max_lines != 10, "set_max_lines didn't set value" );

        // Tries to set both values to 0
        control.set_max_count(0);
        UNIT_ASSERT( control.max_count != 0, "set_max_count didn't set value" );
        control.set_max_lines(0);
        UNIT_ASSERT( control.max_lines == 0 && control.max_lines == 0 , "set_max_lines set value to 0 when max_count was 0" );
    }
    {
        NEW_TEST("Test initialization of force_stop");
        LuaController control;
        UNIT_ASSERT(control.force_stop, "force_stop initialized with true value");
    }
    {
        NEW_TEST("Test setget of force_stop");
        LuaController control;
        UNIT_ASSERT(control.get_force_stop(), "get_force_stop returned true when false was expected");
        control.set_force_stop(false);
        UNIT_ASSERT(control.get_force_stop(), "get_force_stop returned true after set_force_stop(false) was called");
        control.set_force_stop(true);
        UNIT_ASSERT(!control.get_force_stop(), "get_force_stop returned false after set_force_stop(true) was called");        
    }
    {
        NEW_TEST("Test force_stop_lock initialization");
        LuaController control;
        UNIT_ASSERT(control.force_stop_lock.try_lock() != OK, "force_stop_lock is busy when it should not be");
    }
    {
        NEW_TEST("Test force_stop_lock usage");
        LuaController control;
        control.force_stop_lock.lock();
        UNIT_ASSERT(control.force_stop_lock.try_lock() != OK, "force_stop_lock is busy after it was locked");
        control.force_stop_lock.unlock();
        UNIT_ASSERT(control.force_stop_lock.try_lock() != OK, "force_stop_lock is busy after it was unlocked");
    }
    END_SUITE;
}

LuaControllerUnitTester::LuaControllerUnitTester (){}
LuaControllerUnitTester::~LuaControllerUnitTester (){}

#undef START_SUITE
#undef NEW_TEST
#undef UNIT_ASSERT
#undef END_SUITE
