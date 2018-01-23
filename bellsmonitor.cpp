#include "bellsmonitor.h"

#include <QDebug>
#include <QDate>

#define _green      "#006400"
#define _black      "#000000"
#define _white      "#ffffff"
#define _lightgray  "#D3D3D3"
#define _red        "#FF0000"

#define _textSize   "48"

#define dash        "-- : --"

BellsMonitor::BellsMonitor(QWidget *parent) :
    QWidget(parent), textColor(_black), backgroundColor(_white), \
                     SelectTextColor(_red), SelectBackgroundColor(_lightgray), \
                     textSize(_textSize)
{
    m_pTcpSocket = new QTcpSocket(this);

    m_pTcpSocket->connectToHost("localhost", 8083); // !!!!!!!!!!!!!!!!

    connect(m_pTcpSocket,       SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket,       SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pTcpSocket,       SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotError(QAbstractSocket::SocketError)) );

    connect(&timerWait,         SIGNAL(timeout()), this, SLOT(slotTryReconnect())   );
    connect(&timerCurrentTime,  SIGNAL(timeout()), this, SLOT(slotSetCurrentTime()) );
//    connect(&timerCheckInterval,SIGNAL(timeout()), this, SLOT(slotSelectCurrentLesson())  );
    connect(&timerDefininBeginningAndEnd,SIGNAL(timeout()), this, SLOT(slotTimerDefininBeginningAndEnd())  );


    pLayout = new QVBoxLayout;
    pLayout->setMargin(10);

//create table
    createTables(0);

    this->setStyleSheet(QString("background-color:" \
                                + backgroundColor \
                                + "; color: ") \
                                + textColor \
                                +"; font-size: " \
                                + textSize \
                                + "px; gridline-color: black");
    this->setLayout(pLayout);
    this->showFullScreen();
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

//creating tables
    for (int column = 0; column < 3; column++)
        for (int row = 0; row < numbersOfLessons+1; row++){
            pTable->setItem(row,column,new QTableWidgetItem);
            pTable->item(row,column)->setTextAlignment(Qt::AlignCenter);
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

//    timerCheckInterval.start(200);
//    timerDefininBeginningAndEnd.start(1000);

    int currentTimeInSec = QTime::currentTime().hour() * 3600 \
                           + QTime::currentTime().minute() * 60 \
                           + QTime::currentTime().second();

    slotSelectCurrentLesson(currentTimeInSec);
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

//    for (int i = 0; i < 2; ++i)
//        for (int j = 0; j < numbersOfLessonInChange[i]; ++j){
//            qDebug() << i << j << pDoubleArray[i][j].beginInSec <<pDoubleArray[i][j].endInSec << pDoubleArray[i][j].nextLessonBeginInSec;
//    }

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
    if(isLessonNow == 1){
        firstPartClock = "Урок заканчивается через ";

        secondPartClock = restTime(pDoubleArray[0][numberCurrentLesson].endInSec, currentTimeInSec);

        if(pDoubleArray[0][numberCurrentLesson].endInSec <= currentTimeInSec)
            slotSelectCurrentLesson(currentTimeInSec);
    }
    if(isLessonNow == 0){
        firstPartClock = "Начало <span style='color:red'>" \
                + QString::number(numberNextLesson) \
                + QString("</span> урока через ");
        secondPartClock = restTime(pDoubleArray[0][numberNextLesson].beginInSec , currentTimeInSec);

        if(pDoubleArray[0][numberCurrentLesson].beginInSec <= currentTimeInSec)
            slotSelectCurrentLesson(currentTimeInSec);
    }
//    if( (isLessonNow == -1) || (numberNextLesson == -1))
//        firstPartClock = "\0";

    secondPartClock = QString("<span style='color:red'>" + secondPartClock + "</span>");

    clock.setText(firstPartClock\
                  + secondPartClock \
                  + "<br>" \
                  + QTime::currentTime().toString("hh:mm:ss") \
                  + "   " \
                  + QDate::currentDate().toString("dd-MM-yyyy"));


}
void BellsMonitor::slotSelectCurrentLesson(int currentTimeInSec)
{
    if(pTable != 0)
    {
        isLessonNow = 0;
        for (int i = 0; i < numbersOfLessonInChange[0]; ++i) {
            if(pDoubleArray[0][i].begin == "-- : --")
                continue;

            if( (pDoubleArray[0][i].beginInSec < currentTimeInSec) && (currentTimeInSec < pDoubleArray[0][i].endInSec)){
                pTable->item(i + 1, 0)->setBackgroundColor(QColor(SelectBackgroundColor));
                pTable->item(i + 1, 0)->setTextColor(QColor(SelectTextColor));
                pTable->item(i + 1, 1)->setBackgroundColor(QColor(SelectBackgroundColor));
                pTable->item(i + 1, 1)->setTextColor(QColor(SelectTextColor));
                pTable->item(i + 1, 2)->setBackgroundColor(QColor(SelectBackgroundColor));
                pTable->item(i + 1, 2)->setTextColor(QColor(SelectTextColor));

                isLessonNow         = 1;
                numberCurrentLesson = i;

//                secondPartClock     = restTime( pDoubleArray[0][i].endInSec, currentTimeInSec );
            }
            else{
                pTable->item(i + 1, 0)->setBackgroundColor(QColor(backgroundColor));
                pTable->item(i + 1, 0)->setTextColor(QColor(textColor));
                pTable->item(i + 1, 1)->setBackgroundColor(QColor(backgroundColor));
                pTable->item(i + 1, 1)->setTextColor(QColor(textColor));
                pTable->item(i + 1, 2)->setBackgroundColor(QColor(backgroundColor));
                pTable->item(i + 1, 2)->setTextColor(QColor(textColor));
            }
        }
        if(isLessonNow == 0){
            numberNextLesson = -1;
            for (int i = 0; i < numbersOfLessonInChange[0]; ++i){
                if(pDoubleArray[0][i].begin.startsWith(dash))
                    continue;
                if(currentTimeInSec < pDoubleArray[0][i].beginInSec){
                    numberNextLesson = i;
                    qDebug() << i;
                    break;
                }
            }
        }
    }
}
QString BellsMonitor::restTime(int timeInSec, int currentTime)
{
    int result;
    int M, S;

    result = timeInSec - currentTime;
//    if(result < 0){
//        return "ДО ЗАВТРА";
//    }
//        seconds += 86400;

    M = (result / 60);
    S = (result - (M * 60) );


    return QString::number(M) + " мин. " + QString::number(S) + " сек.";
}
void BellsMonitor::slotTimerDefininBeginningAndEnd()
{

}
void BellsMonitor::setLessonNow()
{
//    if(isLessonNow)
//        firstPartClock = "Урок заканчивается через ";
//    else
//        firstPartClock = "Начало урока через ";


//    secondPartClock= "<span style='color:red'>??</span>";
//    thirdPartClock = " мин.<br>";
}
