/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "debugconsole/console.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "io/key.h"
#include "localization/localize.h"
#include "menuui/snazzyui.h"
#include "parse/parselo.h"
#include "playerman/player.h"
#include "popup/popup.h"
#include "stats/medals.h"
#include "ui/ui.h"

#ifndef NDEBUG
#include "cmdline/cmdline.h"
#endif

// define for the medal information
SCP_vector<medal_stuff> Medals;

// coords for indiv medal bitmaps
static int Default_medal_coords[GR_NUM_RESOLUTIONS][NUM_MEDALS_FS2][2] = {
	{				// GR_640
		{ 89, 47 },			// eps. peg. lib
		{ 486, 47 },		// imp. order o' vasuda
		{ 129, 130 },		// dist flying cross
		{ 208, 132 },		// soc service
		{ 361, 131 },		// dist intel cross
		{ 439, 130 },		// order of galatea
		{ 64, 234 },		// meritorious unit comm.
		{ 153, 234 },		// medal of valor
		{ 239, 241 },		// gtva leg of honor
		{ 326, 240 },		// allied defense citation
		{ 411, 234 },		// neb campaign victory
		{ 494, 234 },		// ntf campaign victory
		{ 189, 80 },		// rank
		{ 283, 91 },		// wings
		{ 372, 76 },		// bronze kills badge
		{ 403, 76 },		// silver kills badge
		{ 435, 76 },		// gold kills badge
		{ 300, 152 },		// SOC unit crest
	},
	{				// GR_1024
		{ 143, 75 },		// eps. peg. lib
		{ 777, 75 },		// imp. order o' vasuda
		{ 206, 208 },		// dist flying cross
		{ 333, 212 },		// soc service
		{ 578, 210 },		// dist intel cross
		{ 703, 208 },		// order of galatea
		{ 103, 374 },		// meritorious unit comm.
		{ 245, 374 },		// medal of valor
		{ 383, 386 },		// gtva leg of honor
		{ 522, 384 },		// allied defense citation
		{ 658, 374 },		// neb campaign victory
		{ 790, 374 },		// ntf campaign victory
		{ 302, 128 },		// rank
		{ 453, 146 },		// wings
		{ 595, 121 },		// bronze kills badge
		{ 646, 121 },		// silver kills badge
		{ 696, 121 },		// gold kills badge
		{ 480, 244 },		// SOC unit crest
	}
};

// debriefing bitmaps
static const char *Default_debriefing_bitmaps[NUM_MEDALS_FS2] =
{
	"DebriefMedal00",
	"DebriefMedal01",
	"DebriefMedal02",
	"DebriefMedal03",
	"DebriefMedal04",
	"DebriefMedal05",
	"DebriefMedal06",
	"DebriefMedal07",
	"DebriefMedal08",
	"DebriefMedal09",
	"DebriefMedal10",
	"DebriefMedal11",
	"DebriefRank##",
	"DebriefWings##",
	"DebriefBadge01",
	"DebriefBadge02",
	"DebriefBadge03",
	"DebriefCrest"
};

// argh
typedef struct coord2dw {
	int x,y,w;
} coord2dw;

// coords for the medal title
static int Default_medals_label_coords[GR_NUM_RESOLUTIONS][3] = {
	{ 241, 458, 300 },			// GR_640 x, y, w
	{ 386, 734, 480 }		// GR_1024 x, y, w
};
static coord2dw Medals_label_coords[GR_NUM_RESOLUTIONS];

// coords for the player callsign
static int Default_medals_callsign_coords[GR_NUM_RESOLUTIONS][2] = {
	{ -1, 54 },		// we'll use -1 as a convention to center it
	{ -1, 89 }
};
static coord2d Medals_callsign_coords[GR_NUM_RESOLUTIONS];

