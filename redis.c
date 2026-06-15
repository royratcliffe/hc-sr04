#include "redis.h"
#include "when.h"

#include <stdlib.h>

static const char *host = NULL;
static int port = 6379;
static const char *channel = "hc-sr04";
static const char *key = "hc-sr04";

CAUSES(opt_h, handle_opt_h, const char *optarg) { host = optarg; }
CAUSES(opt_p, handle_opt_p, const char *optarg) { port = atoi(optarg); }

const char *redis_host(void) { return host; }
int redis_port(void) { return port; }
const char *redis_channel(void) { return channel; }
const char *redis_key(void) { return key; }
