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
#include <QScrollBar>

class BellsMonitor;

class BellsMonitor : public QWidget
{
    Q_OBJECT

public:
    BellsMonitor(QWidget *parent = 0);
    ~BellsMonitor();

    QVBoxLayout*                pLayout;
    QTableWidget                *pTable[2]; //таблицы для смен

private:
    QSettings                   settings;
//Settings value
    QString                     server_ip, textSize;
    int                         textSizeBuffer[2];
    int                         minTextSize;
    int                         server_port;
    bool                        fullScreen;
    int                         tableChangeTimer;
//Settings value

    QTcpSocket*                 m_pTcpSocket;
    quint16                     m_nNextBlockSize;

    QTimer                      timerWait, timerCurrentTime, timerCurrentPeriodDisplay, timerPing;
    QLabel                      message, clock;

    bool                        isConnected = false;
    int                         currentPeriodDisplay = -1;
    bool                        isLessonNow [2] = {false, false};

    int                         numberCurrentLesson[2]  = { -1, -1 };
    int                         numberPreviousLesson[2] = { -1, -1 };
    int                         numberNextLesson[2]     = { -1, -1 };

    QString                     firstPartClock, secondPartClock;

    QString                     textColor, backgroundColor;
    QString                     SelectTextColor, SelectBackgroundColor;

    void clockSetText           (void);
    void createTable            (int numbersOfLessons, short unsigned int numberOfTable);
    void errorServerConnection  (void);
    void clear                  (void);
    void deleteTable            (short unsigned int numberOfTable);

    void createClock            (void);

    void selectCurrentLesson    (int currentTimeInSec);

    QString restTime            (int timeInSec, int currentTime);

    void zebra                  (short unsigned int numberOfTable);

    void readSettings           (void);

//****************************************************************************
// contents protocol
//
// in >> размер данных(quint16) >> тип передаваемых данных ( int  0 - send data, 1 - ping)
//    for (int i = 0; i < 2; ++i){
//      in >> состояние смены(bool) >> кол-во уроков в смене(int)
//      for (int j = 0; j < кол-во уроков в смене[i]; ++j)
//         in >> начало урока(QString) >> конец урока(QString)
//
//****************************************************************************

    unsigned short numbersOfLessonInChange[2] = { 0, 0 };

    bool isChangesEnabled[2] = { false, false };


    struct lessonTime
    {
        QString begin                   = 0;
        QString end                     = 0;

        int beginInSec                  = -1;
        int endInSec                    = -1;
        int nextLessonBeginInSec        = -1;
    };

    lessonTime**                pDoubleArray = 0;

private slots:
    void slotReadyRead                      (void);
    void slotError                          (QAbstractSocket::SocketError);
//    void slotSendToServer                   (void);
    void slotConnected                      ();

    void slotTryReconnect                   (void);
    void slotStateChanged                   (QAbstractSocket::SocketState);
    void slotSetCurrentTime                 (void);
    void slotDisplayPeriod                  (void);
    void slotPingOut                        (void);

};

#endif // BELLSMONITOR_H
