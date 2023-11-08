// Copyright (c) 2023 Sri Lakshmi Kanthan P
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "error.hpp"

namespace srilakshmikanthanp::clipbirdesk::ui::gui::modals {
/**
 * @brief Construct a new Modal object
 *
 * @param parent
 */
Error::Error(QWidget *parent) : Modal(parent) {
  // Set the errorMessage label to center
  this->errorMessage->setAlignment(Qt::AlignCenter);

  // Create a layout
  auto layout = new QVBoxLayout(this);

  // Add the errorMessage label to the layout
  layout->addWidget(this->errorMessage);

  // Set the layout
  this->setLayout(layout);
}

/**
 * @brief set the error message
 */
void Error::setErrorMessage(const QString &) {
  this->errorMessage->setText(this->getErrorMessage());
  this->update();
}

/**
 * @brief get the error message
 */
QString Error::getErrorMessage() const {
  return this->errorMessage->text();
}
}  // namespace srilakshmikanthanp::clipbirdesk::ui::gui::modals