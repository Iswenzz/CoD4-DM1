#include "Msg.hpp"
#include "Huffman.hpp"
#include <stdint.h>

namespace Iswenzz
{
	Msg::Msg(unsigned char* buf, int len, MSGCrypt mode)
		: overflowed(false), readonly(false), splitData(nullptr), splitSize(0), readcount(0),
		bit(0), lastRefEntity(0), data(nullptr), cursize(0), maxsize(0)
	{
		if (mode == MSGCrypt::MSG_CRYPT_NONE)
		{
			data = buf;
			cursize = len;
			maxsize = NETCHAN_UNSENTBUFFER_SIZE;
		}
		else if (mode == MSGCrypt::MSG_CRYPT_HUFFMAN)
		{
			data = new unsigned char[NETCHAN_UNSENTBUFFER_SIZE];
			cursize = MSG_ReadBitsCompress(buf, len, data, NETCHAN_FRAGMENTBUFFER_SIZE);
			maxsize = NETCHAN_UNSENTBUFFER_SIZE;
		}
	}

	Msg::~Msg() 
	{ 
		if (data)
			delete data;
	}

	int Msg::readBit()
	{
		int oldbit7, numBytes, bits;

		oldbit7 = bit & 7;
		if (!oldbit7)
		{
			if (readcount >= cursize + splitSize)
			{
				overflowed = 1;
				return -1;
			}
			bit = 8 * readcount;
			readcount++;
		}

		numBytes = bit / 8;
		if (numBytes < cursize)
		{
			bits = data[numBytes] >> oldbit7;
			bit++;
			return bits & 1;
		}
		bits = splitData[numBytes - cursize] >> oldbit7;
		bit++;
		return bits & 1;
	}

	int Msg::readBits(int numBits)
	{
		int i;
		signed int var;
		int retval = 0;

		if (numBits > 0)
		{
			for (i = 0; numBits != i; i++)
			{
				if (!(bit & 7))
				{
					if (readcount >= splitSize + cursize)
					{
						overflowed = 1;
						return -1;
					}
					bit = 8 * readcount;
					readcount++;
				}
				if (((bit / 8)) >= cursize)
				{
					if (splitData == NULL)
						return 0;

					var = splitData[(bit / 8) - cursize];
				}
				else
					var = data[bit / 8];

				retval |= ((var >> (bit & 7)) & 1) << i;
				bit++;
			}
		}
		return retval;
	}

	int Msg::readByte()
	{
		unsigned char* c;
		if (readcount + sizeof(unsigned char) > cursize)
		{
			overflowed = 1;
			return -1;
		}
		c = &data[readcount];

		readcount += sizeof(unsigned char);
		return *c;
	}

	int Msg::readShort()
	{
		signed short* c;
		if (readcount + sizeof(short) > cursize) 
		{
			overflowed = 1;
			return -1;
		}
		c = (short*)&data[readcount];

		readcount += sizeof(short);
		return *c;
	}

	int Msg::readInt()
	{
		int32_t* c;

		if (readcount + sizeof(int32_t) > cursize) 
		{
			overflowed = 1;
			return -1;
		}
		c = (int32_t*)&data[readcount];

		readcount += sizeof(int32_t);
		return *c;
	}

	int64_t Msg::readInt64()
	{
		int64_t* c;
		if (readcount + sizeof(int64_t) > cursize) 
		{
			overflowed = 1;
			return -1;
		}
		c = (int64_t*)&data[readcount];

		readcount += sizeof(int64_t);
		return *c;
	}

	float Msg::readFloat()
	{
		float* c;

		if (readcount + sizeof(int32_t) > cursize) {
			//readcount += sizeof(int32_t); /* Hmm what a bad bug is this ? O_o*/
			return -1;
		}
		c = (float*)&data[readcount];

		readcount += sizeof(float);
		return *c;
	}

	std::string Msg::readString(int len)
	{
		int l = 0, c;
		std::string bigstring;

		do 
		{
			c = readByte();      // use ReadByte so -1 is out of bounds
			if (c == -1 || c == 0)
				break;
			// translate all fmt spec to avoid crash bugs
			if (c == '%')
				c = '.';
			bigstring[l] = c;
			l++;
		} 
		while (l < len - 1);

		return bigstring;
	}

	std::string Msg::readStringLine(int len)
	{
		int	l = 0, c;
		std::string bigstring;

		do 
		{
			c = readByte();	// use ReadByte so -1 is out of bounds
			if (c == -1 || c == 0 || c == '\n')
				break;
			// translate all fmt spec to avoid crash bugs
			if (c == '%')
				c = '.';
			bigstring[l] = c;
			l++;
		} 
		while (l < len - 1);

		return bigstring;
	}

	void Msg::readData(void* data, int len)
	{
		for (int i = 0; i < len; i++)
			((unsigned char*)data)[i] = readByte();
	}

	void Msg::readDeltaUsercmdKey(int key, usercmd_s* from, usercmd_s* to)
	{
		// @TODO
	}

	void Msg::readBase64(unsigned char* outbuf, int len)
	{
		int databyte;
		int b64data;
		int k, shift;
		int i = 0;

		do
		{
			b64data = 0;
			for (k = 0, shift = 18; k < 4; ++k, shift -= 6)
			{
				databyte = readByte();
				if (databyte >= 'A' && databyte <= 'Z')
					databyte -= 'A';
				else if (databyte >= 'a' && databyte <= 'z') 
				{
					databyte -= 'a';
					databyte += 26;
				}
				else if (databyte >= '0' && databyte <= '9') 
				{
					databyte -= '0';
					databyte += 52;
				}
				else if (databyte == '+')
					databyte = 62;
				else if (databyte == '/')
					databyte = 63;
				else 
				{
					databyte = -1;
					break;
				}
				b64data |= (databyte << shift);
			}
			outbuf[i + 0] = ((char*)&b64data)[2];
			outbuf[i + 1] = ((char*)&b64data)[1];
			outbuf[i + 2] = ((char*)&b64data)[0];
			i += 3;
		} 
		while (databyte != -1 && (i + 4) < len);
		outbuf[i] = '\0';
	}
}
