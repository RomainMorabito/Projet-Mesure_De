#include <QCoreApplication>
#include <QThread>
#include "modbus.h"
#include "database.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::locale::global(std::locale(""));

    ModbusCommunicator modbus("192.168.22.1", 502);
    DatabaseConnector dbConnector("192.168.17.2", "Mesure_De", "admin", "admin", 3306);

    float previousEnergyValue = 0.0f;
    bool firstReading = true;

    while (true) {
        unsigned char modbusRequestEnergy[] = {
            0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0x4D, 0x83, 0x00, 0x01
        };
        unsigned char modbusRequestDay[] = {
            0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0xE1, 0x00, 0x00, 0x01
        };
        unsigned char modbusRequestMonth[] = {
            0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0xE1, 0x01, 0x00, 0x01
        };
        unsigned char modbusRequestYear[] = {
            0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0xE1, 0x02, 0x00, 0x01
        };
        unsigned char modbusRequestHour[] = {
            0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0xE1, 0x03, 0x00, 0x01
        };
        unsigned char modbusRequestMinute[] = {
            0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0xE1, 0x04, 0x00, 0x01
        };
        unsigned char modbusRequestSecond[] = {
            0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0xE1, 0x05, 0x00, 0x01
        };

        uint16_t energyValue = 0xFFFF;
        uint16_t day = 0xFFFF;
        uint16_t month = 0xFFFF;
        uint16_t year = 0xFFFF;
        uint16_t hour = 0xFFFF;
        uint16_t minute = 0xFFFF;
        uint16_t second = 0xFFFF;

        // Retry mechanism
        for (int attempt = 0; attempt < 3; ++attempt) {
            if (!modbus.isConnected()) {
                modbus.connectToServer();
            }

            energyValue = modbus.readModbusRegister(modbusRequestEnergy, sizeof(modbusRequestEnergy));
            day = modbus.readModbusRegister(modbusRequestDay, sizeof(modbusRequestDay));
            month = modbus.readModbusRegister(modbusRequestMonth, sizeof(modbusRequestMonth));
            year = modbus.readModbusRegister(modbusRequestYear, sizeof(modbusRequestYear));
            hour = modbus.readModbusRegister(modbusRequestHour, sizeof(modbusRequestHour));
            minute = modbus.readModbusRegister(modbusRequestMinute, sizeof(modbusRequestMinute));
            second = modbus.readModbusRegister(modbusRequestSecond, sizeof(modbusRequestSecond));

            if (energyValue != 0xFFFF && day != 0xFFFF && month != 0xFFFF && year != 0xFFFF && hour != 0xFFFF && minute != 0xFFFF && second != 0xFFFF) {
                break;
            }

            qDebug() << "Erreur lors de la lecture des registres. Réessai...";
            QThread::sleep(2); // Wait before retrying
        }

        if (energyValue == 0xFFFF || day == 0xFFFF || month == 0xFFFF || year == 0xFFFF || hour == 0xFFFF || minute == 0xFFFF || second == 0xFFFF) {
            qDebug() << "Erreur lors de la lecture des registres après plusieurs tentatives";
        } else {
            float currentEnergyValueWh = static_cast<float>(energyValue) / 10.0f;
            float currentEnergyValueKWh = currentEnergyValueWh / 1000.0f;

            float consumptionWh = currentEnergyValueWh - previousEnergyValue;
            float consumptionKWh = consumptionWh / 1000.0f;

            QString dateTimeString = QString("%1-%2-20%3 %4:%5:%6")
                                         .arg(day, 2, 10, QChar('0'))
                                         .arg(month, 2, 10, QChar('0'))
                                         .arg(year, 2, 10, QChar('0'))
                                         .arg(hour, 2, 10, QChar('0'))
                                         .arg(minute, 2, 10, QChar('0'))
                                         .arg(second, 2, 10, QChar('0'));

            if (!firstReading) {
                qDebug() << dateTimeString << " - Consommation (différence): " << consumptionKWh << "kWh";
                dbConnector.insertData(consumptionKWh, dateTimeString);
            } else {
                firstReading = false;
            }

            previousEnergyValue = currentEnergyValueWh;
        }

        QThread::sleep(600); // Delay 60 minutes
    }

    return app.exec();
}
