#include "LeoGeo/usb_comm.hpp"

#include <libusb.h>

#include <expected>
#include <format>
#include <memory>
#include <string>

namespace LeoGeoUsb {

namespace {
constexpr uint16_t kVendor_id = 0x0000;
constexpr uint16_t kProduct_id = 0x0000;
}  // namespace

std::expected<std::string, std::string> UsbStart() {
  if (const auto init_status = libusb_init(NULL); init_status != 0) {
    return std::unexpected(std::format("libusb init error: {}", init_status));
  }

  libusb_device_handle* device_handle =
      libusb_open_device_with_vid_pid(NULL, kVendor_id, kProduct_id);

  if (device_handle == NULL) {
    return std::unexpected(
        std::format("libusb: could not open device with vid: {} and pid: {}",
                    kVendor_id, kProduct_id));
  }

  libusb_exit(NULL);
  return "Ok";
}
}  // namespace LeoGeoUsb
