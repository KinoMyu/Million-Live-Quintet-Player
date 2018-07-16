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
#include "../bass/bass.h"
#include "../bass/bass_fx.h"

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
    std::string findIdolsOfType(char type);
    std::string filterCommand(const std::string &command);
    static void CALLBACK dispatchEvent(HSYNC handle, DWORD channel, DWORD data, void *user);
    static void CALLBACK dispatchOneshotEvent(HSYNC handle, DWORD channel, DWORD data, void *user);
    void addSyncEvents();
    void fuzzyAdjust();
    void applyCommand(const std::string &command);
    void applyOneshotCommand(QWORD pos, const std::string &command);
    void saveConfig(const std::string &filename);
    bool loadConfig(std::unordered_map<std::string, std::string> &config, const std::string &filename);
    void loadConfigFile(const std::string &filename);

public slots:
    void setBGMVolume(int value);
    void setIdolVolume(int value);
    void setBGM(const QString& qStr);
    void setIdolName(const QString& qStr);
    void randomizeUnit();
    void setPosition(int value);
    void play();
    void pause();
    void reset();
    void save();
    void setUnit(bool checked);
    void setSolo(bool checked);
    void set13(bool checked);
    void setReverb(bool checked);
    void setAppeal(bool checked);

private:
    Ui::MainWindow *ui;
    int unit_size,
        old_unit_size;
    double bgm_vol,
           idol_vol;
    int timer_id;
    bool is_usotsuki;
    HCAStreamChannel *bgm,
                     *idols[NUM_IDOLS],
                     *idols_oneshot[NUM_IDOLS];
    HSTREAM play_stream,
            mix_stream,
            idol_mix_stream,
            idol_oneshot_stream;
    HFX freeverb;
    BASS_BFX_FREEVERB freeverb_params;
    std::string current_song,
                current_idols[NUM_IDOLS],
                language_string;
    std::unordered_map<std::string, std::string> filename_to_readableidol,
                                                 filename_to_readablesong;
    std::unordered_map<std::string, char> idol_to_type;
    std::map<QWORD, std::string> event_list,
                                 oneshot_event_list;
    std::deque<HSYNC> sync_list;
    QComboBox *idol_selection_box[NUM_IDOLS];
    QCheckBox *idol_activity[NUM_IDOLS];
    QLabel *idol_image[NUM_IDOLS];
    QPixmap idol_pixmap[NUM_IDOLS];
    HCADecodeService dec;
};

#endif // MAINWINDOW_H
