#include "HCAStreamChannel.h"
#include "../FastHCADecoder/clHCA.h"

HCAStreamChannel::HCAStreamChannel(HCADecodeService* dec, float volume, unsigned int cipher_key_1, unsigned int cipher_key_2, unsigned int sub_key)
    : dec {dec},
      ptr {nullptr},
      size {0},
      playback_channel {0},
      decode_channel {0},
      flags {0},
      volume {volume},
      cipher_key_1 {cipher_key_1},
      cipher_key_2 {cipher_key_2},
      sub_key {sub_key}
{}

HCAStreamChannel::HCAStreamChannel(const HCAStreamChannel& other)
{
    if (other.ptr == nullptr)
    {
        HCAStreamChannel(other.dec);
        return;
    }
    dec = other.dec;
	flags = other.flags;
    size = other.size;
    volume = other.volume;
    cipher_key_1 = other.cipher_key_1;
    cipher_key_2 = other.cipher_key_2;
    ptr = new char[size];
    memcpy(ptr, other.ptr, size);
    __load();
}

HCAStreamChannel::HCAStreamChannel(HCADecodeService* dec, const std::string& filename, float volume, unsigned int cipher_key_1, unsigned int cipher_key_2, unsigned int sub_key)
  : dec {dec},
    flags {0},
    volume {volume},
    cipher_key_1 {cipher_key_1},
    cipher_key_2 {cipher_key_2},
    sub_key {sub_key}
{
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
        dec = other.dec;
        ptr = other.ptr;
        size = other.size;
        volume = other.volume;
        cipher_key_1 = other.cipher_key_1;
        cipher_key_2 = other.cipher_key_2;
        sub_key = other.sub_key;
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

void HCAStreamChannel::wait_for_decode()
{
    if(ptr == nullptr) return;
    dec->wait_on_request(ptr);
}

void HCAStreamChannel::unload()
{
    dec->cancel_decode(ptr);
    if (playback_channel != 0) { BASS_StreamFree(playback_channel); playback_channel = 0; }
    if (decode_channel != 0) { BASS_StreamFree(decode_channel); decode_channel = 0; }
    if (ptr != nullptr) { operator delete(ptr); ptr = nullptr; }
    size = 0;
}

bool HCAStreamChannel::load(const std::string& filename)
{
    unload();
    auto pair = dec->decode(filename.c_str(), 0, cipher_key_1, cipher_key_2, sub_key, volume);
    ptr = pair.first;
    size = pair.second;
    return __load();
}

bool HCAStreamChannel::load(const std::string& filename, DWORD samplenum)
{
    unload();
    auto pair = dec->decode(filename.c_str(), samplenum);
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
        operator delete(ptr);
        ptr = nullptr;
        decode_channel = 0;
        size = 0;
        return false;
    }
    decode_channel = BASS_StreamCreateFile(TRUE, ptr, 0, size, flags | BASS_STREAM_DECODE);
    if (decode_channel == 0)
    {
        dec->cancel_decode(ptr);
        operator delete(ptr);
        ptr = nullptr;
        BASS_StreamFree(playback_channel);
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
        BASS_StreamFree(playback_channel);
        playback_channel = 0;
    }
}

void HCAStreamChannel::set_volume(float volume) { this->volume = volume; }

void HCAStreamChannel::set_ciphkey(unsigned int cipher_key_1, unsigned int cipher_key_2, unsigned int sub_key)
{
    this->cipher_key_1 = cipher_key_1;
    this->cipher_key_2 = cipher_key_2;
    this->sub_key = sub_key;
}
