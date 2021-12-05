# GdUnit generated TestSuite
#warning-ignore-all:unused_argument
#warning-ignore-all:return_value_discarded
class_name StateUncompiledCodeTest
extends GdUnitTestSuite

var ctrl : LuaController

func before_test():
	ctrl = LuaController.new()
	add_child(ctrl)

func after_test():
	ctrl.queue_free()

func test_initial_properties() :
	assert_str(ctrl.get_error_message()).is_empty()
	assert_int(ctrl.lua_core_libraries).is_equal(1023)
	assert_dict(ctrl.methods_to_register).is_empty()
	
func test_run() : 
	assert_int(ctrl.run())\
		.is_equal(ERR_INVALID_DATA)
	
func test_set_lua_code() :
	ctrl.set_lua_code("") 
	assert_int(ctrl.run())\
		.is_equal(ERR_INVALID_DATA)

