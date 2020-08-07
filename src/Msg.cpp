#include "Msg.hpp"
#include "Huffman.hpp"
#include "NetFields.hpp"
#include "ClientSnapshotData.hpp"
#include <cstring>
#include <stdint.h>
#include <iostream>

namespace Iswenzz
{
	int kbitmask[33] =
	{
		0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191,
		16383, 32767, 65535, 131071, 262143, 524287, 1048575, 2097151,
		4194303, 8388607, 16777215, 33554431, 67108863, 134217727,
		268435455, 536870911, 1073741823, 2147483647, 4294967295
	};

	Msg::Msg(unsigned char* buf, std::size_t len, MSGCrypt mode)
		: overflowed(false), readonly(false), splitData(nullptr), splitSize(0), readcount(0),
		bit(0), lastRefEntity(0), data(nullptr), cursize(0), maxsize(0)
	{
		if (mode == MSGCrypt::MSG_CRYPT_NONE)
		{
			data = new unsigned char[NETCHAN_UNSENTBUFFER_SIZE];
			std::memcpy(data, buf, len);
			cursize = len;
			maxsize = NETCHAN_UNSENTBUFFER_SIZE;
		}
		else if (mode == MSGCrypt::MSG_CRYPT_HUFFMAN)
		{
			data = new unsigned char[NETCHAN_UNSENTBUFFER_SIZE];
			cursize = MSG_ReadBitsCompress(buf, len, data, NETCHAN_FRAGMENTBUFFER_SIZE);
			maxsize = NETCHAN_UNSENTBUFFER_SIZE;
		}
	}

	Msg::~Msg() 
	{ 
		if (data)
			delete data;
	}

	int Msg::readBit()
	{
		int oldbit7, numBytes, bits;

		oldbit7 = bit & 7;
		if (!oldbit7)
		{
			if (readcount >= cursize + splitSize)
			{
				overflowed = 1;
				return -1;
			}
			bit = 8 * readcount;
			readcount++;
		}

		numBytes = bit / 8;
		if (numBytes < cursize)
		{
			bits = data[numBytes] >> oldbit7;
			bit++;
			return bits & 1;
		}
		bits = splitData[numBytes - cursize] >> oldbit7;
		bit++;
		return bits & 1;
	}

	int Msg::readBits(int numBits)
	{
		int i;
		signed int var;
		int retval = 0;

		if (numBits > 0)
		{
			for (i = 0; numBits != i; i++)
			{
				if (!(bit & 7))
				{
					if (readcount >= splitSize + cursize)
					{
						overflowed = 1;
						return -1;
					}
					bit = 8 * readcount;
					readcount++;
				}
				if (((bit / 8)) >= cursize)
				{
					if (splitData == nullptr)
						return 0;

					var = splitData[(bit / 8) - cursize];
				}
				else
					var = data[bit / 8];

				retval |= ((var >> (bit & 7)) & 1) << i;
				bit++;
			}
		}
		return retval;
	}

	int Msg::readByte()
	{
		unsigned char* c;
		if (readcount + sizeof(unsigned char) > cursize)
		{
			overflowed = 1;
			return -1;
		}
		c = &data[readcount];

		readcount += sizeof(unsigned char);
		return *c;
	}

	int Msg::readShort()
	{
		signed short* c;
		if (readcount + sizeof(short) > cursize) 
		{
			overflowed = 1;
			return -1;
		}
		c = reinterpret_cast<short*>(&data[readcount]);

		readcount += sizeof(short);
		return *c;
	}

	int Msg::readInt()
	{
		int32_t* c;

		if (readcount + sizeof(int32_t) > cursize) 
		{
			overflowed = 1;
			return -1;
		}
		c = reinterpret_cast<int32_t*>(&data[readcount]);

		readcount += sizeof(int32_t);
		return *c;
	}

