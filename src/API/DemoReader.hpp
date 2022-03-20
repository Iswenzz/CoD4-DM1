#pragma once
#include "Demo/Demo.hpp"

namespace Iswenzz
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
		clientSnapshot_t PreviousSnapshot = { 0 };

		std::shared_ptr<std::array<entityState_t, MAX_PARSE_ENTITIES>> Entities;
		std::shared_ptr<std::array<entityState_t, MAX_PARSE_ENTITIES>> PreviousEntities;

		std::shared_ptr<std::array<clientState_t, MAX_PARSE_CLIENTS>> Clients;
		std::shared_ptr<std::array<clientState_t, MAX_PARSE_CLIENTS>> PreviousClients;

		std::shared_ptr<std::array<archivedFrame_t, MAX_FRAMES>> Frames;
		std::shared_ptr<std::array<archivedFrame_t, MAX_FRAMES>> PreviousFrames;

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
		/// Reads the next demo message.
		/// </summary>
		/// <returns></returns>
		bool Next();

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
}
