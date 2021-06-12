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
		Open(filepath);
	}

	Demo::~Demo()
	{
		Close();
	}

	void Demo::Open(std::string filepath)
	{
		if (isDemoOpen)
			Close();
		isDemoOpen = true;
		demoFilePath = filepath;

		demo.open(filepath, std::ios::binary);
		while (demo.is_open())
		{
			char msgType = 0;
			demo.read(reinterpret_cast<char*>(&msgType), sizeof(char));

			switch (msgType)
			{
				case static_cast<int>(MSGType::MSG_SNAPSHOT):
				{
					Msg snapshotMsg = ReadSnapshot();
					ParseSnapshot(snapshotMsg);
					break;
				}
				case static_cast<int>(MSGType::MSG_FRAME):
				{
					Msg archiveMsg = ReadArchive();	
					ParseArchive(archiveMsg);
					break;
				}
			}
			break; // @TODO
		}
	}

	void Demo::Close()
	{
		if (demo.is_open())
			demo.close();
		Snapshots.clear();
		Archives.clear();
		isDemoOpen = false;
	}

	Msg Demo::ReadSnapshot()
	{
		Msg msg{ };

		unsigned char slen = 0;
		int protocol = 0, messageLength = 0, packetSequence = 0;
		int dummyend[3] = { };

		// CoD4X
		//demo.read(reinterpret_cast<char*>(&protocol), sizeof(int));
		//demo.read(reinterpret_cast<char*>(&dummyend[0]), sizeof(int)); // datalen (-1 = demo ended)
		//demo.read(reinterpret_cast<char*>(&dummyend[1]), sizeof(int));
		//demo.read(reinterpret_cast<char*>(&dummyend[2]), sizeof(int));
		//demo.read(reinterpret_cast<char*>(&slen), sizeof(unsigned char));
		//demo.read(reinterpret_cast<char*>(&packetSequence), sizeof(int));
		//demo.read(reinterpret_cast<char*>(&msg.maxsize), sizeof(int));

		// 1.7
		demo.read(reinterpret_cast<char*>(&packetSequence), sizeof(int));
		demo.read(reinterpret_cast<char*>(&msg.cursize), sizeof(int));
		demo.read(reinterpret_cast<char*>(&dummyend[0]), sizeof(int));

		// Read the client message
		msg.cursize -= sizeof(int);
		msg.Initialize(msg.cursize);
		demo.read(reinterpret_cast<char*>(msg.buffer.data()), msg.cursize);
		return msg;
	}

	Msg Demo::ReadArchive()
	{
		int len = 48, swlen = 0;
		demo.read(reinterpret_cast<char*>(&swlen), sizeof(int));

		std::vector<unsigned char> complen(len);
		demo.read(reinterpret_cast<char*>(complen.data()), len);
		return Msg{ nullptr, 0, MSGCrypt::MSG_CRYPT_NONE };
	}

	void Demo::ParseArchive(Msg& msg)
	{
		ClientArchiveData buffer{ };
		demo.read(reinterpret_cast<char*>(&buffer.index), sizeof(int));
		demo.read(reinterpret_cast<char*>(&buffer.origin[0]), sizeof(float));
		demo.read(reinterpret_cast<char*>(&buffer.origin[1]), sizeof(float));
		demo.read(reinterpret_cast<char*>(&buffer.origin[2]), sizeof(float));
		demo.read(reinterpret_cast<char*>(&buffer.velocity[0]), sizeof(float));
		demo.read(reinterpret_cast<char*>(&buffer.velocity[1]), sizeof(float));
		demo.read(reinterpret_cast<char*>(&buffer.velocity[2]), sizeof(float));
		demo.read(reinterpret_cast<char*>(&buffer.movementDir), sizeof(int));
		demo.read(reinterpret_cast<char*>(&buffer.bobCycle), sizeof(int));
		demo.read(reinterpret_cast<char*>(&buffer.commandTime), sizeof(int));
		demo.read(reinterpret_cast<char*>(&buffer.angles[0]), sizeof(float));
		demo.read(reinterpret_cast<char*>(&buffer.angles[1]), sizeof(float));
		demo.read(reinterpret_cast<char*>(&buffer.angles[2]), sizeof(float));
		Archives.push_back(buffer);
	}

	void Demo::ParseSnapshot(Msg& msg)
	{
		ClientSnapshotData snap{ };
		Msg decMsg{ msg.buffer.data(), msg.cursize, MSGCrypt::MSG_CRYPT_HUFFMAN };

		unsigned char header = 0;
		int cmd = 0;

		while (true)
		{
			if (decMsg.readcount > decMsg.cursize)
				break;

			// Read command
			cmd = decMsg.ReadByte();
			switch (cmd)
			{
				case static_cast<int>(svc_ops_e::svc_gamestate):
					decMsg.ReadGamestate();
					break;
				case static_cast<int>(svc_ops_e::svc_serverCommand):
					decMsg.ReadCommandString();
					break;
				case static_cast<int>(svc_ops_e::svc_download):
					break;
				case static_cast<int>(svc_ops_e::svc_snapshot):
					decMsg.ReadSnapshot(Snapshots, snap);
					break;
			}
			if (cmd == static_cast<int>(svc_ops_e::svc_EOF))
				break;
			break; // @TODO
		}

		//std::cout << "Byte Left: " << snap_msg.readcount << "/" << snap_msg.cursize << std::endl;
		Snapshots.push_back(snap);
	}
}
