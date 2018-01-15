#include <sstream>
#include <fstream>
#include "bass.h"
#include "utils.h"
#include <iostream>
#include <QString>

short safeadd(const short& a, const short& b)
{
    if ((b > 0) && (a > SHRT_MAX - b)) return SHRT_MAX;
    if ((b < 0) && (a < SHRT_MIN - b)) return SHRT_MIN;
    return a + b;
}

void export_to_wav(DWORD bgm, DWORD idols[], const double& bgmVol, const double& idolVol, ControlInfo idolControlInfo[], const std::string& filename)
{
    int pos = 0;
    short tempbuf[10000];
    short buf[10000];
    FILE* fp = fopen(filename.c_str(), "wb");
    VolumePan idolvolpan[NUM_IDOLS];
    WAVEFORMATEX wf;
    BASS_CHANNELINFO info;
    DWORD p, index;
    double leftVol, rightVol;
    std::map<DWORD, VolumePan>::iterator iter;

    // Reset positions to start
    BASS_ChannelSetPosition(bgm, 0, BASS_POS_BYTE);
    for (int j = 0; j < NUM_IDOLS; ++j)
    {
        BASS_ChannelSetPosition(idols[j], 0, BASS_POS_BYTE);
    }

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
        int c = BASS_ChannelGetData(bgm, tempbuf, 20000);
        for (int i = 0; i < 10000; ++i)
        {
            buf[i] = bgmVol * tempbuf[i];
        }
        for (int j = 0; j < NUM_IDOLS; ++j)
        {
            if (idols[j] != 0)
            {
                convert_to_left_right(idolvolpan[j], leftVol, rightVol);
                BASS_ChannelGetData(idols[j], tempbuf, 10000);
                for (int i = 0; i < 10000; ++i)
                {
                    index = pos / 4 + i / 2;
                    iter = idolControlInfo[j].second.find(index);
                    if (iter != idolControlInfo[j].second.end())
                    {
                        idolvolpan[j] = idolControlInfo[j].second[index];
                        convert_to_left_right(idolvolpan[j], leftVol, rightVol);
                    }
                    if (!(i % 2))
                    {
                        buf[i] = safeadd(idolVol * leftVol * tempbuf[i / 2], buf[i]);
                    }
                    else
                    {
                        buf[i] = safeadd(idolVol * rightVol * tempbuf[i / 2], buf[i]);
                    }
                }
            }
        }
        fwrite(buf, 1, c, fp);
        pos = BASS_ChannelGetPosition(bgm, BASS_POS_BYTE);
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

void parse_control_file(ControlInfo idolInfo[], const std::string & control_file, double& idolVol)
{
    double volTable[] = { 0.75, 0.62, 0.53, 0.47, 0.42 };
    VolumePan idolvolpan[NUM_IDOLS];
    std::ifstream infilestream(control_file);
    std::string line;
    int position;
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        idolInfo[i].second.clear();
    }
    while (std::getline(infilestream, line))
    {
        std::istringstream iss(line);
        iss >> position;
        std::getline(infilestream, line);
        for (int i = 0; i < NUM_IDOLS; ++i)
        {
            idolvolpan[i] = { 0, 0 };
        }
        int numIdols = line.length() - 1;
        for (int i = 0; i <= numIdols; ++i)
        {
            idolvolpan[line.at(i) - 48] = { volTable[numIdols], -0.2 * (numIdols / 2.0 - i) };
        }
        for (int i = 0; i < NUM_IDOLS; ++i)
        {
            idolInfo[i].second[position] = idolvolpan[i];
        }
    }
    for (int i = 0; i < NUM_IDOLS; ++i)
    {
        idolInfo[i].first = &idolVol;
    }
}

void convert_to_left_right(VolumePan vp, double& leftVol, double& rightVol)
{
    leftVol = (1 - vp.pan/2) * vp.vol;
    rightVol = (1 + vp.pan/2) * vp.vol;
}

void fuzzy_adjust_vol_pan(DWORD channel, ControlInfo ci)
{
    if (channel == 0) return;
    VolumePan vp;
    DWORD pos = BASS_ChannelGetPosition(channel , BASS_POS_BYTE) / 2;
    DWORD last_pos = 0;
    for (std::pair<DWORD, VolumePan> pos_vp : ci.second)
    {
        if (pos >= last_pos && pos <= pos_vp.first)
            break;
        last_pos = pos_vp.first;
        vp = pos_vp.second;
    }
    BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, vp.vol * *ci.first);
    BASS_ChannelSetAttribute(channel, BASS_ATTRIB_PAN, vp.pan);
}

void CALLBACK adjust_vol_pan(HSYNC handle, DWORD channel, DWORD data, void* user)
{
    DWORD pos = BASS_ChannelGetPosition(channel, BASS_POS_BYTE) / 2;
    VolumePan vp = (*(ControlInfo*)user).second[pos];
    BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, vp.vol * *(*(ControlInfo*)user).first);
    BASS_ChannelSetAttribute(channel, BASS_ATTRIB_PAN, vp.pan);
}

void set_auto_vol_pan_all(ControlInfo idolControlInfo[], DWORD idols[])
{
    for (int i = 0; i < NUM_IDOLS; ++i)
    {
        set_auto_vol_pan(idolControlInfo[i], idols[i]);
    }
}

void set_auto_vol_pan(const ControlInfo& idolControlInfo, DWORD idolchannel)
{
    if (idolchannel != 0)
    {
        for (std::pair<DWORD, VolumePan> pos_vp : idolControlInfo.second)
        {
            BASS_ChannelSetSync(idolchannel, BASS_SYNC_POS, pos_vp.first * 2, adjust_vol_pan, (void*)&idolControlInfo);
        }
    }
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
            sel[i]->addItem(QString::fromStdString(readable));
        }
    }
}
