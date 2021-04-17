#include "WirelessCrawler.h"
#include <iostream>
#include <QBluetoothAddress>
#include <QBluetoothUuid>
#include <QLowEnergyCharacteristic>
#include <QLowEnergyDescriptor>


// NOTE: I think it is probably wise to have a dedicated event loop running in it's own thread that this class is moved to.
WirelessCrawler::WirelessCrawler(const char* crawlerAddress, const char* adapterAddress, QObject *parent)
	: QObject(parent),
	  adapterAddress(adapterAddress),
	  crawlerAddress(crawlerAddress) {

	// Create a Bluetooth discovery agent. Note that the device discovery is necessary because it seems that a connection can only
	// be made to a device that has been previously discovered and cached. Devices that have previously been connected to remain in
	// the cache and once they are in the cache, the device discovery is no longer necessary. However, the device discovery is kept
	// as a step here to avoid forcing the user to first connect to the crawler via other means (e.g. bluetoothctl) before this class
	// will work.
	if (adapterAddress)
		discoveryAgent = new QBluetoothDeviceDiscoveryAgent(QBluetoothAddress(adapterAddress), this);
	else
		discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
	if (discoveryAgent->error()) {
		std::cerr << "WIRELESS_CRAWLER - An Bluetooth related error occurred! ERROR" << std::endl;
		std::cerr << "                   " << discoveryAgent->errorString().toLocal8Bit().toStdString() << std::endl;
		return;
	}
	discoveryAgent->setLowEnergyDiscoveryTimeout(5000);

	// Connect to the device discovery agent's signals
	connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &WirelessCrawler::identifyCrawlerDevice);
	connect(discoveryAgent, QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(&QBluetoothDeviceDiscoveryAgent::error), this, [this]() {
		std::cerr << "WIRELESS_CRAWLER - An error occurred during Bluetooth device discovery! ERROR" << std::endl;
		std::cerr << "                   " << discoveryAgent->errorString().toLocal8Bit().toStdString() << std::endl;
	});
	connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &WirelessCrawler::deviceDiscoveryFinished);
	connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &WirelessCrawler::deviceDiscoveryFinished);

	// Start Bluetooth discovery
	std::cout << "WIRELESS_CRAWLER - Discovering Bluetooth devices..." << std::endl;
	discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

}


/**
 * @brief This function is connected to the QBluetoothDeviceDiscoveryAgent::deviceDiscovered signal. It identifies the crawler during device
 * discovery and initiates a connection if the crawler is found (whether because the crawler is actively advertising or the crawler is cached
 * by the system as a previously discovered device).
 * @param device A newly discovered Bluetooth device
 */
