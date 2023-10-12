
#pragma once

#include "ffdynarray.hpp"
#include "ffstring.hpp"
#include "ffunistring.hpp"

#include "fflog.hpp"

namespace ff
{
	namespace serial
	{
		ff::dynarray<byte> saveUint(const uint& _uint)
		{
			ff::dynarray<byte> result;
			for (uint i = 0; i < sizeof(uint); i += 1) { result.pushback((byte)(_uint >> i * 8)); } // small endian storage (least significant byte first)
			return result;
		}
		uint loadUint(byte*& _data)
		{
			uint result = 0;
			for (uint i = 0; i < sizeof(uint); i += 1) { result |= _data[i] << i * 8; }
			_data += sizeof(uint);
			return result;
		}

		ff::dynarray<byte> saveString(const ff::string& _string)
		{
			ff::dynarray<byte> result = saveUint(_string.size());
			for (uint i = 0; i < _string.size(); i += 1) { result.pushback((byte)_string[i]); }
			return result;
		}
		ff::string loadString(byte*& _data)
		{
			uint size = loadUint(_data);
			ff::string result;
			for (uint i = 0; i < size; i += 1) { result.pushback((char)_data[i]); }
			_data += size;
			return result;
		}

		ff::dynarray<byte> saveCharUtf8(const uint32& _char)
		{
			ff::dynarray<byte> result;
			if (_char >= 0 && _char <= 0x7F)
			{
				return { (byte)(0b00000000 | (0b01111111 & (byte)(_char >> 0))) };
			}
			else if (_char >= 0x80 && _char <= 0x7FF)
			{
				return { (byte)(0b11000000 | (0b00011111 & (byte)(_char >> 6))), (byte)(0b10000000 | (0b00111111 & (byte)(_char >> 0))) }; // most significant byte first
			}
			else if (_char >= 0x800 && _char <= 0xFFFF)
			{
				return { (byte)(0b11100000 | (0b00001111 & (byte)(_char >> 12))), (byte)(0b10000000 | (0b00111111 & (byte)(_char >> 6))), (byte)(0b10000000 | (0b00111111 & (byte)(_char >> 0))) }; // most significant byte first
			}
			else if (_char >= 0x10000 && _char <= 0x10FFFF)
			{
				return { (byte)(0b11110000 | (0b00000111 & (byte)(_char >> 18))), (byte)(0b10000000 | (0b00111111 & (byte)(_char >> 12))), (byte)(0b10000000 | (0b00111111 & (byte)(_char >> 6))), (byte)(0b10000000 | (0b00111111 & (byte)(_char >> 0))) }; // most significant byte first
			}
			else { return {}; } // TODO return error
			return result;
		}
		uint32 loadCharUtf8(byte*& _data)
		{
			uint32 result = 0;
			if ((_data[0] & 0b10000000) == 0b00000000) // MSByte starts with 0
			{
				result += ((_data[0] & 0b01111111) << 0);
				_data += 1;
			}
			else if ((_data[0] & 0b11100000) == 0b11000000) // MSByte starts with 110
			{
				result += ((_data[0] & 0b00011111) << 6) + ((_data[1] & 0b00111111) << 0);
				_data += 2;
			}
			else if ((_data[0] & 0b11110000) == 0b11100000) // MSByte starts with 1110
			{
				result += ((_data[0] & 0b00001111) << 12) + ((_data[1] & 0b00111111) << 6) + ((_data[2] & 0b00111111) << 0);
				_data += 3;
			}
			else if ((_data[0] & 0b11111000) == 0b11110000) // MSByte starts with 11110
			{
				result += ((_data[0] & 0b00000111) << 18) + ((_data[1] & 0b00111111) << 12) + ((_data[2] & 0b00111111) << 6) + ((_data[3] & 0b00111111) << 0);
				_data += 4;
			}
			return result;
		}
		ff::dynarray<byte> saveUnicodeStringUtf8(const ff::unistring& _string)
		{
			ff::dynarray<byte> result;
			for (uint i = 0; i < _string.chars.size(); i += 1) { result.pushback(ff::serial::saveCharUtf8(_string.chars[i])); }
			result.pushback(ff::serial::saveCharUtf8(0));
			return result;
		}
		ff::unistring loadUnicodeStringUtf8(byte*& _ptr)
		{
			ff::unistring result;
			if (_ptr == nullptr) { return result; }

			while (_ptr[0] != '\0')
			{
				result.pushback(loadCharUtf8(_ptr));
			}
			_ptr += 1; // advance pointer for the '\0'
			return result;
		}
	}
}