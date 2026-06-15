#include "pr.h"
#include "redis.h"
#include "when.h"

#include <hiredis.h>

#include <stdlib.h>
#include <string.h>

static struct redisContext *context = NULL;

static size_t maxlen = 100;

CAUSES(opt_m, handle_opt_m, const char *optarg) {
  maxlen = (size_t)atoi(optarg);
  if (maxlen == 0) {
    pr_warn("Invalid maxlen value %s, using default value %zu\n", optarg, maxlen);
    maxlen = 100;
  }
}

CAUSES(trig, handle_trig) {
  if (redis_host() == NULL) {
    pr_info("Redis host is required to add to Redis stream\n");
    return;
  }
  context = redisConnect(redis_host(), redis_port());
  if (context == NULL || context->err) {
    if (context) {
      pr_err("Redis error: %s\n", context->errstr);
    } else {
      pr_err("Can't allocate Redis context\n");
    }
    exit(EXIT_FAILURE);
  }
}

CAUSES(echo, handle_echo, uint64_t pulse_width_ns) {
  if (redisCommand(context, "XADD %s MAXLEN ~ %u * pulse_width_ns %llu", redis_key(), maxlen, pulse_width_ns) == NULL) {
    pr_err("Failed to add entry to Redis stream\n");
    if (context->err) {
      pr_err("Redis error: %s\n", context->errstr);
    }
    redisFree(context);
    exit(EXIT_FAILURE);
  }
}
