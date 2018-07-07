#include <QFileDialog>
#include <QTextStream>
#include <QScreen>
#include <sstream>
#include <fstream>
#include <iostream>
#include <set>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../bass/bass.h"
#include "../bass/bassmix.h"
#include "../bass/bass_fx.h"
#include "HCAStreamChannel.h"
#include "utils.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow {parent},
    ui {new Ui::MainWindow},
    dec {2},
    is_usotsuki {false},
    old_unit_size {5},
    unit_size {5},
    bgm_vol {0.6},
    idol_vol {0.6},
    play_stream {BASS_Mixer_StreamCreate(44100,2,0)},
    mix_stream {BASS_Mixer_StreamCreate(44100,2,BASS_STREAM_DECODE)},
    idol_mix_stream {BASS_Mixer_StreamCreate(44100,2,BASS_STREAM_DECODE)},
    idol_oneshot_stream {BASS_Mixer_StreamCreate(44100,2,BASS_STREAM_DECODE)},
    freeverb_params {1, 0.6f, 0.68f, 0.3f, 1, 0, BASS_BFX_CHANALL}
{
    QString locale = QLocale::system().name();
    locale.truncate(locale.lastIndexOf('_'));
    QFileInfo check_file("QuintetPlayer_ja.qm");
    if(locale == "ja" && check_file.exists())
    {
        language_string = "jp";
    }

    ui->setupUi(this);
    srand((unsigned int)time(NULL));

    BASS_Mixer_StreamAddChannel(play_stream, mix_stream, 0);
    BASS_Mixer_StreamAddChannel(mix_stream, idol_mix_stream, 0);
    BASS_Mixer_StreamAddChannel(mix_stream, idol_oneshot_stream, 0);

    setReverb(true);
    BASS_FXSetParameters(freeverb, &freeverb_params);

    ui->usotsukilabel->setVisible(false);

    idol_selection_box[0] = ui->idolsel0;
    idol_selection_box[1] = ui->idolsel1;
    idol_selection_box[2] = ui->idolsel2;
    idol_selection_box[3] = ui->idolsel3;
    idol_selection_box[4] = ui->idolsel4;
    idol_selection_box[5] = ui->idolsel5;
    idol_selection_box[6] = ui->idolsel6;
    idol_selection_box[7] = ui->idolsel7;
    idol_selection_box[8] = ui->idolsel8;
    idol_selection_box[9] = ui->idolsel9;
    idol_selection_box[10] = ui->idolsel10;
    idol_selection_box[11] = ui->idolsel11;
    idol_selection_box[12] = ui->idolsel12;
    idol_activity[0] = ui->idolactive0;
    idol_activity[1] = ui->idolactive1;
    idol_activity[2] = ui->idolactive2;
    idol_activity[3] = ui->idolactive3;
    idol_activity[4] = ui->idolactive4;
    idol_activity[5] = ui->idolactive5;
    idol_activity[6] = ui->idolactive6;
    idol_activity[7] = ui->idolactive7;
    idol_activity[8] = ui->idolactive8;
    idol_activity[9] = ui->idolactive9;
    idol_activity[10] = ui->idolactive10;
    idol_activity[11] = ui->idolactive11;
    idol_activity[12] = ui->idolactive12;
    idol_image[0] = ui->idolimage0;
    idol_image[1] = ui->idolimage1;
    idol_image[2] = ui->idolimage2;
    idol_image[3] = ui->idolimage3;
    idol_image[4] = ui->idolimage4;
    idol_image[5] = ui->idolimage5;
    idol_image[6] = ui->idolimage6;
    idol_image[7] = ui->idolimage7;
    idol_image[8] = ui->idolimage8;
    idol_image[9] = ui->idolimage9;
    idol_image[10] = ui->idolimage10;
    idol_image[11] = ui->idolimage11;
    idol_image[12] = ui->idolimage12;

    bgm = new HCAStreamChannel(&dec, 0.9f);
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        idols[i] = new HCAStreamChannel(&dec, 0.9f);
        idols_oneshot[i] = new HCAStreamChannel(&dec, 0.9f);
        connect(idol_selection_box[i], SIGNAL(currentIndexChanged(QString)), this, SLOT(setIdolName(QString)));
    }

    connect(ui->BGMSlider, SIGNAL(valueChanged(int)), this, SLOT(setBGMVolume(int)));
    connect(ui->idolSlider, SIGNAL(valueChanged(int)), this, SLOT(setIdolVolume(int)));
    connect(ui->positionSlider, SIGNAL(valueChanged(int)), this, SLOT(setPosition(int)));
    connect(ui->songsel, SIGNAL(currentIndexChanged(QString)), this, SLOT(setBGM(QString)));
    connect(ui->randomizeButton, SIGNAL(released()), this, SLOT(randomizeUnit()));
    connect(ui->playButton, SIGNAL(released()), this, SLOT(play()));
    connect(ui->pauseButton, SIGNAL(released()), this, SLOT(pause()));
    connect(ui->resetButton, SIGNAL(released()), this, SLOT(reset()));
    connect(ui->saveButton, SIGNAL(released()), this, SLOT(save()));
    connect(ui->soloButton, SIGNAL(toggled(bool)), this, SLOT(setSolo(bool)));
    connect(ui->unitButton, SIGNAL(toggled(bool)), this, SLOT(setUnit(bool)));
    connect(ui->unitButton13, SIGNAL(toggled(bool)), this, SLOT(set13(bool)));
    connect(ui->reverbToggle, SIGNAL(toggled(bool)), this, SLOT(setReverb(bool)));
    connect(ui->appealToggle, SIGNAL(toggled(bool)), this, SLOT(setAppeal(bool)));

    QScreen* screen = QApplication::primaryScreen();
    qreal refresh_rate = screen->refreshRate();
    if(refresh_rate <= 0)
    {
        refresh_rate = 60;
    }
    timer_id = startTimer(1/refresh_rate * 1000);

    parse_names(filename_to_readableidol, "res/idollist" + language_string + ".txt", idol_selection_box, NUM_IDOLS);
    parse_types(idol_to_type, "res/idoltypes.txt");
    parse_names(filename_to_readablesong, "res/songlist" + language_string + ".txt", &(ui->songsel), 1);

    loadConfigFile("player.cfg");
}

