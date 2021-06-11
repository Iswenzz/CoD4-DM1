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
		/// Initialize a new Demo object, demo file can be opened with the open() function.
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
		void open(std::string filepath);

		/// <summary>
		/// Close the demo file and free resources.
		/// </summary>
		void close();

	private:
		std::ifstream demo;
		std::string demoFilePath;
		bool isDemoOpen = false;

		std::vector<ClientSnapshotData> snapshots;
		std::vector<ClientArchiveData> archive;

		/// <summary>
		/// Read a server snapshot.
		/// </summary>
		Msg readSnapshot();

		/// <summary>
		/// Read an archive client state.
		/// </summary>
		Msg readArchive();

		/// <summary>
		/// Parse a server snapshot.
		/// </summary>
		/// <param name="msg">The snapshot message.</param>
		void parseSnapshot(Msg &msg);


		/// <summary>
		/// Parse an archive client state.
		/// </summary>
		/// <param name="msg">The archive message.</param>
		void parseArchive(Msg& msg);
	};
}
