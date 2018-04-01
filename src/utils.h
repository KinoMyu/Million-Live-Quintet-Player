#pragma once
#include <map>
#include <unordered_map>
#include <QComboBox>

#define NUM_IDOLS 13
#define MACHIUKE 4575494

struct VolumePan
{
    double vol;
    double pan;
};

typedef std::pair<double*, std::map<DWORD, VolumePan>> ControlInfo;

short safeadd(const short& a, const short& b);
double clamp(const double& d, const double& lower, const double& upper);
void export_to_wav(DWORD bgm, DWORD idols[], const double& bgmVol, const double& idolVol, ControlInfo idolControlInfo[], const std::string& filename, bool usotsuki);
void parse_control_file(ControlInfo idolInfo[], const std::string& control_file, double& idolVol, bool usotsuki);
void convert_to_left_right(VolumePan vp, double& leftVol, double& rightVol);
void fuzzy_adjust_vol_pan(DWORD channel, ControlInfo ci);
void CALLBACK adjust_vol_pan(HSYNC handle, DWORD channel, DWORD data, void *user);
void CALLBACK add_usotsuki(HSYNC handle, DWORD channel, DWORD data, void* user);
void set_auto_vol_pan_all(ControlInfo idolControlInfo[], DWORD idols[]);
void set_auto_vol_pan(const ControlInfo& idolControlInfo, DWORD idolchannel);
void parse_names(std::unordered_map<std::string, std::string>& readable_to_filename, const std::string& infile, QComboBox* sel[], int size);
