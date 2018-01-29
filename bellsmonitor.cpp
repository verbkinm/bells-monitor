#include "bellsmonitor.h"

#include <QDebug>
#include <QDate>
#include <QPixmap>
//#include <QPalette>
#include <QFile>

#define textColor               "#000000"
#define backgroundColor         "#ffffff"
#define SelectBackgroundColor   QColor(211,211,211,215)
#define SelectTextColor         "#FF0000"
#define headerBackgroundColor   QColor(144, 238, 144,200)
#define backgroundColorAlpha    QColor(255,255,255,230)

//#define textSize   "48"

#define dash        "-- : --"

BellsMonitor::BellsMonitor(QWidget *parent) :
    QWidget(parent), settings("LYCEUM", "Bells-monitor")
{
    readSettings();

    m_pTcpSocket = new QTcpSocket(this);

    m_pTcpSocket->connectToHost(server_ip, server_port); // !!!!!!!!!!!!!!!!

    connect(m_pTcpSocket,       SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket,       SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pTcpSocket,       SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotError(QAbstractSocket::SocketError)) );

    connect(&timerWait,         SIGNAL(timeout()), this, SLOT(slotTryReconnect())   );
    connect(&timerCurrentTime,  SIGNAL(timeout()), this, SLOT(slotSetCurrentTime()) );


    pLayout = new QVBoxLayout;
    pLayout->setMargin(10);

//create table
    createTables(0);

    this->setStyleSheet("background-color:" + QString(backgroundColor) + "; \
                                 color: " + textColor +"; \
                                font-size: " + textSize + "px; \
                                gridline-color: black;");
    this->setLayout(pLayout);
    this->showFullScreen();
//    this->showMaximized();

}
void BellsMonitor::createClock()
{
    clock.setText(firstPartClock\
                  + secondPartClock \
                  + "<br>" \
                  + QTime::currentTime().toString("hh:mm:ss") \
                  + "   " \
                  + QDate::currentDate().toString("dd-MM-yyyy"));

    clock.setAlignment(Qt::AlignCenter);
//    clock.setStyleSheet("font-size: 64px;");
    timerCurrentTime.start(200);
    pLayout->addWidget(&clock);
}
BellsMonitor::~BellsMonitor()
{

}
void BellsMonitor::errorServerConnection()
{
    message.setText(tr("Error server connection!"));
    message.setAlignment(Qt::AlignCenter);
    message.setStyleSheet("font-size: 64px;");
    pLayout->addWidget(&message);

    createClock();

    firstPartClock  = "\0";
    secondPartClock = "\0";

    isLessonNow = -1;
}
void BellsMonitor::createTables(int numbersOfLessons)
{
    deleteTable();

    if(numbersOfLessons == 0)
    {
        errorServerConnection();
        return;
    }

    int rows = numbersOfLessons + 1;

    pTable = new QTableWidget(rows,3);
    pTable->setStyleSheet("background-image: url(:/img/lyceum.png); \
                          background-repeat: repeat-xy; \
                          background-position: center; \
                          background-origin: content; background-attachment: scroll;");

//creating tables
    for (int column = 0; column < 3; column++)
        for (int row = 0; row < numbersOfLessons+1; row++){
            pTable->setItem(row,column,new QTableWidgetItem);
            pTable->item(row,column)->setTextAlignment(Qt::AlignCenter);
            pTable->item(row,column)->setBackgroundColor(QColor(backgroundColorAlpha));
        }
//set my headers
    pTable->item(0, 0)->setText("№");
    pTable->item(0, 1)->setText("Начало урока");
    pTable->item(0, 2)->setText("Окончание урока");
//insert text in table
    for (int column = 0; column < 3; column++)
        for (int row = 1; row < numbersOfLessons+1; row++){
            if(column==1)
                pTable->item(row, column)->setText(pDoubleArray[0][row-1].begin);
            if(column==2)
                pTable->item(row, column)->setText(pDoubleArray[0][row-1].end);
        }
//set numbers for lessons
    for (int i = 1; i < rows; ++i)
        pTable->item(i, 0)->setText(QString::number(i-1));
//set table's properties
    pTable->verticalHeader()->hide();
    pTable->horizontalHeader()->hide();

    pTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    pTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
//stretch
    for(int i = 0; i < rows; i++)
        pTable->verticalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);

    pTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    pTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    pTable->setEditTriggers(0);
    pTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    pTable->item(0, 0)->setBackgroundColor(headerBackgroundColor);
    pTable->item(0, 1)->setBackgroundColor(headerBackgroundColor);
    pTable->item(0, 2)->setBackgroundColor(headerBackgroundColor);
    zebra();

