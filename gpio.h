#ifndef GPIO_H
#define GPIO_H

#include <gpiod.h>

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

void gpio_line_settings_free(void *line_settings); /*!< Free a line settings object */
void gpio_line_config_free(void *line_config); /*!< Free a line config object */
void gpio_request_config_free(void *request_config); /*!< Free a request config object */
void gpio_edge_event_buffer_free(void *buffer); /*!< Free an edge event buffer object */

#endif /* GPIO_H */
