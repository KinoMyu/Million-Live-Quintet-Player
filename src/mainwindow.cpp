#include <QFileDialog>
#include <QTextStream>
#include <sstream>
#include <set>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../bass/bass.h"
#include "../bass/bassmix.h"
#include "HCAStreamChannel.h"
#include "utils.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    dec(2)
{
    QString locale = QLocale::system().name();
    locale.truncate(locale.lastIndexOf('_'));
    QFileInfo check_file("QuintetPlayer_ja.qm");
    if(locale == "ja" && check_file.exists())
    {
        langString = "jp";
    }
    else
    {
        langString = "";
    }

    ui->setupUi(this);
    srand((unsigned int)time(NULL));

    play_stream = BASS_Mixer_StreamCreate(44100,2,0);
    mix_stream = BASS_Mixer_StreamCreate(44100,2,BASS_STREAM_DECODE);
    idol_mix_stream = BASS_Mixer_StreamCreate(44100,2,BASS_STREAM_DECODE);
    BASS_Mixer_StreamAddChannel(play_stream, mix_stream, 0);
    BASS_Mixer_StreamAddChannel(mix_stream, idol_mix_stream, 0);

    isusotsuki = false;
    ui->usotsukilabel->setVisible(false);
    oldunitsize = unitsize = 5;
    idolsel[0] = ui->idolsel0;
    idolsel[1] = ui->idolsel1;
    idolsel[2] = ui->idolsel2;
    idolsel[3] = ui->idolsel3;
    idolsel[4] = ui->idolsel4;
    idolsel[5] = ui->idolsel5;
    idolsel[6] = ui->idolsel6;
    idolsel[7] = ui->idolsel7;
    idolsel[8] = ui->idolsel8;
    idolsel[9] = ui->idolsel9;
    idolsel[10] = ui->idolsel10;
    idolsel[11] = ui->idolsel11;
    idolsel[12] = ui->idolsel12;
    idolactivity[0] = ui->idolactive0;
    idolactivity[1] = ui->idolactive1;
    idolactivity[2] = ui->idolactive2;
    idolactivity[3] = ui->idolactive3;
    idolactivity[4] = ui->idolactive4;
    idolactivity[5] = ui->idolactive5;
    idolactivity[6] = ui->idolactive6;
    idolactivity[7] = ui->idolactive7;
    idolactivity[8] = ui->idolactive8;
    idolactivity[9] = ui->idolactive9;
    idolactivity[10] = ui->idolactive10;
    idolactivity[11] = ui->idolactive11;
    idolactivity[12] = ui->idolactive12;
    idolimg[0] = ui->idolimage0;
    idolimg[1] = ui->idolimage1;
    idolimg[2] = ui->idolimage2;
    idolimg[3] = ui->idolimage3;
    idolimg[4] = ui->idolimage4;
    idolimg[5] = ui->idolimage5;
    idolimg[6] = ui->idolimage6;
    idolimg[7] = ui->idolimage7;
    idolimg[8] = ui->idolimage8;
    idolimg[9] = ui->idolimage9;
    idolimg[10] = ui->idolimage10;
    idolimg[11] = ui->idolimage11;
    idolimg[12] = ui->idolimage12;

    bgmVol = 0.6;
    idolVol = 0.6;

    bgm = new HCAStreamChannel(&dec, 0.9f);
    currSong = "";
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        idols[i] = new HCAStreamChannel(&dec, 0.9f);
        idolsoneshot[i] = new HCAStreamChannel(&dec, 0.9f);
        currIdols[i] = "";
        idolimg[i]->setScaledContents(true);
        connect(idolsel[i], SIGNAL(currentIndexChanged(QString)), this, SLOT(setIdolName(QString)));
    }

    connect(ui->BGMSlider, SIGNAL(valueChanged(int)), this, SLOT(setBGMVol(int)));
    connect(ui->idolSlider, SIGNAL(valueChanged(int)), this, SLOT(setIdolVol(int)));
    connect(ui->positionSlider, SIGNAL(valueChanged(int)), this, SLOT(setPosition(int)));
    connect(ui->songsel, SIGNAL(currentIndexChanged(QString)), this, SLOT(setBGM(QString)));
    connect(ui->randomizeButton, SIGNAL(released()), this, SLOT(randomize()));
    connect(ui->playButton, SIGNAL(released()), this, SLOT(play()));
    connect(ui->pauseButton, SIGNAL(released()), this, SLOT(pause()));
    connect(ui->resetButton, SIGNAL(released()), this, SLOT(reset()));
    connect(ui->saveButton, SIGNAL(released()), this, SLOT(save()));
    connect(ui->soloButton, SIGNAL(toggled(bool)), this, SLOT(setSolo(bool)));
    connect(ui->unitButton, SIGNAL(toggled(bool)), this, SLOT(setUnit(bool)));
    connect(ui->unitButton13, SIGNAL(toggled(bool)), this, SLOT(set13(bool)));

    parse_names(readableidol_to_filename, "res/idollist" + langString + ".txt", idolsel, NUM_IDOLS);
    parse_types(idol_to_type, "res/idoltypes.txt");
    parse_names(readablesong_to_filename, "res/songlist" + langString + ".txt", &(ui->songsel), 1);

    updateTimerId = startTimer(50);
}

