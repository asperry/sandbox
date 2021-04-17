#include <iostream>
#include <QCoreApplication>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include "WirelessCrawler.h"

int main(int argc, char *argv[]) {

	QCoreApplication a(argc, argv);

	// Determine what discovery methods are available on the platform
	QBluetoothDeviceDiscoveryAgent::DiscoveryMethods availMethods = QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods();
	switch(availMethods) {

		case QBluetoothDeviceDiscoveryAgent::NoMethod:
			std::cout << "No available discovery method" << std::endl;
			break;

		case QBluetoothDeviceDiscoveryAgent::ClassicMethod:
			std::cout << "BR/EDR Only available" << std::endl;
			break;

		case QBluetoothDeviceDiscoveryAgent::LowEnergyMethod:
			std::cout << "BLE Only available" << std::endl;
			break;

		case (QBluetoothDeviceDiscoveryAgent::ClassicMethod | QBluetoothDeviceDiscoveryAgent::LowEnergyMethod):
			std::cout << "BR/EDR and BLE are available" << std::endl;
			break;

		default:
			std::cout << "Unknown discovery methods." << std::endl;
			break;

	}

	WirelessCrawler crawler("C7:53:D7:C0:18:90");
//	WirelessCrawler crawler("C7:53:D7:C0:18:90", "48:51:B7:EA:E1:69");
//	WirelessCrawler crawler("C7:53:D7:C0:18:90", "D8:DE:44:88:8B:17");

	return a.exec();

}
