#include "Demo.hpp"
#include "ClientSnapshotData.hpp"
#include "ClientArchiveData.hpp"
#include "NetFields.hpp"
#include "Huffman.hpp"
#include "Msg.hpp"
#include <iostream>
#include <iterator>

namespace Iswenzz
{
	Demo::Demo() { }
	Demo::Demo(std::string filepath)
	{
		open(filepath);
	}

	Demo::~Demo()
	{
		close();
	}

	void Demo::open(std::string filepath)
	{
		if (isDemoOpen)
			close();
		isDemoOpen = true;
		demoFilePath = filepath;

		demo.open(filepath, std::ios::binary);
		readHeader();

		while (demo.is_open())
		{
			unsigned char packet_header;
			demo.read(reinterpret_cast<char*>(&packet_header), sizeof(unsigned char));

			switch (static_cast<int>(packet_header))
			{
				case static_cast<int>(MSGType::MSG_SNAPSHOT):
					readSnapshot();
					break;
				case static_cast<int>(MSGType::MSG_FRAME):
					readArchive();
					break;
			}
		}
	}

	void Demo::close()
	{
		if (demo.is_open())
			demo.close();
		snapshots.clear();
		archive.clear();
		isDemoOpen = false;
	}

	void Demo::readHeader()
	{
		unsigned char header, slen;
		int protocol;
		int dummyend[3];
		int len, swlen;

		demo.read(reinterpret_cast<char*>(&header), sizeof(unsigned char)); // 1 head
		demo.read(reinterpret_cast<char*>(&protocol), sizeof(int));         // 5 protocol
		demo.read(reinterpret_cast<char*>(&dummyend[0]), sizeof(int));      // 9 datalen (-1 = demo ended)
		demo.read(reinterpret_cast<char*>(&dummyend[1]), sizeof(int));      // 13
		demo.read(reinterpret_cast<char*>(&dummyend[2]), sizeof(int));      // 17
		demo.read(reinterpret_cast<char*>(&slen), sizeof(unsigned char));   // 18
		demo.read(reinterpret_cast<char*>(&swlen), sizeof(int));            // 22 packet sequence
		demo.read(reinterpret_cast<char*>(&len), sizeof(int));              // 26 client message length

		std::vector<unsigned char> complen(len);
		demo.read(reinterpret_cast<char*>(complen.data()), len);            // client message
	}

	void Demo::readArchive()
	{
		ClientArchiveData data;													// 1 header
		demo.read(reinterpret_cast<char*>(&data.index), sizeof(int));			// 5
		demo.read(reinterpret_cast<char*>(&data.origin[0]), sizeof(float));		// 9 player's position
		demo.read(reinterpret_cast<char*>(&data.origin[1]), sizeof(float));		// 13
		demo.read(reinterpret_cast<char*>(&data.origin[2]), sizeof(float));		// 17
		demo.read(reinterpret_cast<char*>(&data.velocity[0]), sizeof(float));	// 21
		demo.read(reinterpret_cast<char*>(&data.velocity[1]), sizeof(float));	// 25
		demo.read(reinterpret_cast<char*>(&data.velocity[2]), sizeof(float));	// 29
		demo.read(reinterpret_cast<char*>(&data.movementDir), sizeof(int));		// 33
		demo.read(reinterpret_cast<char*>(&data.bobCycle), sizeof(int));		// 37
		demo.read(reinterpret_cast<char*>(&data.commandTime), sizeof(int));		// 41
		demo.read(reinterpret_cast<char*>(&data.angles[0]), sizeof(float));		// 45 player's angles
		demo.read(reinterpret_cast<char*>(&data.angles[1]), sizeof(float));		// 49
		demo.read(reinterpret_cast<char*>(&data.angles[2]), sizeof(float));		// 53
		archive.push_back(data);
	}

	void Demo::readSnapshot()
	{
		ClientSnapshotData snap{};
		unsigned char header = 0;
		int swlen, len, lastClientCommand, cmd;
																				// 1 header
		demo.read(reinterpret_cast<char*>(&swlen), sizeof(int));				// 5 packet sequence
		demo.read(reinterpret_cast<char*>(&len), sizeof(int));					// 9 client message length

		if (swlen == -1 && len == -1)											// demo ended
		{
			demo.close();
			return;
		}
		demo.read(reinterpret_cast<char*>(&lastClientCommand), sizeof(int));	// last client command
		std::vector<unsigned char> complen(len - 4);
		demo.read(reinterpret_cast<char*>(complen.data()), complen.size());				// client message

		// Fill the snapshot struct
		Msg snap_msg{ complen.data(), complen.size(), MSGCrypt::MSG_CRYPT_HUFFMAN };
		snap.lastClientCommand = lastClientCommand;

		// Read command
		if (snap_msg.overflowed)
			snap_msg.~Msg();
		cmd = snap_msg.readByte();

		std::cout << "Command: " << cmd << std::endl;
		switch (cmd)
		{
			case static_cast<int>(svc_ops_e::svc_serverCommand):
				readCommandString(snap_msg);
				break;
			case static_cast<int>(svc_ops_e::svc_snapshot):
				readSnapshot(snap_msg, snap);
				break;
		}

		// Read the rest
		std::vector<unsigned char> rest(NETCHAN_FRAGMENTBUFFER_SIZE);
		snap_msg.readData(rest.data(), NETCHAN_FRAGMENTBUFFER_SIZE);

		//std::cin.get();
		snapshots.push_back(snap);
	}

	void Demo::readCommandString(Msg& msg)
	{
		int seq = msg.readInt();
		int index;
		std::string s = msg.readString(0x400u);

		index = seq & 0x7F;
		std::cout << "Server Command: " << index << " " << s << std::endl;
	}

