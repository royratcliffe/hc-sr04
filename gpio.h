#ifndef GPIO_H
#define GPIO_H

#include <gpiod.h>

/*!
 * \brief Read edge events for a GPIO line request and yield them to a callback
 * function.
 *
 * \param request The GPIO line request to read edge events from.
 *
 * \param buffer The edge event buffer to store the read edge events.
 *
 * \param max_events The maximum number of edge events to read and yield to the
 * callback function.
 *
 * \param yield The callback function to which the read edge events will be
 * yielded. This function will be called for each edge event read from the
 * request, and the edge event will be passed as an argument to the callback
 * function. The callback function can perform any necessary processing on the
 * edge event, such as logging, analysing, or storing the event data. The
 * callback function can also use the user_data argument to access additional
 * context or state information needed for processing the edge events.
 *
 * Its return value will be accumulated and returned by
 * gpio_line_request_read_edge_events. This allows the callback function to
 * contribute to the overall result of the edge event processing, such as
 * counting the number of events processed, or returning an error code if a
 * certain condition is met.
 *
 * \param user_data A pointer to user-defined data that will be passed to the
 * callback function for each edge event. This can be used to provide additional
 * context or state information to the callback function when processing the
 * edge events.
 *
 * \return 0 on success, or a negative error code on failure. If the function
 * returns a negative error code, it indicates that an error occurred while
 * reading edge events from the request or while yielding the events to the
 * callback function. The specific error code can provide more information about
 * the nature of the error, such as whether it was a failure to read edge
 * events, an error in the callback function, or some other issue.
 */
int gpio_line_request_read_edge_events(struct gpiod_line_request *request, struct gpiod_edge_event_buffer *buffer, size_t max_events,
                                       int (*yield)(struct gpiod_edge_event *event, void *user_data), void *user_data);

/*!
 * \brief Convert a line value to a string.
 * \param value The line value to convert.
 * \return A string representation of the line value ("active", "inactive", "error", or "unknown").
 */
const char *gpio_line_value_to_string(enum gpiod_line_value value);

/*!
 * \brief Convert an edge event type to a string.
 * \param event_type The edge event type to convert.
 * \return A string representation of the edge event type ("rising", "falling", or "unknown").
 */
const char *gpio_edge_event_type_to_string(enum gpiod_edge_event_type event_type);

void gpio_line_settings_free(void *line_settings);   /*!< Free a line settings object */
void gpio_line_config_free(void *line_config);       /*!< Free a line config object */
void gpio_request_config_free(void *request_config); /*!< Free a request config object */
void gpio_edge_event_buffer_free(void *buffer);      /*!< Free an edge event buffer object */

#endif /* GPIO_H */
