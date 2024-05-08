#ifndef LEOGEO_UI_H_
#define LEOGEO_UI_H_

#include <QErrorMessage>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <cstddef>
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
  void UpdateCoordButtonHandler();

 private:
  std::unique_ptr<QPushButton> usb_init_button_;
  std::unique_ptr<QPushButton> log_fetch_button_;
  std::unique_ptr<QPushButton> update_coord_button_;
  std::unique_ptr<QErrorMessage> error_message_;
  std::unique_ptr<QMessageBox> message_;
  std::unique_ptr<QChart> alt_chart_;
  std::unique_ptr<QChart> temp_chart_;
  std::unique_ptr<QChart> humid_chart_;
  std::unique_ptr<GraphView> alt_view_;
  std::unique_ptr<GraphView> temp_view_;
  std::unique_ptr<GraphView> humid_view_;
  std::unique_ptr<QLineSeries> alt_series_;
  std::unique_ptr<QLineSeries> temp_series_;
  std::unique_ptr<QLineSeries> humid_series_;
};

}  // namespace LeoGeoUi

#endif  // LEOGEO_UI_H_
