#include "LeoGeo/ui.hpp"

#include <keychain/keychain.h>

#include <QApplication>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QDateTime>
#include <QDateTimeAxis>
#include <QDoubleSpinBox>
#include <QErrorMessage>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QVBoxLayout>
#include <QValueAxis>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <memory>

namespace LeoGeoUi {

namespace {
// details used to store password in system keychain
const std::string kPackage = "com.LeoGeo.LeoGeo";
const std::string kService = "LeoGeo";
const std::string kUser = "Admin";
}  // namespace

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
  using std::make_unique;

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
  temp_series_ = make_unique<QLineSeries>();
  temp_chart_->legend()->hide();
  temp_chart_->setTitle("Temperature");
  temp_chart_->addSeries(temp_series_.get());
  axis_x_ = std::make_unique<QDateTimeAxis>();
  axis_y_ = std::make_unique<QValueAxis>();
  axis_x_->setFormat("dd.MM hh:mm");
  temp_view_ = make_unique<QChartView>(temp_chart_.get(), this);
  temp_view_->show();

  map_container_ = std::make_unique<MapContainer>(this);

  layout_->addWidget(temp_view_.get());
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
}

void MainWindow::UsbInitButtonHandler() {
  QStringList items;
  foreach (auto &port, QSerialPortInfo::availablePorts()) {
    items << port.portName();
  }

  bool ok = false;
  port_name_ = QInputDialog::getItem(
                   this, tr("Serial Port"),
                   tr("Choose the serial port you wish to connect with"), items)
                   .toStdString();
}

void MainWindow::LogFetchButtonHandler() {
  QSerialPort serial_port;

  // configuring serial port, 9600-8-N-1
  serial_port.setPortName(tr(port_name_.c_str()));
  serial_port.setParity(QSerialPort::NoParity);
  serial_port.setBaudRate(QSerialPort::Baud9600);  // NOLINT
  serial_port.setDataBits(QSerialPort::Data8);
  serial_port.setStopBits(QSerialPort::OneStop);
  serial_port.setFlowControl(QSerialPort::NoFlowControl);

  serial_port.open(QIODevice::ReadWrite);
  // we can guess that once it's ready to send data, it's also ready to receive
  serial_port.waitForReadyRead();
  // sending the device a '!' tells it to answer with its data log
  serial_port.write("!");
  serial_port.waitForBytesWritten();

  QByteArray data_bytes;
  while (serial_port.waitForReadyRead(100))  // NOLINT
    data_bytes.append(serial_port.readAll());

  std::string data_string = data_bytes.toStdString();

  std::vector<LogData> data_vec;
  while (!data_string.empty()) {
    qDebug("%s", data_string.c_str());
    std::string date = data_string.substr(0, data_string.find_first_of(","));
    if (date == "0") {
      date = "000000";
    }
    int date_d = stoi(date.substr(0, 1));
    int date_m = stoi(date.substr(2, 3));
    int date_y = stoi(date.substr(4, 5));  // NOLINT
    data_string.erase(0, data_string.find_first_of(",") + 1);
    qDebug("date: %s", date.c_str());

    std::string time = data_string.substr(0, data_string.find_first_of(","));
    if (time == "0") {
      time = "000000";
    }
    int time_h = stoi(time.substr(0, 1));
    int time_m = stoi(time.substr(2, 3));
    int time_s = stoi(time.substr(4, 5));  // NOLINT
    data_string.erase(0, data_string.find_first_of(",") + 1);
    qDebug("time: %s", time.c_str());

    double latitude =
        std::stod(data_string.substr(0, data_string.find_first_of(",")));
    data_string.erase(0, data_string.find_first_of(",") + 1);
    qDebug("lat: %f", latitude);

    double longitude =
        std::stod(data_string.substr(0, data_string.find_first_of(",")));
    data_string.erase(0, data_string.find_first_of(",") + 1);
    qDebug("long: %f", longitude);

    // double temperature =
    //     std::stod(data_string.substr(0, data_string.find_first_of(",")));
    data_string.erase(0, data_string.find_first_of(";") + 1);

    data_vec.push_back(LogData{Coordinates{latitude, longitude}, 0,
                               QDateTime(QDate(date_y, date_m, date_d),
                                         QTime(time_h, time_m, time_s))});
  }

  // i've found removing the lineseries and putting back again is the best way
  // to get the chart to update with new values
  temp_chart_->removeSeries(temp_series_.get());

  temp_series_->clear();
  for (auto datapoint : data_vec) {
    // NOLINTNEXTLINE
    temp_series_->append(datapoint.datetime.toMSecsSinceEpoch(),
                         datapoint.temperature);
  }

  temp_chart_->addSeries(temp_series_.get());
  temp_chart_->legend()->hide();

  temp_chart_->addAxis(axis_x_.get(), Qt::AlignBottom);
  temp_series_->attachAxis(axis_x_.get());
  temp_chart_->addAxis(axis_y_.get(), Qt::AlignBottom);
  temp_series_->attachAxis(axis_y_.get());

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
  temp_view_->show();
  admin_mode_button_->setEnabled(true);
  exit_admin_button_->hide();
  upload_coord_button_->hide();
  change_pass_button_->hide();
  coord_frame_->hide();
};

void MainWindow::UploadCoordButtonHandler(){};

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
  coord_set_1_ = std::make_unique<CoordSet>();
  coord_set_2_ = std::make_unique<CoordSet>();
  coord_set_3_ = std::make_unique<CoordSet>();
}

std::vector<Coordinates> CoordFrame::GetCoords() {
  // loops through all the numbers entered into whatever number of boxes,
  // returns them as a vector of Coordinates

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

  layout_->addWidget(lat_.get());
  layout_->addWidget(long_.get());

  this->setLayout(layout_.get());
}

Coordinates CoordSet::GetCoordinates() {
  // very sophisticated function, bet you can't guess what it does

  return Coordinates{lat_->value(), long_->value()};
}

MapContainer::MapContainer(QWidget *parent) : QWidget(parent) {}

}  // namespace LeoGeoUi
