#pragma once

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <string>
#include <QCheckBox>
#include <QLabel>
#include <unordered_map>
#include "HCAStreamChannel.h"
#include "utils.h"
#include "../HCADecoder/HCADecodeService.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void timerEvent(QTimerEvent *event);
    void updateControls();
    void updateUIPosition();
    void updateIdolActivity();
    void reautomateVolumes();
    void setIdol(int index);
    int numIdolsOfType(char type);
    std::string findIdolsOfType(char type);
    std::string filterCommand(const std::string& command);
    static void CALLBACK dispatchEvent(HSYNC handle, DWORD channel, DWORD data, void *user);
    static void CALLBACK dispatchOneshotEvent(HSYNC handle, DWORD channel, DWORD data, void *user);
    void addSyncEvents();
    void fuzzyAdjust();
    void applyCommand(const std::string& command);
    void applyOneshotCommand(const std::string &command);

public slots:
    void setBGMVol(int value);
    void setIdolVol(int value);
    void setBGM(const QString& qStr);
    void setIdolName(const QString& qStr);
    void randomize();
    void setPosition(int value);
    void play();
    void pause();
    void reset();
    void save();
    void setUnit(bool checked);
    void setSolo(bool checked);
    void set13(bool checked);

private:
    Ui::MainWindow *ui;
    int unitsize;
    double bgmVol;
    double idolVol;
    int updateTimerId;
    HCAStreamChannel* bgm;
    HCAStreamChannel* idols[NUM_IDOLS];
    HCAStreamChannel* idolsoneshot[NUM_IDOLS];
    HSTREAM play_stream, mix_stream, idol_mix_stream, idol_oneshot_stream;
    std::string currSong, currIdols[NUM_IDOLS], langString;
    std::unordered_map<std::string, std::string> readableidol_to_filename, readablesong_to_filename;
    std::unordered_map<std::string, char> idol_to_type;
    std::map<QWORD, std::string> event_list, oneshot_event_list;
    std::deque<HSYNC> sync_list;
    QComboBox* idolsel[NUM_IDOLS];
    QCheckBox* idolactivity[NUM_IDOLS];
    QLabel* idolimg[NUM_IDOLS];
    QPixmap idolpixmap[NUM_IDOLS];
    HCADecodeService dec;
    bool isusotsuki;
    int oldunitsize;
};

#endif // MAINWINDOW_H
