/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
*/

#include <sstream>
#include <climits>

#include "gamesnd.h"
#include "gamesnd/gamesnd.h"
#include "localization/localize.h"
#include "parse/parselo.h"
#include "sound/ds.h"
#include "species_defs/species_defs.h"
#include "tracing/tracing.h"

SCP_vector<game_snd>	Snds;
SCP_vector<game_snd>	Snds_iface;
SCP_vector<sound_handle> Snds_iface_handle;

// jg18 - default priorities and limits for retail gameplay sounds
static const int NUM_RETAIL_GAMEPLAY_SOUNDS = 192; // indices 0-191, from retail sounds.tbl
static EnhancedSoundData Default_sound_priorities[NUM_RETAIL_GAMEPLAY_SOUNDS] =
{
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_MISSILE_TRACKING           = 0,  //!< Missle tracking to acquire a lock (looped)
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_MISSILE_LOCK               = 1,  //!< Missle lock (non-looping)
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_PRIMARY_CYCLE              = 2,  //!< cycle primary weapon
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_SECONDARY_CYCLE            = 3,  //!< cycle secondary weapon
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_ENGINE                     = 4,  //!< engine sound (as heard in cockpit)
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_CARGO_REVEAL               = 5,  //!< cargo revealed
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  3), // SND_DEATH_ROLL                 = 6,  //!< ship death roll
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  2), // SND_SHIP_EXPLODE_1             = 7,  //!< ship explosion 1
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_TARGET_ACQUIRE             = 8,  //!< target acquried
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_ENERGY_ADJUST              = 9,  //!< energy level change success
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_ENERGY_ADJUST_FAIL         = 10, //!< energy level change fail
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_ENERGY_TRANS               = 11, //!< energy transfer success
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_ENERGY_TRANS_FAIL         = 12, //!< energy transfer fail
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_FULL_THROTTLE              = 13, //!< set full throttle
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_ZERO_THROTTLE              = 14, //!< set zero throttle
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_THROTTLE_UP                = 15, //!< set 1/3 or 2/3 throttle (up)
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_THROTTLE_DOWN              = 16, //!< set 1/3 or 2/3 throttle (down)
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  4), // SND_DOCK_APPROACH              = 17, //!< dock approach retros
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  4), // SND_DOCK_ATTACH                = 18, //!< dock attach
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  4), // SND_DOCK_DETACH                = 19, //!< dock detach
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  4), // SND_DOCK_DEPART                = 20, //!< dock depart retros
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  3), // SND_ABURN_ENGAGE               = 21, //!< afterburner engage
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  3), // SND_ABURN_LOOP                 = 22, //!< afterburner burn sound (looped)
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  3), // SND_VAPORIZED                  = 23, //!< Destroyed by a beam (vaporized)
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  2), // SND_ABURN_FAIL                 = 24, //!< afterburner fail (no fuel when aburn pressed)
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  4), // SND_HEATLOCK_WARN              = 25, //!< heat-seeker launch warning
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_OUT_OF_MISSLES             = 26, //!< tried to fire a missle when none are left
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_OUT_OF_WEAPON_ENERGY       = 27, //!< tried to fire lasers when not enough energy left
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_TARGET_FAIL                = 28, //!< target fail sound (i.e. press targeting key, but nothing happens)
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_SQUADMSGING_ON             = 29, //!< squadmate message menu appears
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_SQUADMSGING_OFF            = 30, //!< squadmate message menu disappears
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  5), // SND_DEBRIS                     = 31, //!< debris sound (persistant, looping)
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  2), // SND_SUBSYS_DIE_1               = 32, //!< subsystem gets destroyed on player ship
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  1), // SND_MISSILE_START_LOAD         = 33, //!< missle start load (during rearm/repair)
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  1), // SND_MISSILE_LOAD               = 34, //!< missle load (during rearm/repair)
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  1), // SND_SHIP_REPAIR                = 35, //!< ship is being repaired (during rearm/repair)
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  3), // SND_PLAYER_HIT_LASER           = 36, //!< player ship is hit by laser fire
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  3), // SND_PLAYER_HIT_MISSILE         = 37, //!< player ship is hit by missile
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_CMEASURE_CYCLE             = 38, //!< countermeasure cycle
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  2), // SND_SHIELD_HIT                 = 39, //!< shield hit
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  2), // SND_SHIELD_HIT_YOU             = 40, //!< player shield is hit
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_GAME_MOUSE_CLICK           = 41, //!< mouse click
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  2), // SND_ASPECTLOCK_WARN            = 42, //!< aspect launch warning
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_SHIELD_XFER_OK             = 43, //!< shield quadrant transfer successful
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  5), // SND_ENGINE_WASH                = 44, //!< Engine wash (looped)
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  2), // SND_WARP_IN                    = 45, //!< warp hole opening up for arriving
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  2), // SND_WARP_OUT                   = 46, //!< warp hole opening up for departing (Same as warp in for now)
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_PLAYER_WARP_FAIL           = 47, //!< player warp has failed
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_STATIC                     = 48, //!< hud gauge static
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  3), // SND_SHIP_EXPLODE_2             = 49, //!< ship explosion 2
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_PLAYER_WARP_OUT            = 50, //!< ship is warping out in 3rd person
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  5), // SND_SHIP_SHIP_HEAVY            = 51, //!< heavy ship-ship collide sound
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  5), // SND_SHIP_SHIP_LIGHT            = 52, //!< light ship-ship collide sound
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  5), // SND_SHIP_SHIP_SHIELD           = 53, //!< shield ship-ship collide overlay sound
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_THREAT_FLASH               = 54, //!< missile threat indicator flashes
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  3), // SND_PROXIMITY_WARNING          = 55, //!< proximity warning (heat seeker)
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  3), // SND_PROXIMITY_ASPECT_WARNING   = 56, //!< proximity warning (aspect)
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  2), // SND_DIRECTIVE_COMPLETE         = 57, //!< directive complete
	EnhancedSoundData(SND_ENHANCED_PRIORITY_MEDIUM_HIGH,  3), // SND_SUBSYS_EXPLODE             = 58, //!< other ship subsystem destroyed
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  2), // SND_CAPSHIP_EXPLODE            = 59, //!< captial ship explosion
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_CAPSHIP_SUBSYS_EXPLODE     = 60, //!< captial ship subsystem destroyed
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  2), // SND_LARGESHIP_WARPOUT          = 61, //!< large ship warps out
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  5), // SND_ASTEROID_EXPLODE_LARGE     = 62, //!< large asteroid blows up
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  5), // SND_ASTEROID_EXPLODE_SMALL     = 63, //!< small asteroid blows up
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  2), // SND_CUE_VOICE                  = 64, //!< sound to indicate voice is about to start
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  2), // SND_END_VOICE                  = 65, //!< sound to indicate voice has ended
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  2), // SND_CARGO_SCAN                 = 66, //!< cargo scanning (looped)
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  5), // SND_WEAPON_FLYBY               = 67, //!< weapon flyby sound
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  5), // SND_ASTEROID                   = 68, //!< asteroid sound (persistant, looped)
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  2), // SND_CAPITAL_WARP_IN            = 69, //!< capital warp hole opening
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  2), // SND_CAPITAL_WARP_OUT           = 70, //!< capital warp hole closing
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  5), // SND_ENGINE_LOOP_LARGE          = 71, //!< LARGE engine ambient
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_SUBSPACE_LEFT_CHANNEL      = 72, //!< subspace ambient sound (left channel) (looped)
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_SUBSPACE_RIGHT_CHANNEL     = 73, //!< subspace ambient sound (right channel) (looped)
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  4), // SND_MISSILE_EVADED_POPUP       = 74, //!< "evaded" HUD popup
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  4), // SND_ENGINE_LOOP_HUGE           = 75, //!< HUGE engine ambient
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_LIGHT_LASER_FIRE           = 76, //!< SD-4 Sidearm laser fired
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_LIGHT_LASER_IMPACT         = 77, //!< DR-2 Scalpel fired
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_HVY_LASER_FIRE             = 78, //!< Flail II fired
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_HVY_LASER_IMPACT           = 79, //!< Prometheus R laser fired
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_MASSDRV_FIRED              = 80, //!< Prometheus S laser fired
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_MASSDRV_IMPACT             = 81, //!< GTW-66 Newton Cannon fired
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_FLAIL_FIRED                = 82, //!< UD-8 Kayser Laser fired
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_FLAIL_IMPACT               = 83, //!< GTW-19 Circe laser fired
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_NEUTRON_FLUX_FIRED         = 84, //!< GTW-83 Lich laser fired
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_NEUTRON_FLUX_IMPACT        = 85, //!< Laser impact
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_DEBUG_LASER_FIRED          = 86, //!< Subach-HLV Vasudan laser
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_ROCKEYE_FIRED              = 87, //!< rockeye missile launch
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_MISSILE_IMPACT1            = 88, //!< missile impact 1
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_MAG_MISSILE_LAUNCH         = 89, //!< mag pulse missile launch
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_FURY_MISSILE_LAUNCH        = 90, //!< fury missile launch
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_SHRIKE_MISSILE_LAUNCH      = 91, //!< shrike missile launch
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_ANGEL_MISSILE_LAUNCH       = 92, //!< angel fire missile launch
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_CLUSTER_MISSILE_LAUNCH     = 93, //!< cluster bomb launch
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_CLUSTERB_MISSILE_LAUNCH    = 94, //!< cluster baby bomb launch
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_STILETTO_MISSILE_LAUNCH    = 95, //!< stiletto bomb launch
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_TSUNAMI_MISSILE_LAUNCH     = 96, //!< tsunami bomb launch
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_HARBINGER_MISSILE_LAUNCH   = 97, //!< harbinger bomb launch
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_MEGAWOKKA_MISSILE_LAUNCH   = 98, //!< mega wokka launch
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_CMEASURE1_LAUNCH           = 99, //!< countermeasure 1 launch
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_SHIVAN_LIGHT_LASER_FIRE    = 100,//!< Shivan light laser
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_SHOCKWAVE_EXPLODE          = 101,//!< shockwave ignition
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_SWARM_MISSILE_LAUNCH       = 102,//!< swarm missile sound
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_UNDEFINED_103              = 103,//!< Shivan heavy laser
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  3), // SND_UNDEFINED_104              = 104,//!< Vasudan SuperCap engine
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  5), // SND_UNDEFINED_105              = 105,//!< Shivan SuperCap engine
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  5), // SND_UNDEFINED_106              = 106,//!< Terran SuperCap engine
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_UNDEFINED_107              = 107,//!< Vasudan light laser fired
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_UNDEFINED_108              = 108,//!< Shivan heavy laser
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  2), // SND_SHOCKWAVE_IMPACT           = 109,//!< shockwave impact
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_UNDEFINED_110              = 110,//!< TERRAN TURRET 1
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_UNDEFINED_111              = 111,//!< TERRAN TURRET 2
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_UNDEFINED_112              = 112,//!< VASUDAN TURRET 1
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_UNDEFINED_113              = 113,//!< VASUDAN TURRET 2
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_UNDEFINED_114              = 114,//!< SHIVAN TURRET 1
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_TARG_LASER_LOOP            = 115,//!< targeting laser loop sound
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  8), // SND_FLAK_FIRE                  = 116,//!< Flak Gun Launch
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_SHIELD_BREAKER             = 117,//!< Flak Gun Impact
	EnhancedSoundData( SND_ENHANCED_PRIORITY_MEDIUM_LOW,  8), // SND_EMP_MISSILE                = 118,//!< EMP Missle
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  5), // SND_AUTOCANNON_LOOP            = 119,//!< Escape Pod Drone
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  5), // SND_AUTOCANNON_SHOT            = 120,//!< Beam Hit 1
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  4), // SND_BEAM_LOOP                  = 121,//!< beam loop
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  4), // SND_BEAM_UP                    = 122,//!< beam power up
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  4), // SND_BEAM_DOWN                  = 123,//!< beam power down
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  4), // SND_BEAM_SHOT                  = 124,//!< Beam shot 1
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  4), // SND_BEAM_VAPORIZE              = 125,//!< Beam shot 2
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  4), // SND_TERRAN_FIGHTER_ENG         = 126,//!< Terran fighter engine
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  4), // SND_TERRAN_BOMBER_ENG          = 127,//!< Terran bomber engine
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  4), // SND_TERRAN_CAPITAL_ENG         = 128,//!< Terran cruiser engine
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  4), // SND_SPECIESB_FIGHTER_ENG       = 129,//!< Vasudan fighter engine
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  4), // SND_SPECIESB_BOMBER_ENG        = 130,//!< Vasudan bomber engine
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  4), // SND_SPECIESB_CAPITAL_ENG       = 131,//!< Vasudan cruiser engine
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  4), // SND_SHIVAN_FIGHTER_ENG         = 132,//!< Shivan fighter engine
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  4), // SND_SHIVAN_BOMBER_ENG          = 133,//!< Shivan bomber engine
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  4), // SND_SHIVAN_CAPITAL_ENG         = 134,//!< Shivan cruiser engine
	EnhancedSoundData(SND_ENHANCED_PRIORITY_MEDIUM_HIGH,  3), // SND_REPAIR_SHIP_ENG            = 135,//!< Repair ship beacon/engine sound
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  4), // SND_UNDEFINED_136              = 136,//!< Terran capital engine
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  4), // SND_UNDEFINED_137              = 137,//!< Vasudan capital engine
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  4), // SND_UNDEFINED_138              = 138,//!< Shivan capital engine
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  3), // SND_DEBRIS_ARC_01              = 139,//!< 0.10 second spark sound effect
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  3), // SND_DEBRIS_ARC_02              = 140,//!< 0.25 second spark sound effect
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  3), // SND_DEBRIS_ARC_03              = 141,//!< 0.50 second spark sound effect
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  3), // SND_DEBRIS_ARC_04              = 142,//!< 0.75 second spark sound effect
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  3), // SND_DEBRIS_ARC_05              = 143,//!< 1.00 second spark sound effect
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_144              = 144,//!< LTerSlash beam loop
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_145              = 145,//!< TerSlash	beam loop
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_146              = 146,//!< SGreen 	beam loop
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_147              = 147,//!< BGreen	beem loop
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_148              = 148,//!< BFGreen	been loop
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_149              = 149,//!< Antifighter 	beam loop
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_150              = 150,//!< 1 sec		warm up
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_151              = 151,//!< 1.5 sec 	warm up
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_152              = 152,//!< 2.5 sec 	warm up
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_153              = 153,//!< 3 sec 	warm up
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_154              = 154,//!< 3.5 sec 	warm up
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_155              = 155,//!< 5 sec 	warm up
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_156              = 156,//!< LTerSlash	warm down
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_157              = 157,//!< TerSlash	warm down
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_158              = 158,//!< SGreen	warm down
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_159              = 159,//!< BGreen	warm down
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_160              = 160,//!< BFGreen	warm down
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_161              = 161,//!< T_AntiFtr	warm down
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  1), // SND_COPILOT                    = 162,//!< copilot (SCP)
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  1), // SND_UNDEFINED_163              = 163,//!< (Empty in Retail)
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  1), // SND_UNDEFINED_164              = 164,//!< (Empty in Retail)
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  1), // SND_UNDEFINED_165              = 165,//!< (Empty in Retail)
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  1), // SND_UNDEFINED_166              = 166,//!< (Empty in Retail)
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  1), // SND_UNDEFINED_167              = 167,//!< (Empty in Retail)
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  1), // SND_UNDEFINED_168              = 168,//!< (Empty in Retail)
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  1), // SND_UNDEFINED_169              = 169,//!< (Empty in Retail)
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  1), // SND_UNDEFINED_170              = 170,//!< (Empty in Retail)
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  1), // SND_UNDEFINED_171              = 171,//!< (Empty in Retail)
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  1), // SND_UNDEFINED_172              = 172,//!< (Empty in Retail)
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_SUPERNOVA_1                = 173,//!< SuperNova (distant)
	EnhancedSoundData(  SND_ENHANCED_PRIORITY_MUST_PLAY,  1), // SND_SUPERNOVA_2                = 174,//!< SuperNova (shockwave)
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  3), // SND_UNDEFINED_175              = 175,//!< Shivan large engine
	EnhancedSoundData(     SND_ENHANCED_PRIORITY_MEDIUM,  3), // SND_UNDEFINED_176              = 176,//!< Shivan large engine
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_177              = 177,//!< SRed 		beam loop
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_178              = 178,//!< LRed		beam loop
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  3), // SND_UNDEFINED_179              = 179,//!< Antifighter	beam loop
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  4), // SND_LIGHTNING_1                = 180,//!< Thunder 1 sound in neblua
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  4), // SND_LIGHTNING_2                = 181,//!< Thunder 2 sound in neblua
	EnhancedSoundData(SND_ENHANCED_PRIORITY_MEDIUM_HIGH,  3), // SND_UNDEFINED_182              = 182,//!< 1 sec 	warm up
	EnhancedSoundData(SND_ENHANCED_PRIORITY_MEDIUM_HIGH,  3), // SND_UNDEFINED_183              = 183,//!< 1.5 sec 	warm up
	EnhancedSoundData(SND_ENHANCED_PRIORITY_MEDIUM_HIGH,  3), // SND_UNDEFINED_184              = 184,//!< 3 sec 	warm up
	EnhancedSoundData(SND_ENHANCED_PRIORITY_MEDIUM_HIGH,  2), // SND_UNDEFINED_185              = 185,//!< Shivan Commnode
	EnhancedSoundData(       SND_ENHANCED_PRIORITY_HIGH,  1), // SND_UNDEFINED_186              = 186,//!< Volition PirateShip
	EnhancedSoundData(SND_ENHANCED_PRIORITY_MEDIUM_HIGH,  3), // SND_UNDEFINED_187              = 187,//!< SRed 		warm down
	EnhancedSoundData(SND_ENHANCED_PRIORITY_MEDIUM_HIGH,  3), // SND_UNDEFINED_188              = 188,//!< LRed 		warm down
	EnhancedSoundData(SND_ENHANCED_PRIORITY_MEDIUM_HIGH,  3), // SND_UNDEFINED_189              = 189,//!< AntiFtr	warm down
	EnhancedSoundData(SND_ENHANCED_PRIORITY_MEDIUM_HIGH,  2), // SND_UNDEFINED_190              = 190,//!< Instellation 1
	EnhancedSoundData(SND_ENHANCED_PRIORITY_MEDIUM_HIGH,  2)  // SND_UNDEFINED_191              = 191,//!< Instellation 2
};

