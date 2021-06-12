#pragma once
#include <string>
#include <vector>

#define NETCHAN_UNSENTBUFFER_SIZE		0x20000
#define NETCHAN_FRAGMENTBUFFER_SIZE		0x800
#define NETCHAN_MAXBUFFER_SIZE			NETCHAN_UNSENTBUFFER_SIZE * 10
#define SYS_COMMONVERSION				17.5
#define	PROTOCOL_VERSION				(unsigned int)(SYS_COMMONVERSION + 0.00001)
#define MAX_CLIENTS						64

#define VectorCopy(a, b)	((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#define	ANGLE2SHORT(x)		((int)((x)*65536.0f/360.0f) & 65535)
#define	SHORT2ANGLE(x)		((x)*(360.0/65536))

namespace Iswenzz
{
	struct ClientSnapshotData;
	typedef struct netField_s netField_t;
	typedef struct playerState_s playerState_t;
	typedef struct objective_s objective_t;
	typedef struct entityState_s entityState_t;
	typedef struct clientState_s clientState_t;
	typedef struct hudelem_s hudelem_t;

	enum class MSGType
	{
		MSG_SNAPSHOT,
		MSG_FRAME
	};

	enum class MSGCrypt
	{
		MSG_CRYPT_NONE,
		MSG_CRYPT_HUFFMAN
	};

	/// <summary>
	/// server to client command index
	/// </summary>
	enum class svc_ops_e
	{
		svc_nop,
		svc_gamestate,
		svc_configstring,       // [short] [string] only in gamestate messages
		svc_baseline,           // only in gamestate messages
		svc_serverCommand,      // [string] to be executed by client game module
		svc_download,           // [short] size [size bytes]
		svc_snapshot,
		svc_EOF,
		svc_steamcommands,
		svc_statscommands,
		svc_configdata,
		svc_configclient
	};

	/// <summary>
	/// client to server command index
	/// </summary>
	enum class clc_ops_e
	{
		clc_move,               // [usercmd_t]
		clc_moveNoDelta,        // [usercmd_t]
		clc_clientCommand,      // [string] message
		clc_EOF,
		clc_nop,
		clc_download,
		clc_empty1,
		clc_empty2,
		clc_steamcommands,
		clc_statscommands
	};

	/// <summary>
	/// Server packet handling.
	/// </summary>
	class Msg
	{
	public:
		bool overflowed;
		bool readonly;
		std::vector<unsigned char> buffer;
		std::vector<unsigned char> splitBuffer;
		int	maxsize;
		int	cursize;
		int	splitSize;
		int	readcount;
		int	bit;

		union
		{
			int	lastRefEntity;
			int	lengthoffset;
		};

		/// <summary>
		/// Initialize a new Msg object.
		/// </summary>
		Msg();
		~Msg();

		/// <summary>
		/// Initialize a new Msg object with specified buffer buffer and crypt mode.
		/// </summary>
		/// <param name="buffer">The buffer to read.</param>
		/// <param name="len">Length of the buffer</param>
		/// <param name="mode">Crypt mode</param>
		/// <returns></returns>
		Msg(unsigned char *buffer, int len, MSGCrypt mode);

		void Initialize(int len);
		void Initialize(unsigned char* buf, int len, MSGCrypt mode);

		/// <summary>
		/// Read one bit.
		/// </summary>
		/// <returns></returns>
		int ReadBit();

		/// <summary>
		/// Read specified bits.
		/// </summary>
		/// <param name="numBits">Bit count</param>
		/// <returns></returns>
		int ReadBits(int numBits);

		/// <summary>
		/// Read a byte.
		/// </summary>
		/// <returns></returns>
		int ReadByte();

		/// <summary>
		/// Read a short.
		/// </summary>
		/// <returns></returns>
		int ReadShort();

		/// <summary>
		/// Read a 32 bit integer.
		/// </summary>
		/// <returns></returns>
		int ReadInt();

		/// <summary>
		/// Read a 64 bit integer.
		/// </summary>
		/// <returns></returns>
		int64_t ReadInt64();

		/// <summary>
		/// Read a float.
		/// </summary>
		/// <returns></returns>
		float ReadFloat();

		/// <summary>
		/// Read a string.
		/// </summary>
		/// <returns></returns>
		std::string ReadString();

		/// <summary>
		/// Read a string until line break.
		/// </summary>
		/// <returns></returns>
		std::string ReadStringLine();

		/// <summary>
		/// Read a 16 bit short and return its angle using the SHORT2ANGLE macro.
		/// </summary>
		/// <returns></returns>
		double ReadAngle16();

