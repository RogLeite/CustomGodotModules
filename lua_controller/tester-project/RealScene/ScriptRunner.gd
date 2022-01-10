extends Control
onready var error_message = ""
onready var lua1 := $LuaController
onready var lua2 := $LuaController2
onready var print_label := $VBox/PrintLabel
onready var lua_script := $Script

signal animation_finished

func _ready():
	print_label.text = ""
	lua_script.text = ""

func set_lua_code (code : String):
	lua_script.text = code

func _run1():
	print_label.text = ""
	lua1.set_lua_code(lua_script.text)
	error_message = ""
	if lua1.compile() != OK :
		error_message = lua1.get_error_message()
	elif lua1.run() != OK:
		error_message = lua1.get_error_message()
	
	if error_message != "" :
		_on_print_called("\nLuaController: "+error_message+"\n")
	
	print_label.text += "\n"
	
func _run2():
	print_label.text = ""
	lua2.set_lua_code(lua_script.text)
	error_message = ""
	if lua2.compile() != OK :
		error_message = lua2.get_error_message()
	elif lua2.run() != OK:
		error_message = lua2.get_error_message()
	
	if error_message != "" :
		_on_print_called("\nLuaController2: "+error_message+"\n")
	
	print_label.text += "\n"
		
func _on_print_called(arg):
	print_label.text += String(arg) + "; "


func _on_animation_finished(anim_name):
	emit_signal("animation_finished")

