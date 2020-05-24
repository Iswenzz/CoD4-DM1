#pragma once
#define ARCHIVED_ENTITY_FIELDS_COUNT			69
#define CLIENT_STATE_FIELDS_COUNT				24
#define HUD_ELEM_FIELDS_COUNT					40
#define PLAYER_STATE_FIELDS_COUNT				141
#define NET_FIELDS_COUNT						18
#define PLANE_STATE_FIELDS_COUNT				60
#define HELICOPTER_ENTITY_STATE_FIELDS_COUNT	58
#define ENTITY_STATE_FIELDS_COUNT				59
#define OBJECTIVE_FIELDS_COUNT					6

#define VectorCopy(a, b) ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])

namespace Iswenzz
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

	int GetMinBitCount(int x);

	extern netField_t entityStateFields[ENTITY_STATE_FIELDS_COUNT];
	extern netField_t playerEntityStateFields[ENTITY_STATE_FIELDS_COUNT];
	extern netField_t corpseEntityStateFields[ENTITY_STATE_FIELDS_COUNT];
	extern netField_t itemEntityStateFields[ENTITY_STATE_FIELDS_COUNT];
	extern netField_t missleEntityStateFields[ENTITY_STATE_FIELDS_COUNT];
	extern netField_t scriptMoverStateFields[ENTITY_STATE_FIELDS_COUNT];
	extern netField_t soundBlendEntityStateFields[ENTITY_STATE_FIELDS_COUNT];
	extern netField_t fxStateFields[ENTITY_STATE_FIELDS_COUNT];
	extern netField_t loopFxEntityStateFields[ENTITY_STATE_FIELDS_COUNT];
	extern netField_t helicopterEntityStateFields[HELICOPTER_ENTITY_STATE_FIELDS_COUNT];
	extern netField_t planeStateFields[PLANE_STATE_FIELDS_COUNT];
	extern netField_t vehicleEntityStateFields[ENTITY_STATE_FIELDS_COUNT];
	extern netField_t eventEntityStateFields[ENTITY_STATE_FIELDS_COUNT];
	extern netFieldList_t netFieldList[NET_FIELDS_COUNT];
	extern netField_t playerStateFields[PLAYER_STATE_FIELDS_COUNT];
	extern netField_t hudElemFields[HUD_ELEM_FIELDS_COUNT];
	extern netField_t clientStateFields[CLIENT_STATE_FIELDS_COUNT];
	extern netField_t archivedEntityFields[ARCHIVED_ENTITY_FIELDS_COUNT];
	extern netField_t objectiveFields[OBJECTIVE_FIELDS_COUNT];
}