MainWindow::~MainWindow()
{
    saveConfig("player.cfg");
    delete bgm;
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        delete idols[i];
        delete idols_oneshot[i];
    }
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timer_id)
    {
        updateControls();
    }
}

void MainWindow::updateUIPosition()
{
    QWORD len = BASS_ChannelGetLength(bgm->get_decode_channel(), BASS_POS_BYTE);
    QWORD pos = BASS_ChannelGetPosition(bgm->get_decode_channel(), BASS_POS_BYTE);
    if(!ui->positionSlider->isSliderDown())
    {
        ui->positionSlider->blockSignals(true);
        ui->positionSlider->setValue((double)pos/len*ui->positionSlider->maximum());
        ui->positionSlider->blockSignals(false);
    }
    QWORD p = BASS_ChannelBytes2Seconds(bgm->get_decode_channel(),pos);
    QWORD l = BASS_ChannelBytes2Seconds(bgm->get_decode_channel(),len);
    QString result;
    QTextStream(&result) << QString(language_string == "jp" ? "位置: " : "Position: ") << p/60 << QString(":%1").arg(p%60, 2, 10, QChar('0')) << "/" << l/60 << QString(":%1").arg(l%60, 2, 10, QChar('0'));
    ui->statusBar->showMessage(result);
    if( pos >= len )
    {
        pause();
    }
}

