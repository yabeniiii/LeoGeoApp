#ifndef LEOGEO_MAIN_WINDOW_H_
#define LEOGEO_MAIN_WINDOW_H_

#include <QMainWindow>
#include <QPushButton>

namespace LeoGeoUi {
class MainWindow : public QMainWindow {
 public:
  explicit MainWindow(QWidget *parent = nullptr);

 private slots:
  void UsbInitButtonHandler();

 private:
  std::unique_ptr<QPushButton> usb_init_button;
};
}  // namespace LeoGeoUi

#endif  // LEOGEO_MAIN_WINDOW_H_
