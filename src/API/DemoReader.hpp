#pragma once
#include "Demo/Demo.hpp"
#include <nlohmann/json.hpp>

namespace Iswenzz::CoD4::DM1
{
	/// <summary>
	/// Reader API for demo files. (.DM_1)
	/// </summary>
	class DemoReader
	{
	public:
		std::shared_ptr<Demo> DemoFile;
		std::string FilePath;

		clientSnapshot_t Snapshot = { 0 };
		archivedFrame_t Frame = { 0 };
		std::array<entityState_t, MAX_PARSE_ENTITIES> Entities{ };
		std::array<clientState_t, MAX_PARSE_CLIENTS> Clients{ };

		clientSnapshot_t PreviousSnapshot = { 0 };
		archivedFrame_t PreviousFrame = { 0 };
		std::array<entityState_t, MAX_PARSE_ENTITIES> PreviousEntities{ };
		std::array<clientState_t, MAX_PARSE_CLIENTS> PreviousClients{ };

		/// <summary>
		/// Initialize a new DemoReader instance.
		/// </summary>
		DemoReader() = default;

		/// <summary>
		/// Initialize a new DemoReader instance.
		/// </summary>
		/// <param name="filePath">The file path of the demo file.</param>
		DemoReader(std::string filePath);

		/// <summary>
		/// Dispose all resources.
		/// </summary>
		virtual ~DemoReader() = default;

		/// <summary>
		/// Open a demo file from specified path.
		/// </summary>
		/// <param name="filePath">File path to a demo file (.dm_1).</param>
		void Open(std::string filePath);

		/// <summary>
		/// Check if the demo is open.
		/// </summary>
		bool IsOpen();

		/// <summary>
		/// Reads the next demo message.
		/// </summary>
		/// <returns></returns>
		bool Next();

		/// <summary>
		/// Parse the entire demo file.
		/// </summary>
		void Parse();

		/// <summary>
		/// Close the demo file and free resources.
		/// </summary>
		void Close();

		/// <summary>
		/// Get the demo time.
		/// </summary>
		/// <returns></returns>
		float GetTime();

		/// <summary>
		/// Get the demo time in milliseconds.
		/// </summary>
		/// <returns></returns>
		float GetTimeMilliseconds();

		/// <summary>
		/// Get the server time.
		/// </summary>
		/// <returns></returns>
		int GetServerTime();

		/// <summary>
		/// Get the current snapshot data.
		/// </summary>
		/// <returns></returns>
		clientSnapshot_t GetCurrentSnapshot();

		/// <summary>
		/// Get the current archived frame.
		/// </summary>
		/// <returns></returns>
		archivedFrame_t GetCurrentFrame();

		/// <summary>
		/// Get the last updated clients.
		/// </summary>
		/// <returns></returns>
		std::vector<clientState_t> GetLastUpdatedClients();

		/// <summary>
		/// Get the last updated entities.
		/// </summary>
		/// <returns></returns>
		std::vector<entityState_t> GetLastUpdatedEntities();

		/// <summary>
		/// Reflect demo variables from a path.
		/// i.e: Snapshot.origin.0
		/// i.e: Snapshot.origin.1
		/// i.e: Frame.commandTime
		/// </summary>
		/// <param name="path"></param>
		/// <returns></returns>
		std::string ReflectDemoValue(const std::string path);

	private:
		/// <summary>
		/// Update the reader clients field.
		/// </summary>
		void UpdateClients();

		/// <summary>
		/// Update the reader entities field.
		/// </summary>
		void UpdateEntities();
	};

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DemoReader, FilePath, Snapshot, Frame, Entities, Clients);
}
