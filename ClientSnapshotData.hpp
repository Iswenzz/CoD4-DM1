#pragma once

#define MAX_HUDELEMENTS 31
#define MAX_HUDELEMS_ARCHIVAL MAX_HUDELEMENTS
#define MAX_HUDELEMS_CURRENT MAX_HUDELEMENTS

namespace Iswenzz
{
	typedef struct 
	{
		int	sprintButtonUpRequired;
		int	sprintDelay;
		int	lastSprintStart;
		int	lastSprintEnd;
		int	sprintStartMaxLength;
	} sprintState_t;

	typedef struct 
	{
		float	yaw;
		int	timer;
		int	transIndex;
		int	flags;
	} mantleState_t;

	typedef enum
	{
		PLAYER_OFFHAND_SECONDARY_SMOKE = 0x0,
		PLAYER_OFFHAND_SECONDARY_FLASH = 0x1,
		PLAYER_OFFHAND_SECONDARIES_TOTAL = 0x2,
	}OffhandSecondaryClass_t;

	typedef enum
	{
		PLAYERVIEWLOCK_NONE = 0x0,
		PLAYERVIEWLOCK_FULL = 0x1,
		PLAYERVIEWLOCK_WEAPONJITTER = 0x2,
		PLAYERVIEWLOCKCOUNT = 0x3,
	} ViewLockTypes_t;

	typedef enum
	{
		ACTIONSLOTTYPE_DONOTHING = 0x0,
		ACTIONSLOTTYPE_SPECIFYWEAPON = 0x1,
		ACTIONSLOTTYPE_ALTWEAPONTOGGLE = 0x2,
		ACTIONSLOTTYPE_NIGHTVISION = 0x3,
		ACTIONSLOTTYPECOUNT = 0x4,
	} ActionSlotType_t;

	typedef struct
	{
		unsigned int index;
	} ActionSlotParam_SpecifyWeapon_t;


	typedef struct
	{
		ActionSlotParam_SpecifyWeapon_t specifyWeapon;
	} ActionSlotParam_t;

	typedef enum
	{
		OBJST_EMPTY = 0x0,
		OBJST_ACTIVE = 0x1,
		OBJST_INVISIBLE = 0x2,
		OBJST_DONE = 0x3,
		OBJST_CURRENT = 0x4,
		OBJST_FAILED = 0x5,
		OBJST_NUMSTATES = 0x6,
	} objectiveState_t;

	typedef struct objective_s
	{
		objectiveState_t state;
		float origin[3];
		int entNum;
		int teamNum;
		int icon;
	} objective_t;

	typedef enum
	{
		HE_TYPE_FREE = 0x0,
		HE_TYPE_TEXT = 0x1,
		HE_TYPE_VALUE = 0x2,
		HE_TYPE_PLAYERNAME = 0x3,
		HE_TYPE_MAPNAME = 0x4,
		HE_TYPE_GAMETYPE = 0x5,
		HE_TYPE_MATERIAL = 0x6,
		HE_TYPE_TIMER_DOWN = 0x7,
		HE_TYPE_TIMER_UP = 0x8,
		HE_TYPE_TENTHS_TIMER_DOWN = 0x9,
		HE_TYPE_TENTHS_TIMER_UP = 0xA,
		HE_TYPE_CLOCK_DOWN = 0xB,
		HE_TYPE_CLOCK_UP = 0xC,
		HE_TYPE_WAYPOINT = 0xD,
		HE_TYPE_COUNT = 0xE,
	} he_type_t;

	/* 6853 */
	typedef struct
	{
		char r;
		char g;
		char b;
		char a;
	} hudelem_colorsplit_t;

	/* 6854 */
	typedef union
	{
		hudelem_colorsplit_t split;
		int rgba;
	} hudelem_color_t;

	typedef struct hudelem_s
	{
		he_type_t type;
		float x;
		float y;
		float z;
		int targetEntNum;
		float fontScale;
		int font;
		int alignOrg;
		int alignScreen;
		hudelem_color_t color;
		hudelem_color_t fromColor; //0x28
		int fadeStartTime; //0x2c
		int fadeTime;
		int label;
		int width;
		int height; //0x3C
		int materialIndex;
		int offscreenMaterialIdx; //0x44
		int fromWidth;
		int fromHeight;
		int scaleStartTime;
		int scaleTime;
		float fromX;
		float fromY;
		int fromAlignOrg;
		int fromAlignScreen;
		int moveStartTime;
		int moveTime;
		int time;
		int duration;
		float value;
		int text; //Configstring index
		float sort;
		hudelem_color_t glowColor; //0x84
		int fxBirthTime;
		int fxLetterTime;
		int fxDecayStartTime;
		int fxDecayDuration;
		int soundID;
		int flags;
	} hudelem_t;

	typedef struct hudElemState_s
	{
		hudelem_t current[MAX_HUDELEMENTS];
		hudelem_t archival[MAX_HUDELEMENTS];
	} hudElemState_t;

