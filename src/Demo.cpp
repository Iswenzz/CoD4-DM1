#include "Demo.hpp"

#include <iostream>
#include <iterator>

namespace Iswenzz
{
	Demo::Demo(std::string filepath)
	{
		Open(filepath);
	}

	Demo::~Demo()
	{
		Close();
	}

	void Demo::Open(std::string filepath)
	{
		if (IsDemoOpen)
			Close();
		IsDemoOpen = true;
		DemoFilePath = filepath;

		DemoFile.open(filepath, std::ios::binary);
		while (DemoFile.is_open())
		{
			MSGType msgType = { };
			DemoFile.read(reinterpret_cast<char*>(&msgType), sizeof(char));
			std::cout << "msg: " << static_cast<int>(msgType) << std::endl;

			switch (msgType)
			{
				case MSGType::MSG_SNAPSHOT:
				{
					ReadSnapshotHeader();
					ParseSnapshotHeader();
					break;
				}
				case MSGType::MSG_FRAME:
				{
					ReadArchiveHeader();	
					ParseArchiveHeader();
					break;
				}
			}
		}
	}

	void Demo::Close()
	{
		if (DemoFile.is_open())
			DemoFile.close();
		IsDemoOpen = false;
	}

	void Demo::ReadSnapshotHeader()
	{
		CurrentCompressedMsg = Msg{ };

		unsigned char slen = 0;
		int protocol = 0, messageLength = 0;
		int dummy[2] = { };

		// CoD4X
		//DemoFile.read(reinterpret_cast<char*>(&protocol), sizeof(int));
		//DemoFile.read(reinterpret_cast<char*>(&CurrentCompressedMsg.dummy), sizeof(int)); // datalen (-1 = demo ended)
		//DemoFile.read(reinterpret_cast<char*>(&dummy[1]), sizeof(int));
		//DemoFile.read(reinterpret_cast<char*>(&dummy[2]), sizeof(int));
		//DemoFile.read(reinterpret_cast<char*>(&slen), sizeof(unsigned char));
		//DemoFile.read(reinterpret_cast<char*>(&CurrentCompressedMsg.srvMsgSeq), sizeof(int));
		//DemoFile.read(reinterpret_cast<char*>(&CurrentCompressedMsg.maxsize), sizeof(int));

		// 1.7
		DemoFile.read(reinterpret_cast<char*>(&CurrentCompressedMsg.srvMsgSeq), sizeof(int));
		DemoFile.read(reinterpret_cast<char*>(&CurrentCompressedMsg.cursize), sizeof(int));
		DemoFile.read(reinterpret_cast<char*>(&CurrentCompressedMsg.dummy), sizeof(int));

		// Read the client message
		CurrentCompressedMsg.cursize -= sizeof(int);
		CurrentCompressedMsg.Initialize(CurrentCompressedMsg.cursize);
		DemoFile.read(reinterpret_cast<char*>(CurrentCompressedMsg.buffer.data()), 
			CurrentCompressedMsg.cursize);
	}

	void Demo::ParseSnapshotHeader()
	{
		svc_ops_e command = { };
		CurrentUncompressedMsg = Msg{ CurrentCompressedMsg.buffer.data(), 
			CurrentCompressedMsg.cursize, MSGCrypt::MSG_CRYPT_HUFFMAN };

		while (true)
		{
			if (CurrentUncompressedMsg.readcount > CurrentUncompressedMsg.cursize)
				break;

			// Read command
			command = static_cast<svc_ops_e>(CurrentUncompressedMsg.ReadByte());
			std::cout << "cmd: " << static_cast<int>(command) << std::endl;

			switch (command)
			{
				case svc_ops_e::svc_gamestate:
					ParseGamestate(CurrentUncompressedMsg);
					break;
				case svc_ops_e::svc_serverCommand:
					ParseCommandString(CurrentUncompressedMsg);
					break;
				case svc_ops_e::svc_download:
					//ParseDownload(CurrentUncompressedMsg);
					break;
				case svc_ops_e::svc_snapshot:
					ParseSnapshot(CurrentUncompressedMsg);
					break;
			}
			if (command == svc_ops_e::svc_EOF)
				break;
		}
	}

