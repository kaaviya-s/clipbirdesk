#pragma once  // Header guard see https://en.wikipedia.org/wiki/Include_guard

// Copyright (c) 2023 Sri Lakshmi Kanthan P
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// Qt header
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStyleHints>

// project header
#include "constants/constants.hpp"

namespace srilakshmikanthanp::clipbirdesk::ui::gui::modals {
class Joiner : public QDialog {
 private:  // disable copy and move for this class

  Q_DISABLE_COPY_MOVE(Joiner)

 private:  // just for Qt

  Q_OBJECT

 signals:   // Signals

  void onConnect(const QString &ipv4, const QString &port);

 public:

 /**
  * @brief Construct a new Abstract object
  *
  * @param parent
  */
  explicit Joiner(QWidget * parent = nullptr);

  /**
   * @brief Destroy the Status object
   */
  virtual ~Joiner() = default;
};
}  // namespace srilakshmikanthanp::clipbirdesk::ui::gui::modals