//    timerCheckInterval.start(200);
//    timerDefininBeginningAndEnd.start(1000);

    int currentTimeInSec = QTime::currentTime().hour() * 3600 \
                           + QTime::currentTime().minute() * 60 \
                           + QTime::currentTime().second();

    selectCurrentLesson(currentTimeInSec);
}
void BellsMonitor::slotReadyRead()
{
    QDataStream in(m_pTcpSocket);
    in.setVersion(QDataStream::Qt_5_3);

    if(pDoubleArray != 0)
        for (int i = 0; i < 2; ++i)
            delete []pDoubleArray[i];

    pDoubleArray = new lessonTime* [2];

    for (int i = 0; i < 2; ++i) {
        in >> isChangesEnabled[i] \
           >> numbersOfLessonInChange[i];

        pDoubleArray[i] = new lessonTime[numbersOfLessonInChange[i]];

        for (int j = 0; j < numbersOfLessonInChange[i]; ++j){
            in >> pDoubleArray[i][j].begin >> pDoubleArray[i][j].end;

            QString begin   =  pDoubleArray[i][j].begin;
            QString end     =  pDoubleArray[i][j].end;

//установка переменной nextLessonBeginInSec
            if( !(begin.startsWith(dash)) ) {
                int hourBegin   = begin.split(":")[0].toInt();
                int minutBegin  = begin.split(":")[1].toInt();

                int hourEnd     = end.split(":")[0].toInt();
                int minutEnd    = end.split(":")[1].toInt();

                pDoubleArray[i][j].beginInSec = hourBegin * 3600 + minutBegin * 60;
                pDoubleArray[i][j].endInSec = hourEnd * 3600 + minutEnd * 60;

                if( j > 0){
                        for (int ii = j-1; ii >= 0; --ii) {
                            if(pDoubleArray[i][ii].begin.startsWith(dash))
                                continue;
                            if(pDoubleArray[i][ii].nextLessonBeginInSec == -1){
                                pDoubleArray[i][ii].nextLessonBeginInSec = pDoubleArray[i][j].beginInSec;
                                break;
                            }
                        }
                }
            }
        }
    }
// nextLessonBeginInSec для последнего активного урока - это beginInSec первого активного урока
    for (int i = 0; i < 2; ++i){
        for (int j = numbersOfLessonInChange[i]-1; j > 0; --j){
            if(pDoubleArray[i][j].begin.startsWith(dash))
                continue;
            if(pDoubleArray[i][j].nextLessonBeginInSec == -1){
                for (int ii = 0; ii < numbersOfLessonInChange[i]; ++ii) {
                    if(pDoubleArray[i][ii].begin.startsWith(dash))
                        continue;
                    pDoubleArray[i][j].nextLessonBeginInSec = pDoubleArray[i][ii].beginInSec;
                    break;
                }
            }
            break;
        }
    }

    clear();
    createTables(numbersOfLessonInChange[0]);
    pLayout->addWidget(pTable);
    createClock();
}
void BellsMonitor::slotError(QAbstractSocket::SocketError err)
{
    QString strError =
        "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                     "The host was not found." :
                     err == QAbstractSocket::RemoteHostClosedError ?
                     "The remote host is closed." :
                     err == QAbstractSocket::ConnectionRefusedError ?
                     "The connection was refused." :
                     QString(m_pTcpSocket->errorString())
                    );
    qDebug() << strError;

    clear();
    errorServerConnection();

    m_pTcpSocket->disconnectFromHost();
    timerWait.start(1000);
}
void BellsMonitor::slotSendToServer()
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);
    out << quint16(0) << QTime::currentTime();

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    m_pTcpSocket->write(arrBlock);
}
void BellsMonitor::slotConnected()
{
    timerWait.stop();
    qDebug() << "Received the connected() signal";
}
void BellsMonitor::clear()
{
    deleteTable();
    pLayout->removeWidget(&message);
    pLayout->removeWidget(&clock);
}
void BellsMonitor::deleteTable()
{
    if(pTable != 0){
        for (int column = 0; column < 3; column++){
            for (int row = 0; row < pTable->rowCount(); row++){
                delete pTable->item(row, column);
            }
        }
        delete pTable;
        pTable = 0;
    }

}
void BellsMonitor::slotTryReconnect()
{
    timerWait.stop();
    m_pTcpSocket->connectToHost("localhost", 8083);
}
void BellsMonitor::slotSetCurrentTime()
{
    int currentTimeInSec = QTime::currentTime().hour() * 3600 \
                           + QTime::currentTime().minute() * 60 \
                           + QTime::currentTime().second();
    if(isLessonNow){
        firstPartClock = "Урок заканчивается через ";

        secondPartClock = restTime(pDoubleArray[0][numberCurrentLesson].endInSec, currentTimeInSec);

//        qDebug() << "isLessonNow" << pDoubleArray[0][numberCurrentLesson].endInSec << currentTimeInSec;
        selectCurrentLesson(currentTimeInSec);
    }
    if(!isLessonNow){
        firstPartClock = "Начало <span style='color:red'>" \
                + QString::number(numberNextLesson) \
                + QString("</span> урока через ");
        secondPartClock = restTime(pDoubleArray[0][numberNextLesson].beginInSec , currentTimeInSec);

//        qDebug() << "!isLessonNow" << pDoubleArray[0][numberCurrentLesson].beginInSec << pDoubleArray[0][numberNextLesson].nextLessonBeginInSec << currentTimeInSec;
        selectCurrentLesson(currentTimeInSec);
    }

    secondPartClock = QString("<span style='color:red'>" + secondPartClock + "</span>");

    clock.setText(firstPartClock\
                  + secondPartClock \
                  + "<br>" \
                  + QTime::currentTime().toString("hh:mm:ss") \
                  + "   " \
                  + QDate::currentDate().toString("dd-MM-yyyy"));


}
void BellsMonitor::selectCurrentLesson(int currentTimeInSec)
{
    if(pTable != 0)
    {
        isLessonNow = false;
        numberCurrentLesson = -1;
        for (int i = 0; i < numbersOfLessonInChange[0]; ++i) {
            if(pDoubleArray[0][i].begin == "-- : --")
                continue;

            if( (pDoubleArray[0][i].beginInSec < currentTimeInSec) && (currentTimeInSec < pDoubleArray[0][i].endInSec)){
                pTable->item(i + 1, 0)->setTextColor(QColor(SelectTextColor));
                pTable->item(i + 1, 1)->setTextColor(QColor(SelectTextColor));
                pTable->item(i + 1, 2)->setTextColor(QColor(SelectTextColor));

                isLessonNow         = true;
                numberCurrentLesson = i;
            }
            else{
                pTable->item(i + 1, 0)->setTextColor(QColor(textColor));
                pTable->item(i + 1, 1)->setTextColor(QColor(textColor));
                pTable->item(i + 1, 2)->setTextColor(QColor(textColor));
            }
        }
        if(!isLessonNow){
            numberNextLesson = -1;
            for (int i = 0; i < numbersOfLessonInChange[0]; ++i){
                if(pDoubleArray[0][i].begin.startsWith(dash))
                    continue;
                if(currentTimeInSec < pDoubleArray[0][i].beginInSec){
                    numberNextLesson = i;
                    break;
                }
            }
        }
        if(numberNextLesson == -1){
            for (int i = 0; i < numbersOfLessonInChange[0]; ++i) {
                if(pDoubleArray[0][i].begin.startsWith(dash))
                    continue;
                numberNextLesson = i;
                break;
            }
        }
    }
//    qDebug() << "selectCurrentLesson" << numberCurrentLesson << numberNextLesson << isLessonNow;
}
QString BellsMonitor::restTime(int timeInSec, int currentTime)
{
    int result;
    int M, S;

    result = timeInSec - currentTime;
    if(result < 0)
        result += 86400;

    M = (result / 60);
    S = (result - (M * 60) );

    return QString::number(M) + " мин. " + QString::number(S) + " сек.";
}
void BellsMonitor::zebra()
{
    for (int i = 0; i < numbersOfLessonInChange[0]; i += 2) {
        pTable->item(i + 1, 0)->setBackgroundColor(SelectBackgroundColor);
        pTable->item(i + 1, 1)->setBackgroundColor(SelectBackgroundColor);
        pTable->item(i + 1, 2)->setBackgroundColor(SelectBackgroundColor);
    }
//    qDebug() << settings.fileName();
}
void BellsMonitor::readSettings()
{
    server_ip       = settings.value("settings/server_ip",   "localhost").toString();
    server_port     = settings.value("settings/server_port", 8083).toInt();
    textSize        = settings.value("settings/textSize",    48).toString();

    QFile fileSettings(settings.fileName());

    if( !(fileSettings.exists()) ){
        settings.setValue("settings/server_ip",   "localhost");
        settings.setValue("settings/server_port",  8083);
        settings.setValue("settings/textSize",     48);
    }
}
