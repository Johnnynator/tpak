project('tpak', 'c',
  version : '0.1',
  default_options : ['warning_level=3'],
  license: 'UNLICENSE')

conf = configuration_data()

cc = meson.get_compiler('c')
if cc.has_argument('-Wmultichar')
  add_project_arguments('-Wno-multichar', language : 'c')
endif
if cc.has_argument('-Wfour-char-constants')
  add_project_arguments('-Wno-four-char-constants', language : 'c')
endif

zlib_dep = dependency('zlib', fallback : ['zlib', 'zlib_dep'])

luajit_dep = dependency('luajit', version: '>=2.1.0', static: true, required: get_option('luajit'))
conf.set('HAVE_LUAJIT', luajit_dep.found())

editline_dep = dependency('libedit', required: false)
conf.set('HAVE_READLINE', editline_dep.found())

sources = [ 'main.c', 'tpak.c' ]

if luajit_dep.found()
  luajit = find_program('luajit')
  lua_gen = generator(luajit,
                      output : '@BASENAME@.h',
		      arguments : ['-b', '@INPUT@', '@OUTPUT@'])

  sc_h = lua_gen.process('sc.lua')
  sources += ['lua.c', sc_h]
endif

configure_file(output : 'config.h',
               configuration : conf)

executable('tpak',
           sources: sources,
           dependencies : [zlib_dep, luajit_dep, editline_dep],
           install : true)
