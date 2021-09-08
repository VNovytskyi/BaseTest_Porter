#include <QCoreApplication>
#include <QRandomGenerator>
#include <QDebug>

#define USER_DATA_LENGTH 8
extern "C" {
    #include "../../../Libs/porter/Source/porter.h"
}

// Обвертка данных пользователя и проверка структуры пакета
void testcase_wrap(); //Визуальная проверка 1
void testcase_wrap2(); //Визуальная проверка 2
void testcase_wrapAutoCheck(int iterationsCount); //Автоматическая проверка

//Проверка отправки и отправки по timeout
void testcase_sendInSpace(int testTime);

//Отправка пакета Porter`ом и получение им исскуственного подтверждения
void testcase_singleLoop();

//Замыкание двух Porter`ов друг на друга и отправка одним пакета данных, а другим подтверждения
void testcase_singleLoopReal();




int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qDebug() << "(System) Test begin";

    testcase_wrap();
    testcase_wrap2();
    testcase_wrapAutoCheck(1'000'000);
    testcase_sendInSpace(2'000);
    testcase_singleLoop();
    testcase_singleLoopReal();

    qDebug() << "\n(System) Test end";
    return a.exec();
}

void testcase_wrap()
{
    qDebug() << "\n************************************ Wrap case ************************************";

    porter_t porter;
    uint8_t init_result = porter_init(&porter, 200);
    if (init_result == PORTER_ERROR) {
        qDebug() << "[ ERROR ] Init porter";
        exit(-1);
    }

    uint8_t porter_tx_buff[USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT];
    uint8_t userData[USER_DATA_LENGTH] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    porter_send(&porter, porter_tx_buff, userData, USER_DATA_LENGTH);

    qDebug() << "User data:            " << QByteArray((char*)userData, USER_DATA_LENGTH).toHex('.');
    qDebug() << "Wrappered data: " << QByteArray((char*)porter_tx_buff, USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT).toHex('.');

    qDebug() << "Test case result: [ OK ]";
}

void testcase_wrap2()
{
    qDebug() << "\n************************************ Wrap case 2 ************************************";

    porter_t porter;
    uint8_t init_result = porter_init(&porter, 200);
    if (init_result == PORTER_ERROR) {
        qDebug() << "[ ERROR ] Init porter";
        exit(-1);
    }

    uint8_t porter_tx_buff[USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT];
    uint8_t userData[USER_DATA_LENGTH] = { 0x53, 0x12, 0x33, 0x34, 0x55, 0x26, 0x47, 0x08 };
    porter_send(&porter, porter_tx_buff, userData, USER_DATA_LENGTH);

    qDebug() << "User data:            " << QByteArray((char*)userData, USER_DATA_LENGTH).toHex('.');
    qDebug() << "Wrappered data: " << QByteArray((char*)porter_tx_buff, USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT).toHex('.');
    qDebug() << "Test case result: [ OK ]";
}

void testcase_wrapAutoCheck(int iterationsCount)
{
    qDebug() << "\n************************************ Auto check wrap ************************************";

    porter_t porter;


    uint8_t porter_tx_buff[USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT];
    uint8_t userData[USER_DATA_LENGTH];

    for (int i = 0; i < iterationsCount; ++i) {
        int randomUserDataLength = QRandomGenerator::global()->bounded(1, 9);
        for (int j = 0; j < randomUserDataLength; ++j) {
            userData[j] =  QRandomGenerator::global()->bounded(0, 256);
        }

        uint8_t init_result = porter_init(&porter, 200);
        if (init_result == PORTER_ERROR) {
            qDebug() << "[ ERROR ] Init porter";
            exit(-1);
        }
        porter_send(&porter, porter_tx_buff, userData, USER_DATA_LENGTH);

        if (porter_tx_buff[0] != PORTER_DATA_PACK_ID || porter_tx_buff[1] != 0) {
            qDebug() << "[ ERROR ] Porter pack header error";
            exit(-1);
        }

        for (int j = 0; j < randomUserDataLength; ++j) {
            if (porter_tx_buff[j + 2] != userData[j]) {
                qDebug() << "[ ERROR ] User data in Porter package is damaged";
                exit(-1);
            }
        }

    }

    qDebug() << "Iterations: " << iterationsCount;
    qDebug() << "Test case result: [ OK ]";
}

