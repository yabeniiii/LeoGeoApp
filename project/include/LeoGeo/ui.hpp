#ifndef LEOGEO_UI_H_
#define LEOGEO_UI_H_

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

namespace LeoGeoUi {

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
class CoordFrame;
class CoordSet;
class MapContainer;

class MainWindow : public QWidget {
 public:
  explicit MainWindow(QWidget *parent = nullptr);
  std::vector<LogData> LogVector();

 private slots:
  void UsbInitButtonHandler();
  void LogFetchButtonHandler();
  void AdminModeButtonHandler();
  void ExitAdminButtonHandler();
  void UploadCoordButtonHandler();
  void ChangePassButtonHandler();
  void UnlockButtonHandler();

 private:
  void UartErrorHandler(QSerialPort::SerialPortError error);
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

  std::unique_ptr<QErrorMessage> error_message_;
  std::unique_ptr<QMessageBox> message_;

  std::unique_ptr<QChart> temp_chart_;
  std::unique_ptr<QChartView> temp_view_;
  std::unique_ptr<QLineSeries> temp_series_;
  std::unique_ptr<QDateTimeAxis> axis_x_;
  std::unique_ptr<QValueAxis> axis_y_;
  std::unique_ptr<MapContainer> map_container_;

  std::unique_ptr<CoordFrame> coord_frame_;

  std::unique_ptr<QBoxLayout> layout_;
  std::unique_ptr<QBoxLayout> button_layout_;
  std::unique_ptr<QBoxLayout> button_top_layout_;
  std::unique_ptr<QBoxLayout> button_bottom_layout_;
};

class CoordFrame : public QWidget {
 public:
  explicit CoordFrame(QWidget *parent = nullptr);
  std::string GetCoords();

 private:
  std::unique_ptr<QVBoxLayout> layout_;
  std::unique_ptr<QHBoxLayout> label_layout_;

  std::unique_ptr<QLabel> lat_label_;
  std::unique_ptr<QLabel> long_label_;

  std::unique_ptr<CoordSet> coord_set_1_;
  std::unique_ptr<CoordSet> coord_set_2_;
  std::unique_ptr<CoordSet> coord_set_3_;
};

class CoordSet : public QWidget {
 public:
  explicit CoordSet(QWidget *parent = nullptr);
  Coordinates GetCoordinates();

 private slots:
  void DeleteCoordSetHandler();

 private:
  QWidget *parent_;
  std::unique_ptr<QHBoxLayout> layout_;
  std::unique_ptr<QPushButton> delete_coord_set_button_;
  std::unique_ptr<QDoubleSpinBox> lat_;
  std::unique_ptr<QDoubleSpinBox> long_;
};

class MapContainer : public QWidget {
 public:
  explicit MapContainer(MainWindow *parent = nullptr);
  void UpdateMapImage();

 private:
  std::unique_ptr<QWebEngineView> map_image_;
};

}  // namespace LeoGeoUi

#endif  // LEOGEO_UI_H_
