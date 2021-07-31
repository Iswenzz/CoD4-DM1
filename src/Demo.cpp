#include "Demo.hpp"

#include <iostream>
#include <iterator>

namespace Iswenzz
{
	Demo::Demo(std::string filepath) : Filepath(filepath), Verbose(false)
	{
		Open(filepath);
	}

	Demo::Demo(std::string filepath, bool verbose) : Filepath(filepath), Verbose(verbose)
	{
		Open(filepath);
	}

	Demo::~Demo()
	{
		Close();
	}

	void Demo::Open(std::string filepath)
	{
		if (IsOpen)
			Close();
		IsOpen = true;
		Filepath = filepath;

		DemoFile.open(filepath, std::ios::binary);
	}

	void Demo::Parse()
	{
		while (Next());
	}

	bool Demo::Next()
	{
		if (DemoFile.is_open() && CurrentCompressedMsg.srvMsgSeq != -1)
		{
			MSGType msgType = { };
			DemoFile.read(reinterpret_cast<char*>(&msgType), sizeof(char));
			VerboseLog("msg: " << static_cast<int>(msgType) << std::endl);

			switch (msgType)
			{
			case MSGType::MSG_SNAPSHOT:
				ReadMessage();
				break;
			case MSGType::MSG_FRAME:
				ReadArchive();
				break;
			case MSGType::MSG_PROTOCOL:
				ReadProtocol();
				break;
			case MSGType::MSG_RELIABLE:
				std::cout << "reliable msg" << std::endl;
				break;
			}
			return true;
		}
		return false;
	}

	void Demo::Close()
	{ 
		if (DemoFile.is_open())
			DemoFile.close();
		IsOpen = false;
	}

	void Demo::ReadMessage()
	{
		CurrentCompressedMsg = Msg{ Protocol };

		unsigned char slen = 0;
		int protocol = 0, messageLength = 0;
		int dummy[2] = { };

		DemoFile.read(reinterpret_cast<char*>(&CurrentCompressedMsg.srvMsgSeq), sizeof(int));
		DemoFile.read(reinterpret_cast<char*>(&CurrentCompressedMsg.cursize), sizeof(int));
		DemoFile.read(reinterpret_cast<char*>(&CurrentCompressedMsg.dummy), sizeof(int));

		// EOF
		if (CurrentCompressedMsg.srvMsgSeq == -1)
			return;

		// Read the client message
		CurrentCompressedMsg.cursize -= sizeof(int);
		CurrentCompressedMsg.Initialize(CurrentCompressedMsg.cursize);
		DemoFile.read(reinterpret_cast<char*>(CurrentCompressedMsg.buffer.data()), 
			CurrentCompressedMsg.cursize);

		// Parse message
		svc_ops_e command = { };
		CurrentUncompressedMsg = Msg{ CurrentCompressedMsg, MSGCrypt::MSG_CRYPT_HUFFMAN };
		while (true)
		{
			if (CurrentUncompressedMsg.readcount > CurrentUncompressedMsg.cursize)
				break;

			command = static_cast<svc_ops_e>(CurrentUncompressedMsg.ReadByte());
			VerboseLog("cmd: " << static_cast<int>(command) << std::endl);
			if (command == svc_ops_e::svc_EOF)
				break;

			switch (command)
			{
			case svc_ops_e::svc_gamestate:
				if (Protocol == COD4_PROTOCOL)
					ParseGamestate(CurrentUncompressedMsg);
				else
					ParseGamestateX(CurrentUncompressedMsg);
				break;
			case svc_ops_e::svc_serverCommand:
				ParseCommandString(CurrentUncompressedMsg);
				break;
			case svc_ops_e::svc_download:
				std::cout << "download" << std::endl;
				break;
			case svc_ops_e::svc_snapshot:
				ParseSnapshot(CurrentUncompressedMsg);
				break;
			case svc_ops_e::svc_configstring:
				std::cout << "configstring" << std::endl;
				break;
			default:
				return;
			}
		}
	}

	void Demo::ReadArchive()
	{
		int len = 48;
		archivedFrame_t frame = { 0 };

		CurrentCompressedMsg = Msg{ Protocol };
		CurrentCompressedMsg.Initialize(len);

		DemoFile.read(reinterpret_cast<char*>(&CurrentCompressedMsg.srvMsgSeq), sizeof(int));
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
		std::memcpy(&Frames[LastFrameSrvMsgSeq & MAX_FRAMES - 1], &frame, sizeof(archivedFrame_t));
	}

