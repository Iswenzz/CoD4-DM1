#pragma once
#include "Demo/DemoData.hpp"
#include <vector>

namespace Iswenzz
{
	enum class MSGType
	{
		MSG_SNAPSHOT,
		MSG_FRAME,
		MSG_PROTOCOL,
		MSG_RELIABLE
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
		MSGType Type = { };
		int SrvMsgSeq = 0;
		int Dummy = 0;
		int Protocol = 16;
		float MapCenter[3] = { 0, 0, 0 };

		bool Overflowed = false;
		bool Readonly = false;

		std::vector<unsigned char> Buffer{ };
		std::vector<unsigned char> SplitBuffer{ };

		int	MaxSize = 0;
		int	CurSize = 0;
		int	SplitSize = 0;
		int	ReadCount = 0;
		int	Bit = 0;
		int LastRefEntity = 0;

		/// <summary>
		/// Initialize a new Msg object.
		/// </summary>
		Msg() = default;
		~Msg() = default;

		/// <summary>
		/// Initialize a new Msg object with the specified game protocol.
		/// </summary>
		/// <param name="protocol">The game protocol.</param>
		Msg(int protocol);

		/// <summary>
		/// Initialize a new Msg object with specified buffer and crypt mode.
		/// </summary>
		/// <param name="buffer">The buffer to read.</param>
		/// <param name="len">Length of the buffer.</param>
		/// <param name="mode">The crypt mode.</param>
		/// <param name="protocol">The game protocol.</param>
		/// <returns></returns>
		Msg(unsigned char *buffer, int len, MSGCrypt mode, int protocol);

		/// <summary>
		/// Initialize a new Msg object from an existing msg.
		/// </summary>
		/// <param name="msg">The message to copy.</param>
		/// <param name="mode">The crypto mode.</param>
		Msg(Msg& msg, MSGCrypt mode);

		/// <summary>
		/// Initialize the msg buffer with the specified buffer length.
		/// </summary>
		/// <param name="len">The buffer length.</param>
		void Initialize(int len, bool read);

		/// <summary>
		/// Initialze the msg buffer with specified buffer and crypt mode.
		/// </summary>
		/// <param name="buffer">The buffer to read.</param>
		/// <param name="len">Length of the buffer.</param>
		/// <param name="mode">Crypt mode.</param>
		/// <param name="protocol">The game protocol.</param>
		/// <returns></returns>
		void Initialize(unsigned char* buf, int len, MSGCrypt mode, int protocol);

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
		/// Write a 16 bit short using the ANGLE2SHORT macro.
		/// </summary>
		/// <param name="f">Angle value.</param>
		void WriteAngle16(float f);

		/// <summary>
		/// Write a 24 bit flag.
		/// </summary>
		/// <param name="oldFlags">Old flags.</param>
		/// <param name="newFlags">New flags.</param>
		void Write24BitFlag(const int oldFlags, const int newFlags);

		/// <summary>
		/// Write and int.
		/// </summary>
		/// <param name="value">The int value.</param>
		void WriteInt(int value);

		/// <summary>
		/// Write a byte.
		/// </summary>
		/// <param name="value">The byte value.</param>
		void WriteByte(int value);

		/// <summary>
		/// Write a short.
		/// </summary>
		/// <param name="value">The short value.</param>
		void WriteShort(int value);

		/// <summary>
		/// Write a 0 bit.
		/// </summary>
		void WriteBit0();

		/// <summary>
		/// Write a 1 bit.
		/// </summary>
		void WriteBit1();

		/// <summary>
		/// Write bits.
		/// </summary>
		/// <param name="bits">The bits to write.</param>
		/// <param name="bitcount">The bit count.</param>
		void WriteBits(int bits, int bitcount);

		/// <summary>
		/// Write a string.
		/// </summary>
		/// <param name="string">The string value.</param>
		void WriteString(const char* string);

		/// <summary>
		/// Write origin.
		/// </summary>
		/// <param name="bits">The field bits.</param>
		/// <param name="value">The origin value.</param>
		/// <param name="oldValue">The old origin value.</param>
		void WriteOriginFloat(int bits, float value, float oldValue);

		/// <summary>
		/// Write a Z origin.
		/// </summary>
		/// <param name="value">The origin value.</param>
		/// <param name="oldValue">The old origin value.</param>
		void WriteOriginZFloat(float value, float oldValue);

		/// <summary>
		/// Get the used bit count.
		/// </summary>
		/// <returns></returns>
		int GetUsedBitCount();

		/// <summary>
		/// Get the number of bits currently read.
		/// </summary>
		/// <returns></returns>
		int GetNumBitsRead();

		/// <summary>
		/// Clear the last referenced entity.
		/// </summary>
		void ClearLastReferencedEntity();

		/// <summary>
		/// Discard the message.
		/// </summary>
		void Discard();
	};
}
