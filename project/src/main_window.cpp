#include "LeoGeo/main_window.hpp"

#include <libusb-1.0/libusb.h>

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>

LeoGeoUi::MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  this->setWindowTitle(QApplication::translate("main_window_title", "LeoGeo"));
  this->show();

  usb_init_button = std::make_unique<QPushButton>("Usb Init Button", this);

  connect(usb_init_button.get(), &QPushButton::released, this,
          &MainWindow::UsbInitButtonHandler);
}

void LeoGeoUi::MainWindow::UsbInitButtonHandler() {
  libusb_init_context(NULL, NULL, 0);
  QApplication::beep();
  libusb_exit(NULL);
}
