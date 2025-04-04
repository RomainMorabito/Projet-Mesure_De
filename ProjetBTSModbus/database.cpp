#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseConnector::DatabaseConnector(const QString& hostName, const QString& dbName, const QString& userName, const QString& password, int port)
    : hostName(hostName), dbName(dbName), userName(userName), password(password), port(port) {
    connectToDatabase();
}

DatabaseConnector::~DatabaseConnector() {
    db.close();
}

void DatabaseConnector::insertData(float consumptionKWh, const QString& dateTimeString) {
    if (!db.isOpen()) {
        qDebug() << "Erreur de connexion à la base de données :" << db.lastError().text();
        return;
    }

    QString formattedDateTimeString = formatDateTime(dateTimeString);

    QSqlQuery query(db);
    if (!query.prepare("INSERT INTO Donnee_Mesurer (Valeur_Mesure, Timestamp) VALUES (:consumption_kwh, :timestamp)")) {
        qDebug() << "Erreur lors de la préparation de la requête :" << query.lastError().text();
        return;
    }

    query.bindValue(":consumption_kwh", consumptionKWh);
    query.bindValue(":timestamp", formattedDateTimeString);

    if (!query.exec()) {
        qDebug() << "Erreur lors de l'insertion des données dans la base de données :" << query.lastError().text();
    } else {
        qDebug() << "Données insérées avec succès dans la base de données";
    }
}

void DatabaseConnector::connectToDatabase() {
    if (QSqlDatabase::contains("unique_connection_name")) {
        qDebug() << "Utilisation de la connexion existante à la base de données.";
    } else {
        db = QSqlDatabase::addDatabase("QODBC", "unique_connection_name");
        db.setHostName(hostName);
        db.setDatabaseName(dbName);
        db.setUserName(userName);
        db.setPassword(password);
        db.setPort(port);
    }

    if (!db.open()) {
        qDebug() << "Erreur de connexion à la base de données :" << db.lastError().text();
    } else {
        qDebug() << "Connexion à la base de données réussie";
    }
}

QString DatabaseConnector::formatDateTime(const QString& dateTimeString) {
    return QString("%1-%2-%3 %4:%5:%6")
    .arg(dateTimeString.mid(6, 4))  // Année
        .arg(dateTimeString.mid(3, 2))  // Mois
        .arg(dateTimeString.mid(0, 2))  // Jour
        .arg(dateTimeString.mid(11, 2)) // Heure
        .arg(dateTimeString.mid(14, 2)) // Minute
        .arg(dateTimeString.mid(17, 2));// Seconde
}
