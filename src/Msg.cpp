#include "Msg.hpp"
#include "Huffman.hpp"
#include "NetFields.hpp"

#include <cstring>
#include <stdint.h>
#include <iostream>

namespace Iswenzz
{
	int kbitmask[33] =
	{
		0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191,
		16383, 32767, 65535, 131071, 262143, 524287, 1048575, 2097151,
		4194303, 8388607, 16777215, 33554431, 67108863, 134217727,
		268435455, 536870911, 1073741823, 2147483647, 4294967295
	};

	Msg::Msg(int protocol) : protocol(protocol) { }

	Msg::Msg(unsigned char* buf, int len, MSGCrypt mode, int protocol)
	{
		Initialize(buf, len, mode, protocol);
	}

	Msg::Msg(Msg& msg, MSGCrypt mode)
	{
		Initialize(msg.buffer.data(), msg.cursize, mode, msg.protocol);
		srvMsgSeq = msg.srvMsgSeq;
		dummy = msg.dummy;
	}

	void Msg::Initialize(int len)
	{
		cursize = len;
		maxsize = NETCHAN_UNSENTBUFFER_SIZE;
		buffer.resize(len);
	}

	void Msg::Initialize(unsigned char* buf, int len, MSGCrypt mode, int protocol)
	{
		if (mode == MSGCrypt::MSG_CRYPT_NONE)
		{
			buffer.resize(len);
			cursize = buffer.size();
			maxsize = NETCHAN_MAXBUFFER_SIZE;
			std::memcpy(buffer.data(), buf, buffer.size());
		}
		else if (mode == MSGCrypt::MSG_CRYPT_HUFFMAN)
		{
			buffer.resize(NETCHAN_MAXBUFFER_SIZE);
			cursize = Huffman::Decompress(buf, len, buffer.data(), buffer.size());
			maxsize = NETCHAN_MAXBUFFER_SIZE;
		}
		readcount = 0;
		this->protocol = protocol;
	}

	int Msg::ReadBit()
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
			bits = buffer[numBytes] >> oldbit7;
			bit++;
			return bits & 1;
		}
		bits = splitBuffer[numBytes - cursize] >> oldbit7;
		bit++;
		return bits & 1;
	}

	int Msg::ReadBits(int numBits)
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
					if (splitBuffer.empty())
						return 0;

					var = splitBuffer[(bit / 8) - cursize];
				}
				else
					var = buffer[bit / 8];

				retval |= ((var >> (bit & 7)) & 1) << i;
				bit++;
			}
		}
		return retval;
	}

	int Msg::ReadByte()
	{
		unsigned char* c;
		if (readcount + sizeof(unsigned char) > cursize)
		{
			overflowed = 1;
			return -1;
		}
		c = &buffer[readcount];

		readcount += sizeof(unsigned char);
		return *c;
	}

	int Msg::ReadShort()
	{
		signed short* c;
		if (readcount + sizeof(short) > cursize) 
		{
			overflowed = 1;
			return -1;
		}
		c = reinterpret_cast<short*>(&buffer[readcount]);

		readcount += sizeof(short);
		return *c;
	}

	int Msg::ReadInt()
	{
		int32_t* c;

		if (readcount + sizeof(int32_t) > cursize) 
		{
			overflowed = 1;
			return -1;
		}
		c = reinterpret_cast<int32_t*>(&buffer[readcount]);

		readcount += sizeof(int32_t);
		return *c;
	}

	int64_t Msg::ReadInt64()
	{
		int64_t* c;
		if (readcount + sizeof(int64_t) > cursize) 
		{
			overflowed = 1;
			return -1;
		}
		c = reinterpret_cast<int64_t*>(&buffer[readcount]);

		readcount += sizeof(int64_t);
		return *c;
	}

	float Msg::ReadFloat()
	{
		float* c;
		if (readcount + sizeof(float) > cursize) 
		{
			overflowed = 1;
			return -1;
		}
		c = reinterpret_cast<float*>(&buffer[readcount]);

		readcount += sizeof(float);
		return *c;
	}

	double Msg::ReadAngle16()
	{
		return SHORT2ANGLE((double)ReadShort());
	}

	int Msg::ReadEFlags(int oldFlags)
	{
		int bitChanged, value, i, b;
		if (ReadBit() == 1)
		{
			value = 0;
			for (i = 0; i < 24; i += 8)
			{
				b = ReadByte();
				value |= (b << i);
			}
		}
		else
		{
			bitChanged = ReadBits(5);
			value = oldFlags ^ (1 << bitChanged);
		}
		return value;
	}

	float Msg::ReadOriginFloat(int bits, float oldValue)
	{
		signed int coord;

		if (protocol != 17)
			return ReadFloat();

		if (ReadBit())
		{
			float center[3]{ 0, 0, 0 };
			coord = (signed int)(center[bits != -92] + 0.5);
			return (double)((((signed int)oldValue - coord + 0x8000) ^ ReadBits(16)) + coord - 0x8000);
		}
		return (double)(ReadBits(7) - 64) + oldValue;
	}

	float Msg::ReadOriginZFloat(float oldValue)
	{
		signed int coord;

		if (protocol != 17)
			return ReadFloat();

		if (ReadBit())
		{
			float center[3]{ 0, 0, 0 };
			coord = (signed int)(center[2] + 0.5);
			return (double)((((signed int)oldValue - coord + 0x8000) ^ ReadBits(16)) + coord - 0x8000);
		}
		return (double)(ReadBits(7) - 64) + oldValue;
	}

	std::string Msg::ReadString()
	{
		int l = 0, c;
		std::string str;
		while (true)
		{
			// use ReadByte so -1 is out of bounds
			c = ReadByte();
			if (c == -1 || c == '\0')
				break;

			// translate all fmt spec to avoid crash bugs
			//if (c == '%') c = '.';

			str += static_cast<unsigned char>(c);
			l++;
		} 
		return str;
	}

	std::string Msg::ReadStringLine()
	{
		int	l = 0, c;
		std::string str;
		while (true)
		{
			// use ReadByte so -1 is out of bounds
			c = ReadByte();
			if (c == -1 || c == '\0' || c == '\n')
				break;

			// translate all fmt spec to avoid crash bugs
			//if (c == '%') c = '.';

			str += static_cast<char>(c);
			l++;
		} 
		return str;
	}

	void Msg::ReadData(void* buffer, int len)
	{
		for (int i = 0; i < len; i++)
			reinterpret_cast<unsigned char*>(buffer)[i] = ReadByte();
	}

	void Msg::ReadBase64(unsigned char* outbuf, int len)
	{
		int b64data;
		int k, shift;
		int i = 0, databyte = 0;

		do
		{
			b64data = 0;
			for (k = 0, shift = 18; k < 4; ++k, shift -= 6)
			{
				databyte = ReadByte();
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
			outbuf[i + 0] = (reinterpret_cast<char*>(&b64data))[2];
			outbuf[i + 1] = (reinterpret_cast<char*>(&b64data))[1];
			outbuf[i + 2] = (reinterpret_cast<char*>(&b64data))[0];
			i += 3;
		} 
		while (databyte != -1 && (i + 4) < len);
		outbuf[i] = '\0';
	}

	int Msg::GetNumBitsRead()
	{
		return 8 * readcount - ((8 - bit) & 7);
	}

	void Msg::ClearLastReferencedEntity()
	{
		lastRefEntity = -1;
	}
}
