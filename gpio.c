#include "gpio.h"
#include "pr.h"

void line_settings_free(void *line_settings) {
  pr_debug("Freeing line settings object at address %p\n", line_settings);
  gpiod_line_settings_free(line_settings);
}

void line_config_free(void *line_config) {
  pr_debug("Freeing line config object at address %p\n", line_config);
  gpiod_line_config_free(line_config);
}

void request_config_free(void *request_config) {
  pr_debug("Freeing request config object at address %p\n", request_config);
  gpiod_request_config_free(request_config);
}
