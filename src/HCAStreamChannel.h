#pragma once

#include <string>
#include "bass.h"

class HCAStreamChannel
{
public:
	HCAStreamChannel();
    HCAStreamChannel(const HCAStreamChannel& other);
    HCAStreamChannel& operator=(HCAStreamChannel&& other)
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
	HCAStreamChannel(const std::string& filename);
	~HCAStreamChannel();
	void unload();
	bool load(const std::string& filename);
	bool valid();
	DWORD get_playback_channel();
	DWORD get_decode_channel();
	void set_flags(DWORD flags);
    void destroy_channels();
    void make_channels();
private:
	bool __load();
	void* ptr;
	size_t size;
	DWORD playback_channel, decode_channel;
	DWORD flags;
};

