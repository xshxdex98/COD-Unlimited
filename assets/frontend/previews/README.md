# Front-end menu overhaul

The multiplayer executable in `D:/CODUnlimited Dev/bin/Debug/KisakCOD-mp.exe` includes this pass. Product version remains 0.0.1.

## Screens

- [Local Play](createserver.png): Session and Advanced pages, native map feeder/preview, game-mode rules and Start Session.
- [Advanced server setup](cu_local_advanced.png).
- [Server browser](pc_join_unranked.png): Internet/LAN/Favorites, game-mode control, sortable columns, refresh, filter panel and existing connection actions.
- [Server filters](filter_popmenu.png).
- [Video settings](options_graphics.png) and [Textures](options_graphics_texture.png).
- [Audio and Voice](options_sound.png): merged sound options, voice toggle, recording level and live level meter.
- [Game & Network](options_game.png): existing gameplay, language, name, downloads and legacy connection options.
- Controls: [Mouse / Look](options_look.png), [Movement](options_move.png), [Combat](options_shoot.png), [Interact](options_misc.png), [Multiplayer](controls_multi.png).
- [Mod Library](mods_menu.png): existing mod feeder and launch handler, with live selected-mod name.
- [Barracks Progress](menu_challenges.png) and [Challenges](cu_challenges.png): native stats, rank expressions and unlock conditions retained.
- [Game-mode rules](settings_quick_war.png): native mode-specific option handlers retained.

Buttons use the main menu's translucent panels, icons, condensed lettering, green focus fill and left accent. Scrollbars and sliders use new geometry-based textures. Small native dialogs retain their geometry, scripts and validation, with a matching palette. The legacy main_text return route now resolves to a copy of the custom main menu.

## Source

- `D:/CODUnlimited Dev/src/ui_mp/ui_frontend_theme.h`: UI-owned menu copies, layouts, shared styling and gated diagnostics.
- `D:/CODUnlimited Dev/src/ui_mp/ui_main_mp.cpp`: theme initialization, mod selection display and shared control assets.
- `D:/CODUnlimited Dev/tools/build_frontend_assets.py`: deterministic icons/control textures and game-format conversion.
- `D:/CODUnlimited Dev/tools/build_frontend_menu.py`: editable main-menu source generator.
- `D:/CODUnlimited Dev/tools/preview_frontend.py`: temporary in-game screenshot runner; always restores the production menu files.

The stock asset definitions are copied into the UI context, preserving native field types, dvar bounds, callbacks, binding widgets, list feeders, conditional visibility and apply/restart scripts. Active mods keep their own menu systems. This pass targets the base-game front end.

## Verification

The Debug MP executable compiled and linked. In-game screenshots were reviewed at 1920 x 1080 across the listed screen families. A renderer restart reloaded the custom materials and settings layout cleanly. See [restart log](restart-validation.log).

The in-engine callback smoke test passed 10 checks: both Local Play tab transitions; local voice toggle; map selection; merged Audio voice toggle; Audio to Game & Network transition; Controls tab transition; mod selection and selected-name agreement; browser source transition; and binding widgets retained across the five control categories. Changed test values were restored. See [interaction log](interaction-validation.log).

Live server joins, actual mod launches, microphone hardware capture and an entire gameplay session were not exercised. Reset/default actions were not activated during verification.

Developer commands:

- `python tools/preview_frontend.py createserver`
- `python tools/preview_frontend.py createserver --smoke`
- `python tools/preview_frontend.py options_graphics --restart`

The preview runner inserts screenshot/quit scripts temporarily and restores both menu copies in a finally block. Normal gameplay has no preview commands in its menu files.
