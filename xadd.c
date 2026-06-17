/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026, Roy Ratcliffe, Northumberland, United Kingdom
 */

/*!
 * \file xadd.c
 *
 * \brief Source file for Redis stream addition functions.
 *
 * \details This file contains the implementation of functions that handle
 * adding entries to a Redis stream when certain events occur, such as changes
 * on GPIO lines. It uses the hiredis library to interact with Redis and the
 * when.h macros for modular callback registration.
 */
#include "pr.h"
#include "redis.h"
#include "when.h"
#include "call_at_exit.h"

#include <hiredis.h>

#include <stdlib.h>
#include <string.h>

#define MAXLEN 100

static struct redisContext *context = NULL;

static int maxlen = MAXLEN;

CAUSES(opt_m, handle_opt_m, const char *optarg) {
  maxlen = atoi(optarg);
  if (maxlen <= 0) {
    maxlen = MAXLEN;
    pr_warn("Invalid maxlen value %s, using default value %d\n", optarg, maxlen);
  }
}

/*!
 * \brief Handle the trig event.
 *
 * \details This function is called when the trig event occurs. It checks if the
 * Redis host is set and attempts to connect to the Redis server if it is.
 */
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
  call_at_exit((void (*)(void *))redisFree, context);
}

/*!
 * \brief Handle the echo event.
 *
 * \param pulse_width_ns The pulse width in nanoseconds to be added to the
 * trimmed \c hc-sr04 Redis stream.
 *
 * \details This function is called when the echo event occurs. It adds an entry
 * to the Redis stream with the pulse width in nanoseconds.
 */
CAUSES(echo, handle_echo, uint64_t pulse_width_ns) {
  void *reply = redisCommand(context, "XADD %s MAXLEN ~ %d * pulse_width_ns %lu", redis_key(), maxlen, pulse_width_ns);
  if (reply == NULL) {
    pr_err("Failed to add entry to Redis stream\n");
    if (context->err) {
      pr_err("Redis error: %s\n", context->errstr);
    }
    redisFree(context);
    exit(EXIT_FAILURE);
  }
  freeReplyObject(reply);
}