static const EnhancedSoundData default_enhanced_sound_data(SND_ENHANCED_PRIORITY_MEDIUM_HIGH, 1);


/*
 * Update any uninitialized EnhancedSoundData in Snds
  * with hardcoded defaults for retail.
 */
void gamesnd_add_retail_default_enhanced_sound_data()
{
	int i = 0;

	for (SCP_vector<game_snd>::iterator it = Snds.begin(), end = Snds.end(); it != end; ++it, ++i)
	{
		if (it->enhanced_sound_data.priority== SND_ENHANCED_PRIORITY_INVALID)
		{
			if (i < NUM_RETAIL_GAMEPLAY_SOUNDS)
			{
				it->enhanced_sound_data.priority= Default_sound_priorities[i].priority;
			}
			else
			{
				it->enhanced_sound_data.priority= default_enhanced_sound_data.priority;
			}
		}

		if (it->enhanced_sound_data.limit < 1)
		{
			if (i < NUM_RETAIL_GAMEPLAY_SOUNDS)
			{
				it->enhanced_sound_data.limit = Default_sound_priorities[i].limit;
			}
			else
			{
				it->enhanced_sound_data.limit= default_enhanced_sound_data.limit;
			}
		}
	}
}

void gamesnd_play_iface(interface_snd_id n)
{
	if (Snds_iface_handle[n.value()].isValid())
		snd_stop(Snds_iface_handle[n.value()]);

	Snds_iface_handle[n.value()] = snd_play(gamesnd_get_interface_sound(n));
}

