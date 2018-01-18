#include "widget.h"
#include <QDebug>

Widget::Widget(QWidget *parent) :
    QWidget(parent)
{
    m_pTcpSocket = new QTcpSocket(this);

    m_pTcpSocket->connectToHost("localhost", 83); // !!!!!!!!!!!!!!!!

    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this,         SLOT(slotError(QAbstractSocket::SocketError))
           );



//    connect(pcmd, SIGNAL(clicked()), SLOT(slotSendToServer()));

//tempate
    data[0].begin = QTime(8,30);
    data[0].end = QTime(9,15);

    data[1].begin = QTime(9,30);
    data[1].end = QTime(10,10);

    data[2].begin = QTime(10,30);
    data[2].end = QTime(11,10);

    data[3].begin = QTime(11,30);
    data[3].end = QTime(12,10);

    data[4].begin = QTime(12,25);
    data[4].end = QTime(13,5);

    data[5].begin = QTime(13,15);
    data[5].end = QTime(13,55);

    data[6].begin = QTime(14,5);
    data[6].end = QTime(14,45);

//create table
    createTables(sizeof(data) / sizeof(lesson) );

    pLayout = new QVBoxLayout;
    pLayout->setMargin(0);
    pLayout->addWidget(pTable);
    this->setLayout(pLayout);
}

Widget::~Widget()
{

}

void Widget::createTables(int numbersOfLessons)
{

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
                pTable->item(row, column)->setText(data[row-1].begin.toString("hh:mm"));
            if(column==2)
                pTable->item(row, column)->setText(data[row-1].end.toString("hh:mm"));

            pTable->item(row,column)->setBackground(Qt::black);
            pTable->item(row,column)->setForeground(Qt::green);
        }
//set numbers for lessons
    for (int i = 1; i < rows; ++i)
        pTable->item(i, 0)->setText(QString::number(i));
//set table's properties
    pTable->verticalHeader()->hide();
    pTable->horizontalHeader()->hide();

    pTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    pTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
//stretch
    for(int i = 0; i < rows; i++)
        pTable->verticalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
//set color for all cells, size for text
    QFont font = pTable->font();
    font.setPixelSize(24);
    pTable->setFont(font);
    for (int column = 0; column < 3; column++)
        for (int row = 0; row < numbersOfLessons+1; row++){
            pTable->item(row,column)->setBackground(Qt::black);
            pTable->item(row,column)->setForeground(Qt::green);
        }
    pTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    pTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    pTable->setEditTriggers(0);
    pTable->setSelectionBehavior(QAbstractItemView::SelectRows);
}
void Widget::slotReadyRead()
{
    QDataStream in(m_pTcpSocket);
    in.setVersion(QDataStream::Qt_5_3);
    for (;;) {
        if (!m_nNextBlockSize) {
            if (m_pTcpSocket->bytesAvailable() < (int)sizeof(quint16)) {
                break;
            }
            in >> m_nNextBlockSize;
        }

        if (m_pTcpSocket->bytesAvailable() < m_nNextBlockSize) {
            break;
        }
        QTime   time;
        QString str;
        in >> time >> str;

        m_nNextBlockSize = 0;
    }
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
}

// ----------------------------------------------------------------------
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

// ------------------------------------------------------------------
void Widget::slotConnected()
{
    qDebug() << "Received the connected() signal";
}
