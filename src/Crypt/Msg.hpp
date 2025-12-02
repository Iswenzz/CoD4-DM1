#pragma once
#include "Demo/DemoData.hpp"

#include <cmath>
#include <cstring>
#include <memory>
#include <vector>

namespace CoD4::DM1
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

	enum class svc_ops_e
	{
		svc_nop,
		svc_gamestate,
		svc_configstring,  // [short] [string] only in gamestate messages
		svc_baseline,	   // only in gamestate messages
		svc_serverCommand, // [string] to be executed by client game module
		svc_download,	   // [short] size [size bytes]
		svc_snapshot,
		svc_EOF,
		svc_steamcommands,
		svc_statscommands,
		svc_configdata,
		svc_configclient
	};

	enum class clc_ops_e
	{
		clc_move,		   // [usercmd_t]
		clc_moveNoDelta,   // [usercmd_t]
		clc_clientCommand, // [string] message
		clc_EOF,
		clc_nop,
		clc_download,
		clc_empty1,
		clc_empty2,
		clc_steamcommands,
		clc_statscommands
	};

	class Msg
	{
	public:
		MSGType Type = {};
		int SrvMsgSeq = 0;
		int Dummy = 0;
		int Protocol = 16;
		float MapCenter[3] = { 0, 0, 0 };

		bool Overflowed = false;
		bool Readonly = false;

		std::vector<uint8_t> Buffer{};
		std::vector<uint8_t> SplitBuffer{};

		int MaxSize = 0;
		int CurSize = 0;
		int SplitSize = 0;
		int ReadCount = 0;
		int Bit = 0;
		int LastRefEntity = 0;

		Msg() = default;
		Msg(int protocol);
		Msg(uint8_t* buffer, int len, MSGCrypt mode, int protocol);
		Msg(Msg& msg, MSGCrypt mode);
		~Msg() = default;

		void Initialize(int len, bool read);
		void Initialize(uint8_t* buf, int len, MSGCrypt mode, int protocol);

		int ReadBit();
		int ReadBits(int numBits);
		int ReadByte();
		int ReadShort();
		int ReadInt();
		int64_t ReadInt64();
		float ReadFloat();
		std::string ReadString();
		std::string ReadStringLine();
		double ReadAngle16();
		int ReadEFlags(int oldFlags);
		float ReadOriginFloat(int bits, float oldValue);
		float ReadOriginZFloat(float oldValue);
		void ReadBase64(uint8_t* outbuf, int len);
		void ReadData(void* buffer, int len);

		void WriteAngle16(float f);
		void Write24BitFlag(const int oldFlags, const int newFlags);
		void WriteInt(int value);
		void WriteByte(int value);
		void WriteShort(int value);
		void WriteBit0();
		void WriteBit1();
		void WriteBits(int bits, int bitcount);
		void WriteString(const char* string);
		void WriteOriginFloat(int bits, float value, float oldValue);
		void WriteOriginZFloat(float value, float oldValue);

		int GetUsedBitCount();
		int GetNumBitsRead();
		void ClearLastReferencedEntity();
		void Discard();
	};
}
