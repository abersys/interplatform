# Interplatform API & Tools

This is an open source software for embedded Linux systems.

## ilogger

The tool for making log files on external block devices.

Most embedded systems require to record the data of the running applications.
You can connect them to this tool to make sure data is stored on a single or
multiple devices. It can be linked through a pipe, e.g. here it is writing
random data to scsi device, which major number is 8 and minor number greater
then 2:

```sh
$ cat /dev/urandom | ./ilogger -d 8,2 -m /mnt
```

### Features:
 * read input data from stdin,
 * perform log rotation based on disks usage,
 * split log-files according to the specified size,
 * automatically mount/umount devices,
 * write to the specified devices according to their availability.

Please use 'ilogger -h' to see a complete list of available options.

## Building

### compiling for the current machine
```sh
  $ make
```

### cross-compiling for LinPac controller

* Download LinPAC SDK for Linux from here:
  http://www.abersys.com/support

```sh
  $ make CROSS=lincon
```

Additionally, you can use 'CROSS_DIR' parameter to override SDK
default directory, i.e. '~/lincon' in the above case.
