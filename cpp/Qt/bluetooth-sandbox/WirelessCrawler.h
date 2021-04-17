#ifndef WIRELESS_CRAWLER_H
#define WIRELESS_CRAWLER_H

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QLowEnergyService>


class WirelessCrawler : public QObject {

	Q_OBJECT

	public:
		explicit WirelessCrawler(const char* crawlerAddress, const char* adapterAddress = nullptr, QObject* parent = nullptr);

	signals:

	private:
		const char* adapterAddress;
		const char* crawlerAddress;
		QBluetoothDeviceDiscoveryAgent* discoveryAgent;
		QLowEnergyController* central;
		QLowEnergyService* service1;
		QLowEnergyService* service2;
		bool crawlerFound = false;

		void printServiceDetails(QLowEnergyService* service);

	private slots:
		void identifyCrawlerDevice(const QBluetoothDeviceInfo& device);
		void deviceDiscoveryFinished();
		void serviceDiscoveryFinished();
		void characteristicChanged(const QLowEnergyCharacteristic& c, const QByteArray& value);

};

#endif // WIRELESS_CRAWLER_H
