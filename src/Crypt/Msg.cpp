#include "Msg.hpp"
#include "Huffman.hpp"
#include "NetFields.hpp"

#include <cstring>
#include <cmath>
#include <stdint.h>
#include <iostream>

namespace Iswenzz::CoD4::DM1
{
	long long kbitmask[33] =
	{
		0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191,
		16383, 32767, 65535, 131071, 262143, 524287, 1048575, 2097151,
		4194303, 8388607, 16777215, 33554431, 67108863, 134217727,
		268435455, 536870911, 1073741823, 2147483647, 4294967295
	};

	Msg::Msg(int protocol) : Protocol(protocol) { }

	Msg::Msg(uint8_t* buf, int len, MSGCrypt mode, int protocol)
	{
		Initialize(buf, len, mode, protocol);
	}

	Msg::Msg(Msg& msg, MSGCrypt mode)
	{
		Initialize(msg.Buffer.data(), msg.CurSize, mode, msg.Protocol);
		SrvMsgSeq = msg.SrvMsgSeq;
		Dummy = msg.Dummy;
	}

	void Msg::Initialize(int len, bool read)
	{
		CurSize = read ? len : 0;
		MaxSize = NETCHAN_UNSENTBUFFER_SIZE;
		Buffer.resize(len);
	}

	void Msg::Initialize(uint8_t* buf, int len, MSGCrypt mode, int protocol)
	{
		if (mode == MSGCrypt::MSG_CRYPT_NONE)
		{
			Buffer.resize(len);
			CurSize = Buffer.size();
			MaxSize = NETCHAN_MAXBUFFER_SIZE;
			memcpy(Buffer.data(), buf, Buffer.size());
		}
		else if (mode == MSGCrypt::MSG_CRYPT_HUFFMAN)
		{
			Buffer.resize(NETCHAN_MAXBUFFER_SIZE);
			CurSize = Huffman::Decompress(buf, len, Buffer.data(), Buffer.size());
			MaxSize = NETCHAN_MAXBUFFER_SIZE;
		}
		ReadCount = 0;
		Protocol = protocol;
	}

