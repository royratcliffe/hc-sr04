#define MS_TO_NS(ms) ((ms) * 1000000LL)

/*!
 * \brief Sleep for a specified number of nanoseconds.
 *
 * \param ns The number of nanoseconds to sleep.
 *
 * \return 0 on success, or -1 if an error occurs.
 *
 * \details This function uses the `nanosleep` system call to suspend execution
 * for the specified duration. The input parameter `ns` specifies the sleep
 * duration in nanoseconds, which is converted into a `struct timespec` before
 * being passed to `nanosleep`. If the sleep is interrupted by a signal, the
 * function will automatically resume sleeping for the remaining time until the
 * total specified duration has elapsed. If an error occurs during the sleep,
 * the function returns -1 and sets `errno` to indicate the error.
 *
 * Common errors include `EINVAL` if the input duration is invalid, e.g.,
 * negative or too large.
 */
int sleep_ns(long ns);
