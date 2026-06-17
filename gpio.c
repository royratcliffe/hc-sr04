#include "gpio.h"
#include "pr.h"

int gpio_line_request_read_edge_events(struct gpiod_line_request *request, struct gpiod_edge_event_buffer *buffer, size_t max_events,
                                       int (*yield)(struct gpiod_edge_event *event, void *user_data), void *user_data) {
  int rc = 0;
  const size_t capacity = gpiod_edge_event_buffer_get_capacity(buffer);
  while (max_events > 0) {
    int num_events = gpiod_line_request_read_edge_events(request, buffer, capacity);
    if (num_events < 0) {
      return num_events;
    }
    if (num_events == 0) {
      break;
    }
    for (unsigned long i = 0; i < (unsigned long)num_events; i++) {
      struct gpiod_edge_event *event = gpiod_edge_event_buffer_get_event(buffer, i);
      if (!event) {
        return -1;
      }
      if (yield) {
        rc += yield(event, user_data);
      }
    }
    max_events -= num_events;
  }
  return rc;
}

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