	int64_t Msg::readInt64()
	{
		int64_t* c;
		if (readcount + sizeof(int64_t) > cursize) 
		{
			overflowed = 1;
			return -1;
		}
		c = reinterpret_cast<int64_t*>(&data[readcount]);

		readcount += sizeof(int64_t);
		return *c;
	}

	float Msg::readFloat()
	{
		float* c;
		if (readcount + sizeof(float) > cursize) 
		{
			overflowed = 1;
			return -1;
		}
		c = reinterpret_cast<float*>(&data[readcount]);

		readcount += sizeof(float);
		return *c;
	}

	double Msg::readAngle16()
	{
		return SHORT2ANGLE((double)readShort());
	}

	int Msg::readEFlags(int oldFlags)
	{
		int bitChanged, value, i, b;
		if (readBit() == 1)
		{
			value = 0;
			for (i = 0; i < 24; i += 8)
			{
				b = readByte();
				value |= (b << i);
			}
		}
		else
		{
			bitChanged = readBits(5);
			value = oldFlags ^ (1 << bitChanged);
		}
		return value;
	}

	int Msg::readEntityIndex(int indexBits)
	{
		if (readBit())
			++lastRefEntity;
		else if (indexBits != 10 || readBit())
			lastRefEntity = readBits(indexBits);
		else
			lastRefEntity += readBits(4);
		return lastRefEntity;
	}

	int Msg::readDeltaGroundEntity()
	{
		int j, value;
		if (readBit() == 1) return 1022;
		if (readBit() == 1) return 0;

		value = readBits(2);
		for (j = 2; j < 10; j += 8)
			value |= readByte() << j;
		return value;
	}

	int Msg::readDeltaStruct(const int time, const void* from, void* to, unsigned int number, 
		int numFields, int indexBits, netField_t* stateFields)
	{
		if (readBit() == 1) return 1;
		*(uint32_t*)to = number;
		readDeltaFields(time, reinterpret_cast<const unsigned char*>(from), 
			reinterpret_cast<unsigned char*>(to), numFields, stateFields);
		return 0;
	}

	void Msg::readDeltaFields(const int time, const unsigned char* from, unsigned char* to, 
		int numFields, netField_t* stateFields)
	{
		int lc, i;
		bool print;

		if (!readBit())
		{
			std::memcpy(to, from, 4 * numFields + 4);
			return;
		}
		lc = readLastChangedField(numFields);

		if (lc > numFields)
		{
			overflowed = 1;
			return;
		}

		for (i = 0; i < lc; ++i)
			readDeltaField(time, from, to, &stateFields[i], false, true);
		for (i = lc; i < numFields; ++i)
		{
			int offset = stateFields[i].offset;
			*(uint32_t*)&to[offset] = *(uint32_t*)&from[offset];
		}
	}

	float Msg::readOriginFloat(int bits, float oldValue)
	{
		signed int coord;
		if (readBit())
		{
			float center[3]{ 0, 0, 0 };
			coord = (signed int)(center[bits != -92] + 0.5);
			return (double)((((signed int)oldValue - coord + 0x8000) ^ readBits(16)) + coord - 0x8000);
		}
		return (double)(readBits(7) - 64) + oldValue;
	}

	float Msg::readOriginZFloat(float oldValue)
	{
		signed int coord;
		if (readBit())
		{
			float center[3]{ 0, 0, 0 };
			coord = (signed int)(center[2] + 0.5);
			return (double)((((signed int)oldValue - coord + 0x8000) ^ readBits(16)) + coord - 0x8000);
		}
		return (double)(readBits(7) - 64) + oldValue;
	}

	std::string Msg::readString(int len)
	{
		int l = 0, c;
		std::string bigstring;
		do 
		{
			c = readByte();      // use ReadByte so -1 is out of bounds
			if (c == -1 || c == 0)
				break;
			// translate all fmt spec to avoid crash bugs
			if (c == '%')
				c = '.';
			bigstring += static_cast<unsigned char>(c);
			l++;
		} 
		while (l < len - 1);
		return bigstring;
	}