	void Demo::ReadProtocol()
	{
		int legacyEnd = 0;
		uint64_t reserved = 0;

		DemoFile.read(reinterpret_cast<char*>(&Protocol), sizeof(uint32_t));
		DemoFile.read(reinterpret_cast<char*>(&legacyEnd), sizeof(int));
		DemoFile.read(reinterpret_cast<char*>(&reserved), sizeof(uint64_t));

		std::cout << "Protocol: " << Protocol << std::endl;
	}

	void Demo::ParseGamestate(Msg& msg)
	{
		svc_ops_e command = { };
		int newnum = -1, idx = -1, currIndex = 0;

		msg.ClearLastReferencedEntity();
		ServerCommandSequence = msg.ReadInt();
		MatchInProgress = true;

		while (true)
		{
			command = static_cast<svc_ops_e>(msg.ReadByte());
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
					
					if (!(idx < 0 || idx >= MAX_CONFIGSTRINGS))
					{
						if (s.size() > 0)
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
				newnum = ReadEntityIndex(msg, GENTITYNUM_BITS);

				entityState_t *es = &EntityBaselines[newnum];
				NullEntityState = { 0 };

				ReadDeltaEntity(msg, 0, &NullEntityState, es, newnum);
				ActiveBaselines[newnum] = 1;
			}
			else
				break;
			currIndex++;
		}

		int clientNum = msg.ReadInt();
		if (clientNum >= 64 || clientNum < 0)
			return;
		int checksumFeed = msg.ReadInt();
	}

