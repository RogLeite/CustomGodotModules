# GdUnit generated TestSuite
#warning-ignore-all:unused_argument
#warning-ignore-all:return_value_discarded
class_name RootSceneTest
extends GdUnitTestSuite

# TestSuite generated from
const __source = 'res://RootScene.gd'

func test_unit() -> void:
	var my_tester = LuaControllerUnitTester.new()
	assert_array(my_tester.lua_callable()).is_empty()
	
	assert_array(my_tester.lua_controller()).is_empty()
	
	assert_array(my_tester.lua_controller_context()).is_empty()
	my_tester.queue_free()
	
