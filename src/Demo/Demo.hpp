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
	/// <summary>
	/// Represent a demo file .DM_1
	/// </summary>
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

		/// <summary>
		/// Initialize a new Demo object, demo file can be opened with the Open() function.
		/// </summary>
		/// <returns></returns>
		Demo() = default;

		/// <summary>
		/// Initialize a new Demo object with the specified demo file path.
		/// </summary>
		/// <param name="filepath">File path to a demo file (.dm_1)</param>
		/// <param name="verbose">Prints debug informations.</param>
		/// <returns></returns>
		Demo(std::string filepath);

		/// <summary>
		/// Initialize a new Demo object with the specified demo file path.
		/// </summary>
		/// <param name="filepath">File path to a demo file (.dm_1)</param>
		/// <param name="verbose">Prints debug informations.</param>
		/// <returns></returns>
		Demo(std::string filepath, bool verbose);

		/// <summary>
		/// Initialize a new Demo object with the specified demo file path.
		/// </summary>
		/// <param name="filepath">File path to a demo file (.dm_1)</param>
		/// <param name="write">Rewrite demo.</param>
		/// <param name="verbose">Prints debug informations.</param>
		/// <returns></returns>
		Demo(std::string filepath, bool write, bool verbose);

		/// <summary>
		/// Dispose all resources.
		/// </summary>
		virtual ~Demo();

		/// <summary>
		/// Open a demo file from specified path.
		/// </summary>
		/// <param name="filepath">File path to a demo file (.dm_1).</param>
		void Open(std::string filepath);

		/// <summary>
		/// Parse the opened demo file.
		/// </summary>
		void Parse();

		/// <summary>
		/// Reads the next demo message.
		/// </summary>
		/// <returns></returns>
		bool Next();

		/// <summary>
		/// Close the demo file and free resources.
		/// </summary>
		void Close();

	private:
		Msg CurrentCompressedMsg{};
		Msg CurrentUncompressedMsg{};
		Msg CurrentWritingCompressedMsg{};
		Msg CurrentWritingUncompressedMsg{};

		playerState_t NullPlayerState = { 0 };
		entityState_t NullEntityState = { 0 };
		clientState_t NullClientState = { 0 };

		/// <summary>
		/// Read the message.
		/// </summary>
		void ReadMessage();

		/// <summary>
		/// Read the archive.
		/// </summary>
		void ReadArchive();

		/// <summary>
		/// Read the protocol.
		/// </summary>
		void ReadProtocol();

		/// <summary>
		/// Parse a gamestate.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		void ParseGamestate(Msg& msg);

		/// <summary>
		/// Parse a CoD4X gamestate.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		void ParseGamestateX(Msg& msg);

		/// <summary>
		/// Parse a server snapshot.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		void ParseSnapshot(Msg& msg, int& oldSnapIndex, int& newSnapIndex);

		/// <summary>
		/// Parse a command string.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		void ParseCommandString(Msg& msg, int& seq);

		/// <summary>
		/// Parse a config client.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		void ParseConfigClient(Msg& msg, uint32_t clientnum);

		/// <summary>
		/// Read a delta compressed ground entity.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <returns></returns>
		int ReadDeltaGroundEntity(Msg& msg);

		/// <summary>
		/// Read a delta compressed struct.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">The server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		/// <param name="number">Entity number.</param>
		/// <param name="numFields">Struct field count.</param>
		/// <param name="indexBits">Min bit count.</param>
		/// <param name="stateFields">Netfield fields.</param>
		bool ReadDeltaStruct(Msg& msg, const int time, const void* from, void* to, uint32_t number, int numFields,
			int indexBits, netField_t* stateFields);

		/// <summary>
		/// Read all delta compressed net fields.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		/// <param name="numFields">Struct field count.</param>
		/// <param name="stateFields">Netfield fields.</param>
		void ReadDeltaFields(Msg& msg, const int time, const uint8_t* from, uint8_t* to, int numFields,
			netField_t* stateFields);

		/// <summary>
		/// Read a delta compressed net field.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		/// <param name="field">Current netfield to read.</param>
		/// <param name="noXor">Should start with a value of 0.</param>
		/// <param name="print">Should print debug information.</param>
		void ReadDeltaField(Msg& msg, int time, const void* from, const void* to, const netField_t* field, bool noXor,
			bool print);

		/// <summary>
		/// Read a delta compressed entity state.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		/// <param name="number">Entity number.</param>
		/// <param name="isBaseLine">Is it a baseline entity.</param>
		bool ReadDeltaEntity(Msg& msg, const int time, entityState_t* from, entityState_t* to, int number);

		/// <summary>
		/// Read a delta compressed client state.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		/// <param name="number">Entity number.</param>
		bool ReadDeltaClient(Msg& msg, const int time, clientState_t* from, clientState_t* to, int number);

		/// <summary>
		/// Read a delta compressed objective struct.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		void ReadDeltaObjectiveFields(Msg& msg, const int time, objective_t* from, objective_t* to);

		/// <summary>
		/// Read all delta compressed hud element struct.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		/// <param name="count">HUD Count.</param>
		void ReadDeltaHudElems(Msg& msg, const int time, hudelem_t* from, hudelem_t* to, int count);

		/// <summary>
		/// Read a delta compressed player state.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		/// <param name="predictedFieldsIgnoreXor">Should start with a value of 0.</param>
		void ReadDeltaPlayerState(Msg& msg, int time, playerState_t* from, playerState_t* to,
			bool predictedFieldsIgnoreXor);

		/// <summary>
		/// Parse all entities.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old snapshot state.</param>
		/// <param name="to">Pointer to the new snapshot state.</param>
		/// <returns></returns>
		int ParsePacketEntities(Msg& msg, const int time, clientSnapshot_t* from, clientSnapshot_t* to);

		/// <summary>
		/// Parse all clients.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old snapshot state.</param>
		/// <param name="to">Pointer to the new snapshot state.</param>
		void ParsePacketClients(Msg& msg, const int time, clientSnapshot_t* from, clientSnapshot_t* to);

		/// <summary>
		/// Read last changed net field.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="totalFields">Net field count.</param>
		/// <returns>The last changed field index.</returns>
		int ReadLastChangedField(Msg& msg, int totalFields);

		/// <summary>
		/// Read the entity bits.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="bits">Entity bits to read.</param>
		/// <returns></returns>
		int ReadEntityIndex(Msg& msg, int indexBits);

		/// <summary>
		/// Parse a delta entity.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">Server time.</param>
		/// <param name="frame">The old snapshot frame.</param>
		/// <param name="newnum">The entity num.</param>
		/// <param name="old">The old entity state.</param>
		void DeltaEntity(Msg& msg, const int time, clientSnapshot_t* frame, int newnum, entityState_t* old);

		/// <summary>
		/// Parse a delta client.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">Server time.</param>
		/// <param name="frame">The old snapshot frame.</param>
		/// <param name="newnum">The entity num.</param>
		/// <param name="old">The old client state.</param>
		/// <param name="unchanged">Should not update.</param>
		void DeltaClient(Msg& msg, const int time, clientSnapshot_t* frame, int newnum, clientState_t* old,
			bool unchanged);

		/// <summary>
		/// Get a predicted origin for server time.
		/// </summary>
		/// <param name="time">Server time.</param>
		/// <param name="predictedOrigin">The origin to predict.</param>
		/// <param name="predictedVelocity">The velocity to predict.</param>
		/// <param name="predictedViewangles">The viewangles to predict.</param>
		/// <param name="bobCycle">Bob cycle value</param>
		/// <param name="movementDir">Movement direction.</param>
		bool GetPredictedOriginForServerTime(const int time, float* predictedOrigin, float* predictedVelocity,
			float* predictedViewangles, int* bobCycle, int* movementDir);

		/// <summary>
		/// Write a gamestate.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		void WriteGamestate(Msg& msg);

		/// <summary>
		/// Write a delta entity.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">The server time.</param>
		/// <param name="from">Pointer to an old entity state.</param>
		/// <param name="to">Pointer to the new entity state.</param>
		/// <param name="force">Force updating fields.</param>
		void WriteDeltaEntity(Msg& msg, const int time, entityState_t* from, entityState_t* to, bool force);

		/// <summary>
		/// Write entity removal.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="from">Pointer to an old entity.</param>
		/// <param name="indexBits">Index bits.</param>
		/// <param name="changeBit">Change bits.</param>
		void WriteEntityRemoval(Msg& msg, uint8_t* from, int indexBits, uint8_t changeBit);

		/// <summary>
		/// Write entity index.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="index">The entity index.</param>
		/// <param name="indexBits">Index bits.</param>
		void WriteEntityIndex(Msg& msg, const int index, const int indexBits);

		/// <summary>
		/// Write entity delta.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">The server time.</param>
		/// <param name="from">Pointer to an old entity.</param>
		/// <param name="to">Pointer to the new entity.</param>
		/// <param name="force">Force updating fields.</param>
		/// <param name="numFields">Number of fields.</param>
		/// <param name="indexBits">Index bits.</param>
		/// <param name="stateFields">Net fields to update.</param>
		/// <returns></returns>
		int WriteEntityDelta(Msg& msg, const int time, const uint8_t* from, const uint8_t* to, bool force,
			int numFields, int indexBits, netField_t* stateFields);

		/// <summary>
		/// Write delta struct.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">The server time.</param>
		/// <param name="from">Pointer to an old entity.</param>
		/// <param name="to">Pointer to the new entity.</param>
		/// <param name="force">Force updating fields.</param>
		/// <param name="numFields">Number of fields.</param>
		/// <param name="indexBits">Index bits.</param>
		/// <param name="stateFields">Net fields to update.</param>
		/// <param name="bChangeBit">Change bits.</param>
		/// <returns></returns>
		int WriteDeltaStruct(Msg& msg, const int time, const uint8_t* from, const uint8_t* to, bool force,
			int numFields, int indexBits, netField_t* stateFields, uint8_t bChangeBit);

		/// <summary>
		/// Check if delta values are equal.
		/// </summary>
		/// <param name="bits">Delta bits.</param>
		/// <param name="fromF">Pointer to old variable.</param>
		/// <param name="toF">Pointer to new variable.</param>
		/// <returns></returns>
		bool DeltaValuesAreEqual(int bits, const int* fromF, const int* toF);

		/// <summary>
		/// Write delta field.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">The server time.</param>
		/// <param name="from">Pointer to an old entity.</param>
		/// <param name="to">Pointer to the new entity.</param>
		/// <param name="field">The net field.</param>
		/// <param name="fieldNum">The net field index.</param>
		/// <param name="forceSend">Should force updating.</param>
		void WriteDeltaField(Msg& msg, const int time, const uint8_t* from, const uint8_t* to, netField_s* field,
			int fieldNum, uint8_t forceSend);

		/// <summary>
		/// Write a command string.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="seq">Message sequence.</param>
		void WriteCommandString(Msg& msg, int seq);

		/// <summary>
		/// Write a config client.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="clientnum">The client num.</param>
		void WriteConfigClient(Msg& msg, uint32_t clientnum);

		/// <summary>
		/// Write a snapshot.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="oldSnapIndex">Old snapshot index.</param>
		/// <param name="newSnapIndex">New snapshot index.</param>
		void WriteSnapshot(Msg& msg, int oldSnapIndex, int newSnapIndex);

		/// <summary>
		/// Write delta player state.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">The server time.</param>
		/// <param name="from">Pointer to old player state.</param>
		/// <param name="to">Pointer to new player state.</param>
		void WriteDeltaPlayerState(Msg& msg, const int time, playerState_t* from, playerState_t* to);

		/// <summary>
		/// Write delta objective.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">The server time.</param>
		/// <param name="from">Pointer to old objective.</param>
		/// <param name="to">Pointer to new objective.</param>
		void WriteDeltaObjective(Msg& msg, const int time, objective_t* from, objective_t* to);

		/// <summary>
		/// Write delta last changed field.
		/// </summary>
		/// <param name="from">Pointer to old struct.</param>
		/// <param name="to">Pointer to new struct.</param>
		/// <param name="fields">The net fields.</param>
		/// <param name="numFields">The net field count.</param>
		/// <returns></returns>
		int WriteDeltaLastChangedField(uint8_t* from, uint8_t* to, netField_t* fields, int numFields);

		/// <summary>
		/// Write delta hud elements.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">The server time.</param>
		/// <param name="from">Pointer to old hudelem.</param>
		/// <param name="to">Pointer to new hudelem.</param>
		/// <param name="count">The field count.</param>
		void WriteDeltaHudElems(Msg& msg, const int time, hudelem_t* from, hudelem_t* to, const int count);

		/// <summary>
		/// Write packet entities.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">The server time.</param>
		/// <param name="oldframe">Pointer to old client snapshot.</param>
		/// <param name="newframe">Pointer to new client snapshot.</param>
		void WritePacketEntities(Msg& msg, const int time, clientSnapshot_t* oldframe, clientSnapshot_t* newframe);

		/// <summary>
		/// Write packet clients.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">The server time.</param>
		/// <param name="oldframe">Pointer to old client snapshot.</param>
		/// <param name="newframe">Pointer to new client snapshot.</param>
		void WritePacketClients(Msg& msg, const int time, clientSnapshot_t* oldframe, clientSnapshot_t* newframe);

		/// <summary>
		/// Write delta client.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">The server time.</param>
		/// <param name="from">Pointer to old client state.</param>
		/// <param name="to">Pointer to new client state.</param>
		/// <param name="force">Should force updating.</param>
		void WriteDeltaClient(Msg& msg, const int time, clientState_t* from, clientState_t* to, bool force);

		/// <summary>
		/// Write client delta.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">The server time.</param>
		/// <param name="from">Pointer to old client state.</param>
		/// <param name="to">Pointer to new client state.</param>
		/// <param name="force">Should force updating.</param>
		/// <param name="numFields">The fields count.</param>
		/// <param name="indexBits">Index bits.</param>
		/// <param name="stateFields">The net fields.</param>
		void WriteClientDelta(Msg& msg, const int time, clientState_t* from, clientState_t* to, bool force,
			int numFields, int indexBits, netField_t* stateFields);

		/// <summary>
		/// Should send player state field.
		/// </summary>
		/// <param name="sendOriginAndVel">Should send origin and velocity.</param>
		/// <param name="from">Pointer to old player state.</param>
		/// <param name="to">Pointer to new player state.</param>
		/// <param name="field">The player state field to check.</param>
		/// <returns></returns>
		bool ShouldSendPSField(bool sendOriginAndVel, playerState_t* from, playerState_t* to, netField_t* field);

		/// <summary>
		/// Check the snapshot validity.
		/// </summary>
		/// <param name="snapshot">The snapshot.</param>
		/// <returns></returns>
		bool CheckSnapshotValidity(clientSnapshot_t& snapshot);
	};
}
