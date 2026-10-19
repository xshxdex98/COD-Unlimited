// The match menus' look, shared by the pause menu and every page it opens: a side panel of
// actions over the blurred game, a card on the right, the front end's colours.

#define TEXT_COLOR			.88 .91 .88 1
#define TITLE_COLOR			.94 .97 .93 1
#define CAPTION_COLOR		.55 .61 .55 1
#define BODY_COLOR			.74 .80 .75 1
#define DISABLED_COLOR		.40 .45 .42 1
#define ACCENT				.64 .9 .49 1
#define ACCENT_OFF			.64 .9 .49 0
#define PANEL				.035 .052 .047 .72
#define PANEL_BORDER		.20 .25 .23 .7
#define FOCUS				.13 .22 .115 .83
#define FOCUS_BORDER		.40 .57 .33 .85
#define SIDE_PANEL			.012 .02 .018 .82
#define RULE				.28 .34 .29 .5

#define FILL( x, y, w, h, align, color ) \
	itemDef { style 1 rect x y w h align backcolor color visible 1 decoration }

#define CAPTION( x, y, w, align, label ) \
	itemDef { rect x y w 14 align text label textfont 0 textscale .17 textaligny 11 forecolor CAPTION_COLOR visible 1 decoration }

// Every action is a button in group nav_buttons with an accent edge in nav_edges; focusing one
// resets the others. Variants of one action (per team, locked or not) share their names.
#define BUTTON_FOCUS( button, edge ) \
	setitemcolor nav_buttons backcolor PANEL; setitemcolor nav_buttons bordercolor PANEL_BORDER; \
	setitemcolor nav_edges backcolor ACCENT_OFF; \
	setitemcolor button backcolor FOCUS; setitemcolor button bordercolor FOCUS_BORDER; setitemcolor edge backcolor ACCENT;

#define NAV_BUTTON( button, edge, x, y, w, h, texty, textArg, actionArg, focusArg, visArg ) \
	itemDef { \
		name			button \
		group			"nav_buttons" \
		type			1 \
		style			1 \
		rect			x y w h 1 0 \
		backcolor		PANEL \
		border			1 \
		bordersize		.5 \
		bordercolor		PANEL_BORDER \
		textArg \
		textfont		2 \
		textscale		.25 \
		textalignx		16 \
		textaligny		texty \
		forecolor		TEXT_COLOR \
		visible			visArg \
		onFocus			{ BUTTON_FOCUS( button, edge ) focusArg play mouse_over; } \
		action			{ play mouse_click; actionArg } \
	} \
	itemDef { \
		name			edge \
		group			"nav_edges" \
		style			1 \
		rect			x y 2 h 1 0 \
		backcolor		ACCENT_OFF \
		visible			visArg \
		decoration \
	}

#define BUTTON( button, edge, y, label, actionArg, visArg ) \
	NAV_BUTTON( button, edge, 28, y, 236, 28, 19, text label, actionArg, ;, visArg )

// A shorter row for long lists such as the classes.
#define LIST_BUTTON( button, edge, y, textArg, actionArg, focusArg, visArg ) \
	NAV_BUTTON( button, edge, 28, y, 236, 24, 17, textArg, actionArg, focusArg, visArg )

#define DISABLED_ROW( y, h, texty, textArg, note, visArg ) \
	itemDef { \
		style			1 \
		rect			28 y 236 h 1 0 \
		backcolor		.025 .035 .031 .55 \
		border			1 \
		bordersize		.5 \
		bordercolor		.14 .17 .15 .6 \
		textArg \
		textfont		2 \
		textscale		.25 \
		textalignx		16 \
		textaligny		texty \
		forecolor		DISABLED_COLOR \
		visible			visArg \
		decoration \
	} \
	itemDef { \
		rect			28 y 224 h 1 0 \
		text			note \
		textfont		0 \
		textscale		.16 \
		textalign		2 \
		textaligny		texty \
		forecolor		DISABLED_COLOR \
		visible			visArg \
		decoration \
	}

#define DISABLED_BUTTON( y, label, visArg )		DISABLED_ROW( y, 28, 19, text label, "", visArg )
#define LOCKED_LIST_BUTTON( y, textArg, visArg )	DISABLED_ROW( y, 24, 17, textArg, "LOCKED", visArg )

// The dimmed match, the side panel and its heading: a caption, the title and a lime subtitle.
#define PAGE( caption, titleArg, subtitleArg ) \
	FILL( 0, 0, 640, 480, 4 4, 0 0 0 .35 ) \
	FILL( 0, 0, 292, 480, 1 4, SIDE_PANEL ) \
	FILL( 292, 0, .5, 480, 1 4, RULE ) \
	CAPTION( 28, 30, 240, 1 0, caption ) \
	itemDef { rect 28 48 250 28 1 0 titleArg textfont 2 textscale .40 textaligny 24 forecolor TITLE_COLOR visible 1 decoration } \
	itemDef { rect 28 80 250 18 1 0 subtitleArg textfont 0 textscale .22 textaligny 14 forecolor ACCENT visible 1 decoration } \
	FILL( 29, 104, 23, 1, 1 0, .62 .86 .47 .9 ) \
	FILL( 54, 104, 17, 1, 1 0, .24 .36 .25 .5 )