	void Demo::ReadArchiveHeader()
	{
		int len = 48;
		CurrentCompressedMsg = Msg{ };
		CurrentCompressedMsg.Initialize(len);

		DemoFile.read(reinterpret_cast<char*>(&CurrentCompressedMsg.srvMsgSeq), sizeof(int));
	}

	void Demo::ParseArchiveHeader()
	{
		archivedFrame_t frame = { 0 };
	
		DemoFile.read(reinterpret_cast<char*>(&frame.origin[0]), sizeof(float));
		DemoFile.read(reinterpret_cast<char*>(&frame.origin[1]), sizeof(float));
		DemoFile.read(reinterpret_cast<char*>(&frame.origin[2]), sizeof(float));
		DemoFile.read(reinterpret_cast<char*>(&frame.velocity[0]), sizeof(float));
		DemoFile.read(reinterpret_cast<char*>(&frame.velocity[1]), sizeof(float));
		DemoFile.read(reinterpret_cast<char*>(&frame.velocity[2]), sizeof(float));
		DemoFile.read(reinterpret_cast<char*>(&frame.movementDir), sizeof(int));
		DemoFile.read(reinterpret_cast<char*>(&frame.bobCycle), sizeof(int));
		DemoFile.read(reinterpret_cast<char*>(&frame.commandTime), sizeof(int));
		DemoFile.read(reinterpret_cast<char*>(&frame.angles[0]), sizeof(float));
		DemoFile.read(reinterpret_cast<char*>(&frame.angles[1]), sizeof(float));
		DemoFile.read(reinterpret_cast<char*>(&frame.angles[2]), sizeof(float));

		if (!StartFrameTime)
		{
			StartFrameTime = frame.commandTime;
			CurrentFrameTime = StartFrameTime;
		}
		else if (frame.commandTime > CurrentFrameTime)
			CurrentFrameTime = frame.commandTime;

		LastFrameSrvMsgSeq = CurrentCompressedMsg.srvMsgSeq;
		std::memcpy(&Frames[LastFrameSrvMsgSeq & MAX_FRAMES], &frame, sizeof(archivedFrame_t));
	}

	void Demo::ParseGamestate(Msg& msg)
	{
		svc_ops_e command = { };
		int newnum = -1, idx = -1;

		ServerCommandSequence = msg.ReadInt();
		MatchInProgress = true;

		while (true)
		{
			command = static_cast<svc_ops_e>(msg.ReadByte());
			std::cout << "gamestate cmd: " << static_cast<int>(command) << std::endl;
			if (command == svc_ops_e::svc_EOF)
				break;

			if (command == svc_ops_e::svc_configstring)
			{
				short i = msg.ReadShort();
				if (i < 0 || i >= MAX_CONFIGSTRINGS)
					break;

				while (i > 0)
				{
					if (msg.ReadBit()) idx++;
					else idx = msg.ReadBits(12);

					std::string s = msg.ReadString();
					std::cout << "configString: " << s << std::endl;
					
					if (!(idx < 0 || idx >= MAX_CONFIGSTRINGS))
					{
						if (s.size() != std::string::npos)
						{
							ConfigStrings[idx] = s;
							ValidConfigStrings[idx] = 1;
						}
					}
					i--;
				}
			}
			else if (command == svc_ops_e::svc_baseline)
			{
				if (msg.ReadBit())
					newnum++;
				else
				{
					if (!msg.ReadBit())
					{
						int c = msg.ReadBits(4);
						newnum += c;
					}
					else
						newnum = msg.ReadBits(GENTITYNUM_BITS);
				}
				if (newnum < 0 || newnum >= MAX_GENTITIES)
					break;

				entityState_t es = EntityBaselines[newnum];
				ReadDeltaEntity(msg, 0, &NullEntityState, &es, newnum);
				ActiveBaselines[newnum] = 1;
			}
			else
				break;
		}
	}

