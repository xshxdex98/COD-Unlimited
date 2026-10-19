// The highlighted class's loadout, in a card on the right. The including menu defines the
// STAT_CAC_* values the way stock ui_mp/cac_loadout.inc expects them, and the table columns are
// the stock ones: statsTable 3 name, 4 reference, 6 image, 7 description; attachmentTable keyed
// by column 9 with 3 name and 6 image.

#define PERK_REFERENCE( perk )	tablelookup( "mp/statstable.csv", 1, perk, 4 )
#define TWO_PRIMARIES			(PERK_REFERENCE( STAT_CAC_SPECIALTY_WEAPON ) == "specialty_twoprimaries")
#define SPECIAL_GRENADES		(PERK_REFERENCE( STAT_CAC_SPECIALTY_EQUIPMENT ) == "specialty_specialgrenade")
#define EXTRA_FRAG				(PERK_REFERENCE( STAT_CAC_SPECIALTY_EQUIPMENT ) == "specialty_fraggrenade")

#define LOADOUT_IMAGE( x, y, w, h, materialArg, visArg ) \
	itemDef { style 3 rect x y w h 3 0 forecolor 1 1 1 1 exp material( materialArg ); visible visArg decoration }

#define LOADOUT_TEXT( x, y, w, h, texty, font, scale, color, textArg, visArg ) \
	itemDef { rect x y w h 3 0 exp text( textArg ); textfont font textscale scale textaligny texty forecolor color visible visArg decoration }

#define PERK_ROW( y, perk, nameArg, nameVis ) \
	LOADOUT_IMAGE( -256, y, 26, 26, tablelookup( "mp/statstable.csv", 1, perk, 6 ), 1 ) \
	LOADOUT_TEXT( -222, (y-1), 186, 14, 12, 2, .20, TITLE_COLOR, nameArg, when( nameVis ) ) \
	LOADOUT_TEXT( -222, (y+13), 186, 12, 10, 0, .15, BODY_COLOR, "@" + tablelookup( "mp/statstable.csv", 1, perk, 7 ), 1 )

CARD( 34, 396 )

CAPTION( -256, 42, 220, 3 0, "PRIMARY" )
LOADOUT_IMAGE( -246, 56, 200, 100, tablelookup( "mp/statstable.csv", 0, STAT_CAC_PRIMARY, 6 ), 1 )
LOADOUT_TEXT( -256, 158, 220, 18, 15, 2, .26, TITLE_COLOR, "@" + tablelookup( "mp/statstable.csv", 0, STAT_CAC_PRIMARY, 3 ), 1 )
LOADOUT_IMAGE( -256, 179, 16, 16, tablelookup( "mp/attachmenttable.csv", 9, STAT_CAC_PRIMARY_ATTACHMENT, 6 ), 1 )
LOADOUT_TEXT( -236, 180, 200, 14, 12, 0, .18, BODY_COLOR, "@" + tablelookup( "mp/attachmenttable.csv", 9, STAT_CAC_PRIMARY_ATTACHMENT, 3 ), 1 )

CAPTION( -256, 204, 220, 3 0, "SECONDARY" )
LOADOUT_IMAGE( -256, 220, 120, 60, tablelookup( "mp/statstable.csv", 0, STAT_CAC_SECONDARY, 6 ), when( TWO_PRIMARIES ) )
LOADOUT_IMAGE( -226, 220, 60, 60, tablelookup( "mp/statstable.csv", 0, STAT_CAC_SECONDARY, 6 ), when( !TWO_PRIMARIES ) )
LOADOUT_TEXT( -128, 230, 92, 16, 13, 2, .21, TITLE_COLOR, "@" + tablelookup( "mp/statstable.csv", 0, STAT_CAC_SECONDARY, 3 ), 1 )
LOADOUT_TEXT( -128, 250, 92, 14, 12, 0, .16, BODY_COLOR, "@" + tablelookup( "mp/attachmenttable.csv", 9, STAT_CAC_SECONDARY_ATTACHMENT, 3 ), 1 )

CAPTION( -256, 288, 220, 3 0, "PERKS" )
// Perk 1 is "3x" the special grenade when it is the special grenade perk.
PERK_ROW( 304, STAT_CAC_SPECIALTY_EQUIPMENT, "@" + tablelookup( "mp/statstable.csv", 1, STAT_CAC_SPECIALTY_EQUIPMENT, 3 ), !SPECIAL_GRENADES )
LOADOUT_TEXT( -222, 303, 186, 14, 12, 2, .20, TITLE_COLOR, "@PERKS_3X_" + tablelookup( "mp/statstable.csv", 0, STAT_CAC_SPECIAL_GRENADE, 3 ), when( SPECIAL_GRENADES ) )
PERK_ROW( 336, STAT_CAC_SPECIALTY_WEAPON, "@" + tablelookup( "mp/statstable.csv", 1, STAT_CAC_SPECIALTY_WEAPON, 3 ), 1 )
PERK_ROW( 368, STAT_CAC_SPECIALTY_ABILITY, "@" + tablelookup( "mp/statstable.csv", 1, STAT_CAC_SPECIALTY_ABILITY, 3 ), 1 )

// Grenades: a frag and the special grenade, with the perk that triples one of them.
LOADOUT_IMAGE( -256, 398, 26, 26, "weapon_fraggrenade", when( SPECIAL_GRENADES ) )
LOADOUT_IMAGE( -256, 398, 26, 26, "weapon_smokegrenade", when( EXTRA_FRAG ) )
LOADOUT_IMAGE( -260, 398, 26, 26, "weapon_fraggrenade", when( !SPECIAL_GRENADES && !EXTRA_FRAG ) )
LOADOUT_IMAGE( -248, 398, 26, 26, "weapon_smokegrenade", when( !SPECIAL_GRENADES && !EXTRA_FRAG ) )
LOADOUT_TEXT( -222, 404, 186, 14, 12, 2, .20, TITLE_COLOR, "@PERKS_FRAG_X_1", when( SPECIAL_GRENADES ) )
LOADOUT_TEXT( -222, 404, 186, 14, 12, 2, .20, TITLE_COLOR, "@" + tablelookup( "mp/statstable.csv", 0, STAT_CAC_SPECIAL_GRENADE, 3 ), when( EXTRA_FRAG ) )
LOADOUT_TEXT( -222, 404, 186, 14, 12, 2, .20, TITLE_COLOR, "@PERKS_FRAG_1_" + tablelookup( "mp/statstable.csv", 0, STAT_CAC_SPECIAL_GRENADE, 3 ), when( !SPECIAL_GRENADES && !EXTRA_FRAG ) )
