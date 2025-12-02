#pragma once
#include "Crypt/Msg.hpp"
#include "Crypt/NetFields.hpp"
#include "Utils/Utility.hpp"

#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>

namespace CoD4::DM1
{
	class Demo
	{
	public:
		std::string Filepath;
		std::ifstream DemoFile;
		std::ofstream DemoFileOut;
		bool IsOpen = false;
		bool IsEOF = true;
		bool IsWriting = false;
		bool Verbose = false;

		int Protocol = COD4_PROTOCOL;
		MSGType CurrentMessageType = {};

		int StartFrameTime = 0;
		int PreviousFrameTime = 0;
		int CurrentFrameTime = 0;
		int StartSnapshotTime = 0;
		int PreviousSnapshotTime = 0;
		int CurrentSnapshotTime = 0;
		int LastFrameSrvMsgSeq = 0;
		int FirstFrameSrvMsgSeq = 0;

		int FPS = 0;
		std::vector<int> FrameTimes{};

		int ClientNum = 0;
		int ChecksumFeed = 0;
		int ServerCommandSequence = 0;
		int ServerConfigSequence = 0;
		int GamestateCount = 0;
		float MapCenter[3] = { 0, 0, 0 };
		bool SendOriginAndVel = true;

		int ServCmdSequence = 0;
		int ParseEntitiesNum = 0;
		int WriteEntitiesNum = 0;
		int ParseClientsNum = 0;
		int WriteClientsNum = 0;
		int SnapMessageNum = 0;

		clientSnapshot_t CurrentSnapshot = { 0 };
		archivedFrame_t CurrentFrame = { 0 };

		std::array<std::string, MAX_CMDSTRINGS> CommandStrings{};
		std::array<std::string, MAX_CONFIGSTRINGS> ValidConfigStrings{};
		std::array<std::string, MAX_CONFIGSTRINGS> ConfigStrings{};

		std::array<entityState_t, MAX_GENTITIES> EntityBaselines{};
		std::array<entityState_t, MAX_PARSE_ENTITIES> ParseEntities{};
		std::array<clientState_t, MAX_PARSE_CLIENTS> ParseClients{};
		std::array<entityState_t, MAX_PARSE_ENTITIES> Entities{};
		std::array<clientState_t, MAX_PARSE_CLIENTS> Clients{};
		std::array<clientNames_t, MAX_CLIENTS> ClientNames{};
		std::array<clientSnapshot_t, PACKET_BACKUP> Snapshots{};
		std::array<archivedFrame_t, MAX_FRAMES> Frames{};

		std::array<uint8_t, MAX_GENTITIES> ActiveBaselines{};
		std::array<uint8_t, MAX_GENTITIES> ActiveEntities{};
		std::array<clientState_t, MAX_CLIENTS> ActiveClients{};

		Demo() = default;
		Demo(std::string filepath);
		Demo(std::string filepath, bool verbose);
		Demo(std::string filepath, bool write, bool verbose);
		virtual ~Demo();

		void Open(std::string filepath);
		void Parse();
		bool Next();
		void Close();

	private:
		Msg CurrentCompressedMsg{};
		Msg CurrentUncompressedMsg{};
		Msg CurrentWritingCompressedMsg{};
		Msg CurrentWritingUncompressedMsg{};

		playerState_t NullPlayerState = { 0 };
		entityState_t NullEntityState = { 0 };
		clientState_t NullClientState = { 0 };

		void ReadMessage();
		void ReadArchive();
		void ReadProtocol();

		void ParseGamestate(Msg& msg);
		void ParseGamestateX(Msg& msg);
		void ParseSnapshot(Msg& msg, int& oldSnapIndex, int& newSnapIndex);
		void ParseCommandString(Msg& msg, int& seq);
		void ParseConfigClient(Msg& msg, uint32_t clientnum);
		int ParsePacketEntities(Msg& msg, const int time, clientSnapshot_t* from, clientSnapshot_t* to);
		void ParsePacketClients(Msg& msg, const int time, clientSnapshot_t* from, clientSnapshot_t* to);