MainWindow::~MainWindow()
{
    delete bgm;
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        delete idols[i];
    }
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == updateTimerId)
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
    QTextStream(&result) << QString(langString == "jp" ? "位置: " : "Position: ") << p/60 << QString(":%1").arg(p%60, 2, 10, QChar('0')) << "/" << l/60 << QString(":%1").arg(l%60, 2, 10, QChar('0'));
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
        if(idols[i]->get_decode_channel() == 0 && idolsoneshot[i]->get_decode_channel() == 0)
        {
            idolactivity[i]->setDisabled(true);
            idolactivity[i]->setChecked(false);
        }
        else
        {
            idolactivity[i]->setEnabled(true);
            float vol;
            BASS_ChannelGetAttribute(idols[i]->get_decode_channel(), BASS_ATTRIB_VOL, &vol);
            bool active = false;
            if(idols[i]->get_decode_channel() != 0)
            {
                active |= vol > 0;
            }
            if(idolsoneshot[i]->get_decode_channel() != 0)
            {
                QWORD pos = BASS_ChannelGetPosition(idolsoneshot[i]->get_decode_channel(), BASS_POS_BYTE);
                active |= BASS_ChannelIsActive(idolsoneshot[i]->get_decode_channel()) && pos > 0;
            }
            idolactivity[i]->setChecked(active);
        }
    }
}

void MainWindow::updateControls()
{
    updateUIPosition();
    updateIdolActivity();
}

void MainWindow::setBGMVol(int value)
{
    bgmVol = value/100.;
    BASS_ChannelSetAttribute(bgm->get_decode_channel(),BASS_ATTRIB_VOL,bgmVol);
}

void MainWindow::setIdolVol(int value)
{
    idolVol = value/100.;
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        fuzzyAdjust();
    }
}

void MainWindow::randomize()
{
    if(idolsel[0]->count() > 1)
    {
        std::set<int> seenset;
        int n;
        for(int i = 0; i < unitsize; ++i)
        {
            do
            {
                n = rand() % (idolsel[i]->count() - 1) + 1;
            } while(seenset.find(n) != seenset.end());
            seenset.insert(n);
            idolsel[i]->setCurrentIndex(n);
        }
    }
}

void MainWindow::setIdolName(const QString& qStr)
{
    int index = 0;
    auto signalsender = sender();
    for( ; index < NUM_IDOLS; ++index)
    {
        if(idolsel[index] == signalsender)
        {
            break;
        }
    }
    currIdols[index] = qStr.toLocal8Bit().constData();
    setIdol(index);
}

