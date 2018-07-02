#pragma once

#ifndef HCASTREAMCHANNEL_H
#define HCASTREAMCHANNEL_H

#include <string>
#include "../bass/bass.h"
#include "../HCADecoder/HCADecodeService.h"

class HCAStreamChannel
{
public:
    HCAStreamChannel(HCADecodeService* dec, float volume = 1.0, unsigned int ciphKey1 = 0xBC731A85, unsigned int ciphKey2 = 0x0002B875);
    HCAStreamChannel(const HCAStreamChannel& other);
    HCAStreamChannel(HCADecodeService* dec, const std::string& filename, float volume = 1.0, unsigned int ciphKey1 = 0xBC731A85, unsigned int ciphKey2 = 0x0002B875);
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
    void set_volume(float volume);
    void set_ciphkey(unsigned int ciphKey1, unsigned int ciphKey2);
private:
    bool __load();
    HCADecodeService* dec;
    void* ptr;
    size_t size;
    DWORD playback_channel, decode_channel;
    DWORD flags;
    float volume;
    unsigned int ciphKey1, ciphKey2;
};

#endif //HCASTREAMCHANNEL_H
