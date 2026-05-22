# phatIO

> **Note:** phatio.com is no longer owned or operated by me. Any link to
> phatio.com (including ones still embedded in this project's source headers)
> may now point to an unrelated site.

phatIO is firmware for an AVR-based USB device that lets you control physical
hardware from any computer **without installing drivers**. When plugged in it
enumerates as an ordinary USB mass-storage disk (and a USB keyboard), so you
read sensors and drive outputs simply by reading and writing files on that
virtual disk.

## What it does

- **Looks like a USB flash drive.** It presents a FAT filesystem, so it works
  out of the box on Windows, macOS, and Linux with no drivers or special
  software.
- **Hardware as files.** Read and control the board's pins and peripherals by
  reading/writing files on the disk:
  - Digital I/O pins
  - Analog inputs (ADC)
  - PWM outputs
  - SPI and TWI/I2C buses
- **Acts as a USB keyboard.** It can "type" keystrokes straight into the host
  computer.
- **Scriptable with lio.** lio is a small Lisp-like language (e.g.
  `(config 1 1)`, `(keyboard "hello")`). Scripts live in a runfile on the disk;
  the device watches that file and re-runs it whenever it changes.
- **Field-updatable.** A built-in USB bootloader allows firmware updates.

## How it works

The firmware runs on an AVR USB microcontroller and is built on the
[LUFA](https://github.com/abcminiuser/lufa) USB stack. An SD card backs
the FAT filesystem that the host sees; a SCSI/mass-storage layer exposes it over
USB, and reads/writes to the special files are translated into hardware
operations (ADC, PWM, GPIO, SPI, TWI) or fed to the lio interpreter.

## Examples Using it from the shell

Once plugged in, phatIO mounts like a USB stick with the volume label
`PHATIO`. Everything lives under its `io/` directory:

```
io/mode/<pin>    # read or set a pin's mode
io/pins/<pin>    # read or set a pin's value
io/status        # device status
io/RUN.LIO       # lio script the device runs and re-runs when it changes
io/etc/          # config + pin maps (adc_pins, pwm_pins, digital_pins)
io/lib/          # lio library files
```

Pins are numbered `0`–`19`. Modes are `IN`, `OUT`, `ADC`, `PWM`, `HIGH`
(input with pull-up) — only the first letter matters and case is ignored.
ADC works on pins `0 1 2 3 4 5 9 10 11 16 18 19`; PWM on pins `6 7 8 9 10`.

**Blink an LED on pin 3**
```
echo OUT > /Volumes/PHATIO/io/mode/3      # make pin 3 an output
echo 1   > /Volumes/PHATIO/io/pins/3      # LED on
echo 0   > /Volumes/PHATIO/io/pins/3      # LED off

# blink forever
while true; do
  echo 1 > /Volumes/PHATIO/io/pins/3; sleep 0.5
  echo 0 > /Volumes/PHATIO/io/pins/3; sleep 0.5
done
```

**Set pin 4 as a digital input and read value**

```sh
echo IN  > /Volumes/PHATIO/io/mode/4
cat /Volumes/PHATIO/io/pins/4             # -> 0 or 1
```

**Add an internal pull-up resistor to pin 5 and read **

```sh
echo HIGH > $/Volumes/PHATIO/io/mode/5
cat /Volumes/PHATIO/io/pins/5             # -> 1 idle, 0 when pressed to ground
```

**Set pin 0 as an ADC input and read value**

```sh
echo ADC > /Volumes/PHATIO/io/mode/0
cat /Volumes/PHATIO/io/pins/0             # -> 0..1023
```

**Dim an LED with PWM on pin 6**

```sh
echo PWM > /Volumes/PHATIO/io/mode/6
echo 64  > /Volumes/PHATIO/io/pins/6      # 0 = off, 255 = full (8-bit)
echo 255 > /Volumes/PHATIO/io/pins/6
```

**Run a lio script** — write to the runfile; the device runs it whenever it
changes. This makes phatIO act as a USB keyboard and type "hello":

```sh
echo '(keyboard "hello")' > /Volumes/PHATIO/io/RUN.LIO
```

## Repository layout

- `src/` — firmware source: USB stack glue, FAT/SCSI mass-storage emulation, the
  lio interpreter, ADC/PWM/pin/SPI/TWI drivers, SD card driver, and the
  bootloader.
- `fs/` — files that live on the device's virtual disk (config, pin maps, lio
  library).
- `hardware/` — board schematic and hardware notes.
- `support/` — host-side build helpers (filesystem image builder, CRC tool).
- `test/` — unit test suite (runs on the host with stubbed AVR headers).

## Building

You need an AVR toolchain (`avr-gcc`, `avr-libc`, `avr-binutils`) and `make`.
Build with `make` from the repository root. See the `*.mk` makefiles for the
configurable build options.

## License

BSD 2-clause (see the source-file headers and `fs/io/etc/license.txt`).

## Contact

Andrew Smallbone — andrew@rocketnumbernine.com
