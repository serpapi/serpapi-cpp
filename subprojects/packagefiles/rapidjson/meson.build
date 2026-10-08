# Header-only RapidJSON, pinned by ../../rapidjson.wrap.
project('rapidjson', 'cpp',
  version : '1.1.0-24b5e7a',
  license : 'MIT')

rapidjson_inc = include_directories('include', is_system : true)
rapidjson_dep = declare_dependency(include_directories : rapidjson_inc)

# Installed under the serpapi include subdirectory so that code compiled against
# the installed serpapi.hpp uses the same RapidJSON as the compiled library.
if get_option('install_headers')
  install_subdir('include/rapidjson', install_dir : get_option('includedir') / 'serpapi')
  install_data('license.txt', install_dir : get_option('datadir') / 'licenses' / 'serpapi',
               rename : 'rapidjson-license.txt')
endif
