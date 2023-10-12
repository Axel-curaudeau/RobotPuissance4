
#pragma once

#include "ffunistring.hpp"
#include "ffserial.hpp"

#include "fflog.hpp"

#include <windows.h>
#include <string>
ff::unistring getCurrentDirectory()
{
	WCHAR buffer[MAX_PATH];
	GetModuleFileNameA(NULL, (LPSTR)buffer, MAX_PATH);
	std::wstring::size_type position = std::wstring(buffer).find_last_of(L"\\/");
	return std::wstring(buffer).substr(0, position);
}

enum class nLoadFile { success = 0, fileIsFolder, fileNotFound, fileCantRead };

namespace ff
{
	struct file
	{
		ff::string absolutePath = "";
		bool isFolder = false;

		file() {}
		file(ff::string _path)
		{
			isFolder = !_path.find(".");

			if (_path.findAtIndex(":", 1)) { absolutePath = _path; }
			else { absolutePath = (ff::string)getCurrentDirectory() + "\\" + _path; }
		}
		bool operator==(const file& _r) { return absolutePath == _r.absolutePath; }

		ff::string read() // TODO: rename to loadAscii
		{
			if (isFolder) { return "ERROR_FILE_IS_FOLDER"; }

			std::fstream fileReader = std::fstream(absolutePath, std::ios::in | std::ios::binary);
			if (!fileReader.is_open()) { return "ERROR_READ_FILE_NOTFOUND"; }

			// Get the size of the file
			fileReader.seekg(0, fileReader.end); // set reader position to the end
			std::streamsize fileSize = fileReader.tellg(); // get reader position
			fileReader.seekg(0, fileReader.beg); // set reader position to the start


			ff::string buffer;
			buffer.storage.resize((uint)fileSize);
			//buffer.pushback('\0');

			// Read the file into the buffer
			fileReader.read(buffer.data(), fileSize);

			if (fileReader.fail()) { fileReader.close(); return "ERROR_READ_FILE_CANTREAD"; }

			// Remove '\r' (carriage return) characters from the file
			// This is done because on Windows, new lines are encoded with 2 bytes ("\r\n") instead of 1 ("\n")
			//
			// (side note: fstream is capable of automatically removing \r characters, but it doesn't accept the filesize as parameter of read, this method is simpler)
			//
			for (uint i = 0; i < buffer.size();)
			{
				if (buffer[i] == '\r') { buffer.storage.erase(i); }
				else { i += 1; }
			}

			fileReader.close();

			return buffer;
		}
		nLoadFile loadUnicodeString(ff::unistring& _result, bool _sanitize = true)
		{
			if (isFolder) { return nLoadFile::fileIsFolder; }

			std::fstream fileReader = std::fstream(absolutePath, std::ios::in | std::ios::binary);
			if (!fileReader.is_open()) { return nLoadFile::fileNotFound; }

			// Get the size of the file
			fileReader.seekg(0, fileReader.end); // set reader position to the end
			std::streamsize fileSize = fileReader.tellg(); // get reader position
			fileReader.seekg(0, fileReader.beg); // set reader position to the start

			ff::dynarray<byte> buffer;
			buffer.resize((uint)fileSize + 1);

			// Read the file into the buffer
			fileReader.read((char*)buffer.data(), fileSize);
			buffer[(uint)fileSize] = '\0';
			if (fileReader.fail()) { fileReader.close(); return nLoadFile::fileCantRead; }
			fileReader.close();

			byte* ptr = buffer.data();
			_result = ff::serial::loadUnicodeStringUtf8(ptr);
			if (_sanitize)
			{
				// Remove '\r' (carriage return) characters from the file
				// This is done because on Windows, new lines are encoded with 2 bytes ("\r\n") instead of 1 ("\n")
				//
				// (side note: fstream is capable of automatically removing \r characters, but it doesn't accept the filesize as parameter of read, this method is simpler)
				//
				for (uint i = 0; i < _result.size();)
				{
					if (_result[i] == '\r') { _result.chars.erase(i); }
					else { i += 1; }
				}
			}

			return nLoadFile::success;
		}

		ff::dynarray<byte> loadBin()
		{
			if (isFolder) { return {}; }

			std::fstream fileReader = std::fstream(absolutePath, std::ios::in | std::ios::binary);
			if (!fileReader.is_open()) { return {}; }

			// Get the size of the file
			fileReader.seekg(0, fileReader.end); // set reader position to the end
			std::streamsize fileSize = fileReader.tellg(); // get reader position
			fileReader.seekg(0, fileReader.beg); // set reader position to the start


			ff::dynarray<byte> buffer;
			buffer.resize((uint)fileSize);
			//buffer.pushback('\0');

			// Read the file into the buffer
			fileReader.read((char*)buffer.data(), fileSize);

			if (fileReader.fail()) { fileReader.close(); return {}; }


			fileReader.close();

			return buffer;
		}

		bool saveBin(ff::dynarray<byte> _binaryData)
		{
			if (isFolder) { return false; }

			std::fstream fileReader = std::fstream(absolutePath, std::ios::out | std::ios::binary);
			if (!fileReader.is_open()) { return {}; }

			fileReader.write((char*)_binaryData.data(), _binaryData.size());

			if (fileReader.fail()) { fileReader.close(); return false; }

			fileReader.close();

			return true;
		}
	};
}

// ABSOLUTE PATHS:
//		windows: C:\Users\ZeroZ30o\Desktop\ZeroZ30o\projects\visualStudio\shardParser\Debug\parser.exe
//		linux: /var/boot/ex.txt
// RELATIVE PATHS:
//		windows: Debug\parser.exe
//		linux: boot/ex.txt