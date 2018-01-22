#include "bellsmonitor.h"

#include <QDebug>
#include <QDate>

#define green "#006400"

BellsMonitor::BellsMonitor(QWidget *parent) :
    QWidget(parent), textColor(green)
{
    m_pTcpSocket = new QTcpSocket(this);

    m_pTcpSocket->connectToHost("localhost", 8083); // !!!!!!!!!!!!!!!!

    connect(m_pTcpSocket,       SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket,       SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pTcpSocket,       SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotError(QAbstractSocket::SocketError)) );

    connect(&timerWait,         SIGNAL(timeout()), this, SLOT(slotTryReconnect())   );
    connect(&timerCurrentTime,  SIGNAL(timeout()), this, SLOT(slotSetCurrentTime()) );
    connect(&timerCheckInterval,SIGNAL(timeout()), this, SLOT(slotCheckInterval())  );

    pLayout = new QVBoxLayout;
    pLayout->setMargin(10);

//create table
    createTables(0);

    this->setStyleSheet(QString("background-color:black; color: ") \
                        + textColor \
                        +"; font-size: 48px; gridline-color: gray");
    this->setLayout(pLayout);
    this->showFullScreen();
}
void BellsMonitor::createClock()
{
    clock.setText(firstPartClock \
                  + QTime::currentTime().toString("hh:mm:ss") \
                  + "   " \
                  + QDate::currentDate().toString("dd-MM-yyyy"));

    clock.setAlignment(Qt::AlignCenter);
    clock.setStyleSheet("font-size: 64px;");
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
    firstPartClock = "\0";
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

    firstPartClock = QString("Начало урока через ") \
            + QString("<span style='color:red'>??</span>") \
            + " мин.<br>";

    timerCheckInterval.start(1000);
}
void BellsMonitor::slotReadyRead()
{
    QDataStream in(m_pTcpSocket);
    in.setVersion(QDataStream::Qt_5_3);

//    quint16 blockSize;
//    in >> blockSize;

    if(pDoubleArray != 0)
        for (int i = 0; i < 2; ++i)
            delete []pDoubleArray[i];

    pDoubleArray                = new lessonTime* [2];

    for (int i = 0; i < 2; ++i) {
        in >> isChangesEnabled[i] \
           >> numbersOfLessonInChange[i];
        pDoubleArray[i] = new lessonTime[numbersOfLessonInChange[i]];
        for (int j = 0; j < numbersOfLessonInChange[i]; ++j)
            in >> pDoubleArray[i][j].begin >> pDoubleArray[i][j].end;
    }

    clear();
    createTables(numbersOfLessonInChange[0]);
    pLayout->addWidget(pTable);
    createClock();

//    for (int i = 0; i < 2; ++i)
//        for (int lessons = 0; lessons < numbersOfLessonInChange[i]; ++lessons)
//            qDebug() << "Смена №" << i << isChangesEnabled[i] << " - " << pDoubleArray[i][lessons].begin << pDoubleArray[i][lessons].end << "урок " << pDoubleArray[i][lessons].isLessonEnabled;

//    in >> numbersLessons;
//    lesson data[(int)numbersLessons];

//    for (int i = 0; i < (int)numbersLessons; ++i)
//        in >> data[i].beginH >> data[i].beginM >> data[i].endH >> data[i].endM;

//    for (int i = 0; i < (int)numbersLessons; ++i)
//        qDebug() << data[i].beginH << data[i].beginM << data[i].endH << data[i].endM;

/*    forever {
        qDebug() << "test4";
        if (!m_nNextBlockSize) {
            if (m_pTcpSocket->bytesAvailable() < (int)sizeof(quint16)) {
                break;
            }
            in >> m_nNextBlockSize;
        }
        qDebug() << "test5";
        if (m_pTcpSocket->bytesAvailable() < m_nNextBlockSize) {
            qDebug() << "test6";
            break;
        }
        qDebug() << "test7";
        quint16 x;
        in >> x;
        qDebug() << "X= " << x;

        m_nNextBlockSize = 0;
    }
    qDebug() << "test2"; */
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
    timerWait.start(10000);
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
    clock.setText(firstPartClock \
                  + QTime::currentTime().toString("hh:mm:ss") \
                  + "   " \
                  + QDate::currentDate().toString("dd-MM-yyyy"));
}
void BellsMonitor::slotCheckInterval()
{
    QTime currentTime(QTime::currentTime());
    QTime begin, end;
    int hourBegin, minuteBegin, hourEnd, minuteEnd;

    if(pTable != 0)
    {
        for (int i = 0; i < numbersOfLessonInChange[0]; ++i) {

            if(pDoubleArray[0][i].begin == "-- : --")
                continue;

            hourBegin   = ((pDoubleArray[0][i].begin).split(":")[0]).toInt();
            minuteBegin = ((pDoubleArray[0][i].begin).split(":")[1]).toInt();
            hourEnd     = ((pDoubleArray[0][i].end).split(":")[0]).toInt();
            minuteEnd   = ((pDoubleArray[0][i].end).split(":")[1]).toInt();

            begin.setHMS(hourBegin, minuteBegin,0);
            end.setHMS  (hourEnd,   minuteEnd,  0);

            if( (begin < currentTime) && (currentTime < end)){
                pTable->item(i + 1, 0)->setBackgroundColor(QColor(Qt::lightGray));
                pTable->item(i + 1, 0)->setTextColor(QColor(Qt::blue));
                pTable->item(i + 1, 1)->setBackgroundColor(QColor(Qt::lightGray));
                pTable->item(i + 1, 1)->setTextColor(QColor(Qt::blue));
                pTable->item(i + 1, 2)->setBackgroundColor(QColor(Qt::lightGray));
                pTable->item(i + 1, 2)->setTextColor(QColor(Qt::blue));
            }
            else{
                pTable->item(i + 1, 0)->setBackgroundColor(QColor(Qt::black));
                pTable->item(i + 1, 0)->setTextColor(QColor(textColor));
                pTable->item(i + 1, 1)->setBackgroundColor(QColor(Qt::black));
                pTable->item(i + 1, 1)->setTextColor(QColor(textColor));
                pTable->item(i + 1, 2)->setBackgroundColor(QColor(Qt::black));
                pTable->item(i + 1, 2)->setTextColor(QColor(textColor));
            }
        }
    }
}
