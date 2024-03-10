// #define USBOFF

#include "LeoGeo/ui.hpp"

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <expected>
#include <format>
#include <iostream>
#include <memory>
#include <string>

#include "LeoGeo/usb_comm.hpp"

namespace LeoGeoUi {
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  usb_init_button_ = std::make_unique<QPushButton>("Usb Init", this);
  error_message_ = std::make_unique<QErrorMessage>(this);

  connect(usb_init_button_.get(), &QPushButton::released, this,
          &MainWindow::UsbInitButtonHandler);

  this->setWindowTitle(QApplication::translate("main_window_title", "LeoGeo"));
  this->show();
}

void LeoGeoUi::MainWindow::UsbInitButtonHandler() {
#ifdef USBOFF
  error_message_->showMessage(
      QApplication::translate("error_window", "Usb unimplemented"));
  QApplication::beep();
#else
  if (const auto status = LeoGeoUsb::UsbStart(); !status.has_value()) {
    error_message_->showMessage(
        QApplication::translate("error_window", status.error().c_str()));
  }
#endif
}
}  // namespace LeoGeoUi
