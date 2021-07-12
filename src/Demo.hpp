#pragma once
#include "Msg.hpp"
#include "NetFields.hpp"

#include <string>
#include <fstream>

namespace Iswenzz
{
	/// <summary>
	/// Represent a server demo file .dm_1
	/// </summary>
	class Demo
	{
	public:
		/// <summary>
		/// Initialize a new Demo object with the specified demo file path, output file path and verbosity.
		/// </summary>
		/// <param name="inputFilePath">File path to a demo file (.dm_1)</param>
		/// <param name="outputFilePath">Output path, file will be filled with all frames</param>
		/// <param name="verbose">Prints debug informations.</param>
		/// <returns></returns>
		Demo(std::string inputFilePath, std::string outputFilePath, bool verbose = false);
		~Demo();

	private:
		std::ifstream DemoFile;
		std::ofstream OutputFile;
		bool Verbose;

		int Protocol = 16;

		Msg CurrentCompressedMsg = { };
		Msg CurrentUncompressedMsg = { };
		Msg CurrentWritingMsg = { };

		bool MatchInProgress = false;
		int StartFrameTime = 0;
		int CurrentFrameTime = 0;
		int LastFrameSrvMsgSeq = 0;
		int FirstFrameSrvMsgSeq = 0;

		int MatchNum = 0;
		int RoundNum = 0;
		int ClientNum = 0;
		int TeamNum = 0;
		int PlayerClientNum = 0;
		int PlayerClientNumOld = 0;
		int KillCamEntity = 0;
		int KillCamEntityOld = 0;
		bool DemoStartedInKillCam = 0;
		int ChecksumFeed = 0;
		int ServerCommandSequence = 0;
		clientSnapshot_t CurrentSnapshot = { 0 };
		float LerpPosOffsets[3] = { 0, 0, 0 };
		int IsNewRound = 0;

		bool ModDM = false;
		std::string TeamNameAllies;
		std::string TeamScoreAllies;
		std::string TeamNameAxis;
		std::string TeamScoreAxis;
		std::string DefaultWeapon_mp;
		std::string FogParams;

		int ServCmdSequence = 0;
		int ParseEntitiesNum = 0;
		int WriteEntitiesNum = 0;
		int ParseClientsNum = 0;
		int WriteClientsNum = 0;
		int SnapMessageNum = 0;

		playerState_t NullPlayerState = { 0 };
		entityState_t NullEntityState = { 0 };
		clientState_t NullClientState = { 0 };

		std::array<std::string, MAX_CMDSTRINGS> CommandStrings{ };
		std::array<std::string, MAX_CONFIGSTRINGS> ValidConfigStrings{ };
		std::array<std::string, MAX_CONFIGSTRINGS> ConfigStrings{ };
		std::array<entityState_t, MAX_GENTITIES> EntityBaselines{ };
		std::array<entityState_t, MAX_PARSE_ENTITIES> ParseEntities{ };
		std::array<clientState_t, MAX_PARSE_CLIENTS> ParseClients{ };
		std::array<clientSnapshot_t, PACKET_BACKUP> Snapshots{ };
		std::array<archivedFrame_t, MAX_FRAMES> Frames{ };

		std::array<unsigned char, MAX_GENTITIES> ActiveBaselines{ };
		std::array<unsigned char, MAX_GENTITIES> ActiveEntities{ };
		std::array<clientState_t, MAX_CLIENTS> ActiveClients{ };

		/// <summary>
		/// Start parsing the demo file
		/// </summary>
		void Parse();

		/// <summary>
		/// Dump some interesting info from the struct to the console
		/// </summary>
		void Dump(archivedFrame_t &rFrame);

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
		void ParseSnapshot(Msg& msg);

		/// <summary>
		/// Parse a command string.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		void ParseCommandString(Msg& msg);

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
		void ReadDeltaStruct(Msg& msg, const int time, const void* from, void* to,
			unsigned int number, int numFields, int indexBits, netField_t* stateFields);

		/// <summary>
		/// Read all delta compressed net fields.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		/// <param name="numFields">Struct field count.</param>
		/// <param name="stateFields">Netfield fields.</param>
		void ReadDeltaFields(Msg& msg, const int time, const unsigned char* from, unsigned char* to,
			int numFields, netField_t* stateFields);

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
		void ReadDeltaField(Msg& msg, int time, const void* from, const void* to, const netField_t* field,
			bool noXor, bool print);

		/// <summary>
		/// Read a delta compressed entity state.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		/// <param name="number">Entity number.</param>
		/// <param name="isBaseLine">Is it a baseline entity.</param>
		void ReadDeltaEntity(Msg& msg, const int time, entityState_t* from, entityState_t* to, int number);

		/// <summary>
		/// Read a delta compressed client state.
		/// </summary>
		/// <param name="msg">The current uncompressed message.</param>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		/// <param name="number">Entity number.</param>
		void ReadDeltaClient(Msg& msg, const int time, clientState_t* from, clientState_t* to, int number);

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
		int ReadEntityIndex(Msg &msg, int indexBits);

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
		void DeltaClient(Msg& msg, const int time, clientSnapshot_t* frame, int newnum,
			clientState_t* old, bool unchanged);
	};
}