void MainWindow::updateIdolActivity()
{
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        if(idols[i]->get_decode_channel() == 0 && idols_oneshot[i]->get_decode_channel() == 0)
        {
            idol_activity[i]->setDisabled(true);
            idol_activity[i]->setChecked(false);
        }
        else
        {
            idol_activity[i]->setEnabled(true);
            float vol;
            BASS_ChannelGetAttribute(idols[i]->get_decode_channel(), BASS_ATTRIB_VOL, &vol);
            bool active = false;
            if(idols[i]->get_decode_channel() != 0)
            {
                active |= vol > 0;
            }
            if(idols_oneshot[i]->get_decode_channel() != 0)
            {
                QWORD pos = BASS_ChannelGetPosition(idols_oneshot[i]->get_decode_channel(), BASS_POS_BYTE);
                active |= BASS_ChannelIsActive(idols_oneshot[i]->get_decode_channel()) && pos > 0;
            }
            idol_activity[i]->setChecked(active);
        }
    }
}

void MainWindow::updateControls()
{
    updateUIPosition();
    updateIdolActivity();
}

void MainWindow::setBGMVolume(int value)
{
    bgm_vol = value/100.;
    BASS_ChannelSetAttribute(bgm->get_decode_channel(), BASS_ATTRIB_VOL, bgm_vol);
}

void MainWindow::setIdolVolume(int value)
{
    idol_vol = value/100.;
    fuzzyAdjust();
}

void MainWindow::randomizeUnit()
{
    if(idol_selection_box[0]->count() > 1)
    {
        std::set<int> seen_set;
        int n;
        for(int i = 0; i < unit_size; ++i)
        {
            do
            {
                n = rand() % (idol_selection_box[i]->count() - 1) + 1;
            } while(seen_set.find(n) != seen_set.end());
            seen_set.insert(n);
            idol_selection_box[i]->setCurrentIndex(n);
        }
    }
}

void MainWindow::setIdolName(const QString&)
{
    int index = 0;
    auto signal_sender = sender();
    for( ; index < NUM_IDOLS; ++index)
    {
        if(idol_selection_box[index] == signal_sender)
        {
            break;
        }
    }
    current_idols[index] = idol_selection_box[index]->currentData().value<QString>().toLocal8Bit().constData();
    setIdol(index);
}

void MainWindow::setBGM(const QString&)
{
    current_song = ui->songsel->currentData().value<QString>().toLocal8Bit().constData();

    is_usotsuki = current_song == "macpri";
    unit_size = is_usotsuki ? 1 : old_unit_size;
    ui->usotsukilabel->setVisible(is_usotsuki);
    ui->soloButton->setVisible(!is_usotsuki);
    ui->unitButton->setVisible(!is_usotsuki);
    ui->unitButton13->setVisible(!is_usotsuki);

    pause();
    BASS_Mixer_ChannelRemove(bgm->get_decode_channel());
    bgm->unload();

    bgm->load("res/" + current_song + "/bgm.hca");
    BASS_Mixer_StreamAddChannel(mix_stream, bgm->get_decode_channel(), 0);
    BASS_ChannelSetAttribute(bgm->get_decode_channel(), BASS_ATTRIB_VOL, bgm_vol);

    parse_control_file(event_list, "res/" + current_song + "/control" + std::to_string(unit_size) + ".txt");
    parse_control_file(oneshot_event_list, "res/" + current_song + "/oneshot" + std::to_string(unit_size) + ".txt");
    addSyncEvents();

    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        idol_selection_box[i]->blockSignals(true);
        for(int j = 53; idol_selection_box[i]->count() > 53 ; ++j)
        {
            idol_selection_box[i]->removeItem(j);
        }
    }

    parse_names(filename_to_readableidol, "res/" + current_song + "/idolomake" + language_string + ".txt", idol_selection_box, NUM_IDOLS);
    parse_types(idol_to_type, "res/" + current_song + "/idolomaketypes.txt");
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        idol_selection_box[i]->blockSignals(true);
        int index = idol_selection_box[i]->findData(QString::fromLocal8Bit(current_idols[i].c_str()));
        if(index == -1)
        {
            idol_selection_box[i]->setCurrentIndex(0);
            std::string idol_name = idol_selection_box[i]->currentData().value<QString>().toLocal8Bit().constData();
            QString filename = QString::fromLocal8Bit(("res/img/" + idol_name + ".png").c_str());
            idol_pixmap[i] = QPixmap(filename);
            idol_image[i]->setPixmap(idol_pixmap[i]);
            BASS_Mixer_ChannelRemove(idols[i]->get_decode_channel());
            idols[i]->unload();
        }
        else
        {
            idol_selection_box[i]->setCurrentIndex(index);
            setIdol(i);
        }
        idol_selection_box[i]->blockSignals(false);
    }

    BASS_Mixer_ChannelSetPosition(bgm->get_decode_channel(), 0, BASS_POS_BYTE | BASS_POS_MIXER_RESET);
}