	void Demo::ParseCommandString(Msg& msg)
	{
		int seq = msg.ReadInt();
		int index;
		std::string s = msg.ReadString();

		index = seq & 0x7F;
		std::cout << "Server Command: " << index << " " << s << std::endl;
	}

	// @TODO
	void Demo::ParseSnapshot(Msg& msg)
	{
		clientSnapshot_t old = { 0 };
		CurrentSnapshot = { 0 };
		CurrentSnapshot.serverCommandNum = ServCmdSequence;
		CurrentSnapshot.serverTime = msg.ReadInt();
		CurrentSnapshot.messageNum = msg.srvMsgSeq;

		unsigned char deltaNum = msg.ReadByte();
		CurrentSnapshot.deltaNum = !deltaNum ? -1 : CurrentSnapshot.messageNum - deltaNum;
		CurrentSnapshot.snapFlags = msg.ReadByte();

		if (CurrentSnapshot.deltaNum <= 0)
			CurrentSnapshot.valid = true;
		else
		{
			old = Snapshots[CurrentSnapshot.deltaNum & PACKET_MASK];
			CurrentSnapshot.valid = old.valid;
		}

		if (old.valid)
			ReadDeltaPlayerState(msg, CurrentSnapshot.serverTime, &old.ps, &CurrentSnapshot.ps, true);
		else
			ReadDeltaPlayerState(msg, CurrentSnapshot.serverTime, &NullPlayerState, &CurrentSnapshot.ps, true);

		// Read Player State
		//ReadDeltaPlayerState(msg, serverTime, &from->sn.ps, &frame->sn.ps, true);

		// Read Entities State
		/*ReadEntities(msg, serverTime, from, frame);
		ReadEntityIndex(msg, 10);*/

		// Read Clients State
		//ReadClients(msg, serverTime, from, frame);

		//int isZero = msg.ReadBit();
		//std::cout << "IsZero: " << isZero << std::endl;
		//if (sv_padPackets->integer) // if server has packet padding
		//{
		//	for (int i = 0; i < sv_padPackets->integer; i++)
		//		msg.ReadByte(); // svc_nop
		//}
	}

	int Demo::ReadDeltaGroundEntity(Msg& msg)
	{
		int j, value;
		if (msg.ReadBit() == 1) return 1022;
		if (msg.ReadBit() == 1) return 0;

		value = msg.ReadBits(2);
		for (j = 2; j < 10; j += 8)
			value |= msg.ReadByte() << j;
		return value;
	}

	int Demo::ReadDeltaStruct(Msg& msg, const int time, const void* from, void* to, unsigned int number,
		int numFields, int indexBits, netField_t* stateFields)
	{
		if (msg.ReadBit() == 1) return 1;
		*(uint32_t*)to = number;
		ReadDeltaFields(msg, time, reinterpret_cast<const unsigned char*>(from),
			reinterpret_cast<unsigned char*>(to), numFields, stateFields);
		return 0;
	}

	void Demo::ReadDeltaFields(Msg& msg, const int time, const unsigned char* from, unsigned char* to,
		int numFields, netField_t* stateFields)
	{
		int lc, i;

		if (!msg.ReadBit())
		{
			std::memcpy(to, from, 4 * numFields + 4);
			return;
		}
		lc = ReadLastChangedField(msg, numFields);

		if (lc > numFields)
		{
			msg.overflowed = 1;
			return;
		}

		for (i = 0; i < lc; ++i)
			ReadDeltaField(msg, time, from, to, &stateFields[i], false, true);
		for (i = lc; i < numFields; ++i)
		{
			int offset = stateFields[i].offset;
			*(uint32_t*)&to[offset] = *(uint32_t*)&from[offset];
		}
	}

