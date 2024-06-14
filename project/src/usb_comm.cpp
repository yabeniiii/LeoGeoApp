#include "LeoGeo/usb_comm.hpp"

#include <libusb-1.0/libusb.h>

#include <QTime>
#include <format>
#include <string>
#include <vector>

namespace LeoGeoUsb {

Usb::Usb() : error_("") {
  libusb_version_ =
      std::format("{}.{}.{}", libusb_get_version()->major,
                  libusb_get_version()->minor, libusb_get_version()->micro);

  if (const auto status = libusb_init(NULL); status != LIBUSB_SUCCESS) {
    error_ = std::format("libusb {} init error: {} {}", libusb_version_, status,
                         libusb_error_name(status));
  }

  device_handle_ = libusb_open_device_with_vid_pid(NULL, kV_id, kP_id);

  if (device_handle_ == NULL) {
    error_ = std::format("libusb {} error, could not get device handle",
                         libusb_version_);
  }
}

std::expected<std::vector<LogData>, std::string> Usb::UsbRetrieve() {
  unsigned char data = {0};
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  libusb_bulk_transfer(device_handle_, 0x1, &data, 64000, nullptr, 10000);
  std::string data_string = std::to_string(data);
  qDebug("%s", data_string.c_str());

  std::vector<LogData> data_vec;
  bool exit = false;

  while (!exit) {
    if (data_string.empty()) exit = true;

    std::string time = data_string.substr(data_string.find_first_of(","));
    int time_h = stoi(time.substr(0, 1));
    int time_m = stoi(time.substr(2, 3));
    int time_s = stoi(time.substr(4, 5));  // NOLINT
    data_string.erase(data_string.find_first_of(","));

    double latitude =
        std::stod(data_string.substr(data_string.find_first_of(",")));
    data_string.erase(data_string.find_first_of(","));

    double longitude =
        std::stod(data_string.substr(data_string.find_first_of(",")));
    data_string.erase(data_string.find_first_of(","));

    double temperature =
        std::stod(data_string.substr(data_string.find_first_of(";")));
    data_string.erase(data_string.find_first_of(";"));

    data_vec.push_back(LogData{Coordinates{latitude, longitude}, temperature,
                               QTime(time_h, time_m, time_s)});
  }

  return {};
}

std::expected<void, std::string> Usb::UsbUpload(
    std::vector<Coordinates>* coordinates) {
  return {};
}

}  // namespace LeoGeoUsb
