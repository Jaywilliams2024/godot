extends XRToolsSceneBase

## Playground Level
##
## Demo playground for the VR Starter Kit. Registers itself with GameState.
##
## Note: the MovementTurn node uses TurnMode.DEFAULT which live-reads
## XRToolsUserSettings.snap_turning, so the snap/smooth toggle in the
## in-world menu takes effect immediately without extra wiring.


func _ready() -> void:
	super()
	GameState.current_level = scene_file_path
