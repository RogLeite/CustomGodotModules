extends LuaController

var result = 0
func _ready():
	methods_to_register = {"setget_result" : "result"}

func setget_result(value):
	if not (value == null) :
		result = value	
	return result

