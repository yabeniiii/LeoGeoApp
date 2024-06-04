#include "LeoGeo/ui.hpp"

#include <keychain/keychain.h>

#include <QApplication>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QErrorMessage>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QSerialPort>
#include <QStringList>
#include <QVBoxLayout>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtSerialPort/QSerialPortInfo>
#include <memory>
#include <string>
#include <vector>

#include "LeoGeo/usb_comm.hpp"

namespace LeoGeoUi {

namespace {
// details used to store password in system keychain
const std::string kPackage = "com.LeoGeo.LeoGeo";
const std::string kService = "LeoGeo";
const std::string kUser = "Admin";
}  // namespace

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
  using std::make_unique;

  uart_sender_ = make_unique<LeoGeoUsb::UartSender>(this);
  uart_receiver_ = make_unique<LeoGeoUsb::UartReceiver>(this);

  // two horizontal layouts within a vertical layout, i didn't know grid layout
  // is a thing, will have to try
  layout_ = make_unique<QBoxLayout>(QBoxLayout::TopToBottom);
  button_layout_ = make_unique<QBoxLayout>(QBoxLayout::TopToBottom);
  button_top_layout_ = make_unique<QBoxLayout>(QBoxLayout::LeftToRight);
  button_bottom_layout_ = make_unique<QBoxLayout>(QBoxLayout::LeftToRight);

  button_layout_->addLayout(button_top_layout_.get());
  button_layout_->addLayout(button_bottom_layout_.get());

  layout_->addLayout(button_layout_.get());

  // variable is passed into each keychain function, can
  // then be used to know which particular error
  // occurred (if any)
  keychain_error_ = keychain::Error();
  error_message_ = make_unique<QErrorMessage>(this);
  message_ = make_unique<QMessageBox>(this);

  password_ = keychain::getPassword(kPackage, kService, kUser, keychain_error_);

  if (keychain_error_.type == keychain::ErrorType::NotFound) {
    message_->setText(
        "Administrator password not found in system keychain. Using default "
        "password, please set "
        "custom password as soon as possible.");

    message_->exec();
    password_ = "LeoGeo2024";
  } else if (keychain_error_) {
    error_message_->showMessage(
        tr("error_message",
           std::format("Keychain error: {}", keychain_error_.message).c_str()));

    QApplication::quit();
  }

  // contains the input boxes and buttons for coordinates, hidden until admin
  // mode
  coord_frame_ = make_unique<CoordFrame>();
  coord_frame_->hide();

  layout_->addWidget(coord_frame_.get());

  usb_init_button_ = make_unique<QPushButton>("Connect", this);
  log_fetch_button_ = make_unique<QPushButton>("Fetch Logs", this);
  admin_mode_button_ = make_unique<QPushButton>("Admin Mode", this);

  button_top_layout_->addWidget(usb_init_button_.get());
  button_top_layout_->addWidget(log_fetch_button_.get());
  button_top_layout_->addWidget(admin_mode_button_.get());

  exit_admin_button_ = make_unique<QPushButton>("Exit Admin", this);
  upload_coord_button_ = make_unique<QPushButton>("Upload Coords", this);
  change_pass_button_ = make_unique<QPushButton>("Change Password", this);

  button_bottom_layout_->addWidget(exit_admin_button_.get());
  button_bottom_layout_->addWidget(change_pass_button_.get());
  button_bottom_layout_->addWidget(upload_coord_button_.get());

  exit_admin_button_->hide();
  change_pass_button_->hide();
  upload_coord_button_->hide();

  // chart chartview contains and displays chart, chart contains and displays
  // lineseries, lineseries contains values
  temp_chart_ = make_unique<QChart>();
  humid_chart_ = make_unique<QChart>();
  temp_series_ = make_unique<QLineSeries>();
  humid_series_ = make_unique<QLineSeries>();
  temp_chart_->legend()->hide();
  humid_chart_->legend()->hide();
  humid_chart_->setTitle("Humidity");
  temp_chart_->setTitle("Temperature");
  temp_chart_->createDefaultAxes();
  humid_chart_->createDefaultAxes();
  temp_chart_->addSeries(temp_series_.get());
  humid_chart_->addSeries(humid_series_.get());
  temp_view_ = make_unique<QChartView>(temp_chart_.get(), this);
  humid_view_ = make_unique<QChartView>(humid_chart_.get(), this);
  temp_view_->show();
  humid_view_->show();
  layout_->addWidget(temp_view_.get());
  layout_->addWidget(humid_view_.get());

  layout_->addWidget(coord_frame_.get());

  // assosciating clicking buttons with their corresponding functions.
  connect(usb_init_button_.get(), &QPushButton::clicked, this,
          &MainWindow::UsbInitButtonHandler);
  connect(log_fetch_button_.get(), &QPushButton::clicked, this,
          &MainWindow::LogFetchButtonHandler);
  connect(admin_mode_button_.get(), &QPushButton::clicked, this,
          &MainWindow::AdminModeButtonHandler);
  connect(upload_coord_button_.get(), &QPushButton::clicked, this,
          &MainWindow::UploadCoordButtonHandler);
  connect(exit_admin_button_.get(), &QPushButton::clicked, this,
          &MainWindow::ExitAdminButtonHandler);
  connect(change_pass_button_.get(), &QPushButton::clicked, this,
          &MainWindow::ChangePassButtonHandler);

  this->setLayout(layout_.get());
  this->setWindowTitle(tr("main_window_title", "LeoGeo"));
  this->show();
}

