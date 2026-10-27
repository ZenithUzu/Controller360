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