	std::string Msg::readStringLine(int len)
	{
		int	l = 0, c;
		std::string bigstring;
		do 
		{
			c = readByte();	// use ReadByte so -1 is out of bounds
			if (c == -1 || c == 0 || c == '\n')
				break;
			// translate all fmt spec to avoid crash bugs
			if (c == '%')
				c = '.';
			bigstring += static_cast<char>(c);
			l++;
		} 
		while (l < len - 1);
		return bigstring;
	}

	void Msg::readData(void* data, int len)
	{
		for (int i = 0; i < len; i++)
			reinterpret_cast<unsigned char*>(data)[i] = readByte();
	}

	// @TODO
	void Msg::readDeltaField(int time, const void* from, const void* to, const netField_t* field, 
		bool noXor, bool print)
	{
		unsigned char* fromF;
		unsigned char* toF;
		int bits, b, bit_vect, v, zeroV = 0;
		uint32_t l;
		double f = 0;
		signed int t;

		if (noXor)
			fromF = (unsigned char*)&zeroV;
		else
			fromF = (unsigned char*)from + field->offset;
		toF = (unsigned char*)to + field->offset;

		if (field->changeHints != 2)
		{
			if (!readBit()) // No change
			{
				*(uint32_t*)toF = *(uint32_t*)fromF;
				return;
			}
		} 

		//Changed field
		bits = field->bits;
		if (!bits)
		{
			if (!readBit())
			{
				*(uint32_t*)toF = readBit() << 31;
				return;
			}
			if (!readBit())
			{
				b = readBits(5);
				v = ((32 * readByte() + b) ^ ((signed int)*(float*)fromF + 4096)) - 4096;
				*(float*)toF = (double)v;
				if (print)
					std::cout << field->name << "{" << field->bits << "} = " << v << " ";
				return;
			}
			l = readInt();
			*(uint32_t*)toF = l;
			*(uint32_t*)toF = l ^ *(uint32_t*)fromF;
			
			if (!print) return;
			f = *(float*)toF;
			std::cout << field->name << "{" << field->bits << "} = " << f << " ";
			return;
		}

		// Command
		switch (field->bits)
		{
			case -89:
				if (!readBit())
				{
					b = readBits(5);
					l = ((32 * readByte() + b) ^ ((signed int)*(float*)fromF + 4096)) - 4096;
					*(float*)toF = (double)l;
					if (print)
						std::cout << field->name << "{" << field->bits << "} = " << l << " ";
					return;
				}
				l = readInt();
				*(uint32_t*)toF = l ^ *(uint32_t*)fromF;

				if (!print) return;
				f = *(float*)toF;
				std::cout << field->name << "{" << field->bits << "} = " << f << " ";
				return;

			case -88:
				l = readInt();
				*(uint32_t*)toF = l ^ *(uint32_t*)fromF;

				if (!print) return;
				f = *(float*)toF;
				std::cout << field->name << "{" << field->bits << "} = " << f << " ";
				return;

			case -100:
				if (!readBit())
				{
					*(float*)toF = 0.0;
					return;
				}
				*(float*)toF = readAngle16();
				return;

			case -99:
				if (readBit())
				{
					if (!readBit())
					{
						b = readBits(4);
						v = ((16 * readByte() + b) ^ ((signed int)*(float*)fromF + 2048)) - 2048;
						*(float*)toF = (double)v;
						if (print)
							std::cout << field->name << "{" << field->bits << "} = " << (int)v << " ";
						return;
					}
					l = readInt();
					*(uint32_t*)toF = l ^ *(uint32_t*)fromF;
					
					if (!print) return;
					f = *(float*)toF;
					std::cout << field->name << "{" << field->bits << "} = " << f << " ";
					return;
				}
				*(uint32_t*)toF = 0;
				return;

			case -98:
				*(uint32_t*)toF = readEFlags(*(uint32_t*)fromF);
				return;

			case -97:
				if (readBit())
					*(uint32_t*)toF = readInt();
				else
					*(uint32_t*)toF = time - readBits(8);
				return;

			case -96:
				*(uint32_t*)toF = readDeltaGroundEntity();
				return;

			case -95:
				*(uint32_t*)toF = 100 * readBits(7);
				return;

			case -94:
			case -93:
				*(uint32_t*)toF = readByte();
				return;

			case -92:
			case -91:
				f = readOriginFloat(bits, *(float*)fromF);
				*(float*)toF = f;
				if (print)
					std::cout << field->name << "{" << field->bits << "} = " << f << " ";
				return;

			case -90:
				f = readOriginZFloat(*(float*)fromF);
				*(float*)toF = f;

				if (!print) return;
					std::cout << field->name << "{" << field->bits << "} = " << f << " ";
				return;

			case -87:
				*(float*)toF = readAngle16();
				return;

			case -86:
				*(float*)toF = (double)readBits(5) / 10.0 + 1.399999976158142;
				return;

			case -85:
				if (readBit())
				{
					*(uint32_t*)toF = *(uint32_t*)fromF;
					toF[3] = (fromF[3] != 0) - 1;
					return;
				}
				if (!readBit())
				{
					toF[0] = readByte();
					toF[1] = readByte();
					toF[2] = readByte();
				}
				toF[3] = 8 * readBits(5);
				return;

			default:
				if (!readBit())
				{
					*(uint32_t*)toF = 0;
					return;
				}
				bits = abs(field->bits);
				bit_vect = bits & 7;

				if (bit_vect) t = readBits(bit_vect);
				else t = 0;

				for (; bit_vect < bits; bit_vect += 8)
					t |= (readByte() << bit_vect);

				if (bits == 32) bit_vect = -1;
				else bit_vect = (1 << bits) - 1;

				t = t ^ (*(uint32_t*)fromF & bit_vect);
				if (field->bits < 0 && (t >> (bits - 1)) & 1)
					t |= ~bit_vect;

				if (print)
					std::cout << field->name << "{" << field->bits << "} = " << *(uint32_t*)toF << " ";
				*(uint32_t*)toF = t;
		}
	}

