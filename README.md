# Interplatform Tools

This is an open source software for embedded Linux systems.

## tools/ilogger

This tool makes log files on external block devices.

### Features:
 * read input data from stdin,
 * perform log rotation based on disks usage,
 * split log-files according to the specified size,
 * automatically mount/umount devices,
 * write to the specified devices according to their availability.

Please see options with '-h'.

## Building

### build for the current machine
```sh
  $ make
```

### build for LinPac controller
Setup cross-compiling for LinPac

* Download LinPAC SDK for Linux from here:
  http://www.abersys.com/support

* Place it at the root directory "/", you can make a symbolic link as follows:
```sh
  $ sudo ln -s /home/username/lincon /lincon
```

```sh
  $ make CROSS=linpac
```
