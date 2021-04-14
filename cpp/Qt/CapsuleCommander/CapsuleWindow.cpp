#include "CapsuleWindow.h"
#include "ui_CapsuleWindow.h"
#include <QList>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <iostream>

CapsuleWindow::CapsuleWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CapsuleWindow) {

    ui->setupUi(this);

    // Get a list of the available ports
    avail_ports = QSerialPortInfo::availablePorts();

    // Add the avaliable ports to the combo box
    for (int i = 0; i < avail_ports.length(); i++) {
        ui->comboBox->addItem(avail_ports[i].portName());
    }

    // Set up the QCustomPlot
    sensorData1 = new QVector<double>();
    sensorData2 = new QVector<double>();
    sensorData3 = new QVector<double>();
    sampleNum = new QVector<double>();

    ui->plot->addGraph();
    ui->plot->addGraph();
    ui->plot->addGraph();
    ui->plot->graph(0)->setData(*sampleNum, *sensorData1, true);
    ui->plot->graph(1)->setData(*sampleNum, *sensorData2, true);
    ui->plot->graph(2)->setData(*sampleNum, *sensorData3, true);
    ui->plot->graph(0)->setPen(QPen(Qt::blue));
    ui->plot->graph(1)->setPen(QPen(Qt::red));
    ui->plot->graph(2)->setPen(QPen(Qt::green));

    ui->plot->yAxis->setRange(-50, 5000);
    ui->plot->xAxis->setRange(0, graphTotalSamples);

    // Set up the rest of the GUI
    ui->connectedBox->setEnabled(false);
    ui->capturingBox->setDisabled(true);
    ui->startButton->setDisabled(true);
    ui->stopButton->setDisabled(true);
    ui->statusLabel->setText("");

    // Set up signal-slot connections
    connect(&port, &QSerialPort::readyRead, this, &CapsuleWindow::read_serial_port);
    connect(&port, &QSerialPort::errorOccurred, this, &CapsuleWindow::serial_port_error_handler);

}

CapsuleWindow::~CapsuleWindow() {

    port.close();
    delete ui;

}

void CapsuleWindow::read_serial_port() {

    while (port.canReadLine()) {

        QByteArray bytes = port.readLine(message_length + 1);

        if (bytes.length() == message_length) {

            int16_t val1 = int16_t((bytes[3] << 8) | uint8_t(bytes[2]));
            int16_t val2 = int16_t((bytes[5] << 8) | uint8_t(bytes[4]));
            int16_t val3 = int16_t((bytes[7] << 8) | uint8_t(bytes[6]));

            ui->lcdSensor1->display(int(val1));
            ui->lcdSensor2->display(int(val2));
            ui->lcdSensor3->display(int(val3));

            sampleNumber++;

            if (sampleNumber > graphTotalSamples) {

                sampleNumber = 1;
                delete sensorData1;
                delete sensorData2;
                delete sensorData3;
                delete sampleNum;

                sensorData1 = new QVector<double>();
                sensorData2 = new QVector<double>();
                sensorData3 = new QVector<double>();
                sampleNum = new QVector<double>();

                ui->plot->graph(0)->setData(*sampleNum, *sensorData1, true);
                ui->plot->graph(1)->setData(*sampleNum, *sensorData2, true);
                ui->plot->graph(2)->setData(*sampleNum, *sensorData3, true);

            }

            ui->plot->graph(0)->addData(sampleNumber, val1);
            ui->plot->graph(1)->addData(sampleNumber, val2);
            ui->plot->graph(2)->addData(sampleNumber, val3);

            ui->plot->replot();

        }

    }

}

void CapsuleWindow::serial_port_error_handler() {

    QSerialPort::SerialPortError error = port.error();

    // Provide a message based on the error that occurred
    if (error == QSerialPort::NoError) {
        return; // There was no error so no need to show anything
    }
    if (error == QSerialPort::DeviceNotFoundError) {
        ui->statusLabel->setText("Serial Port Error: Device Not Found Error!");
    }
    else if (error == QSerialPort::PermissionError) {
        ui->statusLabel->setText("Serial Port Error: Permission Error!");
    }
    else if (error == QSerialPort::OpenError) {
        ui->statusLabel->setText("Serial Port Error: Open Error!");
    }
    else if (error == QSerialPort::NotOpenError) {
        ui->statusLabel->setText("Serial Port Error: Not Open Error!");
    }
    else if (error == QSerialPort::ParityError) {
        ui->statusLabel->setText("Serial Port Error: Parity Error!");
    }
    else if (error == QSerialPort::FramingError) {
        ui->statusLabel->setText("Serial Port Error: Framing Error!");
    }
    else if (error == QSerialPort::BreakConditionError) {
        ui->statusLabel->setText("Serial Port Error: Break Condition Error!");
    }
    else if (error == QSerialPort::WriteError) {
        ui->statusLabel->setText("Serial Port Error: Write Error!");
    }
    else if (error == QSerialPort::ReadError) {
        ui->statusLabel->setText("Serial Port Error: Read Error!");
    }
    else if (error == QSerialPort::ResourceError) {
        ui->statusLabel->setText("Serial Port Error: Resource Error!");
    }
    else if (error == QSerialPort::UnsupportedOperationError) {
        ui->statusLabel->setText("Serial Port Error: Unsupported Operation Error!");
    }
    else if (error == QSerialPort::TimeoutError) {
        ui->statusLabel->setText("Serial Port Error: Timeout Error!");
    }
    else if (error == QSerialPort::UnknownError) {
        ui->statusLabel->setText("Serial Port Error: Unknown Error!");
    }

}

void CapsuleWindow::on_openButton_clicked() {

    if (ui->openButton->text() == "Open Port") {

        // Set up the serial port settings
        port.setPort(avail_ports[ui->comboBox->currentIndex()]);
        port.setDataBits(QSerialPort::Data8);
        port.setStopBits(QSerialPort::OneStop);
        port.setBaudRate(QSerialPort::Baud115200);

        // Open the serial port
        if (port.open(QIODevice::ReadWrite)) {
            ui->comboBox->setDisabled(true);
            ui->connectedBox->setCheckState(Qt::Checked);
            ui->openButton->setText("Close Port");
            ui->startButton->setEnabled(true);
            ui->stopButton->setEnabled(true);
        }

    }
    else {

        // Close the serial port
        port.close();
        ui->connectedBox->setCheckState(Qt::Unchecked);
        ui->openButton->setText("Open Port");
        ui->comboBox->setEnabled(true);

    }

}

void CapsuleWindow::on_startButton_clicked() {

    port.write("c\r");
    ui->capturingBox->setCheckState(Qt::Checked);

}

void CapsuleWindow::on_stopButton_clicked() {

    port.write("s\r");
    ui->capturingBox->setCheckState(Qt::Unchecked);

}
