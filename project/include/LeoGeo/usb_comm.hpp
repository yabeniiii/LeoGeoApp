#ifndef LEOGEO_USB_COMM_H_
#define LEOGEO_USB_COMM_H_

#include <QMutex>
#include <QObject>
#include <QString>
#include <QThread>
#include <QWaitCondition>
#include <cstdint>
#include <expected>
#include <string>

namespace LeoGeoUsb {

class UartReceiver;
class UartSender;

struct Coordinates {
  double latitude;
  double longitude;
};

class UartReceiver : public QThread {
 public:
  explicit UartReceiver(QObject *parent = nullptr);

  void Receive(const std::string &port, const std::uint8_t timeout = 0);

  std::string GetData();

 signals:
  void request(const QString &s);
  void error(const QString &s);

 private:
  void run() override;

  std::string port_name_;
  std::string data_;
  int wait_timeout_ = 0;
  bool quit_ = false;
};

class UartSender : public QThread {
 public:
  explicit UartSender(QObject *parent = nullptr);

  void Send(const std::string &port, const std::uint8_t timeout = 0,
            const std::string data = "");

  std::string GetData();

 signals:
  void request(const QString &s);
  void error(const QString &s);

 private:
  void run() override;

  std::string port_name_;
  std::string data_;
  int wait_timeout_ = 0;
  QWaitCondition wait_condition_;
  bool quit_ = false;
};
}  // namespace LeoGeoUsb

#endif  // LEOGEO_USB_COMM_H_
