extends LuaController

signal secret()
signal print_called(arg)
signal play_sig(should_play)

onready var result = 0

func _ready():
	set_methods_to_register({
		"emit_secret":"secret",
		"emit_play_sig":"play",
		"add":"add",
		"my_print":"print",
		"set_result" : "result"
		})

func emit_secret():
	emit_signal("secret")
func emit_play_sig(arg):
	emit_signal("play_sig", arg)
	
func add(op1, op2):
	return op1+op2

func my_print(arg):
	emit_signal("print_called",arg)

func set_result(arg):
	if result != null :
		result = arg
		
func lua_error_handler(error, message):
		print("In handler: call_error_code = %d | message = %d".format([error, message], "%d"))

