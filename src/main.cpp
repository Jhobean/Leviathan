#include "forms/mainwindow.h"
#include <QApplication>

//#include <QStyle>
//#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //qDebug() << a.style()->name();
    //qDebug() << QStyleFactory::keys().join(",");
    // "windowsvista,Windows,Fusion"
    //a.setStyle("Fusion");

    MainWindow w;
    w.show();

    return a.exec();
}
