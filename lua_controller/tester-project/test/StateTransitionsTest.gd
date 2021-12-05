# GdUnit generated TestSuite
#warning-ignore-all:unused_argument
#warning-ignore-all:return_value_discarded
class_name StateTransitionTest
extends GdUnitTestSuite

var ctrl : LuaController

func before_test():
	ctrl = LuaController.new()
	add_child(ctrl)

func after_test():
	ctrl.queue_free()
	
func test_uncompiled_to_bad() :
	ctrl.set_lua_code("p")
	assert_int(ctrl.compile()) \
		.is_equal(ERR_COMPILATION_FAILED)
	assert_str(ctrl.get_error_message()).\
		contains("[LOGIC ERROR]")

func test_bad_to_uncompiled() :
	ctrl.set_lua_code("p")
	ctrl.set_lua_code("")
	assert_int(ctrl.compile()) \
		.is_equal(OK)
		
func test_uncompiled_to_compiled() :
	ctrl.set_lua_code("")
	assert_int(ctrl.compile()) \
		.is_equal(OK)
	assert_int(ctrl.run()) \
		.is_equal(OK)
	
func test_compiled_to_uncompiled() :
	ctrl.set_lua_code("")
	ctrl.compile()
	ctrl.set_lua_code(" ")
	assert_int(ctrl.run()) \
		.is_equal(ERR_INVALID_DATA)
		
func test_compiled_to_bad() :
	ctrl.set_lua_code("")
	ctrl.compile()
	ctrl.set_lua_code("p")
	assert_int(ctrl.compile()) \
		.is_equal(ERR_COMPILATION_FAILED)
	assert_str(ctrl.get_error_message()).\
		contains("[LOGIC ERROR]")
