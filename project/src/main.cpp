#include "LeoGeo/main.hpp"

#include <QApplication>

#include "LeoGeo/ui.hpp"

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  LeoGeoUi::MainWindow main_window;
  return app.exec();
}
