#ifndef CAPSULEWINDOW_H
#define CAPSULEWINDOW_H

#include <QDialog>
#include <QSerialPort>

namespace Ui {
    class CapsuleWindow;
}

class CapsuleWindow : public QDialog {

    Q_OBJECT

public:
    explicit CapsuleWindow(QWidget *parent = nullptr);
    ~CapsuleWindow();

private slots:
    void read_serial_port();
    void serial_port_error_handler();
    void on_openButton_clicked();
    void on_startButton_clicked();
    void on_stopButton_clicked();

private:
    Ui::CapsuleWindow *ui;
    QList<QSerialPortInfo> avail_ports;
    QSerialPort port;
    QByteArray read_buffer;
    int message_length = 9;
    QVector<double>* sensorData1;
    QVector<double>* sensorData2;
    QVector<double>* sensorData3;
    QVector<double>* sampleNum;
    double sampleNumber = 0;
    double graphTotalSamples = 300;

};

#endif // CAPSULEWINDOW_H
