#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

ConnecteurBaseDeDonnees::ConnecteurBaseDeDonnees(const QString& nomHote, const QString& nomBaseDeDonnees, const QString& nomUtilisateur, const QString& motDePasse, int port)
    : nomHote(nomHote), nomBaseDeDonnees(nomBaseDeDonnees), nomUtilisateur(nomUtilisateur), motDePasse(motDePasse), port(port) {
    if (!seConnecterBaseDeDonnees()) {
        qCritical() << "La connexion à la base de données a échoué lors de l'initialisation. L'application risque de ne pas fonctionner correctement.";
    }
}

ConnecteurBaseDeDonnees::~ConnecteurBaseDeDonnees() {
    if (baseDeDonnees.isOpen()) {
        baseDeDonnees.close();
        qDebug() << "Connexion à la base de données fermée.";
    }
}

bool ConnecteurBaseDeDonnees::insererDonnees(float consommationTotaleKWh, const QString& chaineDateHeure, int idDispositif) {
    if (!baseDeDonnees.isOpen()) {
        qCritical() << "Erreur : Base de données non ouverte. Impossible d'insérer les données.";
        return false;
    }

    QString chaineDateHeureFormatee = formaterDateHeure(chaineDateHeure);

    QSqlQuery requete(baseDeDonnees);
    if (!requete.prepare("INSERT INTO Donnee_Mesurer (Valeur_Mesure, Timestamp, ID_Dispositif_FK) VALUES (:consumption_kwh, :timestamp, :dispositif_id)")) {
        qCritical() << "Erreur lors de la préparation de la requête d'insertion :" << requete.lastError().text();
        return false;
    }

    requete.bindValue(":consumption_kwh", consommationTotaleKWh);
    requete.bindValue(":timestamp", chaineDateHeureFormatee);
    requete.bindValue(":dispositif_id", idDispositif);

    if (!requete.exec()) {
        qCritical() << "Erreur lors de l'insertion des données dans la base de données :" << requete.lastError().text();
        return false;
    } else {
        qDebug() << "Données insérées avec succès dans la base de données";
        return true;
    }
}

QString ConnecteurBaseDeDonnees::obtenirAdresseIpPasserelle(int idDispositif) {
    if (!baseDeDonnees.isOpen()) {
        qCritical() << "Erreur : Base de données non ouverte. Impossible de récupérer l'adresse IP.";
        return QString();
    }

    // Modification du nom de la table pour correspondre à la casse probable de phpMyAdmin (minuscules)
    QString requeteStr = QString("SELECT Adresse_IP_Passerelle FROM Dispositif_Passerelle WHERE ID_Dispositif_PK = %1").arg(idDispositif);
    QSqlQuery requete(baseDeDonnees);

    if (requete.exec(requeteStr)) {
        if (requete.next()) {
            return requete.value(0).toString();
        } else {
            qWarning() << "Aucune adresse IP trouvée pour l'ID_Dispositif_PK :" << idDispositif;
            return QString();
        }
    } else {
        qCritical() << "Erreur lors de l'exécution de la requête de récupération d'IP :" << requete.lastError().text() << " - Requête: " << requeteStr;
        return QString();
    }
}

bool ConnecteurBaseDeDonnees::seConnecterBaseDeDonnees() {
    if (QSqlDatabase::contains("unique_connection_name")) {
        qDebug() << "Utilisation de la connexion existante à la base de données.";
        baseDeDonnees = QSqlDatabase::database("unique_connection_name");
        return baseDeDonnees.isOpen(); // Retourne true si la connexion existante est ouverte
    } else {
        baseDeDonnees = QSqlDatabase::addDatabase("QMYSQL", "unique_connection_name");
        baseDeDonnees.setHostName(nomHote);
        baseDeDonnees.setDatabaseName(nomBaseDeDonnees);
        baseDeDonnees.setUserName(nomUtilisateur);
        baseDeDonnees.setPassword(motDePasse);
        baseDeDonnees.setPort(port);
    }

    if (!baseDeDonnees.open()) {
        qCritical() << "Erreur de connexion à la base de données :" << baseDeDonnees.lastError().text();
        return false; // Retourne false en cas d'erreur de connexion
    } else {
        qDebug() << "Connexion à la base de données réussie (via MySQL)";
        return true; // Retourne true si la connexion réussit
    }
}

QString ConnecteurBaseDeDonnees::formaterDateHeure(const QString& chaineDateHeure) {
    return QString("%1-%2-%3 %4:%5:%6")
    .arg(chaineDateHeure.mid(6, 4))  // Année
    .arg(chaineDateHeure.mid(3, 2))  // Mois
    .arg(chaineDateHeure.mid(0, 2))  // Jour
    .arg(chaineDateHeure.mid(11, 2)) // Heure
    .arg(chaineDateHeure.mid(14, 2)) // Minute
    .arg(chaineDateHeure.mid(17, 2)); // Seconde
}