void WirelessCrawler::identifyCrawlerDevice(const QBluetoothDeviceInfo& device) {

	// Check whether the discovered device is the crawler, ignore all other devices
	if (device.address().toString() == crawlerAddress) {

		// The crawler has been discovered
		crawlerFound = true;
		std::cout << "WIRELESS_CRAWLER - Crawler discovered - This does not necessarily mean the crawler is currently advertising (i.e. powered on) as discovery includes cached devices." << std::endl;
		std::cout << "                   Name:    " << device.name().toLocal8Bit().toStdString() << std::endl;
		std::cout << "                   Address: " << device.address().toString().toLocal8Bit().toStdString() << std::endl;
		discoveryAgent->stop();

		// Create a Bluetooth central to connect to the crawler
		if (adapterAddress)
			central = QLowEnergyController::createCentral(device.address(), QBluetoothAddress(QString(adapterAddress)), this);
		else
			central = QLowEnergyController::createCentral(device, this);

		// Connect to the central's signals
		connect(central, QOverload<QLowEnergyController::Error>::of(&QLowEnergyController::error), this, [this]() {
			std::cerr << "WIRELESS_CRAWLER - A Bluetooth related error occurred! ERROR" << std::endl;
			std::cerr << "                   " << central->errorString().toLocal8Bit().toStdString() << std::endl;
		});
//		connect(central, &QLowEnergyController::stateChanged, [](const QLowEnergyController::ControllerState& state) {
//			switch (state) {
//				case QLowEnergyController::UnconnectedState:
//					std::cout << "WIRELESS_CRAWLER - State changed to UnconnectedState" << std::endl; break;
//				case QLowEnergyController::ConnectingState:
//					std::cout << "WIRELESS_CRAWLER - State changed to ConnectingState" << std::endl; break;
//				case QLowEnergyController::ConnectedState:
//					std::cout << "WIRELESS_CRAWLER - State changed to ConnectedState" << std::endl; break;
//				case QLowEnergyController::DiscoveringState:
//					std::cout << "WIRELESS_CRAWLER - State changed to DiscoveringState" << std::endl; break;
//				case QLowEnergyController::DiscoveredState:
//					std::cout << "WIRELESS_CRAWLER - State changed to DiscoveredState" << std::endl; break;
//				case QLowEnergyController::ClosingState:
//					std::cout << "WIRELESS_CRAWLER - State changed to ClosingState" << std::endl; break;
//				case QLowEnergyController::AdvertisingState:
//					std::cout << "WIRELESS_CRAWLER - State changed to AdvertisingState" << std::endl; break;
//			}
//		});
		connect(central, &QLowEnergyController::disconnected, []() {
			std::cout << "WIRELESS_CRAWLER - Disconnected from crawler" << std::endl;
		});
		connect(central, &QLowEnergyController::connected, this, [this]() {
			std::cout << "WIRELESS_CRAWLER - Connection successful!" << std::endl;
			std::cout << "                   Address of adapter used: " << central->localAddress().toString().toLocal8Bit().toStdString() << std::endl;
			std::cout << "                   Performing service discovery..." << std::endl;
			central->discoverServices();
		});
		connect(central, &QLowEnergyController::discoveryFinished, this, &WirelessCrawler::serviceDiscoveryFinished);

		// Initiate a connection to the crawler
		std::cout << "WIRELESS_CRAWLER - Attempting to connect to crawler..." << std::endl;
		central->connectToDevice();

	}

}


void WirelessCrawler::deviceDiscoveryFinished() {

	std::cout << "WIRELESS_CRAWLER - Device discovery finished" << std::endl;
	if (!crawlerFound)
		std::cerr << "WIRELESS_CRAWLER - Crawler not found! ERROR" << std::endl;

}


