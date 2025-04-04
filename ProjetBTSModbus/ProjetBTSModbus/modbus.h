#ifndef MODBUS_H
#define MODBUS_H

#include <QString>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <QDebug>

class ModbusCommunicator {
public:
    ModbusCommunicator(const QString& serverIp, int port);
    ~ModbusCommunicator();

    uint16_t readModbusRegister(unsigned char* request, int requestSize);
    bool isConnected() const;
    void connectToServer();

private:
    void initializeWinsock();
    void disconnectFromServer();

    QString serverIp;
    int port;
    SOCKET sock;
};

#endif // MODBUS_H
