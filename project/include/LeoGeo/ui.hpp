#ifndef LEOGEO_UI_H_
#define LEOGEO_UI_H_

#include <keychain/keychain.h>

#include <QBoxLayout>
#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QErrorMessage>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <memory>

#include "LeoGeo/usb_comm.hpp"

namespace LeoGeoUi {

class MainWindow;
class CoordFrame;
class CoordSet;

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

 private:
  std::unique_ptr<LeoGeoUsb::UartReceiver> uart_receiver_;
  std::unique_ptr<LeoGeoUsb::UartSender> uart_sender_;

  std::string password_;
  std::string serial_port_;
  keychain::Error keychain_error_;

  std::unique_ptr<QPushButton> usb_init_button_;
  std::unique_ptr<QPushButton> log_fetch_button_;
  std::unique_ptr<QPushButton> admin_mode_button_;
  std::unique_ptr<QPushButton> upload_coord_button_;
  std::unique_ptr<QPushButton> exit_admin_button_;
  std::unique_ptr<QPushButton> change_pass_button_;

  std::unique_ptr<QErrorMessage> error_message_;
  std::unique_ptr<QMessageBox> message_;

  std::unique_ptr<QChart> temp_chart_;
  std::unique_ptr<QChart> humid_chart_;
  std::unique_ptr<QChartView> temp_view_;
  std::unique_ptr<QChartView> humid_view_;
  std::unique_ptr<QLineSeries> temp_series_;
  std::unique_ptr<QLineSeries> humid_series_;

  std::unique_ptr<CoordFrame> coord_frame_;

  std::unique_ptr<QBoxLayout> layout_;
  std::unique_ptr<QBoxLayout> button_layout_;
  std::unique_ptr<QBoxLayout> button_top_layout_;
  std::unique_ptr<QBoxLayout> button_bottom_layout_;
};

class CoordFrame : public QWidget {
 public:
  explicit CoordFrame(QWidget *parent = nullptr);
  std::vector<LeoGeoUsb::Coordinates> GetCoords();

 private slots:
  void AddCoordSetHandler();

 private:
  std::unique_ptr<QPushButton> add_coord_set_button_;
  std::shared_ptr<QVBoxLayout> layout_;
};

class CoordSet : public QWidget {
 public:
  explicit CoordSet(QWidget *parent = nullptr);
  LeoGeoUsb::Coordinates GetCoordinates();

 private slots:
  void DeleteCoordSetHandler();

 private:
  QWidget *parent_;
  std::unique_ptr<QHBoxLayout> layout_;
  std::unique_ptr<QPushButton> delete_coord_set_button_;
  std::unique_ptr<QDoubleSpinBox> lat_;
  std::unique_ptr<QDoubleSpinBox> long_;
};

}  // namespace LeoGeoUi

#endif  // LEOGEO_UI_H_
