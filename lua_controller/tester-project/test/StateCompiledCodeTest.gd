# GdUnit generated TestSuite
#warning-ignore-all:unused_argument
#warning-ignore-all:return_value_discarded
class_name StateCompiledCodeTest
extends GdUnitTestSuite

var ctrl : LuaController
var ctrl_script := load("res://HasResult.gd")

func before_test():
	ctrl = LuaController.new()
	ctrl.set_script(ctrl_script)
	add_child(ctrl)

func after_test():
	ctrl.queue_free()

		
func test_run_ok():
	ctrl.set_lua_code("result(1);result(result() + 1);")
	assert_int(ctrl.compile()) \
		.is_equal(OK)
	assert_int(ctrl.run()) \
		.is_equal(OK)
	assert_str(ctrl.get_error_message()).is_empty()
	assert_that(ctrl.result).is_equal(2)

func test_run_error():
	ctrl.set_lua_code("error(\"Just so I raise an error\")")
	assert_int(ctrl.compile()) \
		.is_equal(OK)
	assert_int(ctrl.run()) \
		.is_equal(ERR_SCRIPT_FAILED)
	assert_that(ctrl.result).is_equal(0)

