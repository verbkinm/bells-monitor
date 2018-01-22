#ifndef BELLSMONITOR_H
#define BELLSMONITOR_H

#include <QWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QTime>
#include <QTcpSocket>
#include <QLabel>
#include <QTimer>

class BellsMonitor;

class BellsMonitor : public QWidget
{
    Q_OBJECT

public:
    BellsMonitor(QWidget *parent = 0);
    ~BellsMonitor();

    QVBoxLayout*                pLayout;
    QTableWidget*               pTable = 0;

private:
    QTcpSocket*                 m_pTcpSocket;
    quint16                     m_nNextBlockSize;

    QTimer                      timerWait, timerCurrentTime, timerCheckInterval;
    QLabel                      message, clock;

    QString                     firstPartClock = 0;

    QString                     textColor;

    void createTables           (int numbersOfLessons);
    void errorServerConnection  ();
    void clear                  ();
    void deleteTable            ();

    void createClock            ();

//****************************************************************************
// contents protocol
//
// in >> размер данных(quint16)
//    for (int i = 0; i < 2; ++i){
//      >> №смены(int) >> состояние смены(bool) >> кол-во уроков в смене(int)
//      for (int j = 0; j < кол-во уроков в смене[i]; ++j)
//          >> начало урока(QString) >> конец урока(QString)
//
//****************************************************************************

    unsigned short numbersOfLessonInChange[2] = { 0, 0 };  //0 - 1-я смена, 1 - 2-я смена

    bool isChangesEnabled[2] = { false, false };

    struct lessonTime
    {
        bool    isLessonEnabled = false;
        QString begin = 0;
        QString end   = 0;
    };

    lessonTime**                pDoubleArray = 0;

private slots:
    void slotReadyRead          ();
    void slotError              (QAbstractSocket::SocketError);
    void slotSendToServer       ();
    void slotConnected          ();

    void slotTryReconnect       ();

    void slotSetCurrentTime     ();
    void slotCheckInterval      ();
};

#endif // BELLSMONITOR_H
