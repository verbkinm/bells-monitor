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
    QSettings                   settings;

    QTcpSocket*                 m_pTcpSocket;
    quint16                     m_nNextBlockSize;

    QTimer                      timerWait, timerCurrentTime;
    QLabel                      message, clock;

    bool                        isLessonNow = false;

    int                         numberCurrentLesson  = -1;
    int                         numberPreviousLesson = -1;
    int                         numberNextLesson     = -1;

                            //ÐÐ°ÑÐ°Ð»Ð¾ ÑÑÐ¾ÐºÐ° ÑÐµÑÐµÐ·         n             Ð¼Ð¸Ð½.(ÑÐµÐº.)      Ð´Ð°ÑÐ° Ð¸ Ð²ÑÐµÐ¼Ñ
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
// in >> ÑÐ°Ð·Ð¼ÐµÑ Ð´Ð°Ð½Ð½ÑÑ(quint16)
//    for (int i = 0; i < 2; ++i){
//      >> âÑÐ¼ÐµÐ½Ñ(int) >> ÑÐ¾ÑÑÐ¾ÑÐ½Ð¸Ðµ ÑÐ¼ÐµÐ½Ñ(bool) >> ÐºÐ¾Ð»-Ð²Ð¾ ÑÑÐ¾ÐºÐ¾Ð² Ð² ÑÐ¼ÐµÐ½Ðµ(int)
//      for (int j = 0; j < ÐºÐ¾Ð»-Ð²Ð¾ ÑÑÐ¾ÐºÐ¾Ð² Ð² ÑÐ¼ÐµÐ½Ðµ[i]; ++j)
//          >> Ð½Ð°ÑÐ°Ð»Ð¾ ÑÑÐ¾ÐºÐ°(QString) >> ÐºÐ¾Ð½ÐµÑ ÑÑÐ¾ÐºÐ°(QString)
//
//****************************************************************************

    unsigned short numbersOfLessonInChange[2] = { 0, 0 };  //0 - 1-Ñ ÑÐ¼ÐµÐ½Ð°, 1 - 2-Ñ ÑÐ¼ÐµÐ½Ð°

    bool isChangesEnabled[2] = { false, false };

    struct lessonTime
    {
//        bool    isLessonEnabled         = false;

        QString begin                   = 0;
        QString end                     = 0;
//        QString nextLessonBegin         = 0;

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
