extends Node

## GameState Autoload
##
## Central game-state manager for the VR Starter Kit.
## Tracks score, collected items, elapsed play time and current level, and
## exposes signals so UI panels (and any gameplay systems) can react to
## changes without tight coupling.
##
## Access anywhere with the `GameState` singleton:
## [codeblock]
##     GameState.add_score(10)
##     GameState.collect_item("red_cube")
## [/codeblock]

## Emitted whenever the score changes
signal score_changed(new_score : int)

## Emitted when an item is collected for the first time
signal item_collected(item_id : String, total : int)

## Emitted when the game state has been reset
signal state_reset()

## Emitted after a save-game has been applied to the state
signal state_loaded()


## Current player score
var score : int = 0 : set = _set_score

## Set of collected item ids (Dictionary used as a set)
var collected_items : Dictionary = {}

## Seconds of play time (accumulated while the game is running)
var play_time : float = 0.0

## Path of the level scene the player is currently in
var current_level : String = ""


func _process(delta : float) -> void:
	play_time += delta


## Add (or subtract) score
func add_score(amount : int) -> void:
	self.score = score + amount


## Mark an item as collected. Returns true if it was newly collected.
func collect_item(item_id : String) -> bool:
	if collected_items.has(item_id):
		return false
	collected_items[item_id] = true
	item_collected.emit(item_id, collected_items.size())
	return true


## Check whether an item has been collected
func has_item(item_id : String) -> bool:
	return collected_items.has(item_id)


## Reset all game state (new game)
func reset() -> void:
	score = 0
	collected_items.clear()
	play_time = 0.0
	state_reset.emit()
	score_changed.emit(score)


## Serialize state to a Dictionary (used by SaveSystem)
func to_dict() -> Dictionary:
	return {
		"score": score,
		"collected_items": collected_items.keys(),
		"play_time": play_time,
		"current_level": current_level,
	}


## Restore state from a Dictionary (used by SaveSystem)
func from_dict(data : Dictionary) -> void:
	score = int(data.get("score", 0))
	collected_items.clear()
	for item in data.get("collected_items", []):
		collected_items[str(item)] = true
	play_time = float(data.get("play_time", 0.0))
	current_level = str(data.get("current_level", ""))
	state_loaded.emit()
	score_changed.emit(score)


func _set_score(value : int) -> void:
	score = value
	score_changed.emit(score)
