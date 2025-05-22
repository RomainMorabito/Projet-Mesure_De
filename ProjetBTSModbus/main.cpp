#include <QCoreApplication>
#include <QThread>
#include "modbus.h"
#include "database.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::locale::global(std::locale(""));

    // Connexion à la base de données
    ConnecteurBaseDeDonnees connecteurBaseDeDonnees("192.168.17.10", "Mesure_De", "admin", "admin", 3306);
    float valeurEnergiePrecedenteWh1 = 0.0f;
    float valeurEnergiePrecedenteWh2 = 0.0f;
    float valeurEnergiePrecedenteWh3 = 0.0f;
    float valeurEnergiePrecedenteKWh1 = 0.0f;
    float valeurEnergiePrecedenteKWh2 = 0.0f;
    float valeurEnergiePrecedenteKWh3 = 0.0f;
    bool premiereLecture = true;

    int idDispositif = 1;

    // Récupération de l'adresse IP de la passerelle
    QString adresseIp = connecteurBaseDeDonnees.obtenirAdresseIpPasserelle(idDispositif);

    if (!adresseIp.isEmpty()) {
        qDebug() << "Adresse IP de la passerelle:" << adresseIp;
        while (true) {
            // Initialisation du communicateur Modbus avec l'adresse IP récupérée
            CommunicateurModbus modbus(adresseIp, 502);

            unsigned char requeteModbusEnergieWh1[] = {
                0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0x4D, 0x83, 0x00, 0x01
            };
            unsigned char requeteModbusEnergieWh2[] = {
                0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0x55, 0x83, 0x00, 0x01
            };
            unsigned char requeteModbusEnergieWh3[] = {
                0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0x5D, 0x83, 0x00, 0x01
            };
            unsigned char requeteModbusEnergieKWh1[] = {
                0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0x4D, 0x82, 0x00, 0x01
            };
            unsigned char requeteModbusEnergieKWh2[] = {
                0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0x55, 0x82, 0x00, 0x01
            };
            unsigned char requeteModbusEnergieKWh3[] = {
                0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0x5D, 0x82, 0x00, 0x01
            };
            unsigned char requeteDate[] = {
                0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0xE1, 0x00, 0x00, 0x06
            };

            uint16_t valeurEnergieWh1 = 0xFFFF;
            uint16_t valeurEnergieWh2 = 0xFFFF;
            uint16_t valeurEnergieWh3 = 0xFFFF;
            uint16_t valeurEnergieKWh1 = 0xFFFF;
            uint16_t valeurEnergieKWh2 = 0xFFFF;
            uint16_t valeurEnergieKWh3 = 0xFFFF;

            valeurEnergieWh1 = modbus.lireRegistreModbus(requeteModbusEnergieWh1, sizeof(requeteModbusEnergieWh1));
            valeurEnergieWh2 = modbus.lireRegistreModbus(requeteModbusEnergieWh2, sizeof(requeteModbusEnergieWh2));
            valeurEnergieWh3 = modbus.lireRegistreModbus(requeteModbusEnergieWh3, sizeof(requeteModbusEnergieWh3));
            valeurEnergieKWh1 = modbus.lireRegistreModbus(requeteModbusEnergieKWh1, sizeof(requeteModbusEnergieKWh1));
            valeurEnergieKWh2 = modbus.lireRegistreModbus(requeteModbusEnergieKWh2, sizeof(requeteModbusEnergieKWh2));
            valeurEnergieKWh3 = modbus.lireRegistreModbus(requeteModbusEnergieKWh3, sizeof(requeteModbusEnergieKWh3));

            uint16_t registreDate[6];
            int nombreRegistresLusDate = modbus.lireRegistresModbus(requeteDate, sizeof(requeteDate), registreDate, 6);

            if (nombreRegistresLusDate == 6) {
                uint16_t jour = registreDate[0];
                uint16_t mois = registreDate[1];
                uint16_t annee = registreDate[2];
                uint16_t heure = registreDate[3];
                uint16_t minute = registreDate[4];
                uint16_t seconde = registreDate[5];

                if (valeurEnergieWh1 == 0xFFFF || valeurEnergieWh2 == 0xFFFF || valeurEnergieWh3 == 0xFFFF ||
                    valeurEnergieKWh1 == 0xFFFF || valeurEnergieKWh2 == 0xFFFF || valeurEnergieKWh3 == 0xFFFF ||
                    jour == 0xFFFF || mois == 0xFFFF || annee == 0xFFFF ||
                    heure == 0xFFFF || minute == 0xFFFF || seconde == 0xFFFF) {
                    qDebug() << "Erreur lors de la lecture des registres après plusieurs tentatives";
                } else {
                    float valeurEnergieActuelleWh1 = static_cast<float>(valeurEnergieWh1) / 10.0f;
                    float valeurEnergieActuelleWh2 = static_cast<float>(valeurEnergieWh2) / 10.0f;
                    float valeurEnergieActuelleWh3 = static_cast<float>(valeurEnergieWh3) / 10.0f;
                    float valeurEnergieActuelleKWh1 = static_cast<float>(valeurEnergieKWh1);
                    float valeurEnergieActuelleKWh2 = static_cast<float>(valeurEnergieKWh2);
                    float valeurEnergieActuelleKWh3 = static_cast<float>(valeurEnergieKWh3);

                    float consommationWh1 = valeurEnergieActuelleWh1 - valeurEnergiePrecedenteWh1;
                    float consommationWh2 = valeurEnergieActuelleWh2 - valeurEnergiePrecedenteWh2;
                    float consommationWh3 = valeurEnergieActuelleWh3 - valeurEnergiePrecedenteWh3;

                    float consommationKWh1 = valeurEnergieActuelleKWh1 - valeurEnergiePrecedenteKWh1;
                    float consommationKWh2 = valeurEnergieActuelleKWh2 - valeurEnergiePrecedenteKWh2;
                    float consommationKWh3 = valeurEnergieActuelleKWh3 - valeurEnergiePrecedenteKWh3;

                    float consommationTotaleWh = consommationWh1 + consommationWh2 + consommationWh3;
                    float consommationTotaleKWh = (consommationTotaleWh / 1000.0f) + consommationKWh1 + consommationKWh2 + consommationKWh3;

                    qDebug() << "Date/Heure brute: Jour=" << jour << ", Mois=" << mois << ", Année=" << annee
                             << ", Heure=" << heure << ", Minute=" << minute << ", Seconde=" << seconde;

                    QString chaineDateHeure = QString("%1-%2-20%3 %4:%5:%6")
                                                     .arg(jour, 2, 10, QChar('0'))
                                                     .arg(mois, 2, 10, QChar('0'))
                                                     .arg(annee, 2, 10, QChar('0'))
                                                     .arg(heure, 2, 10, QChar('0'))
                                                     .arg(minute, 2, 10, QChar('0'))
                                                     .arg(seconde, 2, 10, QChar('0'));

                    if (!premiereLecture) {
                        qDebug() << chaineDateHeure << " - Consommation totale (différence): " << consommationTotaleKWh << "kWh";
                        qDebug() << "Détails: Energie 1: " << (consommationWh1 / 1000.0f + consommationKWh1) << "kWh, Energie 2: " << (consommationWh2 / 1000.0f + consommationKWh2) << "kWh, Energie 3: " << (consommationWh3 / 1000.0f + consommationKWh3) << "kWh";
                        qDebug() << "Energie totale 1(KWh):" << valeurEnergieActuelleKWh1;
                        qDebug() << "Energie précédente totale 1(Wh):" << valeurEnergiePrecedenteWh1;
                        qDebug() << "Energie totale 1(Wh):" << valeurEnergieActuelleWh1;
                        qDebug() << "Energie totale 2(KWh):" << valeurEnergieActuelleKWh2;
                        qDebug() << "Energie précédente totale 2(Wh):" << valeurEnergiePrecedenteWh2;
                        qDebug() << "Energie totale 2(Wh):" << valeurEnergieActuelleWh2;
                        qDebug() << "Energie totale 3(KWh):" << valeurEnergieActuelleKWh3;
                        qDebug() << "Energie précédente totale 3(Wh):" << valeurEnergiePrecedenteWh3;
                        qDebug() << "Energie totale 3(Wh):" << valeurEnergieActuelleWh3;
                        connecteurBaseDeDonnees.insererDonnees(consommationTotaleKWh, chaineDateHeure, idDispositif);
                    } else {
                        premiereLecture = false;
                    }

                    valeurEnergiePrecedenteWh1 = valeurEnergieActuelleWh1;
                    valeurEnergiePrecedenteWh2 = valeurEnergieActuelleWh2;
                    valeurEnergiePrecedenteWh3 = valeurEnergieActuelleWh3;
                    valeurEnergiePrecedenteKWh1 = valeurEnergieActuelleKWh1;
                    valeurEnergiePrecedenteKWh2 = valeurEnergieActuelleKWh2;
                    valeurEnergiePrecedenteKWh3 = valeurEnergieActuelleKWh3;
                }
            }
            modbus.fermerConnexion();
            QThread::sleep(600); // Délai de 30 minutes
        }
    }

    return app.exec();
}
