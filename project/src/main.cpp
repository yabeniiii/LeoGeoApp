#include "LeoGeo/main.hpp"

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>

#include "LeoGeo/main_window.hpp"

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  LeoGeoUi::MainWindow main_window;
  return app.exec();
}
