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
		ClientSnapshotData snap;
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

		std::cin.get();
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

		std::memcpy(&snap.cl_snap, frame, sizeof(clientSnapshot_t));
		delete oldFrame;
		delete frame;
	}

	void Demo::readDeltaPlayerState(Msg& msg, int time, playerState_t* from, playerState_t* to, 
		bool predictedFieldsIgnoreXor)
	{
		std::memcpy(to, from, sizeof(playerState_t));

		bool readOriginAndVel = msg.readBit() > 0;
		int lastChangedField = msg.readBits(GetMinBitCount(PLAYER_STATE_FIELDS_COUNT));
		
		std::cout << "LC: " << lastChangedField << std::endl;

		netField_t *stateFields = playerStateFields;
		for (int i = 0; i < lastChangedField; ++i)
		{
			bool noXor = predictedFieldsIgnoreXor && readOriginAndVel && stateFields[i].changeHints == 3;
			msg.readDeltaField(time, from, to, &stateFields[i], noXor);
		}

		// @TODO
		std::cout << "Origin: " 
			<< to->origin[0] << " " << to->origin[1] << " " << to->origin[2]
			<< std::endl;
		std::cout << "Velocity: "
			<< to->velocity[0] << " " << to->velocity[1] << " " << to->velocity[2]
			<< std::endl;
	}

	void Demo::readGameState(Msg& msg)
	{

	}

	void Demo::readMatchState(Msg& msg, int time)
	{
		unsigned int entityIndex = msg.readEntityIndex(1);
	}
}