/**
 * Function to search for a given game_snd with the specified name
 * in the passed vector
 *
 * @param name Name to search for
 * @param sounds Vector to seach in
 *
 */
int gamesnd_lookup_name(const char* name, const SCP_vector<game_snd>& sounds)
{
	// if we get passed -1, don't bother trying to look it up.
	if (name == NULL || *name == 0 || !strcmp(name, "-1"))
	{
		return -1;
	}

	Assert( sounds.size() <= INT_MAX );

	int i = 0;

	for(SCP_vector<game_snd>::const_iterator snd = sounds.begin(); snd != sounds.end(); ++snd)
	{
		if (!stricmp(snd->name.c_str(), name))
		{
			return i;
		}
		i++;
	}

	return -1;
}

// WMC - now ignores file extension.
gamesnd_id gamesnd_get_by_name(const char* name)
{
	Assert( Snds.size() <= INT_MAX );

	// empty name is not valid!
	if (name == nullptr || name[0] == '\0')
		return gamesnd_id(-1);

	int index = gamesnd_lookup_name(name, Snds);

	if (index < 0)
	{
		int i = 0;
		for (auto& Snd : Snds) {
			if (Snd.sound_entries.size() != 1) {
				// Ignore game sounds with more than one sound entry
				continue;
			}

			auto& entry = Snd.sound_entries.front();
			char *p = strrchr( entry.filename, '.' );
			if(p == NULL)
			{
				if(!stricmp(entry.filename, name))
				{
					index = i;
					break;
				}
			}
			else if(!strnicmp(entry.filename, name, p - entry.filename))
			{
				index = i;
				break;
			}

			i++;
		}
	}

	return gamesnd_id(index);
}

