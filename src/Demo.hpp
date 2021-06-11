#pragma once
#include <string>
#include <fstream>
#include <vector>

namespace Iswenzz
{
	struct ClientSnapshotData;
	struct ClientArchiveData;
	class Msg;

	/// <summary>
	/// Represent a server demo file .dm_1
	/// </summary>
	class Demo
	{
	public:
		/// <summary>
		/// Initialize a new Demo object, demo file can be opened with the Open() function.
		/// </summary>
		/// <returns></returns>
		Demo();

		/// <summary>
		/// Initialize a new Demo object with the specified demo file path.
		/// </summary>
		/// <param name="filepath"></param>
		/// <returns></returns>
		Demo(std::string filepath);
		~Demo();

		/// <summary>
		/// Open a demo file from specified path.
		/// </summary>
		/// <param name="filepath">File path to a demo file (.dm_1).</param>
		void Open(std::string filepath);

		/// <summary>
		/// Close the demo file and free resources.
		/// </summary>
		void Close();

	private:
		std::ifstream demo;
		std::string demoFilePath;
		bool isDemoOpen = false;

		std::vector<ClientSnapshotData> Snapshots;
		std::vector<ClientArchiveData> Archives;

		/// <summary>
		/// Read a server snapshot.
		/// </summary>
		Msg ReadSnapshot();

		/// <summary>
		/// Read an Archives client state.
		/// </summary>
		Msg ReadArchive();

		/// <summary>
		/// Parse a server snapshot.
		/// </summary>
		/// <param name="msg">The snapshot message.</param>
		void ParseSnapshot(Msg &msg);


		/// <summary>
		/// Parse an Archives client state.
		/// </summary>
		/// <param name="msg">The Archives message.</param>
		void ParseArchive(Msg& msg);
	};
}