		int ReadEFlags(int oldFlags);
		int ReadEntityIndex(int indexBits);
		float ReadOriginFloat(int bits, float oldValue);
		float ReadOriginZFloat(float oldValue);

		/// <summary>
		/// Read a Base64 buffer.
		/// </summary>
		/// <param name="outbuf">The output buffer.</param>
		/// <param name="len">Length of the buffer.</param>
		void ReadBase64(unsigned char* outbuf, int len);

		/// <summary>
		/// Read buffer to an output buffer.
		/// </summary>
		/// <param name="buffer">The output buffer.</param>
		/// <param name="len">The size to read.</param>
		void ReadData(void *buffer, int len);

		/// <summary>
		/// Read a delta compressed ground entity.
		/// </summary>
		/// <returns></returns>
		int ReadDeltaGroundEntity();

		/// <summary>
		/// Read a delta compressed struct.
		/// </summary>
		/// <param name="time">The server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		/// <param name="number">Entity number.</param>
		/// <param name="numFields">Struct field count.</param>
		/// <param name="indexBits">Min bit count.</param>
		/// <param name="stateFields">Netfield fields.</param>
		/// <returns></returns>
		int ReadDeltaStruct(const int time, const void* from, void* to, 
			unsigned int number, int numFields, int indexBits, netField_t* stateFields);

		/// <summary>
		/// Read all delta compressed net fields.
		/// </summary>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		/// <param name="numFields">Struct field count.</param>
		/// <param name="stateFields">Netfield fields.</param>
		void ReadDeltaFields(const int time, const unsigned char* from, unsigned char* to,
			int numFields, netField_t* stateFields);

		/// <summary>
		/// Read a delta compressed net field.
		/// </summary>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		/// <param name="field">Current netfield to read.</param>
		/// <param name="noXor">Should start with a value of 0.</param>
		/// <param name="print">Should print debug information.</param>
		void ReadDeltaField(int time, const void* from, const void* to, const netField_t* field, 
			bool noXor, bool print);

		/// <summary>
		/// Read a delta compressed entity state.
		/// </summary>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		/// <param name="number">Entity number.</param>
		/// <returns></returns>
		int ReadDeltaEntity(const int time, entityState_t* from, entityState_t* to, int number);

		/// <summary>
		/// Read a delta compressed client state.
		/// </summary>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		/// <param name="number">Entity number.</param>
		/// <returns></returns>
		int ReadDeltaClient(const int time, clientState_t* from, clientState_t* to, int number);

		/// <summary>
		/// Read a delta compressed objective struct.
		/// </summary>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		void ReadDeltaObjectiveFields(const int time, objective_t* from, objective_t* to);

		/// <summary>
		/// Read all delta compressed hud element struct.
		/// </summary>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		/// <param name="count">HUD Count.</param>
		void ReadDeltaHudElems(const int time, hudelem_t* from, hudelem_t* to, int count);

		/// <summary>
		/// Read a delta compressed player state.
		/// </summary>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		/// <param name="predictedFieldsIgnoreXor">Should start with a value of 0.</param>
		void ReadDeltaPlayerState(int time, playerState_t* from, playerState_t* to,
			bool predictedFieldsIgnoreXor);

		/// <summary>
		/// Read a gamestate.
		/// </summary>
		void ReadGamestate();

		/// <summary>
		/// Read a command string.
		/// </summary>
		void ReadCommandString();

		/// <summary>
		/// Read all clients.
		/// </summary>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		/// <returns></returns>
		int ReadClients(const int time, ClientSnapshotData* from, ClientSnapshotData* to);

		/// <summary>
		/// Read all entities.
		/// </summary>
		/// <param name="time">Server time.</param>
		/// <param name="from">Pointer to the old struct state.</param>
		/// <param name="to">Pointer to the new struct state.</param>
		/// <returns></returns>
		int ReadEntities(const int time, ClientSnapshotData* from, ClientSnapshotData* to);

		/// <summary>
		/// Read last changed net field.
		/// </summary>
		/// <param name="totalFields">Net field count.</param>
		/// <returns>The last changed field index.</returns>
		int ReadLastChangedField(int totalFields);

		/// <summary>
		/// Read a server snapshot.
		/// </summary>
		/// <param name="Snapshots">Snapshot vector containing all previous Snapshots.</param>
		/// <param name="snap">The current snapshot that will be added to the vector later.</param>
		void ReadSnapshot(const std::vector<ClientSnapshotData>& Snapshots, ClientSnapshotData& snap);

		/// <summary>
		/// Get the number of bits currently read.
		/// </summary>
		/// <returns></returns>
		int GetNumBitsRead();

		/// <summary>
		/// Set lastRefEntity to -1
		/// </summary>
		void ClearLastReferencedEntity();
	};
}
