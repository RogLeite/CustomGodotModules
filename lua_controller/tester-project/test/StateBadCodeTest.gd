# GdUnit generated TestSuite
#warning-ignore-all:unused_argument
#warning-ignore-all:return_value_discarded
class_name StateBadCodeTest
extends GdUnitTestSuite

var ctrl : LuaController

func before_test():
	ctrl = LuaController.new()
	add_child(ctrl)
	ctrl.set_lua_code("print(\'hi")

func after_test():
	ctrl.queue_free()


func test_set_bad_code() : 
	ctrl.set_lua_code("p")
	assert_int(ctrl.compile())\
		.is_equal(ERR_COMPILATION_FAILED)
	assert_str(ctrl.get_error_message()).\
		is_equal("[LOGIC ERROR] : [string \"p\"]:1: syntax error near <eof>".c_unescape())

func test_compilation_error() :
	assert_int(ctrl.compile())\
		.is_equal(ERR_COMPILATION_FAILED)
	assert_str(ctrl.get_error_message()).\
		is_equal("[LOGIC ERROR] : [string \"print(\'hi\"]:1: unfinished string near <eof>".c_unescape())
	
func test_run() :
	assert_int(ctrl.run())\
		.is_equal(ERR_INVALID_DATA)
	ctrl.compile()
	assert_int(ctrl.run())\
		.is_equal(ERR_INVALID_DATA)	

func test_clear_error_message() : 
	ctrl.compile()
	assert_str(ctrl.get_error_message()).is_not_empty()
	ctrl.clear_error_message()
	assert_str(ctrl.get_error_message()).is_empty()