#define MEDALS_NUM_BUTTONS	1
#define MEDALS_EXIT			0
ui_button_info Medals_buttons[GR_NUM_RESOLUTIONS][MEDALS_NUM_BUTTONS] = {
	{ // GR_640
		ui_button_info("MEB_18",	574,	432,	-1,	-1,	18),
	},
	{ // GR_1024
		ui_button_info("2_MEB_18",	919,	691,	-1,	-1,	18),
	}
};
static int Exit_button_hotspot_override = -1;

#define MEDALS_NUM_TEXT		1
UI_XSTR Medals_text[GR_NUM_RESOLUTIONS][MEDALS_NUM_TEXT] = {
	{	// GR_640
		{"Exit",		1466,		587,	416,	UI_XSTR_COLOR_PINK, -1,	&Medals_buttons[GR_640][MEDALS_EXIT].button },
	},
	{	// GR_1024
		{"Exit",		1466,		943,	673,	UI_XSTR_COLOR_PINK, -1,	&Medals_buttons[GR_1024][MEDALS_EXIT].button },
	},
};

static const char* Default_medals_background_filename = "MedalsDisplayEmpty";
static char Medals_background_filename[NAME_LENGTH];

static const char* Default_medals_mask_filename = "Medals-M";
static char Medals_mask_filename[NAME_LENGTH];

scoring_struct *Player_score=NULL;

int Medals_mode;
player *Medals_player;

void init_medal_bitmaps();
void init_snazzy_regions();
void blit_medals();

// -----------------------------------------------------------------------------
// Main medals screen state
//

static bitmap *Medals_mask;
int Medals_mask_w, Medals_mask_h;
static int Medals_bitmap_mask;			// the mask for the medal case
static int Medals_bitmap;				// the medal case itself
static int Rank_bm;						// bitmap for the rank medal

static UI_WINDOW Medals_window;

typedef struct medal_display_info {
	int 		bitmap;						// image in the medal case
	coord2d		coords[GR_NUM_RESOLUTIONS];	// screen position (can now be defined by the table)
} medal_display_info;

static SCP_vector<medal_display_info> Medal_display_info;
static MENU_REGION *Medal_regions = NULL;

int Rank_medal_index = -1;


#define MEDAL_BITMAP_INIT (1<<0)
#define MASK_BITMAP_INIT  (1<<1)
int Init_flags;

medal_stuff::medal_stuff()
	: num_versions(1), version_starts_at_1(false), available_from_start(false), kills_needed(0)
{
	name[0] = '\0';
	bitmap[0] = '\0';
	debrief_bitmap[0] = '\0';
	voice_base[0] = '\0';
}

const char* medal_stuff::get_display_name() const {
	if (!alt_name.empty()) {
		return alt_name.c_str();
	} else {
		return name;
	}
}

static medal_stuff* get_medal_pointer(const char* medal_name)
{
	for (int i = 0; i < (int)Medals.size(); i++) {
		if (!stricmp(medal_name, Medals[i].name)) {
			return &Medals[i];
		}
	}

	// Didn't find anything.
	return nullptr;
}

static int get_medal_position(const char* medal_name)
{
	for (int i = 0; i < (int)Medals.size(); i++) {
		if (!stricmp(medal_name, Medals[i].name)) {
			return i;
		}
	}

	return -1;
}