	void Msg::readBase64(unsigned char* outbuf, int len)
	{
		int databyte;
		int b64data;
		int k, shift;
		int i = 0;

		do
		{
			b64data = 0;
			for (k = 0, shift = 18; k < 4; ++k, shift -= 6)
			{
				databyte = readByte();
				if (databyte >= 'A' && databyte <= 'Z')
					databyte -= 'A';
				else if (databyte >= 'a' && databyte <= 'z') 
				{
					databyte -= 'a';
					databyte += 26;
				}
				else if (databyte >= '0' && databyte <= '9') 
				{
					databyte -= '0';
					databyte += 52;
				}
				else if (databyte == '+')
					databyte = 62;
				else if (databyte == '/')
					databyte = 63;
				else 
				{
					databyte = -1;
					break;
				}
				b64data |= (databyte << shift);
			}
			outbuf[i + 0] = (reinterpret_cast<char*>(&b64data))[2];
			outbuf[i + 1] = (reinterpret_cast<char*>(&b64data))[1];
			outbuf[i + 2] = (reinterpret_cast<char*>(&b64data))[0];
			i += 3;
		} 
		while (databyte != -1 && (i + 4) < len);
		outbuf[i] = '\0';
	}

	int Msg::getNumBitsRead()
	{
		return 8 * readcount - ((8 - bit) & 7);
	}

	void Msg::clearLastReferencedEntity()
	{
		lastRefEntity = -1;
	}

	// @TODO
	void Msg::readCommandString()
	{
		int seq = readInt();
		int index;
		std::string s = readString(0x400u);

		index = seq & 0x7F;
		//std::cout << "Server Command: " << index << " " << s << std::endl;
	}

