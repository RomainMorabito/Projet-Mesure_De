#ifndef DATABASECONNECTOR_H
#define DATABASECONNECTOR_H

#include <QSqlDatabase>
#include <QString>

class DatabaseConnector {
public:
    DatabaseConnector(const QString& hostName, const QString& dbName, const QString& userName, const QString& password, int port);
    ~DatabaseConnector();
    void insertData(float totalConsumptionKWh, const QString& dateTimeString);

private:
    QString hostName;
    QString dbName;
    QString userName;
    QString password;
    int port;
    QSqlDatabase db;

    void connectToDatabase();
    QString formatDateTime(const QString& dateTimeString);
};

#endif // DATABASECONNECTOR_H
