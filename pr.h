/* SPDX-License-Identifier: MIT */

/*!
 * \file pr.h
 * \details Header file for the pr logging functions. This file declares the
 * pr_verbosity variable, which controls the verbosity level of the logging, and
 * the pr_log function, which is used to log messages at different verbosity
 * levels. The pr_log function takes a log level, a format string, and a
 * variable argument list, and outputs the log message to the appropriate output
 * stream based on the log level. The pr_log function also checks the current
 * verbosity level before outputting the log message, allowing for more or less
 * detailed logging based on the needs of the developer. The pr_log function can
 * be used to log informational messages, debug messages, warnings, and errors,
 * providing a consistent and flexible way to output log messages throughout the
 * program. The implementation of the pr_log function can be designed to write
 * log messages to standard output, standard error, or to a log file, depending
 * on the requirements of the project and the desired logging behaviour.
 */
#ifndef PR_H_
#define PR_H_

#include <stdarg.h>
#include <stdbool.h>

/*!
 * \brief Log a message at a specific log level.
 * \details This function logs a message at the specified log level, using a
 * format string and a variable argument list. The function checks the current
 * verbosity level before outputting the log message, allowing for more or less
 * detailed logging based on the needs of the developer. The log message is
 * output to the appropriate output stream based on the log level, with errors
 * and warnings typically going to standard error, and informational and debug
 * messages going to standard output. The implementation of this function can be
 * designed to write log messages to a log file instead of standard output or
 * standard error, depending on the requirements of the project and the desired
 * logging behaviour.
 *
 * Only error and warning messages will include the standard error message
 * corresponding to the current value of errno, if errno is non-zero.
 * Informational and debug messages will not include the standard error message,
 * as they are typically used for general information and debugging purposes,
 * where the standard error message may not be relevant or necessary.
 *
 * \param level The log level at which to log the message.
 * \param format The printf-style format string for the log message.
 * \param args The variable argument list containing the values to be formatted
 * into the log message.
 * \return The number of characters written to the log output, or a negative
 * value if an error occurs.
 * \note The implementation of this function ensures that the original value of
 * errno is preserved, as logging operations may modify errno. By saving and
 * restoring errno, it maintains the integrity of error handling in the program
 * and prevents unintended side effects from the logging operations.
 */
