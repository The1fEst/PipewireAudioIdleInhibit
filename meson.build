project(
	'PipewireAudioIdleInhibit',
	'c', 'cpp',
	version: '0.3.0',
	default_options: [
		'c_std=c11',
		'cpp_std=c++11',
		'warning_level=2',
		'werror=true',
	],
)

add_project_arguments(
	[
		'-Wno-unused-parameter',
		'-Wno-unused-result',
		'-Wno-missing-braces',
		'-Wno-format-zero-length',
		'-Wno-missing-field-initializers',
		'-Wundef',
		'-Wvla',
		'-Wlogical-op',
		'-Wmissing-include-dirs',
		'-Wpointer-arith',
		'-Winit-self',
		'-Wimplicit-fallthrough=2',
		'-Wendif-labels',
		'-Wstrict-aliasing=2',
		'-Woverflow',
		'-Walloca',
	],
	language: 'cpp',
)

logind_dep = dependency('lib' + get_option('logind-provider'), required: true)
wayland_dep = dependency('wayland-client', required: false)
wayland_protocols_dep = dependency('wayland-protocols', required: false)
wayland_scanner = find_program('wayland-scanner', required: false)

add_project_arguments('-D', 'HAVE_' + get_option('logind-provider').to_upper(), language: 'cpp')

wayland_sources = []
if wayland_dep.found() and wayland_protocols_dep.found() and wayland_scanner.found()
	add_project_arguments('-DHAVE_WAYLAND', language: 'cpp')

	wayland_protocols_dir = wayland_protocols_dep.get_variable(pkgconfig: 'pkgdatadir')
	idle_inhibit_xml = join_paths(wayland_protocols_dir, 'unstable', 'idle-inhibit', 'idle-inhibit-unstable-v1.xml')

	idle_inhibit_header = custom_target(
		'idle-inhibit-client-header',
		input: idle_inhibit_xml,
		output: 'idle-inhibit-unstable-v1-client-protocol.h',
		command: [wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@'],
	)

	idle_inhibit_code = custom_target(
		'idle-inhibit-protocol-code',
		input: idle_inhibit_xml,
		output: 'idle-inhibit-unstable-v1-protocol.c',
		command: [wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@'],
	)

	wayland_sources = [idle_inhibit_header, idle_inhibit_code]
endif

# Systemd service unit
if get_option('logind-provider') == 'systemd'
	dep_systemd = dependency('systemd')
	if dep_systemd.found()
		systemd_service_install_dir = dep_systemd.get_variable(pkgconfig: 'systemduserunitdir')
	else
		systemd_service_install_dir = join_paths(get_option('libdir'), 'systemd', 'user')
	endif
	conf_data = configuration_data()
	conf_data.set('bindir', join_paths(get_option('prefix'), get_option('bindir')))
	configure_file(
		configuration: conf_data,
		input: 'services/systemd/pipewire-audio-idle-inhibit.service.in',
		output: '@BASENAME@',
		install_dir: systemd_service_install_dir,
	)
endif

executable(
	'pipewire-audio-idle-inhibit',
	files(
		'./src/config.cpp',
		'./src/data.cpp',
		'./src/idle.cpp',
		'./src/main.cpp',
		'./src/pipewire.cpp',
	) + wayland_sources,
	include_directories: include_directories('include'),
	dependencies: [
		dependency('libpipewire-0.3'),
		logind_dep,
		wayland_dep,
	],
	install: true,
)

# Default config
install_data(
	'config/config.json',
	install_dir: join_paths(get_option('datadir'), 'pipewire-audio-idle-inhibit'),
)
