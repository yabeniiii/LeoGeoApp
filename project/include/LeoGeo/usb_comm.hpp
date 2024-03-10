#ifndef LEOGEO_USB_COMM_H_
#define LEOGEO_USB_COMM_H_

#include <expected>
#include <string>

namespace LeoGeoUsb {
std::expected<std::string, std::string> UsbStart();
}  // namespace LeoGeoUsb

#endif  // LEOGEO_USB_COMM_H_
