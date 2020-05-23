#pragma once
#define ARCHIVED_ENTITY_FIELDS_COUNT			69
#define CLIENT_STATE_FIELDS_COUNT				24
#define HUD_ELEM_FIELDS_COUNT					40
#define PLAYER_STATE_FIELDS_COUNT				141
#define NET_FIELDS_COUNT						18
#define PLANE_STATE_FIELDS_COUNT				60
#define HELICOPTER_ENTITY_STATE_FIELDS_COUNT	58
#define ENTITY_STATE_FIELDS_COUNT				59

namespace Iswenzz
{
	int GetMinBitCount(int x);

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
}