void MainWindow::UsbInitButtonHandler() {
  QStringList items;
  foreach (auto &port, QSerialPortInfo::availablePorts()) {
    items << port.portName();
  }

  bool ok = false;
  auto selected = QInputDialog::getItem(
      this, tr("Serial Port"),
      tr("Choose the serial port you wish to connect with"), items);

  if (!ok) return;

  QSerialPort serial;

  serial.close();
  serial.setPortName(selected);
  serial.setDataBits(QSerialPort::DataBits::Data8);
  serial.setParity(QSerialPort::Parity::NoParity);
  serial.setStopBits(QSerialPort::StopBits::OneStop);
  serial.setBaudRate(9600);  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
  serial.setFlowControl(QSerialPort::FlowControl::SoftwareControl);

  if (!serial.open(QIODevice::ReadWrite)) {
    error_message_->showMessage(
        tr("Can't open %1, error code %2").arg(selected).arg(serial.error()));
    return;
  }

  QByteArray dataByteArray(selected.toStdString().c_str(),
                           selected.length());  // NOLINT
  serial.write(dataByteArray);
}

void MainWindow::LogFetchButtonHandler() {
  // i've found removing the lineseries and putting back again is the best way
  // to get the chart to update with new values
  temp_chart_->removeSeries(temp_series_.get());

  temp_series_->clear();
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
  bool ok = false;  // returns true once user presses ok button in popup
  QString inputPassword = QInputDialog::getText(
      this, "Administrator", "Enter Administrator Password",
      QLineEdit::EchoMode::Password, "", &ok);

  // only happens if the user presses cancel, in which case we obviously want to
  // return without actually doing anything
  if (!ok) {
    return;
  }

  if (inputPassword.toStdString() == password_) {
    // just hides both charts and shows all the admin-only buttons, easiest way
    // to do it
    humid_view_->hide();
    temp_view_->hide();
    admin_mode_button_->setEnabled(false);
    exit_admin_button_->show();
    upload_coord_button_->show();
    change_pass_button_->show();
    coord_frame_->show();
  } else {
    error_message_->showMessage(tr("error_window", "Incorrect Password"));
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
  uart_receiver_->Receive(serial_port_);
};

void MainWindow::ChangePassButtonHandler() {
  // really simple, spawn popup, save input to temporary variable, spawn another
  // popup, compare with the temp, if it's a match store it to the main password
  // variable, and save to system keychain
  bool ok = false;

  const auto tempPassword =
      QInputDialog::getText(this, "Administrator", "Enter New Password",
                            QLineEdit::EchoMode::Password, "", &ok)
          .toStdString();

  if (!ok) {
    return;
  }

  const auto inputPassword =
      QInputDialog::getText(this, "Administrator", "Re-Enter New Password",
                            QLineEdit::EchoMode::Password, "", &ok)
          .toStdString();

  if (!ok) {
    return;
  }

  if (inputPassword == tempPassword) {
    password_ = tempPassword;
    keychain::setPassword(kPackage, kService, kUser, password_,
                          keychain_error_);
    if (keychain_error_) {
      error_message_->showMessage(tr(
          "error_message",
          std::format("Keychain error: {}", keychain_error_.message).c_str()));
      QApplication::quit();
    }
    message_->setText("Password Changed");
    message_->exec();
  } else {
    error_message_->showMessage(
        tr("error_message",
           "Passwords do not match. Password has not been changed."));
  }
};

CoordFrame::CoordFrame(QWidget *parent) {
  using std::make_unique;

  add_coord_set_button_ = make_unique<QPushButton>("Add Coordinate Set", this);

  layout_ = make_unique<QVBoxLayout>();
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
  // loops through all the numbers entered into whatever number of boxes,
  // returns them as a vector of LeoGeoUsb::Coordinates
  using LeoGeoUsb::Coordinates;

  std::vector<Coordinates> coordinates;

  for (int i = 0; i < layout_->count(); i++) {
    coordinates.push_back(dynamic_cast<CoordSet *>(layout_->itemAt(i)->widget())
                              ->GetCoordinates());
  }

  return coordinates;
}

CoordSet::CoordSet(QWidget *parent) : parent_(parent) {
  // each widget of this class contains a button for deleting itself, and two
  // number entry boxes, one for longitude and latitude. a button in the outer
  // layout allows the user to spawn as many of these as they want (or at least
  // until their screen runs out of space, haven't implemented scrolling)
  using std::make_unique;

  layout_ = make_unique<QHBoxLayout>();
  lat_ = make_unique<QDoubleSpinBox>(this);
  // limiting the possible input values to numbers that are actually valid
  // global coordinates
  // also setting to up to ten decimals of precision, probably more than
  // neccessary, definitely more than the gps module can provide, but costs us
  // nothing with the range of values we're expecting
  lat_->setRange(-90, 90);  // NOLINT
  lat_->setDecimals(10);    // NOLINT
  lat_->show();
  long_ = make_unique<QDoubleSpinBox>(this);
  long_->setRange(-180, 180);  // NOLINT
  long_->setDecimals(10);      // NOLINT
  long_->show();

  delete_coord_set_button_ = make_unique<QPushButton>("Delete Set", this);
  connect(delete_coord_set_button_.get(), &QPushButton::released, this,
          &CoordSet::DeleteCoordSetHandler);

  layout_->addWidget(delete_coord_set_button_.get());
  layout_->addWidget(lat_.get());
  layout_->addWidget(long_.get());

  this->setLayout(layout_.get());
}

void CoordSet::DeleteCoordSetHandler() {
  // i'm sure there's a way to do this without needing to make it its own
  // function, but i don't know what that is and i can't be bothered to find out
  delete this;
}

LeoGeoUsb::Coordinates CoordSet::GetCoordinates() {
  // very sophisticated function, bet you can't guess what it does
  using LeoGeoUsb::Coordinates;

  return Coordinates{lat_->value(), long_->value()};
}

}  // namespace LeoGeoUi
