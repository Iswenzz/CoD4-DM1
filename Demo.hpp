#pragma once
#include <string>
#include <fstream>
#include <vector>

namespace Iswenzz
{
	struct ClientSnapshotData;
	struct ClientArchiveData;
	typedef struct playerState_s playerState_t;
	typedef struct objective_s objective_t;
	typedef struct hudelem_s hudelem_t;
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
			void readSnapshot(Msg& msg, ClientSnapshotData& snap);
			void readDeltaObjectiveFields(Msg& msg, int time, objective_t* from, objective_t* to);
			void readDeltaHudElems(Msg &msg, const int time, hudelem_t* from, hudelem_t* to, int count);
			void readDeltaPlayerState(Msg& msg, int time, playerState_t* from, playerState_t* to,
				bool predictedFieldsIgnoreXor);
	};
}
