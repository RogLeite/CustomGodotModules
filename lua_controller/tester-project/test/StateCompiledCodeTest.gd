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

func test_run_error_with_message():
	ctrl.set_lua_code("error(\"Just so I raise an error\")")
	assert_int(ctrl.compile()) \
		.is_equal(OK)
	assert_int(ctrl.run()) \
		.is_equal(ERR_SCRIPT_FAILED)
	assert_that(ctrl.result).is_equal(0)
	assert_str(ctrl.get_error_message()) \
		.contains("Just so I raise an error")

func test_run_error_no_message():
	ctrl.set_lua_code("error()")
	assert_int(ctrl.compile()) \
		.is_equal(OK)
	assert_int(ctrl.run()) \
		.is_equal(ERR_SCRIPT_FAILED)
	assert_that(ctrl.result).is_equal(0)
	assert_str(ctrl.get_error_message()) \
		.contains("[empty error message]")

func test_run_timeout():
	ctrl.set_lua_code("""print(1)
print(2)
print(3)""")

	# Tests lines timeout
	ctrl.set_max_lines(2)
	assert_int(ctrl.compile()) \
		.is_equal(OK)
	assert_int(ctrl.run()) \
		.is_equal(ERR_TIMEOUT)
	
	assert_str(ctrl.get_error_message()) \
		.starts_with("[TIMEOUT]") \
		.contains("Exceeded line count")
	
	# Tests command timeout
	ctrl.set_max_lines(1000)
	ctrl.set_max_count(3)
	ctrl.set_count_interval(1)
	assert_int(ctrl.compile()) \
		.is_equal(OK)
	assert_int(ctrl.run()) \
		.is_equal(ERR_TIMEOUT)
	
	assert_str(ctrl.get_error_message()) \
		.starts_with("[TIMEOUT]") \
		.contains("Exceeded command count")
	

func test_run_timeout_in_function():
	ctrl.set_lua_code("""function timeout() 
		local x = 1
		while true do x = x+1 end
	end
	timeout()""")

	# Tests lines timeout
	ctrl.set_max_lines(10)
	assert_int(ctrl.compile()) \
		.is_equal(OK)
	assert_int(ctrl.run()) \
		.is_equal(ERR_TIMEOUT)
	
	assert_str(ctrl.get_error_message()) \
		.starts_with("[string") \
		.contains("[TIMEOUT]") \
		.contains("Exceeded line count")
	
	# Tests command timeout
	ctrl.set_max_lines(1000)
	ctrl.set_max_count(100)
	ctrl.set_count_interval(1)
	assert_int(ctrl.compile()) \
		.is_equal(OK)
	assert_int(ctrl.run()) \
		.is_equal(ERR_TIMEOUT)
	
	assert_str(ctrl.get_error_message()) \
		.starts_with("[string") \
		.contains("[TIMEOUT]") \
		.contains("Exceeded command count")
	
