# Pipewire Audio Idle Inhibit

A daemon that prevents the screen from sleeping while audio is actively playing or being recorded through Pipewire. Works with swayidle, hypridle, and other systemd-compatible idle inhibitors.

**Requires:** systemd or elogind for inhibit support. Pipewire.

## How It Works

The daemon monitors the Pipewire audio system for active inputs (microphone/recording) and outputs (speakers/playback). When audio activity is detected, it sends an idle inhibit request to the systemd/elogind logind service, which prevents the screen from entering sleep mode. Once all audio stops, the inhibition is released. When idle is inhibited, the daemon prints which applications are responsible.

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

When audio starts playing or recording, the output shows:
```
IDLE INHIBITED by: Firefox Spotify
```

Add to your Hyprland config to auto-start:
```ini
exec-once = pipewire-audio-idle-inhibit
```

or Sway:
```ini
exec pipewire-audio-idle-inhibit
```

### Monitoring modes (dry-run)

Print the audio activity status without inhibiting idle:

**Monitor both inputs and outputs:**
```bash
pipewire-audio-idle-inhibit --both
```

**Monitor only audio output:**
```bash
pipewire-audio-idle-inhibit --output
```

**Monitor only audio input:**
```bash
pipewire-audio-idle-inhibit --input
```

**Waybar-friendly JSON output:**
```bash
pipewire-audio-idle-inhibit --waybar
```

### Ignoring specific applications

Exclude certain applications from triggering the idle inhibitor.

**Ignore for outputs (speakers/playback):**
```bash
pipewire-audio-idle-inhibit --o "Firefox Spotify"
```

**Ignore for inputs (microphone/recording):**
```bash
pipewire-audio-idle-inhibit --i "Discord Zoom"
```

**Ignore for both inputs and outputs:**
```bash
pipewire-audio-idle-inhibit --b "Firefox Spotify Discord"
```

Multiple applications can be specified separated by spaces. Duplicates are automatically removed.

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

