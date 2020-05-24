#include "Msg.hpp"
#include "Huffman.hpp"
#include <stdint.h>
#include <iostream>

namespace Iswenzz
{
	int kbitmask[33] =
	{
		0,
		1,
		3,
		7,
		15,
		31,
		63,
		127,
		255,
		511,
		1023,
		2047,
		4095,
		8191,
		16383,
		32767,
		65535,
		131071,
		262143,
		524287,
		1048575,
		2097151,
		4194303,
		8388607,
		16777215,
		33554431,
		67108863,
		134217727,
		268435455,
		536870911,
		1073741823,
		2147483647,
		4294967295
	};

	Msg::Msg(unsigned char* buf, std::size_t len, MSGCrypt mode)
		: overflowed(false), readonly(false), splitData(nullptr), splitSize(0), readcount(0),
		bit(0), lastRefEntity(0), data(nullptr), cursize(0), maxsize(0)
	{
		if (mode == MSGCrypt::MSG_CRYPT_NONE)
		{
			data = new unsigned char[NETCHAN_UNSENTBUFFER_SIZE];
			std::memcpy(data, buf, len);
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
					if (splitData == nullptr)
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
		c = reinterpret_cast<short*>(&data[readcount]);

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
		c = reinterpret_cast<int32_t*>(&data[readcount]);

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
		c = reinterpret_cast<int64_t*>(&data[readcount]);

		readcount += sizeof(int64_t);
		return *c;
	}

	float Msg::readFloat()
	{
		float* c;
		if (readcount + sizeof(float) > cursize) 
		{
			overflowed = 1;
			return -1;
		}
		c = reinterpret_cast<float*>(&data[readcount]);

		readcount += sizeof(float);
		return *c;
	}

	double Msg::readAngle16()
	{
		return SHORT2ANGLE((double)readShort());
	}

	int Msg::readEFlags(int oldFlags)
	{
		int bitChanged, value, i, b;
		if (readBit() == 1)
		{
			value = 0;
			for (i = 0; i < 24; i += 8)
			{
				b = readByte();
				value |= (b << i);
			}
		}
		else
		{
			bitChanged = readBits(5);
			value = oldFlags ^ (1 << bitChanged);
		}
		return value;
	}

	int Msg::readEntityIndex(int indexBits)
	{
		if (readBit())
			++lastRefEntity;
		else if (indexBits != 10 || readBit())
			lastRefEntity = readBits(indexBits);
		else
			lastRefEntity += readBits(4);
		return lastRefEntity;
	}

	int Msg::readDeltaGroundEntity()
	{
		int j, value;
		if (readBit() == 1) return 1022;
		if (readBit() == 1) return 0;

		value = readBits(2);
		for (j = 2; j < 10; j += 8)
			value |= readByte() << j;
		return value;
	}

	float Msg::readOriginFloat(int bits, float oldValue)
	{
		signed int coord;
		if (readBit())
		{
			float center[3];
			//MSG_GetMapCenter(center); // @TODO
			coord = (signed int)(center[bits != -92] + 0.5);
			return (double)((((signed int)oldValue - coord + 0x8000) ^ readBits(16)) + coord - 0x8000);
		}
		return (double)(readBits(7) - 64) + oldValue;
	}

	float Msg::readOriginZFloat(float oldValue)
	{
		signed int coord;
		if (readBit())
		{
			float center[3];
			//MSG_GetMapCenter(center); // @TODO
			coord = (signed int)(center[2] + 0.5);
			return (double)((((signed int)oldValue - coord + 0x8000) ^ readBits(16)) + coord - 0x8000);
		}
		return (double)(readBits(7) - 64) + oldValue;
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
			bigstring += static_cast<unsigned char>(c);
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
			bigstring += static_cast<char>(c);
			l++;
		} 
		while (l < len - 1);
		return bigstring;
	}

	void Msg::readData(void* data, int len)
	{
		for (int i = 0; i < len; i++)
			reinterpret_cast<unsigned char*>(data)[i] = readByte();
	}

