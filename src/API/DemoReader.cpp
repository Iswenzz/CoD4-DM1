#include "DemoReader.hpp"
#include "Utils/Utility.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>

namespace CoD4::DM1
{
	DemoReader::DemoReader(std::string filePath) : FilePath(filePath)
	{
		Open(filePath);
	}

	void DemoReader::Open(std::string filePath)
	{
		FilePath = filePath;
		DemoFile = std::make_shared<Demo>(filePath);
	}

	bool DemoReader::IsOpen()
	{
		return DemoFile && DemoFile->IsOpen;
	}

	bool DemoReader::IsEOF()
	{
		return DemoFile && DemoFile->IsEOF;
	}

	bool DemoReader::Next()
	{
		// Update previous data for comparisons
		if (DemoFile->CurrentSnapshot.valid)
		{
			PreviousFrame = GetCurrentFrame();
			PreviousSnapshot = GetCurrentSnapshot();
			PreviousClients = DemoFile->Clients;
			PreviousEntities = DemoFile->Entities;
			PreviousCommandStrings = DemoFile->CommandStrings;
		}

		// Parse demo and update reader fields
		if (DemoFile->Next())
		{
			// Skip not valid snapshot
			if (!DemoFile->CurrentSnapshot.valid)
				return true;

			Frame = GetCurrentFrame();
			Snapshot = GetCurrentSnapshot();
			Entities = DemoFile->Entities;
			Clients = DemoFile->Clients;

			return true;
		}
		return false;
	}

	void DemoReader::Parse()
	{
		while (Next())
			;
	}

	void DemoReader::Close()
	{
		DemoFile->Close();
	}

	float DemoReader::GetTime()
	{
		return (DemoFile->CurrentSnapshotTime - DemoFile->StartSnapshotTime) / 50.0f / 20.0f;
	}

	float DemoReader::GetArchiveTime()
	{
		return (DemoFile->CurrentFrameTime - DemoFile->StartFrameTime) / 50.0f / 20.0f;
	}

	int DemoReader::GetTimeMilliseconds()
	{
		return GetTime() * 1000;
	}

	int DemoReader::GetArchiveTimeMilliseconds()
	{
		return GetArchiveTime() * 1000;
	}

	int DemoReader::GetServerTime()
	{
		return DemoFile->CurrentFrameTime;
	}

	int DemoReader::GetFPS()
	{
		return DemoFile->FPS;
	}

	clientSnapshot_t DemoReader::GetCurrentSnapshot()
	{
		return DemoFile->CurrentSnapshot;
	}

	archivedFrame_t DemoReader::GetCurrentFrame()
	{
		return DemoFile->CurrentFrame;
	}

	std::vector<clientState_t> DemoReader::GetLastUpdatedClients()
	{
		return Utility::GetArrayDifference<clientState_t>(Clients, PreviousClients,
			[](const clientState_t& a, const clientState_t& b) { return !memcmp(&a, &b, sizeof(clientState_t)); });
	}

	std::vector<entityState_t> DemoReader::GetLastUpdatedEntities()
	{
		return Utility::GetArrayDifference<entityState_t>(Entities, PreviousEntities,
			[](const entityState_t& a, const entityState_t& b) { return !memcmp(&a, &b, sizeof(entityState_t)); });
	}

	std::vector<std::string> DemoReader::GetLastCommandStrings()
	{
		return Utility::GetArrayDifference<std::string>(DemoFile->CommandStrings, PreviousCommandStrings,
			[&](const std::string& a, const std::string& b) { return a == b; });
	}

	std::string DemoReader::GetConfigString(const std::string name)
	{
		for (const std::string& config : DemoFile->ConfigStrings)
		{
			if (!config.empty() && !config.find(name))
				return config;
		}
		return "";
	}

	clientNames_t DemoReader::GetPlayerName()
	{
		const clientSnapshot_t& snapshot = GetCurrentSnapshot();
		const int clientNum = snapshot.ps.ClientNum;

		return DemoFile->ClientNames.at(clientNum);
	}

	std::string DemoReader::ParseConfigString(const std::string name)
	{
		for (const std::string& config : DemoFile->ConfigStrings)
		{
			if (config.empty())
				continue;

			std::vector<std::string> tokens = Utility::SplitString(config, '\\');
			for (int i = 1; i < tokens.size(); i += 2)
			{
				std::string key = tokens[i - 1];
				if (key == name)
					return tokens[i];
			}
		}
		return "";
	}
}
