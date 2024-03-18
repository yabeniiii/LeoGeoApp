#ifndef LEOGEO_UI_H_
#define LEOGEO_UI_H_

#include <QErrorMessage>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QtCharts/QChart>

namespace LeoGeoUi {
class MainWindow : public QMainWindow {
 public:
  explicit MainWindow(QWidget *parent = nullptr);

 private slots:
  void UsbInitButtonHandler();
  void LogFetchButtonHandler();
  void UpdateCoordButtonHandler();

 private:
  std::unique_ptr<QPushButton> usb_init_button_;
  std::unique_ptr<QPushButton> log_fetch_button_;
  std::unique_ptr<QPushButton> update_coord_button_;
  std::unique_ptr<QErrorMessage> error_message_;
  std::unique_ptr<QMessageBox> message_;
  std::unique_ptr<QChart> alt_chartview_;
  std::unique_ptr<QChart> temp_chartview_;
  std::unique_ptr<QChart> humid_chartview_;
};
}  // namespace LeoGeoUi

#endif  // LEOGEO_UI_H_
