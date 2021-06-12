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
				//case 2: // @TODO
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

			/*unsigned char packet_header = 0;
			demo.read(reinterpret_cast<char*>(&packet_header), sizeof(unsigned char));
			switch (static_cast<int>(msgType))
			{
				case static_cast<int>(MSGType::MSG_SNAPSHOT):
					ParseSnapshot();
					break;
				case static_cast<int>(MSGType::MSG_FRAME):
					ParseArchive();
					break;
			}*/
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

		//std::cout << msg.cursize << std::endl;

		// Read the client message
		msg.cursize -= sizeof(int);
		msg.Initialize(msg.cursize);
		demo.read(reinterpret_cast<char*>(msg.buffer.data()), msg.cursize);
		return msg;
	}

	Msg Demo::ReadArchive()
	{
		int len = 48, swlen = 0;
		demo.read(reinterpret_cast<char*>(&swlen), sizeof(int));            // 21 packet sequence

		std::vector<unsigned char> complen(len);
		demo.read(reinterpret_cast<char*>(complen.data()), len);            // client message
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
		unsigned char header = 0;
		int swlen = 0, len = 0, lastClientCommand = 0, cmd = 0;
		
		/*demo.read(reinterpret_cast<char*>(&swlen), sizeof(int));
		demo.read(reinterpret_cast<char*>(&len), sizeof(int));*/

		msg.Initialize(msg.buffer.data(), msg.cursize, MSGCrypt::MSG_CRYPT_HUFFMAN);

		//// demo ended
		//if (swlen == -1 && len == -1)
		//{
		//	demo.Close();
		//	return;
		//}
		//demo.read(reinterpret_cast<char*>(&lastClientCommand), sizeof(int)); // last client command
		//std::vector<unsigned char> complen(len - 4);
		//demo.read(reinterpret_cast<char*>(complen.data()), complen.size());	// client message

		// Fill the snapshot struct
		/*Msg snap_msg{ complen.data(), complen.size(), MSGCrypt::MSG_CRYPT_HUFFMAN };
		snap.lastClientCommand = lastClientCommand;*/

		while (true)
		{
			if (msg.readcount > msg.cursize)
				break;

			// Read command
			cmd = msg.ReadByte();
			std::cout << "CMD: " << cmd << std::endl;
			for (int i = 0; i < 10; i++)
				std::cout << "readBits: " << msg.ReadByte() << std::endl;
			break;
			switch (cmd)
			{
				case static_cast<int>(svc_ops_e::svc_gamestate):
					msg.ReadGamestate();
					break;
				case static_cast<int>(svc_ops_e::svc_serverCommand):
					msg.ReadCommandString();
					break;
				case static_cast<int>(svc_ops_e::svc_download):
					break;
				case static_cast<int>(svc_ops_e::svc_snapshot):
					msg.ReadSnapshot(Snapshots, snap);
					break;
			}
			if (cmd == static_cast<int>(svc_ops_e::svc_EOF))
				break;
			break; // @TODO
		}

		//std::cout << "Byte Left: " << snap_msg.readcount << "/" << snap_msg.cursize << std::endl;

		// Read the rest
		//std::vector<unsigned char> rest(NETCHAN_FRAGMENTBUFFER_SIZE);
		//msg.ReadData(rest.data(), NETCHAN_FRAGMENTBUFFER_SIZE);

		////std::cin.get();
		//Snapshots.push_back(snap);
	}
}
