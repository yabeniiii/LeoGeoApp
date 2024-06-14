#pragma once

#include <libusb-1.0/libusb.h>

#include <QTime>
#include <expected>
#include <string>
#include <vector>

namespace LeoGeoUsb {
struct Coordinates {
  double latitude;
  double longitude;
};

struct LogData {
  Coordinates coordinates;
  double temperature;
  QTime time;
};

class Usb {
 public:
  explicit Usb();
  std::expected<std::vector<LogData>, std::string> UsbRetrieve();
  std::expected<void, std::string> UsbUpload(
      std::vector<Coordinates>* coordinates);
  std::string Error();

 private:
  std::string libusb_version_;
  std::string error_;
  libusb_device_handle* device_handle_;
  static constexpr std::uint16_t kP_id = 0x0;
  static constexpr std::uint16_t kV_id = 0x0;
};
}  // namespace LeoGeoUsb
