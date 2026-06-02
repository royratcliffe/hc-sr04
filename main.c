#include "dev.h"
#include "pr.h"
#include "version.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
  char **paths;
  ssize_t num_paths = scan_dev_for_gpiochips(&paths);
  if (num_paths < 0) {
    pr_err("Failed to scan /dev for GPIO chip devices\n");
    return EXIT_FAILURE;
  }
  struct gpiod_chip **chips = malloc(num_paths * sizeof(struct gpiod_chip *));
  if (!chips) {
    pr_err("Failed to allocate memory for chip pointers\n");
    free_paths(paths, num_paths);
    return EXIT_FAILURE;
  }
  for (ssize_t i = 0; i < num_paths; i++) {
    struct gpiod_chip *chip = gpiod_chip_open(paths[i]);
    if (!chip) {
      pr_err("Failed to open GPIO chip device at path %s\n", paths[i]);
      while (i--) {
        gpiod_chip_close(chips[i]);
      }
      free(chips);
      free_paths(paths, num_paths);
      return EXIT_FAILURE;
    }
    chips[i] = chip;
  }

  const char *echo_gpio = NULL;
  const char *trig_gpio = NULL;
  pr_verbosity_set(pr_level_info);
  static const struct option longopts[] = {{"version", no_argument, NULL, 'V'},
                                           {"verbose", no_argument, NULL, 'v'},
                                           {"quietly", no_argument, NULL, 'q'},
                                           {"echo", required_argument, NULL, 'e'},
                                           {"trig", required_argument, NULL, 't'},
                                           {"host", optional_argument, NULL, 'h'},
                                           {"port", optional_argument, NULL, 'p'},
                                           {"help", no_argument, NULL, '?'},
                                           {
                                               NULL,
                                           }};
  int c, longind;
  while ((c = getopt_long(argc, argv, "Vvqe:t:h:p:?", longopts, &longind)) >= 0) {
    switch (c) {
    case 0:
      break;
    case 'V':
      (void)fprintf(stderr, "hc-sr04 version %s\n", VERSION);
      return EXIT_SUCCESS;
    case 'v':
      pr_verbosity_inc();
      break;
    case 'q':
      pr_verbosity_dec();
      break;
    case 'e':
      echo_gpio = optarg;
      break;
    case 't':
      trig_gpio = optarg;
      break;
    case '?':
      (void)fprintf(stderr, "Usage: %s [OPTIONS]\n", argv[0]);
      (void)fprintf(stderr, "Options:\n");
      (void)fprintf(stderr, "  -V, --version          Show version information and exit\n");
      (void)fprintf(stderr, "  -v, --verbose          Increase verbosity level\n");
      (void)fprintf(stderr, "  -q, --quietly          Decrease verbosity level\n");
      (void)fprintf(stderr, "  -e, --echo=GPIO        Use GPIO pin for echo (required)\n");
      (void)fprintf(stderr, "  -t, --trig=GPIO        Use GPIO pin for trigger (required)\n");
      (void)fprintf(stderr, "  -h, --host=HOST        Connect to Redis server at HOST\n");
      (void)fprintf(stderr, "  -p, --port=PORT        Connect to Redis server at PORT\n");
      (void)fprintf(stderr, "  -?, --help             Show this help message and exit\n");
      return EXIT_SUCCESS;
    default:
      (void)fprintf(stderr, "Unknown option: 0%o\n", c);
      return EXIT_FAILURE;
    }
  }
  if (optind < argc) {
    (void)fprintf(stderr, "Unexpected non-option argument: %s\n", argv[optind]);
    return EXIT_FAILURE;
  }
  if (!echo_gpio || !trig_gpio) {
    pr_err("Both echo and trig GPIO pins must be specified\n");
    return EXIT_FAILURE;
  }

  struct gpiod_chip *echo_chip = NULL;
  int echo_line = -1;
  struct gpiod_chip *trig_chip = NULL;
  int trig_line = -1;
  for (ssize_t i = 0; i < num_paths; i++) {
    int line_offset = gpiod_chip_get_line_offset_from_name(chips[i], echo_gpio);
    if (line_offset >= 0) {
      echo_chip = chips[i];
      echo_line = line_offset;
    }
    line_offset = gpiod_chip_get_line_offset_from_name(chips[i], trig_gpio);
    if (line_offset >= 0) {
      trig_chip = chips[i];
      trig_line = line_offset;
    }
  }
  if (!echo_chip || echo_line < 0) {
    pr_err("Failed to find GPIO chip containing echo pin %s\n", echo_gpio);
    return EXIT_FAILURE;
  }
  if (!trig_chip || trig_line < 0) {
    pr_err("Failed to find GPIO chip containing trig pin %s\n", trig_gpio);
    return EXIT_FAILURE;
  }
  if (echo_chip == trig_chip && echo_line == trig_line) {
    pr_err("Echo and trig pins cannot be the same\n");
    return EXIT_FAILURE;
  }

  struct gpiod_line_settings *line_settings = gpiod_line_settings_new();
  if (!line_settings) {
    pr_err("Failed to create line settings for echo pin\n");
    return EXIT_FAILURE;
  }
  if (gpiod_line_settings_set_direction(line_settings, GPIOD_LINE_DIRECTION_INPUT) < 0) {
    pr_err("Failed to set line direction for echo pin\n");
    gpiod_line_settings_free(line_settings);
    return EXIT_FAILURE;
  }
  if (gpiod_line_settings_set_edge_detection(line_settings, GPIOD_LINE_EDGE_BOTH) < 0) {
    pr_err("Failed to set edge detection for echo pin\n");
    gpiod_line_settings_free(line_settings);
    return EXIT_FAILURE;
  }

  struct gpiod_line_config *line_config = gpiod_line_config_new();
  if (!line_config) {
    pr_err("Failed to create line config for echo pin\n");
    gpiod_line_settings_free(line_settings);
    return EXIT_FAILURE;
  }
  if (gpiod_line_config_add_line_settings(line_config, (unsigned int[]){(unsigned int)echo_line}, 1, line_settings) < 0) {
    pr_err("Failed to add line settings for echo pin\n");
    gpiod_line_config_free(line_config);
    gpiod_line_settings_free(line_settings);
    return EXIT_FAILURE;
  }

  struct gpiod_request_config *request_config = gpiod_request_config_new();
  if (!request_config) {
    pr_err("Failed to create request config for echo pin\n");
    gpiod_line_config_free(line_config);
    gpiod_line_settings_free(line_settings);
    return EXIT_FAILURE;
  }
  gpiod_request_config_set_consumer(request_config, "hc-sr04");
  struct gpiod_line_request *line_request = gpiod_chip_request_lines(echo_chip, request_config, line_config);
  if (!line_request) {
    pr_err("Failed to request echo line\n");
    gpiod_request_config_free(request_config);
    gpiod_line_config_free(line_config);
    gpiod_line_settings_free(line_settings);
    return EXIT_FAILURE;
  }

  uint64_t rising_edge_timestamp_ns = 0ULL;
  enum gpiod_edge_event_type last_edge_event_type = GPIOD_EDGE_EVENT_FALLING_EDGE;
  for (;;) {
    int rc;
    if ((rc = gpiod_line_request_wait_edge_events(line_request, 1000000000LL)) < 0) {
      pr_err("Failed to wait for edge events on echo line\n");
      return EXIT_FAILURE;
    }
    if (rc == 0) {
      pr_debug("Wait for edge events on echo line timed out\n");
      continue;
    }
    struct gpiod_edge_event_buffer *buffer = gpiod_edge_event_buffer_new(rc);
    if (!buffer) {
      pr_err("Failed to create edge event buffer for echo line\n");
      return EXIT_FAILURE;
    }
    ssize_t num_events = gpiod_line_request_read_edge_events(line_request, buffer, rc);
    if (num_events < 0) {
      pr_err("Failed to read edge events for echo line\n");
      gpiod_edge_event_buffer_free(buffer);
      return EXIT_FAILURE;
    }
    for (ssize_t i = 0; i < num_events; i++) {
      struct gpiod_edge_event *event = gpiod_edge_event_buffer_get_event(buffer, i);
      if (!event) {
        pr_err("Failed to get edge event from buffer for echo line\n");
        gpiod_edge_event_buffer_free(buffer);
        return EXIT_FAILURE;
      }
      enum gpiod_edge_event_type event_type = gpiod_edge_event_get_event_type(event);
      uint64_t timestamp_ns = gpiod_edge_event_get_timestamp_ns(event);
      pr_debug("Received %s edge event on echo line at timestamp %lu ns\n", event_type == GPIOD_EDGE_EVENT_RISING_EDGE ? "rising" : "falling", timestamp_ns);
      switch (event_type) {
      case GPIOD_EDGE_EVENT_RISING_EDGE:
        rising_edge_timestamp_ns = timestamp_ns;
        break;
      case GPIOD_EDGE_EVENT_FALLING_EDGE:
        if (last_edge_event_type == GPIOD_EDGE_EVENT_RISING_EDGE) {
          uint64_t pulse_width_ns = timestamp_ns - rising_edge_timestamp_ns;
          pr_info("Pulse width: %lu ns\n", pulse_width_ns);
        }
        break;
      default:
        pr_err("Unknown edge event type %d on echo line\n", event_type);
        break;
      }
      last_edge_event_type = event_type;
    }
    gpiod_edge_event_buffer_free(buffer);
  }

  for (ssize_t i = 0; i < num_paths; i++) {
    gpiod_chip_close(chips[i]);
  }
  free(chips);
  free_paths(paths, num_paths);
  return EXIT_SUCCESS;
}