interface_snd_id gamesnd_get_by_iface_name(const char* name)
{
	Assert( Snds_iface.size() <= INT_MAX );
	Assert( Snds_iface.size() == Snds_iface_handle.size() );

	auto index = gamesnd_lookup_name(name, Snds_iface);

	if (index < 0)
	{
		int i = 0;
		for (auto& snd : Snds_iface) {
			if (snd.sound_entries.size() != 1) {
				// Ignore game sounds with more than one sound entry
				continue;
			}

			auto& entry = snd.sound_entries.front();
			char *p = strrchr( entry.filename, '.' );
			if(p == NULL)
			{
				if(!stricmp(entry.filename, name))
				{
					index = i;
					break;
				}
			}
			else if(!strnicmp(entry.filename, name, p - entry.filename))
			{
				index = i;
				break;
			}

			i++;
		}
	}

	return interface_snd_id(index);
}

gamesnd_id gamesnd_get_by_tbl_index(int index)
{
	char temp[11];
	sprintf(temp, "%i", index);

	auto idx = gamesnd_lookup_name(temp, Snds);

	if (idx < 0) {
		return gamesnd_id();
	} else {
		return gamesnd_id(idx);
	}
}

interface_snd_id gamesnd_get_by_iface_tbl_index(int index)
{
	Assert( Snds_iface.size() == Snds_iface_handle.size() );

	char temp[11];
	sprintf(temp, "%i", index);

	auto idx = gamesnd_lookup_name(temp, Snds_iface);

	if (idx < 0) {
		return interface_snd_id();
	} else {
		return interface_snd_id(idx);
	}
}

/**
 * Parse a sound. When using this function for a table entry,
 * required_string and optional_string aren't needed, as this function deals with
 * that as its tag parameter, just make sure that the destination sound index can
 * handle -1 if things don't work out.
 *
 * @param tag Tag
 * @param idx_dest Sound index destination
 * @param flags See the parse_sound_flags enum
 *
 */
bool parse_game_sound(const char* tag, gamesnd_id* idx_dest)
{
	if (optional_string(tag))
	{
		*idx_dest = parse_game_sound_inline();
		return true;
	}

	return false;
}

/**
 * @brief Parses a game sound that should appear at the current parsing location
 * @return The game sound id or invalid id when parsing fails
 */
gamesnd_id parse_game_sound_inline()
{
	SCP_string buf;
	stuff_string(buf, F_NAME);

	auto id = gamesnd_get_by_name(buf.c_str());

	// The special case "-1" is needed to silence warnings where sounds are intentionally removed
	if (!id.isValid() && buf != "-1") {
		error_display(0, "Could not find game sound with name '%s'!", buf.c_str());
	}

	return id;
}

/**
 * Parse a sound. When using this function for a table entry,
 * required_string and optional_string aren't needed, as this function deals with
 * that as its tag parameter, just make sure that the destination sound index can
 * handle -1 if things don't work out.
 *
 * @param tag Tag
 * @param idx_dest Sound index destination
 * @param flags See the parse_sound_flags enum
 *
 */
void parse_iface_sound(const char* tag, interface_snd_id* idx_dest) {
	Assert( Snds_iface.size() == Snds_iface_handle.size() );

	if(optional_string(tag))
	{
		SCP_string buf;
		stuff_string(buf, F_NAME);

		*idx_dest = gamesnd_get_by_iface_name(buf.c_str());

		// The special case "-1" is needed to silence warnings where sounds are intentionally removed
		if (!idx_dest->isValid() && buf != "-1") {
			error_display(0, "Could not find interface sound with name '%s'!", buf.c_str());
		}
	}
}

/**
 * CommanderDJ - Parse a list of sounds. When using this function for a table entry,
 * required_string and optional_string aren't needed, as this function deals with
 * that as its tag parameter, just make sure that the destination sound index(es) can
 * handle -1 if things don't work out.
 *
 * @param destination Vector where sound indexes are to be stored
 * @param tag Tag
 * @param object_name Name of object being parsed
 * @param flags See the parse_sound_flags enum
 *
 */
void parse_iface_sound_list(const char* tag, SCP_vector<interface_snd_id>& destination, const char* object_name, bool scp_list)
{
	if(optional_string(tag))
	{
		int check=0;

		//if we're using the old format, parse the first entry separately
		if(!scp_list)
		{
			stuff_int(&check);
		}

		//now read the rest of the entries on the line
		for(size_t i=0; !check_for_eoln(); i++)
		{
			SCP_string buf;
			stuff_string_white(buf);

			//we do this conditionally to avoid adding needless entries when reparsing
			if(destination.size() <= i)
			{
				destination.push_back(interface_snd_id());
			}

			destination[i] = gamesnd_get_by_iface_name(buf.c_str());

			if (!destination[i].isValid()) {
				error_display(0, "Could not find interface sound with name '%s'!", buf.c_str());
			}
		}

		//if we're using the old format, double check the size)
		if(!scp_list && (destination.size() != (unsigned)check))
		{
			mprintf(("%s in '%s' has " SIZE_T_ARG " entries. This does not match entered size of %i.\n", tag, object_name, destination.size(), check));
		}
	}
}

