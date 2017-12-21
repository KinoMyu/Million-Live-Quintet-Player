#include "HCAStreamChannel.h"
#include "clHCA.h"



HCAStreamChannel::HCAStreamChannel()
{
	this->flags = 0;
	ptr = nullptr;
	size = 0;
	playback_channel = 0;
	decode_channel = 0;
}

HCAStreamChannel::HCAStreamChannel(const HCAStreamChannel& other)
{
	if (other.ptr == nullptr)
	{
		HCAStreamChannel();
		return;
	}
	size = other.size;
	ptr = new char[size];
	for (size_t i = 0; i < size; ++i) ((char*)ptr)[i] = ((char*)other.ptr)[i];
	__load();
}

HCAStreamChannel::HCAStreamChannel(const std::string& filename)
{
	this->flags = 0;
	load(filename);
}


HCAStreamChannel::~HCAStreamChannel()
{
	unload();
}

void HCAStreamChannel::unload()
{
    destroy_channels();
	if (ptr != nullptr) { delete[] ptr; ptr = nullptr; }
	size = 0;
}

bool HCAStreamChannel::load(const std::string& filename)
{
	clHCA hca(0xBC731A85, 0x0002B875);
	ptr = hca.DecodeToMemory(size, filename.c_str());
	return __load();
}

bool HCAStreamChannel::__load()
{
	if (ptr == nullptr)
	{
		playback_channel = 0;
		decode_channel = 0;
		return false;
    }
    make_channels();
    if (playback_channel == 0 || decode_channel == 0)
	{
        destroy_channels();
		delete[] ptr;
		ptr = nullptr;
        playback_channel = 0;
		decode_channel = 0;
		size = 0;
		return false;
    }
	return true;
}

bool HCAStreamChannel::valid()
{
	return ptr != nullptr;
}

DWORD HCAStreamChannel::get_playback_channel()
{
	return playback_channel;
}

DWORD HCAStreamChannel::get_decode_channel()
{
	return decode_channel;
}

void HCAStreamChannel::set_flags(DWORD flags)
{
	this->flags = flags;
}

void HCAStreamChannel::destroy_channels()
{
    if (playback_channel != 0) { BASS_StreamFree(playback_channel); playback_channel = 0; }
    if (decode_channel != 0) { BASS_StreamFree(decode_channel); decode_channel = 0; }
}

void HCAStreamChannel::make_channels()
{
    playback_channel = BASS_StreamCreateFile(TRUE, ptr, 0, size, flags);
    if (playback_channel == 0)
    {
        decode_channel = 0;
    }
    decode_channel = BASS_StreamCreateFile(TRUE, ptr, 0, size, flags | BASS_STREAM_DECODE);
    if (decode_channel == 0)
    {
        BASS_ChannelStop(playback_channel);
        playback_channel = 0;
    }
}
