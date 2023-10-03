# `keyboard.sh`

This is the "original" version that was the workaround. You had to call it yourself.

### Usage

```console
$ ./keyboard.sh
usage: ./keyboard.sh <on|off>
       ./keyboard.sh brightness <number>
```

That's it. Quite self explanatory, isn't it?

Okay, small note though: `<number>` should be a number from 0 to 3 (inclusive). Other values shouldn't do anything.

If you get a `failed to find approapriate hidraw` error, check and replace the `MODALIAS` at the beginning of the file if you still wish to try it out.