void testcase_sendInSpace(int testTime)
{
    qDebug() << "\n************************************ Send in space case ************************************";

    porter_t porter;
    uint8_t init_result = porter_init(&porter, 100);
    if (init_result == PORTER_ERROR) {
        qDebug() << "[ ERROR ] Init porter";
        exit(-1);
    }

    uint8_t porter_tx_buff[USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT];
    uint8_t userData[USER_DATA_LENGTH];

    int randomUserDataLength = QRandomGenerator::global()->bounded(1, 9);
    for (int j = 0; j < randomUserDataLength; ++j) {
        userData[j] =  QRandomGenerator::global()->bounded(0, 256);
    }

    porter_send(&porter, porter_tx_buff, userData, USER_DATA_LENGTH);
    clock_t getData = 0;
    clock_t testStartTime = clock();
    while(1) {
        porter_frame_t frame = porter_process(&porter, NULL, 0, clock());
        if (frame.data != NULL && frame.length != 0) {
            getData = clock();
        }

        if ((clock() - getData) > 150) {
            qDebug() << "[ ERROR ] Porter must give the data to send. Timeout.";
            exit(-1);
        }

        if ((clock() - testStartTime) > testTime) {
            break;
        }
    }

    qDebug() << "Test case result: [ OK ]";
}

void testcase_singleLoop()
{
    qDebug() << "\n************************************ Single loop case ************************************";

    porter_t porter;
    uint8_t init_result = porter_init(&porter, 200);
    if (init_result == PORTER_ERROR) {
        qDebug() << "[ ERROR ] Init porter";
        exit(-1);
    }

    uint8_t porter_tx_buff[USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT];
    uint8_t userData[USER_DATA_LENGTH];
    int randomUserDataLength = QRandomGenerator::global()->bounded(1, 9);
    for (int j = 0; j < randomUserDataLength; ++j) {
        userData[j] =  QRandomGenerator::global()->bounded(0, 256);
    }

    porter_send(&porter, porter_tx_buff, userData, USER_DATA_LENGTH);

    porter_frame_t frame = porter_process(&porter, NULL, 0, clock());
    if (frame.data != NULL && frame.length != 0) {
        if (porter_tx_buff[0] != PORTER_DATA_PACK_ID || porter_tx_buff[1] != 0) {
            qDebug() << "[ ERROR ] Porter pack header error";
            exit(-1);
        }

        for (int j = 0; j < randomUserDataLength; ++j) {
            if (porter_tx_buff[j + 2] != userData[j]) {
                qDebug() << "[ ERROR ] User data in Porter package is damaged";
                exit(-1);
            }
        }

    } else {
        qDebug() << "[ ERROR ] Porter must give the data to send. Timeout.";
        exit(-1);
    }

    uint8_t ack[1] = {PORTER_ACK_PACK_ID};
    frame = porter_process(&porter, ack, 1, clock());
    if (frame.data != NULL || frame.length != 0) {
        qDebug() << "[ ERROR ] Porter get data to send after receive the ack";
    }


    qDebug() << "Test case result: [ OK ]";
}

void testcase_singleLoopReal()
{
    qDebug() << "\n************************************ Real single loop case ************************************";
    //ccsa
    qDebug() << "Test case result: [ OK ]";
}

/*
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

void sync()
{
    qDebug() << "\n***sync case";
    uint8_t porter_tx_buff[USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT];
    uint8_t userData[] = { 0x56, 0x12, 0x34, 0x24, 0x95, 0x56, 0x37, 0x18 };
    porter_send(&porter, porter_tx_buff, userData, USER_DATA_LENGTH);

    qDebug() << "User data:            " << QByteArray((char*)userData, USER_DATA_LENGTH).toHex('.');
    qDebug() << "Wrappered data: " << QByteArray((char*)porter_tx_buff, USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT).toHex('.') << "\n";

    uint8_t porter_rx_buff[USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT];
    uint8_t porter_rx_buff_length = 0;

    uint8_t porter2_rx_buff[USER_DATA_LENGTH + PORTER_SERVICE_BYTES_COUNT];
    uint8_t porter2_rx_buff_length = 0;

    porter.tx_pack_id = 23;
    porter2.rx_pack_id = 54;

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
*/
