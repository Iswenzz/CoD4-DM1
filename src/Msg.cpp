#include "Msg.hpp"
#include "Huffman.hpp"
#include "NetFields.hpp"
#include "ClientSnapshotData.hpp"
#include "ClientGamestateData.hpp"
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

	Msg::Msg() : overflowed(false), readonly(false), splitBuffer(0), splitSize(0), readcount(0),
		bit(0), lastRefEntity(0), buffer(0), cursize(0), maxsize(0) { }
	Msg::~Msg() { }

	Msg::Msg(unsigned char* buf, int len, MSGCrypt mode)
	{
		Initialize(buf, len, mode);
	}

	void Msg::Initialize(int len)
	{
		cursize = len;
		maxsize = NETCHAN_UNSENTBUFFER_SIZE;
		buffer = std::vector<unsigned char>(len);
	}

	void Msg::Initialize(unsigned char* buf, int len, MSGCrypt mode)
	{
		if (mode == MSGCrypt::MSG_CRYPT_NONE)
		{
			buffer = std::vector<unsigned char>(len);
			cursize = buffer.size();
			maxsize = NETCHAN_MAXBUFFER_SIZE;
			std::memcpy(buffer.data(), buf, buffer.size());
		}
		else if (mode == MSGCrypt::MSG_CRYPT_HUFFMAN)
		{
			buffer = std::vector<unsigned char>(NETCHAN_MAXBUFFER_SIZE);
			cursize = Huffman::Decompress(buf, len, buffer.data(), buffer.size());
			maxsize = NETCHAN_MAXBUFFER_SIZE;
		}
		readcount = 0;
	}

	int Msg::ReadBit()
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
			bits = buffer[numBytes] >> oldbit7;
			bit++;
			return bits & 1;
		}
		bits = splitBuffer[numBytes - cursize] >> oldbit7;
		bit++;
		return bits & 1;
	}

	int Msg::ReadBits(int numBits)
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
					if (splitBuffer.empty())
						return 0;

					var = splitBuffer[(bit / 8) - cursize];
				}
				else
					var = buffer[bit / 8];

				retval |= ((var >> (bit & 7)) & 1) << i;
				bit++;
			}
		}
		return retval;
	}

	int Msg::ReadByte()
	{
		unsigned char* c;
		if (readcount + sizeof(unsigned char) > cursize)
		{
			overflowed = 1;
			return -1;
		}
		c = &buffer[readcount];

		readcount += sizeof(unsigned char);
		return *c;
	}

	int Msg::ReadShort()
	{
		signed short* c;
		if (readcount + sizeof(short) > cursize) 
		{
			overflowed = 1;
			return -1;
		}
		c = reinterpret_cast<short*>(&buffer[readcount]);

		readcount += sizeof(short);
		return *c;
	}

	int Msg::ReadInt()
	{
		int32_t* c;

		if (readcount + sizeof(int32_t) > cursize) 
		{
			overflowed = 1;
			return -1;
		}
		c = reinterpret_cast<int32_t*>(&buffer[readcount]);

		readcount += sizeof(int32_t);
		return *c;
	}

	int64_t Msg::ReadInt64()
	{
		int64_t* c;
		if (readcount + sizeof(int64_t) > cursize) 
		{
			overflowed = 1;
			return -1;
		}
		c = reinterpret_cast<int64_t*>(&buffer[readcount]);

		readcount += sizeof(int64_t);
		return *c;
	}

	float Msg::ReadFloat()
	{
		float* c;
		if (readcount + sizeof(float) > cursize) 
		{
			overflowed = 1;
			return -1;
		}
		c = reinterpret_cast<float*>(&buffer[readcount]);

		readcount += sizeof(float);
		return *c;
	}

	double Msg::ReadAngle16()
	{
		return SHORT2ANGLE((double)ReadShort());
	}

	int Msg::ReadEFlags(int oldFlags)
	{
		int bitChanged, value, i, b;
		if (ReadBit() == 1)
		{
			value = 0;
			for (i = 0; i < 24; i += 8)
			{
				b = ReadByte();
				value |= (b << i);
			}
		}
		else
		{
			bitChanged = ReadBits(5);
			value = oldFlags ^ (1 << bitChanged);
		}
		return value;
	}

	int Msg::ReadEntityIndex(int indexBits)
	{
		if (ReadBit())
			++lastRefEntity;
		else if (indexBits != 10 || ReadBit())
			lastRefEntity = ReadBits(indexBits);
		else
			lastRefEntity += ReadBits(4);
		return lastRefEntity;
	}

	int Msg::ReadDeltaGroundEntity()
	{
		int j, value;
		if (ReadBit() == 1) return 1022;
		if (ReadBit() == 1) return 0;

		value = ReadBits(2);
		for (j = 2; j < 10; j += 8)
			value |= ReadByte() << j;
		return value;
	}

	int Msg::ReadDeltaStruct(const int time, const void* from, void* to, unsigned int number, 
		int numFields, int indexBits, netField_t* stateFields)
	{
		if (ReadBit() == 1) return 1;
		*(uint32_t*)to = number;
		ReadDeltaFields(time, reinterpret_cast<const unsigned char*>(from), 
			reinterpret_cast<unsigned char*>(to), numFields, stateFields);
		return 0;
	}

	void Msg::ReadDeltaFields(const int time, const unsigned char* from, unsigned char* to, 
		int numFields, netField_t* stateFields)
	{
		int lc, i;
		bool print;

		if (!ReadBit())
		{
			std::memcpy(to, from, 4 * numFields + 4);
			return;
		}
		lc = ReadLastChangedField(numFields);

		if (lc > numFields)
		{
			overflowed = 1;
			return;
		}

		for (i = 0; i < lc; ++i)
			ReadDeltaField(time, from, to, &stateFields[i], false, true);
		for (i = lc; i < numFields; ++i)
		{
			int offset = stateFields[i].offset;
			*(uint32_t*)&to[offset] = *(uint32_t*)&from[offset];
		}
	}

	float Msg::ReadOriginFloat(int bits, float oldValue)
	{
		signed int coord;
		if (ReadBit())
		{
			float center[3]{ 0, 0, 0 };
			coord = (signed int)(center[bits != -92] + 0.5);
			return (double)((((signed int)oldValue - coord + 0x8000) ^ ReadBits(16)) + coord - 0x8000);
		}
		return (double)(ReadBits(7) - 64) + oldValue;
	}

	float Msg::ReadOriginZFloat(float oldValue)
	{
		signed int coord;
		if (ReadBit())
		{
			float center[3]{ 0, 0, 0 };
			coord = (signed int)(center[2] + 0.5);
			return (double)((((signed int)oldValue - coord + 0x8000) ^ ReadBits(16)) + coord - 0x8000);
		}
		return (double)(ReadBits(7) - 64) + oldValue;
	}

	std::string Msg::ReadString()
	{
		int l = 0, c;
		std::string bigstring;
		do 
		{
			c = ReadByte();      // use ReadByte so -1 is out of bounds
			if (c == -1 || c == 0)
				break;
			// translate all fmt spec to avoid crash bugs
			if (c == '%')
				c = '.';
			bigstring += static_cast<unsigned char>(c);
			l++;
		} 
		while (l < bigstring.size());
		return bigstring;
	}

	std::string Msg::ReadStringLine()
	{
		int	l = 0, c;
		std::string bigstring;
		do 
		{
			c = ReadByte();	// use ReadByte so -1 is out of bounds
			if (c == -1 || c == 0 || c == '\n')
				break;
			// translate all fmt spec to avoid crash bugs
			if (c == '%')
				c = '.';
			bigstring += static_cast<char>(c);
			l++;
		} 
		while (l < bigstring.size());
		return bigstring;
	}

	void Msg::ReadData(void* buffer, int len)
	{
		for (int i = 0; i < len; i++)
			reinterpret_cast<unsigned char*>(buffer)[i] = ReadByte();
	}

	// @TODO
	void Msg::ReadDeltaField(int time, const void* from, const void* to, const netField_t* field, 
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
			if (!ReadBit()) // No change
			{
				*(uint32_t*)toF = *(uint32_t*)fromF;
				return;
			}
		} 

		//Changed field
		bits = field->bits;
		if (!bits)
		{
			if (!ReadBit())
			{
				*(uint32_t*)toF = ReadBit() << 31;
				return;
			}
			if (!ReadBit())
			{
				b = ReadBits(5);
				v = ((32 * ReadByte() + b) ^ ((signed int)*(float*)fromF + 4096)) - 4096;
				*(float*)toF = (double)v;
				//if (print)
					//std::cout << field->name << "{" << field->bits << "} = " << v << std::endl;
				return;
			}
			l = ReadInt();
			*(uint32_t*)toF = l;
			*(uint32_t*)toF = l ^ *(uint32_t*)fromF;
			
			if (!print) return;
			f = *(float*)toF;
			//std::cout << field->name << "{" << field->bits << "} = " << f << std::endl;
			return;
		}

		// Command
		switch (field->bits)
		{
			case -89:
				if (!ReadBit())
				{
					b = ReadBits(5);
					l = ((32 * ReadByte() + b) ^ ((signed int)*(float*)fromF + 4096)) - 4096;
					*(float*)toF = (double)l;
					//if (print)
						//std::cout << field->name << "{" << field->bits << "} = " << l << std::endl;
					return;
				}
				l = ReadInt();
				*(uint32_t*)toF = l ^ *(uint32_t*)fromF;

				if (!print) return;
				f = *(float*)toF;
				//std::cout << field->name << "{" << field->bits << "} = " << f << std::endl;
				return;

			case -88:
				l = ReadInt();
				*(uint32_t*)toF = l ^ *(uint32_t*)fromF;

				if (!print) return;
				f = *(float*)toF;
				std::cout << field->name << "{" << field->bits << "} = " << f << std::endl;
				return;

			case -100:
				if (!ReadBit())
				{
					*(float*)toF = 0.0;
					return;
				}
				*(float*)toF = ReadAngle16();
				return;

			case -99:
				if (ReadBit())
				{
					if (!ReadBit())
					{
						b = ReadBits(4);
						v = ((16 * ReadByte() + b) ^ ((signed int)*(float*)fromF + 2048)) - 2048;
						*(float*)toF = (double)v;
						//if (print)
							//std::cout << field->name << "{" << field->bits << "} = " << (int)v << std::endl;
						return;
					}
					l = ReadInt();
					*(uint32_t*)toF = l ^ *(uint32_t*)fromF;
					
					if (!print) return;
					f = *(float*)toF;
					//std::cout << field->name << "{" << field->bits << "} = " << f << std::endl;
					return;
				}
				*(uint32_t*)toF = 0;
				return;

			case -98:
				*(uint32_t*)toF = ReadEFlags(*(uint32_t*)fromF);
				return;

			case -97:
				if (ReadBit())
					*(uint32_t*)toF = ReadInt();
				else
					*(uint32_t*)toF = time - ReadBits(8);
				return;

			case -96:
				*(uint32_t*)toF = ReadDeltaGroundEntity();
				return;

			case -95:
				*(uint32_t*)toF = 100 * ReadBits(7);
				return;

			case -94:
			case -93:
				*(uint32_t*)toF = ReadByte();
				return;

			case -92:
			case -91:
				f = ReadOriginFloat(bits, *(float*)fromF);
				*(float*)toF = f;
				//if (print)
					//std::cout << field->name << "{" << field->bits << "} = " << f << std::endl;
				return;

			case -90:
				f = ReadOriginZFloat(*(float*)fromF);
				*(float*)toF = f;

				//if (!print) return;
					//std::cout << field->name << "{" << field->bits << "} = " << f << std::endl;
				return;

			case -87:
				*(float*)toF = ReadAngle16();
				return;

			case -86:
				*(float*)toF = (double)ReadBits(5) / 10.0 + 1.399999976158142;
				return;

			case -85:
				if (ReadBit())
				{
					*(uint32_t*)toF = *(uint32_t*)fromF;
					toF[3] = (fromF[3] != 0) - 1;
					return;
				}
				if (!ReadBit())
				{
					toF[0] = ReadByte();
					toF[1] = ReadByte();
					toF[2] = ReadByte();
				}
				toF[3] = 8 * ReadBits(5);
				return;

			default:
				if (!ReadBit())
				{
					*(uint32_t*)toF = 0;
					return;
				}
				bits = abs(field->bits);
				bit_vect = bits & 7;

				if (bit_vect) t = ReadBits(bit_vect);
				else t = 0;

				for (; bit_vect < bits; bit_vect += 8)
					t |= (ReadByte() << bit_vect);

				if (bits == 32) bit_vect = -1;
				else bit_vect = (1 << bits) - 1;

				t = t ^ (*(uint32_t*)fromF & bit_vect);
				if (field->bits < 0 && (t >> (bits - 1)) & 1)
					t |= ~bit_vect;

				//if (print)
					//std::cout << field->name << "{" << field->bits << "} = " << *(uint32_t*)toF << std::endl;
				*(uint32_t*)toF = t;
		}
	}

	void Msg::ReadBase64(unsigned char* outbuf, int len)
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
				databyte = ReadByte();
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

	int Msg::GetNumBitsRead()
	{
		return 8 * readcount - ((8 - bit) & 7);
	}

	void Msg::ClearLastReferencedEntity()
	{
		lastRefEntity = -1;
	}

	// @TODO
	void Msg::ReadCommandString()
	{
		int seq = ReadInt();
		int index;
		std::string s = ReadString();

		index = seq & 0x7F;
		std::cout << "Server Command: " << index << " " << s << std::endl;
	}

	// @TODO
	int Msg::ReadClients(const int time, ClientSnapshotData* from, ClientSnapshotData* to)
	{
		while (!overflowed)
		{
			int newnum = ReadEntityIndex(GetMinBitCount(MAX_CLIENTS - 1));
			//std::cout << "CS NN: " << newnum << std::endl;
			ReadDeltaClient(time, &from->cs[from->sn.num_clients], &to->cs[to->sn.num_clients], newnum);
			++to->sn.num_clients;
		}
		return to->sn.num_clients;
	}

	// @TODO
	int Msg::ReadEntities(const int time, ClientSnapshotData* from, ClientSnapshotData* to)
	{
		while (!overflowed)
		{
			int newnum = ReadEntityIndex(10);
			if (newnum == 1023 || newnum < 0 || newnum >= 1024)
				break;
			//std::cout << "ES NN: " << newnum << std::endl;
			ReadDeltaEntity(time, &from->es[from->sn.num_entities], &to->es[to->sn.num_entities], newnum);
			++to->sn.num_entities;
		}
		return to->sn.num_entities;
	}

	int Msg::ReadLastChangedField(int totalFields)
	{
		int idealBits, lastChanged;
		idealBits = GetMinBitCount(totalFields);
		lastChanged = ReadBits(idealBits);
		return lastChanged;
	}

	// @TODO
	void Msg::ReadSnapshot(const std::vector<ClientSnapshotData>& Snapshots, ClientSnapshotData& snap)
	{
		ClientSnapshotData* frame = new ClientSnapshotData{};
		ClientSnapshotData* from = new ClientSnapshotData{};

		if (Snapshots.size() > 0) // if from exists
			std::memcpy(from, &Snapshots.back(), sizeof(ClientSnapshotData));
		std::memcpy(frame, from, sizeof(ClientSnapshotData));

		int serverTime = ReadInt();
		unsigned char lastFrame = ReadByte();
		unsigned char snapFlags = ReadByte();

		// Read Player State
		ReadDeltaPlayerState(serverTime, &from->sn.ps, &frame->sn.ps, true);
		ClearLastReferencedEntity();

		// Read Entities State
		/*ReadEntities(serverTime, from, frame);
		ReadEntityIndex(10);
		ClearLastReferencedEntity();*/

		// Read Clients State
		//ReadClients(serverTime, from, frame);

		//int isZero = ReadBit();
		//std::cout << "IsZero: " << isZero << std::endl;
		//if (sv_padPackets->integer) // if server has packet padding
		//{
		//	for (int i = 0; i < sv_padPackets->integer; i++)
		//		ReadByte(); // svc_nop
		//}

		std::memcpy(&snap, frame, sizeof(ClientSnapshotData));
		delete from;
		delete frame;
	}

	int Msg::ReadDeltaEntity(const int time, entityState_t* from, entityState_t* to, int number)
	{
		int numFields = sizeof(entityStateFields) / sizeof(entityStateFields[0]);
		return ReadDeltaStruct(time, (unsigned char*)from, (unsigned char*)to, number,
			numFields, GetMinBitCount(MAX_CLIENTS - 1), entityStateFields);
	}

	int Msg::ReadDeltaClient(const int time, clientState_t* from, clientState_t* to, int number)
	{
		int numFields = sizeof(clientStateFields) / sizeof(clientStateFields[0]);
		return ReadDeltaStruct(time, (unsigned char*)from, (unsigned char*)to, number,
			numFields, GetMinBitCount(MAX_CLIENTS - 1), clientStateFields);
	}

	void Msg::ReadDeltaPlayerState(int time, playerState_t* from, playerState_t* to,
		bool predictedFieldsIgnoreXor)
	{
		int i, j;
		bool readOriginAndVel = ReadBit() > 0;
		int lastChangedField = ReadBits(GetMinBitCount(PLAYER_STATE_FIELDS_COUNT));

		netField_t* stateFields = playerStateFields;
		for (int i = 0; i < lastChangedField; ++i)
		{
			bool noXor = predictedFieldsIgnoreXor && readOriginAndVel && stateFields[i].changeHints == 3;
			ReadDeltaField(time, from, to, &stateFields[i], noXor, true);
		}

		for (i = lastChangedField; i < PLAYER_STATE_FIELDS_COUNT; ++i)
		{
			int offset = stateFields[i].offset;
			*(uint32_t*)&((unsigned char*)to)[offset] = *(uint32_t*)&((unsigned char*)from)[offset];
		}

		if (ReadBit())
		{
			int statsbits = ReadBits(5);
			for (i = 0; i < 3; i++)
			{
				if (statsbits & (1 << i))
					to->stats[i] = ReadShort();
			}
			if (statsbits & 8)
				to->stats[3] = ReadBits(6);
			if (statsbits & 0x10)
				to->stats[4] = ReadByte();
		}

		for (i = 0; i < 31; i++)
		{
			hudelem_t hud = to->hud.current[i];
			/*if (hud.value > 0)
				std::cout << "HUD Velocity: " << hud.value << std::endl;*/
		}

		// ammo stored
		if (ReadBit())
		{
			// check for any ammo change (0-63)
			for (j = 0; j < 4; j++)
			{
				if (ReadBit())
				{
					int bits = ReadShort();
					for (i = 0; i < 16; i++)
					{
						if (bits & (1 << i))
							to->ammo[i + (j * 16)] = ReadShort();
					}
				}
			}
		}

		// ammo in clip
		for (j = 0; j < 8; j++)
		{
			if (ReadBit())
			{
				int bits = ReadShort();
				for (i = 0; i < 16; i++)
				{
					if (bits & (1 << i))
						to->ammoclip[i + (j * 16)] = ReadShort();
				}
			}
		}

		int numObjective = sizeof(from->objective) / sizeof(from->objective[0]);
		if (ReadBit())
		{
			for (int fieldNum = 0; fieldNum < numObjective; ++fieldNum)
			{
				to->objective[fieldNum].state = static_cast<objectiveState_t>(ReadBits(3));
				ReadDeltaObjectiveFields(time, &from->objective[fieldNum], &to->objective[fieldNum]);
			}
		}
		if (ReadBit())
		{
			ReadDeltaHudElems(time, from->hud.archival, to->hud.archival, 31);
			ReadDeltaHudElems(time, from->hud.current, to->hud.current, 31);
		}

		if (ReadBit())
		{
			for (i = 0; i < 128; ++i)
				to->weaponmodels[i] = ReadByte();
		}
	}

	void Msg::ReadGamestate()
	{
		int cmd = 0, newnum = -1, idx = -1; 
		int seq = ReadInt();
		std::cout << seq << std::endl;

		while (true)
		{
			cmd = ReadByte();
			std::cout << "gamestate cmd: " << cmd << std::endl;
			if (cmd == static_cast<int>(svc_ops_e::svc_EOF))
				break;

			if (cmd == static_cast<int>(svc_ops_e::svc_configstring))
			{
				short i = ReadShort();
				if (i < 0 || i >= MAX_CONFIGSTRINGS)
					break;

				while (i > 0)
				{
					if (ReadBit()) idx++;
					else idx = ReadBits(12);

					std::string s = ReadString();
					i--;

					std::cout << "configString: " << s << std::endl;
				}
			}
			else if (cmd == static_cast<int>(svc_ops_e::svc_configstring))
			{
				if (ReadBit())
					newnum++;
				else
				{
					if (!ReadBit())
					{
						int c = ReadBits(4);
						newnum += c;
					}
					else
						newnum = ReadBits(GENTITYNUM_BITS);
				}
				if (newnum < 0 || newnum >= MAX_GENTITIES)
					break;

				//ReadDeltaEntity() // @TODO
			}
			else 
				break;
		}
	}

	void Msg::ReadDeltaObjectiveFields(const int time, objective_t* from, objective_t* to)
	{
		if (ReadBit())
		{
			for (int i = 0; i < OBJECTIVE_FIELDS_COUNT; ++i)
				ReadDeltaField(time, from, to, &objectiveFields[i], false, false);
		}
		else
		{
			VectorCopy(from->origin, to->origin);
			to->icon = from->icon;
			to->entNum = from->entNum;
			to->teamNum = from->teamNum;
		}
	}

	void Msg::ReadDeltaHudElems(const int time, hudelem_t* from, hudelem_t* to, int count)
	{
		int alignY, alignX, inuse;
		unsigned int lc;
		int i, y;

		inuse = ReadBits(5);
		for (i = 0; i < inuse; ++i)
		{
			lc = ReadBits(6);
			for (y = 0; y <= lc; ++y)
				ReadDeltaField(time, &from[i], &to[i], &hudElemFields[y], false, false);

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
