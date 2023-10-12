
#pragma once

#include "ffsetup.hpp"
#include "ffstring.hpp"
#include "ffrawmem.hpp"
#include "fftime.hpp"
#include "ffmutex.hpp"

#include <iostream>
#include <Windows.h>
//#include <mutex>
//#include <shared_mutex>
#include <atomic>
#include <functional>

namespace ff
{
	struct sharedmem
	{
		static const uint slots = 16;

		struct processSection { std::atomic<uint> pid; ff::mutex mtx; std::atomic<uint> currentOffset; };
		struct processTimeout { ff::timer timer; bool readOnce = false; };

		ff::string name = "";
		HANDLE mapHandle = INVALID_HANDLE_VALUE;
		ff::rawmem mem;

		ff::dynarray<processTimeout> timeouts;

		sharedmem(ff::string _name);
		bool create(uint _size);
		void destroy();
		bool request(uint _size);

		bool isInitialized();

		uint clientWrite(std::function<uint(ff::rawmem)> _function);
		bool serverCanRead(uint& _slot, uint& _pid);
		ff::rawmem serverRead(ff::rawmem _memSave, uint _slot);
	};
}

ff::sharedmem::sharedmem(ff::string _name) { name = _name; timeouts.resize(ff::sharedmem::slots); }
bool ff::sharedmem::create(uint _size)
{
	mapHandle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_EXECUTE_READWRITE, 0, _size, (LPCSTR)name.data());
	if (mapHandle == NULL) { std::cout << "Couldn't create " << name.data() << "\n"; return false; }

	mem = ff::rawmem((byte*)MapViewOfFile(mapHandle, FILE_MAP_ALL_ACCESS, 0, 0, _size), _size);
	if (mem.address == nullptr) { CloseHandle(mapHandle); return false; }

	mem.zero();
	for (uint i = 0; i < ff::sharedmem::slots; i += 1)
	{
		ff::rawmem split = mem.split<ff::sharedmem::slots>(i);

		split.getAtByte<processSection>(0).currentOffset = 0;
		split.getAtByte<processSection>(0).pid = (uint)-1;
		split.getAtByte<processSection>(0).mtx = ff::mutex();
		//new (&split.getAtByte<processSection>(0).mtx) std::shared_mutex{}; // dumb placement new crap because default constructor is deleted

		timeouts[i].readOnce = false;

		std::cout << "Slot " << i << ": PID " << split.getAtByte<processSection>(0).pid << "\n";
	}
	
	return true;
}
void ff::sharedmem::destroy()
{
	if (mem.address == nullptr) { return; }
	UnmapViewOfFile(mem.address);
	CloseHandle(mapHandle);
}
bool ff::sharedmem::request(uint _size)
{
	mapHandle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, (LPCSTR)name.data());
	if (mapHandle == NULL) { return false; }
	
	mem = ff::rawmem((byte*)MapViewOfFile(mapHandle, FILE_MAP_ALL_ACCESS, 0, 0, _size), _size);
	if (mem.address == nullptr) { CloseHandle(mapHandle); return false; }

	return true;
}
bool ff::sharedmem::isInitialized() { return mem.address != nullptr; }
uint ff::sharedmem::clientWrite(std::function<uint(ff::rawmem)> _function)
{
	static const uint localPid = GetCurrentProcessId();

	/*for (uint i = 0; i < ff::sharedmem::slots; i += 1)
	{
		ff::rawmem split = mem.split<ff::sharedmem::slots>(i);
		uint splitPid = split.getAtByte<processSection>(0).pid;
		std::cout << "Slot " << i << ": PID " << splitPid << "\n";
	}*/

	for (uint i = 0; i < ff::sharedmem::slots; i += 1)
	{
		ff::rawmem split = mem.split<ff::sharedmem::slots>(i);
		uint splitPid = split.getAtByte<processSection>(0).pid;

		if (splitPid == (uint)-1 || splitPid == localPid)
		{
			split.getAtByte<processSection>(0).mtx.lock();

			uint offset = split.getAtByte<processSection>(0).currentOffset;
			uint added = _function(split.getSubMem(sizeof(processSection) + offset));
			split.getAtByte<processSection>(0).currentOffset = offset + added;
			split.getAtByte<processSection>(0).pid = localPid;

			split.getAtByte<processSection>(0).mtx.unlock();

			return added;
		}
	}

	return 0;
}
bool ff::sharedmem::serverCanRead(uint& _slot, uint& _pid)
{
	/*for (uint i = 0; i < ff::sharedmem::slots; i += 1)
	{
		ff::rawmem split = mem.split<ff::sharedmem::slots>(i);
		uint splitPid = split.getAtByte<processSection>(0).pid;
		std::cout << "Slot " << i << ": PID " << splitPid << "\n";
	}*/

	for (uint i = 0; i < ff::sharedmem::slots; i += 1)
	{
		ff::rawmem split = mem.split<ff::sharedmem::slots>(i);
		uint splitPid = split.getAtByte<processSection>(0).pid;
		if (splitPid != (uint)-1)
		{
			if (split.getAtByte<processSection>(0).currentOffset == 0)
			{
				if (!timeouts[i].readOnce) { timeouts[i].readOnce = true; timeouts[i].timer.restart(); }
				else if (timeouts[i].timer.waitedForMilli(10000))
				{
					split.getAtByte<processSection>(0).mtx.lock();
					split.getAtByte<processSection>(0).pid = -1;
					timeouts[i].readOnce = false;
					split.getAtByte<processSection>(0).mtx.unlock();
				}
			}
			else { timeouts[i].timer.restart(); }
		}
	}

	for (uint i = 0; i < ff::sharedmem::slots; i += 1)
	{
		ff::rawmem split = mem.split<ff::sharedmem::slots>(i);
		uint splitPid = split.getAtByte<processSection>(0).pid;
		if (splitPid != (uint)-1 && split.getAtByte<processSection>(0).currentOffset > 0) { _slot = i; _pid = splitPid; return true; }
	}
	return false;
}
ff::rawmem ff::sharedmem::serverRead(ff::rawmem _memSave, uint _slot)
{
	ff::rawmem split = mem.split<ff::sharedmem::slots>(_slot);

	split.getAtByte<processSection>(0).mtx.lock();

	uint splitSize = split.getAtByte<processSection>(0).currentOffset;
	split.getSubMem(sizeof(processSection), splitSize).copyTo(_memSave);
	split.getAtByte<processSection>(0).currentOffset = 0;

	split.getAtByte<processSection>(0).mtx.unlock();

	return ff::rawmem(_memSave.address, splitSize);
}


