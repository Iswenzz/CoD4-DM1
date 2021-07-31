#pragma once
#include <string>
#include <array>

#define NETCHAN_UNSENTBUFFER_SIZE		0x20000
#define NETCHAN_FRAGMENTBUFFER_SIZE		0x800
#define NETCHAN_MAXBUFFER_SIZE			NETCHAN_UNSENTBUFFER_SIZE * 10
#define SYS_COMMONVERSION				17.5
#define	PROTOCOL_VERSION				(unsigned int)(SYS_COMMONVERSION + 0.00001)
#define COD4_PROTOCOL					1
#define COD4X_FALLBACK_PROTOCOL			17

#define VectorCopy(a, b)	((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#define	ANGLE2SHORT(x)		((int)((x)*65536.0f/360.0f) & 65535)
#define	SHORT2ANGLE(x)		((x)*(360.0/65536))

#define GENTITYNUM_BITS 10
#define BIG_INFO_STRING 8192
#define PACKET_BACKUP 32
#define PACKET_MASK PACKET_BACKUP - 1

#define FREETEXT_STARTINDEX 309
#define MATERIAL_STARTINDEX 2002

#define GROUND_CLIENT_NUM 1022
#define KILLCAM_ENTITY_ON 1023
#define KILLCAM_ENTITY_OFF 0

#define MAX_HUDELEMENTS 31
#define MAX_HUDELEMS_ARCHIVAL MAX_HUDELEMENTS
#define MAX_HUDELEMS_CURRENT MAX_HUDELEMENTS
#define MAX_CONFIGSTRINGS 3000
#define MAX_GENTITIES (1 << GENTITYNUM_BITS)
#define MAX_STRING_CHARS 1024 
#define MAX_FRAMES 256
#define MAX_CLIENTS 64
#define MAX_CMDSTRINGS 2048
#define MAX_STATS 5
#define MAX_WEAPONS 16
#define MAX_PS_EVENTS 4
#define MAX_RELIABLE_COMMANDS 128
#define MAX_MAP_AREA_BYTES 32
#define MAX_PARSE_ENTITIES 2048
#define MAX_PARSE_CLIENTS 2048
#define MAX_CLIENTEVENTS 128
#define MAX_KILLFEED 128
	         
#define ENTITYNUM_NONE MAX_GENTITIES - 1
#define ENTITYNUM_WORLD MAX_GENTITIES - 2
#define ENTITYNUM_MAX_NORMAL MAX_GENTITIES - 2

#define LogIf(printIf, ostream) \
	if (printIf) std::cout << ostream
#define VerboseLog(ostream) \
	if (Verbose) std::cout << ostream

namespace Iswenzz
{
	typedef unsigned int clipHandle_t;

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

		// Animations as in mp/playeranim.script and animtrees/multiplayer.atr
		// it also depends on mp/playeranimtypes.txt (the currently used weapon)
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

		int	hudElemLastAssignedSoundID; // 1624
		objective_t objective[16];
		char weaponmodels[128];
		int	deltaTime;					// 2204
		int	killCamEntity;				// 2208

