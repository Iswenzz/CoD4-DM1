#pragma once
#include <string>
#include <fstream>
#include <vector>

namespace Iswenzz
{
	struct ClientSnapshotData;
	struct ClientArchiveData;
	class Msg;

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

			std::vector<ClientSnapshotData> snapshots;
			std::vector<ClientArchiveData> archive;

			void readHeader();
			void readArchive();
			void readSnapshot();

			void readCommandString(Msg *msg);
			void readSnapshot(Msg* msg);
			void readGamestate(Msg* msg);
	};
}