#define SECTION( y, label )	CAPTION( 30, y, 230, 1 0, label )

// A card on the right of the screen; contents are placed from x -256 (its inner left edge).
#define CARD( y, h ) \
	FILL( -268, y, 244, h, 3 0, SIDE_PANEL ) \
	FILL( -268, y, 244, .5, 3 0, RULE )

#define FOOTER( hint ) \
	FILL( 0, 453, 640, 27, 4 4, .01 .015 .013 .83 ) \
	FILL( 0, 453, 640, .5, 4 4, RULE ) \
	itemDef { rect 28 459 240 15 1 0 text "COD UNLIMITED   |   v0.0.1" textfont 0 textscale .17 textaligny 11 forecolor CAPTION_COLOR visible 1 decoration } \
	itemDef { rect -158 459 130 15 3 0 text hint textfont 0 textscale .17 textalign 2 textaligny 11 forecolor CAPTION_COLOR visible 1 decoration }

#define MAP_NAME			"@" + tableLookup( "mp/mapsTable.csv", 0, dvarString( mapname ), 3 )

// A list in a card, styled like the front end's: feeder lists (maps, players) keep their engine
// behaviour, only the frame and selection colours are ours.
#define CARD_LIST( listName, y, h, feederArg, columnsArg, doubleClickArg ) \
	itemDef { \
		name			listName \
		type			6 \
		style			1 \
		rect			-256 y 220 h 3 0 \
		backcolor		.025 .04 .033 .7 \
		border			1 \
		bordersize		.5 \
		bordercolor		PANEL_BORDER \
		outlinecolor	.13 .22 .115 .9 \
		forecolor		TEXT_COLOR \
		textfont		1 \
		textscale		.22 \
		textalignx		6 \
		textaligny		0 \
		elementtype		0 \
		elementwidth	120 \
		elementheight	22 \
		feeder			feederArg \
		columns			columnsArg \
		visible			1 \
		doubleclick		{ play mouse_click; doubleClickArg } \
	}

// The card's action, its own button below the list.
#define CARD_BUTTON( button, y, label, actionArg ) \
	itemDef { \
		name			button \
		type			1 \
		style			1 \
		rect			-256 y 220 28 3 0 \
		backcolor		FOCUS \
		border			1 \
		bordersize		.5 \
		bordercolor		FOCUS_BORDER \
		text			label \
		textfont		2 \
		textscale		.25 \
		textalign		1 \
		textaligny		19 \
		forecolor		TITLE_COLOR \
		visible			1 \
		onFocus			{ setitemcolor button bordercolor ACCENT; play mouse_over; } \
		leaveFocus		{ setitemcolor button bordercolor FOCUS_BORDER; } \
		action			{ play mouse_click; actionArg } \
	}

#define HAS_UAV				(dvarInt( ui_uav_allies ) && team( name ) == "TEAM_ALLIES") || (dvarInt( ui_uav_axis ) && team( name ) == "TEAM_AXIS") || (dvarInt( ui_uav_client ) && team( name ) == "TEAM_FREE")

#define MINIMAP_LAYER( ownerdrawArg, material, visArg ) \
	itemDef { rect -256 64 220 220 3 0 forecolor 1 1 1 1 background material ownerdraw ownerdrawArg visible visArg decoration }

// The tactical map and the mode's briefing.
#define MATCH_CARD \
	CARD( 34, 352 ) \
	CAPTION( -256, 42, 220, 3 0, "TACTICAL MAP" ) \
	FILL( -256, 64, 220, 220, 3 0, 0 0 0 .5 ) \
	MINIMAP_LAYER( 181, "compass_map_default", 1 ) \
	itemDef { \
		style			3 \
		rect			-256 64 220 220 3 0 \
		forecolor		1 1 1 1 \
		exp				material( tableLookup( "mp/mapsTable.csv", 0, dvarString( mapname ), 7 ) ); \
		visible			1 \
		decoration \
	} \
	MINIMAP_LAYER( 182, "objective_line", 1 ) \
	MINIMAP_LAYER( 185, "compassping_friendly", 1 ) \
	MINIMAP_LAYER( 188, "compassping_enemy", when( dvarBool( ui_hud_hardcore ) == 0 || HAS_UAV ) ) \
	MINIMAP_LAYER( 183, "compassping_player", 1 ) \
	CAPTION( -256, 296, 220, 3 0, "OBJECTIVE" ) \
	itemDef { \
		rect			-256 314 220 64 3 0 \
		exp				text( gametypedescription() ); \
		textfont		0 \
		textscale		.19 \
		textaligny		12 \
		forecolor		BODY_COLOR \
		autowrapped \
		visible			1 \
		decoration \
	}
