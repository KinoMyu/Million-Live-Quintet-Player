#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "bass.h"
#include "bassmix.h"
#include "HCAStreamChannel.h"
#include "utils.h"
#include <QFileDialog>
#include <iostream>
#include <sstream>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    BASS_SetConfig(BASS_CONFIG_MIXER_BUFFER,5);
    idolsel[0] = ui->idolsel0;
    idolsel[1] = ui->idolsel1;
    idolsel[2] = ui->idolsel2;
    idolsel[3] = ui->idolsel3;
    idolsel[4] = ui->idolsel4;
    idolactivity[0] = ui->idolactive0;
    idolactivity[1] = ui->idolactive1;
    idolactivity[2] = ui->idolactive2;
    idolactivity[3] = ui->idolactive3;
    idolactivity[4] = ui->idolactive4;
    bgmVol = 0.6;
    idolVol = 0.6;
    solo = false;
    mix_stream = BASS_Mixer_StreamCreate(44100,2,0);
    currSong = "bnthtr";
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        currIdols[i] = "001har";
    }
    ui->idolimage0->setScaledContents(true);
    ui->idolimage1->setScaledContents(true);
    ui->idolimage2->setScaledContents(true);
    ui->idolimage3->setScaledContents(true);
    ui->idolimage4->setScaledContents(true);
    connect(ui->BGMSlider, SIGNAL(valueChanged(int)), this, SLOT(setBGMVol(int)));
    connect(ui->idolSlider, SIGNAL(valueChanged(int)), this, SLOT(setIdolVol(int)));
    connect(ui->positionSlider, SIGNAL(valueChanged(int)), this, SLOT(setPosition(int)));
    connect(ui->songsel, SIGNAL(currentIndexChanged(QString)), this, SLOT(setBGM(QString)));
    connect(ui->idolsel0, SIGNAL(currentIndexChanged(QString)), this, SLOT(setIdol0(QString)));
    connect(ui->idolsel1, SIGNAL(currentIndexChanged(QString)), this, SLOT(setIdol1(QString)));
    connect(ui->idolsel2, SIGNAL(currentIndexChanged(QString)), this, SLOT(setIdol2(QString)));
    connect(ui->idolsel3, SIGNAL(currentIndexChanged(QString)), this, SLOT(setIdol3(QString)));
    connect(ui->idolsel4, SIGNAL(currentIndexChanged(QString)), this, SLOT(setIdol4(QString)));
    connect(ui->playButton, SIGNAL(released()), this, SLOT(play()));
    connect(ui->pauseButton, SIGNAL(released()), this, SLOT(pause()));
    connect(ui->resetButton, SIGNAL(released()), this, SLOT(reset()));
    connect(ui->saveButton, SIGNAL(released()), this, SLOT(save()));
    connect(ui->soloButton, SIGNAL(toggled(bool)), this, SLOT(setSolo(bool)));
    connect(ui->unitButton, SIGNAL(toggled(bool)), this, SLOT(setUnit(bool)));
    parse_names(readablesong_to_filename, "res/songlist.txt", &(ui->songsel), 1);
    parse_names(readableidol_to_filename, "res/idollist.txt", idolsel, NUM_IDOLS);

    updateTimerId = startTimer(50);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == updateTimerId)
    {
        updateControls();
    }
}

void MainWindow::updateControls()
{
    DWORD len = BASS_ChannelGetLength(bgm.get_decode_channel(), BASS_POS_BYTE);
    DWORD pos = BASS_ChannelGetPosition(bgm.get_decode_channel(), BASS_POS_BYTE);
    if(!ui->positionSlider->isSliderDown())
    {
        ui->positionSlider->blockSignals(true);
        ui->positionSlider->setValue((double)pos/len*500);
        ui->positionSlider->blockSignals(false);
    }
    DWORD p = BASS_ChannelBytes2Seconds(bgm.get_decode_channel(),pos);
    DWORD l = BASS_ChannelBytes2Seconds(bgm.get_decode_channel(),len);
    QString result;
    QTextStream(&result) << "Position: " << p/60 << QString(":%1").arg(p%60, 2, 10, QChar('0')) << "/" << l/60 << QString(":%1").arg(l%60, 2, 10, QChar('0'));
    ui->statusBar->showMessage(result);
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        VolumePan vp;
        pos = BASS_ChannelGetPosition(idols[i].get_decode_channel() , BASS_POS_BYTE) / 2;
        DWORD last_pos = 0;
        for (std::pair<DWORD, VolumePan> pos_vp : idolInfo[i].second)
        {
            if (pos >= last_pos && pos <= pos_vp.first)
                break;
            last_pos = pos_vp.first;
            vp = pos_vp.second;
        }
        idolactivity[i]->setChecked(vp.vol > 0);
    }
}