	typedef struct playerState_s 
	{
		int	commandTime;				// 0
		int	pm_type;					// 4
		int	bobCycle;					// 8
		int	pm_flags;					// 12
		int	weapFlags;					// 16
		int	otherFlags;					// 20
		int	pm_time;					// 24
		float origin[3];				// 28
		float velocity[3];				// 40
		float oldVelocity[2];
		int	weaponTime;					// 60
		int	weaponDelay;				// 64
		int	grenadeTimeLeft;			// 68
		int	throwBackGrenadeOwner;		// 72
		int	throwBackGrenadeTimeLeft;	// 76
		int	weaponRestrictKickTime;		// 80
		int	foliageSoundTime;			// 84
		int	gravity;					// 88
		float leanf;					// 92
		int	speed;						// 96
		float delta_angles[3];			// 100

		/*The ground entity's rotation will be added onto the player's view.  In particular, this will
		* cause the player's yaw to rotate around the entity's z-axis instead of the world z-axis.
		* Any rotation that the reference entity undergoes will affect the player.
		* http://zeroy.com/script/player/playersetgroundreferenceent.htm */
		int	groundEntityNum;			// 112
		float vLadderVec[3];			// 116
		int	jumpTime;					// 128
		float jumpOriginZ;				// 132

		// Animations as in mp/playeranim.script and animtrees/multiplayer.atr, it also depends on mp/playeranimtypes.txt (the currently used weapon)
		int	legsTimer;					// 136
		int	legsAnim;					// 140
		int	torsoTimer;					// 144
		int	torsoAnim;					// 148
		int	legsAnimDuration;
		int	torsoAnimDuration;

		int	damageTimer;				// 160
		int	damageDuration;				// 164
		int	flinchYawAnim;				// 168
		int	movementDir;				// 172
		int	eFlags;						// 176
		int	eventSequence;				// 180

		int events[4];
		unsigned int eventParms[4];
		int	oldEventSequence;

		int	clientNum;					// 220
		int	offHandIndex;				// 224
		OffhandSecondaryClass_t	offhandSecondary;  // 228
		unsigned int weapon;			// 232
		int	weaponstate;				// 236
		unsigned int weaponShotCount;	// 240
		float fWeaponPosFrac;			// 244
		int	adsDelayTime;				// 248

		// http://zeroy.com/script/player/resetspreadoverride.htm
		// http://zeroy.com/script/player/setspreadoverride.htm
		int	spreadOverride;				// 252
		int	spreadOverrideState;		// 256

		int	viewmodelIndex;				// 260
		float viewangles[3];			// 264
		int	viewHeightTarget;			// 276
		float viewHeightCurrent;		// 280
		int	viewHeightLerpTime;			// 284
		int	viewHeightLerpTarget;		// 288
		int	viewHeightLerpDown;			// 292
		float viewAngleClampBase[2];	// 296
		float viewAngleClampRange[2];	// 304

		int	damageEvent;				// 312
		int	damageYaw;					// 316
		int	damagePitch;				// 320
		int	damageCount;				// 324

		int	stats[5];
		int	ammo[128];
		int	ammoclip[128];

		unsigned int weapons[4];
		unsigned int weaponold[4];
		unsigned int weaponrechamber[4];

		float proneDirection;			// 1420
		float proneDirectionPitch;		// 1424
		float proneTorsoPitch;			// 1428
		ViewLockTypes_t	viewlocked;		// 1432
		int	viewlocked_entNum;			// 1436

		int	cursorHint;					// 1440
		int	cursorHintString;			// 1444
		int	cursorHintEntIndex;			// 1448

		int	iCompassPlayerInfo;			// 1452
		int	radarEnabled;				// 1456

		int	locationSelectionInfo;		// 1460
		sprintState_t sprintState;		// 1464

		// used for leaning?
		float fTorsoPitch;				// 1484
		float fWaistPitch;				// 1488

		float holdBreathScale;			// 1492
		int	holdBreathTimer;			// 1496

		// Scales player movement speed by this amount
		// http://zeroy.com/script/player/setmovespeedscale.htm
		float moveSpeedScaleMultiplier; // 1500

		mantleState_t mantleState;		// 1504
		float meleeChargeYaw;			// 1520
		int meleeChargeDist;			// 1524
		int	meleeChargeTime;			// 1528
		int	perks;						// 1532

		ActionSlotType_t actionSlotType[4];		// 1536
		ActionSlotParam_t actionSlotParam[4];	// 1552

		int	entityEventSequence;		// 1568

		int	weapAnim;					// 1572
		float aimSpreadScale;			// 1576

		// http://zeroy.com/script/player/shellshock.htm
		int	shellshockIndex;			// 1580
		int	shellshockTime;				// 1584
		int	shellshockDuration;			// 1588

		// http://zeroy.com/script/player/setdepthoffield.htm
		float dofNearStart;				// 1592
		float dofNearEnd;				// 1596
		float dofFarStart;				// 1600
		float dofFarEnd;				// 1604
		float dofNearBlur;				// 1608
		float dofFarBlur;				// 1612
		float dofViewmodelStart;		// 1616
		float dofViewmodelEnd;			// 1620

		int	hudElemLastAssignedSoundID;  // 1624
		objective_t objective[16];
		char weaponmodels[128];
		int	deltaTime;					// 2204
		int	killCamEntity;				// 2208

		hudElemState_t hud;				// 2212
	} playerState_t;						//Size: 0x2f64

	typedef struct 
	{										// (0x2146c)
		playerState_t	ps;					// (0x2146c)
		int		num_entities;
		int		num_clients;				// (0x2f68)
		int		first_entity;				// (0x2f6c)into the circular sv_packet_entities[]
		int		first_client;
											// the entities MUST be in increasing state number
											// order, otherwise the delta compression will fail
		unsigned int	messageSent;		// (0x243e0 | 0x2f74) time the message was transmitted
		unsigned int	messageAcked;		// (0x243e4 | 0x2f78) time the message was acked
		int		messageSize;				// (0x243e8 | 0x2f7c) used to rate drop packets
		int		var_03;
	} clientSnapshot_t;						// size: 0x2f84

	struct ClientSnapshotData
	{
		int lastClientCommand;		// 0x4
		int lastServerCommand;		// 0x8
		clientSnapshot_t s;			// 0x2F8C
	};
}