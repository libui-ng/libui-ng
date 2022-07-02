cmocka_deps = dependency('cmocka', version : '>=1.0.0', default_options: ['default_library=static'])

libui_unit_sources = [
	'main.c',
]

if libui_OS == 'windows'
	libui_unit_manifest = 'unit.manifest'
	if libui_mode == 'static'
		libui_unit_manifest = 'unit.static.manifest'
	endif
	libui_unit_sources += [
		windows.compile_resources('resources.rc',
			args: libui_manifest_args,
			depend_files: [libui_unit_manifest]),
	]
endif

executable('unit', libui_unit_sources,
	dependencies: [libui_binary_deps, cmocka_deps],
	link_with: libui_libui,
	gui_app: false,
	install: false)

