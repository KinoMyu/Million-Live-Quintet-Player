#include <QApplication>
#include <QTranslator>
#include "mainwindow.h"
#include "../bass/bass.h"
#include "../bass/bassmix.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;

    BASS_Init(-1,44100,BASS_DEVICE_LATENCY,0,NULL);
    BASS_Start();

    BASS_SetConfig(BASS_CONFIG_BUFFER,100);
    BASS_SetConfig(BASS_CONFIG_MIXER_BUFFER,100);
    BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD,5);

    QString locale = QLocale::system().name();
    locale.truncate(locale.lastIndexOf('_'));
    if(locale == "ja")
    {
        translator.load("QuintetPlayer_ja.qm");
        a.installTranslator(&translator);
    }

    MainWindow w;
    w.show();

    int exitcode = a.exec();
    BASS_Free();

    return exitcode;
}
