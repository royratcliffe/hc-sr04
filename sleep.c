#include "sleep.h"

#include <errno.h>
#include <time.h>

int sleep_ns(long ns) {
  struct timespec duration = {.tv_sec = 0, .tv_nsec = ns};
  for (struct timespec rem; nanosleep(&duration, &rem) < 0; duration = rem) {
    if (errno != EINTR) {
      return -1;
    }
  }
  return 0;
}
