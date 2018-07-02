#include <QString>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "../bass/bass.h"
#include "../bass/bassmix.h"
#include "utils.h"
#include "HCAStreamChannel.h"

void export_to_wav(DWORD bgm, HSTREAM mix_stream, const std::string& filename)
{
    short tempbuf[10000];
    short buf[10000];
    FILE* fp = fopen(filename.c_str(), "wb");
    WAVEFORMATEX wf;
    BASS_CHANNELINFO info;
    DWORD p;

    // Start WAV Header
    BASS_ChannelGetInfo(bgm, &info);
    wf.wFormatTag = 1;
    wf.nChannels = info.chans;
    wf.wBitsPerSample = (info.flags&BASS_SAMPLE_8BITS ? 8 : 16);
    wf.nBlockAlign = wf.nChannels*wf.wBitsPerSample / 8;
    wf.nSamplesPerSec = info.freq;
    wf.nAvgBytesPerSec = wf.nSamplesPerSec*wf.nBlockAlign;
    fwrite("RIFF\0\0\0\0WAVEfmt \20\0\0\0", 20, 1, fp);
    fwrite(&wf, 16, 1, fp);
    fwrite("data\0\0\0\0", 8, 1, fp);

    // Write sample data
    while (BASS_ChannelIsActive(bgm))
    {
        int c = BASS_ChannelGetData(mix_stream, tempbuf, 20000);
        for (int i = 0; i < 10000; ++i)
        {
            buf[i] = tempbuf[i];
        }

        fwrite(buf, 1, c, fp);
    }
    // Complete WAV header
    fflush(fp);
    p = ftell(fp);
    fseek(fp, 4, SEEK_SET);
    putw(p - 8, fp);
    fflush(fp);
    fseek(fp, 40, SEEK_SET);
    putw(p - 44, fp);
    fflush(fp);
    fclose(fp);
}

void parse_control_file(std::map<QWORD, std::string>& event_list, const std::string & control_file)
{
    event_list.clear();
    std::string line;
    DWORD position = 0;
    std::ifstream infilestream(control_file);
    while (std::getline(infilestream, line))
    {
        std::istringstream iss(line);
        iss >> position;
        std::getline(infilestream, line);
        event_list[position] = line;
    }
}

void CALLBACK add_usotsuki(HSYNC handle, DWORD channel, DWORD data, void* user)
{
    DWORD idolchan = ((HCAStreamChannel*)user)->get_decode_channel();
    QWORD len = BASS_ChannelGetLength(idolchan, BASS_POS_BYTE);
    QWORD mappos = BASS_ChannelGetPosition(channel, BASS_POS_BYTE) / 2 - MACHIUKE * 2;
    BASS_ChannelSetPosition(idolchan, mappos >= len || mappos < 0 ? len - 1 : mappos, BASS_POS_BYTE);
}

void parse_names(std::unordered_map<std::string, std::string>& readable_to_filename, const std::string& infile, QComboBox* sel[], int size)
{
    std::ifstream infilestream(infile);
    std::string line, readable, translated;
    std::stringstream ss;
    while (std::getline(infilestream, line))
    {
        ss = std::stringstream(line);
        std::getline(ss, readable, ':');
        std::getline(ss, translated);
        readable_to_filename[readable] = translated;
        for(int i = 0; i < size; ++i)
        {
            sel[i]->addItem(QString::fromLocal8Bit(readable.c_str()));
        }
    }
}

void parse_types(std::unordered_map<std::string, char>& filename_to_type, const std::string& infile)
{
    std::ifstream infilestream(infile);
    std::string line, idol, type;
    std::stringstream ss;
    while (std::getline(infilestream, line))
    {
        ss = std::stringstream(line);
        std::getline(ss, type, ':');
        std::getline(ss, idol);
        filename_to_type[idol] = type[0] & ALL;
    }
}
