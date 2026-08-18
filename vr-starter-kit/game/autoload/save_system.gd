extends Node

## SaveSystem Autoload
##
## JSON save-game system with multiple slots, stored in `user://saves/`.
## On Quest/Android this maps to the app's sandboxed storage; on PC it is
## `%APPDATA%/Godot/app_userdata/VR Starter Kit/saves/`.
##
## Usage:
## [codeblock]
##     SaveSystem.save_game()          # save to slot 0
##     SaveSystem.save_game(2)         # save to slot 2
##     SaveSystem.load_game()          # load slot 0
##     SaveSystem.list_saves()         # -> Array of slot info dictionaries
## [/codeblock]

## Emitted after a successful save
signal game_saved(slot : int)

## Emitted after a successful load
signal game_loaded(slot : int)

## Emitted when a save/load fails
signal save_error(message : String)

const SAVE_DIR := "user://saves"
const SAVE_VERSION := 1
const MAX_SLOTS := 4


func _ready() -> void:
	# Ensure the save directory exists
	DirAccess.make_dir_recursive_absolute(SAVE_DIR)


## Save current GameState to a slot. Returns true on success.
func save_game(slot : int = 0) -> bool:
	if slot < 0 or slot >= MAX_SLOTS:
		save_error.emit("Invalid save slot %d" % slot)
		return false

	var data := {
		"version": SAVE_VERSION,
		"timestamp": Time.get_unix_time_from_system(),
		"date_string": Time.get_datetime_string_from_system(false, true),
		"state": GameState.to_dict(),
	}

	var file := FileAccess.open(_slot_path(slot), FileAccess.WRITE)
	if not file:
		save_error.emit("Cannot open save file for writing (slot %d)" % slot)
		return false

	file.store_string(JSON.stringify(data, "\t"))
	file.close()
	game_saved.emit(slot)
	return true


## Load a save slot into GameState. Returns true on success.
func load_game(slot : int = 0) -> bool:
	var path := _slot_path(slot)
	if not FileAccess.file_exists(path):
		save_error.emit("No save data in slot %d" % slot)
		return false

	var file := FileAccess.open(path, FileAccess.READ)
	if not file:
		save_error.emit("Cannot open save file (slot %d)" % slot)
		return false

	var parsed = JSON.parse_string(file.get_as_text())
	file.close()

	if typeof(parsed) != TYPE_DICTIONARY:
		save_error.emit("Save file corrupt (slot %d)" % slot)
		return false

	var version := int(parsed.get("version", 0))
	if version > SAVE_VERSION:
		save_error.emit("Save file from a newer version (slot %d)" % slot)
		return false

	GameState.from_dict(parsed.get("state", {}))
	game_loaded.emit(slot)
	return true


## Does a slot contain save data?
func has_save(slot : int = 0) -> bool:
	return FileAccess.file_exists(_slot_path(slot))


## Delete a save slot
func delete_save(slot : int = 0) -> void:
	if has_save(slot):
		DirAccess.remove_absolute(_slot_path(slot))


## List info for all slots: [{slot, exists, date_string, score, play_time}, ...]
func list_saves() -> Array:
	var result : Array = []
	for slot in MAX_SLOTS:
		var info := {"slot": slot, "exists": false}
		var path := _slot_path(slot)
		if FileAccess.file_exists(path):
			var file := FileAccess.open(path, FileAccess.READ)
			if file:
				var parsed = JSON.parse_string(file.get_as_text())
				file.close()
				if typeof(parsed) == TYPE_DICTIONARY:
					info["exists"] = true
					info["date_string"] = parsed.get("date_string", "?")
					var state : Dictionary = parsed.get("state", {})
					info["score"] = int(state.get("score", 0))
					info["play_time"] = float(state.get("play_time", 0.0))
		result.append(info)
	return result


func _slot_path(slot : int) -> String:
	return "%s/slot_%d.json" % [SAVE_DIR, slot]
