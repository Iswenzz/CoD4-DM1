#include "Demo.hpp"
#include "DemoSnapshot.hpp"
#include "DemoFrame.hpp"
#include "EMSGType.hpp"
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
				case MSG_SNAPSHOT:
					readSnapshot();
					break;
				case MSG_FRAME:
					readFrame();
					break;
			}
		}
	}

	void Demo::close()
	{
		if (demo.is_open())
			demo.close();
		demoSnaphots.clear();
		demoFrames.clear();
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

	void Demo::readFrame()
	{
		unsigned char header = 1;
		int archiveIndex, unk1;
		float origin[3], viewangles[3], nullvec[3];
		int nullvelocity, commandTime;
		unsigned char unk3[19];
																			// 1 header
		demo.read(reinterpret_cast<char*>(&archiveIndex), sizeof(int));     // 5
		demo.read(reinterpret_cast<char*>(&origin[0]), sizeof(float));      // 9 player's position
		demo.read(reinterpret_cast<char*>(&origin[1]), sizeof(float));      // 13
		demo.read(reinterpret_cast<char*>(&origin[2]), sizeof(float));      // 17
		demo.read(reinterpret_cast<char*>(&nullvec[0]), sizeof(float));     // 21
		demo.read(reinterpret_cast<char*>(&nullvec[1]), sizeof(float));     // 25
		demo.read(reinterpret_cast<char*>(&nullvec[2]), sizeof(float));     // 29
		demo.read(reinterpret_cast<char*>(&nullvelocity), sizeof(int));     // 33
		demo.read(reinterpret_cast<char*>(&unk1), sizeof(int));             // 37
		demo.read(reinterpret_cast<char*>(&commandTime), sizeof(int));      // 41
		demo.read(reinterpret_cast<char*>(&viewangles[0]), sizeof(float));  // 45 player's angles
		demo.read(reinterpret_cast<char*>(&viewangles[1]), sizeof(float));  // 49
		demo.read(reinterpret_cast<char*>(&viewangles[2]), sizeof(float));  // 53

		std::cout << "Frame " << origin[0] << " " << origin[1] << " " << origin[2] << std::endl;
	}

	void Demo::readSnapshot()
	{
		unsigned char header = 0, unk1[63];
		int swlen, len;
																			   // 1 header
		demo.read(reinterpret_cast<char*>(&swlen), sizeof(int));               // 5 packet sequence
		demo.read(reinterpret_cast<char*>(&len), sizeof(int));                 // 9 client message length

		std::cout << "Snapshot " << swlen << " " << len << std::endl;
		if (swlen == -1 && len == -1)                                          // demo ended
		{
			demo.close();
			return;
		}

		std::vector<unsigned char> complen(len);
		demo.read(reinterpret_cast<char*>(complen.data()), len);               // client message
	}
}
