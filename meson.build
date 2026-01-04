project('serpapi', 'cpp',
  version : '0.2.0',
  default_options : ['warning_level=3', 'cpp_std=c++17'])

compiler = meson.get_compiler('cpp')
add_project_arguments(['-Wno-deprecated-declarations', '-funsigned-char'], language : 'cpp')

incdirs = include_directories('src')

libserpapi = library('serpapi', 
                        [
                          'src/serpapi.cpp', 
                          'src/callback.cpp'
                        ],
                        dependencies : [
                          dependency('libcurl'),
                          dependency('RapidJSON')
                        ],
                        include_directories : incdirs,
                        install : true)

install_headers('src/serpapi.hpp', 'src/callback.hpp', subdir : 'serpapi')

pkg = import('pkgconfig')
pkg.generate(libserpapi,
             description : 'C++ library for SerpApi',
             version : meson.project_version(),
             name : 'serpapi',
             requires : ['libcurl', 'RapidJSON'],
             subdirs : 'serpapi')

libserpapi_dep = declare_dependency(include_directories : incdirs, link_with : libserpapi)

subdir('oobt')
subdir('example')

# Configure the testing setup
test_src = files('test/test_main.cpp', 'test/test_serpapi.cpp')
test_deps = [
#  dependency('libcurl'),
  dependency('RapidJSON'),
  libserpapi_dep,
  dependency('gtest')
]
test_args = ['-Wno-missing-field-initializers','-g','-lgtest']

e = executable('test_compiled', test_src, dependencies : test_deps)
test('gtest', e, protocol : 'gtest')