void MainWindow::setBGM(const QString& qStr)
{
    currSong = qStr.toLocal8Bit().constData();
    std::string convSongName = readablesong_to_filename[currSong];
    isusotsuki = convSongName == "macpri";
    unitsize = isusotsuki ? 1 : oldunitsize;
    ui->usotsukilabel->setVisible(isusotsuki);
    ui->soloButton->setVisible(!isusotsuki);
    ui->unitButton->setVisible(!isusotsuki);
    ui->unitButton13->setVisible(!isusotsuki);
    BASS_ChannelPause(play_stream);
    BASS_Mixer_ChannelRemove(bgm->get_decode_channel());
    bgm->unload();
    bgm->load("res/" + convSongName + "/bgm.hca");
    BASS_Mixer_StreamAddChannel(mix_stream, bgm->get_decode_channel(), 0);
    BASS_Mixer_ChannelSetPosition(bgm->get_decode_channel(), 0, BASS_POS_BYTE | BASS_POS_MIXER_RESET);
    BASS_ChannelSetAttribute(bgm->get_decode_channel(),BASS_ATTRIB_VOL,bgmVol);
    parse_control_file(event_list, "res/" + convSongName + "/control" + std::to_string(unitsize) + ".txt");
    parse_control_file(oneshot_event_list, "res/" + convSongName + "/oneshot" + std::to_string(unitsize) + ".txt");
    addSyncEvents();
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        idolsel[i]->blockSignals(true);
        for(int j = 53; idolsel[i]->count() > 53 ; ++j)
        {
            idolsel[i]->removeItem(j);
        }
    }
    parse_names(readableidol_to_filename, "res/" + convSongName + "/idolomake" + langString + ".txt", idolsel, NUM_IDOLS);
    parse_types(idol_to_type, "res/" + convSongName + "/idolomaketypes.txt");
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        idolsel[i]->blockSignals(false);
        int index = idolsel[i]->findText(QString::fromLocal8Bit(currIdols[i].c_str()));
        if(index == -1)
        {
            idolsel[i]->setCurrentIndex(0);
            std::string convIdolName = readableidol_to_filename[idolsel[i]->currentText().toLocal8Bit().constData()];
            QString filename = QString::fromLocal8Bit(("res/img/" + convIdolName + ".png").c_str());
            idolpixmap[i] = QPixmap(filename);
            idolimg[i]->setPixmap(idolpixmap[i]);
            BASS_Mixer_ChannelRemove(idols[i]->get_decode_channel());
            idols[i]->unload();
        }
        else
        {
            idolsel[i]->setCurrentIndex(index);
            setIdol(i);
        }
    }
    BASS_ChannelSetPosition(bgm->get_decode_channel(), 0, BASS_POS_BYTE);
}

void MainWindow::setPosition(int value)
{
    QWORD len = BASS_ChannelGetLength(bgm->get_decode_channel(), BASS_POS_BYTE);
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        BASS_ChannelSetPosition(idols[i]->get_decode_channel(), (long double)len / 2 / ui->positionSlider->maximum() * value, BASS_POS_BYTE);
    }
    BASS_ChannelSetPosition(bgm->get_decode_channel(), (long double)len / ui->positionSlider->maximum() * value, BASS_POS_BYTE);
    fuzzyAdjust();
}

