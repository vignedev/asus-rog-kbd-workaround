#!/usr/bin/env sh

set -e
MODALIAS='hid:b0003g0001v00000B05p000019B6'

function print_usage(){
    echo "usage: $0 <on|off>"
    echo "       $0 brightness <number>"
    exit 1
}

function find_hidraw() {
    for device in /sys/class/hidraw/hidraw[0-9]*; do
        if grep -Fqe "MODALIAS=$MODALIAS" "$device/device/uevent"; then
            HIDRAW_PATH="/dev/$(basename "$device")"
            return
        fi
    done

    echo 'failed to find approapriate hidraw'
    exit 1
}

function write_aura_init() {
    printf '\x5d\xb3\x00\x00\xa6\x00\x00\xeb\x00\x00\x00\x00\x00\x00\x00\x00\x00' > "$1"
}

function write_led_set() {
    printf '\x5d\xb5\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' > "$1"
}

function write_led_apply() {
    printf '\x5d\xb4\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' > "$1"
}

function write_led_on() {
    printf '\x5d\xbd\x01\x0f\x06\x03\x03' > "$1"
}

function write_led_off() {
    printf '\x5d\xbd\x01\x00\x00\x00\x00' > "$1"
}

function write_brightness() {
    # i am so sorry this is so dirty.
    BRIGHT="\x$2"
    printf "\x5a\xba\xc5\xc4$BRIGHT\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" > "$1"
}

if [ "$1" = "on" ]; then
    find_hidraw
    write_aura_init "$HIDRAW_PATH"
    write_led_set   "$HIDRAW_PATH"
    write_led_on    "$HIDRAW_PATH"
    write_aura_init "$HIDRAW_PATH"
    write_led_set   "$HIDRAW_PATH"
elif [ "$1" = "off" ]; then
    find_hidraw
    write_aura_init "$HIDRAW_PATH"
    write_led_set   "$HIDRAW_PATH"
    write_led_off   "$HIDRAW_PATH"
    write_aura_init "$HIDRAW_PATH"
    write_led_set   "$HIDRAW_PATH"
elif [ "$1" = "brightness" ]; then
    find_hidraw
    write_brightness "$HIDRAW_PATH" "$2"
else
    print_usage
fi