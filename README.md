xwobf
=====

A simple program that takes a screenshot of your desktop and obscures all
visible windows. It is intended for use with programs such as i3lock to
dynamically generate a lockscreen.

<img src="https://raw.githubusercontent.com/glindste/i/master/xwobf.gif" height="450">

### Usage

The program takes a filename argument. The filename should end with a .png, .jpg or equivalent suffix.

### Example lock script using i3lock

```
#!/bin/bash
xwobf /tmp/.i3lock.png
i3lock -i /tmp/.i3lock.png
```

### Credit

All credit for the idea belongs to [Airblader](https://github.com/Airblader) and
his [gist](https://gist.github.com/Airblader/3a96a407e16dae155744).

### License

MIT
