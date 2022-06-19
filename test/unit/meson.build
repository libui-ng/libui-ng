cmocka_deps = dependency('cmocka', version : '>=1.0.8')

libui_unit_sources = [
	'main.c',
]

executable('unit', libui_unit_sources,
	dependencies: [libui_binary_deps, cmocka_deps],
	link_with: libui_libui,
	gui_app: false,
	install: false)