void MainWindow::play()
{
    // Check if we're at the end
    QWORD pos = BASS_ChannelGetLength(bgm->get_decode_channel(),BASS_POS_BYTE);
    if(pos == BASS_ChannelGetPosition(bgm->get_decode_channel(),BASS_POS_BYTE))
    {
        BASS_ChannelPause(play_stream);
        BASS_ChannelSetPosition(bgm->get_decode_channel(), 0, BASS_POS_BYTE);
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
}

void MainWindow::reset()
{
    // Check if we're at the end so we don't automatically play after reset
    QWORD pos = BASS_ChannelGetLength(bgm->get_decode_channel(),BASS_POS_BYTE);
    if(pos == BASS_ChannelGetPosition(bgm->get_decode_channel(),BASS_POS_BYTE))
    {
        BASS_ChannelPause(play_stream);
    }
    // Set positions and flush buffer
    BASS_Mixer_ChannelSetPosition(bgm->get_decode_channel(), 0, BASS_POS_BYTE | BASS_POS_MIXER_RESET);
}

void MainWindow::save()
{
    QString qFilename = QFileDialog::getSaveFileName(this, "Export to WAV", "", "Wave file (*.wav)");
    std::string filename = qFilename.toLocal8Bit().constData();
    if(filename == "")
    {
        return;
    }
    reset();
    // Wait for all HCA audio to be decoded
    dec.wait_for_finish();
    // Stream needs to be paused else the output will be garbled
    BASS_ChannelPause(play_stream);
    export_to_wav(bgm->get_decode_channel(), mix_stream, filename);
    reset();
}

void MainWindow::setIdol(int index)
{
    std::string convIdolName = readableidol_to_filename[currIdols[index]];
    std::string convSongName = readablesong_to_filename[currSong];
    QString filename = QString::fromLocal8Bit(("res/img/" + convIdolName + ".png").c_str());
    idolpixmap[index] = QPixmap(filename);
    idolimg[index]->setPixmap(idolpixmap[index]);
    DWORD oldchan = idols[index]->get_decode_channel();
    DWORD oldchan2 = idolsoneshot[index]->get_decode_channel();
    HCAStreamChannel&& hcastream = HCAStreamChannel(&dec, 0.9f);
    HCAStreamChannel&& hcastream2 = HCAStreamChannel(&dec, 0.9f);
    QWORD pos = BASS_ChannelGetPosition(bgm->get_decode_channel(), BASS_POS_BYTE);
    hcastream.load("res/" + convSongName + "/" + convIdolName + ".hca", pos/4);
    hcastream2.load("res/" + convSongName + "/oneshot/" + convIdolName + ".hca", 0);
    if(index >= unitsize)
    {
        hcastream.destroy_channels();
        hcastream2.destroy_channels();
    }
    else
    {
        BASS_ChannelSetPosition(hcastream.get_decode_channel(), pos / 2, BASS_POS_BYTE);
    }
    // Cleanup wave and channel data
    BASS_Mixer_ChannelRemove(oldchan);
    BASS_Mixer_ChannelRemove(oldchan2);
    *idols[index] = std::move(hcastream);
    *idolsoneshot[index] = std::move(hcastream2);
    fuzzyAdjust();
    BASS_Mixer_StreamAddChannel(idol_mix_stream, idols[index]->get_decode_channel(), 0);
}

void MainWindow::setUnit(bool checked)
{
    if(unitsize!=5 && checked)
    {
        oldunitsize = unitsize = 5;
        reautomateVolumes();
    }
}

void MainWindow::setSolo(bool checked)
{
    if(unitsize!=1 && checked)
    {
        oldunitsize = unitsize = 1;
        reautomateVolumes();
    }
}

void MainWindow::set13(bool checked)
{
    if(unitsize!=13 && checked)
    {
        oldunitsize = unitsize = 13;
        reautomateVolumes();
    }
}

void MainWindow::reautomateVolumes()
{
    std::string convSongName = readablesong_to_filename[currSong];
    parse_control_file(event_list, "res/" + convSongName + "/control" + std::to_string(unitsize) + ".txt");
    parse_control_file(oneshot_event_list, "res/" + convSongName + "/oneshot" + std::to_string(unitsize) + ".txt");
    addSyncEvents();
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        // Don't unload, rather just cleanup channels
        BASS_Mixer_ChannelRemove(idols[i]->get_decode_channel());
        BASS_Mixer_ChannelRemove(idolsoneshot[i]->get_decode_channel());
        idols[i]->destroy_channels();
        idolsoneshot[i]->destroy_channels();
        if(i < unitsize)
        {
            idols[i]->make_channels();
            idolsoneshot[i]->make_channels();
            QWORD position = BASS_ChannelGetPosition(bgm->get_decode_channel(), BASS_POS_BYTE);
            BASS_ChannelSetPosition(idols[i]->get_decode_channel(), position / 2, BASS_POS_BYTE);
            BASS_Mixer_StreamAddChannel(idol_mix_stream, idols[i]->get_decode_channel(), 0);
        }
    }
    fuzzyAdjust();
}