void parse_medals_table(const char* filename)
{

	try
	{
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		required_string("#Medals");

		// special background information
		if (optional_string("+Background Bitmap:")) {
			stuff_string(Medals_background_filename, F_NAME, NAME_LENGTH);
		}
		else {
			strcpy_s(Medals_background_filename, Default_medals_background_filename);
		}

		// special mask information
		if (optional_string("+Mask Bitmap:")) {
			stuff_string(Medals_mask_filename, F_NAME, NAME_LENGTH);
		}
		else {
			strcpy_s(Medals_mask_filename, Default_medals_mask_filename);
		}

		// configurable hotspot for the exit button
		if (optional_string("+Exit Button Hotspot Index:")) {
			stuff_int(&Exit_button_hotspot_override);
		}

		// special positioning for player callsign
		if (optional_string("+Callsign Position 640:")) {
			stuff_int(&Medals_callsign_coords[GR_640].x);
			stuff_int(&Medals_callsign_coords[GR_640].y);
		}

		if (optional_string("+Callsign Position 1024:")) {
			stuff_int(&Medals_callsign_coords[GR_1024].x);
			stuff_int(&Medals_callsign_coords[GR_1024].y);
		}

		// special positioning for medal label
		if (optional_string("+Label Position 640:")) {
			stuff_int(&Medals_label_coords[GR_640].x);
			stuff_int(&Medals_label_coords[GR_640].y);
			stuff_int(&Medals_label_coords[GR_640].w);
		}

		if (optional_string("+Label Position 1024:")) {
			stuff_int(&Medals_label_coords[GR_1024].x);
			stuff_int(&Medals_label_coords[GR_1024].y);
			stuff_int(&Medals_label_coords[GR_1024].w);
		}

		// parse in all the medal names
		while (required_string_either("#End", "$Name:"))
		{
			medal_stuff medal_t;
			medal_stuff* medal_p;
			medal_display_info display_t;
			medal_display_info* display_p;
			bool create_if_not_found = true;

			required_string("$Name:");
			stuff_string(medal_t.name, F_NAME, NAME_LENGTH);

			if (optional_string("+nocreate")) {
				if (!Parsing_modular_table) {
					Warning(LOCATION, "+nocreate flag used for medal in non-modular table\n");
				}
				create_if_not_found = false;
			}

			// Does this medal exist already?
			// If so, load this new info into it
			medal_p = get_medal_pointer(medal_t.name);
			if (medal_p != nullptr) {
				if (!Parsing_modular_table) {
					error_display(1,
						"Error:  Medal %s already exists.  All medal names must be unique.", medal_t.name);
				}
				display_p = &Medal_display_info[get_medal_position(medal_p->name)];
			} else {
				// Don't create poof if it has +nocreate and is in a modular table.
				if (!create_if_not_found && Parsing_modular_table) {
					if (!skip_to_start_of_string_either("$Name:", "#end")) {
						error_display(1, "Missing [#end] or [$Name] after medal %s", medal_t.name);
					}
					continue;
				}

				Medals.push_back(medal_t);
				medal_p = &Medals[Medals.size() - 1];
				Medal_display_info.push_back(display_t);
				display_p = &Medal_display_info[Medal_display_info.size() - 1];
			}

			if (optional_string("$Alt Name:")) {
				stuff_string(medal_p->alt_name, F_NAME);
			}

			if (optional_string("$Bitmap:")) {
				stuff_string(medal_p->bitmap, F_NAME, MAX_FILENAME_LEN);
			}

			// Check here that the medal has a bitmap. If not, then error out
			if (!stricmp(medal_p->bitmap, "")) {
				error_display(1, "Missing valid bitmap file for medal %s", medal_p->name);
			}

			if (optional_string("+Position 640:")) {
				stuff_int(&display_p->coords[GR_640].x);
				stuff_int(&display_p->coords[GR_640].y);
			}
			else if (Medals.size() <= NUM_MEDALS_FS2) {
				display_p->coords[GR_640].x = Default_medal_coords[GR_640][Medals.size() - 1][0];
				display_p->coords[GR_640].y = Default_medal_coords[GR_640][Medals.size() - 1][1];
			}
			else {
				error_display(0, "No default GR_640 position for medal '%s'!", medal_p->name);
				display_p->coords[GR_640].x = 0;
				display_p->coords[GR_640].y = 0;
			}
			if (optional_string("+Position 1024:")) {
				stuff_int(&display_p->coords[GR_1024].x);
				stuff_int(&display_p->coords[GR_1024].y);
			}
			else if (Medals.size() <= NUM_MEDALS_FS2) {//
				display_p->coords[GR_1024].x = Default_medal_coords[GR_1024][Medals.size() - 1][0];
				display_p->coords[GR_1024].y = Default_medal_coords[GR_1024][Medals.size() - 1][1];
			}
			else {
				error_display(0, "No default GR_1024 position for medal '%s'!", medal_p->name);
				display_p->coords[GR_1024].x = 0;
				display_p->coords[GR_1024].y = 0;
			}

			if (optional_string("+Mask index:")) {
				stuff_int(&medal_p->mask_index);
			} else {
				medal_p->mask_index = get_medal_position(medal_p->name);
			}

			if (optional_string("+Debriefing Bitmap:")) {
				stuff_string(medal_p->debrief_bitmap, F_NAME, MAX_FILENAME_LEN);
			}
			else if (Medals.size() <= NUM_MEDALS_FS2) {
				strcpy_s(medal_p->debrief_bitmap, Default_debriefing_bitmaps[Medals.size() - 1]);
			}
			else {
				error_display(0, "No default debriefing bitmap for medal '%s'!", medal_p->name);
				strcpy_s(medal_p->debrief_bitmap, "");
			}

			if (optional_string("$Num mods:")) {
				stuff_int(&medal_p->num_versions);
			} else {
				medal_p->num_versions = 1;
			}

			// this is dumb
			//this setting only exists because Volition started the numbering 
			//for Rank images at DebriefRank01 and for Wings images at DebriefWings00.
			medal_p->version_starts_at_1 = (!stricmp(medal_p->name, "Rank"));
			if (optional_string("+Version starts at 1:")) {
				stuff_boolean(&medal_p->version_starts_at_1);
			}

			if (optional_string("+Available From Start:")) {
				stuff_boolean(&medal_p->available_from_start);
			}

			// some medals are based on kill counts.  When string +Num Kills: is present, we know that
			// this medal is a badge and should be treated specially
			if (optional_string("+Num Kills:")) {
				char buf[MULTITEXT_LENGTH];
				int persona;
				stuff_int(&medal_p->kills_needed);

				if (optional_string("$Wavefile 1:"))
					stuff_string(medal_p->voice_base, F_NAME, MAX_FILENAME_LEN);

				if (optional_string("$Wavefile 2:"))
					stuff_string(medal_p->voice_base, F_NAME, MAX_FILENAME_LEN);

				if (optional_string("$Wavefile Base:"))
					stuff_string(medal_p->voice_base, F_NAME, MAX_FILENAME_LEN);

				while (check_for_string("$Promotion Text:")) {
					required_string("$Promotion Text:");
					stuff_string(buf, F_MULTITEXT, sizeof(buf));
					persona = -1;
					if (optional_string("+Persona:")) {
						stuff_int(&persona);
						if (persona < 0) {
							error_display(0,
								"Debriefing text for %s is assigned to an invalid persona: %i (must be 0 or "
								"greater).\n",
								medal_p->name,
								persona);
							continue;
						}
					}
					medal_p->promotion_text[persona] = SCP_string(buf);
				}
				if (medal_p->promotion_text.find(-1) == medal_p->promotion_text.end()) {
					error_display(0, "%s medal is missing default debriefing text.\n", medal_p->name);
					medal_p->promotion_text[-1] = "";
				}
			}
		}

		required_string("#End");

	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", "medals.tbl", e.what()));
		return;
	}
}

