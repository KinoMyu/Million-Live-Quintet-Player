#pragma once
#include <map>
#include <unordered_map>
#include <QComboBox>

#define NUM_IDOLS 13
#define MACHIUKE 4575494
#define ALL 0x15
#define PRINCESS 0x10
#define FAIRY 0x4
#define ANGEL 0x1

void export_to_wav(DWORD bgm, HSTREAM mix_stream, const std::string& filename);
void parse_control_file(std::map<QWORD, std::string>& event_list, const std::string & control_file);
void CALLBACK add_usotsuki(HSYNC handle, DWORD channel, DWORD data, void* user);
void parse_names(std::unordered_map<std::string, std::string>& readable_to_filename, const std::string& infile, QComboBox* sel[], int size);
void parse_types(std::unordered_map<std::string, char>& filename_to_type, const std::string& infile);
