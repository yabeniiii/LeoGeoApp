#include "LeoGeo/mainWindow.hpp"

#include <keychain/keychain.h>

#include <QApplication>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QDateTime>
#include <QDateTimeAxis>
#include <QDir>
#include <QDoubleSpinBox>
#include <QErrorMessage>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QVBoxLayout>
#include <QValueAxis>
#include <QWebEngineView>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "LeoGeo/../../../.env"

namespace {
// clang-format off
//NOLINTNEXTLINE
constexpr char html[] =
    R"(
      <!DOCTYPE html>
      <html> 
        <head>
          <title>Logged Data Map</title>
          <script src='https://maps.googleapis.com/maps/api/js?key=AIzaSyDAHENFKMG6TTIBHi3AfdpGlnx-U4V5FNI&callback=initMap' async defer></script>
          <script>
            let map;
            function initMap() {{
              map = new google.maps.Map(document.getElementById('map'), {{ zoom: 12, center: {{lat: 51.98, lng: 5.91}} }});
              {}
            }}
          </script>
        </head>
        <body>
          <div id='map' style='height: 800px; width: 100%;'></div>
        </body>
      </html>
    )";
// clang-format on
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
  error_message_ = make_unique<QErrorMessage>(this);
  message_ = make_unique<QMessageBox>(this);

  keychain_error_ = keychain::Error();
  password_ = keychain::getPassword(kPackage, kService, kUser, keychain_error_);

  usb_init_button_ = make_unique<QPushButton>("Connect", this);
  log_fetch_button_ = make_unique<QPushButton>("Fetch Logs", this);
  admin_mode_button_ = make_unique<QPushButton>("Admin Mode", this);
  switch_data_view_button_ = make_unique<QPushButton>("Show Route", this);
  switch_data_view_button_->setEnabled(false);

  button_top_layout_->addWidget(usb_init_button_.get());
  button_top_layout_->addWidget(log_fetch_button_.get());
  button_top_layout_->addWidget(admin_mode_button_.get());
  button_top_layout_->addWidget(switch_data_view_button_.get());

  exit_admin_button_ = make_unique<QPushButton>("Exit Admin", this);
  upload_coord_button_ = make_unique<QPushButton>("Upload Coords", this);
  change_pass_button_ = make_unique<QPushButton>("Change Password", this);
  unlock_button_ = make_unique<QPushButton>("Unlock Box", this);

  button_bottom_layout_->addWidget(exit_admin_button_.get());
  button_bottom_layout_->addWidget(change_pass_button_.get());
  button_bottom_layout_->addWidget(upload_coord_button_.get());
  button_bottom_layout_->addWidget(unlock_button_.get());

  exit_admin_button_->hide();
  change_pass_button_->hide();
  upload_coord_button_->hide();
  unlock_button_->hide();

  // chart chartview contains and displays chart, chart contains and displays
  // lineseries, lineseries contains values
  temp_chart_ = make_unique<QChart>();
  temp_series_ = make_unique<QLineSeries>();
  temp_chart_->legend()->hide();
  temp_chart_->setTitle("Temperature");
  temp_chart_->addSeries(temp_series_.get());
  axis_x_ = std::make_unique<QDateTimeAxis>();
  axis_x_->setFormat("dd.MM hh:mm");
  axis_y_ = std::make_unique<QValueAxis>();
  axis_y_->setLabelFormat("%i");
  axis_y_->setTitleText("Temperature");
  temp_view_ = make_unique<QChartView>(temp_chart_.get(), this);
  temp_view_->show();
  temp_chart_->addAxis(axis_x_.get(), Qt::AlignBottom);
  temp_chart_->addAxis(axis_y_.get(), Qt::AlignLeft);

  map_view_ = std::make_unique<QWebEngineView>(this);
  map_view_->hide();

  // contains the input boxes and buttons for coordinates, hidden until admin
  // mode
  coord_frame_ = make_unique<CoordFrame>();
  coord_frame_->hide();

  temp_view_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  map_view_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  layout_->addWidget(temp_view_.get());
  layout_->addWidget(map_view_.get());
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
  connect(unlock_button_.get(), &QPushButton::clicked, this,
          &MainWindow::UnlockButtonHandler);
  connect(switch_data_view_button_.get(), &QPushButton::clicked, this,
          &MainWindow::DataSwitchButtonHandler);

  this->setLayout(layout_.get());
  this->setWindowTitle(tr("LeoGeo"));
  this->show();
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
  bool good_port_name = false;
  foreach (auto &port, QSerialPortInfo::availablePorts()) {
    if (port.portName().toStdString() == port_name_) good_port_name = true;
  }
  if (!good_port_name) {
    error_message_->showMessage(
        tr("Error: invalid or no port name specified. Please select a port "
           "using the connect button"));
    return;
  }
  QSerialPort serial_port;

  // configuring serial port, 9600-8-N-1
  serial_port.setPortName(tr(port_name_.c_str()));
  serial_port.setParity(QSerialPort::NoParity);
  serial_port.setBaudRate(QSerialPort::Baud9600);
  serial_port.setDataBits(QSerialPort::Data8);
  serial_port.setStopBits(QSerialPort::OneStop);
  serial_port.setFlowControl(QSerialPort::SoftwareControl);

  if (!serial_port.open(QIODevice::ReadWrite)) {
    UartErrorHandler(serial_port.error());
    return;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(500));  // NOLINT
  // sending the device a '!' tells it to answer with its data log
  if (serial_port.write("!") == -1) {
    error_message_->showMessage(tr("Device write error"));
  }
  if (!serial_port.waitForBytesWritten()) {
    UartErrorHandler(serial_port.error());
    return;
  }

  QByteArray data_bytes;
  serial_port.waitForReadyRead(2000);  // NOLINT
  data_bytes.append(serial_port.readAll());
  while (serial_port.waitForReadyRead(500)) {  // NOLINT
    data_bytes.append(serial_port.readAll());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));  // NOLINT
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));  // NOLINT
  data_bytes.append(serial_port.readAll());
  serial_port.close();

  std::string data_string = data_bytes.toStdString();
  if (data_string.empty()) {
    error_message_->showMessage(tr("Received no data"));
    return;
  }

  ParseData(data_string);
  BuildChart();
  BuildWebView();

  switch_data_view_button_->setEnabled(true);
  this->show();
}

