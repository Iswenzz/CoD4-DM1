#pragma once
#include <string>
#include <fstream>
#include <vector>

namespace Iswenzz
{
	struct ClientSnapshotData;
	struct ClientArchiveData;
	typedef struct playerState_s playerState_t;
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

			void readCommandString(Msg &msg);
			void readSnapshot(Msg& msg);
			void readGameState(Msg& msg);
			void readMatchState(Msg& msg, int time);
			void readDeltaPlayerState(Msg& msg, int time, playerState_t* from, playerState_t* to,
				bool predictedFieldsIgnoreXor);
	};
}