void MainWindow::setBGMVol(int value)
{
    bgmVol = value/100.;
    BASS_ChannelSetAttribute(bgm.get_decode_channel(),BASS_ATTRIB_VOL,bgmVol);
}

void MainWindow::setIdolVol(int value)
{
    idolVol = value/100.;
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        fuzzy_adjust_vol_pan(idols[i].get_decode_channel(), idolInfo[i]);
    }
}

void MainWindow::setIdol0(const QString& qStr)
{
    currIdols[0] = readableidol_to_filename[qStr.toUtf8().constData()];
    QString filename = QString::fromStdString("res/img/" + currIdols[0] + ".png");
    QPixmap pixmap = QPixmap(filename);
    ui->idolimage0->setPixmap(pixmap);
    setIdol(0);
}

void MainWindow::setIdol1(const QString& qStr)
{
    currIdols[1] = readableidol_to_filename[qStr.toUtf8().constData()];
    QString filename = QString::fromStdString("res/img/" + currIdols[1] + ".png");
    QPixmap pixmap = QPixmap(filename);
    ui->idolimage1->setPixmap(pixmap);
    setIdol(1);
}

void MainWindow::setIdol2(const QString& qStr)
{
    currIdols[2] = readableidol_to_filename[qStr.toUtf8().constData()];
    QString filename = QString::fromStdString("res/img/" + currIdols[2] + ".png");
    QPixmap pixmap = QPixmap(filename);
    ui->idolimage2->setPixmap(pixmap);
    setIdol(2);
}

void MainWindow::setIdol3(const QString& qStr)
{
    currIdols[3] = readableidol_to_filename[qStr.toUtf8().constData()];
    QString filename = QString::fromStdString("res/img/" + currIdols[3] + ".png");
    QPixmap pixmap = QPixmap(filename);
    ui->idolimage3->setPixmap(pixmap);
    setIdol(3);
}

void MainWindow::setIdol4(const QString& qStr)
{
    currIdols[4] = readableidol_to_filename[qStr.toUtf8().constData()];
    QString filename = QString::fromStdString("res/img/" + currIdols[4] + ".png");
    QPixmap pixmap = QPixmap(filename);
    ui->idolimage4->setPixmap(pixmap);
    setIdol(4);
}

void MainWindow::setBGM(const QString& qStr)
{
    std::string name = qStr.toUtf8().constData();
    currSong = readablesong_to_filename[name];
    BASS_ChannelPause(mix_stream);
    bgm.unload();
    bgm.load("res/" + currSong + "/bgm.hca");
    BASS_Mixer_StreamAddChannel(mix_stream, bgm.get_decode_channel(), 0);
    BASS_Mixer_ChannelSetPosition(bgm.get_decode_channel(), 0, BASS_POS_BYTE | BASS_POS_MIXER_RESET);
    BASS_ChannelSetAttribute(bgm.get_decode_channel(),BASS_ATTRIB_VOL,bgmVol);
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        idols[i].unload();
        idols[i].load("res/" + currSong + "/" + currIdols[i] + ".hca");
        BASS_Mixer_StreamAddChannel(mix_stream, idols[i].get_decode_channel(), 0);
        idolInfo[i].second.clear();
    }
    parse_control_file(idolInfo, "res/" + currSong + "/control" + (solo?"solo":"") + ".txt", idolVol);
    DWORD idoldecodechannels[NUM_IDOLS];
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        idoldecodechannels[i] = idols[i].get_decode_channel();

    }
    set_auto_vol_pan_all(idolInfo, idoldecodechannels);
}

void MainWindow::setPosition(int value)
{
    DWORD len = BASS_ChannelGetLength(bgm.get_decode_channel(), BASS_POS_BYTE);
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        BASS_ChannelSetPosition(idols[i].get_decode_channel(), len / 1000. * value, BASS_POS_BYTE);
        fuzzy_adjust_vol_pan(idols[i].get_decode_channel(), idolInfo[i]);
    }
    BASS_ChannelSetPosition(bgm.get_decode_channel(), len / 500. * value, BASS_POS_BYTE);
}