void MainWindow::AdminModeButtonHandler() {
  if (keychain_error_.type == keychain::ErrorType::NotFound) {
    message_->setText(
        "Administrator password not found in system keychain. Using default "
        "password, please set "
        "custom password as soon as possible.");

    message_->exec();
    password_ = "LeoGeo2024";
  } else if (keychain_error_) {
    error_message_->showMessage(
        tr(std::format("Keychain error: {}", keychain_error_.message).c_str()));

    QApplication::quit();
  }
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
    map_view_->hide();
    admin_mode_button_->setEnabled(false);
    switch_data_view_button_->setEnabled(false);
    exit_admin_button_->show();
    upload_coord_button_->show();
    change_pass_button_->show();
    unlock_button_->show();
    coord_frame_->show();
  } else {
    error_message_->showMessage(tr("error_window", "Incorrect Password"));
  }
}

void MainWindow::ExitAdminButtonHandler() {
  temp_view_->show();
  admin_mode_button_->setEnabled(true);
  switch_data_view_button_->setEnabled(true);
  exit_admin_button_->hide();
  upload_coord_button_->hide();
  change_pass_button_->hide();
  unlock_button_->hide();
  coord_frame_->hide();
};

void MainWindow::UploadCoordButtonHandler() {
  bool good_port_name = false;
  foreach (auto &port, QSerialPortInfo::availablePorts()) {
    if (port.portName().toStdString() == port_name_) good_port_name = true;
  }
  if (!good_port_name) {
    error_message_->showMessage(
        tr("Error: invalid or no port name specified. Please select a port "
           "using the connect button"));
    return;
  }
  QSerialPort serial_port;

  // configuring serial port, 9600-8-N-1
  serial_port.setPortName(tr(port_name_.c_str()));
  serial_port.setParity(QSerialPort::NoParity);
  serial_port.setBaudRate(QSerialPort::Baud9600);  // NOLINT
  serial_port.setDataBits(QSerialPort::Data8);
  serial_port.setStopBits(QSerialPort::OneStop);
  serial_port.setFlowControl(QSerialPort::SoftwareControl);

  if (!serial_port.open(QIODevice::ReadWrite)) {
    UartErrorHandler(serial_port.error());
    return;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(500));  // NOLINT
  // sending the device a '@' tells it get ready to receive new coordinates
  if (serial_port.write("@") == -1) {
    error_message_->showMessage(tr("Device write error"));
  }
  if (!serial_port.waitForBytesWritten()) {
    UartErrorHandler(serial_port.error());
    return;
  }

  if (serial_port.write(coord_frame_->GetCoords().c_str()) == -1) {
    error_message_->showMessage(tr("Device write error"));
  }
  if (!serial_port.waitForBytesWritten()) {
    UartErrorHandler(serial_port.error());
    return;
  }

  if (serial_port.write("\n\r") == -1) {
    error_message_->showMessage(tr("Device write error"));
  }
  if (!serial_port.waitForBytesWritten()) {
    UartErrorHandler(serial_port.error());
    return;
  }

  QByteArray data_bytes;
  int loop_num = 0;
  while (data_bytes.toStdString() != "Da") {
    if (loop_num >= 50) {  // NOLINT
      break;
    }
    serial_port.waitForReadyRead(1000);          // NOLINT
    while (serial_port.waitForReadyRead(100)) {  // NOLINT
      data_bytes.append(serial_port.readAll());
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));  // NOLINT
    loop_num++;
  }
  serial_port.close();
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

