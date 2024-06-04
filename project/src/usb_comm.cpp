#include "LeoGeo/usb_comm.hpp"

#include <QMutex>
#include <QObject>
#include <QString>
#include <QThread>
#include <QTime>
#include <QWaitCondition>
#include <QtSerialPort/QSerialPort>
#include <cstdint>
#include <expected>
#include <string>

namespace LeoGeoUsb {

UartReceiver::UartReceiver(QObject *parent) : QThread() {}

void UartReceiver::error(const QString &s) {}

void UartReceiver::Receive(const std::string &port,
                           const std::uint8_t timeout) {
  port_name_ = port;
  wait_timeout_ = timeout;

  if (!isRunning()) start();
}

std::string UartReceiver::GetData() { return data_; }

void UartReceiver::run() {
  QSerialPort serial;
  while (!quit_) {
    serial.close();
    serial.setPortName(tr(port_name_.c_str()));
    serial.setDataBits(QSerialPort::DataBits::Data8);
    serial.setParity(QSerialPort::Parity::NoParity);
    serial.setStopBits(QSerialPort::StopBits::OneStop);
    serial.setBaudRate(9600);  // NOLINT(cppcoreguidelines-avoid-magic-numbers)

    if (!serial.open(QIODevice::ReadWrite)) {
      emit error(tr("Can't open %1, error code %2")
                     .arg(port_name_.c_str())
                     .arg(serial.error()));
      return;
    }

    if (serial.waitForReadyRead(wait_timeout_)) {
      QByteArray request_data = serial.readAll();
      while (serial.waitForReadyRead()) request_data += serial.readAll();
    } else {
      emit error(tr("Wait read request timeout %1")
                     .arg(QTime::currentTime().toString()));
    }
  }
}

UartSender::UartSender(QObject *parent) : QThread() {}

void UartSender::Send(const std::string &port, const std::uint8_t timeout,
                      std::string data) {
  port_name_ = port;
  wait_timeout_ = timeout;
  data_ = data;

  if (!isRunning()) {
    run();
  }
}

std::string UartSender::GetData() { return data_; }

void UartSender::error(const QString &s) {}

void UartSender::run() {
  QSerialPort serial;

  serial.close();
  serial.setPortName(tr(port_name_.c_str()));
  serial.setDataBits(QSerialPort::DataBits::Data8);
  serial.setParity(QSerialPort::Parity::NoParity);
  serial.setStopBits(QSerialPort::StopBits::OneStop);
  serial.setBaudRate(9600);  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
  serial.setFlowControl(QSerialPort::FlowControl::SoftwareControl);

  if (!serial.open(QIODevice::ReadWrite)) {
    emit error(tr("Can't open %1, error code %2")
                   .arg(port_name_.c_str())
                   .arg(serial.error()));
    return;
  }

  QByteArray dataByteArray(data_.c_str(), data_.length());  // NOLINT
  serial.write(dataByteArray);
}

}  // namespace LeoGeoUsb
