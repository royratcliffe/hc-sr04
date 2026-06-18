# while true; do gpioset -t 10us,0 GPIO11=1; sleep 0.5; done &
# gpiomon -F %S GPIO8 | awk -W interactive 'NR%2==1 {t=$1; next} {print ($1 - t) * 1e6 / 52}'

# The HC-SR04 ultrasonic sensor measures distance by sending a trigger pulse and
# then measuring the time it takes for the echo pulse to return. The distance
# can be calculated using the formula: distance = (time * speed of sound) / 2.
# The speed of sound is approximately 343 meters per second, which is equivalent
# to 29 microseconds per centimeter. Therefore, the distance in centimeters can
# be calculated as: distance = (time in microseconds) / 58.
gpioset -t 10us,0 GPIO11=1; gpiomon -F %S GPIO8 | awk -W interactive 'NR%2==1 {t=$1; next} {print ($1 - t) * 1e6 / 58; system("gpioset -t 10us,0 GPIO11=1")}'
