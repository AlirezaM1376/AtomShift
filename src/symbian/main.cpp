#include <QtGui/QApplication>
#include "mainwindow.h"
#include "audiomanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AudioManager::instance().init();
    AudioManager::instance().playMenuMusic();
    MainWindow w;
#if defined(Q_OS_SYMBIAN)
    w.setAttribute(Qt::WA_LockLandscapeOrientation);
    w.showFullScreen();
#elif defined(Q_WS_SIMULATOR)
    w.showFullScreen();
#elif defined(Q_WS_S60)
    w.showMaximized();
#else
    w.show();
#endif

    return a.exec();
}
