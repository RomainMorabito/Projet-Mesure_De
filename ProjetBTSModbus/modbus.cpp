#include "modbus.h"
#include <QDebug>

ModbusCommunicator::ModbusCommunicator(const QString& serverIp, int port)
    : serverIp(serverIp), port(port), sock(INVALID_SOCKET) {
    initializeWinsock();
    connectToServer();
}

ModbusCommunicator::~ModbusCommunicator() {
    disconnectFromServer();
    WSACleanup();
}

uint16_t ModbusCommunicator::readModbusRegister(unsigned char* request, int requestSize) {
    if (send(sock, (char*)request, requestSize, 0) == SOCKET_ERROR) {
        qDebug() << "Erreur d'envoi de la requête Modbus";
        return 0xFFFF; // Valeur d'erreur
    }
    unsigned char response[256];
    int bytesRead = recv(sock, (char*)response, sizeof(response), 0);
    if (bytesRead == SOCKET_ERROR) {
        qDebug() << "Erreur de réception de la réponse Modbus";
        return 0xFFFF; // Valeur d'erreur
    }
    if (bytesRead >= 9 && !(response[7] & 0x80)) {
        return (response[9] << 8) | response[10];
    } else {
        qDebug() << "Erreur Modbus ou réponse invalide";
        return 0xFFFF; // Valeur d'erreur
    }
}

void ModbusCommunicator::initializeWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        qDebug() << "Erreur d'initialisation de Winsock";
    }
}

void ModbusCommunicator::connectToServer() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        qDebug() << "Erreur de création du socket";
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIp.toStdString().c_str(), &serverAddr.sin_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        qDebug() << "Erreur de connexion";
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
}

void ModbusCommunicator::disconnectFromServer() {
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
}
