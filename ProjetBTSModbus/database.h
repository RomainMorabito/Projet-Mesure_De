#ifndef CONNECTEURBASEDEDONNEES_H
#define CONNECTEURBASEDEDONNEES_H

#include <QSqlDatabase>
#include <QString>

class ConnecteurBaseDeDonnees {
public:
    ConnecteurBaseDeDonnees(const QString& nomHote, const QString& nomBaseDeDonnees, const QString& nomUtilisateur, const QString& motDePasse, int port);
    ~ConnecteurBaseDeDonnees();
    bool insererDonnees(float consommationTotaleKWh, const QString& chaineDateHeure, int idDispositif);
    QString obtenirAdresseIpPasserelle(int idDispositif);

private:
    QString nomHote;
    QString nomBaseDeDonnees;
    QString nomUtilisateur;
    QString motDePasse;
    int port;
    QSqlDatabase baseDeDonnees;

    bool seConnecterBaseDeDonnees();
    QString formaterDateHeure(const QString& chaineDateHeure);
};

#endif // CONNECTEURBASEDEDONNEES_H
