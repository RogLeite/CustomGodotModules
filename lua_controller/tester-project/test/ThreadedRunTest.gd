# GdUnit generated TestSuite
#warning-ignore-all:unused_argument
#warning-ignore-all:return_value_discarded
class_name ThreadedRunTest
extends GdUnitTestSuite

var ctrl : LuaController
var ctrl_script := load("res://ThreadableHasResult.gd")
var thread : Thread

const code_wait = """function wait(seconds)
	local target = os.time() +  seconds;
	repeat until os.time() > target
end
result(1)
wait(5)
result(2)"""
func before_test():
	thread = Thread.new()
	
	ctrl = LuaController.new()
	ctrl.set_count_interval(1000000)
	ctrl.set_max_lines(0)
	ctrl.set_script(ctrl_script)
	ctrl.set_lua_code(code_wait)
	add_child(ctrl)

func after_test():
	ctrl.queue_free()
		
func test_run_threaded_ok():
	# Compiles script
	assert_int(ctrl.compile()) \
		.is_equal(OK)
	# Launches Thread
	assert_int(thread.start(ctrl,"run")) \
		.is_equal(OK)
	
	# Thread is alive and well
	assert_bool(thread.is_alive()) \
		.is_equal(true)
	assert_bool(thread.is_active()) \
		.is_equal(true)
	
	# Thread joins having ended gracefully
	assert_int(thread.wait_to_finish()) \
		.is_equal(OK)

	# Thread is no longer active
	assert_bool(thread.is_active()) \
		.is_equal(false)

	# Script finished correctly
	assert_str(ctrl.get_error_message()).is_empty()
	assert_that(ctrl.result).is_equal(2)

func test_run_threaded_force_stop():
	# Compiles script
	assert_int(ctrl.compile()) \
		.is_equal(OK)
	# Launches Thread
	assert_int(thread.start(ctrl,"run")) \
		.is_equal(OK)
	
	# Thread is alive and well
	assert_bool(thread.is_alive()) \
		.is_equal(true)
	assert_bool(thread.is_active()) \
		.is_equal(true)
		
	ctrl.set_force_stop(true)
	
	# Thread joins having ended gracefully
	assert_int(thread.wait_to_finish()) \
		.is_equal(OK)

	# Thread is no longer active
	assert_bool(thread.is_active()) \
		.is_equal(false)

	# Script finished correctly
	assert_str(ctrl.get_error_message()) \
		.contains("[INTERRUPTED]") \
		.contains("Execution of the script was manualy interrupted")
	assert_that(ctrl.result).is_equal(1)

func test_run_threaded_timeout():
	# Tests timeout by line count
	ctrl.set_max_lines(5)
	# Compiles script
	assert_int(ctrl.compile()) \
		.is_equal(OK)
	# Launches Thread
	assert_int(thread.start(ctrl,"run")) \
		.is_equal(OK)
	# Thread joins having ended gracefully
	assert_int(thread.wait_to_finish()) \
		.is_equal(ERR_TIMEOUT)
	
	assert_str(ctrl.get_error_message()) \
		.starts_with("[string") \
		.contains("[TIMEOUT]") \
		.contains("Exceeded line count")
	
	# Tests command timeout
	ctrl.set_max_count(10)
	ctrl.set_count_interval(2)
	ctrl.set_max_lines(0)
	assert_int(ctrl.compile()) \
		.is_equal(OK)
	# Launches Thread
	assert_int(thread.start(ctrl,"run")) \
		.is_equal(OK)
	# Thread joins having ended gracefully
	assert_int(thread.wait_to_finish()) \
		.is_equal(ERR_TIMEOUT)
	
	assert_str(ctrl.get_error_message()) \
		.starts_with("[string") \
		.contains("[TIMEOUT]") \
		.contains("Exceeded command count")
	
