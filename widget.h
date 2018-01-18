#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QTime>
#include <QTcpSocket>

class Widget;


class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

    QVBoxLayout* pLayout;
    QTableWidget* pTable;

private:
    QTcpSocket* m_pTcpSocket;
    quint16     m_nNextBlockSize;

    void createTables(int numbersOfLessons);

    struct lesson {
        QTime begin;
        QTime end;
    };

    lesson data[7];

private slots:
    void slotReadyRead   (                            );
    void slotError       (QAbstractSocket::SocketError);
    void slotSendToServer(                            );
    void slotConnected   (                            );
};

#endif // WIDGET_H
