# Pipewire Audio Idle Inhibit

A daemon that prevents the screen from sleeping while audio is actively playing or being recorded through Pipewire. Works with swayidle, hypridle, and other systemd-compatible idle inhibitors.

**Requires:** systemd or elogind for inhibit support. Pipewire.

## How It Works

The daemon monitors the Pipewire audio system for active sources (microphone/input) and sinks (speakers/output). When audio activity is detected, it sends an idle inhibit request to the systemd/elogind logind service, which prevents the screen from entering sleep mode. Once all audio stops, the inhibition is released.

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

Add to your Hyprland config to auto-start:
```ini
exec-once = pipewire-audio-idle-inhibit
```

or Sway:
```ini
exec pipewire-audio-idle-inhibit
```

### Monitoring modes (dry-run)

Print the audio playback/recording status without inhibiting idle:

**Monitor both sinks and sources:**
```bash
pipewire-audio-idle-inhibit --dry-print-both
```

**Monitor only audio output (sinks):**
```bash
pipewire-audio-idle-inhibit --dry-print-sink
```

**Monitor only audio input (sources):**
```bash
pipewire-audio-idle-inhibit --dry-print-source
```

**Waybar-friendly JSON output:**
```bash
pipewire-audio-idle-inhibit --dry-print-both-waybar
```

### Ignoring specific sources
Exclude certain applications or devices from triggering the idle inhibitor:
```bash
pipewire-audio-idle-inhibit --ignore-source-outputs "app1 app2"
```

## Waybar Integration

Display an icon in Waybar when audio is active. Add to `~/.config/waybar/config`:

```json
"custom/audio_idle_inhibitor": {
  "format": "{icon}",
  "exec": "pipewire-audio-idle-inhibit --dry-print-both-waybar",
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

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

Based on the original work by Erik Reider - [SwayAudioIdleInhibit](https://github.com/ErikReider/SwayAudioIdleInhibit)