	void Demo::ParseGamestateX(Msg& msg)
	{
		svc_ops_e command = { };
		int newnum = -1, clientnum = -1, idx = -1, currIndex = 0;

		msg.ClearLastReferencedEntity();
		ServerCommandSequence = msg.ReadInt();
		MatchInProgress = true;

		while (true)
		{
			command = static_cast<svc_ops_e>(msg.ReadByte());
			if (command == svc_ops_e::svc_EOF)
				break;

			if (command == svc_ops_e::svc_configstring)
			{
				int i = msg.ReadInt();
				if (i < 0 || i >= 2 * MAX_CONFIGSTRINGS)
					break;

				while (i > 0)
				{
					idx = msg.ReadInt();

					std::string s = msg.ReadString();

					if (!(idx < 0 || idx >= 2 * MAX_CONFIGSTRINGS))
					{
						if (s.size() > 0)
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
				newnum = ReadEntityIndex(msg, GENTITYNUM_BITS);

				entityState_t* es = &EntityBaselines[newnum];
				NullEntityState = { 0 };

				ReadDeltaEntity(msg, 0, &NullEntityState, es, newnum);
				ActiveBaselines[newnum] = 1;
			}
			else if (command == svc_ops_e::svc_configclient)
			{
				clientnum = msg.ReadByte();
				if (clientnum >= MAX_CLIENTS)
					break;
	
				ClientNames[clientnum].netname = msg.ReadString();
				ClientNames[clientnum].clantag = msg.ReadString();
			}
			else
				break;
			currIndex++;
		}

		int serverConfigDataSequence = msg.ReadInt();
		int clientNum = msg.ReadInt();
		if (clientNum >= 64 || clientNum < 0)
			return;
		int checksumFeed = msg.ReadInt();

		if (Protocol != COD4X_FALLBACK_PROTOCOL)
		{
			int dbchecksumFeed = msg.ReadInt();
			//DB_SetPureChecksumFeed(dbchecksumFeed);
		}
	}

	void Demo::ParseCommandString(Msg& msg)
	{
		int seq = msg.ReadInt();
		int index;
		std::string s = msg.ReadString();

		index = seq & 0x7F;
		VerboseLog("server command[" << index << "]: " << s << std::endl);
	}

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

		// Integrity checks
		if (CurrentSnapshot.deltaNum > 0) 
		{
			old = Snapshots[CurrentSnapshot.deltaNum & PACKET_MASK];
			if (!old.valid) 
			{
				msg.Discard();
				return;
			}
			if (Snapshots[CurrentSnapshot.deltaNum & PACKET_MASK].messageNum != CurrentSnapshot.deltaNum) 
			{
				msg.Discard();
				return;
			}
			if (ParseEntitiesNum - Snapshots[CurrentSnapshot.deltaNum & PACKET_MASK].parseEntitiesNum > 1920) 
			{
				msg.Discard();
				return;
			}
			if (ParseClientsNum - Snapshots[CurrentSnapshot.deltaNum & PACKET_MASK].parseClientsNum > 1920) 
			{
				msg.Discard();
				return;
			}
		}
		CurrentSnapshot.valid = true;

		if (old.valid)
			ReadDeltaPlayerState(msg, CurrentSnapshot.serverTime, &old.ps, &CurrentSnapshot.ps, true);
		else
			ReadDeltaPlayerState(msg, CurrentSnapshot.serverTime, &NullPlayerState, &CurrentSnapshot.ps, true);
		msg.ClearLastReferencedEntity();

		// Entities State
		ParsePacketEntities(msg, CurrentSnapshot.serverTime, &old, &CurrentSnapshot);
		msg.ClearLastReferencedEntity();

		// Clients State
		ParsePacketClients(msg, CurrentSnapshot.serverTime, &old, &CurrentSnapshot);

		if (msg.overflowed)
			return;

		/* Clear the valid flags of any snapshots between the last received and this one,
			so if there was a dropped packet it won't look like something valid to delta from next time
			we wrap around in the buffer. */
		int oldMessageNum = SnapMessageNum + 1;
		if (CurrentSnapshot.messageNum - oldMessageNum >= PACKET_BACKUP)
			oldMessageNum = CurrentSnapshot.messageNum - (PACKET_BACKUP - 1);
		while (oldMessageNum < CurrentSnapshot.messageNum)
		{
			Snapshots[oldMessageNum & PACKET_MASK].valid = false;
			oldMessageNum++;
		}
		SnapMessageNum = CurrentSnapshot.messageNum;
		CurrentSnapshot.ping = 999;

		// Save the snapshot in the backup array for later delta comparisons
		std::memcpy(&Snapshots[SnapMessageNum & PACKET_MASK], &CurrentSnapshot, sizeof(clientSnapshot_t));
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

	bool Demo::ReadDeltaStruct(Msg& msg, const int time, const void* from, void* to, unsigned int number,
		int numFields, int indexBits, netField_t* stateFields)
	{
		if (msg.ReadBit() == 1)
			return true;
		*(uint32_t*)to = number;
		ReadDeltaFields(msg, time, reinterpret_cast<const unsigned char*>(from),
			reinterpret_cast<unsigned char*>(to), numFields, stateFields);
		return false;
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

		if (numFields == ENTITY_STATE_FIELDS_COUNT)
			lc = ReadLastChangedField(msg, 0x3D); // The game parse entities using 0x3D instead of 0x3B
		else
			lc = ReadLastChangedField(msg, numFields);

		if (lc > numFields)
		{
			msg.overflowed = 1;
			return;
		}
		ReadDeltaField(msg, time, from, to, &stateFields[0], false, false);
		
		// Get the right field list from the eType value
		if (std::string{ stateFields[0].name } == "eType")
		{
			int entityFieldOffset = *reinterpret_cast<int*>(reinterpret_cast<int>(to) + stateFields[0].offset);
			if (entityFieldOffset > NET_FIELDS_COUNT - 1)
				entityFieldOffset = NET_FIELDS_COUNT - 1;

			netFieldList_t fieldList = netFieldList[entityFieldOffset];
			stateFields = fieldList.field;
			numFields = fieldList.numFields;
		}

		for (i = 1; i < lc; ++i)
			ReadDeltaField(msg, time, from, to, &stateFields[i], false, false);
		for (i = lc; i < numFields; ++i)
		{
			int offset = stateFields[i].offset;
			*(uint32_t*)&to[offset] = *(uint32_t*)&from[offset];
		}
	}

	void Demo::ReadDeltaField(Msg& msg, int time, const void* from, const void* to, const netField_t* field,
		bool noXor, bool print)
	{
		unsigned char* fromF;
		unsigned char* toF;
		int bits, b, bit_vect, v, zeroV = 0;
		uint32_t l;
		double f = 0;
		signed int t;

		if (noXor && field->changeHints == 3)
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
		VerboseLog("bits: " << field->bits << std::endl);
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
				LogIf(print, field->name << "{" << field->bits << "} = " << v << std::endl);
				return;
			}
			l = msg.ReadInt();
			*(uint32_t*)toF = l;
			*(uint32_t*)toF = l ^ *(uint32_t*)fromF;

			f = *(float*)toF;
			LogIf(print, field->name << "{" << field->bits << "} = " << f << std::endl);
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
				LogIf(print, field->name << "{" << field->bits << "} = " << l << std::endl);
				return;
			}
			l = msg.ReadInt();
			*(uint32_t*)toF = l ^ *(uint32_t*)fromF;

			f = *(float*)toF;
			LogIf(print, field->name << "{" << field->bits << "} = " << f << std::endl);
			return;

		case -88:
			l = msg.ReadInt();
			*(uint32_t*)toF = l ^ *(uint32_t*)fromF;

			f = *(float*)toF;
			LogIf(print, field->name << "{" << field->bits << "} = " << f << std::endl);
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
					LogIf(print, field->name << "{" << field->bits << "} = " << (int)v << std::endl);
					return;
				}
				l = msg.ReadInt();
				*(uint32_t*)toF = l ^ *(uint32_t*)fromF;

