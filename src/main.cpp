#include "updater.h"
#include <QApplication>
#include <QSharedMemory>
#include <QTranslator>
#include <QLocale>

/*                              Horoneru updater V1.0.1
 *    A very basic and lightweight updater used to update an application's executable
 *
*/
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //Check if another instance has been started already
    QSharedMemory sharedMemory;
    sharedMemory.setKey("Horoneru-updater");

    //If it's already in memory or
    //if it has been called by the user
    if (!sharedMemory.create(1) || argc == 1)
    {
        exit(0); // Exit process immediately
    }

    // Init Trad engine
    QString locale = QLocale::system().name().section('_', 0, 0);
    QTranslator translator;
    //Load correct translation based on system locale
    translator.load("updater_" + locale);
    a.installTranslator(&translator);
    Updater u(argv[1], argv[2] , argv[3], argv[4], argv[5], nullptr);
    u.start();

    return a.exec();
}
