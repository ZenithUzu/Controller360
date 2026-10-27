**English** | [Español](README.md)

# Controller360

Xbox 360 homebrew test program that checks whether the console can make a generic controller vibrate, for example one connected through UsbdSecPatch / XeUnshackle.

## Usage

1. Open `Controller360.sln` in Visual Studio 2010 with the XDK and build **Release | Xbox 360**.
2. Copy `Controller360.xex` over FTP to its own folder on the console and launch it from Aurora.
3. Plug in the controller. If it was already plugged in, unplug it and plug it back in.
4. Try the buttons:
   - **A**: left motor at 100%
   - **B**: right motor at 100%
   - **X**: both motors at 50%
   - **Y**: stop vibration
   - **BACK + START**: exit
5. Exit the program and download `Controller360.log` over FTP. It is in the same folder as the `.xex`.

## Screen colors

- **Blue**: idle, or vibration off.
- **Green**: the console accepted the vibration command.
- **Red**: the console returned an error when trying to vibrate.

Green without vibration means the command left the console but the controller did not react.

## Spy plugin (`Plugin/`)

Dashlaunch plugin that **changes nothing**: it only logs what games ask the console about the controller, and every vibration request.

1. Open an XDK command prompt in the `Plugin` folder and run `build_plugin.bat`. You get `Controller360Spy.xex`.
2. Copy it to the USB drive and add it to `launch.ini`, e.g. `plugin4 = Usb:\Controller360Spy.xex`.
3. Restart the console and play a game that vibrates for a while: first with the original controller, then with the generic one.
4. Download `Controller360Spy.log` over FTP from the root of the hard drive (or the USB drive if there is no hard drive).
5. To remove it, delete the line from `launch.ini` and restart.

A line identical to the previous one is not repeated, and logging stops at 5000 lines.
