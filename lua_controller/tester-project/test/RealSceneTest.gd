# GdUnit generated TestSuite
#warning-ignore-all:unused_argument
#warning-ignore-all:return_value_discarded
class_name RealSceneTest
extends GdUnitTestSuite

var spy_scene
var _runner

const code_print = "print(\"[PRINT CALLED]\")"
const code_play = "play()"
const code_secret = "secret()"
const code_add = "result(add(1,add(1,0)))"

func before():
	var scene_instance = load("res://RealScene/ScriptRunner.tscn").instance()
	spy_scene = spy(scene_instance)

func before_test():
	reset(spy_scene)
	_runner = scene_runner(spy_scene)
	
func test_code_print():
	spy_scene.set_lua_code(code_print)
	spy_scene.find_node("Run").emit_signal("pressed")
	assert_str(spy_scene.find_node("PrintLabel").text) \
		.contains("[PRINT CALLED]")
	
func test_code_play():
	spy_scene.set_lua_code(code_play)
	spy_scene.find_node("Run").emit_signal("pressed")
	yield(_runner.simulate_until_signal("animation_finished"), "completed")
	assert_that(spy_scene.find_node("ColorRect").color.g).is_equal(1)
	assert_that(spy_scene.find_node("ColorRect").color.r).is_equal(0)


func test_interrupt_code_play():
	spy_scene.set_lua_code(code_play)
	spy_scene.find_node("Run").emit_signal("pressed")
	yield(_runner.simulate_frames(10), "completed")
	assert_that(spy_scene.find_node("ColorRect").color.r).is_not_equal(0)
	assert_that(spy_scene.find_node("ColorRect").color.r).is_not_equal(1)
	assert_that(spy_scene.find_node("ColorRect").color.g).is_not_equal(1)
	assert_that(spy_scene.find_node("ColorRect").color.g).is_not_equal(0)	
	
func test_code_secret():
	spy_scene.set_lua_code(code_secret)
	spy_scene.find_node("Run").emit_signal("pressed")
	assert_bool(spy_scene.find_node("SignalNoArgs").secret).is_false()
	
func test_code_add():
	spy_scene.set_lua_code(code_add)
	spy_scene.find_node("Run").emit_signal("pressed")
	assert_that(spy_scene.find_node("LuaController").result).is_equal(2)

func after():
	spy_scene.free()
		
