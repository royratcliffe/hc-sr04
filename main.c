#include "call_at_exit.h"
#include "gpio.h"
#include "gpiochip.h"
#include "pr.h"
#include "redis.h"
#include "sleep.h"
#include "version.h"
#include "when.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
  char **paths;
  ssize_t num_paths = scan_dir_for_gpiochip_paths("/dev", &paths);
  if (num_paths < 0) {
    pr_err("Failed to scan /dev for GPIO chip devices\n");
    return EXIT_FAILURE;
  }
  struct gpiod_chip **chips = malloc(num_paths * sizeof(struct gpiod_chip *));
  if (!chips) {
    pr_err("Failed to allocate memory for chip pointers\n");
    free_gpiochip_paths(paths, num_paths);
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
      free_gpiochip_paths(paths, num_paths);
      return EXIT_FAILURE;
    }
    chips[i] = chip;
  }

  const char *echo_gpio = NULL;
  const char *trig_gpio = NULL;
  static const struct option longopts[] = {{"version", no_argument, NULL, 'V'},
                                           {"verbose", no_argument, NULL, 'v'},
                                           {"quietly", no_argument, NULL, 'q'},
                                           {"echo", required_argument, NULL, 'e'},
                                           {"trig", required_argument, NULL, 't'},
                                           {"host", required_argument, NULL, 'h'},
                                           {"port", required_argument, NULL, 'p'},
                                           {"maxlen", required_argument, NULL, 'm'},
                                           {"help", no_argument, NULL, '?'},
                                           {
                                               NULL,
                                           }};
  int c, longind;
  while ((c = getopt_long(argc, argv, "Vvqe:t:h:p:m:?", longopts, &longind)) >= 0) {
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
    case 'h':
      OCCURS(opt_h, optarg);
      break;
    case 'p':
      OCCURS(opt_p, optarg);
      break;
    case 'm':
      OCCURS(opt_m, optarg);
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
      (void)fprintf(stderr, "  -m, --maxlen=MAXLEN    Set maximum length of Redis stream\n");
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

  /*
   * Find the GPIO chips and lines for the echo and trig pins.
   * The echo and trig pins must be on different lines of the same or different chips.
   */
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

  struct gpiod_line_settings *echo_settings = gpiod_line_settings_new();
  if (!echo_settings) {
    pr_err("Failed to create line settings for echo pin\n");
    return EXIT_FAILURE;
  }
  call_at_exit(line_settings_free, echo_settings);
  if (gpiod_line_settings_set_direction(echo_settings, GPIOD_LINE_DIRECTION_INPUT) < 0) {
    pr_err("Failed to set line direction for echo pin\n");
    return EXIT_FAILURE;
  }
  if (gpiod_line_settings_set_edge_detection(echo_settings, GPIOD_LINE_EDGE_BOTH) < 0) {
    pr_err("Failed to set edge detection for echo pin\n");
    return EXIT_FAILURE;
  }

  struct gpiod_line_settings *trig_settings = gpiod_line_settings_new();
  if (!trig_settings) {
    pr_err("Failed to create line settings for trig pin\n");
    return EXIT_FAILURE;
  }
  call_at_exit(line_settings_free, trig_settings);
  if (gpiod_line_settings_set_direction(trig_settings, GPIOD_LINE_DIRECTION_OUTPUT) < 0) {
    pr_err("Failed to set line direction for trig pin\n");
    return EXIT_FAILURE;
  }

  struct gpiod_line_config *echo_config = gpiod_line_config_new();
  if (!echo_config) {
    pr_err("Failed to create line config for echo pin\n");
    return EXIT_FAILURE;
  }
  call_at_exit(line_config_free, echo_config);
  if (gpiod_line_config_add_line_settings(echo_config, (unsigned int[]){(unsigned int)echo_line}, 1, echo_settings) < 0) {
    pr_err("Failed to add line settings for echo pin\n");
    return EXIT_FAILURE;
  }

  struct gpiod_line_config *trig_config = gpiod_line_config_new();
  if (!trig_config) {
    pr_err("Failed to create line config for trig pin\n");
    return EXIT_FAILURE;
  }
  call_at_exit(line_config_free, trig_config);
  if (gpiod_line_config_add_line_settings(trig_config, (unsigned int[]){(unsigned int)trig_line}, 1, trig_settings) < 0) {
    pr_err("Failed to add line settings for trig pin\n");
    return EXIT_FAILURE;
  }

  /*
   * Create a request configuration for the echo line.
   */
  struct gpiod_request_config *echo_request_config = gpiod_request_config_new();
  if (!echo_request_config) {
    pr_err("Failed to create request config for echo pin\n");
    return EXIT_FAILURE;
  }
  call_at_exit(request_config_free, echo_request_config);
  gpiod_request_config_set_consumer(echo_request_config, "hc-sr04");
  struct gpiod_line_request *line_request = gpiod_chip_request_lines(echo_chip, echo_request_config, echo_config);
  if (!line_request) {
    pr_err("Failed to request echo line\n");
    return EXIT_FAILURE;
  }

  /*
   * Create a request configuration for the trig line. The trig line will be set
   * to active initially, and then toggled between active and inactive states
   * based on the edge events detected on the echo line. The trig line will be
   * set to active for 10 ms, and then set to inactive for 60 ms, and this cycle
   * will repeat indefinitely. The edge events on the echo line will be used to
   * measure the pulse width of the signal received from the ultrasonic sensor.
   */
  struct gpiod_request_config *trig_request_config = gpiod_request_config_new();
  if (!trig_request_config) {
    pr_err("Failed to create request config for trig pin\n");
    return EXIT_FAILURE;
  }
  call_at_exit(request_config_free, trig_request_config);
  gpiod_request_config_set_consumer(trig_request_config, "hc-sr04");
  struct gpiod_line_request *trig_line_request = gpiod_chip_request_lines(trig_chip, trig_request_config, trig_config);
  if (!trig_line_request) {
    pr_err("Failed to request trig line\n");
    return EXIT_FAILURE;
  }

  /*
   * Initially set the trig line to active and wait for edge events on the echo
   * line with a timeout of 10 ms. If no edge events are detected within the
   * timeout, set the trig line to inactive and wait for edge events on the echo
   * line with a timeout of 60 ms. The HC-SR04 emits eight pulses of 40 kHz
   * sound waves on the trig line's falling edge.
   *
   * If no edge events are detected within the timeout, set the trig line to
   * active again and wait for edge events on the echo line with a timeout of 10
   * ms. If an edge event is detected on the echo line, record the timestamp of
   * the event and determine whether it is a rising or falling edge. If it is a
   * rising edge, record the timestamp and wait for a falling edge. If it is a
   * falling edge, calculate the pulse width by subtracting the timestamp of the
   * rising edge from the timestamp of the falling edge and print the pulse
   * width in nanoseconds to stdout. If the pulse width is greater than 0, print
   * the pulse width in nanoseconds to stdout. If the pulse width is 0, print a
   * warning message to stderr.
   *
   * Repeat this process indefinitely, alternating between active and inactive
   * states for the trig line and waiting for edge events on the echo line with
   * the corresponding timeouts. When an edge event is detected on the echo
   * line, record the timestamp of the event and determine whether it is a
   * rising or falling edge.
   */
  enum gpiod_line_value trig_value = GPIOD_LINE_VALUE_ACTIVE;
  int64_t timeout_ns = MS_TO_NS(10);
  if (gpiod_line_request_set_value(trig_line_request, trig_line, trig_value) < 0) {
    pr_err("Failed to set initial value for trig line\n");
    return EXIT_FAILURE;
  }
  OCCURS(trig);
  uint64_t rising_edge_timestamp_ns = 0ULL;
  enum gpiod_edge_event_type last_edge_event_type = GPIOD_EDGE_EVENT_FALLING_EDGE;
  for (;;) {
    int max_events;
    if ((max_events = gpiod_line_request_wait_edge_events(line_request, timeout_ns)) < 0) {
      pr_err("Failed to wait for edge events on echo line\n");
      return EXIT_FAILURE;
    }
    if (max_events == 0) {
      pr_debug("Wait for edge events on echo line timed out\n");
      switch (trig_value) {
      case GPIOD_LINE_VALUE_ACTIVE:
        trig_value = GPIOD_LINE_VALUE_INACTIVE;
        timeout_ns = MS_TO_NS(60);
        break;
      case GPIOD_LINE_VALUE_INACTIVE:
        trig_value = GPIOD_LINE_VALUE_ACTIVE;
        timeout_ns = MS_TO_NS(10);
        break;
      default:
        pr_warn("Unknown trig line value %d\n", trig_value);
        trig_value = GPIOD_LINE_VALUE_ACTIVE;
        timeout_ns = MS_TO_NS(10);
        break;
      }
      if (gpiod_line_request_set_value(trig_line_request, trig_line, trig_value) < 0) {
        pr_err("Failed to set initial value for trig line\n");
        return EXIT_FAILURE;
      }
      pr_debug("Set trig line to %s and waiting for edge events on echo line with timeout %lu ns\n",
               trig_value == GPIOD_LINE_VALUE_ACTIVE ? "active" : "inactive", timeout_ns);
      continue;
    }
    struct gpiod_edge_event_buffer *buffer = gpiod_edge_event_buffer_new(max_events);
    if (!buffer) {
      pr_err("Failed to create edge event buffer for echo line\n");
      return EXIT_FAILURE;
    }
    ssize_t num_events = gpiod_line_request_read_edge_events(line_request, buffer, max_events);
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
          if (pulse_width_ns > 0) {
            if (redis_host()) {
              OCCURS(echo, pulse_width_ns);
            } else {
              (void)printf("%lu\n", pulse_width_ns);
            }
          } else {
            pr_warn("Received falling edge event on echo line with zero pulse width\n");
          }
        }
        break;
      default:
        pr_warn("Unknown edge event type %d on echo line\n", event_type);
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
  free_gpiochip_paths(paths, num_paths);
  return EXIT_SUCCESS;
}
