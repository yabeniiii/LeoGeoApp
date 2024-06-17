#include "LeoGeo/coordFrame.hpp"

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

#include "LeoGeo/mainWindow.hpp"

CoordFrame::CoordFrame(QWidget *parent) {
  layout_ = std::make_unique<QVBoxLayout>();
  label_layout_ = std::make_unique<QHBoxLayout>();
  lat_label_ = std::make_unique<QLabel>("Latitude:");
  long_label_ = std::make_unique<QLabel>("Longitude:");
  label_layout_->addWidget(lat_label_.get());
  label_layout_->addWidget(long_label_.get());
  layout_->addLayout(label_layout_.get());
  coord_set_1_ = std::make_unique<CoordSet>();
  layout_->addWidget(coord_set_1_.get());
  coord_set_2_ = std::make_unique<CoordSet>();
  layout_->addWidget(coord_set_2_.get());
  coord_set_3_ = std::make_unique<CoordSet>();
  layout_->addWidget(coord_set_3_.get());
  this->setLayout(layout_.get());
}

std::string CoordFrame::GetCoords() {
  return std::format("{},{},{},{},{},{}",
                     coord_set_1_->GetCoordinates().latitude,
                     coord_set_1_->GetCoordinates().longitude,
                     coord_set_2_->GetCoordinates().latitude,
                     coord_set_2_->GetCoordinates().longitude,
                     coord_set_3_->GetCoordinates().latitude,
                     coord_set_3_->GetCoordinates().longitude);
}

CoordSet::CoordSet(QWidget *parent) : parent_(parent) {
  // each widget of this class contains a button for deleting itself, and two
  // number entry boxes, one for longitude and latitude. a button in the outer
  // layout allows the user to spawn as many of these as they want (or at least
  // until their screen runs out of space, haven't implemented scrolling)
  using std::make_unique;

  layout_ = make_unique<QHBoxLayout>();
  lat_ = make_unique<QDoubleSpinBox>(this);
  // limiting the possible input values to numbers that are actually valid
  // global coordinates
  // also setting to up to ten decimals of precision, probably more than
  // neccessary, definitely more than the gps module can provide, but costs us
  // nothing with the range of values we're expecting
  lat_->setRange(-90, 90);  // NOLINT
  lat_->setDecimals(10);    // NOLINT
  lat_->show();
  long_ = make_unique<QDoubleSpinBox>(this);
  long_->setRange(-180, 180);  // NOLINT
  long_->setDecimals(10);      // NOLINT
  long_->show();

  layout_->addWidget(lat_.get());
  layout_->addWidget(long_.get());

  this->setLayout(layout_.get());
}

Coordinates CoordSet::GetCoordinates() {
  // very sophisticated function, bet you can't guess what it does
  return Coordinates{lat_->value(), long_->value()};
}