void medals_init()
{
	//Set the retail defaults here - Mjn
	Medals_callsign_coords[GR_640].x = Default_medals_callsign_coords[GR_640][0];
	Medals_callsign_coords[GR_640].y = Default_medals_callsign_coords[GR_640][1];

	Medals_callsign_coords[GR_1024].x = Default_medals_callsign_coords[GR_1024][0];
	Medals_callsign_coords[GR_1024].y = Default_medals_callsign_coords[GR_1024][1];

	Medals_label_coords[GR_640].x = Default_medals_label_coords[GR_640][0];
	Medals_label_coords[GR_640].y = Default_medals_label_coords[GR_640][1];
	Medals_label_coords[GR_640].w = Default_medals_label_coords[GR_640][2];

	Medals_label_coords[GR_1024].x = Default_medals_label_coords[GR_1024][0];
	Medals_label_coords[GR_1024].y = Default_medals_label_coords[GR_1024][1];
	Medals_label_coords[GR_1024].w = Default_medals_label_coords[GR_1024][2];

	// first parse the default table
	parse_medals_table("medals.tbl");

	// parse any modular tables
	parse_modular_table("*-mdl.tbm", parse_medals_table);


	Rank_medal_index = get_medal_position("Rank");

	// be sure that we know where the rank is
	if (Rank_medal_index < 0) {
		Warning(LOCATION, "Could not find the 'Rank' medal!");
		Rank_medal_index = 0;
	}
}