void MainWindow::UnlockButtonHandler() {
  bool good_port_name = false;
  foreach (auto &port, QSerialPortInfo::availablePorts()) {
    if (port.portName().toStdString() == port_name_) good_port_name = true;
  }
  if (!good_port_name) {
    error_message_->showMessage(
        tr("Error: invalid or no port name specified. Please select a port "
           "using the connect button"));
    return;
  }
  QSerialPort serial_port;

  // configuring serial port, 9600-8-N-1
  serial_port.setPortName(tr(port_name_.c_str()));
  serial_port.setParity(QSerialPort::NoParity);
  serial_port.setBaudRate(QSerialPort::Baud9600);  // NOLINT
  serial_port.setDataBits(QSerialPort::Data8);
  serial_port.setStopBits(QSerialPort::OneStop);
  serial_port.setFlowControl(QSerialPort::SoftwareControl);

  if (!serial_port.open(QIODevice::ReadWrite)) {
    UartErrorHandler(serial_port.error());
    return;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(500));  // NOLINT
  // sending the device a '%' tells it to unlock the box
  if (serial_port.write("%") == -1) {
    error_message_->showMessage(tr("Device write error"));
  }
  if (!serial_port.waitForBytesWritten()) {
    UartErrorHandler(serial_port.error());
    return;
  }
  QByteArray data_bytes;
  int loop_num = 0;
  while (data_bytes.toStdString() != "Da") {
    if (loop_num >= 50) {  // NOLINT
      break;
    }
    serial_port.waitForReadyRead(1000);          // NOLINT
    while (serial_port.waitForReadyRead(100)) {  // NOLINT
      data_bytes.append(serial_port.readAll());
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));  // NOLINT
    loop_num++;
  }
  serial_port.close();
}

void MainWindow::DataSwitchButtonHandler() {
  if (map_view_->isVisible()) {
    map_view_->hide();
    temp_view_->show();
    switch_data_view_button_->setText("Show Route");
  } else {
    map_view_->show();
    temp_view_->hide();
    switch_data_view_button_->setText("Show Temperature");
  }
}

