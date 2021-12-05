extends AnimationPlayer

func _on_play_sig(should_play):
	if typeof(should_play) == TYPE_NIL :
		play("flow") if not is_playing() else stop(false)
	else:
		play("flow") if should_play else stop(false)
