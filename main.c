/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026, Roy Ratcliffe, Northumberland, United Kingdom
 */

/*!
 * \file main.c
 * \brief Main source file for the hc-sr04 program.
 */
#include "call_at_exit.h"
#include "gpio.h"
#include "gpio_line.h"
#include "gpiochip.h"
#include "pr.h"
#include "redis.h"
#include "sleep.h"
#include "version.h"
#include "when.h"

#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*!
 * \brief Maximum number of edge events to read from the echo line at a time.
 * \details This constant defines the maximum number of edge events that can be
 * read from the echo line in a single call to
 * gpiod_line_request_read_edge_events. The edge events are stored in a buffer,
 * and this constant determines the size of that buffer. The value of 10 is
 * chosen as a reasonable limit to ensure that the program can handle bursts of
 * edge events without consuming excessive memory. If more than 10 edge events
 * occur on the echo line before the program can read them, the additional
 * events will be lost. However, in typical use cases with the HC-SR04
 * ultrasonic sensor, it is unlikely that more than 10 edge events will occur in
 * such a short time frame, so this limit should be sufficient for most
 * applications.
 */
#define MAX_EVENTS 10

static void handle_sig(int signum);
static void clean_up(void *);

static void gpio_line_settings_free(void *line_settings);   /*!< Free a line settings object */
static void gpio_line_config_free(void *line_config);       /*!< Free a line config object */
static void gpio_request_config_free(void *request_config); /*!< Free a request config object */
static void gpio_line_request_release(void *line_request);  /*!< Release a line request object */
static void gpio_edge_event_buffer_free(void *buffer);      /*!< Free an edge event buffer object */

static volatile sig_atomic_t sig;

static char **paths;
static ssize_t num_paths;
static struct gpiod_chip **chips;

/*
 * Retain the GPIO line information in static variables so that they can be
 * freed at exit using the call_at_exit mechanism. This allows for proper
 * cleanup of resources when the program exits, ensuring that any allocated
 * memory or opened GPIO lines are released appropriately. The free_gpio_line
 * function will be registered to be called at exit for both the echo and trig
 * lines, allowing for a clean shutdown of the program and preventing resource
 * leaks.
 */
static struct gpio_line echo, trig;

