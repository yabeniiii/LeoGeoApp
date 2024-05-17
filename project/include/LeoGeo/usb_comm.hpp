#ifndef LEOGEO_USB_COMM_H_
#define LEOGEO_USB_COMM_H_

#include <expected>
#include <string>
#include <vector>

namespace LeoGeoUsb {

struct Coordinates {
  double latitude;
  double longitude;
};

std::expected<void, std::string> UsbStart();
std::expected<void, std::string> GetLogged();
std::expected<std::vector<Coordinates>, std::string> UpdateCoordinates(
    std::vector<Coordinates> new_coordinates);
}  // namespace LeoGeoUsb

#endif  // LEOGEO_USB_COMM_H_
