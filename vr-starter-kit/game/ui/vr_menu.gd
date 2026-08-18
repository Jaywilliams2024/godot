extends Control

## VR Menu Panel
##
## 2D UI shown in-world on an XRToolsViewport2Din3D quad. The player points
## at it with the controller laser (function_pointer) and clicks with the
## trigger. Provides: score/time display, save & load, snap-turn toggle,
## and reset.

@onready var _score_label : Label = %ScoreLabel
@onready var _time_label : Label = %TimeLabel
@onready var _items_label : Label = %ItemsLabel
@onready var _status_label : Label = %StatusLabel
@onready var _snap_turn_check : CheckButton = %SnapTurnCheck
@onready var _save_button : Button = %SaveButton
@onready var _load_button : Button = %LoadButton
@onready var _reset_button : Button = %ResetButton


func _ready() -> void:
	# Reflect current settings
	_snap_turn_check.button_pressed = XRToolsUserSettings.snap_turning

	# Wire signals
	_save_button.pressed.connect(_on_save_pressed)
	_load_button.pressed.connect(_on_load_pressed)
	_reset_button.pressed.connect(_on_reset_pressed)
	_snap_turn_check.toggled.connect(_on_snap_turn_toggled)

	GameState.score_changed.connect(func(_s): _refresh())
	GameState.item_collected.connect(func(_id, _n): _refresh())
	SaveSystem.game_saved.connect(func(slot): _set_status("Saved to slot %d" % slot))
	SaveSystem.game_loaded.connect(func(slot): _set_status("Loaded slot %d" % slot))
	SaveSystem.save_error.connect(func(msg): _set_status(msg))

	_load_button.disabled = not SaveSystem.has_save(0)
	_refresh()


func _process(_delta : float) -> void:
	_time_label.text = "Time: %s" % _format_time(GameState.play_time)


func _refresh() -> void:
	_score_label.text = "Score: %d" % GameState.score
	_items_label.text = "Items: %d" % GameState.collected_items.size()


func _on_save_pressed() -> void:
	SaveSystem.save_game(0)
	_load_button.disabled = not SaveSystem.has_save(0)


func _on_load_pressed() -> void:
	SaveSystem.load_game(0)


func _on_reset_pressed() -> void:
	GameState.reset()
	_set_status("Game state reset")


func _on_snap_turn_toggled(pressed : bool) -> void:
	XRToolsUserSettings.snap_turning = pressed
	XRToolsUserSettings.save()
	_set_status("Snap turning: %s" % ("ON" if pressed else "OFF (smooth)"))


func _set_status(text : String) -> void:
	_status_label.text = text


func _format_time(seconds : float) -> String:
	var total := int(seconds)
	@warning_ignore("integer_division")
	return "%02d:%02d" % [total / 60, total % 60]
