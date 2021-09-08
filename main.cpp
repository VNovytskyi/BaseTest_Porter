#include <QCoreApplication>
#include <QDebug>

extern "C" {
    #include "../../../Libs/porter/Source/porter.h"
}

#define USER_DATA_LENGTH 8
porter_t porter;
porter_t porter2;

void init();           // Инициализация Porter`а

//Одиночные тесты
void wrap1();          // Обвертка данных пользователя
void unansweredSend(); // Отправка data без получения ack
void answeredSend();   // Отправка data c получением искусственным ack

//Замкнутые тесты
void sendAndRecv();
void sendAndRecvInfinite();

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qDebug() << "*** Test begin ***\n";

    init();
    //wrap1();
    //unansweredSend();
    //answeredSend();


    for(uint16_t i = 0; i < 195; ++i) {
        sendAndRecv();
    }


    /*
    porter.tx_pack_id = 0;
    sendAndRecvInfinite();
    */

    qDebug() << "\n*** Test end ***";
    return a.exec();
}

void init()
{
    qDebug() << "\n***Init case";
    uint8_t init_result = porter_init(&porter, 200);
    if (init_result == PORTER_ERROR) {
        qDebug() << "[ ERROR ] Init porter";
    } else {
        qDebug() << "[ OK ] Init porter";
    }

    init_result = porter_init(&porter2, 200);
    if (init_result == PORTER_ERROR) {
        qDebug() << "[ ERROR ] Init porter2";
    } else {
        qDebug() << "[ OK ] Init porter2";
    }
}

void wrap1()
{
    qDebug() << "\n***Wrap case";
    uint8_t porter_tx_buff[USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT];
    uint8_t userData[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    porter_send(&porter, porter_tx_buff, userData, USER_DATA_LENGTH);

    qDebug() << "User data:         " << QByteArray((char*)userData, USER_DATA_LENGTH).toHex('.');
    qDebug() << "Wrappered data: " << QByteArray((char*)porter_tx_buff, USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT).toHex('.');
}

void unansweredSend()
{
    qDebug() << "\n***Unanswered send case";
    uint8_t porter_tx_buff[USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT];
    uint8_t userData[] = { 0x56, 0x12, 0x34, 0x24, 0x95, 0x56, 0x37, 0x18 };
    porter_send(&porter, porter_tx_buff, userData, USER_DATA_LENGTH);

    qDebug() << "User data:         " << QByteArray((char*)userData, USER_DATA_LENGTH).toHex('.');
    qDebug() << "Wrappered data: " << QByteArray((char*)porter_tx_buff, USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT).toHex('.') << "\n";

    while(1) {
        porter_frame_t frame = porter_process(&porter, NULL, 0, clock());
        if (frame.length > 0) {
            qDebug() << "Want send: " << QByteArray((char*)frame.data, frame.length).toHex('.');
        }
    }
}

void answeredSend()
{
    qDebug() << "\n***Unanswered send case";
    uint8_t porter_tx_buff[USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT];
    uint8_t userData[] = { 0x56, 0x12, 0x34, 0x24, 0x95, 0x56, 0x37, 0x18 };
    porter_send(&porter, porter_tx_buff, userData, USER_DATA_LENGTH);

    qDebug() << "User data:            " << QByteArray((char*)userData, USER_DATA_LENGTH).toHex('.');
    qDebug() << "Wrappered data: " << QByteArray((char*)porter_tx_buff, USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT).toHex('.') << "\n";

    uint8_t test_ack = PORTER_ACK_PACK_ID;
    //uint8_t test_ack = 65;
    while(1) {
        porter_frame_t frame = porter_process(&porter, &test_ack, 1, clock());
        if (frame.length > 0) {
            qDebug() << "Want send: " << QByteArray((char*)frame.data, frame.length).toHex('.');
        }
    }
}

void sendAndRecv()
{
    qDebug() << "\n***sendAndRecv case";
    uint8_t porter_tx_buff[USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT];
    uint8_t userData[] = { 0x56, 0x12, 0x34, 0x24, 0x95, 0x56, 0x37, 0x18 };
    porter_send(&porter, porter_tx_buff, userData, USER_DATA_LENGTH);

    qDebug() << "User data:            " << QByteArray((char*)userData, USER_DATA_LENGTH).toHex('.');
    qDebug() << "Wrappered data: " << QByteArray((char*)porter_tx_buff, USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT).toHex('.') << "\n";

    uint8_t porter_rx_buff[USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT];
    uint8_t porter_rx_buff_length = 0;

    uint8_t porter2_rx_buff[USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT];
    uint8_t porter2_rx_buff_length = 0;

    porter_frame_t frame, frame2;
    for(uint16_t i = 0; i < 5000; ++i) {
        frame = porter_process(&porter, porter_rx_buff, porter_rx_buff_length, clock());
        if (frame.length > 0) {
            memset(porter_rx_buff, 0, USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT);
            qDebug() << "Want send: " << QByteArray((char*)frame.data, frame.length).toHex('.');
            memcpy(porter2_rx_buff, frame.data, frame.length);
            porter2_rx_buff_length = frame.length;
        }

        frame2 = porter_process(&porter2, porter2_rx_buff, porter2_rx_buff_length, clock());
        if (frame2.length > 0) {
            memset(porter2_rx_buff, 0, USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT);
            qDebug() << "Want send2: " << QByteArray((char*)frame2.data, frame2.length).toHex('.');
            memcpy(porter_rx_buff, frame2.data, frame2.length);
            porter_rx_buff_length = frame2.length;
        }
    }
}


void sendAndRecvInfinite()
{
    qDebug() << "\n***sendAndRecv case";
    uint8_t porter_tx_buff[USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT];
    uint8_t userData[] = { 0x56, 0x12, 0x34, 0x24, 0x95, 0x56, 0x37, 0x18 };
    porter_send(&porter, porter_tx_buff, userData, USER_DATA_LENGTH);

    qDebug() << "User data:            " << QByteArray((char*)userData, USER_DATA_LENGTH).toHex('.');
    qDebug() << "Wrappered data: " << QByteArray((char*)porter_tx_buff, USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT).toHex('.') << "\n";

    uint8_t porter_rx_buff[USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT];
    uint8_t porter_rx_buff_length = 0;

    uint8_t porter2_rx_buff[USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT];
    uint8_t porter2_rx_buff_length = 0;

    porter_frame_t frame, frame2;
    while(1) {
        frame = porter_process(&porter, porter_rx_buff, porter_rx_buff_length, clock());
        if (frame.length > 0) {
            memset(porter_rx_buff, 0, USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT);
            qDebug() << "Want send: " << QByteArray((char*)frame.data, frame.length).toHex('.');
            memcpy(porter2_rx_buff, frame.data, frame.length);
            porter2_rx_buff_length = frame.length;
        }

        frame2 = porter_process(&porter2, porter2_rx_buff, porter2_rx_buff_length, clock());
        if (frame2.length > 0) {
            memset(porter2_rx_buff, 0, USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT);
            qDebug() << "Want send2: " << QByteArray((char*)frame2.data, frame2.length).toHex('.');
            memcpy(porter_rx_buff, frame2.data, frame2.length);
            porter_rx_buff_length = frame2.length;
        }
    }
}
