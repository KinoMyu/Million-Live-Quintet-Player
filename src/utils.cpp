#include <QString>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "../bass/bass.h"
#include "../bass/bassmix.h"
#include "utils.h"
#include "HCAStreamChannel.h"

void export_to_wav(HSTREAM mix_stream, const std::string& filename, const std::map<QWORD, std::string>& event_list, const std::map<QWORD, std::string> &oneshot_event_list)
{
    short buf[100000];
    FILE* fp = fopen(filename.c_str(), "wb");
    WAVEFORMATEX wf;
    BASS_CHANNELINFO info;
    QWORD p = 0;
    int c;

    // Start WAV Header
    BASS_ChannelGetInfo(mix_stream, &info);
    wf.wFormatTag = 1;
    wf.nChannels = info.chans;
    wf.wBitsPerSample = (info.flags & BASS_SAMPLE_8BITS ? 8 : 16);
    wf.nBlockAlign = wf.nChannels * wf.wBitsPerSample / 8;
    wf.nSamplesPerSec = info.freq;
    wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
    fwrite("RIFF\0\0\0\0WAVEfmt \20\0\0\0", 20, 1, fp);
    fwrite(&wf, 16, 1, fp);
    fwrite("data\0\0\0\0", 8, 1, fp);

    // Write sample data
    do
    {
        // Get number of samples until next sync event
        int numsamples_to_read = 50000;
        auto it = event_list.upper_bound(p);
        if(it != event_list.end())
        {
            int samples_to_next = it->first - p;
            if(samples_to_next > 0 && samples_to_next < numsamples_to_read)
            {
                numsamples_to_read = it->first - p;
            }
        }
        it = oneshot_event_list.upper_bound(p);
        if(it != oneshot_event_list.end())
        {
            int samples_to_next = it->first - p;
            if(samples_to_next > 0 && samples_to_next < numsamples_to_read)
            {
                numsamples_to_read = it->first - p;
            }
        }
        c = BASS_ChannelGetData(mix_stream, buf, numsamples_to_read * 4);
        p += numsamples_to_read;
        fwrite(buf, 1, c, fp);
    } while (c > 0);
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

void parse_names(std::unordered_map<std::string, std::string>& filename_to_readable, const std::string& infile, QComboBox* sel[], int size)
{
    std::ifstream infilestream(infile);
    std::string line, readable, translated;
    std::stringstream ss;
    while (std::getline(infilestream, line))
    {
        ss = std::stringstream(line);
        std::getline(ss, translated, ':');
        std::getline(ss, readable);
        filename_to_readable[translated] = readable;
        for(int i = 0; i < size; ++i)
        {
            sel[i]->addItem(QString::fromLocal8Bit(readable.c_str()), QString::fromLocal8Bit(translated.c_str()));
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
        std::getline(ss, idol, ':');
        std::getline(ss, type);
        filename_to_type[idol] = type[0] & ALL;
    }
}
