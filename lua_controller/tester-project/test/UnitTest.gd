# GdUnit generated TestSuite
#warning-ignore-all:unused_argument
#warning-ignore-all:return_value_discarded
class_name UnitTest
extends GdUnitTestSuite

var my_tester : LuaControllerUnitTester

func before():
	my_tester = LuaControllerUnitTester.new()
	add_child(my_tester)

func after():
	my_tester.queue_free()

func test_lua_callable() -> void:
	assert_array(my_tester.lua_callable()).is_empty()
	

func test_lua_controller() -> void:
	
	assert_array(my_tester.lua_controller()).is_empty()
	

func test_lua_controller_context() -> void:
	assert_array(my_tester.lua_controller_context()).is_empty()
	