		int ReadDeltaGroundEntity(Msg& msg);
		bool ReadDeltaStruct(Msg& msg, const int time, const void* from, void* to, uint32_t number, int numFields,
			int indexBits, netField_t* stateFields);
		void ReadDeltaFields(Msg& msg, const int time, const uint8_t* from, uint8_t* to, int numFields,
			netField_t* stateFields);
		void ReadDeltaField(Msg& msg, int time, const void* from, const void* to, const netField_t* field, bool noXor,
			bool print);
		bool ReadDeltaEntity(Msg& msg, const int time, entityState_t* from, entityState_t* to, int number);
		bool ReadDeltaClient(Msg& msg, const int time, clientState_t* from, clientState_t* to, int number);
		void ReadDeltaObjectiveFields(Msg& msg, const int time, objective_t* from, objective_t* to);
		void ReadDeltaHudElems(Msg& msg, const int time, hudelem_t* from, hudelem_t* to, int count);
		void ReadDeltaPlayerState(Msg& msg, int time, playerState_t* from, playerState_t* to,
			bool predictedFieldsIgnoreXor);
		int ReadLastChangedField(Msg& msg, int totalFields);
		int ReadEntityIndex(Msg& msg, int indexBits);

		void WriteGamestate(Msg& msg);
		void WriteDeltaEntity(Msg& msg, const int time, entityState_t* from, entityState_t* to, bool force);
		void WriteEntityRemoval(Msg& msg, uint8_t* from, int indexBits, uint8_t changeBit);
		void WriteEntityIndex(Msg& msg, const int index, const int indexBits);
		int WriteEntityDelta(Msg& msg, const int time, const uint8_t* from, const uint8_t* to, bool force,
			int numFields, int indexBits, netField_t* stateFields);
		int WriteDeltaStruct(Msg& msg, const int time, const uint8_t* from, const uint8_t* to, bool force,
			int numFields, int indexBits, netField_t* stateFields, uint8_t bChangeBit);
		void WriteDeltaField(Msg& msg, const int time, const uint8_t* from, const uint8_t* to, netField_t* field,
			int fieldNum, uint8_t forceSend);
		void WriteCommandString(Msg& msg, int seq);
		void WriteConfigClient(Msg& msg, uint32_t clientnum);
		void WriteSnapshot(Msg& msg, int oldSnapIndex, int newSnapIndex);
		void WriteDeltaPlayerState(Msg& msg, const int time, playerState_t* from, playerState_t* to);
		void WriteDeltaObjective(Msg& msg, const int time, objective_t* from, objective_t* to);
		int WriteDeltaLastChangedField(uint8_t* from, uint8_t* to, netField_t* fields, int numFields);
		void WriteDeltaHudElems(Msg& msg, const int time, hudelem_t* from, hudelem_t* to, const int count);
		void WritePacketEntities(Msg& msg, const int time, clientSnapshot_t* oldframe, clientSnapshot_t* newframe);
		void WritePacketClients(Msg& msg, const int time, clientSnapshot_t* oldframe, clientSnapshot_t* newframe);
		void WriteDeltaClient(Msg& msg, const int time, clientState_t* from, clientState_t* to, bool force);
		void WriteClientDelta(Msg& msg, const int time, clientState_t* from, clientState_t* to, bool force,
			int numFields, int indexBits, netField_t* stateFields);

		void DeltaEntity(Msg& msg, const int time, clientSnapshot_t* frame, int newnum, entityState_t* old);
		void DeltaClient(Msg& msg, const int time, clientSnapshot_t* frame, int newnum, clientState_t* old,
			bool unchanged);
		bool DeltaValuesAreEqual(int bits, const int* fromF, const int* toF);

		bool GetPredictedOriginForServerTime(const int time, float* predictedOrigin, float* predictedVelocity,
			float* predictedViewangles, int* bobCycle, int* movementDir);
		bool ShouldSendPSField(bool sendOriginAndVel, playerState_t* from, playerState_t* to, netField_t* field);
		bool CheckSnapshotValidity(clientSnapshot_t& snapshot);
	};
}
