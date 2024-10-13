#pragma once
#define ARCHIVED_ENTITY_FIELDS_COUNT 69
#define CLIENT_STATE_FIELDS_COUNT 24
#define HUD_ELEM_FIELDS_COUNT 40
#define PLAYER_STATE_FIELDS_COUNT 141
#define NET_FIELDS_COUNT 18
#define PLANE_STATE_FIELDS_COUNT 60
#define HELICOPTER_ENTITY_STATE_FIELDS_COUNT 58
#define ENTITY_STATE_FIELDS_COUNT 59
#define OBJECTIVE_FIELDS_COUNT 6

#define NETFE(x) x, sizeof(x) / sizeof(netField_t)
#define NETF(x) const_cast<char*>(#x), static_cast<int>(reinterpret_cast<uintptr_t>(&((entityState_t*)0)->x))
#define OBJF(x) const_cast<char*>(#x), static_cast<int>(reinterpret_cast<uintptr_t>(&((objective_t*)0)->x))
#define AEF(x) const_cast<char*>(#x), static_cast<int>(reinterpret_cast<uintptr_t>(&((archivedEntity_t*)0)->x))
#define CSF(x) const_cast<char*>(#x), static_cast<int>(reinterpret_cast<uintptr_t>(&((clientState_t*)0)->x))
#define HEF(x) const_cast<char*>(#x), static_cast<int>(reinterpret_cast<uintptr_t>(&((hudelem_t*)0)->x))
#define PSF(x) const_cast<char*>(#x), static_cast<int>(reinterpret_cast<uintptr_t>(&((playerState_t*)0)->x))

#ifdef _MSC_VER
	#define __clz __lzcnt
#else
	#define __clz __builtin_clz
#endif

namespace CoD4::DM1
{
	typedef struct
	{
		const char* name;
		int a;
		int b;
	} subNetEntlist_t;

	typedef struct
	{
		subNetEntlist_t* sub;
		int z;
	} netEntlist_t;

	typedef struct netField_s
	{
		char* name;
		int offset;
		int bits; // 0 = float
		unsigned char changeHints;
		unsigned char pad[3];
	} netField_t;

	typedef struct
	{
		netField_t* field;
		int numFields;
	} netFieldList_t;

	enum PacketEntityType
	{
		ANALYZE_DATATYPE_ENTITYTYPE_GENERALENTITY = 0x0,
		ANALYZE_DATATYPE_ENTITYTYPE_PLAYERENTITY = 0x1,
		ANALYZE_DATATYPE_ENTITYTYPE_PLAYERCORPSEENTITY = 0x2,
		ANALYZE_DATATYPE_ENTITYTYPE_ITEMENTITY = 0x3,
		ANALYZE_DATATYPE_ENTITYTYPE_MISSILEENTITY = 0x4,
		ANALYZE_DATATYPE_ENTITYTYPE_INVISIBLEENTITY = 0x5,
		ANALYZE_DATATYPE_ENTITYTYPE_SCRIPTMOVERENTITY = 0x6,
		ANALYZE_DATATYPE_ENTITYTYPE_SOUNDBLENDENTITY = 0x7,
		ANALYZE_DATATYPE_ENTITYTYPE_FXENTITY = 0x8,
		ANALYZE_DATATYPE_ENTITYTYPE_LOOPFXENTITY = 0x9,
		ANALYZE_DATATYPE_ENTITYTYPE_PRIMARYLIGHTENTITY = 0xA,
		ANALYZE_DATATYPE_ENTITYTYPE_MG42ENTITY = 0xB,
		ANALYZE_DATATYPE_ENTITYTYPE_HELICOPTER = 0xC,
		ANALYZE_DATATYPE_ENTITYTYPE_PLANE = 0xD,
		ANALYZE_DATATYPE_ENTITYTYPE_VEHICLE = 0xE,
		ANALYZE_DATATYPE_ENTITYTYPE_VEHICLE_COLLMAP = 0xF,
		ANALYZE_DATATYPE_ENTITYTYPE_VEHICLE_CORPSE = 0x10,
		ANALYZE_DATATYPE_ENTITYTYPE_ACTOR = 0x11,
		ANALYZE_DATATYPE_ENTITYTYPE_ACTOR_SPAWNER = 0x12,
		ANALYZE_DATATYPE_ENTITYTYPE_ACTOR_CORPSE = 0x13,
		ANALYZE_DATATYPE_ENTITYTYPE_STREAMER_HINT = 0x14,
		ANALYZE_DATATYPE_ENTITYTYPE_TEMPENTITY = 0x15,
		ANALYZE_DATATYPE_ENTITYTYPE_ARCHIVEDENTITY = 0x16,
		ANALYZE_DATATYPE_ENTITYTYPE_MATCHSTATE = 0x17,
		ANALYZE_DATATYPE_ENTITYTYPE_CLIENTSTATE = 0x18,
		ANALYZE_DATATYPE_ENTITYTYPE_PLAYERSTATE = 0x19,
		ANALYZE_DATATYPE_ENTITYTYPE_HUDELEM = 0x1A,
		ANALYZE_DATATYPE_ENTITYTYPE_BASELINE = 0x1B,
		ANALYZE_DATATYPE_ENTITYTYPE_COUNT = 0x1C,
	};

	/// <summary>
	/// This class contains all net fields used in delta compressed packets.
	/// A net field is composed by a field name, its offset in a specific struct,
	/// and bits for specific variable reading.
	/// </summary>
	class NetFields
	{
	public:
		static netField_t EntityStateFields[ENTITY_STATE_FIELDS_COUNT];
		static netField_t PlayerEntityStateFields[ENTITY_STATE_FIELDS_COUNT];
		static netField_t CorpseEntityStateFields[ENTITY_STATE_FIELDS_COUNT];
		static netField_t ItemEntityStateFields[ENTITY_STATE_FIELDS_COUNT];
		static netField_t MissleEntityStateFields[ENTITY_STATE_FIELDS_COUNT];
		static netField_t ScriptMoverStateFields[ENTITY_STATE_FIELDS_COUNT];
		static netField_t SoundBlendEntityStateFields[ENTITY_STATE_FIELDS_COUNT];
		static netField_t FxStateFields[ENTITY_STATE_FIELDS_COUNT];
		static netField_t LoopFxEntityStateFields[ENTITY_STATE_FIELDS_COUNT];
		static netField_t HelicopterEntityStateFields[HELICOPTER_ENTITY_STATE_FIELDS_COUNT];
		static netField_t PlaneStateFields[PLANE_STATE_FIELDS_COUNT];
		static netField_t VehicleEntityStateFields[ENTITY_STATE_FIELDS_COUNT];
		static netField_t EventEntityStateFields[ENTITY_STATE_FIELDS_COUNT];
		static netFieldList_t List[NET_FIELDS_COUNT];
		static netField_t PlayerStateFields[PLAYER_STATE_FIELDS_COUNT];
		static netField_t HudElemFields[HUD_ELEM_FIELDS_COUNT];
		static netField_t ClientStateFields[CLIENT_STATE_FIELDS_COUNT];
		static netField_t ArchivedEntityFields[ARCHIVED_ENTITY_FIELDS_COUNT];
		static netField_t ObjectiveFields[OBJECTIVE_FIELDS_COUNT];

		/// <summary>
		/// Get the min bit count of an integer.
		/// </summary>
		/// <param name="x">The integer to check the min bit count.</param>
		/// <returns></returns>
		static int GetMinBitCount(int x);
	};
}
