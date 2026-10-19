# COD Unlimited front-end art

Generated using the built-in ImageGen tool, with the user's nine-screen inspiration board as a visual reference. The approved logo was preserved. Icons are deterministic geometry in tools/build_frontend_assets.py.

## Saved assets

- Background source: D:/CODUnlimited Dev/assets/frontend/background-v2.png
- Logo source (alpha): D:/CODUnlimited Dev/assets/frontend/logo.png
- Verified in-game capture: D:/CODUnlimited Dev/assets/frontend/menu-preview-final.png
- Runtime textures: D:/CODUnlimited Dev/bin/Debug/main/images/codunlimited_*.iwi
- Runtime menu: D:/CODUnlimited Dev/bin/Debug/main/ui_mp/scriptmenus/main.menu

Regenerate the game texture files with `python tools/build_frontend_assets.py` (requires Pillow). This performs format conversion and texture sizing, preserving alpha; the artwork is generated with ImageGen. The texture aspect ratios are restored by the menu's display rectangles. Regenerate the editable menu with `python tools/build_frontend_menu.py`.

## Logo prompt

Use case: logo-brand. Create a production transparent-background game logo for COD UNLIMITED. Reference image: attached inspiration board, match the visual character of the small top-left logo. Exact text: top line "COD", second line "UNLIMITED". Top line large heavy wide squared military block sans-serif lettering, light silver/off-white distressed worn metal concrete texture, subtle chipped edges, flat restrained lighting, no excessive bevel. Bottom line much smaller wide letter-spaced clean pale olive-green capitals, full width visually matches COD. Compact horizontal lockup aspect approximately 2.4:1, centered and tightly framed, generous enough padding that edges are not clipped. Premium clean military shooter branding, no shield, no emblems, no extra words, no background, no drop-shadow rectangle. True alpha transparency.

## Initial background prompt

Use case: stylized-concept. Asset type: production background artwork for a playable COD Unlimited main menu, wide 16:9 landscape. Reference image: attached nine-screen inspiration board, use ONLY the top-left main menu's battlefield art style as inspiration, not the UI or text. Create original cinematic realistic military key art: ruined Middle Eastern urban street, dusty weathered buildings, palms, dramatic golden late-afternoon light breaking through charcoal clouds, helicopter overhead in upper right, a seated modern soldier in olive tactical equipment seen from behind in the lower center-right foreground watching the city. Composition: leftmost 35 percent is dark low-detail ruined wall/shadow negative space for live menu buttons; detailed city and soldier occupy center and right. Muted charcoal, khaki, olive, restrained warm highlights, deep cinematic blacks, crisp detailed texture, premium 2007 military shooter atmosphere. Art fills the frame edge to edge. No text, no logo, no menus, no borders, no UI, no watermarks. Output landscape 16:9.

## Final background revision prompt

Edit this military main-menu background to look grounded, plausible, photographic and closer to a restrained gritty Call of Duty 4 environment rather than glossy AI concept art. Preserve wide 16:9 framing, dark usable left third for live menu, seated rear-view soldier in center-right, ruined urban setting and helicopter upper right. Replace dramatic orange sunset with ordinary overcast late-afternoon dusty gray sky, diffuse natural illumination, just a faint warm horizon. Remove theatrical god rays, orange HDR bloom and postcard mountain panorama. Buildings should be a coherent modest cluster of 3-5 ordinary worn concrete apartment blocks with consistent straight edges, realistic windows/balconies, less repetitive busy micro-detail. The street and rubble should look practical and naturally irregular, not sculptural. Soldier wears plausible 2007-era combat gear and a normal identifiable rifle, clear straps/pouches with restrained detail, no ornate sci-fi equipment. Distant helicopter smaller, realistic proportions, subtle rotor motion blur. Use subdued olive-gray, concrete brown, charcoal, washed khaki, restrained contrast with readable midtones, slight natural film grain, documentary still / original 2007 military game promotional photography feeling. No text, UI, logo, borders or watermark.

## Validation

The multiplayer executable was rebuilt successfully. The runtime loaded all eight custom materials without missing-image or menu-parse errors. Screenshots verified the menu at 1920 x 1080 before and after vid_restart; the process exited normally. Temporary screenshot/quit commands were removed from both menu copies after testing. The custom product footer reads v0.0.1; the stock build number is suppressed while this front page is visible. Stock submenu styling remains unchanged.