int main(int argc, char *argv[]) {
  struct sigaction sa;
  sa.sa_handler = handle_sig;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIGINT, &sa, NULL) < 0) {
    pr_err("Failed to set signal handler for SIGINT\n");
    return EXIT_FAILURE;
  }
  if (sigaction(SIGTERM, &sa, NULL) < 0) {
    pr_err("Failed to set signal handler for SIGTERM\n");
    return EXIT_FAILURE;
  }

  num_paths = scan_dir_for_gpiochip_paths("/dev", &paths);
  if (num_paths < 0) {
    pr_err("Failed to scan /dev for GPIO chip devices\n");
    return EXIT_FAILURE;
  }
  chips = malloc(num_paths * sizeof(struct gpiod_chip *));
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
  call_at_exit(clean_up, NULL);

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
      echo.name = optarg;
      break;
    case 't':
      trig.name = optarg;
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
  if (!echo.name || !trig.name) {
    pr_err("Both echo and trig GPIO pins must be specified\n");
    return EXIT_FAILURE;
  }

  /*
   * Find the GPIO chips and lines for the echo and trig pins.
   * The echo and trig pins must be on different lines of the same or different chips.
   */
  if (gpio_line_add_offset_from_name(&echo, chips, num_paths) < 0) {
    pr_err("Failed to find GPIO chip containing echo pin %s\n", echo.name);
    return EXIT_FAILURE;
  }
  if (gpio_line_add_offset_from_name(&trig, chips, num_paths) < 0) {
    pr_err("Failed to find GPIO chip containing trig pin %s\n", trig.name);
    return EXIT_FAILURE;
  }
  if (echo.chip == trig.chip && echo.offset == trig.offset) {
    pr_err("Echo and trig pins cannot be the same\n");
    return EXIT_FAILURE;
  }

  /*
   * Close any GPIO chips that are not needed for the echo and trig pins. This
   * is done to free up resources and avoid potential conflicts with other GPIO
   * lines that may be on the same chips.
   */
  for (size_t i = 0; i < num_paths; i++) {
    if (chips[i] != echo.chip && chips[i] != trig.chip) {
      pr_debug("Closing unused GPIO chip at path %s\n", paths[i]);
      gpiod_chip_close(chips[i]);
      chips[i] = NULL;
    }
  }

  /*
   * Create line settings for the echo pin. The echo pin will be configured as
   * an input with edge detection enabled for both rising and falling edges. The
   * edge events on the echo pin will be used to measure the pulse width of the
   * signal received from the ultrasonic sensor. The echo pin will be set to
   * active on a rising edge and set to inactive on a falling edge. The pulse
   * width will be calculated by subtracting the timestamp of the rising edge
   * from the timestamp of the falling edge.
   */
  if (gpio_line_set_direction(&echo, GPIOD_LINE_DIRECTION_INPUT) < 0) {
    pr_err("Failed to set line direction for echo pin\n");
    return EXIT_FAILURE;
  }
  if (gpio_line_set_edge_detection(&echo, GPIOD_LINE_EDGE_BOTH) < 0) {
    pr_err("Failed to set edge detection for echo pin\n");
    return EXIT_FAILURE;
  }
  call_at_exit(gpio_line_settings_free, echo.line_settings);

  /*
   * Create line settings for the trig pin. The trig pin will be configured as
   * an output. The trig pin will be toggled between active and inactive states
   * based on the edge events detected on the echo line. The trig pin will be
   * set to active for 10 ms, and then set to inactive for 60 ms, and this cycle
   * will repeat indefinitely.
   */
  if (gpio_line_set_direction(&trig, GPIOD_LINE_DIRECTION_OUTPUT) < 0) {
    pr_err("Failed to set line direction for trig pin\n");
    return EXIT_FAILURE;
  }
  call_at_exit(gpio_line_settings_free, trig.line_settings);

  /*
   * Configure the GPIO lines for the echo and trig pins. This involves creating
   * line configuration objects for each pin and adding the corresponding line
   * settings to those configurations. The line configuration objects will be
   * used when requesting the lines later in the program. The line configuration
   * for the echo pin will include the settings for input direction and edge
   * detection, while the line configuration for the trig pin will include the
   * settings for output direction.
   */
  if (gpio_line_config(&echo) == NULL) {
    pr_err("Failed to configure line for echo pin\n");
    return EXIT_FAILURE;
  }
  call_at_exit(gpio_line_config_free, echo.line_config);
  if (gpio_line_config(&trig) == NULL) {
    pr_err("Failed to configure line for trig pin\n");
    return EXIT_FAILURE;
  }
  call_at_exit(gpio_line_config_free, trig.line_config);

  /*
   * Create a request configuration for the echo line.
   */
  if (gpio_line_set_consumer(&echo, "hc-sr04") < 0) {
    pr_err("Failed to set consumer for echo pin\n");
    return EXIT_FAILURE;
  }
  call_at_exit(gpio_request_config_free, echo.request_config);
  if (gpio_line_request(&echo) == NULL) {
    pr_err("Failed to request echo line\n");
    return EXIT_FAILURE;
  }
  call_at_exit(gpio_line_request_release, echo.line_request);

  /*
   * Create a request configuration for the trig line. The trig line will be set
   * to active initially, and then toggled between active and inactive states
   * based on the edge events detected on the echo line. The trig line will be
   * set to active for 10 ms, and then set to inactive for 60 ms, and this cycle
   * will repeat indefinitely. The edge events on the echo line will be used to
   * measure the pulse width of the signal received from the ultrasonic sensor.
   */
  if (gpio_line_set_consumer(&trig, "hc-sr04") < 0) {
    pr_err("Failed to set consumer for trig pin\n");
    return EXIT_FAILURE;
  }
  call_at_exit(gpio_request_config_free, trig.request_config);
  if (gpio_line_request(&trig) == NULL) {
    pr_err("Failed to request trig line\n");
    return EXIT_FAILURE;
  }
  call_at_exit(gpio_line_request_release, trig.line_request);

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
  int64_t timeout_ns = MS_TO_NS(10);
  if (gpio_line_set_value(&trig, GPIOD_LINE_VALUE_ACTIVE) < 0) {
    pr_err("Failed to set initial value for trig line\n");
    return EXIT_FAILURE;
  }
  OCCURS(trig);
  uint64_t rising_edge_timestamp_ns = 0ULL;
  enum gpiod_edge_event_type last_edge_event_type = GPIOD_EDGE_EVENT_FALLING_EDGE;
  struct gpiod_edge_event_buffer *buffer = gpiod_edge_event_buffer_new(MAX_EVENTS);
  if (!buffer) {
    pr_err("Failed to create edge event buffer for echo line\n");
    return EXIT_FAILURE;
  }
  call_at_exit(gpio_edge_event_buffer_free, buffer);
  while (sig == 0) {
    int max_events;
    if ((max_events = gpiod_line_request_wait_edge_events(echo.line_request, timeout_ns)) < 0) {
      if (errno == EINTR) {
        pr_debug("Wait for edge events on echo line interrupted by signal\n");
        continue;
      }
      pr_err("Failed to wait for edge events on echo line\n");
      return EXIT_FAILURE;
    }
    if (max_events == 0) {
      pr_debug("Wait for edge events on echo line timed out\n");
      enum gpiod_line_value value;
      switch (gpio_line_get_value(&trig)) {
      case GPIOD_LINE_VALUE_ACTIVE:
        value = GPIOD_LINE_VALUE_INACTIVE;
        timeout_ns = MS_TO_NS(60);
        break;
      case GPIOD_LINE_VALUE_INACTIVE:
        value = GPIOD_LINE_VALUE_ACTIVE;
        timeout_ns = MS_TO_NS(10);
        break;
      default:
        pr_warn("Unknown trig line value %d\n", gpio_line_get_value(&trig));
        value = GPIOD_LINE_VALUE_ACTIVE;
        timeout_ns = MS_TO_NS(10);
        break;
      }
      if (gpio_line_set_value(&trig, value) < 0) {
        pr_err("Failed to set initial value for trig line\n");
        return EXIT_FAILURE;
      }
      pr_debug("Set trig line to %s and waiting for edge events on echo line with timeout %ld ns\n", gpio_line_value_to_string(value), (long)timeout_ns);
      continue;
    }
    struct gpio_edge_event_generator generator;
    gpio_edge_event_generator_init(&generator, echo.line_request, buffer, (size_t)max_events);
    for (;;) {
      if (sig != 0) {
        break;
      }
      struct gpiod_edge_event *event = NULL;
      int num_events = gpio_edge_event_generator_next(&generator, &event);
      if (num_events < 0) {
        pr_err("Failed to read edge events for echo line\n");
        return EXIT_FAILURE;
      }
      if (num_events == 0) {
        break;
      }
      enum gpiod_edge_event_type event_type = gpiod_edge_event_get_event_type(event);
      uint64_t timestamp_ns = gpiod_edge_event_get_timestamp_ns(event);
      pr_debug("Received %s edge event on echo line at timestamp %lu ns\n", gpio_edge_event_type_to_string(event_type), (unsigned long)timestamp_ns);
      switch (event_type) {
      case GPIOD_EDGE_EVENT_RISING_EDGE:
        rising_edge_timestamp_ns = timestamp_ns;
        break;
      case GPIOD_EDGE_EVENT_FALLING_EDGE:
        if (last_edge_event_type == GPIOD_EDGE_EVENT_RISING_EDGE) {
          if (timestamp_ns > rising_edge_timestamp_ns) {
            uint64_t pulse_width_ns = timestamp_ns - rising_edge_timestamp_ns;
            if (redis_host()) {
              OCCURS(echo, pulse_width_ns);
            } else {
              (void)printf("%lu\n", (unsigned long)pulse_width_ns);
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
  }

  exit(EXIT_SUCCESS);
}

static void handle_sig(int signum) { sig |= 1 << signum; }

static void clean_up(void *) {
  for (ssize_t i = 0; i < num_paths; i++) {
    if (chips[i]) {
      pr_debug("Closing GPIO chip at path %s\n", paths[i]);
      gpiod_chip_close(chips[i]);
    }
  }
  free(chips);
  free_gpiochip_paths(paths, num_paths);
}

static void gpio_line_settings_free(void *line_settings) {
  pr_debug("Freeing line settings object at address %p\n", line_settings);
  gpiod_line_settings_free(line_settings);
}

static void gpio_line_config_free(void *line_config) {
  pr_debug("Freeing line config object at address %p\n", line_config);
  gpiod_line_config_free(line_config);
}

static void gpio_request_config_free(void *request_config) {
  pr_debug("Freeing request config object at address %p\n", request_config);
  gpiod_request_config_free(request_config);
}

static void gpio_line_request_release(void *line_request) {
  pr_debug("Releasing line request object at address %p\n", line_request);
  gpiod_line_request_release(line_request);
}

static void gpio_edge_event_buffer_free(void *buffer) {
  pr_debug("Freeing edge event buffer object at address %p\n", buffer);
  gpiod_edge_event_buffer_free(buffer);
}
