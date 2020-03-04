project('tpak', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

cc = meson.get_compiler('c')
if cc.has_argument('-Wmultichar')
	add_project_arguments('-Wno-multichar', language : 'c')
endif
if cc.has_argument('-Wfour-char-constants')
	add_project_arguments('-Wno-four-char-constants', language : 'c')
endif

zlib_dep = dependency('zlib', fallback : ['zlib', 'zlib_dep'])

executable('tpak',
           [ 'main.c', 'tpak.c' ],
	   dependencies : zlib_dep,
           install : true)
