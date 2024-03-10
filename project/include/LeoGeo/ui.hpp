#ifndef LEOGEO_UI_H_
#define LEOGEO_UI_H_

#include <QErrorMessage>
#include <QMainWindow>
#include <QPushButton>
#include <memory>

namespace LeoGeoUi {
class MainWindow : public QMainWindow {
 public:
  explicit MainWindow(QWidget *parent = nullptr);

 private slots:
  void UsbInitButtonHandler();

 private:
  std::unique_ptr<QPushButton> usb_init_button_;
  std::unique_ptr<QErrorMessage> error_message_;
};
}  // namespace LeoGeoUi

#endif  // LEOGEO_UI_H_
