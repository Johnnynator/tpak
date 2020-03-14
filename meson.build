project('tpak', 'c',
  version : '0.1',
  default_options : ['warning_level=3'],
  license: 'UNLICENSE')

cc = meson.get_compiler('c')
if cc.has_argument('-Wmultichar')
  add_project_arguments('-Wno-multichar', language : 'c')
endif
if cc.has_argument('-Wfour-char-constants')
  add_project_arguments('-Wno-four-char-constants', language : 'c')
endif

zlib_dep = dependency('zlib', fallback : ['zlib', 'zlib_dep'])

sources = [ 'main.c', 'tpak.c' ]

executable('tpak',
           sources: sources,
           dependencies : [zlib_dep],
           install : true)
