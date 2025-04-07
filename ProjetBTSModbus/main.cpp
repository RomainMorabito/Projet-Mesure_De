#include <QCoreApplication>
#include <QThread>
#include "modbus.h"
#include "database.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::locale::global(std::locale(""));

    ModbusCommunicator modbus("192.168.22.1", 502);
    DatabaseConnector dbConnector("192.168.16.25", "Mesure_De", "admin", "admin", 3306);

    float previousEnergyValueWh1 = 0.0f;
    float previousEnergyValueWh2 = 0.0f;
    float previousEnergyValueWh3 = 0.0f;
    float previousEnergyValueKWh1 = 0.0f;
    float previousEnergyValueKWh2 = 0.0f;
    float previousEnergyValueKWh3 = 0.0f;
    bool firstReading = true;

    while (true) {
        unsigned char modbusRequestEnergyWh1[] = {
            0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0x4D, 0x83, 0x00, 0x01
        };
        unsigned char modbusRequestEnergyWh2[] = {
            0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0x55, 0x83, 0x00, 0x01
        };
        unsigned char modbusRequestEnergyWh3[] = {
            0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0x5D, 0x83, 0x00, 0x01
        };
        unsigned char modbusRequestEnergyKWh1[] = {
            0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0x4D, 0x81, 0x00, 0x01
        };
        unsigned char modbusRequestEnergyKWh2[] = {
            0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0x55, 0x81, 0x00, 0x01
        };
        unsigned char modbusRequestEnergyKWh3[] = {
            0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05, 0x03, 0x5D, 0x81, 0x00, 0x01
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

        uint16_t energyValueWh1 = 0xFFFF;
        uint16_t energyValueWh2 = 0xFFFF;
        uint16_t energyValueWh3 = 0xFFFF;
        uint16_t energyValueKWh1 = 0xFFFF;
        uint16_t energyValueKWh2 = 0xFFFF;
        uint16_t energyValueKWh3 = 0xFFFF;
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

            energyValueWh1 = modbus.readModbusRegister(modbusRequestEnergyWh1, sizeof(modbusRequestEnergyWh1));
            energyValueWh2 = modbus.readModbusRegister(modbusRequestEnergyWh2, sizeof(modbusRequestEnergyWh2));
            energyValueWh3 = modbus.readModbusRegister(modbusRequestEnergyWh3, sizeof(modbusRequestEnergyWh3));
            energyValueKWh1 = modbus.readModbusRegister(modbusRequestEnergyKWh1, sizeof(modbusRequestEnergyKWh1));
            energyValueKWh2 = modbus.readModbusRegister(modbusRequestEnergyKWh2, sizeof(modbusRequestEnergyKWh2));
            energyValueKWh3 = modbus.readModbusRegister(modbusRequestEnergyKWh3, sizeof(modbusRequestEnergyKWh3));
            day = modbus.readModbusRegister(modbusRequestDay, sizeof(modbusRequestDay));
            month = modbus.readModbusRegister(modbusRequestMonth, sizeof(modbusRequestMonth));
            year = modbus.readModbusRegister(modbusRequestYear, sizeof(modbusRequestYear));
            hour = modbus.readModbusRegister(modbusRequestHour, sizeof(modbusRequestHour));
            minute = modbus.readModbusRegister(modbusRequestMinute, sizeof(modbusRequestMinute));
            second = modbus.readModbusRegister(modbusRequestSecond, sizeof(modbusRequestSecond));

            if (energyValueWh1 != 0xFFFF && energyValueWh2 != 0xFFFF && energyValueWh3 != 0xFFFF &&
                energyValueKWh1 != 0xFFFF && energyValueKWh2 != 0xFFFF && energyValueKWh3 != 0xFFFF &&
                day != 0xFFFF && month != 0xFFFF && year != 0xFFFF &&
                hour != 0xFFFF && minute != 0xFFFF && second != 0xFFFF) {
                break;
            }

            qDebug() << "Erreur lors de la lecture des registres. Réessai...";
            QThread::sleep(2); // Wait before retrying
        }

        if (energyValueWh1 == 0xFFFF || energyValueWh2 == 0xFFFF || energyValueWh3 == 0xFFFF ||
            energyValueKWh1 == 0xFFFF || energyValueKWh2 == 0xFFFF || energyValueKWh3 == 0xFFFF ||
            day == 0xFFFF || month == 0xFFFF || year == 0xFFFF ||
            hour == 0xFFFF || minute == 0xFFFF || second == 0xFFFF) {
            qDebug() << "Erreur lors de la lecture des registres après plusieurs tentatives";
        } else {
            float currentEnergyValueWh1 = static_cast<float>(energyValueWh1) / 10.0f;
            float currentEnergyValueWh2 = static_cast<float>(energyValueWh2) / 10.0f;
            float currentEnergyValueWh3 = static_cast<float>(energyValueWh3) / 10.0f;
            float currentEnergyValueKWh1 = static_cast<float>(energyValueKWh1);
            float currentEnergyValueKWh2 = static_cast<float>(energyValueKWh2);
            float currentEnergyValueKWh3 = static_cast<float>(energyValueKWh3);

            float consumptionWh1 = currentEnergyValueWh1 - previousEnergyValueWh1;
            float consumptionWh2 = currentEnergyValueWh2 - previousEnergyValueWh2;
            float consumptionWh3 = currentEnergyValueWh3 - previousEnergyValueWh3;

            float consumptionKWh1 = currentEnergyValueKWh1 - previousEnergyValueKWh1;
            float consumptionKWh2 = currentEnergyValueKWh2 - previousEnergyValueKWh2;
            float consumptionKWh3 = currentEnergyValueKWh3 - previousEnergyValueKWh3;

            float totalConsumptionWh = consumptionWh1 + consumptionWh2 + consumptionWh3;
            float totalConsumptionKWh = (totalConsumptionWh / 1000.0f) + consumptionKWh1 + consumptionKWh2 + consumptionKWh3;

            QString dateTimeString = QString("%1-%2-20%3 %4:%5:%6")
                                         .arg(day, 2, 10, QChar('0'))
                                         .arg(month, 2, 10, QChar('0'))
                                         .arg(year, 2, 10, QChar('0'))
                                         .arg(hour, 2, 10, QChar('0'))
                                         .arg(minute, 2, 10, QChar('0'))
                                         .arg(second, 2, 10, QChar('0'));

            if (!firstReading) {
                qDebug() << dateTimeString << " - Consommation totale (différence): " << totalConsumptionKWh << "kWh";
                qDebug() << "Détails: Energie 1: " << (consumptionWh1 / 1000.0f + consumptionKWh1) << "kWh, Energie 2: " << (consumptionWh2 / 1000.0f + consumptionKWh2) << "kWh, Energie 3: " << (consumptionWh3 / 1000.0f + consumptionKWh3) << "kWh";
                dbConnector.insertData(totalConsumptionKWh, dateTimeString);
            } else {
                firstReading = false;
            }

            previousEnergyValueWh1 = currentEnergyValueWh1;
            previousEnergyValueWh2 = currentEnergyValueWh2;
            previousEnergyValueWh3 = currentEnergyValueWh3;
            previousEnergyValueKWh1 = currentEnergyValueKWh1;
            previousEnergyValueKWh2 = currentEnergyValueKWh2;
            previousEnergyValueKWh3 = currentEnergyValueKWh3;
        }

        QThread::sleep(10); // Delay 60 minutes
    }

    return app.exec();
}
