# `asus-rog-kbd-workaround`

...that's quite a mouthful. 

There's an ongoing issue with ASUS Zephyrus G15 2022 (GA503) devices, whose keyboards backlight cannot be controlled on Linux. While it provides your with a `/sys/devices/platform/asus-nb-wmi/leds/asus::kbd_backlight`, changing its value does nothing and the keyboard will shine or be off regardless.

To learn the story behind it, go on below. Otherwise, use the script attached, you ought to double check if everything regarding IDs fit. 

## Usage

```console
$ ./keyboard.sh
usage: ./keyboard.sh <on|off>
       ./keyboard.sh brightness <number>
```

That's it. Quite self explanatory, isn't it? Okay, small note though: `<number>` should be a number from 0 to 3 (inclusive). Other values shouldn't do anything.

If you get a `failed to find approapriate hidraw` error, check and replace the `MODALIAS` at the beginning of the file if you still wish to try it out.

## Okay, how?

During fiddling with `rog-control-center`, I somehow was able to make the backlight go off and couldn't get it back on. It even persisted throughout reboots (notably after `asusd` service started).

Knowing that, I started fiddling with the `/etc/asusd/aura.ron` configuration file:

```diff
--- /etc/asusd/aura.ron.on      2023-08-28 00:23:42.772230250 +0200
+++ /etc/asusd/aura.ron.off     2023-08-28 00:24:46.138402944 +0200
@@ -88,36 +88,36 @@
     enabled: AuraDevRog2((
         keyboard: (
             zone: Keyboard,
-            boot: true,
-            awake: true,
+            boot: false,
+            awake: false,
             sleep: false,
             shutdown: false,
         ),
         logo: (
             zone: Keyboard,
-            boot: true,
-            awake: true,
+            boot: false,
+            awake: false,
             sleep: false,
             shutdown: false,
         ),
         lightbar: (
             zone: Keyboard,
-            boot: true,
-            awake: true,
+            boot: false,
+            awake: false,
             sleep: false,
             shutdown: false,
         ),
         lid: (
             zone: Keyboard,
-            boot: true,
-            awake: true,
+            boot: false,
+            awake: false,
             sleep: false,
             shutdown: false,
         ),
         rear_glow: (
             zone: Keyboard,
-            boot: true,
-            awake: true,
+            boot: false,
+            awake: false,
             sleep: false,
             shutdown: false,
         ),
```

Where `---` denote what I'd put to turn it on and `+++` for turning it off. After that, restarting the `asusd` service would turn the backlight on/off respectively.

Given that it was a messy solution that even I wasn't satisfied with, I decided to dig a bit deeper and tried Wireshark, from which I learned that it was sending `USBHID` requests to the device:

|Order|Backlight On|Backlight Off|
|-----|------------|-------------|
|0|`5d b3 00 00 a6 00 00 eb 00 00 00 00 00 00 00 00 00`|`5d b3 00 00 a6 00 00 eb 00 00 00 00 00 00 00 00 00`
|1|`5d b5 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00`|`5d b5 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00`
|2|`5d b4 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00`|`5d b4 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00`
|3|`5d bd 01 0f 06 03 03`|`5d bd 01 00 00 00 00`
|4|`5d b5 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00`|`5d b5 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00`
|5|`5d b4 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00`|`5d b4 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00`

I'll spare you the headache, request number 3 is different. Now, if we cross reference the different bytes in request 3 with [`power.rs`](https://gitlab.com/asus-linux/asusctl/-/blob/4641e19c43ef5fa85e5d8d2aa5b6d39fce67db33/rog-aura/src/power.rs#L71) from the `asusctl` project, we can see that the `0f 06 03 03` are correctly set in accordance to what we'd expect from the config.

From the [same project](https://gitlab.com/asus-linux/asusctl/-/blob/4641e19c43ef5fa85e5d8d2aa5b6d39fce67db33/rog-aura/src/usb.rs#L18), we can also see that request 1,4 and 2,5 are called `LED_APPLY` and `LED_SET`. The first request sems to be [`AuraEffect` packet data](https://gitlab.com/asus-linux/asusctl/-/blob/4641e19c43ef5fa85e5d8d2aa5b6d39fce67db33/rog-aura/src/builtin_modes.rs#L407).

~~And this is where my journey ends for now, got what I wanted after all.~~

After some digging around, I remembered there was a `rogauracore` project, which allowed to change the brightness of the keyboard. It didn't work, however I thought that perhaps the packets might be similiarso I found out this [packet sequence](https://github.com/wroberts/rogauracore/blob/a872431a59e47c1ab0b2a523e413723bdcd93a6e/src/rogauracore.c#L112). Cross referencing it with the `asusctl` project, we can see that in `usb.rs` I somehow missed [`aura_brightness_bytes`](https://gitlab.com/asus-linux/asusctl/-/blob/4641e19c43ef5fa85e5d8d2aa5b6d39fce67db33/rog-aura/src/usb.rs#L22).

And so, if that packet is sent over (with correct brightness being 0-3 inclusive), the the backlight works, yay.

## Disclaimer

Provided as is, no warranty or liability over whatever might happen. This just happens to work for me.