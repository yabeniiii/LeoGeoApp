#include "LeoGeo/ui.hpp"

#include <keychain/keychain.h>

#include <QApplication>
#include <QInputDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <expected>
#include <format>
#include <memory>

#include "LeoGeo/usb_comm.hpp"

namespace LeoGeoUi {

namespace {
constexpr int kPadding = 20;

constexpr int kButton_height = 50;
constexpr int kButton_width = 200;

constexpr int kChart_height = 200;
constexpr int kChart_width = 640;
constexpr int kChart_pos = 220;
constexpr int kChart_top_padding = 90;

const std::string kPackage = "com.LeoGeo.LeoGeo";
const std::string kService = "LeoGeo";
const std::string kUser = "Admin";
}  // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  keychain_error_ = keychain::Error();

  error_message_ = std::make_unique<QErrorMessage>(this);
  message_ = std::make_unique<QMessageBox>(this);

  password_ = keychain::getPassword(kPackage, kService, kUser, keychain_error_);
  if (keychain_error_.type == keychain::ErrorType::NotFound) {
    message_->setText(
        "Administrator password not found in system keychain. Using default "
        "password, please set "
        "custom password as soon as possible.");
    message_->exec();
    password_ = "LeoGeo2024";
  } else if (keychain_error_) {
    error_message_->showMessage(QApplication::translate(
        "error_message",
        std::format("Keychain error: {}", keychain_error_.message).c_str()));
    QApplication::quit();
  }

  usb_init_button_ = std::make_unique<QPushButton>("Connect", this);
  log_fetch_button_ = std::make_unique<QPushButton>("Fetch Logs", this);
  admin_mode_button_ = std::make_unique<QPushButton>("Admin Mode", this);

  exit_admin_button_ = std::make_unique<QPushButton>("Exit Admin", this);
  exit_admin_button_->hide();
  upload_coord_button_ = std::make_unique<QPushButton>("Upload Coords", this);
  upload_coord_button_->hide();
  change_pass_button_ = std::make_unique<QPushButton>("Change Password", this);
  change_pass_button_->hide();

  temp_chart_ = std::make_unique<QChart>();
  humid_chart_ = std::make_unique<QChart>();
  temp_series_ = std::make_unique<QLineSeries>();
  humid_series_ = std::make_unique<QLineSeries>();
  temp_chart_->legend()->hide();
  humid_chart_->legend()->hide();
  temp_chart_->setTitle("Temperature");
  humid_chart_->setTitle("Humidity");
  temp_chart_->createDefaultAxes();
  humid_chart_->createDefaultAxes();
  temp_chart_->addSeries(temp_series_.get());
  humid_chart_->addSeries(humid_series_.get());
  temp_view_ = std::make_unique<GraphView>(temp_chart_.get(), this);
  temp_view_->locate(0);
  humid_view_ = std::make_unique<GraphView>(humid_chart_.get(), this);
  humid_view_->locate(1);

  usb_init_button_->setGeometry((kPadding * 1) + (kButton_width * 0), kPadding,
                                kButton_width, kButton_height);
  log_fetch_button_->setGeometry((kPadding * 2) + (kButton_width * 1), kPadding,
                                 kButton_width, kButton_height);
  admin_mode_button_->setGeometry((kPadding * 3) + (kButton_width * 2),
                                  kPadding, kButton_width, kButton_height);

  exit_admin_button_->setGeometry((kPadding * 1) + (kButton_width * 0),
                                  (kPadding * 2) + kButton_height,
                                  kButton_width, kButton_height);
  upload_coord_button_->setGeometry((kPadding * 2) + (kButton_width * 1),
                                    (kPadding * 2) + kButton_height,
                                    kButton_width, kButton_height);
  change_pass_button_->setGeometry((kPadding * 3) + (kButton_width * 2),
                                   (kPadding * 2) + kButton_height,
                                   kButton_width, kButton_height);

  connect(usb_init_button_.get(), &QPushButton::released, this,
          &MainWindow::UsbInitButtonHandler);
  connect(log_fetch_button_.get(), &QPushButton::released, this,
          &MainWindow::LogFetchButtonHandler);
  connect(admin_mode_button_.get(), &QPushButton::released, this,
          &MainWindow::AdminModeButtonHandler);
  connect(upload_coord_button_.get(), &QPushButton::released, this,
          &MainWindow::UploadCoordButtonHandler);
  connect(exit_admin_button_.get(), &QPushButton::released, this,
          &MainWindow::ExitAdminButtonHandler);
  connect(change_pass_button_.get(), &QPushButton::released, this,
          &MainWindow::ChangePassButtonHandler);

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
  temp_chart_->removeSeries(temp_series_.get());

  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  // magic numbers for testing graph
  temp_series_->append(0, 500);
  temp_series_->append(2, 750);
  temp_series_->append(3, 1000);
  temp_series_->append(4, 1100);
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

  temp_chart_->addSeries(temp_series_.get());
  temp_chart_->createDefaultAxes();
  temp_view_->update();
}

void MainWindow::AdminModeButtonHandler() {
  bool ok = false;
  QString inputPassword = QInputDialog::getText(this, "Administrator",
                                                "Enter Administrator Password",
                                                QLineEdit::Normal, "", &ok);
  if (ok) {
    if (inputPassword.toStdString() == password_) {
      humid_view_->hide();
      temp_view_->hide();
      admin_mode_button_->setEnabled(false);
      exit_admin_button_->show();
      upload_coord_button_->show();
      change_pass_button_->show();
    } else {
      error_message_->showMessage(
          QApplication::translate("error_window", "Incorrect Password"));
    }
  }
}

void MainWindow::ExitAdminButtonHandler() {
  humid_view_->show();
  temp_view_->show();
  admin_mode_button_->setEnabled(true);
  exit_admin_button_->hide();
  upload_coord_button_->hide();
  change_pass_button_->hide();
};

void MainWindow::UploadCoordButtonHandler(){};

void MainWindow::ChangePassButtonHandler() {
  bool ok = false;
  std::string tempPassword = "";
  QString inputPassword = "";
  inputPassword = QInputDialog::getText(
      this, "Administrator", "Enter New Password", QLineEdit::Normal, "", &ok);
  if (ok) {
    tempPassword = inputPassword.toStdString();
  }

  inputPassword =
      QInputDialog::getText(this, "Administrator", "Re-Enter New Password",
                            QLineEdit::Normal, "", &ok);
  if (ok) {
    if (inputPassword.toStdString() == tempPassword) {
      password_ = tempPassword;
      keychain::setPassword(kPackage, kService, kUser, password_,
                            keychain_error_);
      if (keychain_error_) {
        error_message_->showMessage(QApplication::translate(
            "error_message",
            std::format("Keychain error: {}", keychain_error_.message)
                .c_str()));
        QApplication::quit();
      }
      message_->setText("Password Changed");
      message_->exec();
    } else {
      error_message_->showMessage(QApplication::translate(
          "error_message",
          "Passwords do not match. Password has not been changed."));
    }
  }
};

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

}  // namespace LeoGeoUi
