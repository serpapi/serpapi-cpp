project('serpapi', 'cpp',
  version : '1.0.0',
  default_options : ['warning_level=3', 'cpp_std=c++17'])

compiler = meson.get_compiler('cpp')
add_project_arguments(['-Wno-deprecated-declarations', '-funsigned-char'], language : 'cpp')
# Single source of truth for the version reported in every request (source=serpapi-cpp:<version>).
add_project_arguments('-DSERPAPI_VERSION="@0@"'.format(meson.project_version()), language : 'cpp')

incdirs = include_directories('src')

# RapidJSON types appear in the public API, so the library and everything that
# includes serpapi.hpp must use the same RapidJSON headers.
if get_option('system_rapidjson')
  rapidjson_dep = dependency('RapidJSON', include_type : 'system')
else
  rapidjson_dep = subproject('rapidjson', default_options : ['install_headers=true']).get_variable('rapidjson_dep')
endif

libserpapi = library('serpapi', 
                        [
                          'src/serpapi.cpp', 
                          'src/callback.cpp',
                          'src/query.cpp',
                          'src/response.cpp'
                        ],
                        dependencies : [
                          dependency('libcurl'),
                          rapidjson_dep
                        ],
                        include_directories : incdirs,
                        # soversion is the ABI major: bump it on any binary-incompatible change
                        # so the loader rejects mismatched binaries instead of failing on a symbol.
                        version : meson.project_version(),
                        soversion : '1',
                        install : true)

install_headers('src/serpapi.hpp', subdir : 'serpapi')

# The bundled RapidJSON is installed under include/serpapi, which the 'serpapi'
# subdir already puts on the consumer's include path, so it must not also pull
# in a system RapidJSON via Requires.
pkg = import('pkgconfig')
pkg.generate(libserpapi,
             description : 'C++ library for SerpApi',
             version : meson.project_version(),
             name : 'serpapi',
             requires : get_option('system_rapidjson') ? ['RapidJSON'] : [],
             requires_private : ['libcurl'],
             subdirs : 'serpapi')

libserpapi_dep = declare_dependency(include_directories : incdirs, link_with : libserpapi,
                                    dependencies : rapidjson_dep)

subdir('oobt')
subdir('example')

if get_option('tests')
  test_src = files('test/test_main.cpp', 'test/test_serpapi.cpp', 'test/test_unit.cpp')
  test_deps = [
    libserpapi_dep,
    dependency('gtest'),
    dependency('threads')
  ]

  e = executable('test_compiled', test_src, dependencies : test_deps)
  test('gtest', e, protocol : 'gtest')
endif