/**
 * Load in sounds that we expect will get played
 *
 * The method currently used is to load all those sounds that have the hardware flag
 * set.  This works well since we don't want to try and load hardware sounds in on the
 * fly (too slow).
 */
void gamesnd_preload_common_sounds()
{
	if ( !Sound_enabled )
		return;

	Assert( Snds.size() <= INT_MAX );
	for (auto& gs: Snds) {
		if ( gs.preload ) {
			for (auto& entry : gs.sound_entries) {
				if ( entry.filename[0] != 0 && strnicmp(entry.filename, NOX("none.wav"), 4) != 0 ) {
					game_busy( NOX("** preloading common game sounds **") );	// Animate loading cursor... does nothing if loading screen not active.
					entry.id = snd_load(&entry, &gs.flags);
				}
			}
		}
	}
}

/**
 * Load the ingame sounds into memory
 */
void gamesnd_load_gameplay_sounds()
{
	if ( !Sound_enabled )
		return;

	Assert( Snds.size() <= INT_MAX );
	for (auto& gs: Snds) {
		if ( !gs.preload ) { // don't try to load anything that's already preloaded
			for (auto& entry : gs.sound_entries) {
				if (entry.filename[0] != 0 && strnicmp(entry.filename, NOX("none.wav"), 4) != 0) {
					game_busy(NOX("** preloading gameplay sounds **"));        // Animate loading cursor... does nothing if loading screen not active.
					entry.id = snd_load(&entry, &gs.flags);
				}
			}
		}
	}
}

/**
 * Unload the ingame sounds from memory
 */
void gamesnd_unload_gameplay_sounds()
{
	Assert( Snds.size() <= INT_MAX );
	for (auto& gs: Snds) {
		for (auto& entry : gs.sound_entries) {
			if (entry.id.isValid()) {
				snd_unload(entry.id);
				entry.id = sound_load_id::invalid();
			}
		}
	}
}

/**
 * Load the interface sounds into memory
 */
void gamesnd_load_interface_sounds()
{
	if ( !Sound_enabled )
		return;

	Assert( Snds_iface.size() < INT_MAX );
	for (auto& gs: Snds) {
		for (auto& entry : gs.sound_entries) {
			if ( entry.filename[0] != 0 && strnicmp(entry.filename, NOX("none.wav"), 4) != 0 ) {
				entry.id = snd_load(&entry, &gs.flags);
			}
		}
	}
}

/**
 * Unload the interface sounds from memory
 */
void gamesnd_unload_interface_sounds()
{
	Assert( Snds_iface.size() < INT_MAX );
	for (SCP_vector<game_snd>::iterator si = Snds_iface.begin(); si != Snds_iface.end(); ++si) {
		for (auto& entry : si->sound_entries) {
			if (entry.id.isValid()) {
				snd_unload( entry.id );
				entry.id     = sound_load_id::invalid();
				entry.id_sig = -1;
			}
		}
	}
}

void parse_gamesnd_old(game_snd* gs)
{
	int is_3d;
	int temp;

	// An old sound is just a single entry sound set
	gs->sound_entries.resize(1);
	auto& entry = gs->sound_entries.back();

	// Default pitch is 1.0. This is set here in case we don't have a valid file name
	gs->pitch_range = util::UniformFloatRange(1.0f);

	stuff_string(entry.filename, F_NAME, MAX_FILENAME_LEN, ",");

	// since we have a new filename, first assume it's valid
	gs->flags &= ~GAME_SND_NOT_VALID;

	if (!stricmp(entry.filename, NOX("empty")) || !stricmp(entry.filename, NOX("none")))
	{
		entry.filename[0] = 0;
		advance_to_eoln(nullptr);
		gs->flags |= GAME_SND_NOT_VALID;
		return;
	}
	Mp++;

	stuff_int(&temp);

	if (temp > 0)
	{
		gs->preload = true;
	}

	float default_volume;
	stuff_float(&default_volume);
	gs->volume_range = util::UniformFloatRange(default_volume);

	stuff_int(&is_3d);

	if (is_3d)
	{
		gs->flags |= GAME_SND_USE_DS3D;
		stuff_int(&gs->min);
		stuff_int(&gs->max);
	}
	else
	{
		gs->min = 0;
		gs->max = 0;

		// silly retail, not abiding by its own format...
		if (!stricmp(entry.filename, "l_hit.wav") || !stricmp(entry.filename, "m_hit.wav"))
		{
			int temp_min, temp_max;

			if (stuff_int_optional(&temp_min) == 2)
			{
				if (stuff_int_optional(&temp_max) == 2)
				{
					nprintf(("Sound","Dutifully converting retail sound %s, '%s' to a 3D sound...\n", gs->name.c_str(), entry.filename));
					is_3d = 1;

					gs->flags |= GAME_SND_USE_DS3D;
					gs->min = temp_min;
					gs->max = temp_max;
				}
			}
		}
	}

	// check for extra values per Mantis #2408
	if (stuff_int_optional(&temp) == 2)
	{
		Warning(LOCATION, "Unexpected extra value %d found for sound '%s' (filename '%s')!  Check the format of the sounds.tbl (or .tbm) entry.", temp, gs->name.c_str(), entry.filename);
	}

	advance_to_eoln(NULL);
}

EnhancedSoundPriority convert_to_enhanced_priority(const char * priority_str)
{
	Assertion(priority_str != NULL, "convert_to_enhanced_priority given null priority_str!");

	if (!stricmp(priority_str, "Must Play"))
	{
		return SND_ENHANCED_PRIORITY_MUST_PLAY;
	}
	else if (!stricmp(priority_str, "High"))
	{
		return SND_ENHANCED_PRIORITY_HIGH;
	}
	else if (!stricmp(priority_str, "Medium-High"))
	{
		return SND_ENHANCED_PRIORITY_MEDIUM_HIGH;
	}
	else if (!stricmp(priority_str, "Medium"))
	{
		return SND_ENHANCED_PRIORITY_MEDIUM;
	}
	else if (!stricmp(priority_str, "Medium-Low"))
	{
		return SND_ENHANCED_PRIORITY_MEDIUM_LOW;
	}
	else if (!stricmp(priority_str, "Low"))
	{
		return SND_ENHANCED_PRIORITY_LOW;
	}
	else
	{
		error_display(1, "Unknown enhanced sound priority: %s\n", priority_str);
		return SND_ENHANCED_PRIORITY_INVALID;
	}
}

