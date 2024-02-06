libui_qa_sources = [
	'qa.c',
	'main.c',
	'button.c',
	'checkbox.c',
	'entry.c',
	'label.c',
	'progressbar.c',
	'window.c',
]

if libui_OS == 'windows'
	libui_qa_manifest = 'qa.manifest'
	if libui_mode == 'static'
		libui_qa_manifest = 'qa.static.manifest'
	endif
	libui_qa_sources += [
		windows.compile_resources('resources.rc',
			args: libui_manifest_args,
			depend_files: [libui_qa_manifest]),
	]
endif

executable('qa', libui_qa_sources,
	dependencies: libui_binary_deps,
	link_with: libui_libui,
	gui_app: false,
	install: false)
