#include "bellsmonitor.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BellsMonitor w;
//    w.setCursor(QCursor(Qt::BlankCursor));
//    w.show();

    return a.exec();
}
