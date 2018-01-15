#include "HCAStreamChannel.h"
#include "clHCA.h"

HCAStreamChannel::HCAStreamChannel(HCADecodeService* dec)
{
	this->dec = dec;
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
		HCAStreamChannel(nullptr);
		return;
	}
	size = other.size;
	ptr = new char[size];
	for (size_t i = 0; i < size; ++i) ((char*)ptr)[i] = ((char*)other.ptr)[i];
	__load();
}

HCAStreamChannel::HCAStreamChannel(HCADecodeService* dec, const std::string& filename)
{
	this->dec = dec;
	this->flags = 0;
    load(filename, 0);
}


HCAStreamChannel::~HCAStreamChannel()
{
	unload();
}

HCAStreamChannel& HCAStreamChannel::operator=(HCAStreamChannel&& other)
{
    unload();
    if (this != &other)
    {
        ptr = other.ptr;
        size = other.size;
        playback_channel = other.playback_channel;
        decode_channel = other.decode_channel;
        flags = other.flags;
        other.ptr = nullptr;
        other.size = 0;
        other.playback_channel = 0;
        other.decode_channel = 0;
    }
    return *this;
}

void HCAStreamChannel::unload()
{
	dec->cancel_decode(ptr);
	if (playback_channel != 0) { BASS_ChannelStop(playback_channel); playback_channel = 0; }
	if (decode_channel != 0) { BASS_ChannelStop(decode_channel); decode_channel = 0; }
	if (ptr != nullptr) { delete[] ptr; ptr = nullptr; }
	size = 0;
}

bool HCAStreamChannel::load(const std::string& filename)
{
    auto pair = dec->decode(filename, 0);
	ptr = pair.first;
	size = pair.second;
	return __load();
}

bool HCAStreamChannel::load(const std::string& filename, DWORD samplenum)
{
    auto pair = dec->decode(filename, samplenum);
    ptr = pair.first;
    size = pair.second;
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
	playback_channel = BASS_StreamCreateFile(TRUE, ptr, 0, size, flags);
	if (playback_channel == 0)
	{
        dec->cancel_decode(ptr);
		delete[] ptr;
		ptr = nullptr;
		decode_channel = 0;
		size = 0;
		return false;
	}
	decode_channel = BASS_StreamCreateFile(TRUE, ptr, 0, size, flags | BASS_STREAM_DECODE);
	if (decode_channel == 0)
	{
        dec->cancel_decode(ptr);
		delete[] ptr;
		ptr = nullptr;
		BASS_ChannelStop(playback_channel);
		playback_channel = 0;
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
