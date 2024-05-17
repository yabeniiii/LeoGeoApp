#include "LeoGeo/usb_comm.hpp"

#include <libusb-1.0/libusb.h>

#include <expected>
#include <format>
#include <string>
#include <vector>

namespace LeoGeoUsb {

namespace {
constexpr std::uint16_t kVendor_id = 0x0781;
constexpr std::uint16_t kProduct_id = 0x5595;
}  // namespace

std::expected<void, std::string> UsbStart() {
  std::string libusb_version =
      std::format("{}.{}.{}", libusb_get_version()->major,
                  libusb_get_version()->minor, libusb_get_version()->micro);

  if (const auto init_status = libusb_init(NULL);
      init_status != LIBUSB_SUCCESS) {
    return std::unexpected(std::format(
        "libusb {}: init error: {} {}: {}", libusb_version, init_status,
        libusb_error_name(init_status), libusb_strerror(init_status)));
  }

  libusb_device_handle* device_handle =
      libusb_open_device_with_vid_pid(NULL, kVendor_id, kProduct_id);

  if (device_handle == NULL) {
    return std::unexpected(
        std::format("libusb {}: could not open device with vid: {} and pid: {}",
                    libusb_version, kVendor_id, kProduct_id));
  }

  libusb_device* device = libusb_get_device(device_handle);

  libusb_device_descriptor device_descriptor = {0};
  libusb_get_device_descriptor(device, &device_descriptor);

  if (auto descriptor_get_status =
          libusb_get_device_descriptor(device, &device_descriptor);
      descriptor_get_status != LIBUSB_SUCCESS) {
    return std::unexpected(std::format(
        "libusb {}: device descriptor fetch error: {} {}: {}", libusb_version,
        descriptor_get_status, libusb_error_name(descriptor_get_status),
        libusb_strerror(descriptor_get_status)));
  };

  libusb_exit(NULL);
  return {};
}

std::expected<void, std::string> GetLogged() { return {}; }

std::expected<std::vector<Coordinates>, std::string> UpdateCoordinates(
    std::vector<Coordinates> new_coordinates) {
  return new_coordinates;
}

}  // namespace LeoGeoUsb
