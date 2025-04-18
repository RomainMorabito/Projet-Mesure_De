#ifndef MODBUS_H
#define MODBUS_H

#include <QString>
#include <sys/socket.h> // Pour les sockets
#include <netinet/in.h> // Pour sockaddr_in
#include <arpa/inet.h>  // Pour inet_pton et htons
#include <QDebug>
#include <cstdint> // Pour uint16_t

class ModbusCommunicator {
public:
    ModbusCommunicator(const QString& serverIp, int port);
    int readRawModbusResponse(unsigned char* request, int requestSize, unsigned char* responseBuffer, int responseBufferSize);
    ~ModbusCommunicator();

    uint16_t readModbusRegister(unsigned char* request, int requestSize);
    bool isConnected() const;
    void connectToServer();

private:
    void disconnectFromServer();

    QString serverIp;
    int port;
    int sock; // Utilisation d'un descripteur de fichier entier pour le socket
};

#endif // MODBUS_H