	void Demo::readSnapshot(Msg& msg, ClientSnapshotData& snap)
	{
		clientSnapshot_t* frame = new clientSnapshot_t{};
		clientSnapshot_t* oldFrame = new clientSnapshot_t{};
		if (snapshots.size() > 0) // if oldFrame exists
			std::memcpy(oldFrame, &snapshots.back().cl_snap, sizeof(clientSnapshot_t));
		std::memcpy(frame, oldFrame, sizeof(clientSnapshot_t));

		int serverTime = msg.readInt();
		unsigned char lastFrame = msg.readByte();
		unsigned char snapFlag = msg.readByte();

		readDeltaPlayerState(msg, serverTime, &oldFrame->ps, &frame->ps, false);
		// @TODO

		std::memcpy(&snap.cl_snap, frame, sizeof(clientSnapshot_t));
		delete oldFrame;
		delete frame;
	}

	void Demo::readDeltaPlayerState(Msg& msg, int time, playerState_t* from, playerState_t* to, 
		bool predictedFieldsIgnoreXor)
	{
		int i, j;
		bool readOriginAndVel = msg.readBit() > 0;
		int lastChangedField = msg.readBits(GetMinBitCount(PLAYER_STATE_FIELDS_COUNT));

		netField_t *stateFields = playerStateFields;
		for (int i = 0; i < lastChangedField; ++i)
		{
			bool noXor = predictedFieldsIgnoreXor && readOriginAndVel && stateFields[i].changeHints == 3;
			msg.readDeltaField(time, from, to, &stateFields[i], noXor);
		}

		for (i = lastChangedField; i < PLAYER_STATE_FIELDS_COUNT; ++i)
		{
			int offset = stateFields[i].offset;
			*(uint32_t*)&((unsigned char*)to)[offset] = *(uint32_t*)&((unsigned char*)from)[offset];
		}

		if (msg.readBit())
		{
			int statsbits = msg.readBits(5);
			for (i = 0; i < 3; i++)
			{
				if (statsbits & (1 << i))
					to->stats[i] = msg.readShort();
			}
			if (statsbits & 8)
				to->stats[3] = msg.readBits(6);
			if (statsbits & 0x10)
				to->stats[4] = msg.readByte();
		}

		// @TODO
		std::cout << "Command Time: " << to->commandTime << std::endl;
		std::cout << "Origin: " 
			<< to->origin[0] << " " << to->origin[1] << " " << to->origin[2]
			<< std::endl;
		std::cout << "Speed: "
			<< to->speed << " "
			<< to->moveSpeedScaleMultiplier << " "
			<< std::endl;
		for (i = 0; i < 31; i++)
		{
			hudelem_t hud = to->hud.current[i];
			if (hud.value > 0)
				std::cout << "HUD Velocity: " << hud.value << std::endl;
			if (hud.materialIndex > 0)
				std::cout << "HUD FPS ID: " << hud.materialIndex << std::endl;
		}

		// ammo stored
		if (msg.readBit())
		{ // check for any ammo change (0-63)
			for (j = 0; j < 4; j++) 
			{
				if (msg.readBit())
				{
					int bits = msg.readShort();
					for (i = 0; i < 16; i++) 
					{
						if (bits & (1 << i))
							to->ammo[i + (j * 16)] = msg.readShort();
					}
				}
			}
		}

		// ammo in clip
		for (j = 0; j < 8; j++) 
		{
			if (msg.readBit()) 
			{
				int bits = msg.readShort();
				for (i = 0; i < 16; i++) 
				{
					if (bits & (1 << i))
						to->ammoclip[i + (j * 16)] = msg.readShort();
				}
			}
		}

		int numObjective = sizeof(from->objective) / sizeof(from->objective[0]);
		if (msg.readBit())
		{
			for (int fieldNum = 0; fieldNum < numObjective; ++fieldNum)
			{
				to->objective[fieldNum].state = static_cast<objectiveState_t>(msg.readBits(3));
				readDeltaObjectiveFields(msg, time, &from->objective[fieldNum], &to->objective[fieldNum]);
			}
		}
		if (msg.readBit())
		{
			readDeltaHudElems(msg, time, from->hud.archival, to->hud.archival, 31);
			readDeltaHudElems(msg, time, from->hud.current, to->hud.current, 31);
		}

		if (msg.readBit())
		{
			for (i = 0; i < 128; ++i)
				to->weaponmodels[i] = msg.readByte();
		}
		std::cout << "Weapon: " << to->weaponmodels << std::endl; // @TODO
	}

	void Demo::readDeltaObjectiveFields(Msg& msg, int time, objective_t* from, objective_t* to)
	{
		if (msg.readBit())
		{
			for (int i = 0; i < OBJECTIVE_FIELDS_COUNT; ++i)
				msg.readDeltaField(time, from, to, &objectiveFields[i], false);
		}
		else
		{
			VectorCopy(from->origin, to->origin);
			to->icon = from->icon;
			to->entNum = from->entNum;
			to->teamNum = from->teamNum;
		}
	}

	void Demo::readDeltaHudElems(Msg& msg, const int time, hudelem_t* from, hudelem_t* to, int count)
	{
		int alignY, alignX, inuse;
		unsigned int lc;
		int i, y;

		inuse = msg.readBits(5);
		for (i = 0; i < inuse; ++i)
		{
			lc = msg.readBits(6);
			for (y = 0; y <= lc; ++y)
				msg.readDeltaField(time, &from[i], &to[i], &hudElemFields[y], false);

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