bool required_string_no_create(const char* token, bool no_create)
{
    if (no_create)
    {
        return optional_string(token) == 1;
    }

    required_string(token);
    return true;
}

static GameSoundCycleType parse_cycle_type() {
	if (optional_string("Sequential")) {
		return GameSoundCycleType::SequentialCycle;
	} else if (optional_string("Random")) {
		return GameSoundCycleType::RandomCycle;
	} else {
		error_display(0, "Failed to parse sound cycle type. Expected 'sequential' or 'random'. Got [%.32s]", next_tokens());
		// Ignore everything until the end of the line. That should hopefully skip the bad token.
		advance_to_eoln(nullptr);
		return GameSoundCycleType::SequentialCycle;
	}
}

void parse_gamesnd_soundset(game_snd* gs, bool no_create) {
	// If this gets called then we just saw "+Entry:" so we can begin processing the first entry immediately

	do {
		// For now there is no way to change an existing entry so ever +Entry statement creates a new sound entry
		gs->sound_entries.resize(gs->sound_entries.size() + 1);
		auto& entry = gs->sound_entries.back();

		stuff_string(entry.filename, F_NAME, MAX_FILENAME_LEN);
	} while (optional_string("+Entry:"));

	if (required_string_no_create("+Cycle type:", no_create)) {
		gs->cycle_type = parse_cycle_type();
	}

	if (required_string_no_create("+Preload:", no_create))
	{
		stuff_boolean(&gs->preload);
	}

	if (required_string_no_create("+Volume:", no_create))
	{
		gs->volume_range = util::parseUniformRange(0.0f, 1.0f);
	}

	if (optional_string("+3D Sound:"))
	{
		gs->flags |= GAME_SND_USE_DS3D;
		required_string("+Attenuation start:");

		stuff_int(&gs->min);

		required_string("+Attenuation end:");

		stuff_int(&gs->max);
	}
	else
	{
		gs->min = 0;
		gs->max = 0;
	}

	// jg18 - enhanced sound parameters
	if (optional_string("+Priority:"))
	{
		SCP_string priority_string;
		stuff_string(priority_string, F_NAME);
		EnhancedSoundPriority priority = convert_to_enhanced_priority(priority_string.c_str());
		if (priority != SND_ENHANCED_PRIORITY_INVALID)
		{
			gs->enhanced_sound_data.priority= priority;
		}
		// else case not needed since conversion function displays message on error
	}

	if (optional_string("+Limit:"))
	{
		int temp_limit;
		stuff_int(&temp_limit);

		if (temp_limit > 0)
		{
			gs->enhanced_sound_data.limit = (unsigned int)temp_limit;
		}
		else
		{
			error_display(1, "Invalid enhanced sound limit: %d\n", temp_limit);
		}
	}

	if (optional_string("+Pitch:")) {
		gs->pitch_range = util::parseUniformRange(0.0001f);
	} else if (!no_create) {
		// Default pitch is 1.0
		gs->pitch_range = util::UniformFloatRange(1.0f);
	}
}

void parse_gamesnd_new(game_snd* gs, bool no_create)
{
	game_snd_entry* entry = nullptr;
	if (!no_create) {
		gs->sound_entries.resize(1);
		entry = &gs->sound_entries.back();
	} else {
		if (gs->sound_entries.size() != 1) {
			error_display(1, "The SCP syntax cannot be used to modify an existing sound that has more than one entry! Use the soundset syntax for adding new entries.");
		}
		entry = &gs->sound_entries.back();
	}

	// Default pitch is 1.0. This is set here in case we don't have a valid file name
	gs->pitch_range = util::UniformFloatRange(1.0f);

	char name[MAX_FILENAME_LEN];
	// New extended format found
	stuff_string(name, F_NAME, MAX_FILENAME_LEN);

	// since we have a new filename, first assume it's valid
	gs->flags &= ~GAME_SND_NOT_VALID;

	if (!stricmp(name, NOX("empty")) || !stricmp(name, NOX("none")))
	{
		entry->filename[0] = 0;
		gs->flags |= GAME_SND_NOT_VALID;
		return;
	}

	// If the name _doesn't_ match <same> put it into gs->filename;
	if (stricmp(name, "<same>") != 0)
	{
		strcpy_s(entry->filename, name);
	}
	else if (!no_create)
	{
		// Throw an error if <same> was specified but we are creating a new entry
		error_display(1, "'<same>' is only allowed if +nocreate was specified!");
		return;
	}

	if (required_string_no_create("+Preload:", no_create))
	{
		stuff_boolean(&gs->preload);
	}

	if (required_string_no_create("+Volume:", no_create))
	{
		float default_volume;
		stuff_float(&default_volume);
		gs->volume_range = util::UniformFloatRange(default_volume);
	}

	if (optional_string("+3D Sound:"))
	{
		gs->flags |= GAME_SND_USE_DS3D;
		required_string("+Attenuation start:");

		stuff_int(&gs->min);

		required_string("+Attenuation end:");

		stuff_int(&gs->max);
	}
	else
	{
		gs->min = 0;
		gs->max = 0;
	}

	// jg18 - enhanced sound parameters
	if (optional_string("+Priority:"))
	{
		SCP_string priority_string;
		stuff_string(priority_string, F_NAME);
		EnhancedSoundPriority priority = convert_to_enhanced_priority(priority_string.c_str());
		if (priority != SND_ENHANCED_PRIORITY_INVALID)
		{
			gs->enhanced_sound_data.priority= priority;
		}
		// else case not needed since conversion function displays message on error
	}

	if (optional_string("+Limit:"))
	{
		int temp_limit;
		stuff_int(&temp_limit);

		if (temp_limit > 0)
		{
			gs->enhanced_sound_data.limit = (unsigned int)temp_limit;
		}
		else
		{
			error_display(1, "Invalid enhanced sound limit: %d\n", temp_limit);
		}
	}
}

