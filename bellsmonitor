#include "widget.h"

#include <QDebug>

Widget::Widget(QWidget *parent) :
    QWidget(parent)
{
    m_pTcpSocket = new QTcpSocket(this);

    m_pTcpSocket->connectToHost("localhost", 8083); // !!!!!!!!!!!!!!!!

    connect(m_pTcpSocket,       SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket,       SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pTcpSocket,       SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotError(QAbstractSocket::SocketError)) );

    connect(&timerWait,         SIGNAL(timeout()), this, SLOT(slotTryReconnect()) );
    connect(&timerCurrentTime,  SIGNAL(timeout()), this, SLOT(slotSetCurrentTime()));

    pLayout = new QVBoxLayout;
    pLayout->setMargin(0);

//create table
    createTables(0);

    this->setStyleSheet("background-color:black; color: green; font-size: 24px; gridline-color: gray");
    this->setLayout(pLayout);
    this->showFullScreen();
}
void Widget::createClock()
{
    clock.setText(QTime::currentTime().toString("hh:mm:ss"));
    clock.setAlignment(Qt::AlignCenter);
    clock.setStyleSheet("font-size: 64px;");
    timerCurrentTime.start(1000);
    pLayout->addWidget(&clock);
}
Widget::~Widget()
{

}
void Widget::errorServerConnection()
{
    message.setText(tr("Error server connection!"));
    message.setAlignment(Qt::AlignCenter);
    message.setStyleSheet("font-size: 44px;");
    pLayout->addWidget(&message);
    createClock();

}
void Widget::createTables(int numbersOfLessons)
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
    pTable->item(0, 0)->setText("№ Урока");
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
}
void Widget::slotReadyRead()
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
void Widget::slotError(QAbstractSocket::SocketError err)
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
void Widget::slotSendToServer()
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);
    out << quint16(0) << QTime::currentTime();

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    m_pTcpSocket->write(arrBlock);
}
void Widget::slotConnected()
{
    timerWait.stop();
    qDebug() << "Received the connected() signal";

}
void Widget::clear()
{
    deleteTable();
    pLayout->removeWidget(&message);
    pLayout->removeWidget(&clock);
}
void Widget::deleteTable()
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
void Widget::slotTryReconnect()
{
    timerWait.stop();
    m_pTcpSocket->connectToHost("localhost", 8083);
}
void Widget::slotSetCurrentTime()
{
    clock.setText(QTime::currentTime().toString("hh:mm:ss"));
}
