/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026, Roy Ratcliffe, Northumberland, United Kingdom
 */

/*!
 * \file gpio_line.h
 * \brief Header file for GPIO line functions.
 */
#ifndef GPIO_LINE_H
#define GPIO_LINE_H

#include <gpiod.h>

/*!
 * \brief Represents a GPIO line.
 *
 * \details This structure contains the information needed to interact with a
 * specific GPIO line, including the chip it belongs to and its offset within
 * the chip.
 */
struct gpio_line {
  const char *name;
  struct gpiod_chip *chip;
  struct gpiod_line_settings *line_settings;
  struct gpiod_line_config *line_config;
  struct gpiod_request_config *request_config;
  struct gpiod_line_request *line_request;
  unsigned int offset;
};

/*!
 * \brief Add the line offset for a GPIO line by its name.
 * \param line Pointer to a gpio_line structure to be filled with the chip and offset information.
 * \param chips Array of gpiod_chip pointers representing the available GPIO chips.
 * \param num_chips The number of GPIO chips in the chips array.
 * \return The line offset if found, or -1 if the line with the specified name is not found in any of the provided chips.
 */
int gpio_line_add_offset_from_name(struct gpio_line *line, struct gpiod_chip **chips, size_t num_chips);

struct gpiod_line_settings *gpio_line_settings(struct gpio_line *line);        /*!< Access or create the line settings for a GPIO line */
struct gpiod_line_config *gpio_line_config(struct gpio_line *line);            /*!< Access or create the line config for a GPIO line */
struct gpiod_request_config *gpio_line_request_config(struct gpio_line *line); /*!< Access or create the request config for a GPIO line */
struct gpiod_line_request *gpio_line_request(struct gpio_line *line);          /*!< Access or create the line request for a GPIO line */

/*!
 * \brief Set the direction of a GPIO line.
 * \param line Pointer to a gpio_line structure representing the GPIO line to configure.
 * \param direction The direction to set for the GPIO line (input or output).
 * \return 0 on success, -1 on failure.
 */
int gpio_line_set_direction(struct gpio_line *line, enum gpiod_line_direction direction);

/*!
 * \brief Set the edge detection of a GPIO line.
 * \param line Pointer to a gpio_line structure representing the GPIO line to configure.
 * \param edge The edge detection to set for the GPIO line (none, rising, falling, or both).
 * \return 0 on success, -1 on failure.
 */
int gpio_line_set_edge_detection(struct gpio_line *line, enum gpiod_line_edge edge);

/*!
 * \brief Set the consumer label for a GPIO line.
 * \param line Pointer to a gpio_line structure representing the GPIO line to configure.
 * \param consumer The consumer label to set for the GPIO line.
 * \return 0 on success, -1 on failure.
 */
int gpio_line_set_consumer(struct gpio_line *line, const char *consumer);

/*!
 * \brief Set the value of a GPIO line.
 * \param line Pointer to a gpio_line structure representing the GPIO line to configure.
 * \param value The value to set for the GPIO line (active or inactive).
 * \return 0 on success, -1 on failure.
 */
int gpio_line_set_value(struct gpio_line *line, enum gpiod_line_value value);

/*!
 * \brief Get the value of a GPIO line.
 * \param line Pointer to a gpio_line structure representing the GPIO line to read from.
 * \return The value of the GPIO line (active, inactive, or error).
 */
enum gpiod_line_value gpio_line_get_value(struct gpio_line *line);

#endif /* GPIO_LINE_H */