void MainWindow::play()
{
    QWORD pos = BASS_ChannelGetLength(bgm.get_decode_channel(),BASS_POS_BYTE);
    if(pos == BASS_ChannelGetPosition(bgm.get_decode_channel(),BASS_POS_BYTE))
    {
        BASS_ChannelPause(mix_stream);
        BASS_ChannelSetPosition(bgm.get_decode_channel(), 0, BASS_POS_BYTE);
        for(int i = 0; i < NUM_IDOLS; ++i)
        {
            BASS_ChannelSetPosition(idols[i].get_decode_channel(), 0, BASS_POS_BYTE);
        }
    }
    BASS_ChannelPlay(mix_stream, FALSE);
}

void MainWindow::pause()
{
    BASS_ChannelPause(mix_stream);
}

void MainWindow::reset()
{
    QWORD pos = BASS_ChannelGetLength(bgm.get_decode_channel(),BASS_POS_BYTE);
    if(pos == BASS_ChannelGetPosition(bgm.get_decode_channel(),BASS_POS_BYTE))
    {
        BASS_ChannelPause(mix_stream);
    }
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        BASS_Mixer_ChannelSetPosition(idols[i].get_decode_channel(), 0, BASS_POS_BYTE);
    }
    BASS_Mixer_ChannelSetPosition(bgm.get_decode_channel(), 0, BASS_POS_BYTE | BASS_POS_MIXER_RESET);
}

void MainWindow::save()
{
    QString qFilename = QFileDialog::getSaveFileName(this, tr("Save file"), "", tr("Wave file (*.wav)"));
    std::string filename = qFilename.toUtf8().constData();
    if(filename == "")
    {
        return;
    }
    DWORD idoldecodechannels[NUM_IDOLS];
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        idoldecodechannels[i] = idols[i].get_decode_channel();
    }
    BASS_ChannelPause(mix_stream);
    export_to_wav(bgm.get_decode_channel(), idoldecodechannels, bgmVol, idolVol, idolInfo, filename);
    reset();
}

void MainWindow::setIdol(int index)
{
    if(idols[index].get_decode_channel() != 0)
    {
        BASS_Mixer_ChannelRemove(idols[index].get_decode_channel());
        idols[index].unload();
    }
    idols[index].load("res/" + currSong + "/" + currIdols[index] + ".hca");
    set_auto_vol_pan(idolInfo[index], idols[index].get_decode_channel());
    DWORD position = BASS_ChannelGetPosition(bgm.get_decode_channel(), BASS_POS_BYTE);
    BASS_ChannelSetPosition(idols[index].get_decode_channel(), position / 2, BASS_POS_BYTE);
    fuzzy_adjust_vol_pan(idols[index].get_decode_channel(), idolInfo[index]);
    BASS_Mixer_StreamAddChannel(mix_stream, idols[index].get_decode_channel(), 0);
}

void MainWindow::setUnit(bool checked)
{
    if(solo && checked)
    {
        solo = false;
        for(int i = 0; i < NUM_IDOLS; ++i)
        {
            idols[i].destroy_channels();
            idols[i].make_channels();
            idolInfo[i].second.clear();
        }
        parse_control_file(idolInfo, "res/" + currSong + "/control.txt", idolVol);
        DWORD idoldecodechannels[NUM_IDOLS];
        for(int i = 0; i < NUM_IDOLS; ++i)
        {
            idoldecodechannels[i] = idols[i].get_decode_channel();
            DWORD position = BASS_ChannelGetPosition(bgm.get_decode_channel(), BASS_POS_BYTE);
            BASS_ChannelSetPosition(idols[i].get_decode_channel(), position / 2, BASS_POS_BYTE);
            BASS_Mixer_StreamAddChannel(mix_stream, idols[i].get_decode_channel(), 0);
            fuzzy_adjust_vol_pan(idoldecodechannels[i], idolInfo[i]);
        }
        set_auto_vol_pan_all(idolInfo, idoldecodechannels);
    }
}

void MainWindow::setSolo(bool checked)
{
    if(!solo && checked)
    {
        solo = true;
        for(int i = 0; i < NUM_IDOLS; ++i)
        {
            idols[i].destroy_channels();
            if(i == 2)
            {
                idols[i].make_channels();
                DWORD position = BASS_ChannelGetPosition(bgm.get_decode_channel(), BASS_POS_BYTE);
                BASS_ChannelSetPosition(idols[i].get_decode_channel(), position / 2, BASS_POS_BYTE);
            }
            BASS_Mixer_StreamAddChannel(mix_stream, idols[i].get_decode_channel(), 0);
            idolInfo[i].second.clear();
        }
        parse_control_file(idolInfo, "res/" + currSong + "/controlsolo.txt", idolVol);
        fuzzy_adjust_vol_pan(idols[2].get_decode_channel(), idolInfo[2]);
        set_auto_vol_pan(idolInfo[2], idols[2].get_decode_channel());
    }
}