	// @TODO
	int Msg::readClients(const int time, ClientSnapshotData* from, ClientSnapshotData* to)
	{
		while (!overflowed)
		{
			int newnum = readEntityIndex(GetMinBitCount(MAX_CLIENTS - 1));
			std::cout << "CS NN: " << newnum << std::endl;
			readDeltaClient(time, &from->cs[from->sn.num_clients], &to->cs[to->sn.num_clients], newnum);
			++to->sn.num_clients;
		}
		return to->sn.num_clients;
	}

	// @TODO
	int Msg::readEntities(const int time, ClientSnapshotData* from, ClientSnapshotData* to)
	{
		while (!overflowed)
		{
			int newnum = readEntityIndex(10);
			if (newnum == 1023 || newnum < 0 || newnum >= 1024)
				break;
			std::cout << "ES NN: " << newnum << std::endl;
			readDeltaEntity(time, &from->es[from->sn.num_entities], &to->es[to->sn.num_entities], newnum);
			++to->sn.num_entities;
		}
		return to->sn.num_entities;
	}

	int Msg::readLastChangedField(int totalFields)
	{
		int idealBits, lastChanged;
		idealBits = GetMinBitCount(totalFields);
		lastChanged = readBits(idealBits);
		return lastChanged;
	}

	// @TODO
	void Msg::readSnapshot(const std::vector<ClientSnapshotData>& snapshots, ClientSnapshotData& snap)
	{
		ClientSnapshotData* frame = new ClientSnapshotData{};
		ClientSnapshotData* from = new ClientSnapshotData{};

		if (snapshots.size() > 0) // if from exists
			std::memcpy(from, &snapshots.back(), sizeof(ClientSnapshotData));
		std::memcpy(frame, from, sizeof(ClientSnapshotData));

		int serverTime = readInt();
		unsigned char lastFrame = readByte();
		unsigned char snapFlags = readByte();

		// Read Player State
		readDeltaPlayerState(serverTime, &from->sn.ps, &frame->sn.ps, true);
		clearLastReferencedEntity();

		// Read Entities State
		/*readEntities(serverTime, from, frame);
		readEntityIndex(10);
		clearLastReferencedEntity();*/

		// Read Clients State
		//readClients(serverTime, from, frame);

		//int isZero = readBit();
		//std::cout << "IsZero: " << isZero << std::endl;
		//if (sv_padPackets->integer) // if server has packet padding
		//{
		//	for (int i = 0; i < sv_padPackets->integer; i++)
		//		readByte(); // svc_nop
		//}

		std::memcpy(&snap, frame, sizeof(ClientSnapshotData));
		delete from;
		delete frame;
	}

	int Msg::readDeltaEntity(const int time, entityState_t* from, entityState_t* to, int number)
	{
		int numFields = sizeof(entityStateFields) / sizeof(entityStateFields[0]);
		return readDeltaStruct(time, (unsigned char*)from, (unsigned char*)to, number,
			numFields, GetMinBitCount(MAX_CLIENTS - 1), entityStateFields);
	}

	int Msg::readDeltaClient(const int time, clientState_t* from, clientState_t* to, int number)
	{
		int numFields = sizeof(clientStateFields) / sizeof(clientStateFields[0]);
		return readDeltaStruct(time, (unsigned char*)from, (unsigned char*)to, number,
			numFields, GetMinBitCount(MAX_CLIENTS - 1), clientStateFields);
	}

