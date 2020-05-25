#pragma once
#include <string>
#include "NetFields.hpp"

#define NETCHAN_UNSENTBUFFER_SIZE 0x20000
#define NETCHAN_FRAGMENTBUFFER_SIZE 0x800
#define SYS_COMMONVERSION 17.5
#define	PROTOCOL_VERSION (unsigned int)(SYS_COMMONVERSION + 0.00001)

#define	ANGLE2SHORT(x) ((int)((x)*65536.0f/360.0f) & 65535)
#define	SHORT2ANGLE(x) ((x)*(360.0/65536))

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

	// server to client
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

	// client to server
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

	class Msg
	{
		public:
			bool overflowed;
			bool readonly;
			unsigned char* data;
			unsigned char* splitData;
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

			Msg(unsigned char *data, std::size_t len, MSGCrypt mode);
			~Msg();

			int readBit();
			int readBits(int numBits);
			int readByte();
			int readShort();
			int readInt();
			int64_t readInt64();
			float readFloat();

			std::string readString(int len);
			std::string readStringLine(int len);

			double readAngle16();
			int readEFlags(int oldFlags);
			int readEntityIndex(int indexBits);
			int readDeltaGroundEntity();
			float readOriginFloat(int bits, float oldValue);
			float readOriginZFloat(float oldValue);

			void readData(void *data, int len);
			void readDeltaField(int time, const void* from, const void* to, const netField_t* field, bool noXor);
			void readDeltaUsercmdKey(int key, struct usercmd_s *from, struct usercmd_s* to);
			void readBase64(unsigned char *outbuf, int len);

			int getNumBitsRead();
			void clearLastReferencedEntity();
	};
}
