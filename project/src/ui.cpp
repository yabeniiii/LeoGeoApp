#include "LeoGeo/ui.hpp"

#include <keychain/keychain.h>

#include <QApplication>
#include <QBoxLayout>
#include <QInputDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <expected>
#include <format>
#include <memory>
#include <vector>

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

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
  layout_ = std::make_unique<QBoxLayout>(QBoxLayout::Direction::TopToBottom);
  button_layout_ =
      std::make_unique<QBoxLayout>(QBoxLayout::Direction::TopToBottom);
  button_top_layout_ =
      std::make_unique<QBoxLayout>(QBoxLayout::Direction::LeftToRight);
  button_bottom_layout_ =
      std::make_unique<QBoxLayout>(QBoxLayout::Direction::LeftToRight);
  keychain_error_ = keychain::Error();

  button_layout_->addLayout(button_top_layout_.get());
  button_layout_->addLayout(button_bottom_layout_.get());

  layout_->addLayout(button_layout_.get());
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

  coord_frame_ = std::make_unique<CoordFrame>();
  coord_frame_->hide();

  layout_->addWidget(coord_frame_.get());

  usb_init_button_ = std::make_unique<QPushButton>("Connect", this);
  log_fetch_button_ = std::make_unique<QPushButton>("Fetch Logs", this);
  admin_mode_button_ = std::make_unique<QPushButton>("Admin Mode", this);

  button_top_layout_->addWidget(usb_init_button_.get());
  button_top_layout_->addWidget(log_fetch_button_.get());
  button_top_layout_->addWidget(admin_mode_button_.get());

  exit_admin_button_ = std::make_unique<QPushButton>("Exit Admin", this);
  exit_admin_button_->hide();
  upload_coord_button_ = std::make_unique<QPushButton>("Upload Coords", this);
  upload_coord_button_->hide();
  change_pass_button_ = std::make_unique<QPushButton>("Change Password", this);
  change_pass_button_->hide();

  button_bottom_layout_->addWidget(change_pass_button_.get());
  button_bottom_layout_->addWidget(upload_coord_button_.get());
  button_bottom_layout_->addWidget(exit_admin_button_.get());

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
  temp_view_ = std::make_unique<QChartView>(temp_chart_.get(), this);
  temp_view_->show();
  humid_view_ = std::make_unique<QChartView>(humid_chart_.get(), this);
  humid_view_->show();

  layout_->addWidget(temp_view_.get());
  layout_->addWidget(humid_view_.get());
  layout_->addWidget(coord_frame_.get());

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

  this->setLayout(layout_.get());
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
  QString inputPassword = QInputDialog::getText(
      this, "Administrator", "Enter Administrator Password",
      QLineEdit::EchoMode::Password, "", &ok);

  if (!ok) {
    return;
  }

  if (inputPassword.toStdString() == password_) {
    humid_view_->hide();
    temp_view_->hide();
    admin_mode_button_->setEnabled(false);
    exit_admin_button_->show();
    upload_coord_button_->show();
    change_pass_button_->show();
    coord_frame_->show();
  } else {
    error_message_->showMessage(
        QApplication::translate("error_window", "Incorrect Password"));
  }
}

void MainWindow::ExitAdminButtonHandler() {
  humid_view_->show();
  temp_view_->show();
  admin_mode_button_->setEnabled(true);
  exit_admin_button_->hide();
  upload_coord_button_->hide();
  change_pass_button_->hide();
  coord_frame_->hide();
};

void MainWindow::UploadCoordButtonHandler() {
  const auto status = LeoGeoUsb::UpdateCoordinates(coord_frame_->GetCoords());

  if (!status) {
    error_message_->showMessage(
        QApplication::translate("error_window", status.error().c_str()));
  } else {
    message_->setText(std::format("{}, {}", status.value().end()->latitude,
                                  status.value().end()->longitude)
                          .c_str());
    message_->exec();
  }
};

void MainWindow::ChangePassButtonHandler() {
  bool ok = false;
  std::string tempPassword = "";
  QString inputPassword = "";

  inputPassword =
      QInputDialog::getText(this, "Administrator", "Enter New Password",
                            QLineEdit::EchoMode::Password, "", &ok);

  if (ok) {
    tempPassword = inputPassword.toStdString();
  }

  inputPassword =
      QInputDialog::getText(this, "Administrator", "Re-Enter New Password",
                            QLineEdit::EchoMode::Password, "", &ok);
  if (!ok) {
    return;
  }
  if (inputPassword.toStdString() == tempPassword) {
    password_ = tempPassword;
    keychain::setPassword(kPackage, kService, kUser, password_,
                          keychain_error_);
    if (keychain_error_) {
      error_message_->showMessage(QApplication::translate(
          "error_message",
          std::format("Keychain error: {}", keychain_error_.message).c_str()));
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

CoordFrame::CoordFrame(QWidget *parent) {
  add_coord_set_button_ =
      std::make_unique<QPushButton>("Add Coordinates", this);

  layout_ = std::make_unique<QVBoxLayout>();

  layout_->addWidget(add_coord_set_button_.get());

  connect(add_coord_set_button_.get(), &QPushButton::released, this,
          &CoordFrame::AddCoordSetHandler);

  this->setLayout(layout_.get());
}

void CoordFrame::AddCoordSetHandler() {
  CoordSet *coord_set = new CoordSet(this);  // NOLINT
  layout_->insertWidget(0, coord_set);
  coord_set->show();
}

std::vector<LeoGeoUsb::Coordinates> CoordFrame::GetCoords() {
  std::vector<LeoGeoUsb::Coordinates> coordinates;

  for (int i = 0; i < layout_->count(); i++) {
    coordinates.push_back(dynamic_cast<CoordSet *>(layout_->itemAt(i)->widget())
                              ->GetCoordinates());
  }

  return coordinates;
}

CoordSet::CoordSet(QWidget *parent) : parent_(parent) {
  layout_ = std::make_unique<QHBoxLayout>();
  lat_ = std::make_unique<QDoubleSpinBox>(this);
  lat_->setRange(-90, 90);  // NOLINT
  lat_->setDecimals(10);    // NOLINT
  lat_->show();
  long_ = std::make_unique<QDoubleSpinBox>(this);
  long_->setRange(-180, 180);  // NOLINT
  long_->setDecimals(10);      // NOLINT
  long_->show();

  delete_coord_set_button_ = std::make_unique<QPushButton>("X", this);
  connect(delete_coord_set_button_.get(), &QPushButton::released, this,
          &CoordSet::DeleteCoordSetHandler);

  layout_->addWidget(delete_coord_set_button_.get());
  layout_->addWidget(lat_.get());
  layout_->addWidget(long_.get());

  this->setLayout(layout_.get());
}

void CoordSet::DeleteCoordSetHandler() { delete this; }

LeoGeoUsb::Coordinates CoordSet::GetCoordinates() {
  return LeoGeoUsb::Coordinates{lat_->value(), long_->value()};
}

}  // namespace LeoGeoUi
