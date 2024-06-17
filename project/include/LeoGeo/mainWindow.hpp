#pragma once

#include <keychain/keychain.h>

#include <QBoxLayout>
#include <QButtonGroup>
#include <QDateTime>
#include <QDateTimeAxis>
#include <QDoubleSpinBox>
#include <QErrorMessage>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QSerialPort>
#include <QVBoxLayout>
#include <QValueAxis>
#include <QWebEngineView>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <memory>
#include <vector>

#include "LeoGeo/coordFrame.hpp"

struct Coordinates {
  double latitude;
  double longitude;
};

struct LogData {
  Coordinates coordinates;
  double temperature;
  QDateTime datetime;
};

class MainWindow;

class MainWindow : public QWidget {
 public:
  explicit MainWindow(QWidget *parent = nullptr);

 private slots:
  void UsbInitButtonHandler();
  void LogFetchButtonHandler();
  void AdminModeButtonHandler();
  void ExitAdminButtonHandler();
  void UploadCoordButtonHandler();
  void ChangePassButtonHandler();
  void UnlockButtonHandler();
  void DataSwitchButtonHandler();

 private:
  void UartErrorHandler(QSerialPort::SerialPortError error);
  void ParseData(std::string data_string);
  void BuildWebView();
  void BuildChart();

  std::string port_name_;
  std::vector<LogData> log_vector_;
  std::string password_;
  keychain::Error keychain_error_;

  std::unique_ptr<QPushButton> usb_init_button_;
  std::unique_ptr<QPushButton> log_fetch_button_;
  std::unique_ptr<QPushButton> admin_mode_button_;
  std::unique_ptr<QPushButton> upload_coord_button_;
  std::unique_ptr<QPushButton> exit_admin_button_;
  std::unique_ptr<QPushButton> change_pass_button_;
  std::unique_ptr<QPushButton> unlock_button_;
  std::unique_ptr<QPushButton> switch_data_view_button_;

  std::unique_ptr<QErrorMessage> error_message_;
  std::unique_ptr<QMessageBox> message_;

  std::unique_ptr<QWebEngineView> map_view_;
  std::unique_ptr<QChartView> temp_view_;
  std::unique_ptr<QChart> temp_chart_;
  std::unique_ptr<QLineSeries> temp_series_;
  std::unique_ptr<QDateTimeAxis> axis_x_;
  std::unique_ptr<QValueAxis> axis_y_;

  std::unique_ptr<CoordFrame> coord_frame_;
  std::unique_ptr<QBoxLayout> layout_;
  std::unique_ptr<QBoxLayout> button_layout_;
  std::unique_ptr<QBoxLayout> button_top_layout_;
  std::unique_ptr<QBoxLayout> button_bottom_layout_;
};