				f = *(float*)toF;
				LogIf(print, field->name << "{" << field->bits << "} = " << f << std::endl);
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
			LogIf(print, field->name << "{" << field->bits << "} = " << f << std::endl);
			return;

		case -90:
			f = msg.ReadOriginZFloat(*(float*)fromF);
			*(float*)toF = f;

			LogIf(print, field->name << "{" << field->bits << "} = " << f << std::endl);
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

			LogIf(print, field->name << "{" << field->bits << "} = " << *(uint32_t*)toF << std::endl);
			*(uint32_t*)toF = t;
		}
	}

	int Demo::ParsePacketEntities(Msg& msg, const int time, clientSnapshot_t* from, clientSnapshot_t* to)
	{
		entityState_s* oldstate = nullptr;
		int oldindex = 0, oldnum = 0, numChanged = 0, newnum = -1;

		to->parseEntitiesNum = ParseEntitiesNum;
		to->numEntities = 0;

		if (from)
		{
			if (from->numEntities > 0)
			{
				oldstate = &ParseEntities[from->parseEntitiesNum & MAX_PARSE_ENTITIES - 1];
				oldnum = oldstate->number;
			}
			else
				oldnum = 99999;
		}
		else
			oldnum = 99999;

		while (!msg.overflowed)
		{
			VerboseLog("entitynum: " << newnum << std::endl);
			newnum = ReadEntityIndex(msg, GENTITYNUM_BITS);
			if (newnum == 1023)
				break;
			if (msg.readcount > msg.cursize)
			{
				VerboseLog("Error parsing entities");
				return -1;
			}

			while (oldnum < newnum && !msg.overflowed && oldstate)
			{
				std::memcpy(&ParseEntities[ParseEntitiesNum++ & MAX_PARSE_ENTITIES - 1],
					oldstate, sizeof(entityState_t));
				++to->numEntities;

				if (from && ++oldindex < from->numEntities)
				{
					oldstate = &ParseEntities[(oldindex + from->parseEntitiesNum) & MAX_PARSE_ENTITIES - 1];
					oldnum = oldstate->number;
				}
				else
					oldnum = 99999;
			}
			if (oldnum == newnum)
			{
				++numChanged;
				DeltaEntity(msg, time, to, newnum, oldstate);

				if (from && ++oldindex < from->numEntities)
				{
					oldstate = &ParseEntities[(oldindex + from->parseEntitiesNum) & MAX_PARSE_ENTITIES - 1];
					oldnum = oldstate->number;
				}
				else
					oldnum = 99999;
			}
			else
			{
				++numChanged;
				DeltaEntity(msg, time, to, newnum, &EntityBaselines[newnum]);
			}
		}

		while (oldnum != 99999 && !msg.overflowed)
		{
			std::memcpy(&ParseEntities[ParseEntitiesNum++ & MAX_PARSE_ENTITIES - 1],
				oldstate, sizeof(entityState_t));
			++to->numEntities;

			if (++oldindex < from->numEntities)
			{
				oldstate = &ParseEntities[((short)oldindex + (unsigned __int16)from->parseEntitiesNum) &
					MAX_PARSE_ENTITIES - 1];
				oldnum = oldstate->number;
			}
			else
				oldnum = 99999;
		}
		return numChanged;
	}

	void Demo::ParsePacketClients(Msg& msg, const int time, clientSnapshot_t* from, clientSnapshot_t* to)
	{
		clientState_s* oldstate = nullptr;
		int oldnum = 0, oldindex = 0, newnum = -1;

		to->parseClientsNum = ParseClientsNum;
		to->numClients = 0;

		if (from)
		{
			if (oldindex < from->numClients)
			{
				oldstate = &ParseClients[(oldindex + from->parseClientsNum) & MAX_PARSE_CLIENTS - 1];
				oldnum = oldstate->clientIndex;
			}
			else
				oldnum = 99999;
		}
		else
			oldnum = 99999;

		while (!msg.overflowed && msg.ReadBit())
		{
			VerboseLog("clientnum: " << newnum << std::endl);
			newnum = ReadEntityIndex(msg, 5);
			if (msg.readcount > msg.cursize)
			{
				VerboseLog("Error parsing clients");
				return;
			}

			while (oldnum < newnum)
			{
				DeltaClient(msg, time, to, oldnum, oldstate, true);

				if (from && ++oldindex < from->numClients)
				{
					oldstate = &ParseClients[(oldindex + from->parseClientsNum) & MAX_PARSE_CLIENTS - 1];
					oldnum = oldstate->clientIndex;
				}
				else
					oldnum = 99999;
			}

			if (oldnum == newnum)
			{
				DeltaClient(msg, time, to, newnum, oldstate, false);

				if (from && ++oldindex < from->numClients)
				{
					oldstate = &ParseClients[(oldindex + from->parseClientsNum) & MAX_PARSE_CLIENTS - 1];
					oldnum = oldstate->clientIndex;
				}
				else
					oldnum = 99999;
			}
			else
			{
				NullClientState = { 0 };
				DeltaClient(msg, time, to, newnum, &NullClientState, 0);
			}
		}

		while (oldnum != 99999 && !msg.overflowed)
		{
			DeltaClient(msg, time, to, oldnum, oldstate, true);

			if (++oldindex < from->numClients)
			{
				oldstate = &ParseClients[(oldindex + from->parseClientsNum) & MAX_PARSE_CLIENTS - 1];
				oldnum = oldstate->clientIndex;
			}
			else
				oldnum = 99999;
		}
	}

	int Demo::ReadLastChangedField(Msg& msg, int totalFields)
	{
		int idealBits, lastChanged;
		idealBits = GetMinBitCount(totalFields);
		lastChanged = msg.ReadBits(idealBits);
		return lastChanged;
	}

	bool Demo::ReadDeltaEntity(Msg& msg, const int time, entityState_t* from, entityState_t* to, int number)
	{
		int numFields = sizeof(entityStateFields) / sizeof(entityStateFields[0]);
		return ReadDeltaStruct(msg, time, (unsigned char*)from, (unsigned char*)to, number,
			numFields, GetMinBitCount(MAX_GENTITIES - 1), entityStateFields);
	}

	bool Demo::ReadDeltaClient(Msg& msg, const int time, clientState_t* from, clientState_t* to, int number)
	{
		int numFields = sizeof(clientStateFields) / sizeof(clientStateFields[0]);
		return ReadDeltaStruct(msg, time, (unsigned char*)from, (unsigned char*)to, number,
			numFields, GetMinBitCount(MAX_CLIENTS - 1), clientStateFields);
	}

	void Demo::ReadDeltaPlayerState(Msg& msg, int time, playerState_t* from, playerState_t* to,
		bool predictedFieldsIgnoreXor)
	{
		int i, j;
		playerState_t dst;

		if (!from)
		{
			from = &dst;
			std::memset(&dst, 0, sizeof(dst));
		}
		std::memcpy(to, from, sizeof(playerState_t));

		bool readOriginAndVel = msg.ReadBit() > 0;
		int lastChangedField = ReadLastChangedField(msg, PLAYER_STATE_FIELDS_COUNT);

		netField_t* stateFields = playerStateFields;
		for (int i = 0; i < lastChangedField; ++i)
		{
			bool noXor = predictedFieldsIgnoreXor && readOriginAndVel && stateFields[i].changeHints == 3;
			ReadDeltaField(msg, time, from, to, &stateFields[i], noXor, false);
		}

		for (i = lastChangedField; i < PLAYER_STATE_FIELDS_COUNT; ++i)
		{
			int offset = stateFields[i].offset;
			*(uint32_t*)&((unsigned char*)to)[offset] = *(uint32_t*)&((unsigned char*)from)[offset];
		}

		if (!readOriginAndVel) 
		{
			if (!GetPredictedOriginForServerTime(to->commandTime, to->origin, 
				to->velocity, to->viewangles, &to->bobCycle, &to->movementDir)) 
			{
				VectorCopy(from->origin, to->origin);
				VectorCopy(from->velocity, to->velocity);
				VectorCopy(from->viewangles, to->viewangles);

				to->bobCycle = from->bobCycle;
				to->movementDir = from->movementDir;
			}
		}

		// Stats
		if (msg.ReadBit())
		{
			int statsbits = msg.ReadBits(5);
			if (statsbits & 1)
				to->stats[0] = msg.ReadShort();
			if (statsbits & 2)
				to->stats[1] = msg.ReadShort();
			if (statsbits & 4)
				to->stats[2] = msg.ReadShort();
			if (statsbits & 8)
				to->stats[3] = msg.ReadBits(6);
			if (statsbits & 16)
				to->stats[4] = msg.ReadByte();
		}

		// Ammo stored
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

		// Ammo in clip
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

		// Objectives
		int numObjective = sizeof(from->objective) / sizeof(from->objective[0]);
		if (msg.ReadBit())
		{
			for (int fieldNum = 0; fieldNum < numObjective; ++fieldNum)
			{
				to->objective[fieldNum].state = static_cast<objectiveState_t>(msg.ReadBits(3));
				ReadDeltaObjectiveFields(msg, time, &from->objective[fieldNum], &to->objective[fieldNum]);
			}
		}

		// HUDs
		if (msg.ReadBit())
		{
			ReadDeltaHudElems(msg, time, from->hud.archival, to->hud.archival, 31);
			ReadDeltaHudElems(msg, time, from->hud.current, to->hud.current, 31);
		}

		// Weapon models
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

	int Demo::ReadEntityIndex(Msg &msg, int indexBits)
	{
		if (msg.ReadBit())
			++msg.lastRefEntity;
		else if (indexBits != 10 || msg.ReadBit())
			msg.lastRefEntity = msg.ReadBits(indexBits);
		else
			msg.lastRefEntity += msg.ReadBits(4);
		return msg.lastRefEntity;
	}

	void Demo::DeltaEntity(Msg& msg, const int time, clientSnapshot_t* frame, int newnum, entityState_t* old)
	{
		if (!ReadDeltaEntity(msg, time, old, &ParseEntities[ParseEntitiesNum & MAX_PARSE_ENTITIES - 1], newnum))
		{
			++ParseEntitiesNum;
			++frame->numEntities;
		}
	}

	void Demo::DeltaClient(Msg& msg, const int time, clientSnapshot_t* frame, int newnum, 
		clientState_t* old, bool unchanged)
	{
		clientState_t* state = nullptr;
		state = &ParseClients[ParseClientsNum & MAX_PARSE_CLIENTS - 1];

		if (unchanged)
			std::memcpy(state, old, sizeof(clientState_s));
		else if (ReadDeltaClient(msg, time, old, state, newnum))
			return;

		if (Protocol == COD4_PROTOCOL)
			ClientNames[newnum].netname = state->netname;

		++ParseClientsNum;
		++frame->numClients;
	}

	bool Demo::GetPredictedOriginForServerTime(const int time, float* predictedOrigin, float* predictedVelocity, float* predictedViewangles, int* bobCycle, int* movementDir)
	{
		int index = -1;
		int counter = 0;

		for (int i = LastFrameSrvMsgSeq; counter <= 256; --i, ++counter) 
		{
			i += 256;
			i %= 256;

			if (Frames[i].commandTime <= time)
			{
				index = i;
				break;
			}
		}

		if (index < 0)
			return false;
		else 
		{
			VectorCopy(Frames[index].origin, predictedOrigin);
			VectorCopy(Frames[index].velocity, predictedVelocity);
			VectorCopy(Frames[index].angles, predictedViewangles);

			*bobCycle = Frames[index].bobCycle;
			*movementDir = Frames[index].movementDir;
			return true;
		}
	}
}
