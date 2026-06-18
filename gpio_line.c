/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026, Roy Ratcliffe, Northumberland, United Kingdom
 */

/*!
 * \file gpio_line.c
 * \brief Implementation of GPIO line functions.
 */
#include "gpio_line.h"

int gpio_line_add_offset_from_name(struct gpio_line *line, struct gpiod_chip **chips, size_t num_chips) {
  for (size_t i = 0; i < num_chips; i++) {
    int offset = gpiod_chip_get_line_offset_from_name(chips[i], line->name);
    if (offset >= 0) {
      line->chip = chips[i];
      line->offset = (unsigned int)offset;
      return offset;
    }
  }
  return -1;
}

struct gpiod_line_settings *gpio_line_settings(struct gpio_line *line) {
  if (!line->line_settings) {
    line->line_settings = gpiod_line_settings_new();
  }
  return line->line_settings;
}

struct gpiod_line_config *gpio_line_config(struct gpio_line *line) {
  if (!line->line_config) {
    /*
     * Fail if the line settings are not set, since these are required to create
     * a line config. If the line settings are missing, return NULL to indicate
     * that the line config cannot be created. This ensures that the line config
     * is only created when the necessary line settings are available,
     * preventing potential issues with incomplete configurations or null
     * pointer dereferences when trying to use the line config without valid
     * line settings.
     */
    if (!line->line_settings) {
      return NULL;
    }
    struct gpiod_line_config *line_config = gpiod_line_config_new();
    if (!line_config) {
      return NULL;
    }
    if (gpiod_line_config_add_line_settings(line_config, &line->offset, 1, line->line_settings) < 0) {
      gpiod_line_config_free(line_config);
      return NULL;
    }
    line->line_config = line_config;
  }
  return line->line_config;
}

struct gpiod_request_config *gpio_line_request_config(struct gpio_line *line) {
  if (!line->request_config) {
    line->request_config = gpiod_request_config_new();
  }
  return line->request_config;
}

struct gpiod_line_request *gpio_line_request(struct gpio_line *line) {
  if (!line->line_request) {
    /*
     * Fail if the chip, request config, or line config is not set, since these
     * are required to create a line request. If any of these are missing,
     * return NULL to indicate that the line request cannot be created.
     */
    if (!line->chip || !line->request_config || !line->line_config) {
      return NULL;
    }
    line->line_request = gpiod_chip_request_lines(line->chip, line->request_config, line->line_config);
  }
  return line->line_request;
}

int gpio_line_set_direction(struct gpio_line *line, enum gpiod_line_direction direction) {
  struct gpiod_line_settings *line_settings = gpio_line_settings(line);
  if (!line_settings) {
    return -1;
  }
  return gpiod_line_settings_set_direction(line_settings, direction);
}

int gpio_line_set_edge_detection(struct gpio_line *line, enum gpiod_line_edge edge) {
  struct gpiod_line_settings *line_settings = gpio_line_settings(line);
  if (!line_settings) {
    return -1;
  }
  return gpiod_line_settings_set_edge_detection(line_settings, edge);
}

int gpio_line_set_consumer(struct gpio_line *line, const char *consumer) {
  struct gpiod_request_config *request_config = gpio_line_request_config(line);
  if (!request_config) {
    return -1;
  }
  gpiod_request_config_set_consumer(request_config, consumer);
  return 0;
}

int gpio_line_set_value(struct gpio_line *line, enum gpiod_line_value value) {
  struct gpiod_line_request *line_request = gpio_line_request(line);
  if (!line_request) {
    return -1;
  }
  return gpiod_line_request_set_value(line_request, line->offset, value);
}

enum gpiod_line_value gpio_line_get_value(struct gpio_line *line) {
  struct gpiod_line_request *line_request = gpio_line_request(line);
  if (!line_request) {
    return GPIOD_LINE_VALUE_ERROR;
  }
  return gpiod_line_request_get_value(line_request, line->offset);
}