void MainWindow::setPosition(int value)
{
    bool was_paused = BASS_ChannelIsActive(play_stream) == BASS_ACTIVE_PAUSED;
    if(!was_paused)
    {
        BASS_ChannelPause(play_stream);
    }
    QWORD len = BASS_ChannelGetLength(bgm->get_decode_channel(), BASS_POS_BYTE);
    QWORD pos = (long double)len / 2 / ui->positionSlider->maximum() * value;
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        BASS_ChannelSetPosition(idols[i]->get_decode_channel(), pos, BASS_POS_BYTE);
    }
    BASS_ChannelSetPosition(bgm->get_decode_channel(), pos * 2, BASS_POS_BYTE);
    if(!was_paused)
    {
        BASS_ChannelPlay(play_stream, FALSE);
    }
    fuzzyAdjust();
}

void MainWindow::play()
{
    // Check if we're at the end
    QWORD len = BASS_ChannelGetLength(bgm->get_decode_channel(), BASS_POS_BYTE);
    QWORD pos = BASS_ChannelGetPosition(bgm->get_decode_channel(), BASS_POS_BYTE);
    if(pos >= len)
    {
        reset();
    }
    // Clear buffer if player was paused
    if(BASS_ChannelIsActive(play_stream) == BASS_ACTIVE_PAUSED)
    {
        BASS_ChannelPlay(play_stream, TRUE);
    }
    BASS_ChannelPlay(play_stream, FALSE);
}

void MainWindow::pause()
{
    BASS_ChannelPause(play_stream);
    BASS_FXReset(freeverb);
}

void MainWindow::reset()
{
    // Set positions and flush buffer
    bool was_paused = BASS_ChannelIsActive(play_stream) == BASS_ACTIVE_PAUSED;
    if(!was_paused)
    {
        BASS_ChannelPause(play_stream);
    }
    for(int i = 0; i < unit_size; ++i)
    {
        BASS_ChannelSetPosition(idols[i]->get_decode_channel(), 0, BASS_POS_BYTE);
    }
    BASS_Mixer_ChannelSetPosition(bgm->get_decode_channel(), 0, BASS_POS_BYTE | BASS_POS_MIXER_RESET);
    if(!was_paused)
    {
        BASS_ChannelPlay(play_stream, FALSE);
    }
    fuzzyAdjust();
}

void MainWindow::save()
{
    QString qFilename = QFileDialog::getSaveFileName(this, "Export to WAV", "", "Wave file (*.wav)");
    std::string filename = qFilename.toLocal8Bit().constData();
    if(filename == "")
    {
        return;
    }
    // Wait for all HCA audio to be decoded
    dec.wait_for_finish();
    // Stream needs to be paused else the output will be garbled
    pause();
    reset();
    export_to_wav(mix_stream, filename);
    reset();
}

