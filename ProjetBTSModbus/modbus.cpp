#include "modbus.h"
#include <QDebug>
#include <QAbstractSocket>
#include <QHostAddress>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

ModbusCommunicator::ModbusCommunicator(const QString& serverIp, int port)
    : serverIp(serverIp), port(port), sock(INVALID_SOCKET) {
    connectToServer();
}

ModbusCommunicator::~ModbusCommunicator() {
    disconnectFromServer();
}

uint16_t ModbusCommunicator::readModbusRegister(unsigned char* request, int requestSize) {
    if (send(sock, (char*)request, requestSize, 0) == SOCKET_ERROR) {
        qDebug() << "Erreur d'envoi de la requ�te Modbus:" << errno;
        return 0xFFFF; // Valeur d'erreur
    }
    unsigned char response[256];
    int bytesRead = recv(sock, (char*)response, sizeof(response), 0);
    if (bytesRead == SOCKET_ERROR) {
        qDebug() << "Erreur de r�ception de la r�ponse Modbus:" << errno;
        return 0xFFFF; // Valeur d'erreur
    }
    if (bytesRead >= 9 && !(response[7] & 0x80)) {
        return (response[9] << 8) | response[10];
    } else {
        qDebug() << "Erreur Modbus ou r�ponse invalide";
        return 0xFFFF; // Valeur d'erreur
    }
}

uint16_t ModbusCommunicator::readModbusRegisters(unsigned char* request, int requestSize, uint16_t* responseTrame, int responseTrameSize) {
    if (send(sock, (char*)request, requestSize, 0) == SOCKET_ERROR) {
        qDebug() << "Erreur d'envoi de la requ�te Modbus";
        return -1; // Valeur d'erreur
    }
    unsigned char response[256];
    int bytesRead = recv(sock, (char*)response, sizeof(response), 0);
    if (bytesRead == SOCKET_ERROR) {
        qDebug() << "Erreur de r�ception de la r�ponse Modbus";
        return -1; // Valeur d'erreur
    }

    // Afficher la r�ponse Modbus compl�te
    qDebug() << "R�ponse Modbus (bytes lus:" << bytesRead << "):" << QByteArray((char*)response, bytesRead).toHex();

    if (bytesRead >= 9 && !(response[7] & 0x80)) {
        int numRegisters = bytesRead / 2 - 4; // Calculer le nombre de registres re�us
        for (int i = 0; i < numRegisters && i < responseTrameSize; ++i) {
            responseTrame[i] = (response[9 + i * 2] << 8) | response[10 + i * 2];
        }
        return numRegisters;
    } else {
        qDebug() << "Erreur Modbus ou r�ponse invalide";
        return -1; // Valeur d'erreur
    }
}

int ModbusCommunicator::readRawModbusResponse(unsigned char* request, int requestSize, unsigned char* responseBuffer, int responseBufferSize) {
    if (send(sock, (char*)request, requestSize, 0) == SOCKET_ERROR) {
        qDebug() << "Erreur d'envoi de la requ�te Modbus (raw):" << errno;
        return 0;
    }
    int bytesRead = recv(sock, (char*)responseBuffer, responseBufferSize, 0);
    if (bytesRead == SOCKET_ERROR) {
        qDebug() << "Erreur de r�ception de la r�ponse Modbus (raw):" << errno;
        return 0;
    }
    return bytesRead;
}

bool ModbusCommunicator::isConnected() const {
    return sock != INVALID_SOCKET;
}

void ModbusCommunicator::connectToServer() {
    disconnectFromServer();

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        qDebug() << "Erreur de cr�ation du socket:" << errno;
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIp.toStdString().c_str(), &serverAddr.sin_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        qDebug() << "Erreur de connexion:" << errno;
        disconnectFromServer();
    }
}

void ModbusCommunicator::disconnectFromServer() {
    if (sock != INVALID_SOCKET) {
        close(sock); // Utilisez close() sur Linux
        sock = INVALID_SOCKET;
    }
}
