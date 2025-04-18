#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseConnector::DatabaseConnector(const QString& hostName, const QString& dbName, const QString& userName, const QString& password, int port)
    : hostName(hostName), dbName(dbName), userName(userName), password(password), port(port) {
    if (!connectToDatabase()) {
        qCritical() << "La connexion à la base de données a échoué lors de l'initialisation. L'application risque de ne pas fonctionner correctement.";
    }
}

DatabaseConnector::~DatabaseConnector() {
    if (db.isOpen()) {
        db.close();
        qDebug() << "Connexion à la base de données fermée.";
    }
}

bool DatabaseConnector::insertData(float totalConsumptionKWh, const QString& dateTimeString, int dispositifId) {
    if (!db.isOpen()) {
        qCritical() << "Erreur : Base de données non ouverte. Impossible d'insérer les données.";
        return false;
    }

    QString formattedDateTimeString = formatDateTime(dateTimeString);

    QSqlQuery query(db);
    if (!query.prepare("INSERT INTO Donnee_Mesurer (Valeur_Mesure, Timestamp, ID_Dispositif_FK) VALUES (:consumption_kwh, :timestamp, :dispositif_id)")) {
        qCritical() << "Erreur lors de la préparation de la requête d'insertion :" << query.lastError().text();
        return false;
    }

    query.bindValue(":consumption_kwh", totalConsumptionKWh);
    query.bindValue(":timestamp", formattedDateTimeString);
    query.bindValue(":dispositif_id", dispositifId);

    if (!query.exec()) {
        qCritical() << "Erreur lors de l'insertion des données dans la base de données :" << query.lastError().text();
        return false;
    } else {
        qDebug() << "Données insérées avec succès dans la base de données";
        return true;
    }
}

QString DatabaseConnector::getGatewayIpAddress(int dispositifId) {
    if (!db.isOpen()) {
        qCritical() << "Erreur : Base de données non ouverte. Impossible de récupérer l'adresse IP.";
        return QString();
    }

    // Modification du nom de la table pour correspondre � la casse probable de phpMyAdmin (minuscules)
    QString queryStr = QString("SELECT Adresse_IP_Passerelle FROM Dispositif_Passerelle WHERE ID_Dispositif_PK = %1").arg(dispositifId);
    QSqlQuery query(db);

    if (query.exec(queryStr)) {
        if (query.next()) {
            return query.value(0).toString();
        } else {
            qWarning() << "Aucune adresse IP trouvée pour l'ID_Dispositif_PK :" << dispositifId;
            return QString();
        }
    } else {
        qCritical() << "Erreur lors de l'exécution de la requête de récupération d'IP :" << query.lastError().text() << " - Requête: " << queryStr;
        return QString();
    }
}

bool DatabaseConnector::connectToDatabase() {
    if (QSqlDatabase::contains("unique_connection_name")) {
        qDebug() << "Utilisation de la connexion existante à la base de données.";
        db = QSqlDatabase::database("unique_connection_name");
        return db.isOpen(); // Retourne true si la connexion existante est ouverte
    } else {
        db = QSqlDatabase::addDatabase("QMYSQL", "unique_connection_name");
        db.setHostName(hostName);
        db.setDatabaseName(dbName);
        db.setUserName(userName);
        db.setPassword(password);
        db.setPort(port);
    }

    if (!db.open()) {
        qCritical() << "Erreur de connexion à la base de données :" << db.lastError().text();
        return false; // Retourne false en cas d'erreur de connexion
    } else {
        qDebug() << "Connexion à la base de données réussie (via MySQL)";
        return true; // Retourne true si la connexion réussit
    }
}

QString DatabaseConnector::formatDateTime(const QString& dateTimeString) {
    return QString("%1-%2-%3 %4:%5:%6")
    .arg(dateTimeString.mid(6, 4))  // Année
    .arg(dateTimeString.mid(3, 2))  // Mois
    .arg(dateTimeString.mid(0, 2))  // Jour
    .arg(dateTimeString.mid(11, 2)) // Heure
    .arg(dateTimeString.mid(14, 2)) // Minute
    .arg(dateTimeString.mid(17, 2)); // Seconde
}
