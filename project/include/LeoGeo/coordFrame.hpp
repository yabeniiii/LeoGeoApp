#pragma once

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

struct Coordinates;

class CoordFrame;
class CoordSet;

class CoordFrame : public QWidget {
 public:
  explicit CoordFrame(QWidget *parent = nullptr);
  std::string GetCoords();

 private:
  std::unique_ptr<QVBoxLayout> layout_;
  std::unique_ptr<QHBoxLayout> label_layout_;

  std::unique_ptr<QLabel> lat_label_;
  std::unique_ptr<QLabel> long_label_;

  std::unique_ptr<CoordSet> coord_set_1_;
  std::unique_ptr<CoordSet> coord_set_2_;
  std::unique_ptr<CoordSet> coord_set_3_;
};

class CoordSet : public QWidget {
 public:
  explicit CoordSet(QWidget *parent = nullptr);
  Coordinates GetCoordinates();

 private:
  QWidget *parent_;
  std::unique_ptr<QHBoxLayout> layout_;
  std::unique_ptr<QDoubleSpinBox> lat_;
  std::unique_ptr<QDoubleSpinBox> long_;
};
