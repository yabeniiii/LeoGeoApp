// #define USBOFF

#include "LeoGeo/ui.hpp"

#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QtCharts/QChart>
#include <cstddef>
#include <expected>
#include <format>
#include <type_traits>

#include "LeoGeo/usb_comm.hpp"

namespace LeoGeoUi {

namespace {
constexpr std::size_t button_height = 50;
constexpr std::size_t button_width = 200;
constexpr std::size_t button_pos = 300;

constexpr std::size_t chart_height = 500;
constexpr std::size_t chart_width = 500;
constexpr std::size_t chart_pos = 600;
}  // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  error_message_ = std::make_unique<QErrorMessage>(this);
  message_ = std::make_unique<QMessageBox>(this);

  alt_chartview_ = std::make_unique<QChart>();
  temp_chartview_ = std::make_unique<QChart>();
  humid_chartview_ = std::make_unique<QChart>();

  alt_chartview_->setTitle(
      QApplication::translate("alt_chart_title", "Altitude"));
  temp_chartview_->setTitle(
      QApplication::translate("temp_chart_title", "Temperature"));
  humid_chartview_->setTitle(
      QApplication::translate("humid_chart_title", "Humidity"));

  alt_chartview_->setGeometry(0, (chart_pos * 0), chart_width, chart_height);
  temp_chartview_->setGeometry(0, (chart_pos * 1), chart_width, chart_height);
  humid_chartview_->setGeometry(0, (chart_pos * 2), chart_width, chart_height);

  usb_init_button_ = std::make_unique<QPushButton>("Connect", this);
  log_fetch_button_ = std::make_unique<QPushButton>("Fetch Logs", this);
  update_coord_button_ = std::make_unique<QPushButton>("Update Coords", this);

  usb_init_button_->setGeometry((button_pos * 0), 0, button_width,
                                button_height);
  log_fetch_button_->setGeometry((button_pos * 1), 0, button_width,
                                 button_height);
  update_coord_button_->setGeometry((button_pos * 2), 0, button_width,
                                    button_height);

  connect(usb_init_button_.get(), &QPushButton::released, this,
          &MainWindow::UsbInitButtonHandler);
  connect(log_fetch_button_.get(), &QPushButton::released, this,
          &MainWindow::LogFetchButtonHandler);
  connect(update_coord_button_.get(), &QPushButton::released, this,
          &MainWindow::UpdateCoordButtonHandler);

  this->setWindowTitle(QApplication::translate("main_window_title", "LeoGeo"));
  this->show();
}

void MainWindow::UsbInitButtonHandler() {
#ifdef USBOFF
  error_message_->showMessage(
      QApplication::translate("error_window", "Usb unimplemented"));
  QApplication::beep();
#else
  if (const auto status = LeoGeoUsb::UsbStart(); !status) {
    error_message_->showMessage(
        QApplication::translate("error_window", status.error().c_str()));
  }
#endif
}

void MainWindow::LogFetchButtonHandler() {
#ifdef USBOFF
  error_message_->showMessage(
      QApplication::translate("error_window", "Usb unimplemented"));
  QApplication::beep();
#else
#endif
}
void MainWindow::UpdateCoordButtonHandler() {
#ifdef USBOFF
  error_message_->showMessage(
      QApplication::translate("error_window", "Usb unimplemented"));
  QApplication::beep();
#else
#endif
}
}  // namespace LeoGeoUi
