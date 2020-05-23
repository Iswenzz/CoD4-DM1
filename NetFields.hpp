#pragma once

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