void MainWindow::setIdol(int index)
{
    QString filename = QString::fromLocal8Bit(("res/img/" + current_idols[index] + ".png").c_str());
    idol_pixmap[index] = QPixmap(filename);
    idol_image[index]->setPixmap(idol_pixmap[index]);

    DWORD old_channel = idols[index]->get_decode_channel();
    DWORD old_channel2 = idols_oneshot[index]->get_decode_channel();

    HCAStreamChannel&& hcastream = HCAStreamChannel(&dec, 0.9f);
    HCAStreamChannel&& hcastream2 = HCAStreamChannel(&dec, 0.9f);
    QWORD pos = BASS_ChannelGetPosition(bgm->get_decode_channel(), BASS_POS_BYTE);
    hcastream.load("res/" + current_song + "/" + current_idols[index] + ".hca", pos/4);
    hcastream2.load("res/" + current_song + "/oneshot/" + current_idols[index] + ".hca", 0);

    if(index >= unit_size)
    {
        hcastream.destroy_channels();
        hcastream2.destroy_channels();
    }
    else
    {
        bool was_paused = BASS_ChannelIsActive(play_stream) == BASS_ACTIVE_PAUSED;
        if(!was_paused)
        {
            BASS_ChannelPause(play_stream);
        }
        pos = BASS_ChannelGetPosition(bgm->get_decode_channel(), BASS_POS_BYTE);
        BASS_ChannelSetPosition(hcastream.get_decode_channel(), pos / 2, BASS_POS_BYTE);
        if(!was_paused)
        {
            BASS_ChannelPlay(play_stream, FALSE);
        }
    }
    // Cleanup wave and channel data
    BASS_Mixer_ChannelRemove(old_channel);
    BASS_Mixer_ChannelRemove(old_channel2);

    *idols[index] = std::move(hcastream);
    *idols_oneshot[index] = std::move(hcastream2);

    fuzzyAdjust();
    BASS_Mixer_StreamAddChannel(idol_mix_stream, idols[index]->get_decode_channel(), 0);
}

void MainWindow::setUnit(bool checked)
{
    if(unit_size != 5 && checked)
    {
        old_unit_size = unit_size = 5;
        reautomateVolumes();
    }
}

void MainWindow::setSolo(bool checked)
{
    if(unit_size != 1 && checked)
    {
        old_unit_size = unit_size = 1;
        reautomateVolumes();
    }
}

void MainWindow::set13(bool checked)
{
    if(unit_size != 13 && checked)
    {
        std::cout << "aaa" << std::endl;
        old_unit_size = unit_size = 13;
        reautomateVolumes();
    }
}

void MainWindow::reautomateVolumes()
{
    parse_control_file(event_list, "res/" + current_song + "/control" + std::to_string(unit_size) + ".txt");
    parse_control_file(oneshot_event_list, "res/" + current_song + "/oneshot" + std::to_string(unit_size) + ".txt");
    addSyncEvents();
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        // Don't unload, rather just cleanup channels
        BASS_Mixer_ChannelRemove(idols[i]->get_decode_channel());
        BASS_Mixer_ChannelRemove(idols_oneshot[i]->get_decode_channel());
        idols[i]->destroy_channels();
        idols_oneshot[i]->destroy_channels();
        if(i < unit_size)
        {
            idols[i]->make_channels();
            idols_oneshot[i]->make_channels();
            QWORD position = BASS_ChannelGetPosition(bgm->get_decode_channel(), BASS_POS_BYTE);
            BASS_ChannelSetPosition(idols[i]->get_decode_channel(), position / 2, BASS_POS_BYTE);
            BASS_Mixer_StreamAddChannel(idol_mix_stream, idols[i]->get_decode_channel(), 0);
        }
    }
    fuzzyAdjust();
}

std::string MainWindow::findIdolsOfType(char type)
{
    static int iterationOrder[] = {11, 9, 7, 5, 3, 1, 0, 2, 4, 6, 8, 10, 12};
    std::string command = "";
    command.reserve(13);
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        int j = iterationOrder[i];
        if(j < unit_size && idol_to_type[current_idols[j]] & type)
        {
            command += char(j + 48);
        }
    }
    if(command.length() == 0)
    {
        for(int i = 0; i < unit_size; ++i)
        {
            command += char(iterationOrder[(NUM_IDOLS - unit_size)/ 2 + i] + 48);
        }
    }
    return command;
}

std::string MainWindow::filterCommand(const std::string &command)
{
    std::string filtered;
    filtered.reserve(13);
    for(unsigned int i = 0; i < command.length(); ++i)
    {
        int idolnum = command[i] - 48;
        if((idolnum == 'x' - 48) || (idolnum >= 0 && idolnum < NUM_IDOLS && idol_to_type[current_idols[idolnum]] & ALL))
        {
            filtered += command[i];
        }
    }
    return filtered;
}

