extends XRToolsPickable

## Collectible Pickable
##
## A grabbable object that awards score the first time the player picks
## it up. Give each instance a unique [member item_id] in the inspector.

## Unique id used by GameState to track collection
@export var item_id : String = "collectible"

## Score awarded on first pickup
@export var score_value : int = 10


func _ready() -> void:
	super()
	picked_up.connect(_on_first_pickup)


func _on_first_pickup(_pickable) -> void:
	if GameState.collect_item(item_id):
		GameState.add_score(score_value)
