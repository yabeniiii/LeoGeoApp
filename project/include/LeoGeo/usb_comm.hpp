#ifndef LEOGEO_USB_COMM_H_
#define LEOGEO_USB_COMM_H_

#include <cstdint>
#include <expected>
#include <string>

namespace LeoGeoUsb {

struct Coordinates {
  std::uint64_t latitude;
  std::uint64_t longitude;
};

std::expected<void, std::string> UsbStart();
std::expected<void, std::string> GetLogged();
std::expected<void, std::string> UpdateCoordinates(Coordinates new_coordinates);
}  // namespace LeoGeoUsb

#endif  // LEOGEO_USB_COMM_H_
