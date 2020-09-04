#include "bellsmonitor.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    BellsMonitor w;
    w.setWindowIcon(QPixmap(":/bell"));
    w.setWindowTitle("«Школьное расписание звонков — Монитор»");
    w.show();

    return a.exec();
}
