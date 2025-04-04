#ifndef MODBUS_H
#define MODBUS_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <QString>
#include <cstdint>

class ModbusCommunicator {
public:
    ModbusCommunicator(const QString& serverIp, int port);
    ~ModbusCommunicator();
    uint16_t readModbusRegister(unsigned char* request, int requestSize);

private:
    QString serverIp;
    int port;
    SOCKET sock;

    void initializeWinsock();
    void connectToServer();
    void disconnectFromServer();
};

#endif // MODBUS_H
