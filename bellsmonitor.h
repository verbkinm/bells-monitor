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
#include <QSettings>

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
    QSettings           settings;

    QTcpSocket*                 m_pTcpSocket;
    quint16                     m_nNextBlockSize;

    QTimer                      timerWait, timerCurrentTime;
    QLabel                      message, clock;

    int                         isLessonNow = -1;

    int                         numberCurrentLesson  = -1;
    int                         numberPreviousLesson = -1;
    int                         numberNextLesson     = -1;

                            //Начало урока через         n             мин.(сек.)      дата и время
    QString                     firstPartClock,                     secondPartClock;

    QString                     textColor, backgroundColor;
    QString                     SelectTextColor, SelectBackgroundColor;

//Settings value
    QString                     server_ip, textSize;
    int                         server_port;
//Settings value

    void createTables           (int numbersOfLessons);
    void errorServerConnection  ();
    void clear                  ();
    void deleteTable            ();

    void createClock            ();

    void selectCurrentLesson    (int currentTimeInSec);

    QString restTime            (int timeInSec, int currentTime);

    void zebra                  ();

    void readSettings           ();

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
        bool    isLessonEnabled         = false;

        QString begin                   = 0;
        QString end                     = 0;
        QString nextLessonBegin         = 0;

        int beginInSec                  = -1;
        int endInSec                    = -1;
        int nextLessonBeginInSec        = -1;
    };

    lessonTime**                pDoubleArray = 0;

private slots:
    void slotReadyRead                      ();
    void slotError                          (QAbstractSocket::SocketError);
    void slotSendToServer                   ();
    void slotConnected                      ();

    void slotTryReconnect                   ();

    void slotSetCurrentTime                 ();
};

#endif // BELLSMONITOR_H
