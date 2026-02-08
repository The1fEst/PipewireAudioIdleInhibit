# Pipewire Audio Idle Inhibit

A daemon that prevents the screen from sleeping while audio is actively playing or being recorded through Pipewire. Works with swayidle, hypridle, and other idle inhibitors.

**Requires:** Pipewire. Optionally: systemd/elogind (for DBus inhibition) or Wayland compositor with `zwp_idle_inhibit_manager_v1` support.

## How It Works

The daemon monitors the Pipewire audio system for active inputs (microphone/recording) and outputs (speakers/playback). When audio activity is detected, it sends an idle inhibit request via either:

- **Wayland** (default) — Uses the `zwp_idle_inhibit_manager_v1` protocol directly. Works with Sway, Hyprland, and other Wayland compositors that support this protocol.
- **systemd/elogind** — Uses D-Bus to call `org.freedesktop.login1.Manager.Inhibit`.

The inhibition method is selected in the config file. If Wayland is unavailable at runtime, it automatically falls back to systemd.

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
pacman -Syu base-devel meson pkgconf systemd pipewire wayland wayland-protocols jq
```

**Debian/Ubuntu (systemd):**
```bash
apt install meson pkgconf libsystemd-dev libpipewire-0.3-dev libwayland-dev wayland-protocols libjq-dev
```

**Fedora/RHEL (systemd):**
```bash
dnf install meson pkgconf systemd-devel pipewire-devel wayland-devel wayland-protocols-devel jq-devel
```

> **Note:** Wayland support is optional. If `wayland-client` and `wayland-protocols` are not found at build time, the daemon will only support systemd/elogind inhibition.

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

or systemd:
```ini
systemctl --user enable --now pipewire-audio-idle-inhibit.service
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
1. `$XDG_CONFIG_HOME/pipewire-audio-idle-inhibit/config.json`
2. `/etc/pipewire-audio-idle-inhibit/config.json`
3. `/usr/share/pipewire-audio-idle-inhibit/config.json`

A default example config is installed to `/usr/share/pipewire-audio-idle-inhibit/config.json`. Copy it to customize:

```bash
mkdir -p ~/.config/pipewire-audio-idle-inhibit
cp /usr/share/pipewire-audio-idle-inhibit/config.json ~/.config/pipewire-audio-idle-inhibit/
```

**Config format:**
```json
{
    "input": ["Discord", "Zoom"],
    "output": ["Firefox", "Spotify"],
    "both": ["mpv"],
    "inhibition_type": "wayland"
}
```

- `input` — ignore these apps for input (microphone/recording)
- `output` — ignore these apps for output (speakers/playback)
- `both` — ignore these apps for both input and output
- `inhibition_type` — `"wayland"` (default) or `"systemd"`

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

