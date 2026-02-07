# vim: syntax=spec
Name:       pipewire-audio-idle-inhibit-git
Version:    {{{ git_repo_release lead="$(git describe --tags --abbrev=0)" }}}
Release:    {{{ echo -n "$(git rev-list --all --count)" }}}%{?dist}
Summary:    Prevents the screen from sleeping while audio is actively playing or being recorded through Pipewire.
License:    GPLv3
URL:        https://github.com/The1fEst/PipewireAudioIdleInhibit
VCS:        {{{ git_repo_vcs }}}
Source:     {{{ git_repo_pack }}}

BuildRequires: meson >= 0.60.0
BuildRequires: git
BuildRequires: gcc-c++

BuildRequires: pkgconfig(libpipewire-0.3)
BuildRequires: pkgconfig(libsystemd)
BuildRequires: pkgconfig(systemd)

%{?systemd_requires}

%description
Prevents the screen from sleeping while audio is actively playing or being recorded through Pipewire.
Works with swayidle, hypridle, and other systemd-compatible idle inhibitors.

%prep
{{{ git_repo_setup_macro }}}

%build
%meson
%meson_build

%install
%meson_install

%post
%systemd_user_post pipewire-audio-idle-inhibit.service

%preun
%systemd_user_preun pipewire-audio-idle-inhibit.service

%files
%doc README.md
%{_bindir}/pipewire-audio-idle-inhibit
%{_userunitdir}/pipewire-audio-idle-inhibit.service
%license LICENSE

# Changelog will be empty until you make first annotated Git tag.
%changelog
{{{ git_repo_changelog }}}
