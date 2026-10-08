# One executable per example/<name>.cpp, run with `rake example` or `rake example[<name>]`.
examples = [
  # Search, search archive and pagination combined
  'google_example',
  # One example per API endpoint
  'search_example',
  'html_example',
  'markdown_example',
  'location_example',
  'search_archive_example',
  'account_example',
  # Other search engines
  'baidu_example',
  'bing_example',
  'duckduckgo_example',
  'ebay_example',
  'google_maps_example',
  'naver_example',
  'walmart_example',
  'yahoo_example',
  'yandex_example',
  'youtube_example',
]

foreach name : examples
  executable(name, name + '.cpp',
             install : false,
             dependencies : [libserpapi_dep])
endforeach
