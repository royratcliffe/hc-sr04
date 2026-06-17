#include "gpio.h"
#include "pr.h"

void gpio_edge_event_generator_init(struct gpio_edge_event_generator *generator, struct gpiod_line_request *request, struct gpiod_edge_event_buffer *buffer,
                                    size_t max_events) {
  generator->request = request;
  generator->buffer = buffer;
  generator->max_events = max_events;
  generator->num_events = 0;
  generator->index = 0;
}

int gpio_edge_event_generator_next(struct gpio_edge_event_generator *generator, struct gpiod_edge_event **event) {
  if (!generator || !event) {
    return -1;
  }
  if (generator->index >= generator->num_events) {
    if (generator->max_events == 0) {
      return 0;
    }
    const size_t capacity = gpiod_edge_event_buffer_get_capacity(generator->buffer);
    const size_t max_events = generator->max_events < capacity ? generator->max_events : capacity;
    if (max_events == 0) {
      return 0;
    }
    generator->num_events = gpiod_line_request_read_edge_events(generator->request, generator->buffer, max_events);
    if (generator->num_events <= 0) {
      return (int)generator->num_events;
    }
    generator->index = 0;
    generator->max_events -= (size_t)generator->num_events;
  }
  *event = gpiod_edge_event_buffer_get_event(generator->buffer, (size_t)generator->index);
  if (!*event) {
    return -1;
  }
  generator->index++;
  return 1;
}

int gpio_line_request_read_edge_events(struct gpiod_line_request *request, struct gpiod_edge_event_buffer *buffer, size_t max_events,
                                       int (*yield)(struct gpiod_edge_event *event, void *user_data), void *user_data) {
  int rc = 0;
  struct gpio_edge_event_generator generator;
  gpio_edge_event_generator_init(&generator, request, buffer, max_events);
  for (;;) {
    struct gpiod_edge_event *event = NULL;
    int num_events = gpio_edge_event_generator_next(&generator, &event);
    if (num_events < 0) {
      return num_events;
    }
    if (num_events == 0) {
      break;
    }
    if (yield) {
      rc += yield(event, user_data);
    }
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