void gamesnd_parse_entry(game_snd *gs, bool no_create, SCP_vector<game_snd> *lookupVector)
{
	SCP_string name;

	stuff_string(name, F_NAME, "\t \n");

	if (!no_create)
	{
		if (lookupVector != NULL)
		{
			if (gamesnd_lookup_name(name.c_str(), *lookupVector) >= 0)
			{
				Warning(LOCATION, "Duplicate sound name \"%s\" found!", name.c_str());
			}
		}

		gs->name = name;
	}
	else
	{
		int vectorIndex = gamesnd_lookup_name(name.c_str(), *lookupVector);

		if (vectorIndex < 0)
		{
			Warning(LOCATION, "No existing sound entry with name \"%s\" found!", name.c_str());
			no_create = false;
			gs->name = name;
		}
		else
		{
			gs = &lookupVector->at(vectorIndex);
		}
	}

	if (optional_string("+Filename:"))
	{
		parse_gamesnd_new(gs, no_create);
	} else if (optional_string("+Entry:")) {
		parse_gamesnd_soundset(gs, no_create);
	}
	else
	{
		parse_gamesnd_old(gs);
	}
}

/**
 * Parse a sound effect entry by requiring the given tag at the beginning.
 *
 * @param gs The game_snd instance to fill in
 * @param tag The tag that's required before an entry
 * @param lookupVector If non-NULL used to look up @c +nocreate entries
 *
 * @return @c true when a new entry has been parsed and should be added to the list of known
 *			entries. @c false otherwise, for example in case of @c +nocreate
 */
bool gamesnd_parse_line(game_snd *gs, const char *tag, SCP_vector<game_snd> *lookupVector = NULL)
{
	Assertion(gs != NULL, "Invalid game_snd pointer passed to gamesnd_parse_line!");

	required_string(tag);

	bool no_create = false;

	if (lookupVector != NULL)
	{
		if(optional_string("+nocreate"))
		{
			no_create = true;
		}
	}

	gamesnd_parse_entry(gs, no_create, lookupVector);

	return !no_create;
}

void parse_sound_environments()
{
	char name[65] = { '\0' };
	char template_name[65] = { '\0' };
	EFXREVERBPROPERTIES *props;

	while (required_string_either("#Sound Environments End", "$Name:"))
	{
		required_string("$Name:");
		stuff_string(name, F_NAME, sizeof(name)-1);

		if (optional_string("$Template:"))
		{
			stuff_string(template_name, F_NAME, sizeof(template_name)-1);
		}
		else
		{
			template_name[0] = '\0';
		}

		ds_eax_get_prop(&props, name, template_name);

		if (optional_string("+Density:"))
		{
			stuff_float(&props->flDensity);
			CLAMP(props->flDensity, 0.0f, 1.0f);
		}

		if (optional_string("+Diffusion:"))
		{
			stuff_float(&props->flDiffusion);
			CLAMP(props->flDiffusion, 0.0f, 1.0f);
		}

		if (optional_string("+Gain:"))
		{
			stuff_float(&props->flGain);
			CLAMP(props->flGain, 0.0f, 1.0f);
		}

		if (optional_string("+Gain HF:"))
		{
			stuff_float(&props->flGainHF);
			CLAMP(props->flGainHF, 0.0f, 1.0f);
		}

		if (optional_string("+Gain LF:"))
		{
			stuff_float(&props->flGainLF);
			CLAMP(props->flGainLF, 0.0f, 1.0f);
		}

		if (optional_string("+Decay Time:"))
		{
			stuff_float(&props->flDecayTime);
			CLAMP(props->flDecayTime, 0.01f, 20.0f);
		}

		if (optional_string("+Decay HF Ratio:"))
		{
			stuff_float(&props->flDecayHFRatio);
			CLAMP(props->flDecayHFRatio, 0.1f, 20.0f);
		}

		if (optional_string("+Decay LF Ratio:"))
		{
			stuff_float(&props->flDecayLFRatio);
			CLAMP(props->flDecayLFRatio, 0.1f, 20.0f);
		}

		if (optional_string("+Reflections Gain:"))
		{
			stuff_float(&props->flReflectionsGain);
			CLAMP(props->flReflectionsGain, 0.0f, 3.16f);
		}

		if (optional_string("+Reflections Delay:"))
		{
			stuff_float(&props->flReflectionsDelay);
			CLAMP(props->flReflectionsDelay, 0.0f, 0.3f);
		}

		if (optional_string("+Reflections Pan:"))
		{
			stuff_float_list(props->flReflectionsPan, 3);
			CLAMP(props->flReflectionsPan[0], 0.0f, 1.0f);
			CLAMP(props->flReflectionsPan[1], 0.0f, 1.0f);
			CLAMP(props->flReflectionsPan[2], 0.0f, 1.0f);
		}

		if (optional_string("+Late Reverb Gain:"))
		{
			stuff_float(&props->flLateReverbGain);
			CLAMP(props->flLateReverbGain, 0.0f, 10.0f);
		}

		if (optional_string("+Late Reverb Delay:"))
		{
			stuff_float(&props->flLateReverbDelay);
			CLAMP(props->flLateReverbDelay, 0.0f, 0.1f);
		}

		if (optional_string("+Late Reverb Pan:"))
		{
			stuff_float_list(props->flLateReverbPan, 3);
			CLAMP(props->flLateReverbPan[0], 0.0f, 1.0f);
			CLAMP(props->flLateReverbPan[1], 0.0f, 1.0f);
			CLAMP(props->flLateReverbPan[2], 0.0f, 1.0f);
		}

		if (optional_string("+Echo Time:"))
		{
			stuff_float(&props->flEchoTime);
			CLAMP(props->flEchoTime, 0.075f, 0.25f);
		}

		if (optional_string("+Echo Depth:"))
		{
			stuff_float(&props->flEchoDepth);
			CLAMP(props->flEchoDepth, 0.0f, 1.0f);
		}

		if (optional_string("+Modulation Time:"))
		{
			stuff_float(&props->flModulationTime);
			CLAMP(props->flModulationTime, 0.004f, 4.0f);
		}

		if (optional_string("+Modulation Depth:"))
		{
			stuff_float(&props->flModulationDepth);
			CLAMP(props->flModulationDepth, 0.0f, 1.0f);
		}

		if (optional_string("+HF Reference:"))
		{
			stuff_float(&props->flHFReference);
			CLAMP(props->flHFReference, 1000.0f, 20000.0f);
		}

		if (optional_string("+LF Reference:"))
		{
			stuff_float(&props->flLFReference);
			CLAMP(props->flLFReference, 20.0f, 1000.0f);
		}

		if (optional_string("+Room Rolloff Factor:"))
		{
			stuff_float(&props->flRoomRolloffFactor);
			CLAMP(props->flRoomRolloffFactor, 0.0f, 10.0f);
		}

		if (optional_string("+Air Absorption Gain HF:"))
		{
			stuff_float(&props->flAirAbsorptionGainHF);
			CLAMP(props->flAirAbsorptionGainHF, 0.892f, 1.0f);
		}

		if (optional_string("+Decay HF Limit:"))
		{
			stuff_int(&props->iDecayHFLimit);
			CLAMP(props->iDecayHFLimit, 0, 1);
		}
	}

	required_string("#Sound Environments End");
}

