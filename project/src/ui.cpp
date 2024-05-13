// #define USBOFF

#include "LeoGeo/ui.hpp"

#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <expected>
#include <memory>

#include "LeoGeo/usb_comm.hpp"

namespace LeoGeoUi {

namespace {
constexpr int kPadding = 20;

constexpr int kButton_height = 50;
constexpr int kButton_width = 200;
constexpr int kButton_pos = 300;

constexpr int kChart_height = 200;
constexpr int kChart_width = 800;
constexpr int kChart_pos = 220;
constexpr int kChart_top_padding = 90;
}  // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  error_message_ = std::make_unique<QErrorMessage>(this);
  message_ = std::make_unique<QMessageBox>(this);

  usb_init_button_ = std::make_unique<QPushButton>("Connect", this);
  log_fetch_button_ = std::make_unique<QPushButton>("Fetch Logs", this);
  update_coord_button_ = std::make_unique<QPushButton>("Update Coords", this);

  alt_chart_ = std::make_unique<QChart>();
  temp_chart_ = std::make_unique<QChart>();
  humid_chart_ = std::make_unique<QChart>();
  alt_series_ = std::make_unique<QLineSeries>();
  temp_series_ = std::make_unique<QLineSeries>();
  humid_series_ = std::make_unique<QLineSeries>();
  alt_chart_->legend()->hide();
  temp_chart_->legend()->hide();
  humid_chart_->legend()->hide();
  alt_chart_->setTitle("Altitude");
  temp_chart_->setTitle("Temperature");
  humid_chart_->setTitle("Humidity");
  temp_chart_->createDefaultAxes();
  humid_chart_->createDefaultAxes();
  alt_chart_->addSeries(alt_series_.get());
  temp_chart_->addSeries(temp_series_.get());
  humid_chart_->addSeries(humid_series_.get());
  alt_view_ = std::make_unique<GraphView>(alt_chart_.get(), this);
  alt_view_->locate(0);
  temp_view_ = std::make_unique<GraphView>(temp_chart_.get(), this);
  temp_view_->locate(1);
  humid_view_ = std::make_unique<GraphView>(humid_chart_.get(), this);
  humid_view_->locate(2);

  usb_init_button_->setGeometry((kButton_pos * 0) + kPadding, kPadding,
                                kButton_width, kButton_height);
  log_fetch_button_->setGeometry((kButton_pos * 1) + kPadding, kPadding,
                                 kButton_width, kButton_height);
  update_coord_button_->setGeometry((kButton_pos * 2) + kPadding, kPadding,
                                    kButton_width, kButton_height);

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
  if (const auto status = LeoGeoUsb::UsbStart(); !status) {
    error_message_->showMessage(
        QApplication::translate("error_window", status.error().c_str()));
  }
}

void MainWindow::LogFetchButtonHandler() {
  alt_chart_->removeSeries(alt_series_.get());
  alt_series_->append(0, 500);   // NOLINT (magic number for test purposes)
  alt_series_->append(2, 750);   // NOLINT (magic number for test purposes)
  alt_series_->append(3, 1000);  // NOLINT (magic number for test purposes)
  alt_series_->append(4, 1100);  // NOLINT (magic number for test purposes)
  alt_chart_->addSeries(alt_series_.get());
  alt_chart_->createDefaultAxes();
  alt_view_->update();
}

void MainWindow::UpdateCoordButtonHandler() {}

GraphView::GraphView(QChart *chart, QWidget *parent)
    : QChartView(chart, parent) {
  this->setRenderHint(QPainter::Antialiasing);
  this->show();
}

void GraphView::locate(const int location_index) {
  this->setGeometry(kPadding,
                    (kChart_pos * location_index) + kChart_top_padding,
                    kChart_width, kChart_height);
}
// test

}  // namespace LeoGeoUi