void MainWindow::applyOneshotCommand(const std::string &command)
{
    static double volTable[] = { 0.75, 0.61, 0.51, 0.469, 0.413, 0.373, 0.345, 0.326, 0.308, 0.29, 0.271, 0.259, 0.246 };
    // Game volume table = { 1, 0.89, 0.71, 0.67, 0.59 };
    static double panTable[NUM_IDOLS][NUM_IDOLS] = {{    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                    {-0.25,  0.25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                    {-0.25,     0,  0.25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                    { -0.4,  -0.2,   0.2,   0.4, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                    { -0.5, -0.25,     0,  0.25,   0.5, 0, 0, 0, 0, 0, 0, 0, 0},
                                                    { -0.5,  -0.3,  -0.1,   0.1,   0.3,   0.5, 0, 0, 0, 0, 0, 0, 0},
                                                    { -0.6,  -0.4,  -0.2,     0,   0.2,   0.4,   0.6, 0, 0, 0, 0, 0, 0},
                                                    { -0.6,  -0.5,  -0.3, -0.15,  0.15,   0.3,   0.5,   0.6, 0, 0, 0, 0, 0},
                                                    { -0.7, -0.55,  -0.4,  -0.2,     0,   0.2,   0.4,  0.55,   0.7, 0, 0, 0, 0},
                                                    { -0.8,  -0.6,  -0.5,  -0.3, -0.15,  0.15,   0.3,   0.5,   0.6,   0.8, 0, 0, 0},
                                                    { -0.9,  -0.7, -0.55,  -0.4,  -0.2,     0,   0.2,   0.4,  0.55,   0.7,   0.9, 0, 0},
                                                    {   -1,  -0.8,  -0.7,  -0.6,  -0.4,  -0.2,   0.2,   0.4,   0.6,   0.7,   0.8,     1, 0},
                                                    {   -1,  -0.9,  -0.8,  -0.6,  -0.4,  -0.2,     0,   0.2,   0.4,   0.6,   0.8,   0.9,    1}};
    QWORD bgmpos = BASS_ChannelGetPosition(bgm->get_decode_channel(), BASS_POS_BYTE) / 4;
    std::istringstream iss(command);
    std::string com;
    QWORD ospos = 0, mappos = 0;
    iss >> ospos;
    iss.get();
    mappos = (bgmpos - ospos) * 2;
    std::getline(iss, com);

    for(int i = 0; i < unit_size; ++i)
    {
        BASS_Mixer_ChannelRemove(idols_oneshot[i]->get_decode_channel());
        BASS_ChannelSetPosition(idols_oneshot[i]->get_decode_channel(), 0, BASS_POS_BYTE);
    }

    std::string filtered = filterCommand(com);
    int numSinging = filtered.length();
    size_t n = std::count(filtered.begin(), filtered.end(), 'x');
    for(int i = 0; i < numSinging; ++i)
    {
        if(filtered[i] != 'x')
        {
            QWORD len = BASS_ChannelGetLength(idols_oneshot[filtered[i] - 48]->get_decode_channel(), BASS_POS_BYTE);
            if(mappos >= 0 && mappos < len)
            {
                BASS_ChannelSetAttribute(idols_oneshot[filtered[i] - 48]->get_decode_channel(), BASS_ATTRIB_VOL, idol_vol * volTable[numSinging - 1 - n] / (is_usotsuki ? 0.75 : 0.95));
                BASS_ChannelSetAttribute(idols_oneshot[filtered[i] - 48]->get_decode_channel(), BASS_ATTRIB_PAN, panTable[numSinging - 1][i]);
                BASS_ChannelSetPosition(idols_oneshot[filtered[i] - 48]->get_decode_channel(), mappos, BASS_POS_BYTE);
                BASS_Mixer_StreamAddChannel(idol_oneshot_stream, idols_oneshot[filtered[i] - 48]->get_decode_channel(), 0);
            }
        }
    }
}

void MainWindow::applyCommand(const std::string &command)
{
    if(command.length() == 0)
    {
        for(int i = 0; i < unit_size; ++i)
        {
            BASS_ChannelSetAttribute(idols[i]->get_decode_channel(), BASS_ATTRIB_VOL, 0);
        }
        return;
    }
    static double volTable[] = { 0.75, 0.61, 0.51, 0.469, 0.413, 0.373, 0.345, 0.326, 0.308, 0.29, 0.271, 0.259, 0.246 };
    // Game volume table = { 1, 0.89, 0.71, 0.67, 0.59 };
    static double panTable[NUM_IDOLS][NUM_IDOLS] = {{    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                    {-0.25,  0.25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                    {-0.25,     0,  0.25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                    { -0.4,  -0.2,   0.2,   0.4, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                    { -0.5, -0.25,     0,  0.25,   0.5, 0, 0, 0, 0, 0, 0, 0, 0},
                                                    { -0.5,  -0.3,  -0.1,   0.1,   0.3,   0.5, 0, 0, 0, 0, 0, 0, 0},
                                                    { -0.6,  -0.4,  -0.2,     0,   0.2,   0.4,   0.6, 0, 0, 0, 0, 0, 0},
                                                    { -0.6,  -0.5,  -0.3, -0.15,  0.15,   0.3,   0.5,   0.6, 0, 0, 0, 0, 0},
                                                    { -0.7, -0.55,  -0.4,  -0.2,     0,   0.2,   0.4,  0.55,   0.7, 0, 0, 0, 0},
                                                    { -0.8,  -0.6,  -0.5,  -0.3, -0.15,  0.15,   0.3,   0.5,   0.6,   0.8, 0, 0, 0},
                                                    { -0.9,  -0.7, -0.55,  -0.4,  -0.2,     0,   0.2,   0.4,  0.55,   0.7,   0.9, 0, 0},
                                                    {   -1,  -0.8,  -0.7,  -0.6,  -0.4,  -0.2,   0.2,   0.4,   0.6,   0.7,   0.8,     1, 0},
                                                    {   -1,  -0.9,  -0.8,  -0.6,  -0.4,  -0.2,     0,   0.2,   0.4,   0.6,   0.8,   0.9,    1}};
    switch(command[0])
    {
    case 'P':
        applyCommand(findIdolsOfType(PRINCESS));
        break;
    case 'F':
        applyCommand(findIdolsOfType(FAIRY));
        break;
    case 'A':
        applyCommand(findIdolsOfType(ANGEL));
        break;
    case 'O':
        applyOneshotCommand(command.substr(1, command.length() - 1));
        break;
    default:
        for(int i = 0; i < unit_size; ++i)
        {
            BASS_ChannelSetAttribute(idols[i]->get_decode_channel(), BASS_ATTRIB_VOL, 0);
        }
        std::string filtered = filterCommand(command);
        int numSinging = filtered.length();
        size_t n = std::count(filtered.begin(), filtered.end(), 'x');
        for(int i = 0; i < numSinging; ++i)
        {
            if(filtered[i] != 'x')
            {
                BASS_ChannelSetAttribute(idols[filtered[i] - 48]->get_decode_channel(), BASS_ATTRIB_VOL, idol_vol * volTable[numSinging - 1 - n]);
                BASS_ChannelSetAttribute(idols[filtered[i] - 48]->get_decode_channel(), BASS_ATTRIB_PAN, panTable[numSinging - 1][i]);
            }
        }
    }
}

void CALLBACK MainWindow::dispatchEvent(HSYNC, DWORD channel, DWORD, void *user)
{
    QWORD pos = BASS_ChannelGetPosition(channel, BASS_POS_BYTE) / 4;
    ((MainWindow*)user)->applyCommand(((MainWindow*)user)->event_list[pos]);
}

void CALLBACK MainWindow::dispatchOneshotEvent(HSYNC, DWORD channel, DWORD, void *user)
{
    QWORD pos = BASS_ChannelGetPosition(channel, BASS_POS_BYTE) / 4;
    ((MainWindow*)user)->applyCommand(((MainWindow*)user)->oneshot_event_list[pos]);
}

void MainWindow::addSyncEvents()
{
    for(auto sync : sync_list)
    {
        BASS_ChannelRemoveSync(bgm->get_decode_channel(), sync);
    }
    sync_list.clear();
    for(auto event : event_list)
    {
        sync_list.push_back(BASS_ChannelSetSync(bgm->get_decode_channel(), BASS_SYNC_POS, event.first * 4, dispatchEvent, this));
    }
    for(auto event : oneshot_event_list)
    {
        sync_list.push_back(BASS_ChannelSetSync(bgm->get_decode_channel(), BASS_SYNC_POS, event.first * 4, dispatchOneshotEvent, this));
    }
}

void MainWindow::fuzzyAdjust()
{
    QWORD pos = BASS_ChannelGetPosition(bgm->get_decode_channel(), BASS_POS_BYTE) / 4;
    if(!event_list.empty())
    {
        auto it = event_list.upper_bound(pos);
        if(it != event_list.begin())
        {
            --it;
        }
        applyCommand(event_list[it->first]);
    }
    if(!oneshot_event_list.empty())
    {
        auto it = oneshot_event_list.upper_bound(pos);
        if(it != oneshot_event_list.begin())
        {
            --it;
        }
        applyCommand(oneshot_event_list[it->first]);
    }
}

void MainWindow::setReverb(bool checked)
{
    BASS_ChannelRemoveFX(mix_stream, freeverb);
    if(checked)
    {
        freeverb = BASS_ChannelSetFX(mix_stream, BASS_FX_BFX_FREEVERB, 0);
        BASS_FXSetParameters(freeverb, &freeverb_params);
    }
}

void MainWindow::setAppeal(bool checked)
{
    BASS_Mixer_ChannelRemove(idol_oneshot_stream);
    if(checked)
    {
        BASS_Mixer_StreamAddChannel(mix_stream, idol_oneshot_stream, 0);
        fuzzyAdjust();
    }
}

void MainWindow::saveConfig(const std::string &filename)
{
    std::ofstream outfile = std::ofstream(filename, std::ios::out | std::ios::trunc);
    outfile << "bgm=" << current_song << std::endl;
    outfile << "reverb=" << (ui->reverbToggle->isChecked() ? 1 : 0) << std::endl;
    outfile << "appeal=" << (ui->appealToggle->isChecked() ? 1 : 0) << std::endl;
    outfile << "unitsize=" << unit_size;
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        outfile << std::endl << "idol" << i << "=" << current_idols[i];
    }
}

bool MainWindow::loadConfig(std::unordered_map<std::string, std::string> &config, const std::string &filename)
{
    std::ifstream infilestream(filename);
    if(!infilestream.good())
    {
        return false;
    }
    std::string line, key, value;
    std::stringstream ss;
    while (std::getline(infilestream, line))
    {
        ss = std::stringstream(line);
        std::getline(ss, key, '=');
        std::getline(ss, value);
        config[key] = value;
    }
    return true;
}

void MainWindow::loadConfigFile(const std::string &filename)
{
    std::unordered_map<std::string, std::string> config;
    if(loadConfig(config, filename))
    {
        current_song = config["bgm"];
        ui->reverbToggle->setChecked(config["reverb"] == "1");
        ui->appealToggle->setChecked(config["appeal"] == "1");
        for(int i = 0; i < NUM_IDOLS; ++i)
        {
            current_idols[i] = config["idol" + std::to_string(i)];
        }
        ui->songsel->setCurrentIndex(ui->songsel->findData(current_song.c_str()));
        if(config["unitsize"] != "")
        {
            old_unit_size = unit_size = std::stoi(config["unitsize"]);
            switch(unit_size)
            {
            case 1:
                ui->soloButton->setChecked(true);
                break;
            case 5:
                ui->unitButton->setChecked(true);
                break;
            case 13:
                ui->unitButton13->setChecked(true);
                break;
            }
            reautomateVolumes();
        }
    }
}
