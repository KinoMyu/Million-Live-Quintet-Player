#pragma once
#include <map>
#include <unordered_map>
#include <QComboBox>

#define NUM_IDOLS 13
#define ALL 0x15
#define PRINCESS 0x10
#define FAIRY 0x4
#define ANGEL 0x1

void export_to_wav(HSTREAM mix_stream, const std::string& filename, const std::map<QWORD, std::string>& event_list, const std::map<QWORD, std::string> &oneshot_event_list);
void parse_control_file(std::map<QWORD, std::string>& event_list, const std::string & control_file);
void parse_names(std::unordered_map<std::string, std::string>& filename_to_readable, const std::string& infile, QComboBox* sel[], int size);
void parse_types(std::unordered_map<std::string, char>& filename_to_type, const std::string& infile);

static const double volTable[] = { 0.75, 0.61, 0.51, 0.469, 0.413, 0.373, 0.345, 0.326, 0.308, 0.29, 0.271, 0.259, 0.246 };
// Game volume table = { 1, 0.89, 0.71, 0.67, 0.59 };
static const double panTable[NUM_IDOLS][NUM_IDOLS] = {{    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0},
                                                      {-0.25,  0.25,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0},
                                                      {-0.25,     0,  0.25,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0},
                                                      { -0.4,  -0.2,   0.2,   0.4,     0,     0,     0,     0,     0,     0,     0,     0,     0},
                                                      { -0.5, -0.25,     0,  0.25,   0.5,     0,     0,     0,     0,     0,     0,     0,     0},
                                                      { -0.5,  -0.3,  -0.1,   0.1,   0.3,   0.5,     0,     0,     0,     0,     0,     0,     0},
                                                      { -0.6,  -0.4,  -0.2,     0,   0.2,   0.4,   0.6,     0,     0,     0,     0,     0,     0},
                                                      { -0.6,  -0.5,  -0.3, -0.15,  0.15,   0.3,   0.5,   0.6,     0,     0,     0,     0,     0},
                                                      { -0.7, -0.55,  -0.4,  -0.2,     0,   0.2,   0.4,  0.55,   0.7,     0,     0,     0,     0},
                                                      { -0.8,  -0.6,  -0.5,  -0.3, -0.15,  0.15,   0.3,   0.5,   0.6,   0.8,     0,     0,     0},
                                                      { -0.9,  -0.7, -0.55,  -0.4,  -0.2,     0,   0.2,   0.4,  0.55,   0.7,   0.9,     0,     0},
                                                      {   -1,  -0.8,  -0.7,  -0.6,  -0.4,  -0.2,   0.2,   0.4,   0.6,   0.7,   0.8,     1,     0},
                                                      {   -1,  -0.9,  -0.8,  -0.6,  -0.4,  -0.2,     0,   0.2,   0.4,   0.6,   0.8,   0.9,     1}};
static const std::unordered_map<int, int> fallback{{11, 3}, { 9, 1}, { 7, 3}, { 5, 1},
                                                   { 3, 1}, { 1, 0}, { 0,-1}, { 2, 0}, { 4, 2},
                                                            { 6, 2}, { 8, 4}, {10, 2}, {12, 4}};
