# Pipewire Audio Idle Inhibit

A daemon that prevents the screen from sleeping while audio is actively playing or being recorded through Pipewire. Works with swayidle, hypridle, and other systemd-compatible idle inhibitors.

**Requires:** systemd or elogind for inhibit support. Pipewire.

## How It Works

The daemon monitors the Pipewire audio system for active inputs (microphone/recording) and outputs (speakers/playback). When audio activity is detected, it sends an idle inhibit request to the systemd/elogind logind service, which prevents the screen from entering sleep mode. Once all audio stops, the inhibition is released.

Supports both **PulseAudio-style streams** and **PipeWire-JACK clients** (e.g. Guitarix, Ardour, REAPER, guitar amp simulators). JACK clients are detected and treated as bidirectional audio nodes, making it ideal for musicians and audio engineers.

## Installation

### Arch Linux

The package is available on the [AUR](https://aur.archlinux.org/packages/pipewire-audio-idle-inhibit-git/)

### From Source

**Compile with systemd** (default):
```bash
meson setup build -Dlogind-provider=systemd
meson compile -C build
meson install -C build
```

**Compile with elogind** (for systems without systemd):
```bash
meson setup build -Dlogind-provider=elogind
meson compile -C build
meson install -C build
```

### Build Dependencies

**Arch Linux:**
```bash
pacman -Syu base-devel meson pkgconf systemd pipewire
```

**Debian/Ubuntu (systemd):**
```bash
apt install meson pkgconf libsystemd-dev libpipewire-0.3-dev
```

**Fedora/RHEL (systemd):**
```bash
dnf install meson pkgconf systemd-devel pipewire-devel
```

## Usage

### Basic daemon mode
Run in the background to prevent idle when audio is active:
```bash
pipewire-audio-idle-inhibit
```

Output:
```
IDLE INHIBITED
NOT IDLE INHIBITED
```

Add to your Hyprland config to auto-start:
```ini
exec-once = pipewire-audio-idle-inhibit
```

or Sway:
```ini
exec pipewire-audio-idle-inhibit
```

### Monitor mode

Show a live in-place table of audio activity without inhibiting idle:
```bash
pipewire-audio-idle-inhibit --monitor
```

Example output:
```
AppName   | Input | Output
----------|-------|---------
Firefox   |       | *
REAPER    | *     | *
Discord   | *     |
```

The table updates in-place as streams start and stop.

### Waybar mode

Output JSON for Waybar integration:
```bash
pipewire-audio-idle-inhibit --waybar
```

### Ignoring specific applications

Exclude certain applications from triggering the idle inhibitor using a config file.

The config file is searched in the following order (first found wins):
1. `$XDG_CONFIG_HOME/pipewire-audio-idle-inhibit/ignore.conf`
2. `/etc/pipewire-audio-idle-inhibit/ignore.conf`
3. `/usr/share/pipewire-audio-idle-inhibit/ignore.conf`

A default example config is installed to `/usr/share/pipewire-audio-idle-inhibit/ignore.conf`. Copy it to customize:

```bash
mkdir -p ~/.config/pipewire-audio-idle-inhibit
cp /usr/share/pipewire-audio-idle-inhibit/ignore.conf ~/.config/pipewire-audio-idle-inhibit/
```

**Config format:**
```json
{
    # Ignore these apps only for input (microphone/recording)
    input: ["Discord", "Zoom"],

    # Ignore these apps only for output (speakers/playback)
    output: ["Firefox", "Spotify"],

    # Ignore these apps for both input and output
    both: ["mpv"]
}
```

## Waybar Integration

Display an icon in Waybar when audio is active. Add to `~/.config/waybar/config`:

```json
"custom/audio_idle_inhibitor": {
  "format": "{icon}",
  "exec": "pipewire-audio-idle-inhibit --waybar",
  "exec-if": "which pipewire-audio-idle-inhibit",
  "return-type": "json",
  "format-icons": {
    "output": "🔊",
    "input": "🎤",
    "output-input": "🔊🎤",
    "none": ""
  }
}
```

Then add `custom/audio_idle_inhibitor` to your desired module list (modules-left, modules-center, or modules-right).

## Development

### Build with debug symbols:
```bash
meson setup build --buildtype=debug
meson compile -C build
```

### Debug in VS Code:
1. Open the workspace in VS Code
2. Press **F5** to start debugging (configured in `.vscode/launch.json`)
3. Set breakpoints as needed
4. The pre-launch task will build the project automatically

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

Based on the original work by Erik Reider - [SwayAudioIdleInhibit](https://github.com/ErikReider/SwayAudioIdleInhibit)