// replacement for -gimmemedals
DCF(medals, "Grant or revoke medals")
{
	int i;
	int idx;

	if (dc_optional_string_either("help", "--help"))
	{
		dc_printf ("Usage: medals all | clear | promote | demote | [index]\n");
		dc_printf ("       [index] --  index of medal to grant\n");
		dc_printf ("       with no parameters, displays the available medals\n");
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?"))
	{
		dc_printf("You have the following medals:\n");

		for (i = 0; i < (int)Medals.size(); i++)
		{
			if (Player->stats.medal_counts[i] > 0)
				dc_printf("%d %s\n", Player->stats.medal_counts[i], Medals[i].name);
		}
		dc_printf("%s\n", Ranks[verify_rank(Player->stats.rank)].name);
		return;
	}

	if (dc_optional_string("all")) {
		for (i = 0; i < (int)Medals.size(); i++) {
			Player->stats.medal_counts[i]++;
		}
		dc_printf("Granted all medals\n");
		return;

	} else if (dc_optional_string("clear")) {
		for (i = 0; i < (int)Medals.size(); i++) {
			Player->stats.medal_counts[i] = 0;
		}
		dc_printf("Cleared all medals\n");
		return;

	} else if (dc_optional_string("promote")) {
		if (Player->stats.rank < ((int)Ranks.size() - 1)) {
			Player->stats.rank++;
		}
		dc_printf("Promoted to %s\n", Ranks[verify_rank(Player->stats.rank)].name);
		return;

	} else if (dc_optional_string("demote")) {
		if (Player->stats.rank > 0) {
			Player->stats.rank--;
		}
		dc_printf("Demoted to %s\n", Ranks[verify_rank(Player->stats.rank)].name);
		return;
	}

	if (dc_maybe_stuff_int(&idx)) {
		if (idx < 0 || idx >= (int)Medals.size())
		{
			dc_printf("Medal index %d is out of range\n", idx);
			return;
		}

		dc_printf("Granted %s\n", Medals[idx].name);
		Player->stats.medal_counts[idx]++;
		return;
	}

	dc_printf("The following medals are available:\n");
	for (i = 0; i < (int)Medals.size(); i++) {
		dc_printf("%d: %s\n", i, Medals[i].name);
	}
}


void medal_main_init(player *pl, int mode)
{
	int idx;
	char bitmap_buf[NAME_LENGTH];

	Assert(pl != NULL);
	Medals_player = pl;
	Player_score = &Medals_player->stats;

#ifndef NDEBUG
	if (Cmdline_gimme_all_medals){
		for (idx = 0; idx < (int)Medals.size(); idx++) {
			Medals_player->stats.medal_counts[idx] = 1;
		}
	}
#endif

	for (idx = 0; idx < (int)Medals.size(); idx++) {
		if ((Medals[idx].available_from_start) && (Medals_player->stats.medal_counts[idx] < 1)) {
			Medals_player->stats.medal_counts[idx] = 1;
		}
	}

	Medals_mode = mode;
	snazzy_menu_init();
	Medals_window.create( 0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0 );

	// maybe override which hotspot is used for the exit button
	if (Exit_button_hotspot_override >= 0) {
		for (idx = 0; idx < GR_NUM_RESOLUTIONS; idx++) {
			Medals_buttons[idx][MEDALS_EXIT].hotspot = Exit_button_hotspot_override;
		}
	}

	// create the interface buttons
	for (idx=0; idx<MEDALS_NUM_BUTTONS; idx++) {
		// create the object
		Medals_buttons[gr_screen.res][idx].button.create(&Medals_window, "", Medals_buttons[gr_screen.res][idx].x, Medals_buttons[gr_screen.res][idx].y, 1, 1, 0, 1);

		// set the sound to play when highlighted
		Medals_buttons[gr_screen.res][idx].button.set_highlight_action(common_play_highlight_sound);

		// set the ani for the button
		Medals_buttons[gr_screen.res][idx].button.set_bmaps(Medals_buttons[gr_screen.res][idx].filename);

		// set the hotspot
		Medals_buttons[gr_screen.res][idx].button.link_hotspot(Medals_buttons[gr_screen.res][idx].hotspot);
	}

	// add all xstrs
	for (idx=0; idx<MEDALS_NUM_TEXT; idx++) {
		Medals_window.add_XSTR(&Medals_text[gr_screen.res][idx]);
	}

	Init_flags = 0;

	strcpy_s(bitmap_buf, Resolution_prefixes[gr_screen.res]);
	strcat_s(bitmap_buf, Medals_background_filename);

	Medals_bitmap = bm_load(bitmap_buf);
	if (Medals_bitmap < 0) {
		Warning(LOCATION, "Error loading medal background bitmap %s", bitmap_buf);
	} else {
		Init_flags |= MEDAL_BITMAP_INIT;
	}

	Medals_mask_w = -1;
	Medals_mask_h = -1;

	strcpy_s(bitmap_buf, Resolution_prefixes[gr_screen.res]);
	strcat_s(bitmap_buf, Medals_mask_filename);

	Medals_bitmap_mask = bm_load(bitmap_buf);
	if (Medals_bitmap_mask < 0) {
		Warning(LOCATION, "Error loading medal mask file %s", bitmap_buf);
	} else {
		Init_flags |= MASK_BITMAP_INIT;
		Medals_mask = bm_lock(Medals_bitmap_mask, 8, BMP_AABITMAP | BMP_MASK_BITMAP);
		bm_get_info(Medals_bitmap_mask, &Medals_mask_w, &Medals_mask_h);

		init_medal_bitmaps();
	}

	init_snazzy_regions();

	gr_set_color_fast(&Color_normal);

	if (Medals_bitmap_mask >= 0)
		Medals_window.set_mask_bmap(bitmap_buf);
}

void blit_label(const char *label, int num)
{
	int x, y, sw;
	char text[256];

	gr_set_color_fast(&Color_bright);

	// translate medal names before displaying
	// can't translate in table cuz the names are used in comparisons
	// Medals now have alternate display names so this code can be disabled in the mod table
	if (Lcl_gr && !Disable_built_in_translations) {
		char translated_label[256];
		strcpy_s(translated_label, label);
		lcl_translate_medal_name_gr(translated_label);

		// set correct string
		if ( num > 1 ) {
			sprintf_safe( text, NOX("%s (%d)"), translated_label, num );
		} else {
			sprintf_safe( text, "%s", translated_label );
		}
	} else if (Lcl_pl && !Disable_built_in_translations) {
		char translated_label[256];
		strcpy_s(translated_label, label);
		lcl_translate_medal_name_pl(translated_label);

		// set correct string
		if ( num > 1 ) {
			sprintf_safe( text, NOX("%s (%d)"), translated_label, num );
		} else {
			sprintf_safe( text, "%s", translated_label );
		}
	} else {
		// set correct string
		if ( num > 1 ) {
			sprintf_safe( text, NOX("%s (%d)"), label, num );
		} else {
			sprintf_safe( text, "%s", label );
		}
	}

	// find correct coords
	gr_get_string_size(&sw, NULL, text);
	x = Medals_label_coords[gr_screen.res].x + (Medals_label_coords[gr_screen.res].w - sw) / 2;
	y = Medals_label_coords[gr_screen.res].y;

	// do it
	gr_string(x, y, text, GR_RESIZE_MENU);
}

void blit_callsign()
{
	int x, y;

	gr_set_color_fast(&Color_normal);

	// find correct coords
	x = Medals_callsign_coords[gr_screen.res].x;
	y = Medals_callsign_coords[gr_screen.res].y;

	// nothing special, just do it.
	// Goober5000 - from previous code revisions, I assume 0x8000 means center it on-screen...
	// m!m - 0x8000 was removed so we need to calculate it ourself
	if (x < 0)
	{
		int w;
		gr_get_string_size(&w, NULL, Medals_player->callsign);

		x = (gr_screen.clip_width_unscaled - w) / 2;
	}
	gr_string(x, y, Medals_player->callsign, GR_RESIZE_MENU);
}

int medal_main_do()
{
	int region,selected, k;
	int medal_index = -1;

	// If we don't have a mask, we don't have enough data to do anything with this screen.
	if (Medals_bitmap_mask == -1) {
		popup_game_feature_not_in_demo();
		if (Medals_mode == MM_NORMAL)
			gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
		// any calling popup function will know to close the screen down
		return 0;
	}

	k = Medals_window.process();

	// process an exit command
	if ( (k == KEY_ESC) && (Medals_mode == MM_NORMAL) ) {
		gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
	}

	// draw the background medal display case
	gr_reset_clip();
	GR_MAYBE_CLEAR_RES(Medals_bitmap);
	if (Medals_bitmap != -1) {
		gr_set_bitmap(Medals_bitmap);
		gr_bitmap(0,0,GR_RESIZE_MENU);
	}

	// check to see if a button was pressed
	if ( (k == (KEY_CTRLED|KEY_ENTER)) || (Medals_buttons[gr_screen.res][MEDALS_EXIT].button.pressed()) ) {
		gamesnd_play_iface(InterfaceSounds::COMMIT_PRESSED);
		if (Medals_mode == MM_NORMAL) {
			gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
		} else {
			// any calling popup function will know to close the screen down
			return 0;
		}
	}

	// blit medals also takes care of blitting the rank insignia
	blit_medals();
	blit_callsign();

	region = snazzy_menu_do((ubyte*)Medals_mask->data,
		Medals_mask_w,
		Medals_mask_h,
		(int)Medals.size(),
		Medal_regions,
		&selected);

	for (int i = 0; i < (int)Medals.size(); i++) {
		if (region == Medals[i].mask_index) {
			medal_index = i;
		}
	}

	if (medal_index == Rank_medal_index)
	{
		blit_label(Ranks[verify_rank(Player_score->rank)].name, 1);
	}
	else switch (region)
	{
		case ESC_PRESSED:
			if (Medals_mode == MM_NORMAL) {
				gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
			} else {
				// any calling popup function will know to close the screen down
				return 0;
			}
			break;

		case -1:
			break;

		default:
			if (Player_score->medal_counts[medal_index] > 0) {
				blit_label(Medals[medal_index].get_display_name(), Player_score->medal_counts[medal_index]);
			}
			break;
	} // end switch

	Medals_window.draw();

	gr_flip();

	return 1;
}

void medal_main_close()
{
	if (Init_flags & MEDAL_BITMAP_INIT)
		bm_release(Medals_bitmap);

	if (Init_flags & MASK_BITMAP_INIT) {
		bm_unlock(Medals_bitmap_mask);
	}

	for (SCP_vector<medal_display_info>::iterator idx = Medal_display_info.begin(); idx != Medal_display_info.end(); ++idx) {
		if (idx->bitmap >= 0){
			bm_release(idx->bitmap);
		}
		idx->bitmap = -1;
	}

	delete[] Medal_regions;
	Medal_regions = NULL;

	Player_score = NULL;

	Medals_window.destroy();

	if (Init_flags & MASK_BITMAP_INIT) {
		bm_release(Medals_bitmap_mask);
	}

	snazzy_menu_close();
}

// function to load in the medals for this player.  It loads medals that the player has (known
// by whether or not a non-zero number is present in the player's medal array), then loads the
// rank bitmap
void init_medal_bitmaps()
{
	int idx;
	Assert(Player_score);

	for (idx = 0; idx < (int)Medals.size(); idx++) {
		Medal_display_info[idx].bitmap = -1;

		if (Player_score->medal_counts[idx] > 0) {
			int num_medals;
			char filename[MAX_FILENAME_LEN], base[MAX_FILENAME_LEN];

			// possibly load a different filename that is specified by the bitmap filename
			// for this medal.  if the player has > 1 of these types of medals, then determien
			// which of the possible version to use based on the player's count of this medal
			strcpy_s( filename, Medals[idx].bitmap );

			_splitpath( filename, NULL, NULL, base, NULL );

			num_medals = Player_score->medal_counts[idx];

			// can't display more than the maximum number of version for this medal
			if ( num_medals > Medals[idx].num_versions )
				num_medals = Medals[idx].num_versions;

			if ( num_medals > 1 ) {
				// append the proper character onto the end of the medal filename.  Base version
				// has no character. next version is a, then b, etc.
				char temp[MAX_FILENAME_LEN];
				strcpy_s(temp, base);
				sprintf_safe( base, "%s%c", temp, (num_medals-2)+'a');
			}

			// hi-res support
			if (gr_screen.res == GR_1024) {
				sprintf_safe( filename, "2_%s", base );
			}

			// base now contains the actual medal bitmap filename needed to load
			// we don't need to pass extension to bm_load anymore, so just use the basename
			// as is.
			Medal_display_info[idx].bitmap = bm_load((gr_screen.res == GR_1024) ? filename : base);
			Assert( Medal_display_info[idx].bitmap != -1 );
		}
	}

	// load up rank insignia
	if (gr_screen.res == GR_1024) {
		char filename[NAME_LENGTH];
		if (snprintf(filename, NAME_LENGTH, "2_%s", Ranks[verify_rank(Player_score->rank)].bitmap) >= NAME_LENGTH) {
			// Make sure the string is null terminated
			filename[NAME_LENGTH - 1] = '\0';
		}
		Rank_bm = bm_load(filename);
	} else {
		Rank_bm = bm_load(Ranks[verify_rank((int)Player_score->rank)].bitmap);
	}
}

void init_snazzy_regions()
{
	int idx;

	// well, we need regions in an array (versus a vector), so...
	Assert(Medal_regions == NULL);
	Medal_regions = new MENU_REGION[(int)Medals.size()];

	// snazzy regions for the medals/ranks, etc.
	for (idx = 0; idx < (int)Medals.size(); idx++) {
		snazzy_menu_add_region(&Medal_regions[idx], "", idx, 0);
	}
}

// blit the medals -- this includes the rank insignia
void blit_medals()
{
	int idx;

	for (idx = 0; idx < (int)Medals.size(); idx++) {
		if (idx != Rank_medal_index && Player_score->medal_counts[idx] > 0) {
#ifndef NDEBUG
			// this can happen if gimmemedals was used on the medal screen
			if (Medal_display_info[idx].bitmap < 0) {
				continue;
			}
#endif
			gr_set_bitmap(Medal_display_info[idx].bitmap);
			gr_bitmap(Medal_display_info[idx].coords[gr_screen.res].x, Medal_display_info[idx].coords[gr_screen.res].y, GR_RESIZE_MENU);
		}
	}

	// now blit rank, since that "medal" doesn't get loaded (or drawn) the normal way
	gr_set_bitmap(Rank_bm);
	gr_bitmap(Medal_display_info[Rank_medal_index].coords[gr_screen.res].x, Medal_display_info[Rank_medal_index].coords[gr_screen.res].y, GR_RESIZE_MENU);
}

int medals_info_lookup(const char *name)
{
	if ( !name ) {
		return -1;
	}

	for (int i = 0; i < (int)Medals.size(); i++) {
		if ( !stricmp(name, Medals[i].name) ) {
			return i;
		}
	}

	return -1;
}
