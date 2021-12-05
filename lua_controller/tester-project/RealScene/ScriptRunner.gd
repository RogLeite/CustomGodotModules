extends Control
onready var error_message = ""
onready var lua := $LuaController
onready var print_label := $VBox/PrintLabel
onready var lua_script := $Script

signal animation_finished

func _ready():
	print_label.text = ""
	lua_script.text = ""

func set_lua_code (code : String):
	lua_script.text = code

func _on_Run_pressed():
	lua.set_lua_code(lua_script.text)
	if lua.compile() != OK :
		error_message = lua.get_error_message()
	elif lua.run() != OK:
		error_message = lua.get_error_message()
	else :
		error_message = ""

func _on_print_called(arg):
	print_label.text += arg + "\n"


func _on_animation_finished(anim_name):
	emit_signal("animation_finished")