	int Msg::ReadBit()
	{
		int oldbit7, numBytes, bits;

		oldbit7 = Bit & 7;
		if (!oldbit7)
		{
			if (ReadCount >= CurSize + SplitSize)
			{
				Overflowed = 1;
				return -1;
			}
			Bit = 8 * ReadCount;
			ReadCount++;
		}

		numBytes = Bit / 8;
		if (numBytes < CurSize)
		{
			bits = Buffer[numBytes] >> oldbit7;
			Bit++;
			return bits & 1;
		}
		bits = SplitBuffer[numBytes - CurSize] >> oldbit7;
		Bit++;
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
				if (!(Bit & 7))
				{
					if (ReadCount >= SplitSize + CurSize)
					{
						Overflowed = 1;
						return -1;
					}
					Bit = 8 * ReadCount;
					ReadCount++;
				}
				if (((Bit / 8)) >= CurSize)
				{
					if (SplitBuffer.empty())
						return 0;

					var = SplitBuffer[(Bit / 8) - CurSize];
				}
				else
					var = Buffer[Bit / 8];

				retval |= ((var >> (Bit & 7)) & 1) << i;
				Bit++;
			}
		}
		return retval;
	}

	int Msg::ReadByte()
	{
		uint8_t* c;
		if (ReadCount + sizeof(uint8_t) > CurSize)
		{
			Overflowed = 1;
			return -1;
		}
		c = &Buffer[ReadCount];

		ReadCount += sizeof(uint8_t);
		return *c;
	}

	int Msg::ReadShort()
	{
		signed short* c;
		if (ReadCount + sizeof(short) > CurSize) 
		{
			Overflowed = 1;
			return -1;
		}
		c = reinterpret_cast<short*>(&Buffer[ReadCount]);

		ReadCount += sizeof(short);
		return *c;
	}

	int Msg::ReadInt()
	{
		int32_t* c;

		if (ReadCount + sizeof(int32_t) > CurSize) 
		{
			Overflowed = 1;
			return -1;
		}
		c = reinterpret_cast<int32_t*>(&Buffer[ReadCount]);

		ReadCount += sizeof(int32_t);
		return *c;
	}

	int64_t Msg::ReadInt64()
	{
		int64_t* c;
		if (ReadCount + sizeof(int64_t) > CurSize) 
		{
			Overflowed = 1;
			return -1;
		}
		c = reinterpret_cast<int64_t*>(&Buffer[ReadCount]);

		ReadCount += sizeof(int64_t);
		return *c;
	}

	float Msg::ReadFloat()
	{
		float* c;
		if (ReadCount + sizeof(float) > CurSize) 
		{
			Overflowed = 1;
			return -1;
		}
		c = reinterpret_cast<float*>(&Buffer[ReadCount]);

		ReadCount += sizeof(float);
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

		if (Protocol > COD4X_FALLBACK_PROTOCOL)
		{
			int intf = ReadInt();
			return *(float*)&intf;
		}

		if (ReadBit())
		{
			float center[3]{ 0, 0, 0 };
			VectorCopy(center, MapCenter);
			
			coord = static_cast<signed int>(center[bits != -92] + 0.5);
			return static_cast<double>(((static_cast<signed int>(oldValue) - coord + 0x8000) 
				^ ReadBits(16)) + coord - 0x8000);
		}
		return static_cast<double>(ReadBits(7) - 64) + oldValue;
	}

	float Msg::ReadOriginZFloat(float oldValue)
	{
		signed int coord;

		if (Protocol > COD4X_FALLBACK_PROTOCOL)
		{
			int intf = ReadInt();
			return *reinterpret_cast<float*>(&intf);
		}

		if (ReadBit())
		{
			float center[3]{ 0, 0, 0 };
			VectorCopy(center, MapCenter);
			
			coord = static_cast<signed int>(center[2] + 0.5);
			return static_cast<double>(((static_cast<signed int>(oldValue) - coord + 0x8000)
				^ ReadBits(16)) + coord - 0x8000);
		}
		return static_cast<double>(ReadBits(7) - 64) + oldValue;
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

			str += static_cast<uint8_t>(c);
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
			reinterpret_cast<uint8_t*>(buffer)[i] = ReadByte();
	}

	void Msg::ReadBase64(uint8_t* outbuf, int len)
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
			outbuf[i + 0] = reinterpret_cast<char*>(&b64data)[2];
			outbuf[i + 1] = reinterpret_cast<char*>(&b64data)[1];
			outbuf[i + 2] = reinterpret_cast<char*>(&b64data)[0];
			i += 3;
		} 
		while (databyte != -1 && (i + 4) < len);
		outbuf[i] = '\0';
	}
	
	void Msg::WriteInt(int value) 
	{
		int32_t* dst;
		if (MaxSize - CurSize < 4) 
		{
			Overflowed = true;
			return;
		}

		dst = reinterpret_cast<int32_t*>(&Buffer[CurSize]);
		*dst = value;
		CurSize += sizeof(int32_t);
	}

	void Msg::WriteByte(int value)
	{
		int8_t* dst;
		if (MaxSize - CurSize < 1) 
		{
			Overflowed = true;
			return;
		}

		dst = reinterpret_cast<int8_t*>(&Buffer[CurSize]);
		*dst = value;
		CurSize += sizeof(int8_t);
	}

	void Msg::WriteShort(int value)
	{
		signed short* dst;
		if (MaxSize - CurSize < 2) 
		{
			Overflowed = true;
			return;
		}

		dst = reinterpret_cast<int16_t*>(&Buffer[CurSize]);
		*dst = value;
		CurSize += sizeof(short);
	}

	void Msg::WriteBit0()
	{
		if (!(Bit & 7))
		{
			if (MaxSize <= CurSize)
			{
				Overflowed = true;
				return;
			}
			Bit = CurSize * 8;
			Buffer[CurSize] = 0;
			CurSize++;
		}
		Bit++;
	}

	void Msg::WriteBit1()
	{
		if (!(Bit & 7))
		{
			if (CurSize >= MaxSize)
			{
				Overflowed = true;
				return;
			}
			Bit = 8 * CurSize;
			Buffer[CurSize] = 0;
			CurSize++;
		}
		Buffer[Bit >> 3] |= 1 << (Bit & 7);
		Bit++;
	}

	void Msg::WriteBits(int bits, int bitcount)
	{
		int i;
		if (MaxSize - CurSize < 4)
		{
			Overflowed = true;
			return;
		}

		for (i = 0; bitcount != i; i++)
		{
			if (!(Bit & 7))
			{
				Bit = 8 * CurSize;
				Buffer[CurSize] = 0;
				CurSize++;
			}
			if (bits & 1)
				Buffer[Bit >> 3] |= 1 << (Bit & 7);

			Bit++;
			bits >>= 1;
		}
	}

	void Msg::WriteString(const char* string) 
	{
		for (int i = 0; i < strlen(string); i++)
			WriteByte(string[i]);
		WriteByte('\0');
	}

	void Msg::WriteOriginFloat(int bits, float value, float oldValue)
	{
		signed int ival;
		signed int ioldval;
		signed int mcenterbits, delta;
		float center[3];

		if (Protocol > COD4X_FALLBACK_PROTOCOL)
		{
			ival = *reinterpret_cast<int*>(&value);
			WriteInt(ival);
			return;
		}

		ival = static_cast<signed int>(floorf(value + 0.5f));
		ioldval = static_cast<signed int>(floorf(oldValue + 0.5f));
		delta = ival - ioldval;

		if (static_cast<uint32_t>(delta + 64) > 127)
		{
			VectorCopy(MapCenter, center);

			WriteBit1();
			mcenterbits = static_cast<signed int>(center[bits != -92] + 0.5);
			WriteBits((ival - mcenterbits + 0x8000) ^ (ioldval - mcenterbits + 0x8000), 16);
		}
		else
		{
			WriteBit0();
			WriteBits(delta + 64, 7);
		}
	}

	void Msg::WriteOriginZFloat(float value, float oldValue)
	{
		signed int ival;
		signed int ioldval;
		signed int mcenterbits;
		float center[3];

		if (Protocol > COD4X_FALLBACK_PROTOCOL)
		{
			ival = *reinterpret_cast<int*>(&value);
			WriteInt(ival);
			return;
		}

		ival = static_cast<signed int>(floorf(value + 0.5f));
		ioldval = static_cast<signed int>(floorf(oldValue + 0.5f));

		if (static_cast<uint32_t>(ival - ioldval + 64) > 127)
		{
			VectorCopy(MapCenter, center);

			WriteBit1();
			mcenterbits = static_cast<signed int>(center[2] + 0.5);
			WriteBits((ival - mcenterbits + 0x8000) ^ (ioldval - mcenterbits + 0x8000), 16);
		}
		else
		{
			WriteBit0();
			WriteBits(ival - ioldval + 64, 7);
		}
	}

	void Msg::WriteAngle16(float f)
	{
		WriteShort(ANGLE2SHORT(f));
	}

	void Msg::Write24BitFlag(const int oldFlags, const int newFlags)
	{
		int xorflags;
		signed int shiftedflags;
		signed int i;

		xorflags = newFlags ^ oldFlags;
		if ((xorflags - 1) & xorflags)
		{
			WriteBit1();
			shiftedflags = newFlags;

			for (i = 3; i; --i)
			{
				WriteByte(shiftedflags);
				shiftedflags >>= 8;
			}
		}
		else
		{
			for (i = 0; !(xorflags & 1); ++i)
				xorflags >>= 1;
			WriteBit0();
			WriteBits(i, 5);
		}
	}

	int Msg::GetUsedBitCount()
	{
		return ((CurSize + SplitSize) * 8) - ((8 - Bit) & 7);
	}

	int Msg::GetNumBitsRead()
	{
		return 8 * ReadCount - ((8 - Bit) & 7);
	}

	void Msg::ClearLastReferencedEntity()
	{
		LastRefEntity = -1;
	}

	void Msg::Discard()
	{
		CurSize = ReadCount;
		SplitSize = 0;
		Overflowed = true;
	}
}
