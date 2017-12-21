#include "mainwindow.h"
#include <QApplication>
#include "bass.h"
#include "bassmix.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    BASS_Init(-1,44100,BASS_DEVICE_LATENCY,0,NULL);
    BASS_Start();

    MainWindow w;
    w.show();

    int exitcode = a.exec();
    BASS_Free();

    return exitcode;
}
