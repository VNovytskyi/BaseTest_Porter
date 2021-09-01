#include <QCoreApplication>
#include <QDebug>

extern "C" {
    #include "../../../Libs/porter/Source/porter.h"
}

#define DATA_LENGTH 8
porter_t porter1;
porter_t porter2;

uint8_t testSend1[] = {1, 2, 3, 4, 5, 6, 7, 8};
void send1();

void initPorter();
void deinitPorter();

void testSendCallback_Porter1(const uint8_t *data, const uint8_t length);
void testSendCallback_Porter2(const uint8_t *data, const uint8_t length);

void testRecvCallback_Porter1(const uint8_t *data, const uint8_t length);
void testRecvCallback_Porter2(const uint8_t *data, const uint8_t length);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qDebug() << "*** Test begin ***\n";

    initPorter();

    send1();


    while(1) {
        porter_process(&porter1, NULL, 0, clock());
    }

    deinitPorter();

    qDebug() << "\n*** Test end ***";
    return a.exec();
}

void testSendCallback_Porter1(const uint8_t *data, const uint8_t length)
{
    QByteArray arr((char*)data, length);
    qDebug() << "Porter1 send: " << arr.toHex('.');

    porter_process(&porter2, data, length, clock());
}

void testSendCallback_Porter2(const uint8_t *data, const uint8_t length)
{
    QByteArray arr((char*)data, length);
    qDebug() << "Porter2 send: " << arr.toHex('.');

    porter_process(&porter1, data, length, clock());
}

void testRecvCallback_Porter1(const uint8_t *data, const uint8_t length)
{
    QByteArray arr((char*)data, length);
    qDebug() << "Porter1 recv: " << arr.toHex('.');
}

void testRecvCallback_Porter2(const uint8_t *data, const uint8_t length)
{
    QByteArray arr((char*)data, length);
    qDebug() << "Porter2 recv: " << arr.toHex('.');
}

void send1()
{
    uint8_t sendResult = porter_send(&porter1, testSend1, 8);
    if (sendResult != PORTER_OK) {
        qDebug() <<  "[ ERROR ] Porter send";
        while(1);
    } else {
        qDebug() <<  "[ OK ] Porter send";
    }
}

void initPorter()
{
    uint8_t initResult = porter_init(&porter1, DATA_LENGTH, testSendCallback_Porter1, testRecvCallback_Porter1, 100);
    if (initResult != PORTER_OK) {
        qDebug() <<  "[ ERROR ] Porter1 init";
        while(1);
    } else {
        qDebug() <<  "[ OK ] Porter1 init";
    }

    uint8_t initResult2 = porter_init(&porter2, DATA_LENGTH, testSendCallback_Porter2, testRecvCallback_Porter2, 100);
    if (initResult2 != PORTER_OK) {
        qDebug() <<  "[ ERROR ] Porter2 init";
        while(1);
    } else {
        qDebug() <<  "[ OK ] Porter2 init";
    }

    porter_set_tx_free_callback(&porter1, [](){
        qDebug() << "Porter1 free tx";
    });

    porter_set_tx_free_callback(&porter2, [](){
        qDebug() << "Porter2 free tx";
    });
}

void deinitPorter()
{
    porter_deinit(&porter1);
    porter_deinit(&porter2);
}
