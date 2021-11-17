#include "Demo.hpp"
#include "Huffman.hpp"

#include <cassert>
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
		DemoFileOut.open(filepath + ".1.dm_1", std::ios::binary);
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
		CurrentWritingMsg1 = Msg{ Protocol };
		CurrentWritingMsg2 = Msg{ Protocol };

		CurrentWritingMsg1.Initialize(0x20000, false);
		CurrentWritingMsg2.Initialize(0x20000, false);

		unsigned char slen = 0;
		int protocol = 0, messageLength = 0;
		int dummyEnd = -1;

		DemoFile.read(reinterpret_cast<char*>(&CurrentCompressedMsg.srvMsgSeq), sizeof(int));
		DemoFile.read(reinterpret_cast<char*>(&CurrentCompressedMsg.cursize), sizeof(int));
		DemoFile.read(reinterpret_cast<char*>(&CurrentCompressedMsg.dummy), sizeof(int));

		// EOF
		if (CurrentCompressedMsg.srvMsgSeq == -1) {
			DemoFileOut.write(reinterpret_cast<char*>(&slen), sizeof(slen));
			DemoFileOut.write(reinterpret_cast<char*>(&dummyEnd), sizeof(dummyEnd));
			DemoFileOut.write(reinterpret_cast<char*>(&dummyEnd), sizeof(dummyEnd));

			return;
		}

		// Read the client message
		CurrentCompressedMsg.cursize -= sizeof(int);
		CurrentCompressedMsg.Initialize(CurrentCompressedMsg.cursize, true);
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
				
				VectorCopy(CurrentUncompressedMsg.mapCenter, CurrentWritingMsg1.mapCenter);
				WriteGamestate(CurrentWritingMsg1);
				break;
			case svc_ops_e::svc_serverCommand: 
			{
				int seq = -1;
				ParseCommandString(CurrentUncompressedMsg, seq);

				WriteCommandString(CurrentWritingMsg1, seq);
				break;
			}
			case svc_ops_e::svc_snapshot:
			{
				int oldSnapIndex = -1, newSnapIndex = -1;
				ParseSnapshot(CurrentUncompressedMsg, oldSnapIndex, newSnapIndex);

				WriteSnapshot(CurrentWritingMsg1, oldSnapIndex, newSnapIndex);
				break;
			}
			case svc_ops_e::svc_configclient:
			{
				unsigned int clientnum = 0;
				ParseConfigClient(CurrentUncompressedMsg, clientnum);

				WriteConfigClient(CurrentWritingMsg1, clientnum);
				break;
			}
			default:
				return;
			}
		}

		CurrentWritingMsg1.WriteByte(static_cast<int>(svc_ops_e::svc_EOF));
		CurrentWritingMsg2.cursize = 4 + Huffman::Compress(&CurrentWritingMsg1.buffer[0], CurrentWritingMsg1.cursize, &CurrentWritingMsg2.buffer[0], NULL);

		char seq = 0;
		DemoFileOut.write(reinterpret_cast<char*>(&seq), sizeof(seq));

		DemoFileOut.write(reinterpret_cast<char*>(&CurrentCompressedMsg.srvMsgSeq), sizeof(CurrentCompressedMsg.srvMsgSeq));
		DemoFileOut.write(reinterpret_cast<char*>(&CurrentWritingMsg2.cursize), sizeof(CurrentWritingMsg2.cursize));
		DemoFileOut.write(reinterpret_cast<char*>(&CurrentCompressedMsg.dummy), sizeof(CurrentCompressedMsg.dummy));
		DemoFileOut.write(reinterpret_cast<char*>(&CurrentWritingMsg2.buffer[0]), CurrentWritingMsg2.cursize - 4); // - 4 because of writing CurrentCompressedMsg.dummy directly to disk instead of including it in the message buffer!
	}

	void Demo::ReadArchive()
	{
		int len = 48;
		archivedFrame_t frame = { 0 };

		CurrentCompressedMsg = Msg{ Protocol };
		CurrentCompressedMsg.Initialize(len, true);

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
		
		char seq = 1;
		DemoFileOut.write(reinterpret_cast<char*>(&seq), sizeof(seq));
		DemoFileOut.write(reinterpret_cast<char*>(&CurrentCompressedMsg.srvMsgSeq), sizeof(int));
		DemoFileOut.write(reinterpret_cast<char*>(&frame.origin[0]), sizeof(float));
		DemoFileOut.write(reinterpret_cast<char*>(&frame.origin[1]), sizeof(float));
		DemoFileOut.write(reinterpret_cast<char*>(&frame.origin[2]), sizeof(float));
		DemoFileOut.write(reinterpret_cast<char*>(&frame.velocity[0]), sizeof(float));
		DemoFileOut.write(reinterpret_cast<char*>(&frame.velocity[1]), sizeof(float));
		DemoFileOut.write(reinterpret_cast<char*>(&frame.velocity[2]), sizeof(float));
		DemoFileOut.write(reinterpret_cast<char*>(&frame.movementDir), sizeof(int));
		DemoFileOut.write(reinterpret_cast<char*>(&frame.bobCycle), sizeof(int));
		DemoFileOut.write(reinterpret_cast<char*>(&frame.commandTime), sizeof(int));
		DemoFileOut.write(reinterpret_cast<char*>(&frame.angles[0]), sizeof(float));
		DemoFileOut.write(reinterpret_cast<char*>(&frame.angles[1]), sizeof(float));
		DemoFileOut.write(reinterpret_cast<char*>(&frame.angles[2]), sizeof(float));
	}

	void Demo::ReadProtocol()
	{
		int legacyEnd = 0;
		uint64_t reserved = 0;

		DemoFile.read(reinterpret_cast<char*>(&Protocol), sizeof(uint32_t));
		DemoFile.read(reinterpret_cast<char*>(&legacyEnd), sizeof(int));
		DemoFile.read(reinterpret_cast<char*>(&reserved), sizeof(uint64_t));

		std::cout << "Protocol: " << Protocol << std::endl;
		
		char seq = 2;
		DemoFileOut.write(reinterpret_cast<char*>(&seq), sizeof(seq));
		DemoFileOut.write(reinterpret_cast<char*>(&Protocol), sizeof(Protocol));
		DemoFileOut.write(reinterpret_cast<char*>(&legacyEnd), sizeof(legacyEnd));
		DemoFileOut.write(reinterpret_cast<char*>(&reserved), sizeof(reserved));
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
					
					if (idx == 12) {
#ifdef _MSC_VER
						sscanf_s(s.c_str(), "%f %f %f", &mapCenter[0], &mapCenter[1], &mapCenter[2]);
#else
						sscanf(s.c_str(), "%f %f %f", &mapCenter[0], &mapCenter[1], &mapCenter[2]);
#endif
						VectorCopy(mapCenter, msg.mapCenter);
					}
					
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
					
					if (idx == 12) {
#ifdef _MSC_VER
						sscanf_s(s.c_str(), "%f %f %f", &mapCenter[0], &mapCenter[1], &mapCenter[2]);
#else
						sscanf(s.c_str(), "%f %f %f", &mapCenter[0], &mapCenter[1], &mapCenter[2]);
#endif
						VectorCopy(mapCenter, msg.mapCenter);
					}
					
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

		ServerConfigSequence = msg.ReadInt();
		clientNum = msg.ReadInt();
		if (clientNum >= 64 || clientNum < 0)
			return;
		int checksumFeed = msg.ReadInt();

		if (Protocol == COD4X_FALLBACK_PROTOCOL + 1)
		{
			int dbchecksumFeed = msg.ReadInt();
			//DB_SetPureChecksumFeed(dbchecksumFeed);
		}
	}

	void Demo::ParseCommandString(Msg& msg, int& seq)
	{
		seq = msg.ReadInt();
		int index;
		std::string cs = msg.ReadString();

		index = seq & 0x7F;
		CommandStrings[index] = cs;

		VerboseLog("server command[" << index << "]: " << cs << std::endl);
	}

	void Demo::ParseConfigClient(Msg& msg, unsigned int clientnum)
	{
		std::string name;
		std::string clantag;
		unsigned int sequence;

		sequence = msg.ReadInt();
		if (sequence != ServerConfigSequence + 1) 
		{
			msg.ReadByte();
			name = msg.ReadString();
			clantag = msg.ReadString();
			return;
		}
		ServerConfigSequence = sequence;

		clientnum = msg.ReadByte();
		name = msg.ReadString();
		clantag = msg.ReadString();

		if (clientnum >= 64 || clientnum < 0)
			return;

		ClientNames[clientnum].netname = name;
		ClientNames[clientnum].clantag = clantag;
	}

	void Demo::ParseSnapshot(Msg& msg, int& oldSnapIndex, int& newSnapIndex)
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
		
		oldSnapIndex = CurrentSnapshot.deltaNum & PACKET_MASK;
		newSnapIndex = SnapMessageNum & PACKET_MASK;
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
			newnum = ReadEntityIndex(msg, 6);
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

		bool readOriginAndVel = sendOriginAndVel = msg.ReadBit() > 0;
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
			assert(lc <= sizeof(hudElemFields) / sizeof(hudElemFields[0]));
			
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
			i &= 255;

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
	
	void Demo::WriteGamestate(Msg& msg)
	{
		entityState_t* ent;
		entityState_t nullstate = {};

		int numCS, i;
		const char* s;

		if (Protocol == COD4_PROTOCOL)
		{
			msg.WriteByte(static_cast<int>(svc_ops_e::svc_gamestate));
			msg.WriteInt(ServerCommandSequence);
			msg.WriteByte(static_cast<int>(svc_ops_e::svc_configstring));

			// configstrings
			for (i = 0, numCS = 0; i < MAX_CONFIGSTRINGS; i++) {
				if (ConfigStrings[i] != "") {
					numCS++;
				}
			}

			msg.WriteShort(numCS);

			// configstrings
			for (i = 0; i < MAX_CONFIGSTRINGS; i++) {
				if (ConfigStrings[i] == "") {
					continue;
				}
				s = ConfigStrings[i].c_str();
				msg.WriteBit0();
				msg.WriteBits(i, 12);
				msg.WriteString(s);
			}

			// baselines
			for (i = 0; i < MAX_GENTITIES; i++) {
				ent = &EntityBaselines[i];

				if (!ent->number) {
					continue;
				}

				msg.WriteByte(static_cast<int>(svc_ops_e::svc_baseline));
				WriteDeltaEntity(msg, -90000, &nullstate, ent, true);
			}

			msg.WriteByte(static_cast<int>(svc_ops_e::svc_EOF));
			// finished writing the gamestate stuff

			// write the client num
			msg.WriteInt(clientNum);
			// write the checksum feed
			msg.WriteInt(ChecksumFeed);

			// finished writing the client packet
			msg.WriteByte(static_cast<int>(svc_ops_e::svc_EOF));
		}
		else {
			msg.WriteByte(static_cast<int>(svc_ops_e::svc_gamestate));
			msg.WriteInt(ServerCommandSequence);
			msg.WriteByte(static_cast<int>(svc_ops_e::svc_configstring));

			// configstrings
			for (i = 0, numCS = 0; i < MAX_CONFIGSTRINGS; i++) {
				if (ConfigStrings[i] != "") {
					numCS++;
				}
			}
			for (i = 0; i < MAX_CONFIGSTRINGS; i++) {
				//if (extGameState.stringOffsets[i]) {
				//	numCS++;
				//}
			}
			msg.WriteInt(numCS);

			// configstrings
			for (i = 0; i < MAX_CONFIGSTRINGS; i++) {
				if (ConfigStrings[i] == "") {
					continue;
				}
				s = ConfigStrings[i].c_str();
				msg.WriteInt(i);
				msg.WriteString(s);
			}
			for (i = 0; i < MAX_CONFIGSTRINGS; i++) {
				//if (!extGameState.stringOffsets[i]) {
				//	continue;
				//}
				//s = extGameState.stringData + extGameState.stringOffsets[i];
				//msg.WriteInt(i + MAX_CONFIGSTRINGS);
				//msg.WriteString(s);
			}

			// baselines
			for (i = 0; i < MAX_GENTITIES; i++) {
				ent = &EntityBaselines[i];

				if (!ent->number) {
					continue;
				}

				msg.WriteByte(static_cast<int>(svc_ops_e::svc_baseline));
				WriteDeltaEntity(msg, -90000, &nullstate, ent, true);
			}

			for (i = 0; i < 64; ++i)
			{
				if (ClientNames[i].netname == "")
				{
					continue;
				}

				msg.WriteByte(static_cast<int>(svc_ops_e::svc_configclient));
				msg.WriteByte(i);
				msg.WriteString(ClientNames[i].netname.c_str());
				msg.WriteString(ClientNames[i].clantag.c_str());
			}

			msg.WriteByte(static_cast<int>(svc_ops_e::svc_EOF));
			// finished writing the gamestate stuff

			//Writing the sequence for configdata so all configdata is acknowledged
			msg.WriteInt(ServerConfigSequence);

			// write the client num
			msg.WriteInt(clientNum);
			// write the checksum feed
			msg.WriteInt(ChecksumFeed);

			if (Protocol == COD4X_FALLBACK_PROTOCOL + 1)
			{
				msg.WriteInt(ChecksumFeed);
			}
		}
	}
	
	void Demo::WriteDeltaEntity(Msg& msg, const int time, entityState_t* from, entityState_t* to, bool force) 
	{
		// all fields should be 32 bits to avoid any compiler packing issues
		// the "number" field is not part of the field list
		// if this assert fails, someone added a field to the entityState_t
		// struct without updating the message fields
		//	assert( numFields + 1 == sizeof( *from ) / 4 );

		netFieldList_t* fieldtype;

		if (!to) {
			WriteEntityRemoval(msg, (unsigned char*)from, 10, 0);
			return;
		}

		if (to->number < 0 || to->number >= MAX_GENTITIES) {
			VerboseLog("MSG_WriteDeltaEntity: Bad entity number: %i" << to->number);
		}

		unsigned int index = 17;

		if (to->eType <= 17)
			index = to->eType;
		fieldtype = &netFieldList[index];

		WriteEntityDelta(msg, time, (const unsigned char*)from, (const unsigned char*)to, force, fieldtype->numFields, 10, fieldtype->field);
	}
	
	void Demo::WriteEntityRemoval(Msg& msg, unsigned char* from, int indexBits, unsigned char changeBit)
	{
		if (changeBit)
		{
			msg.WriteBit1();
		}

		WriteEntityIndex(msg, *(uint32_t*)from, indexBits);
		msg.WriteBit1();
	}
	
	void Demo::WriteEntityIndex(Msg& msg, const int index, const int indexBits)
	{

		if (index - msg.lastRefEntity == 1)
		{
			msg.WriteBit1();
			msg.lastRefEntity = index;
			return;
		}

		msg.WriteBit0();

		if (indexBits != 10 || index - msg.lastRefEntity > 15)
		{
			if (indexBits == 10)
			{
				msg.WriteBit1();
			}
			msg.WriteBits(index, indexBits);
			msg.lastRefEntity = index;
			return;
		}

		msg.WriteBit0();
		msg.WriteBits(index - msg.lastRefEntity, 4);
		msg.lastRefEntity = index;
	}
	
	int Demo::WriteEntityDelta(Msg& msg, const int time, const unsigned char* from, const unsigned char* to, bool force, int numFields, int indexBits, netField_t* stateFields)
	{
		return WriteDeltaStruct(msg, time, from, to, force, numFields, indexBits, stateFields, 0);
	}
	
	int Demo::WriteDeltaStruct(Msg& msg, const int time, const unsigned char* from, const unsigned char* to, bool force, int numFields, int indexBits, netField_t* stateFields, unsigned char bChangeBit)
	{
		int i, lc;
		int* fromF, * toF;
		netField_s* field;
		int entityNumber = *(uint32_t*)to;

		int oldbitcount = msg.GetUsedBitCount();

		lc = 0;

		for (i = 0, field = stateFields; i < numFields; i++, field++)
		{
			fromF = (int*)((unsigned char*)from + field->offset);
			toF = (int*)((unsigned char*)to + field->offset);

			if (*fromF == *toF) {
				continue;
			}

			if (!ValuesAreEqual(field->bits, fromF, toF))
			{
				lc = i + 1;
			}
		}

		if (lc == 0)
		{
			// nothing at all changed
			if (!force)
			{
				return 0; // nothing at all
			}
			if (bChangeBit)
			{
				msg.WriteBit1();
			}
			// write two bits for no change
			WriteEntityIndex(msg, entityNumber, indexBits);
			msg.WriteBit0();   // not removed
			msg.WriteBit0();   // no delta
			return msg.GetUsedBitCount() - oldbitcount;
		}

		if (bChangeBit)
		{
			msg.WriteBit1();
		}

		WriteEntityIndex(msg, entityNumber, indexBits);
		msg.WriteBit0();
		msg.WriteBit1();
		msg.WriteBits(lc, GetMinBitCount(numFields));

		for (i = 0, field = stateFields; i < lc; i++, field++)
		{
			WriteDeltaField(msg, time, from, to, field, i, 0);
		}

		return msg.GetUsedBitCount() - oldbitcount;
	}
	
	bool Demo::ValuesAreEqual(int bits, const int* fromF, const int* toF)
	{
		bool result;

		if (*fromF == *toF)
			return true;

		switch (bits + 100)
		{
		case 0:
		case 13:
			result = (int16_t)ANGLE2SHORT(*(float*)toF) == (int16_t)ANGLE2SHORT(*(float*)fromF);
			//			((signed int)(182.044449f * *(float *)toF + 0.5f) == (signed int)(*(float *)fromF * 182.044449f  + 0.5f));
			break;
		case 8:
		case 9:
		case 10:
			result = (signed int)floorf(*(float*)fromF + 0.5f) == (signed int)floorf(*(float*)toF + 0.5f);
			break;
		case 5:
			result = *fromF / 100 == *toF / 100;
			break;
		default:
			result = 0;
			break;
		}

		return result;
	}
	
	void Demo::WriteDeltaField(Msg& msg, const int time, const unsigned char* from, const unsigned char* to, netField_s* field, int fieldNum, unsigned char forceSend)
	{
		int nullfield;
		int32_t timetodata;
		int32_t absbits;
		uint8_t abs3bits;
		int32_t fromtoxor;
		const unsigned char* fromdata;
		const unsigned char* todata;
		uint32_t uint32todata;
		uint32_t uint32fromdata;
		int32_t int32todata;
		int32_t int32todatafromfloat;
		int32_t int32fromdatafromfloat;

		int32_t int32data1;
		float floattodata;
		float floatfromdata;

		fromdata = &from[field->offset];
		todata = &to[field->offset];
		if (forceSend)
		{
			nullfield = 0;
			fromdata = (const unsigned char*)& nullfield;
		}
		if (field->changeHints != 2)
		{
			if (!forceSend && (*(uint32_t*)fromdata == *(uint32_t*)todata || ValuesAreEqual(field->bits, (const int*)fromdata, (const int*)todata)))
			{
				msg.WriteBit0();
				return;
			}
			msg.WriteBit1();
		}

		int32todata = *(uint32_t*)todata;
		floattodata = *(float*)todata;
		floatfromdata = *(float*)fromdata;
		int32todatafromfloat = (signed int)(*(float*)todata);
		int32fromdatafromfloat = (signed int)(*(float*)fromdata);
		uint32todata = *(uint32_t*)todata;
		uint32fromdata = *(uint32_t*)fromdata;

		switch (field->bits)
		{
		case 0:
			if (floattodata == 0.0)
			{
				msg.WriteBit0();
				if (int32todata == 0x80000000)
				{
					msg.WriteBit1();
					break;
				}
				msg.WriteBit0();
				break;
			}
			msg.WriteBit1();
			if (int32todata == 0x80000000 || floattodata != (float)int32todatafromfloat || int32todatafromfloat + 4096 < 0 || int32todatafromfloat + 4096 > 0x1FFF || int32fromdatafromfloat + 4096 < 0 || int32fromdatafromfloat + 4096 > 0x1FFF)
			{
				msg.WriteBit1();
				msg.WriteInt(uint32todata ^ uint32fromdata);
				break;
			}
			msg.WriteBit0();
			int32data1 = (int32fromdatafromfloat + 4096) ^ (int32todatafromfloat + 4096);
			msg.WriteBits(int32data1, 5);
			msg.WriteByte(int32data1 >> 5);
			break;

		case -89:
			if (int32todata == 0x80000000 || floattodata != (float)int32todatafromfloat || int32todatafromfloat + 4096 < 0 || int32todatafromfloat + 4096 > 0x1FFF)
			{
				msg.WriteBit1();
				msg.WriteInt(uint32todata ^ uint32fromdata);
				break;
			}
			msg.WriteBit0();
			int32data1 = (int32todatafromfloat + 4096) ^ (int32fromdatafromfloat + 4096);
			msg.WriteBits(int32data1, 5);
			msg.WriteByte(int32data1 >> 5);
			break;
			//LABEL_54;

		case -88:
			msg.WriteInt(uint32todata ^ uint32fromdata);
			break;

		case -99:
			if (*(float*)todata != 0.0 || int32todata == 0x80000000)
			{
				msg.WriteBit1();
				if (int32todata != 0x80000000 && floattodata == (float)int32todatafromfloat && int32todatafromfloat + 2048 >= 0 && int32todatafromfloat + 2048 <= 4095)
				{
					msg.WriteBit0();
					msg.WriteBits((int32todatafromfloat + 2048) ^ (int32fromdatafromfloat + 2048), 4);
					msg.WriteByte(((int32todatafromfloat + 2048) ^ (int32fromdatafromfloat + 2048)) >> 4);
				}
				else
				{
					msg.WriteBit1();
					msg.WriteInt(uint32todata ^ uint32fromdata);
				}
				break;
			}
			//LABEL_28
			msg.WriteBit0();
			break;

		case -100:
			if (uint32todata)
			{
				msg.WriteBit1();
				WriteAngle16(msg, floattodata);
			}
			else
			{
				msg.WriteBit0();
			}
			break;

		case -87:
			WriteAngle16(msg, floattodata);
			break;

		case -86:
			msg.WriteBits(floorf(((floattodata - 1.4) * 10.0)), 5);
			break;

		case -85:
			if (!((fromdata[3] == -1 && todata[3]) || (fromdata[3] != -1 && (fromdata[3] || todata[3] != -1))))
			{
				if (!memcmp(fromdata, todata, 3))
				{
					//LABEL_47
					msg.WriteBit1();
					break;
				}
			}

			msg.WriteBit0();
			if (fromdata[0] != todata[0] || fromdata[1] != todata[1] || fromdata[2] != todata[2])
			{
				msg.WriteBit0();
				msg.WriteByte(todata[0]);
				msg.WriteByte(todata[1]);
				msg.WriteByte(todata[2]);
			}
			else
			{
				msg.WriteBit1();
			}
			msg.WriteBits((unsigned int)todata[3] >> 3, 5);
			break;

		case -97:
			timetodata = uint32todata - time;
			if ((unsigned int)timetodata >= 0xFFFFFF01 || timetodata == 0)
			{
				msg.WriteBit0();
				msg.WriteBits(-timetodata, 8);
			}
			else
			{
				msg.WriteBit1();
				msg.WriteInt(uint32todata);
			}
			break;

		case -98:
			Write24BitFlag(msg, uint32fromdata, uint32todata);
			break;

		case -96:
			if (uint32todata != 1022)
			{
				msg.WriteBit0();
				if (uint32todata)
				{
					msg.WriteBit0();
					msg.WriteBits(uint32todata, 2);
					msg.WriteByte(uint32todata >> 2);
					break;
				}
			}
			//LABEL_47
			msg.WriteBit1();
			break;

		case -93:
		case -94:
			msg.WriteByte(uint32todata);
			break;

		case -91:
		case -92:
			msg.WriteOriginFloat(field->bits, floattodata, floatfromdata);
			break;

		case -90:
			msg.WriteOriginZFloat(floattodata, floatfromdata);
			break;

		case -95:
			msg.WriteBits(uint32todata / 100, 7);
			break;

		default:
			if (uint32todata)
			{
				msg.WriteBit1();
				absbits = abs(field->bits);
				fromtoxor = uint32todata ^ uint32fromdata;
				abs3bits = absbits & 7;
				if (abs3bits)
				{
					msg.WriteBits(fromtoxor, absbits & 7);
					absbits -= abs3bits;
					fromtoxor >>= abs3bits;
				}
				while (absbits)
				{
					msg.WriteByte(fromtoxor);
					fromtoxor >>= 8;
					absbits -= 8;
				}
			}
			else
			{
				msg.WriteBit0();
			}
			break;
		}
	}
	
	void Demo::WriteAngle16(Msg& msg, float f) {
		msg.WriteShort(ANGLE2SHORT(f));
	}

	void Demo::Write24BitFlag(Msg& msg, const int oldFlags, const int newFlags)
	{
		int xorflags;
		signed int shiftedflags;
		signed int i;

		xorflags = newFlags ^ oldFlags;
		if ((xorflags - 1) & xorflags)
		{
			msg.WriteBit1();
			shiftedflags = newFlags;

			for (i = 3; i; --i)
			{
				msg.WriteByte(shiftedflags);
				shiftedflags >>= 8;
			}

		}
		else
		{
			for (i = 0; !(xorflags & 1); ++i)
			{
				xorflags >>= 1;
			}
			msg.WriteBit0();
			msg.WriteBits(i, 5);
		}
	}

	void Demo::WriteCommandString(Msg& msg, int seq)
	{
		msg.WriteByte(static_cast<int>(svc_ops_e::svc_serverCommand));
		msg.WriteInt(seq);

		std::string cs = CommandStrings[seq & 0x7F];
		msg.WriteString(cs.c_str());
	}

	void Demo::WriteConfigClient(Msg& msg, unsigned int clientnum)
	{
		msg.WriteByte(static_cast<int>(svc_ops_e::svc_configclient));
		msg.WriteInt(ServerConfigSequence);

		msg.WriteByte(clientnum);
		msg.WriteString(ClientNames[clientnum].netname.c_str());
		msg.WriteString(ClientNames[clientnum].clantag.c_str());
	}
	
	void Demo::WriteSnapshot(Msg& msg, int oldSnapIndex, int newSnapIndex)
	{
		if (oldSnapIndex == -1 || newSnapIndex == -1)
			return;

		clientSnapshot_t* oldSnap, * newSnap;
		newSnap = &Snapshots[newSnapIndex];

		if (newSnap->deltaNum == -1) {
			oldSnap = &CurrentSnapshot;
			memset(oldSnap, 0, sizeof(*oldSnap));
		}
		else 
			oldSnap = &Snapshots[oldSnapIndex];

		msg.WriteByte(static_cast<int>(svc_ops_e::svc_snapshot));
		msg.WriteInt(newSnap->serverTime);
		msg.WriteByte((newSnap->deltaNum == -1) ? 0 : newSnap->messageNum - newSnap->deltaNum);
		msg.WriteByte(newSnap->snapFlags);

		WriteDeltaPlayerState(msg, newSnap->serverTime, &oldSnap->ps, &newSnap->ps);

		msg.lastRefEntity = -1;

		WritePacketEntities(msg, newSnap->serverTime, oldSnap, newSnap);

		msg.lastRefEntity = -1;

		WritePacketClients(msg, newSnap->serverTime, oldSnap, newSnap);
	}
	
	void Demo::WriteDeltaPlayerState(Msg& msg, const int time, playerState_t* from, playerState_t* to)
	{
		int i, j, lc;

		for (i = 0, lc = 0; i < sizeof(playerStateFields) / sizeof(playerStateFields[0]); ++i) {
			if (ShouldSendPSField(sendOriginAndVel, to, from, &playerStateFields[i]))
				lc = i + 1;
		}

		assert(lc >= 0);

		msg.WriteBits(sendOriginAndVel, 1);
		msg.WriteBits(lc, GetMinBitCount(sizeof(playerStateFields) / sizeof(playerStateFields[0])));

		for (i = 0; i < lc; ++i) {
			if (playerStateFields[i].changeHints == 2 || ShouldSendPSField(sendOriginAndVel, from, to, &playerStateFields[i])) {
				bool forceSend = sendOriginAndVel && playerStateFields[i].changeHints == 3;
				WriteDeltaField(msg, time, (unsigned char*)from, (unsigned char*)to, &playerStateFields[i], i, forceSend);
			}
			else
				msg.WriteBit0(); // no change
		}

		int statsbits = 0;
		for (i = 0; i < 5; ++i) {
			if (to->stats[i] != from->stats[i])
				statsbits |= 1 << i;
		}

		if (statsbits) {
			msg.WriteBit1(); // changed
			msg.WriteBits(statsbits, 5);

			for (i = 0; i < 3; ++i) {
				if (statsbits & (1 << i))
					msg.WriteShort(to->stats[i]);
			}

			if (statsbits & 8)
				msg.WriteBits(to->stats[3], 6);
			if (statsbits & 16)
				msg.WriteByte(to->stats[4]);
		}
		else
			msg.WriteBit0(); // no change to stats

		int ammobits[8];
		for (j = 0; j < 4; ++j) {  //----(SA)	modified for 64 weaps
			ammobits[j] = 0;	  //Hmm only 64? CoD4 has 128 but its still 64

			for (i = 0; i < 16; ++i) {
				if (to->ammo[i + (j * 16)] != from->ammo[i + (j * 16)])
					ammobits[j] |= 1 << i;
			}
		}

		//----(SA)	also encapsulated ammo changes into one check.  clip values will change frequently,
		// but ammo will not.  (only when you get ammo/reload rather than each shot)
		if (ammobits[0] || ammobits[1] || ammobits[2] || ammobits[3]) {  // if any were set...
			msg.WriteBit1(); // changed
			for (j = 0; j < 4; ++j) {
				if (ammobits[j]) {

					msg.WriteBit1(); // changed
					msg.WriteShort(ammobits[j]);

					for (i = 0; i < 16; ++i) {
						if (ammobits[j] & (1 << i))
							msg.WriteShort(to->ammo[i + (j * 16)]);
					}
				}
				else
					msg.WriteBit0(); // no change
			}
		}
		else
			msg.WriteBit0(); // no change

		// ammo in clip
		for (j = 0; j < 8; ++j) {  //----(Ninja)	modified for 128 weaps (CoD4)
			int clipbits = 0;

			for (i = 0; i < 16; ++i) {
				if (to->ammoclip[i + (j * 16)] != from->ammoclip[i + (j * 16)])
					clipbits |= 1 << i;
			}

			if (clipbits) {
				msg.WriteBit1(); // changed
				msg.WriteShort(clipbits);

				for (i = 0; i < 16; ++i) {
					if (clipbits & (1 << i))
						msg.WriteShort(to->ammoclip[i + (j * 16)]);
				}
			}
			else
				msg.WriteBit0(); // no change
		}

		if (!memcmp(from->objective, to->objective, sizeof(from->objective)))
			msg.WriteBit0(); // no change
		else {
			msg.WriteBit1(); // changed

			for (i = 0; i < sizeof(from->objective) / sizeof(from->objective[0]); ++i) {
				msg.WriteBits(to->objective[i].state, 3);
				WriteDeltaObjective(msg, time, &from->objective[i], &to->objective[i]);
			}
		}

		if (memcmp(&from->hud, &to->hud, sizeof(from->hud))) {
			msg.WriteBit1();

			WriteDeltaHudElems(msg, time, from->hud.archival, to->hud.archival, 31);
			WriteDeltaHudElems(msg, time, from->hud.current, to->hud.current, 31);
		}
		else
			msg.WriteBit0();

		if (!memcmp(from->weaponmodels, to->weaponmodels, sizeof(from->weaponmodels)))
			msg.WriteBit0(); //No weapon viewmodel has changed
		else {
			msg.WriteBit1(); //Any weapon viewmodel has changed

			for (i = 0; i < sizeof(from->weaponmodels); ++i)
				msg.WriteByte(to->weaponmodels[i]);
		}
	}
	
	bool Demo::ShouldSendPSField(bool sendOriginAndVel, playerState_t* from, playerState_t* to, netField_t* field)
	{
		// if (field->bits == -87 && (to->otherFlags & 2 || (((to->eFlags & 0xFF) ^ (from->eFlags & 0xFF)) & 2) || to->viewlocked_entNum != 1023 || to->pm_type == 5))
			// return true;

		if (field->changeHints == 3 || field->bits == -87)
			return sendOriginAndVel;

		int* fromF = (int*)((unsigned char*)from + field->offset);
		int* toF = (int*)((unsigned char*)to + field->offset);

		bool result = ValuesAreEqual(field->bits, fromF, toF);
		return result ^= true;
	}
	
	void Demo::WriteDeltaObjective(Msg& msg, const int time, objective_t* from, objective_t* to)
	{
		int lc = WriteDelta_LastChangedField((unsigned char*)from, (unsigned char*)to, objectiveFields, sizeof(objectiveFields) / sizeof(objectiveFields[0]));

		if (lc == -1) {
			msg.WriteBit0(); // no change
			return;
		}

		msg.WriteBit1(); //Something has changed

		for (int i = 0; i < sizeof(objectiveFields) / sizeof(objectiveFields[0]); ++i) //Write out all fields in case a single one has changed
			WriteDeltaField(msg, time, (unsigned char*)from, (unsigned char*)to, &objectiveFields[i], i, false);
	}
	
	int Demo::WriteDelta_LastChangedField(unsigned char* from, unsigned char* to, netField_t* fields, int numFields)
	{
		int lc = -1;

		for (int j = numFields - 1; j >= 0; --j) {
			int* fromF = (int*)((unsigned char*)from + fields[j].offset);
			int* toF = (int*)((unsigned char*)to + fields[j].offset);

			if (!ValuesAreEqual(fields[j].bits, fromF, toF)) {
				lc = j;
				break;
			}
		}

		return lc;
	}
	
	void Demo::WriteDeltaHudElems(Msg& msg, const int time, hudelem_t* from, hudelem_t* to, const int count)
	{
		int i = 0;
		for (; i < count; ++i) {
			if (to[i].type == HE_TYPE_FREE)
				break;
		}

		int numHE = i;
		msg.WriteBits(numHE, GetMinBitCount(31)); // 5

		for (int k = 0; k < numHE; ++k) {
			int lc = WriteDelta_LastChangedField((unsigned char*)(from + k), (unsigned char*)(to + k), hudElemFields, sizeof(hudElemFields) / sizeof(hudElemFields[0]));

			int size = sizeof(hudElemFields) / sizeof(hudElemFields[0]);
			assert(lc <= size);

			if (lc == -1)
				lc = 0; //First field gets written always in IW3 - bug with no functional effect

			msg.WriteBits(lc, GetMinBitCount(sizeof(hudElemFields) / sizeof(hudElemFields[0]))); //Write highest changed field

			for (i = 0; i <= lc; ++i) //Write out the fields unit the last changed one
				WriteDeltaField(msg, time, (unsigned char*)(from + k), (unsigned char*)(to + k), &hudElemFields[i], i, false);
		}
	}
	
	void Demo::WritePacketEntities(Msg& msg, const int time, clientSnapshot_t* oldframe, clientSnapshot_t* newframe)
	{
		entityState_t r_newent, * newent = &r_newent, r_oldent, * oldent = &r_oldent;
		int oldindex = 0, newindex = 0, oldnum = 0, newnum = -1;

		while (newindex < newframe->numEntities || oldindex < oldframe->numEntities) {
			if (newindex >= newframe->numEntities) {
				newent = nullptr;
				newnum = 99999;
			}
			else {
				newent = &ParseEntities[(newframe->parseEntitiesNum + newindex) & 2047];
				newnum = newent->number;
			}

			if (oldindex >= oldframe->numEntities) {
				oldent = nullptr;
				oldnum = 99999;
			}
			else {
				oldent = &ParseEntities[(oldframe->parseEntitiesNum + oldindex) & 2047];
				oldnum = oldent->number;
			}

			if (newnum == oldnum) {
				WriteDeltaEntity(msg, time, oldent, newent, false);
				oldindex++;
				newindex++;
			}
			else if (newnum < oldnum) {
				WriteDeltaEntity(msg, time, &EntityBaselines[newnum], newent, true);
				newindex++;
			}
			else {
				WriteDeltaEntity(msg, time, oldent, nullptr, false);
				oldindex++;
			}
		}

		WriteEntityIndex(msg, 1023, 10);
	}
	
	void Demo::WritePacketClients(Msg& msg, const int time, clientSnapshot_t* oldframe, clientSnapshot_t* newframe)
	{
		clientState_t r_newcs, * newcs = &r_newcs, r_oldcs, * oldcs = &r_oldcs;
		int oldindex = 0, newindex = 0, oldnum = 0, newnum = -1;

		while (newindex < newframe->numClients || oldindex < oldframe->numClients) {
			if (newindex >= newframe->numClients) {
				newcs = nullptr;
				newnum = 99999;
			}
			else {
				newcs = &ParseClients[(newframe->parseClientsNum + newindex) & 2047];
				newnum = newcs->clientIndex;
			}

			if (oldindex >= oldframe->numClients) {
				oldcs = nullptr;
				oldnum = 99999;
			}
			else {
				oldcs = &ParseClients[(oldframe->parseClientsNum + oldindex) & 2047];
				oldnum = oldcs->clientIndex;
			}

			if (newnum == oldnum) {
				WriteDeltaClient(msg, time, oldcs, newcs, false);
				oldindex++;
				newindex++;
			}
			else if (newnum < oldnum) {
				WriteDeltaClient(msg, time, nullptr, newcs, true);
				newindex++;
			}
			else {
				WriteDeltaClient(msg, time, oldcs, nullptr, true);
				oldindex++;
			}
		}

		msg.WriteBit0();
	}
	
	void Demo::WriteDeltaClient(Msg& msg, const int time, clientState_t* from, clientState_t* to, bool force)
	{
		clientState_t nullstate;

		if (!from) {
			from = &nullstate;
			memset(&nullstate, 0, sizeof(nullstate));
		}

		if (to)
			WriteClientDelta(msg, time, from, to, force, sizeof(clientStateFields) / sizeof(clientStateFields[0]), GetMinBitCount(64 - 1), clientStateFields);
		else
			WriteEntityRemoval(msg, (unsigned char*)from, GetMinBitCount(64 - 1), true);
	}

	void Demo::WriteClientDelta(Msg& msg, const int time, clientState_t* from, clientState_t* to, bool force, int numFields, int indexBits, netField_t* stateFields)
	{
		WriteDeltaStruct(msg, time, (const unsigned char*)from, (const unsigned char*)to, force, numFields, indexBits, stateFields, true);
	}
}
