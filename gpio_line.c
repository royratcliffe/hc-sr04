#include "gpio_line.h"
#include "gpio.h"

struct gpiod_line_settings *gpio_line_settings(struct gpio_line *line) {
  if (!line->line_settings) {
    line->line_settings = gpiod_line_settings_new();
  }
  return line->line_settings;
}

struct gpiod_line_config *gpio_line_config(struct gpio_line *line) {
  if (!line->line_config) {
    line->line_config = gpiod_line_config_new();
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
    /*
     * If a line request already exists, release it before creating a new one to
     * prevent resource leaks. This ensures that any existing line request is
     * properly cleaned up before a new one is created, avoiding potential
     * issues with dangling pointers or open file descriptors.
     */
    if (line->line_request) {
      gpiod_line_request_release(line->line_request);
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