	// @TODO
	void Msg::readDeltaField(int time, const void* from, const void* to, const netField_t* field, bool noXor)
	{
		unsigned char* fromF;
		unsigned char* toF;
		int bits, b, bit_vect, v, zeroV = 0;
		uint32_t l;
		double f = 0;
		signed int t;

		if (noXor)
			fromF = (unsigned char*)&zeroV;
		else
			fromF = (unsigned char*)from + field->offset;
		toF = (unsigned char*)to + field->offset;

		if (field->changeHints != 2)
		{
			if (!readBit()) // No change
			{
				*(uint32_t*)toF = *(uint32_t*)fromF;
				return;
			}
		} 

		//Changed field
		bits = field->bits;
		if (!bits)
		{
			if (!readBit())
			{
				*(uint32_t*)toF = readBit() << 31;
				return;
			}
			if (!readBit())
			{
				b = readBits(5);
				v = ((32 * readByte() + b) ^ ((signed int)*(float*)fromF + 4096)) - 4096;
				*(float*)toF = (double)v;
				return;
			}
			l = readInt();
			*(uint32_t*)toF = l;
			*(uint32_t*)toF = l ^ *(uint32_t*)fromF;
		}

		std::cout << "Field Bits: " << field->bits << std::endl;

		// Command
		switch (field->bits)
		{
			case -89:
				if (!readBit())
				{
					b = readBits(5);
					l = ((32 * readByte() + b) ^ ((signed int)*(float*)fromF + 4096)) - 4096;
					*(float*)toF = (double)l;
					return;
				}
				l = readInt();
				*(uint32_t*)toF = l ^ *(uint32_t*)fromF;
				return;

			case -88:
				l = readInt();
				*(uint32_t*)toF = l ^ *(uint32_t*)fromF;
				return;

			case -100:
				if (!readBit())
				{
					*(float*)toF = 0.0;
					return;
				}
				*(float*)toF = readAngle16();
				return;

			case -99:
				if (readBit())
				{
					if (!readBit())
					{
						b = readBits(4);
						v = ((16 * readByte() + b) ^ ((signed int)*(float*)fromF + 2048)) - 2048;
						*(float*)toF = (double)v;
						return;
					}
					l = readInt();
					*(uint32_t*)toF = l ^ *(uint32_t*)fromF;
					return;
				}
				*(uint32_t*)toF = 0;
				return;

			case -98:
				*(uint32_t*)toF = readEFlags(*(uint32_t*)fromF);
				return;

			case -97:
				if (readBit())
					*(uint32_t*)toF = readInt();
				else
					*(uint32_t*)toF = time - readBits(8);
				return;

			case -96:
				*(uint32_t*)toF = readDeltaGroundEntity();
				return;

			case -95:
				*(uint32_t*)toF = 100 * readBits(7);
				return;

			case -94:
			case -93:
				*(uint32_t*)toF = readByte();
				return;

			case -92:
			case -91:
				f = readOriginFloat(bits, *(float*)fromF);
				*(float*)toF = f;
				return;

			case -90:
				f = readOriginZFloat(*(float*)fromF);
				*(float*)toF = f;
				return;

			case -87:
				*(float*)toF = readAngle16();
				return;

			case -86:
				*(float*)toF = (double)readBits(5) / 10.0 + 1.399999976158142;
				return;

			case -85:
				if (readBit())
				{
					*(uint32_t*)toF = *(uint32_t*)fromF;
					toF[3] = (fromF[3] != 0) - 1;
					return;
				}
				if (!readBit())
				{
					toF[0] = readByte();
					toF[1] = readByte();
					toF[2] = readByte();
				}
				toF[3] = 8 * readBits(5);
				return;

			default:
				if (!readBit())
				{
					*(uint32_t*)toF = 0;
					return;
				}
				bits = abs(field->bits);
				bit_vect = bits & 7;

				if (bit_vect) t = readBits(bit_vect);
				else t = 0;

				for (; bit_vect < bits; bit_vect += 8)
					t |= (readByte() << bit_vect);

				if (bits == 32) bit_vect = -1;
				else bit_vect = (1 << bits) - 1;

				t = t ^ (*(uint32_t*)fromF & bit_vect);
				if (field->bits < 0 && (t >> (bits - 1)) & 1)
					t |= ~bit_vect;
				*(uint32_t*)toF = t;
		}
	}

	// @TODO
	void Msg::readDeltaUsercmdKey(int key, usercmd_s* from, usercmd_s* to)
	{
		
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
			outbuf[i + 0] = (reinterpret_cast<char*>(&b64data))[2];
			outbuf[i + 1] = (reinterpret_cast<char*>(&b64data))[1];
			outbuf[i + 2] = (reinterpret_cast<char*>(&b64data))[0];
			i += 3;
		} 
		while (databyte != -1 && (i + 4) < len);
		outbuf[i] = '\0';
	}

	int Msg::getNumBitsRead()
	{
		return 8 * readcount - ((8 - bit) & 7);
	}
}
