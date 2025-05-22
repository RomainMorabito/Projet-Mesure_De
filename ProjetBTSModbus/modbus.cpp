#include "modbus.h"
#include <QDebug>
#include <QAbstractSocket>
#include <QHostAddress>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define SOCKET_INVALIDE -1
#define ERREUR_SOCKET -1

CommunicateurModbus::CommunicateurModbus(const QString& ipServeur, int port)
    : ipServeur(ipServeur), port(port), socket(SOCKET_INVALIDE) {
    connecterAuServeur();
}

CommunicateurModbus::~CommunicateurModbus() {
    deconnecterDuServeur();
}

uint16_t CommunicateurModbus::lireRegistreModbus(unsigned char* requete, int tailleRequete) {
    if (send(socket, (char*)requete, tailleRequete, 0) == ERREUR_SOCKET) {
        qDebug() << "Erreur d'envoi de la requête Modbus:" << errno;
        return 0xFFFF; // Valeur d'erreur
    }
    unsigned char reponse[256];
    int octetsLus = recv(socket, (char*)reponse, sizeof(reponse), 0);
    if (octetsLus == ERREUR_SOCKET) {
        qDebug() << "Erreur de réception de la réponse Modbus:" << errno;
        return 0xFFFF; // Valeur d'erreur
    }
    if (octetsLus >= 9 && !(reponse[7] & 0x80)) {
        return (reponse[9] << 8) | reponse[10];
    } else {
        qDebug() << "Erreur Modbus ou réponse invalide";
        return 0xFFFF; // Valeur d'erreur
    }
}

uint16_t CommunicateurModbus::lireRegistresModbus(unsigned char* requete, int tailleRequete, uint16_t* trameReponse, int tailleTrameReponse) {
    if (send(socket, (char*)requete, tailleRequete, 0) == ERREUR_SOCKET) {
        qDebug() << "Erreur d'envoi de la requête Modbus";
        return -1; // Valeur d'erreur
    }
    unsigned char reponse[256];
    int octetsLus = recv(socket, (char*)reponse, sizeof(reponse), 0);
    if (octetsLus == ERREUR_SOCKET) {
        qDebug() << "Erreur de réception de la réponse Modbus";
        return -1; // Valeur d'erreur
    }

    // Afficher la réponse Modbus complète
    qDebug() << "Réponse Modbus (octets lus:" << octetsLus << "):" << QByteArray((char*)reponse, octetsLus).toHex();

    if (octetsLus >= 9 && !(reponse[7] & 0x80)) {
        int nombreRegistres = octetsLus / 2 - 4; // Calculer le nombre de registres reçus
        for (int i = 0; i < nombreRegistres && i < tailleTrameReponse; ++i) {
            trameReponse[i] = (reponse[9 + i * 2] << 8) | reponse[10 + i * 2];
        }
        return nombreRegistres;
    } else {
        qDebug() << "Erreur Modbus ou réponse invalide";
        return -1; // Valeur d'erreur
    }
}

int CommunicateurModbus::lireReponseModbusBrute(unsigned char* requete, int tailleRequete, unsigned char* tamponReponse, int tailleTamponReponse) {
    if (send(socket, (char*)requete, tailleRequete, 0) == ERREUR_SOCKET) {
        qDebug() << "Erreur d'envoi de la requête Modbus (brute):" << errno;
        return 0;
    }
    int octetsLus = recv(socket, (char*)tamponReponse, tailleTamponReponse, 0);
    if (octetsLus == ERREUR_SOCKET) {
        qDebug() << "Erreur de réception de la réponse Modbus (brute):" << errno;
        return 0;
    }
    return octetsLus;
}

bool CommunicateurModbus::estConnecte() const {
    return socket != SOCKET_INVALIDE;
}

void CommunicateurModbus::connecterAuServeur() {
    deconnecterDuServeur();

    socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket == SOCKET_INVALIDE) {
        qDebug() << "Erreur de création du socket:" << errno;
        return;
    }

    sockaddr_in adresseServeur;
    adresseServeur.sin_family = AF_INET;
    adresseServeur.sin_port = htons(port);
    inet_pton(AF_INET, ipServeur.toStdString().c_str(), &adresseServeur.sin_addr);

    if (connect(socket, (sockaddr*)&adresseServeur, sizeof(adresseServeur)) == ERREUR_SOCKET) {
        qDebug() << "Erreur de connexion:" << errno;
        deconnecterDuServeur();
    }
}

void CommunicateurModbus::fermerConnexion() {
    deconnecterDuServeur();
}

void CommunicateurModbus::deconnecterDuServeur() {
    if (socket != SOCKET_INVALIDE) {
        close(socket); // Utilisez close() sur Linux
        socket = SOCKET_INVALIDE;
    }
}
