#pragma once
#include "Demo/Demo.hpp"

namespace CoD4::DM1
{
	class DemoReader
	{
	public:
		std::shared_ptr<Demo> DemoFile;
		std::string FilePath;

		clientSnapshot_t Snapshot = { 0 };
		archivedFrame_t Frame = { 0 };
		std::array<entityState_t, MAX_PARSE_ENTITIES> Entities{};
		std::array<clientState_t, MAX_PARSE_CLIENTS> Clients{};

		clientSnapshot_t PreviousSnapshot = { 0 };
		archivedFrame_t PreviousFrame = { 0 };
		std::array<entityState_t, MAX_PARSE_ENTITIES> PreviousEntities{};
		std::array<clientState_t, MAX_PARSE_CLIENTS> PreviousClients{};
		std::array<std::string, MAX_CMDSTRINGS> PreviousCommandStrings{};

		DemoReader() = default;
		DemoReader(std::string filePath);
		virtual ~DemoReader() = default;

		void Open(std::string filePath);
		bool IsOpen();
		bool IsEOF();

		bool Next();
		void Parse();
		void Close();

		float GetTime();
		float GetArchiveTime();
		int GetTimeMilliseconds();
		int GetArchiveTimeMilliseconds();
		int GetServerTime();
		int GetFPS();

		clientSnapshot_t GetCurrentSnapshot();
		archivedFrame_t GetCurrentFrame();
		std::vector<clientState_t> GetLastUpdatedClients();
		std::vector<entityState_t> GetLastUpdatedEntities();
		std::vector<std::string> GetLastCommandStrings();
		std::string GetConfigString(const std::string name);
		clientNames_t GetPlayerName();

		std::string ParseConfigString(const std::string name);
	};
}
