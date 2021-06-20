#pragma once
#include "DemoData.hpp"
#include <string>

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
		MSGType type = { };
		int srvMsgSeq = 0;
		int dummy = 0;

		bool overflowed = 0;
		bool readonly = 0;

		std::vector<unsigned char> buffer{ };
		std::vector<unsigned char> splitBuffer{ };

		int	maxsize = 0;
		int	cursize = 0;
		int	splitSize = 0;
		int	readcount = 0;
		int	bit = 0;
		int lastRefEntity = 0;

		/// <summary>
		/// Initialize a new Msg object.
		/// </summary>
		Msg() = default;
		~Msg() = default;

		/// <summary>
		/// Initialize a new Msg object with specified buffer and crypt mode.
		/// </summary>
		/// <param name="buffer">The buffer to read.</param>
		/// <param name="len">Length of the buffer.</param>
		/// <param name="mode">Crypt mode.</param>
		/// <returns></returns>
		Msg(unsigned char *buffer, int len, MSGCrypt mode);

		/// <summary>
		/// Initialize the msg buffer with the specified buffer length.
		/// </summary>
		/// <param name="len">The buffer length.</param>
		void Initialize(int len);

		/// <summary>
		/// Initialze the msg buffer with specified buffer and crypt mode.
		/// </summary>
		/// <param name="buffer">The buffer to read.</param>
		/// <param name="len">Length of the buffer.</param>
		/// <param name="mode">Crypt mode.</param>
		/// <returns></returns>
		void Initialize(unsigned char* buf, int len, MSGCrypt mode);

		/// <summary>
		/// Read one bit.
		/// </summary>
		/// <returns></returns>
		int ReadBit();

		/// <summary>
		/// Read specified bits.
		/// </summary>
		/// <param name="numBits">Bit count.</param>
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

		/// <summary>
		/// Read an enum flag.
		/// </summary>
		/// <param name="oldFlags">The old flag value.</param>
		/// <returns></returns>
		int ReadEFlags(int oldFlags);

		/// <summary>
		/// Read an origin vector.
		/// </summary>
		/// <param name="oldValue">The old vector value.</param>
		/// <returns></returns>
		float ReadOriginFloat(int bits, float oldValue);

		/// <summary>
		/// Read an origin vector Z.
		/// </summary>
		/// <param name="oldValue">The old vector value.</param>
		float ReadOriginZFloat(float oldValue);

		/// <summary>
		/// Read a Base64 buffer.
		/// </summary>
		/// <param name="outbuf">The output buffer.</param>
		/// <param name="len">Length of the buffer.</param>
		void ReadBase64(unsigned char* outbuf, int len);

		/// <summary>
		/// Read the specified length to an output buffer.
		/// </summary>
		/// <param name="buffer">The output buffer.</param>
		/// <param name="len">The size to read.</param>
		void ReadData(void *buffer, int len);

		/// <summary>
		/// Get the number of bits currently read.
		/// </summary>
		/// <returns></returns>
		int GetNumBitsRead();

		/// <summary>
		/// Clear the last referenced entity.
		/// </summary>
		void ClearLastReferencedEntity();
	};
}
