#ifndef LEOGEO_UI_H_
#define LEOGEO_UI_H_

#include <keychain/keychain.h>

#include <QErrorMessage>
#include <QInputDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <memory>

namespace LeoGeoUi {

class GraphView : public QChartView {
 public:
  explicit GraphView(QChart *chart, QWidget *parent = nullptr);
  void locate(const int location_index);
};

class MainWindow : public QMainWindow {
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
  std::unique_ptr<GraphView> temp_view_;
  std::unique_ptr<GraphView> humid_view_;
  std::unique_ptr<QLineSeries> temp_series_;
  std::unique_ptr<QLineSeries> humid_series_;
  std::string password_;
  keychain::Error keychain_error_;
};

}  // namespace LeoGeoUi

#endif  // LEOGEO_UI_H_
