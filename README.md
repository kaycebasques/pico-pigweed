# Kudzu

[TOC]

## Getting Started

Make sure you've set up [Pigweed's
prerequisites](https://pigweed.dev/docs/getting_started.html#prerequisites).

**If you're on Windows**, you can automate the initial setup by downloading the
first-time setup script **from cmd.exe**:

```bat
curl https://pigweed.googlesource.com/pigweed/sample_project/+/main/tools/setup_windows_prerequisites.bat?format=TEXT > setup_pigweed_prerequisites.b64 && certutil -decode -f setup_pigweed_prerequisites.b64 setup_pigweed_prerequisites.bat && del setup_pigweed_prerequisites.b64
```

Then you can run the script with the following command **in cmd.exe**:

```bat
setup_pigweed_prerequisites.bat
```

Note: You may see a few UAC prompts as the script installs Git, Python, and
enables developer mode.

Once that is done, you can clone this project with the following command:
```sh
git clone https://pigweed.googlesource.com/pigweed/kudzu
```

### Environment setup

Pigweed uses a local development environment for most of its tools. This
means tools are not installed to your machine, and are instead stored in a
directory inside your project (Note: git ignores this directory). The tools
are temporarily added to the PATH of the current shell session.

To make sure the latest tooling has been fetched and set up, run the bootstrap
command for your operating system:

**Windows**

```bat
bootstrap.bat
```

**Linux & Mac**

```sh
source ./bootstrap.sh
```

After tooling updates, you might need to run bootstrap again to ensure the
latest tools.

After the initial bootstrap, you can use use the `activate` scripts to configure
the current shell for development without doing a full update.

**Windows**

```sh
activate.bat
```

**Linux & Mac**

```sh
source ./activate.sh
```

### Device tools setup

Install the pico SDK and tool to flash the device.

```sh
pw package install pico_sdk
pw package install picotool
```
These packages will be built and added to the path automatically. There is no
need to add these to the gn arguments.

## Linux Setup

### GLFW Dependency:

Install the GLFW OpenGL library
```sh
sudo apt install libglfw3-dev libglfw3
```

### Udev Rules:

Put the following into `/usr/lib/udev/rules.d/49-picoprobe.rules`

```
# Pico app mode
SUBSYSTEMS=="usb", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="000a", MODE:="0666"
KERNEL=="ttyACM*", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="000a", MODE:="0666", SYMLINK+="rp2040"

# RP2 Boot
SUBSYSTEMS=="usb", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="0003", MODE:="0666"
KERNEL=="ttyACM*", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="0003", MODE:="0666", SYMLINK+="rp2040"

# Picoprobe
SUBSYSTEMS=="usb", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="0004", MODE:="0666"
KERNEL=="ttyACM*", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="0004", MODE:="0666", SYMLINK+="picoprobe"
```

This will also symlink `/dev/picoprobe` and `/dev/rp2040` to the respective
vendor and product ids.

Apply the above rules with:

```sh
sudo udevadm control --reload-rules
sudo udevadm trigger
```

## Compile:

```sh
pw build
```

## Run:

### Host

Run the host app and connect to it via `pw console`:

```sh
./out/gn/host_device_simulator.speed_optimized/obj/applications/badge/bin/badge & \
  pw console --socket-addr default ; \
  killall badge
```

### Kudzu

```sh
export ELF=./out/gn/rp2040.size_optimized/obj/applications/badge/bin/badge.elf

picotool reboot -f -u && \
  sleep 3 && \
  picotool load -x $ELF
```

Connect with `pw console`:

```sh
pw console --verbose \
  --baudrate 115200 \
  --token-databases ./out/gn/rp2040.size_optimized/obj/applications/badge/bin/badge.elf \
  --device /dev/rp2040
```

From Python Repl window you can issue RPCs interactively:

```
>>> device.rpcs.kudzu.rpc.Kudzu.PackageTemp()
(Status.OK, kudzu.rpc.PackageTempResponse(temp=27.60657501220703))
```
