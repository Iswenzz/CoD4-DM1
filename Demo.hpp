#pragma once
#include <string>
#include <fstream>
#include <vector>

#define FS_DEMOWRITEBUF_SIZE 0x20000
#define SYS_COMMONVERSION 17.5
#define	PROTOCOL_VERSION (unsigned int)(SYS_COMMONVERSION + 0.00001)

namespace Iswenzz
{
	struct DemoSnapshot;
	struct DemoFrame;

	class Demo
	{
		public:
			Demo();
			Demo(std::string filepath);
			~Demo();

			void open(std::string filepath);
			void close();

		private:
			std::ifstream demo;
			std::string demoFilePath;
			bool isDemoOpen = false;

			std::vector<DemoSnapshot> demoSnaphots;
			std::vector<DemoFrame> demoFrames;

			void readHeader();
			void readFrame();
			void readSnapshot();
	};
}
