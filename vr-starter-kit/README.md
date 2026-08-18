# Godot 4.x VR Starter Kit

A complete, ready-to-run VR project for **Godot 4.4+** built on **OpenXR** and
**[godot-xr-tools 4.5.1](https://github.com/GodotVR/godot-xr-tools)**.
Open it in Godot, press **F5** with a headset connected, and you're in VR.

## Features

### VR Interaction (godot-xr-tools)
- ✅ **OpenXR** pre-configured (works with SteamVR, Meta Quest Link/Air Link, Monado, native Quest builds)
- ✅ **Teleport** — hold the left controller's teleport function and point
- ✅ **Smooth locomotion** — left thumbstick (with strafe), right thumbstick forward/back
- ✅ **Snap turn / smooth turn** — right thumbstick; toggle mode from the in-world menu
- ✅ **Jump** — A/X button
- ✅ **Hand grabbing** — grip button picks up nearby or ranged pickable objects
- ✅ **Climbing** — grip the yellow holds on the climbing wall and pull yourself up
- ✅ **Laser pointer UI** — right-hand pointer + trigger clicks in-world menus
- ✅ **Animated low-poly hands** with grip/trigger poses
- ✅ **Player body** with gravity, collisions, and comfort fade on wall intersection
- ✅ **Staging system** with loading screen and scene transitions

### Game Framework Layer (this kit)
- ✅ **`GameState` autoload** — score, collected items, play time, signals
- ✅ **`SaveSystem` autoload** — JSON save games, 4 slots, stored in `user://saves/`
- ✅ **In-world VR menu** — save / load / reset, snap-turn toggle, live stats (point & click with the laser)
- ✅ **Collectible pickable base class** — grab an object the first time → score + persistence

### Demo Playground
- A table with **grabbable cubes & ball** (collectibles that award score)
- A **snap zone** that objects click into
- A **climbing wall** with holds
- **Teleport zones** — step on the green pad to be teleported across the map
- Sky, sun with shadows, and a 30×30 m play area

## Requirements

| Item | Version |
|---|---|
| Godot | **4.4 or newer** (standard build, not .NET required) |
| godot-xr-tools | 4.5.1 (bundled in `addons/`) |
| A PC VR headset via OpenXR runtime (SteamVR / Oculus / Monado), or Meta Quest for Android export |

## Quick Start (PC VR)

1. Install [Godot 4.4+](https://godotengine.org/download).
2. Make sure your OpenXR runtime is running (SteamVR, or Meta Quest Link with OpenXR set as active runtime).
3. Open this folder in Godot (Import → select `project.godot`).
4. Wait for first import to finish (the addon assets take a minute).
5. Press **F5** (Run Project). Put on your headset.

> If the editor asks to enable the *Godot XR Tools* plugin, it is already
> enabled in `project.godot` — just restart the editor if prompted.

## Controls

| Action | Input |
|---|---|
| Move | Left thumbstick (strafe enabled) |
| Turn (snap or smooth) | Right thumbstick |
| Jump | A / X button |
| Teleport | Left controller teleport function (hold, aim, release) |
| Grab / climb | Grip button |
| UI click | Right-hand laser + trigger |

## Project Layout

```
vr-starter-kit/
├── project.godot              # OpenXR enabled, autoloads, physics layers
├── addons/godot-xr-tools/     # Bundled XR Tools 4.5.1 (unmodified)
├── game/
│   ├── main.tscn              # Entry: XR Tools staging → playground
│   ├── autoload/
│   │   ├── game_state.gd      # Score / items / time singleton
│   │   └── save_system.gd     # JSON multi-slot save games
│   ├── levels/
│   │   ├── playground.tscn    # Demo level + full VR player rig
│   │   └── playground.gd
│   ├── objects/
│   │   ├── collectible.gd     # XRToolsPickable subclass with scoring
│   │   ├── grab_cube.tscn
│   │   ├── grab_ball.tscn
│   │   └── climbing_wall.tscn
│   └── ui/
│       ├── vr_menu.tscn       # In-world menu (Viewport2Din3D)
│       └── vr_menu.gd
└── icon.svg
```

## Making Your Own Level

1. Create a new scene inheriting `res://addons/godot-xr-tools/staging/scene_base.tscn`
   (or duplicate `game/levels/playground.tscn`).
2. Keep the `XROrigin3D` rig (hands, movement providers, `PlayerBody`).
3. Add static geometry on **physics layer 1 (Static World)**.
4. Make objects grabbable by instancing `objects/pickable.tscn` or
   subclassing `game/objects/collectible.gd` (layer 3, *Pickable Objects*).
5. Make things climbable with a `StaticBody3D` + `climbable.gd`
   on layers 1 + 19 (*Grab Handles*).
6. Point staging at your scene: change `main_scene` in `game/main.tscn`, or call
   `load_scene("res://game/levels/your_level.tscn")` from any scene script.

## Exporting to Meta Quest (standalone)

1. Install *Android Build Template* + set up Android SDK per the
   [Godot XR docs](https://docs.godotengine.org/en/stable/tutorials/xr/deploying_to_android.html).
2. Project → Export → add **Android** preset.
3. In the preset: XR Features → *XR Mode: OpenXR*, enable *Use Gradle Build*.
4. The renderer is already `gl_compatibility` (mobile friendly) and
   ETC2/ASTC texture compression is enabled.

## Credits & Licenses

- [Godot Engine](https://github.com/godotengine/godot) — MIT
- [godot-xr-tools](https://github.com/GodotVR/godot-xr-tools) — MIT (see `addons/godot-xr-tools/LICENSE`)
- Starter kit code (`game/`) — MIT
