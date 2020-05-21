#pragma once
#include <string>
#include <fstream>
#include <vector>

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