int MainWindow::numIdolsOfType(char type)
{
    int count = 0;
    for(int i = 0; i < unitsize; ++i)
    {
        if(idol_to_type[readableidol_to_filename[currIdols[i]]] & type)
        {
            ++count;
        }
    }
    return count;
}

std::string MainWindow::findIdolsOfType(char type)
{
    static int iterationOrder[] = {11, 9, 7, 5, 3, 1, 0, 2, 4, 6, 8, 10, 12};
    std::string command = "";
    command.reserve(13);
    for(int i = 0; i < NUM_IDOLS; ++i)
    {
        int j = iterationOrder[i];
        if(j < unitsize && idol_to_type[readableidol_to_filename[currIdols[j]]] & type)
        {
            command += char(j + 48);
        }
    }
    if(command.length() == 0)
    {
        for(int i = 0; i < unitsize; ++i)
        {
            command += char(iterationOrder[(NUM_IDOLS - unitsize)/ 2 + i] + 48);
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
        if((idolnum == 'x' - 48) || (idolnum >= 0 && idolnum < NUM_IDOLS && idol_to_type[readableidol_to_filename[currIdols[idolnum]]] & ALL))
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

    for(int i = 0; i < unitsize; ++i)
    {
        BASS_Mixer_ChannelRemove(idolsoneshot[i]->get_decode_channel());
    }

    std::string filtered = filterCommand(com);
    int numSinging = filtered.length();
    size_t n = std::count(filtered.begin(), filtered.end(), 'x');
    for(int i = 0; i < numSinging; ++i)
    {
        if(filtered[i] != 'x')
        {
            QWORD len = BASS_ChannelGetLength(idolsoneshot[i]->get_decode_channel(), BASS_POS_BYTE);
            BASS_ChannelSetAttribute(idolsoneshot[filtered[i] - 48]->get_decode_channel(), BASS_ATTRIB_VOL, idolVol * volTable[numSinging - 1 - n] / 0.75);
            BASS_ChannelSetAttribute(idolsoneshot[filtered[i] - 48]->get_decode_channel(), BASS_ATTRIB_PAN, panTable[numSinging - 1][i]);
            BASS_ChannelSetPosition(idolsoneshot[filtered[i] - 48]->get_decode_channel(), mappos >= len || mappos < 0 ? len - 1 : mappos, BASS_POS_BYTE);
            BASS_Mixer_StreamAddChannel(idol_mix_stream, idolsoneshot[filtered[i] - 48]->get_decode_channel(), 0);
        }
    }
}

void MainWindow::applyCommand(const std::string &command)
{
    if(command.length() == 0)
    {
        for(int i = 0; i < unitsize; ++i)
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
        for(int i = 0; i < unitsize; ++i)
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
                BASS_ChannelSetAttribute(idols[filtered[i] - 48]->get_decode_channel(), BASS_ATTRIB_VOL, idolVol * volTable[numSinging - 1 - n]);
                BASS_ChannelSetAttribute(idols[filtered[i] - 48]->get_decode_channel(), BASS_ATTRIB_PAN, panTable[numSinging - 1][i]);
            }
        }
    }
}

void CALLBACK MainWindow::dispatchEvent(HSYNC handle, DWORD channel, DWORD data, void* user)
{
    QWORD pos = BASS_ChannelGetPosition(channel, BASS_POS_BYTE) / 4;
    ((MainWindow*)user)->applyCommand(((MainWindow*)user)->event_list[pos]);
}

void CALLBACK MainWindow::dispatchOneshotEvent(HSYNC handle, DWORD channel, DWORD data, void* user)
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