	void Msg::readDeltaPlayerState(int time, playerState_t* from, playerState_t* to,
		bool predictedFieldsIgnoreXor)
	{
		int i, j;
		bool readOriginAndVel = readBit() > 0;
		int lastChangedField = readBits(GetMinBitCount(PLAYER_STATE_FIELDS_COUNT));

		netField_t* stateFields = playerStateFields;
		for (int i = 0; i < lastChangedField; ++i)
		{
			bool noXor = predictedFieldsIgnoreXor && readOriginAndVel && stateFields[i].changeHints == 3;
			readDeltaField(time, from, to, &stateFields[i], noXor, false);
		}

		for (i = lastChangedField; i < PLAYER_STATE_FIELDS_COUNT; ++i)
		{
			int offset = stateFields[i].offset;
			*(uint32_t*)&((unsigned char*)to)[offset] = *(uint32_t*)&((unsigned char*)from)[offset];
		}

		if (readBit())
		{
			int statsbits = readBits(5);
			for (i = 0; i < 3; i++)
			{
				if (statsbits & (1 << i))
					to->stats[i] = readShort();
			}
			if (statsbits & 8)
				to->stats[3] = readBits(6);
			if (statsbits & 0x10)
				to->stats[4] = readByte();
		}

		std::cout << std::endl;
		for (i = 0; i < 31; i++)
		{
			hudelem_t hud = to->hud.current[i];
			/*if (hud.value > 0)
				std::cout << "HUD Velocity: " << hud.value << std::endl;*/
		}

		// ammo stored
		if (readBit())
		{
			// check for any ammo change (0-63)
			for (j = 0; j < 4; j++)
			{
				if (readBit())
				{
					int bits = readShort();
					for (i = 0; i < 16; i++)
					{
						if (bits & (1 << i))
							to->ammo[i + (j * 16)] = readShort();
					}
				}
			}
		}

		// ammo in clip
		for (j = 0; j < 8; j++)
		{
			if (readBit())
			{
				int bits = readShort();
				for (i = 0; i < 16; i++)
				{
					if (bits & (1 << i))
						to->ammoclip[i + (j * 16)] = readShort();
				}
			}
		}

		int numObjective = sizeof(from->objective) / sizeof(from->objective[0]);
		if (readBit())
		{
			for (int fieldNum = 0; fieldNum < numObjective; ++fieldNum)
			{
				to->objective[fieldNum].state = static_cast<objectiveState_t>(readBits(3));
				readDeltaObjectiveFields(time, &from->objective[fieldNum], &to->objective[fieldNum]);
			}
		}
		if (readBit())
		{
			readDeltaHudElems(time, from->hud.archival, to->hud.archival, 31);
			readDeltaHudElems(time, from->hud.current, to->hud.current, 31);
		}

		if (readBit())
		{
			for (i = 0; i < 128; ++i)
				to->weaponmodels[i] = readByte();
		}
	}

	void Msg::readDeltaObjectiveFields(const int time, objective_t* from, objective_t* to)
	{
		if (readBit())
		{
			for (int i = 0; i < OBJECTIVE_FIELDS_COUNT; ++i)
				readDeltaField(time, from, to, &objectiveFields[i], false, false);
		}
		else
		{
			VectorCopy(from->origin, to->origin);
			to->icon = from->icon;
			to->entNum = from->entNum;
			to->teamNum = from->teamNum;
		}
	}

	void Msg::readDeltaHudElems(const int time, hudelem_t* from, hudelem_t* to, int count)
	{
		int alignY, alignX, inuse;
		unsigned int lc;
		int i, y;

		inuse = readBits(5);
		for (i = 0; i < inuse; ++i)
		{
			lc = readBits(6);
			for (y = 0; y <= lc; ++y)
				readDeltaField(time, &from[i], &to[i], &hudElemFields[y], false, false);

			for (; y < HUD_ELEM_FIELDS_COUNT; ++y)
			{
				int offset = hudElemFields[y].offset;
				((unsigned char*)&to[i])[offset] = ((unsigned char*)&from[i])[offset];
			}

			alignX = (from[i].alignOrg >> 2) & 3;
			alignY = from[i].alignOrg & 3;
			alignX = (to[i].alignOrg >> 2) & 3;
			alignY = to[i].alignOrg & 3;
		}
		while (inuse < count && to[inuse].type)
		{
			memset(&to[inuse], 0, sizeof(hudelem_t));
			++inuse;
		}
	}
}
