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
    void setIdol(int index);
    void updateUIPosition();
    void updateIdolActivity();
    void reautomateVolumes();

public slots:
    void setBGMVol(int value);
    void setIdolVol(int value);
    void setBGM(const QString& qStr);
    void randomize();
    void setIdol0(const QString& qStr);
    void setIdol1(const QString& qStr);
    void setIdol2(const QString& qStr);
    void setIdol3(const QString& qStr);
    void setIdol4(const QString& qStr);
    void setIdol5(const QString& qStr);
    void setIdol6(const QString& qStr);
    void setIdol7(const QString& qStr);
    void setIdol8(const QString& qStr);
    void setIdol9(const QString& qStr);
    void setIdol10(const QString& qStr);
    void setIdol11(const QString& qStr);
    void setIdol12(const QString& qStr);
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
    ControlInfo idolInfo[NUM_IDOLS];
    HSTREAM mix_stream, idol_mix_stream;
    std::string currSong, currIdols[NUM_IDOLS];
    std::unordered_map<std::string, std::string> readableidol_to_filename, readablesong_to_filename;
    QComboBox* idolsel[NUM_IDOLS];
    QCheckBox* idolactivity[NUM_IDOLS];
    QLabel* idolimg[NUM_IDOLS];
    QPixmap idolpixmap[NUM_IDOLS];
    HCADecodeService dec;
};

#endif // MAINWINDOW_H
