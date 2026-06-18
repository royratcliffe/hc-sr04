# HC-SR04

This is a Linux C application for reading an HC-SR04 ultrasonic distance sensor
via libgpiod edge events, with optional Redis Stream output.

## What It Does

- Discovers GPIO chips under /dev and resolves GPIO line names that you pass using the --echo and --trig options.
- Drives the trigger line in a repeating pulse cycle.
- Measures echo pulse width from rising and falling edge timestamps.
- Prints pulse width in nanoseconds to stdout by default.
- Optionally publishes readings to a Redis Stream using `hiredis`.

## Requirements

- Linux with GPIO character device support.
- C compiler with C11 support.
- CMake 3.10 or newer.
- `pkg-config`.
- `libgpiod` development package.
- `hiredis` development package.

On Debian/Ubuntu-like systems:

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config libgpiod-dev libhiredis-dev
```

## Build

From the repository root:

```bash
cmake -S . -B build
cmake --build build
```

The executable is created at:

- `build/hc-sr04`

Optional install:

```bash
sudo cmake --install build
```

This installs the binary to `/usr/local/bin/hc-sr04` by default.

## Command Line Usage

Show help:

```bash
./build/hc-sr04 --help
```

General form:

```bash
./build/hc-sr04 --echo <GPIO_NAME> --trig <GPIO_NAME> [options]
```

Options:

- -V, --version: Show version information and exit.
- -v, --verbose: Increase log verbosity.
- -q, --quietly: Decrease log verbosity.
- -e, --echo=GPIO: Echo GPIO line name (required).
- -t, --trig=GPIO: Trigger GPIO line name (required).
- -h, --host=HOST: Redis host. If omitted, output goes to stdout.
- -p, --port=PORT: Redis port (default: 6379).
- -m, --maxlen=MAXLEN: Approximate Redis stream trim length (default: 100).
- -?, --help: Show usage.

## Examples

Print pulse widths to stdout:

```bash
./build/hc-sr04 --echo GPIO8 --trig GPIO11
```

Send readings to Redis Stream hc-sr04 on localhost:

```bash
./build/hc-sr04 --echo GPIO8 --trig GPIO11 --host 127.0.0.1
```

With explicit Redis port and stream max length:

```bash
./build/hc-sr04 --echo GPIO8 --trig GPIO11 --host 127.0.0.1 --port 6379 --maxlen 1000
```

Read the stream in another terminal:

```bash
redis-cli XREAD BLOCK 0 STREAMS hc-sr04 $
```

Each stream entry includes:

- `pulse_width_ns`: Echo pulse width in nanoseconds.

## Converting Pulse Width To Distance

The program emits pulse width, not distance. For HC-SR04, a common approximation is:

- `distance_cm = pulse_width_us / 58`

where `pulse_width_us` is pulse width in microseconds.

Convert from nanoseconds first:

- `pulse_width_us = pulse_width_ns / 1000`
- `distance_cm = pulse_width_us / 58`

## Notes

- You must provide different GPIO lines for echo and trig.
- GPIO line names depend on your board/kernel configuration (for example `GPIO8`, `GPIO11`).
- Access to `/dev/gpiochip*` may require root privileges or group permissions.
- Stop with Ctrl+C (SIGINT) or SIGTERM; the program releases GPIO and Redis resources on exit.

## License

MIT (see SPDX headers in source files).