#define pr(level, ...)                              \
  do {                                              \
    if (pr_logging(pr_level_##level)) {             \
      (void)pr_log_level(pr_level_##level);         \
      if (pr_level_##level <= pr_level_warn) {      \
        (void)pr_log_errno(pr_level_##level);       \
      }                                             \
      (void)pr_logf(pr_level_##level, __VA_ARGS__); \
    }                                               \
  } while (0)

/*
 * Convenience macros for logging at specific levels, allowing developers to log
 * messages at different levels without needing to specify the log level
 * explicitly each time. These macros simply call the pr macro with the
 * appropriate log level, making it easier to log messages at the desired level
 * with a simpler syntax.
 */
#define pr_err(...) pr(err, __VA_ARGS__)
#define pr_warn(...) pr(warn, __VA_ARGS__)
#define pr_info(...) pr(info, __VA_ARGS__)
#define pr_debug(...) pr(debug, __VA_ARGS__)

/*!
 * \brief Enumeration of log levels.
 * \details This enumeration defines the different log levels that can be used
 * with the pr logging functions. The log levels are ordered from most severe
 * (error) to least severe (debug), allowing developers to control the verbosity
 * of the logging output. The pr_level_max value can be used to define the
 * maximum log level, which can be useful for validating log level inputs or for
 * iterating over log levels in a loop.
 */
enum pr_level {
  pr_level_err = 0,   /*!< Error level; goes to standard error */
  pr_level_warn = 1,  /*!< Warning level; goes to standard error */
  pr_level_info = 2,  /*!< Informational level; goes to standard output */
  pr_level_debug = 3, /*!< Debug level; goes to standard output */
  pr_level_max = 4,   /*!< Maximum log level */
};

/*!
 * \brief Check if logging is enabled for a specific log level.
 * \details This function checks if logging is enabled for the specified log
 * level based on the current verbosity level. If the specified log level is
 * less than or equal to the current verbosity level, the function returns true,
 * indicating that logging is enabled for that level. Otherwise, it returns
 * false, indicating that logging is not enabled for that level. This allows
 * developers to control the verbosity of the logging output by adjusting the
 * verbosity level, enabling more detailed logging for debugging purposes or
 * less detailed logging for production use.
 * \param level The log level to check (e.g., error, warning, info, debug).
 * \return true if logging is enabled for the specified log level, false otherwise.
 */
bool pr_logging(enum pr_level level);

/*!
 * \brief Log a formatted message at a specific log level.
 * \details This function logs a formatted message at the specified log level,
 * using a printf-style format string and a variable argument list. The function
 * checks the current verbosity level before outputting the log message.
 * \param level The log level at which to log the message.
 * \param format The printf-style format string for the log message.
 * \param args The variable argument list containing the values to be formatted.
 * \return The number of characters written to the log output, or a negative
 * value if an error occurs.
 * \note The logging function does \e not provide a terminating newline
 * character at the end of the log message, as this allows for more flexible
 * logging behaviour. By not automatically appending a newline, the logging
 * function allows developers to control the formatting of log messages more
 * precisely, enabling them to create multi-line log entries or to format log
 * messages in a specific way without being constrained by an automatic newline.
 * This design choice can be particularly useful when logging complex data
 * structures or when integrating with other logging systems that may have their
 * own formatting requirements. Developers can choose to include a newline
 * character in the format string if they want each log message to be on a
 * separate line, or they can omit it if they want to continue logging on the
 * same line or if they want to format the output in a custom way.
 * \note The implementation saves the current value of errno at the beginning of
 * the function and restores it before returning. This is important because the
 * logging function may perform operations that could modify errno (e.g., file
 * I/O), and we want to ensure that the original errno value is preserved for
 * the caller, allowing it to correctly handle any errors that may have occurred
 * before the logging function was called. By saving and restoring errno, we
 * maintain the integrity of error handling in the program and prevent
 * unintended side effects from the logging operations.
 */
int pr_log(enum pr_level level, const char *format, va_list args);

/*!
 * \brief Log a formatted message at a specific log level, with variable arguments.
 * \details This function is a convenience wrapper around pr_log that takes a
 * variable number of arguments instead of a va_list. It initialises a va_list
 * with the provided arguments and then calls pr_log to perform the actual
 * logging. This allows developers to log messages with a simpler syntax without
 * needing to manually manage va_list objects.
 * \param level The log level at which to log the message.
 * \param format The printf-style format string for the log message.
 * \param ... The variable arguments to be formatted into the log message.
 * \return The number of characters written to the log output, or a negative
 * value if an error occurs.
 */
int pr_logf(enum pr_level level, const char *format, ...);

/*!
 * \brief Logs the given level name to the appropriate output stream based on
 * the log level.
 * \details This function takes a log level as an argument and logs the
 * corresponding level name (e.g., "ERR", "WARN", "INFO", "DEBUG") to the
 * appropriate output stream.
 */
int pr_log_level(enum pr_level level);

/*!
 * \brief Logs the standard error message corresponding to the current value of
 * errno, if errno is non-zero.
 * \details This function checks the current value of errno, and if it is
 * non-zero, it logs the corresponding error message (e.g., "No such file or
 * directory") to the appropriate output stream based on the log level. If errno
 * is zero, indicating that there is no error to report, the function simply
 * returns without logging anything.
 * \param level The log level at which to log the error message.
 * \return The number of characters written to the log output, or 0 if errno is
 * zero (indicating no error).
 */
int pr_log_errno(enum pr_level level);

/*!
 * \brief Set the verbosity level for logging.
 * \details This function sets the global verbosity level for logging, which
 * controls which log messages will be output based on their log level. By
 * setting the verbosity level, developers can control the amount of logging
 * output, allowing for more detailed logging during development and debugging,
 * or less detailed logging in production environments. The verbosity level can
 * be set to a specific value, or it can be incremented or decremented using the
 * pr_verbosity_inc and pr_verbosity_dec functions, respectively, to adjust the
 * logging verbosity dynamically at runtime.
 * \param verbosity The verbosity level to set (e.g., 0 for error only, 1 for
 * warning, 2 for info, 3 for debug).
 */
void pr_verbosity_set(int verbosity);

void pr_verbosity_inc(void);
void pr_verbosity_dec(void);

#endif /* PR_H_ */