void WirelessCrawler::serviceDiscoveryFinished() {

	QList<QBluetoothUuid> services = central->services();
//	std::cout << "Number of services: " << services.length() << std::endl;
//	for (int i = 0; i < services.length(); i++)
//		std::cout << "Service: " << services[i].toString().toLocal8Bit().toStdString() << std::endl;

//	service1 = central->createServiceObject(services[0], this);
//	if (service1) {

//		connect(service1, &QLowEnergyService::stateChanged, this, [this](QLowEnergyService::ServiceState s) {

//			switch(s) {

//				case QLowEnergyService::DiscoveringServices:
//					std::cout << "Service 1 - Discovering Service..." << std::endl;
//					break;

//				case QLowEnergyService::ServiceDiscovered:
//					std::cout << "Service 1 - Discovery complete" << std::endl;
////					printServiceDetails(service1);
//					break;

//				default:
//					std::cout << "Service 1 - State changed to " << s << std::endl;
//					break;

//			}


//		});

//		service1->discoverDetails();

//	}

	service2 = central->createServiceObject(services[1], this);
	if (service2) {

		connect(service2, &QLowEnergyService::characteristicChanged, this, &WirelessCrawler::characteristicChanged);
		connect(service2, &QLowEnergyService::stateChanged, this, [this](QLowEnergyService::ServiceState s) {

			switch(s) {

				case QLowEnergyService::DiscoveringServices:
//					std::cout << "Service 2 - Discovering Service..." << std::endl;
					break;

				case QLowEnergyService::ServiceDiscovered:
				{
					std::cout << "WIRELESS_CRAWLER - Service discovery complete" << std::endl;
//					printServiceDetails(service2);

					// Turn of notifications for the TX characterstic
					const QLowEnergyCharacteristic TXChar = service2->characteristic(QBluetoothUuid(QStringLiteral("6e400003-b5a3-f393-e0a9-e50e24dcca9e")));
					if (!TXChar.isValid()) {
						std::cerr << "WIRELESS_CRAWLER - UART TX Characteristic is not valid! ERROR" << std::endl;
						break;
					}
					const QLowEnergyDescriptor TXChar_notificationDesc = TXChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
					if (TXChar_notificationDesc.isValid())
						service2->writeDescriptor(TXChar_notificationDesc, QByteArray::fromHex("0100"));

					// Tell the crawler to start reading and sending data
					const QLowEnergyCharacteristic RXChar = service2->characteristic(QBluetoothUuid(QStringLiteral("6e400002-b5a3-f393-e0a9-e50e24dcca9e")));
					if (RXChar.isValid())
						service2->writeCharacteristic(RXChar, QByteArray::fromHex("630D"), QLowEnergyService::WriteWithResponse);

					break;
				}
				default:
					// Nothing for now
					break;

			}

		});

		service2->discoverDetails();

	}

}


void WirelessCrawler::characteristicChanged(const QLowEnergyCharacteristic& c, const QByteArray& value) {

	if (c.uuid() != QBluetoothUuid(QStringLiteral("6e400003-b5a3-f393-e0a9-e50e24dcca9e"))) {
		std::cout << "Some other service: " << c.uuid().toString().toLocal8Bit().toStdString() << std::endl;
		return;
	}

	std::cout << "New Value: " << value.toHex().toStdString() << std::endl;

}


void WirelessCrawler::printServiceDetails(QLowEnergyService* service) {

	std::cout << " Name: " << service->serviceName().toLocal8Bit().toStdString() << std::endl;
	std::cout << " UUID: " << service->serviceUuid().toString().toLocal8Bit().toStdString() << std::endl;
	std::cout << " Type: " << service->type() << std::endl;
	std::cout << " Number of included services: " << service->includedServices().length() << std::endl;

	QList<QLowEnergyCharacteristic> characteristics = service->characteristics();
	std::cout << " Number of charachteristics: " << characteristics.length() << std::endl;
	for (int i = 0; i < characteristics.length(); i++) {

		QLowEnergyCharacteristic c = characteristics[i];
		if (c.isValid()) {

			std::cout << "  Characteristic " << i+1 << std::endl;
			std::cout << "   Name: " << c.name().toLocal8Bit().toStdString() << std::endl;
			std::cout << "   UUID: " << c.uuid().toString().toLocal8Bit().toStdString() << std::endl;
			std::cout << "   Props: " << c.properties() << std::endl;
			std::cout << "   Value: " << c.value().toStdString() << std::endl;

			QList<QLowEnergyDescriptor> descriptors = c.descriptors();
			std::cout << "   Number of descriptors: " << descriptors.length() << std::endl;
			for (int j = 0; j < descriptors.length(); j++) {

				QLowEnergyDescriptor d = descriptors[j];
				if (d.isValid()) {

					std::cout << "    Descriptor " << j+1 << std::endl;
					std::cout << "     Name: " << d.name().toLocal8Bit().toStdString() << std::endl;
					std::cout << "     UUID: " << d.uuid().toString().toLocal8Bit().toStdString() << std::endl;
					std::cout << "     Type: " << d.type() << std::endl;
					std::cout << "     Value: " << d.value().toHex('-').toStdString() << std::endl;

				}

			}

		}

	}

}
