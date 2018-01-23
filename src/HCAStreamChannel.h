#pragma once

#ifndef HCASTREAMCHANNEL_H
#define HCASTREAMCHANNEL_H

#include <string>
#include "../bass/bass.h"
#include "../HCADecoder/HCADecodeService.h"

class HCAStreamChannel
{
public:
    HCAStreamChannel(HCADecodeService* dec);
    HCAStreamChannel(const HCAStreamChannel& other);
    HCAStreamChannel(HCADecodeService* dec, const std::string& filename);
    HCAStreamChannel& operator=(HCAStreamChannel&& other);
    ~HCAStreamChannel();
    void wait_for_decode();
    void unload();
    bool load(const std::string& filename);
    bool load(const std::string& filename, DWORD samplenum);
    bool valid();
    DWORD get_playback_channel();
    DWORD get_decode_channel();
    void set_flags(DWORD flags);
    void destroy_channels();
    void make_channels();
private:
    bool __load();
    HCADecodeService* dec;
    void* ptr;
    size_t size;
    DWORD playback_channel, decode_channel;
    DWORD flags;
};

#endif //HCASTREAMCHANNEL_H
