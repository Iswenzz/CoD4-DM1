#pragma once
#include "EMSGType.hpp"
#include <string>

#define NETCHAN_UNSENTBUFFER_SIZE 0x20000
#define NETCHAN_FRAGMENTBUFFER_SIZE 0x800
#define SYS_COMMONVERSION 17.5
#define	PROTOCOL_VERSION (unsigned int)(SYS_COMMONVERSION + 0.00001)

namespace Iswenzz
{
	class Msg
	{
		public:
			Msg(unsigned char *data, int len, MSGCrypt mode);
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

			void readData(void *data, int len);
			void readDeltaUsercmdKey(int key, struct usercmd_s *from, struct usercmd_s* to);
			void readBase64(unsigned char *outbuf, int len);

		private:
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
	};
}