	// @TODO
	void Demo::ReadDeltaField(Msg& msg, int time, const void* from, const void* to, const netField_t* field,
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
			if (!msg.ReadBit()) // No change
			{
				*(uint32_t*)toF = *(uint32_t*)fromF;
				return;
			}
		}

		//Changed field
		bits = field->bits;
		if (!bits)
		{
			if (!msg.ReadBit())
			{
				*(uint32_t*)toF = msg.ReadBit() << 31;
				return;
			}
			if (!msg.ReadBit())
			{
				b = msg.ReadBits(5);
				v = ((32 * msg.ReadByte() + b) ^ ((signed int)*(float*)fromF + 4096)) - 4096;
				*(float*)toF = (double)v;
				//if (print)
					//std::cout << field->name << "{" << field->bits << "} = " << v << std::endl;
				return;
			}
			l = msg.ReadInt();
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
			if (!msg.ReadBit())
			{
				b = msg.ReadBits(5);
				l = ((32 * msg.ReadByte() + b) ^ ((signed int)*(float*)fromF + 4096)) - 4096;
				*(float*)toF = (double)l;
				//if (print)
					//std::cout << field->name << "{" << field->bits << "} = " << l << std::endl;
				return;
			}
			l = msg.ReadInt();
			*(uint32_t*)toF = l ^ *(uint32_t*)fromF;

			if (!print) return;
			f = *(float*)toF;
			//std::cout << field->name << "{" << field->bits << "} = " << f << std::endl;
			return;

		case -88:
			l = msg.ReadInt();
			*(uint32_t*)toF = l ^ *(uint32_t*)fromF;

			if (!print) return;
			f = *(float*)toF;
			std::cout << field->name << "{" << field->bits << "} = " << f << std::endl;
			return;

		case -100:
			if (!msg.ReadBit())
			{
				*(float*)toF = 0.0;
				return;
			}
			*(float*)toF = msg.ReadAngle16();
			return;

		case -99:
			if (msg.ReadBit())
			{
				if (!msg.ReadBit())
				{
					b = msg.ReadBits(4);
					v = ((16 * msg.ReadByte() + b) ^ ((signed int)*(float*)fromF + 2048)) - 2048;
					*(float*)toF = (double)v;
					//if (print)
						//std::cout << field->name << "{" << field->bits << "} = " << (int)v << std::endl;
					return;
				}
				l = msg.ReadInt();
				*(uint32_t*)toF = l ^ *(uint32_t*)fromF;

				if (!print) return;
				f = *(float*)toF;
				//std::cout << field->name << "{" << field->bits << "} = " << f << std::endl;
				return;
			}
			*(uint32_t*)toF = 0;
			return;

		case -98:
			*(uint32_t*)toF = msg.ReadEFlags(*(uint32_t*)fromF);
			return;

		case -97:
			if (msg.ReadBit())
				*(uint32_t*)toF = msg.ReadInt();
			else
				*(uint32_t*)toF = time - msg.ReadBits(8);
			return;

		case -96:
			*(uint32_t*)toF = ReadDeltaGroundEntity(msg);
			return;

		case -95:
			*(uint32_t*)toF = 100 * msg.ReadBits(7);
			return;

		case -94:
		case -93:
			*(uint32_t*)toF = msg.ReadByte();
			return;

		case -92:
		case -91:
			f = msg.ReadOriginFloat(bits, *(float*)fromF);
			*(float*)toF = f;
			//if (print)
				//std::cout << field->name << "{" << field->bits << "} = " << f << std::endl;
			return;

		case -90:
			f = msg.ReadOriginZFloat(*(float*)fromF);
			*(float*)toF = f;

			//if (!print) return;
				//std::cout << field->name << "{" << field->bits << "} = " << f << std::endl;
			return;

		case -87:
			*(float*)toF = msg.ReadAngle16();
			return;

		case -86:
			*(float*)toF = (double)msg.ReadBits(5) / 10.0 + 1.399999976158142;
			return;

		case -85:
			if (msg.ReadBit())
			{
				*(uint32_t*)toF = *(uint32_t*)fromF;
				toF[3] = (fromF[3] != 0) - 1;
				return;
			}
			if (!msg.ReadBit())
			{
				toF[0] = msg.ReadByte();
				toF[1] = msg.ReadByte();
				toF[2] = msg.ReadByte();
			}
			toF[3] = 8 * msg.ReadBits(5);
			return;

		default:
			if (!msg.ReadBit())
			{
				*(uint32_t*)toF = 0;
				return;
			}
			bits = abs(field->bits);
			bit_vect = bits & 7;

			if (bit_vect) t = msg.ReadBits(bit_vect);
			else t = 0;

			for (; bit_vect < bits; bit_vect += 8)
				t |= (msg.ReadByte() << bit_vect);

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

	// @TODO
	int Demo::ReadClients(Msg& msg, const int time, clientState_t* from, clientState_t* to)
	{
		//while (!overflowed)
		//{
		//	int newnum = msg.ReadEntityIndex(GetMinBitCount(MAX_CLIENTS - 1));
		//	//std::cout << "CS NN: " << newnum << std::endl;
		//	ReadDeltaClient(time, from, to, newnum);
		//	++to->sn.num_clients;
		//}
		//return to->sn.num_clients;
		return -1;
	}

	// @TODO
	int Demo::ReadEntities(Msg& msg, const int time, entityState_t* from, entityState_t* to)
	{
		//while (!overflowed)
		//{
		//	int newnum = msg.ReadEntityIndex(10);
		//	if (newnum == 1023 || newnum < 0 || newnum >= 1024)
		//		break;
		//	//std::cout << "ES NN: " << newnum << std::endl;
		//	ReadDeltaEntity(time, from, to, newnum);
		//	++to->num_entities;
		//}
		//return to->num_entities;
		return -1;
	}

	int Demo::ReadLastChangedField(Msg& msg, int totalFields)
	{
		int idealBits, lastChanged;
		idealBits = GetMinBitCount(totalFields);
		lastChanged = msg.ReadBits(idealBits);
		return lastChanged;
	}

	int Demo::ReadDeltaEntity(Msg& msg, const int time, entityState_t* from, entityState_t* to, int number)
	{
		int numFields = sizeof(entityStateFields) / sizeof(entityStateFields[0]);
		return ReadDeltaStruct(msg, time, (unsigned char*)from, (unsigned char*)to, number,
			numFields, GetMinBitCount(MAX_CLIENTS - 1), entityStateFields);
	}

	int Demo::ReadDeltaClient(Msg& msg, const int time, clientState_t* from, clientState_t* to, int number)
	{
		int numFields = sizeof(clientStateFields) / sizeof(clientStateFields[0]);
		return ReadDeltaStruct(msg, time, (unsigned char*)from, (unsigned char*)to, number,
			numFields, GetMinBitCount(MAX_CLIENTS - 1), clientStateFields);
	}

	void Demo::ReadDeltaPlayerState(Msg& msg, int time, playerState_t* from, playerState_t* to,
		bool predictedFieldsIgnoreXor)
	{
		int i, j;
		bool readOriginAndVel = msg.ReadBit() > 0;
		int lastChangedField = msg.ReadBits(GetMinBitCount(PLAYER_STATE_FIELDS_COUNT));

		netField_t* stateFields = playerStateFields;
		for (int i = 0; i < lastChangedField; ++i)
		{
			bool noXor = predictedFieldsIgnoreXor && readOriginAndVel && stateFields[i].changeHints == 3;
			ReadDeltaField(msg, time, from, to, &stateFields[i], noXor, true);
		}

		for (i = lastChangedField; i < PLAYER_STATE_FIELDS_COUNT; ++i)
		{
			int offset = stateFields[i].offset;
			*(uint32_t*)&((unsigned char*)to)[offset] = *(uint32_t*)&((unsigned char*)from)[offset];
		}

		if (msg.ReadBit())
		{
			int statsbits = msg.ReadBits(5);
			for (i = 0; i < 3; i++)
			{
				if (statsbits & (1 << i))
					to->stats[i] = msg.ReadShort();
			}
			if (statsbits & 8)
				to->stats[3] = msg.ReadBits(6);
			if (statsbits & 0x10)
				to->stats[4] = msg.ReadByte();
		}

		for (i = 0; i < 31; i++)
		{
			hudelem_t hud = to->hud.current[i];
			/*if (hud.value > 0)
				std::cout << "HUD Velocity: " << hud.value << std::endl;*/
		}

		// ammo stored
		if (msg.ReadBit())
		{
			// check for any ammo change (0-63)
			for (j = 0; j < 4; j++)
			{
				if (msg.ReadBit())
				{
					int bits = msg.ReadShort();
					for (i = 0; i < 16; i++)
					{
						if (bits & (1 << i))
							to->ammo[i + (j * 16)] = msg.ReadShort();
					}
				}
			}
		}

		// ammo in clip
		for (j = 0; j < 8; j++)
		{
			if (msg.ReadBit())
			{
				int bits = msg.ReadShort();
				for (i = 0; i < 16; i++)
				{
					if (bits & (1 << i))
						to->ammoclip[i + (j * 16)] = msg.ReadShort();
				}
			}
		}

		int numObjective = sizeof(from->objective) / sizeof(from->objective[0]);
		if (msg.ReadBit())
		{
			for (int fieldNum = 0; fieldNum < numObjective; ++fieldNum)
			{
				to->objective[fieldNum].state = static_cast<objectiveState_t>(msg.ReadBits(3));
				ReadDeltaObjectiveFields(msg, time, &from->objective[fieldNum], &to->objective[fieldNum]);
			}
		}
		if (msg.ReadBit())
		{
			ReadDeltaHudElems(msg, time, from->hud.archival, to->hud.archival, 31);
			ReadDeltaHudElems(msg, time, from->hud.current, to->hud.current, 31);
		}

		if (msg.ReadBit())
		{
			for (i = 0; i < 128; ++i)
				to->weaponmodels[i] = msg.ReadByte();
		}
	}

	void Demo::ReadDeltaObjectiveFields(Msg& msg, const int time, objective_t* from, objective_t* to)
	{
		if (msg.ReadBit())
		{
			for (int i = 0; i < OBJECTIVE_FIELDS_COUNT; ++i)
				ReadDeltaField(msg, time, from, to, &objectiveFields[i], false, false);
		}
		else
		{
			VectorCopy(from->origin, to->origin);
			to->icon = from->icon;
			to->entNum = from->entNum;
			to->teamNum = from->teamNum;
		}
	}

	void Demo::ReadDeltaHudElems(Msg& msg, const int time, hudelem_t* from, hudelem_t* to, int count)
	{
		int alignY, alignX, inuse;
		unsigned int lc;
		int i, y;

		inuse = msg.ReadBits(5);
		for (i = 0; i < inuse; ++i)
		{
			lc = msg.ReadBits(6);
			for (y = 0; y <= lc; ++y)
				ReadDeltaField(msg, time, &from[i], &to[i], &hudElemFields[y], false, false);

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
			std::memset(&to[inuse], 0, sizeof(hudelem_t));
			++inuse;
		}
	}

	// @TODO
	int Demo::ReadEntityIndex(Msg &msg, int indexBits)
	{
		/*if (msg.ReadBit())
			++lastRefEntity;
		else if (indexBits != 10 || msg.ReadBit())
			lastRefEntity = msg.ReadBits(indexBits);
		else
			lastRefEntity += msg.ReadBits(4);
		return lastRefEntity;*/
		return -1;
	}
}
