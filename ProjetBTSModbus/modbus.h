#ifndef MODBUS_H
#define MODBUS_H

#include <QString>
#include <sys/socket.h> // Pour les sockets
#include <netinet/in.h> // Pour sockaddr_in
#include <arpa/inet.h>  // Pour inet_pton et htons
#include <QDebug>
#include <cstdint> // Pour uint16_t

class CommunicateurModbus {
public:
    CommunicateurModbus(const QString& ipServeur, int port);
    int lireReponseModbusBrute(unsigned char* requete, int tailleRequete, unsigned char* tamponReponse, int tailleTamponReponse);
    ~CommunicateurModbus();

    uint16_t lireRegistreModbus(unsigned char* requete, int tailleRequete);
    uint16_t lireRegistresModbus(unsigned char* requete, int tailleRequete, uint16_t* trameReponse, int tailleTrameReponse);
    bool estConnecte() const;
    void connecterAuServeur();
    void deconnecterDuServeur();

private:

    QString ipServeur;
    int port;
    int socket; // Utilisation d'un descripteur de fichier entier pour le socket
};

#endif // MODBUS_H
