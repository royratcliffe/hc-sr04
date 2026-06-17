#include "gpio.h"
#include "pr.h"

const char *gpio_line_value_to_string(enum gpiod_line_value value) {
  switch (value) {
  case GPIOD_LINE_VALUE_ERROR:
    return "error";
  case GPIOD_LINE_VALUE_INACTIVE:
    return "inactive";
  case GPIOD_LINE_VALUE_ACTIVE:
    return "active";
  default:
    return "unknown";
  }
}

const char *gpio_edge_event_type_to_string(enum gpiod_edge_event_type event_type) {
  switch (event_type) {
  case GPIOD_EDGE_EVENT_RISING_EDGE:
    return "rising";
  case GPIOD_EDGE_EVENT_FALLING_EDGE:
    return "falling";
  default:
    return "unknown";
  }
}

void gpio_line_settings_free(void *line_settings) {
  pr_debug("Freeing line settings object at address %p\n", line_settings);
  gpiod_line_settings_free(line_settings);
}

void gpio_line_config_free(void *line_config) {
  pr_debug("Freeing line config object at address %p\n", line_config);
  gpiod_line_config_free(line_config);
}

void gpio_request_config_free(void *request_config) {
  pr_debug("Freeing request config object at address %p\n", request_config);
  gpiod_request_config_free(request_config);
}

void gpio_edge_event_buffer_free(void *buffer) {
  pr_debug("Freeing edge event buffer object at address %p\n", buffer);
  gpiod_edge_event_buffer_free(buffer);
}
