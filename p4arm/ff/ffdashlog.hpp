
#pragma once

#include "ffsharedmem.hpp"

#include "ffcompcontainer.hpp"
#include "ffcomphierarchy.hpp"
#include "ffcompid.hpp"
#include "ffcolor.hpp"
#include "ffdashlogtypes.hpp"
#include "fftime.hpp"


namespace ff
{

	/// \brief Dash console writing utility
	struct dlog
	{
		static ff::sharedmem sharedMemory;

		static byte buf[1024 * 1024];
		static ff::rawmem buffer;
		static uint writeIdx;


		static ff::compidmanager<dashlog::container> containerManager;
		static ff::comphierarchy<dashlog::container> containerHierarchy;

		static ff::color remoteColor;
		static ff::color currentColor;

		static ff::id<dashlog::container> remoteContainerId;
		static ff::id<dashlog::container> currentContainerId;

		static int enabled;


		dlog();
		~dlog();

		
		/// \brief Guarantees that shared memory is requested and that the buffered data is emptied (placed inside the shared memory)
		static void waitSend();

		/// \brief Creates and switches to a children container of the current container
		/// \return The ID of the newly created child container
		static ff::id<dashlog::container> makeSwitchChildContainer();

		/// \brief Switches to the parent of the current container
		static void switchParentContainer();

		ff::id<dashlog::container> makeAdjacentContainer();
		void switchToContainer(ff::id<dashlog::container> _id);

		template<typename T> dlog& operator<<(T _print);

		/// \brief Disable all dashlog commands
		static void disable();

		/// \brief Enable all dashlog commands
		static void enable();
	};
}


ff::sharedmem ff::dlog::sharedMemory = ff::sharedmem("dashconsole");
byte ff::dlog::buf[] = { 0 };
ff::rawmem ff::dlog::buffer = ff::rawmem(ff::dlog::buf, 1024 * 1024);
uint ff::dlog::writeIdx = 0;
ff::compidmanager<ff::dashlog::container> ff::dlog::containerManager = ff::compidmanager<ff::dashlog::container>();
ff::comphierarchy<ff::dashlog::container> ff::dlog::containerHierarchy = ff::comphierarchy<ff::dashlog::container>();

ff::color ff::dlog::remoteColor = ff::color::white();
ff::color ff::dlog::currentColor = ff::color::white();
ff::id<ff::dashlog::container> ff::dlog::remoteContainerId = ff::id<ff::dashlog::container>();
ff::id<ff::dashlog::container> ff::dlog::currentContainerId = ff::dlog::containerManager.addNew();

int ff::dlog::enabled = 1;


ff::dlog::dlog() { /*TODO*/ }
ff::dlog::~dlog() { if (enabled <= 0) { return; } waitSend(); }
void ff::dlog::waitSend()
{
	if (ff::dlog::writeIdx == 0) { return; }

	if (!sharedMemory.isInitialized())
	{
		sharedMemory.request(1024 * 1024 * 4);

		ff::timer timer;
		while (!sharedMemory.isInitialized()) { sharedMemory.request(1024 * 1024 * 4); if (timer.waitedForMilli(3000)) { std::cout << "Waiting for dash console...\n"; timer.restart(); } }
	}

	ff::rawmem stuff = ff::rawmem(ff::dlog::buffer.address, ff::dlog::writeIdx);

	uint added = sharedMemory.clientWrite([](ff::rawmem _mem) { return ff::rawmem(ff::dlog::buffer.address, ff::dlog::writeIdx).copyTo(_mem); });
	while (added == 0) { added = sharedMemory.clientWrite([](ff::rawmem _mem) { return ff::rawmem(ff::dlog::buffer.address, ff::dlog::writeIdx).copyTo(_mem); }); }

	if (stuff.read<char>() == 0) { std::cout << "!!! BAD SEND\n"; ff::rawmem(ff::dlog::buffer.address, ff::dlog::writeIdx).printString(); }
	//if (added <= 18) { std::cout << "!!! BAD SEND\n"; ff::rawmem(ff::dlog::buffer.address, ff::dlog::writeIdx).printString(); }

	ff::dlog::writeIdx = 0;
}
ff::id<ff::dashlog::container> ff::dlog::makeSwitchChildContainer()
{
	if (enabled <= 0) { return ff::id<dashlog::container>(); }

	ff::id<dashlog::container> child = ff::dlog::containerManager.addNew();

	// Send "containerHierarchy"
	ff::rawmem writableMem = ff::dlog::buffer.getSubMem(ff::dlog::writeIdx);
	if (sizeof(dashlog::nSend::type) + 2 * sizeof(uint) > writableMem.size) { waitSend(); }
	ff::dlog::writeIdx += writableMem.write<dashlog::nSend::type, uint, uint>(dashlog::nSend::containerHierarchy, ff::dlog::currentContainerId.value, child.value);

	waitSend();

	ff::dlog::containerHierarchy.setParent(child, ff::dlog::currentContainerId);
	ff::dlog::currentContainerId = child;

	//ff::rawmem writableMem = ff::dlog::buffer.getSubMem(ff::dlog::writeIdx);
	//ff::dlog::writeIdx += writableMem.write(nDlogSend::containerHierarchy, dlogSendContainerHierarchy(currentContainerId, result));

	return child;
}
void ff::dlog::switchParentContainer()
{
	if (enabled <= 0) { return; }

	ff::id<dashlog::container> parent = ff::dlog::containerHierarchy.getParent(ff::dlog::currentContainerId);

	ff::dlog::currentContainerId = parent;
}
ff::id<ff::dashlog::container> ff::dlog::makeAdjacentContainer()
{
	if (enabled <= 0) { return ff::id<dashlog::container>(); }

	ff::id<dashlog::container> result = ff::dlog::containerManager.addNew();

	//ff::rawmem writableMem = ff::dlog::buffer.getSubMem(ff::dlog::writeIdx);
	//ff::dlog::writeIdx += writableMem.write(nDlogSend::containerHierarchy, dlogSendContainerHierarchy(currentContainerId, result));

	return result;
}
void ff::dlog::switchToContainer(ff::id<dashlog::container> _id)
{
	if (enabled <= 0) { return; }

	currentContainerId = _id;
}
template<typename T> ff::dlog& ff::dlog::operator<<(T _print)
{
	if (enabled <= 0) { return *this; }

	ff::string text = (ff::string)_print;
	ff::rawmem writableMem = ff::dlog::buffer.getSubMem(ff::dlog::writeIdx);

	//if (ff::dlog::remoteColor != ff::dlog::currentColor) {  } TODO color text
	if (ff::dlog::remoteContainerId != ff::dlog::currentContainerId)
	{
		ff::dlog::remoteContainerId = ff::dlog::currentContainerId;

		if (sizeof(dashlog::nSend::type) + sizeof(uint) > writableMem.size) { waitSend(); }
		ff::dlog::writeIdx += writableMem.write(dashlog::nSend::containerSwitch, ff::dlog::currentContainerId.value);
		writableMem = ff::dlog::buffer.getSubMem(ff::dlog::writeIdx);
	}


	if (sizeof(dashlog::nSend::type) + text.getSerialSaveSize() > writableMem.size) { waitSend(); }

	ff::dlog::writeIdx += writableMem.write(dashlog::nSend::text);
	writableMem = ff::dlog::buffer.getSubMem(ff::dlog::writeIdx);
	ff::dlog::writeIdx += text.serialSave(writableMem);

	return *this;
}
void ff::dlog::enable() { enabled += 1; }
void ff::dlog::disable() { if (enabled > 0) { waitSend(); } enabled -= 1; }