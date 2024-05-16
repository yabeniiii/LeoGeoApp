#include "LeoGeo/main.hpp"

#include <QApplication>
#include <memory>

#include "LeoGeo/ui.hpp"

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  std::unique_ptr<QMainWindow> main_window = std::make_unique<QMainWindow>();
  LeoGeoUi::MainWindow main_widget;
  main_window->show();
  main_window->setCentralWidget(&main_widget);
  return app.exec();
}