void MainWindow::UartErrorHandler(QSerialPort::SerialPortError error) {
  std::string error_name;
  switch (error) {
    case QSerialPort::NoError:
      error_name = "Timed Out (or NoError)";
      break;
    case QSerialPort::DeviceNotFoundError:
      error_name = "DeviceNotFoundError";
      break;
    case QSerialPort::PermissionError:
      error_name = "PermissionError";
      break;
    case QSerialPort::OpenError:
      error_name = "OpenError";
      break;
    case QSerialPort::NotOpenError:
      error_name = "NotOpenError";
      break;
    case QSerialPort::WriteError:
      error_name = "WriteError";
      break;
    case QSerialPort::ReadError:
      error_name = "ReadError";
      break;
    case QSerialPort::ResourceError:
      error_name = "ResourceError";
      break;
    case QSerialPort::UnsupportedOperationError:
      error_name = "UnsupportedOperationError";
      break;
    case QSerialPort::TimeoutError:
      error_name = "TimeoutError";
      break;
    case QSerialPort::UnknownError:
      error_name = "UnknownError";
      break;
  }
  error_message_->showMessage(
      tr(std::format("Failed to open serial port: {}, with error: {}",
                     port_name_, error_name)
             .c_str()));
}

void MainWindow::ParseData(std::string data_string) {
  QFile file(QDir::homePath() + "/LeoGeoData.csv");
  if (file.exists()) file.remove();
  if (!file.open(QIODevice::ReadWrite)) {
    error_message_->showMessage(tr("Error: could not create data file"));
    return;
  }
  QTextStream stream(&file);
  stream << "date, time, latitude, longitude, temperature" << Qt::endl;
  while (!data_string.empty()) {
    stream << data_string.substr(0, data_string.find_first_of(";")).c_str()
           << Qt::endl;
    std::string date = data_string.substr(0, data_string.find_first_of(","));
    data_string.erase(0, data_string.find_first_of(",") + 1);
    std::string time = data_string.substr(0, data_string.find_first_of(","));
    if (time.length() == 5) time = "0" + time;  // NOLINT
    data_string.erase(0, data_string.find_first_of(",") + 1);
    QDateTime datetime =
        QDateTime::fromString((date + time).c_str(), "ddMMyyhhmmss")
            .addYears(100);  // NOLINT

    double latitude =
        std::stod(data_string.substr(0, data_string.find_first_of(",")));
    data_string.erase(0, data_string.find_first_of(",") + 1);

    double longitude =
        std::stod(data_string.substr(0, data_string.find_first_of(",")));
    data_string.erase(0, data_string.find_first_of(",") + 1);

    double temperature =
        std::stod(data_string.substr(0, data_string.find_first_of(";")));
    data_string.erase(0, data_string.find_first_of(";") + 1);

    log_vector_.push_back(
        LogData{Coordinates{latitude, longitude}, temperature, datetime});
  }

  file.close();
}

void MainWindow::BuildWebView() {
  std::string markers_js;
  foreach (auto &logdata, log_vector_) {
    markers_js += std::format(
        "new google.maps.Marker({{ position: {{lat: {}, lng: {}}}, map: map "
        "}});\n",
        logdata.coordinates.latitude, logdata.coordinates.longitude);
  }

  auto request_html = std::format(html, markers_js);
  map_view_->setHtml(request_html.c_str());
}

void MainWindow::BuildChart() {
  // i've found removing the lineseries and putting back again is the best way
  // to get the chart to update with new values
  temp_chart_->removeSeries(temp_series_.get());

  temp_series_ = std::make_unique<QLineSeries>();
  foreach (auto datapoint, log_vector_) {
    // NOLINTNEXTLINE
    temp_series_->append(datapoint.datetime.toMSecsSinceEpoch(),
                         datapoint.temperature);
  }

  temp_chart_->addSeries(temp_series_.get());
  temp_series_->attachAxis(axis_x_.get());
  temp_series_->attachAxis(axis_y_.get());
  temp_chart_->legend()->hide();

  temp_view_->update();
}