		hudElemState_t hud;				// 2212
	} playerState_t;					//Size: 0x2f64

	typedef struct 
	{
		bool valid;
		int snapFlags;
		int serverTime;
		int messageNum;
		int deltaNum;
		int ping;
		std::array<unsigned char, MAX_MAP_AREA_BYTES> areamask;
		int cmdNum;
		playerState_t ps;
		int numEntities;
		int parseEntitiesNum;
		int numClients;
		int parseClientsNum;
		int serverCommandNum;
	} clientSnapshot_t;

	enum entityType_t
	{
		ET_GENERAL = 0x0,
		ET_PLAYER = 0x1,
		ET_PLAYER_CORPSE = 0x2,
		ET_ITEM = 0x3,
		ET_MISSILE = 0x4,
		ET_INVISIBLE = 0x5,
		ET_SCRIPTMOVER = 0x6,
		ET_SOUND_BLEND = 0x7,
		ET_FX = 0x8,
		ET_LOOP_FX = 0x9,
		ET_PRIMARY_LIGHT = 0xA,
		ET_MG42 = 0xB,
		ET_HELICOPTER = 0xC,
		ET_PLANE = 0xD,
		ET_VEHICLE = 0xE,
		ET_VEHICLE_COLLMAP = 0xF,
		ET_VEHICLE_CORPSE = 0x10,
		ET_EVENTS = 0x11,
		ET_MOVER = 0x99					//Dummy for botlib
	};

	struct LerpEntityStatePhysicsJitter
	{
		float innerRadius;
		float minDisplacement;
		float maxDisplacement;
	};

	struct LerpEntityStatePlayer
	{
		float leanf;
		int movementDir;
	};

	struct LerpEntityStateLoopFx
	{
		float cullDist;
		int period;
	};

	struct LerpEntityStateCustomExplode
	{
		int startTime;
	};

	struct LerpEntityStateTurret
	{
		float gunAngles[3];
	};

	struct LerpEntityStateAnonymous
	{
		int buffer[7];
	};

	struct LerpEntityStateExplosion
	{
		float innerRadius;
		float magnitude;
	};

	struct LerpEntityStateBulletHit
	{
		float start[3];
	};

	struct LerpEntityStatePrimaryLight
	{
		unsigned char colorAndExp[4];
		float intensity;
		float radius;
		float cosHalfFovOuter;
		float cosHalfFovInner;
	};

	struct LerpEntityStateMissile
	{
		int launchTime;
	};

	struct LerpEntityStateSoundBlend
	{
		float lerp;
	};

	struct LerpEntityStateExplosionJolt
	{
		float innerRadius;
		float impulse[3];
	};

	struct LerpEntityStateVehicle
	{
		float bodyPitch;
		float bodyRoll;
		float steerYaw;
		int materialTime;
		float gunPitch;
		float gunYaw;
		int team;
	};

	struct LerpEntityStateEarthquake
	{
		float scale;
		float radius;
		int duration;
	};

	union LerpEntityStateTypeUnion
	{
		struct LerpEntityStateTurret turret;
		struct LerpEntityStateLoopFx loopFx;
		struct LerpEntityStatePrimaryLight primaryLight;
		struct LerpEntityStatePlayer player;
		struct LerpEntityStateVehicle vehicle;
		struct LerpEntityStateMissile missile;
		struct LerpEntityStateSoundBlend soundBlend;
		struct LerpEntityStateBulletHit bulletHit;
		struct LerpEntityStateEarthquake earthquake;
		struct LerpEntityStateCustomExplode customExplode;
		struct LerpEntityStateExplosion explosion;
		struct LerpEntityStateExplosionJolt explosionJolt;
		struct LerpEntityStatePhysicsJitter physicsJitter;
		struct LerpEntityStateAnonymous anonymous;
	};

	typedef enum 
	{
		TR_STATIONARY,
		TR_INTERPOLATE,				// non-parametric, but interpolate between Snapshots
		TR_LINEAR,
		TR_LINEAR_STOP,
		TR_SINE,					// value = base + sin( time / duration ) * delta
		TR_GRAVITY
	} trType_t;

	typedef struct 
	{
		trType_t trType;
		int trTime;
		int trDuration;			// if non 0, trTime + trDuration = stop time
		float trBase[3];
		float trDelta[3];		// velocity, etc
	} trajectory_t;

	struct LerpEntityState
	{
		int eFlags;
		trajectory_t pos;
		trajectory_t apos;
		union LerpEntityStateTypeUnion u;
	};

	// entityState_t is the information conveyed from the server
	// in an update message about entities that the client will
	// need to render in some way
	// Different eTypes may use the information in different ways
	// The messages are delta compressed, so it doesn't really matter if
	// the structure size is fairly large
	typedef struct entityState_s 
	{									// Confirmed names and offsets but not types
		int	number;						// entity index	//0x00
		enum entityType_t eType;		// entityType_t	//0x04

		struct LerpEntityState lerp;
		int	time2;						// 0x70
		int	otherEntityNum;				// 0x74 shotgun sources, etc
		int	attackerEntityNum;			// 0x78
		int	groundEntityNum;			// 0x7c -1 = in air
		int	loopSound;					// 0x80 constantly loop this sound
		int	surfType;					// 0x84

		clipHandle_t index;				// 0x88
		int	clientNum;					// 0x8c 0 to (MAX_CLIENTS - 1), for players and corpses
		int	iHeadIcon;					// 0x90
		int	iHeadIconTeam;				// 0x94
		int	solid;						// 0x98 for client side prediction, trap_linkentity sets this properly	0x98

		int	eventParm;					// 0x9c impulse events -- muzzle flashes, footsteps, etc
		int	eventSequence;				// 0xa0

		float events[4];				// 0xa4
		float eventParms[4];			// 0xb4

		// for players
		int	weapon;						// 0xc4 determines weapon and flash model, etc
		int	weaponModel;				// 0xc8
		int	legsAnim;					// 0xcc mask off ANIM_TOGGLEBIT
		int	torsoAnim;					// 0xd0 mask off ANIM_TOGGLEBIT

		union 
		{
			int	helicopterStage;		// 0xd4
		} un1;

		int	un2;						// 0xd8
		int	fTorsoPitch;				// 0xdc
		int	fWaistPitch;				// 0xe0
		unsigned int partBits[4];		// 0xe4
	} entityState_t;					// sizeof(entityState_t): 0xf4

	#define MAX_NETNAME 32
	#define MAX_CLANNAME 16

	typedef enum team_s 
	{
		TEAM_FREE,
		TEAM_RED,
		TEAM_BLUE,
		TEAM_SPECTATOR,
		TEAM_NUM_TEAMS
	} team_t;

	typedef struct clientState_s
	{
		int clientIndex;				// 4
		team_t team;					// 8
		int modelindex;					// 12
		int attachModelIndex[6];		// 36
		int attachTagIndex[6];			// 60
		char netname[MAX_NETNAME];		// 76
		float maxSprintTimeMultiplier;	// 80
		int rank;						// 84
		int prestige;					// 88
		int perks;						// 92
		int attachedVehEntNum;			// 96
		int attachedVehSlotIndex;		// 100
	} clientState_t;

	typedef struct
	{
		std::string netname;
		std::string clantag;
	} clientNames_t;

	/* 7472 */
	typedef struct
	{
		int svFlags;
		int clientMask[2];
		float absmin[3];
		float absmax[3];
	} archivedEntityShared_t;

	/* 7473 */
	typedef struct archivedEntity_s
	{
		entityState_t s;
		archivedEntityShared_t r;
	} archivedEntity_t;

	typedef struct archivedFrame_s
	{
		int index;
		float origin[3];
		float velocity[3];
		int movementDir;
		int bobCycle;
		int commandTime;
		float angles[3];
	} archivedFrame_t;
}