void parse_sound_table(const char* filename)
{
	try
	{
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		// Parse the gameplay sounds section
		if (optional_string("#Game Sounds Start"))
		{
			while (!check_for_string("#Game Sounds End"))
			{
				game_snd tempSound;
				if (gamesnd_parse_line(&tempSound, "$Name:", &Snds))
				{
					Snds.push_back(game_snd(tempSound));
				}
			}

			required_string("#Game Sounds End");
		}

		// Parse the interface sounds section
		if (optional_string("#Interface Sounds Start"))
		{
			while (!check_for_string("#Interface Sounds End"))
			{
				game_snd tempSound;
				if (gamesnd_parse_line(&tempSound, "$Name:", &Snds_iface))
				{
					Snds_iface.push_back(game_snd(tempSound));
					Snds_iface_handle.push_back(sound_handle::invalid());
				}
			}

			required_string("#Interface Sounds End");
		}

		// parse flyby sound section
		if (optional_string("#Flyby Sounds Start"))
		{
			// try parsing sounds for each species
			// Note 1: instead of going through Species_info and requiring a sound for each species,
			// we now parse the token and extract the species name from it
			// Note 2: if a species doesn't have a flyby sound, the flyby code will just not play anything
			while (!check_for_string("#Flyby Sounds End"))
			{
				char species_name_tag[NAME_LENGTH + 2];
				stuff_string_until(species_name_tag, ":", NAME_LENGTH + 2);

				if (species_name_tag[0] != '$')
				{
					error_display(0, "Unexpected token tag %s", species_name_tag);
					advance_to_eoln(nullptr);
					continue;
				}

				int idx = species_info_lookup(&species_name_tag[1]);
				if (idx < 0)
				{
					mprintf(("Skipping flyby sound for unknown species %s\n", &species_name_tag[1]));
					advance_to_eoln(nullptr);
					continue;
				}

				// now we have a known species
				species_info *species = &Species_info[idx];
				strcat_s(species_name_tag, ":");	// put back the terminator to restore the entire tag

				// parse the two sounds for it
				gamesnd_parse_line(&species->snd_flyby_fighter, ":");				// since we stuffed most of the tag, the : is all that remains on this line
				gamesnd_parse_line(&species->snd_flyby_bomber, species_name_tag);	// for the subsequent line we use required_string on the whole tag
			}

			required_string("#Flyby Sounds End");
		}

		if (optional_string("#Sound Environments Start"))
		{
			parse_sound_environments();
		}
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}

	gamesnd_add_retail_default_enhanced_sound_data();
}

/**
 * Parse the sounds.tbl file, and load the specified sounds.
 */
void gamesnd_parse_soundstbl()
{
	parse_sound_table("sounds.tbl");

	parse_modular_table("*-snd.tbm", parse_sound_table);

	//Set any flyby sounds for species that use the borrowed feature
	for (size_t i = 0; i < Species_info.size(); i++) {
		if (Species_info[i].borrows_flyby_sounds_species >= 0) {
			int idx = Species_info[i].borrows_flyby_sounds_species;
			Species_info[i].snd_flyby_fighter = Species_info[idx].snd_flyby_fighter;
			Species_info[i].snd_flyby_bomber = Species_info[idx].snd_flyby_bomber;
		}
	}
}

/**
 * Close out gamesnd, ONLY CALL FROM game_shutdown()!!!!
 */
void gamesnd_close()
{
	Snds.clear();
	Snds_iface.clear();
	Snds_iface_handle.clear();
}

/**
 * Callback function for the UI code to call when the mouse first goes over a button.
 */
void common_play_highlight_sound()
{
	gamesnd_play_iface(InterfaceSounds::USER_OVER);
}

/**
 * Callback function for the UI code to call when an error beep needed.
 */
void gamesnd_play_error_beep()
{
	gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
}

game_snd* gamesnd_get_game_sound(gamesnd_id handle) {
	Assertion(handle.isValid(), "Invalid game sound handle detected!");
	Assertion(handle.value() < (int) Snds.size(), "Invalid game sound handle %d detected!", handle.value());
	return &Snds[handle.value()];
}
game_snd* gamesnd_get_interface_sound(interface_snd_id handle) {
	Assertion(handle.isValid(), "Invalid interface sound handle detected!");
	Assertion(handle.value() < (int) Snds_iface.size(), "Invalid interface sound handle %d detected!", handle.value());
	return &Snds_iface[handle.value()];
}

bool gamesnd_game_sound_valid(gamesnd_id sound) {
	return sound.isValid() && sound.value() < (int) Snds.size();
}
bool gamesnd_interface_sound_valid(interface_snd_id sound) {
	return sound.isValid() && sound.value() < (int) Snds_iface.size();
}

bool gamesnd_game_sound_try_load(gamesnd_id sound)
{
	if (!gamesnd_game_sound_valid(sound)) {
		return false;
	}
	auto gs = gamesnd_get_game_sound(sound);

	for (auto& entry : gs->sound_entries) {
		if (!entry.id.isValid()) {
			// Lazily load unloaded sound entries when required
			entry.id = snd_load(&entry, &gs->flags);
		}
	}

	// check flag
	return (gs->flags & GAME_SND_NOT_VALID) == 0;
}

float gamesnd_get_max_duration(game_snd* gs) {
	int max_length = 0;

	for (auto& entry : gs->sound_entries) {
		if (!entry.id.isValid()) {
			// Lazily load unloaded sound entries when required
			entry.id = snd_load(&entry, &gs->flags);
		}

		max_length = std::max(max_length, snd_get_duration(entry.id));
	}

	return max_length / gs->pitch_range.max();
}
game_snd_entry* gamesnd_choose_entry(game_snd* gs) {
	Assertion(!gs->sound_entries.empty(), "No sound entries found in game sound! This may not happen!");

	size_t index = 0;
	switch(gs->cycle_type) {
	case GameSoundCycleType::RandomCycle:
		if (gs->sound_entries.size() == 1) {
			index = 0;
		} else {
			index = util::UniformRange<size_t>(0, gs->sound_entries.size() - 1).next();
		}
		break;
	case GameSoundCycleType::SequentialCycle:
		if (gs->last_entry_index == std::numeric_limits<size_t>::max()) {
			// If this is the first time we must return the first sound to keep everything consistent
			index = 0;
		} else {
			index = (gs->last_entry_index + 1) % gs->sound_entries.size();
		}
		break;
	}

	gs->last_entry_index = index;
	return &gs->sound_entries[index];
}
