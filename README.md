# `asus-rog-kbd-workaround`

...that's quite a mouthful. 

There's an ongoing issue with ASUS Zephyrus G15 2022 (GA503RM) devices, whose keyboards backlight cannot be controlled on Linux. While it does provide you with a `/sys/devices/platform/asus-nb-wmi/leds/asus::kbd_backlight`, changing its value does nothing and the keyboard will remain the same regardless.

To learn the story behind it, you can read on at [`STORY.md`](./STORY.md). If you are looking for the old version, you can find it in [`ancient/keyboard.sh`](./ancient/).

---

## `rogkbdsync`

As the name suggests, this program monitors the `/sys/devices/platform/asus-nb-wmi/leds/asus::kbd_backlight/brightness` file for changes and once a change is detected, it will send brightness change packets to the keyboard.

Because of that, you will need your Window Manager (or whatever) to update said file when you press the Fn keyboard brightness keys. For example, KDE already handles that.

Small disclaimer: my knowledge of C is kind of dusty, so be aware of that. I did however try to handle all errors and such, so hopefully it won't cause havok.

### Requirements

- `hidapi`
- `g++` or `gcc`

### Building

```sh
# For any Linux distro
g++ src/main.c $(pkg-config --cflags hidapi-hidraw) $(pkg-config --libs hidapi-hidraw) -o rogkbdsync

# Only on Arch-based systems (uses PKGBUILD)
makepkg -si
```

### Usage

```sh
# For any Linux distro
sudo ./rogkbdsync

# If you have installed the *.service or used the PKGBUILD
sudo systemctl enable --now rogkbdsync
```

Again, that's it (thanks to the hard coded stuff in it). You will need to either have `sudo` or write access to the approapriate HID device. You will also need to have this running in the background, of course.

### Minor troubleshooting section

- Ensure that the `/sys/.../asus::kbd_backlight/brightness` is being updated when you press the `Fn+F2/F3`
- It is possible that your keyboard backlight to be shut off after Boot, in that case no matter the brightness you change, it'd remain turned off
  - Usually the backlight is on during POST but shuts off after Linux boots (specifically after `asusd` service starts)
  - **Solution 1**: You can either use `ancient/keyboard.sh on` to force the keyboard backlight to be on
  - **Solution 2**: Use `rog-control-center` to change Keyboard LED power settings to be enabled on Boot
    - At the time of writing, it seems to be forcing it to be off
  - **Solution 3**: Manually edit the `aura.ron` config
    - Firstly, stop the `asusd` service
    - Edit the `/etc/asusd/aura.ron`, where at the bottom of the file you might see something like:
        ```
        ...
        multizone: None,
        multizone_on: false,
        enabled: AuraDevRog2((
            keyboard: (
                zone: Keyboard,
                boot: false,
                awake: false,
                sleep: false,
                shutdown: false,
            ),
        ...
        ```
    - Set `keyboard.boot` and `keyboard.awake` to `true`
    - Start the `asusd` service

---

## Disclaimer

Provided as is, no warranty or liability over whatever might happen. This just happens to work for me.