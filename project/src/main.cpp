// #include "LeoGeo/main.hpp"

#include <QApplication>
#include <QMainWindow>

class MainWindow : public QMainWindow {
 public:
  explicit MainWindow(QWidget *parent = nullptr) {
    this->setWindowTitle(
        QApplication::translate("main_window_title", "LeoGeo"));
    this->show();
  }

 private slots:

 private:
};

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  MainWindow main_window;
  return app.exec();
}
