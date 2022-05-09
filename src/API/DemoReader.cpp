#include "DemoReader.hpp"
#include "Utils/Utility.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>

namespace Iswenzz::CoD4::DM1
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

	bool DemoReader::Next()
	{
		// Update previous data for comparisons
		PreviousFrame = GetCurrentFrame();
		PreviousSnapshot = GetCurrentSnapshot();
		PreviousClients = DemoFile->ParseClients;
		PreviousEntities = DemoFile->ParseEntities;

		// Parse demo and update reader fields
		if (DemoFile->Next())
		{
			Frame = GetCurrentFrame();
			Snapshot = GetCurrentSnapshot();

			UpdateClients();
			UpdateEntities();

			return true;
		}
		return false;
	}

	void DemoReader::Parse()
	{
		while (Next());
	}

	void DemoReader::Close()
	{
		DemoFile->Close();
	}

	float DemoReader::GetTime()
	{
		return (DemoFile->CurrentFrameTime - DemoFile->StartFrameTime) / 50.0f / 20.0f;
	}

	float DemoReader::GetTimeMilliseconds()
	{
		return GetTime() * 1000.0f;
	}

	int DemoReader::GetServerTime()
	{
		return DemoFile->CurrentFrameTime;
	}

	clientSnapshot_t DemoReader::GetCurrentSnapshot()
	{
		return DemoFile->CurrentSnapshot;
	}

	archivedFrame_t DemoReader::GetCurrentFrame()
	{
		auto orderedFrames = DemoFile->Frames;
		std::sort(orderedFrames.begin(), orderedFrames.end(),
			[](const archivedFrame_t& a, const archivedFrame_t& b) { return a.commandTime > b.commandTime; });
		return orderedFrames.size() > 0 ? orderedFrames.back() : archivedFrame_t();
	}

	std::vector<clientState_t> DemoReader::GetLastUpdatedClients()
	{
		return Utility::GetArrayDifference<clientState_t>(DemoFile->ParseClients, PreviousClients,
			[](const clientState_t& a, const clientState_t& b) { return a.clientIndex == b.clientIndex; });
	}

	std::vector<entityState_t> DemoReader::GetLastUpdatedEntities()
	{
		return Utility::GetArrayDifference<entityState_t>(DemoFile->ParseEntities, PreviousEntities,
			[](const entityState_t& a, const entityState_t& b) { return a.number == b.number; });
	}

	void DemoReader::UpdateClients()
	{
		for (const clientState_t& client : GetLastUpdatedClients())
			Clients[client.clientIndex] = client;
	}

	void DemoReader::UpdateEntities()
	{
		for (const entityState_t& entity : GetLastUpdatedEntities())
			Entities[entity.number] = entity;
	}

	std::string DemoReader::ReflectDemoValue(const std::string path)
	{
		nlohmann::json json = *this;
		std::string result;

		std::istringstream stream{ path };
		std::string current;

		while (std::getline(stream, current, '.'))
		{
			if (current.find_first_not_of("0123456789") == std::string::npos)
				json = json[std::stoi(current)];
			else
				json = json[current];
		}
		if (json.is_boolean())
			result = json.get<bool>() ? "1" : "0";
		if (json.is_number())
			result = std::to_string(json.get<int>());
		if (json.is_number_float())
			result = std::to_string(json.get<float>());
		if (json.is_string())
			result = json.get<std::string>();
		return result;
	}
}
