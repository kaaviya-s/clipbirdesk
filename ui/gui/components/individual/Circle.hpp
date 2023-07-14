#pragma once  // Header guard see https://en.wikipedia.org/wiki/Include_guard

// Copyright (c) 2023 Sri Lakshmi Kanthan P
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <QColor>
#include <QPainter>
#include <QWidget>

namespace srilakshmikanthanp::clipbirdesk::ui::gui::components::individual {
class Circle : public QWidget {
 private:          // Member
  int radius = 0;  // radius of the circle
  QColor color;    // color of the circle
 private:          // just for Qt
  Q_OBJECT

 public:
  /**
   * @brief Construct a new Circle object
   * with parent as QWidget
   * @param parent parent object
   */
  explicit Circle(QWidget *parent = nullptr) : QWidget(parent) {
    // TODO: set style sheet
  }

  /**
   * @brief set the Radius of the circle
   */
  void setRadius(int radius) {
    this->radius = radius;
  }

  /**
   * @brief set the Color of the circle
   */
  void setColor(const QColor &color) {
    this->color = color;
  }

  /**
   * @brief get the Radius of the circle
   */
  int getRadius() const {
    return radius;
  }

  /**
   * @brief get the Color of the circle
   */
  QColor getColor() const {
    return color;
  }

  /**
   * @brief paint event
   */
  void paintEvent(QPaintEvent *event) override {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawEllipse(0, 0, radius, radius);
  }

  /**
   * @brief Destroy the Circle object
   * with virtual destructor
   */
  ~Circle() override = default;

 private:  // disable copy and move
  Q_DISABLE_COPY_MOVE(Circle)
};
}  // namespace srilakshmikanthanp::clipbirdesk::ui::gui::components::core