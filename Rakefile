# Automate serpapi C++ library
# 

desc('full check: clean build, tests with coverage, lint, google_example, demo')
task default: [:clean, :coverage, :lint] do
	# Only google_example: running every example costs ~17 searches.
	Rake::Task[:example].invoke('google_example')
	Rake::Task[:demo].invoke
	puts "all good ready to release"
	puts " next step: rake release"
end

desc('remove the build directory')
task :clean do
	sh('rm -rf build/')
end

def homebrew_prefix
  ENV['HOMEBREW_PREFIX'] || (system('command -v brew >/dev/null 2>&1') ? `brew --prefix`.strip : nil)
end

def pkg_config_env
  # Homebrew's curl formula is keg-only, so it must be listed explicitly or the
  # build silently falls back to the macOS SDK libcurl.
  prefix = homebrew_prefix
  pkg_paths = prefix ? [
     "#{prefix}/lib/pkgconfig",
     "#{prefix}/opt/rapidjson/lib/pkgconfig",
     "#{prefix}/opt/curl/lib/pkgconfig"
  ].select { |path| File.directory?(path) } : []

  # Append existing PKG_CONFIG_PATH
  existing_path = ENV['PKG_CONFIG_PATH']
  pkg_paths << existing_path if existing_path && !existing_path.empty?

  require 'shellwords'
  "PKG_CONFIG_PATH=#{Shellwords.escape(pkg_paths.join(':'))}"
end

# Include directory of the RapidJSON the build uses: the pinned subproject by
# default, or the system package (pkg-config name is case-sensitive on Linux).
def rapidjson_include
  bundled = Dir.glob('subprojects/rapidjson-*/include').first
  return bundled if bundled

  system_inc = `#{pkg_config_env} pkg-config --variable=includedir RapidJSON 2>/dev/null`.strip
  abort 'RapidJSON headers not found: run `rake setup` first to fetch the bundled copy.' if system_inc.empty?
  system_inc
end

# Extra meson options from the environment, e.g. CPP_STD=c++20 rake coverage
def meson_extra_args
  ENV['CPP_STD'] ? " -Dcpp_std=#{ENV['CPP_STD']}" : ''
end

desc('initialize meson build (set CPP_STD=c++20 to change the C++ standard)')
task :setup do
  sh("#{pkg_config_env} meson setup --wipe build -Dtests=true#{meson_extra_args}")
end

desc('build library')
task :build do
	sh('meson compile -C build')
end

desc('run test suite')
task :test do
	sh('ninja -C build test')
end

desc('run static analysis')
task :lint do
	require 'shellwords'
	sh("cppcheck src/*.cpp test/*.cpp demo/*.cpp example/*.cpp -I src -I #{Shellwords.escape(rapidjson_include)} --enable=warning,style,performance,portability --suppress='*:*/rapidjson/*' --suppress=missingIncludeSystem --suppress=unusedFunction --error-exitcode=1")
end

desc('run tests with coverage and print the report')
task :coverage do
	sh("#{pkg_config_env} meson setup build --wipe -Db_coverage=true -Dtests=true#{meson_extra_args}")
	sh('ninja -C build')
	sh('ninja -C build test')
	# -s appends the summary (lines/functions/branches) to the per-file table
	sh('gcovr -r . --filter src/ -s')
end

desc('generate API documentation with Doxygen (output: docs/)')
task :doc do
	abort 'doxygen not found: install it with `brew install doxygen` or `sudo apt install doxygen`.' unless system('command -v doxygen >/dev/null 2>&1')
	sh('doxygen Doxyfile')
end

desc('run all examples, or one: rake example[bing_example] (each search uses one credit)')
task :example, [:name] do |_task, args|
	examples = Dir.glob('build/example/*_example').select { |f| File.file?(f) && File.executable?(f) }.sort
	abort 'No examples built: run `rake setup build` first.' if examples.empty?

	if args[:name]
		path = "build/example/#{args[:name]}"
		unless examples.include?(path)
			abort "Unknown example '#{args[:name]}'. Available: #{examples.map { |f| File.basename(f) }.join(', ')}"
		end
		examples = [path]
	end

	# Run every example even if one fails, then report all failures.
	failed = examples.reject do |file|
		puts "== #{File.basename(file)}"
		system(file)
	end
	abort "Failed examples: #{failed.map { |f| File.basename(f) }.join(', ')}" unless failed.empty?
end

desc('run the demo under build/demo')
task :demo do
	sh('./build/demo/demo')
end

desc('release current library')
task release: [:default] do
  require 'shellwords'

  # Extract version from meson.build
  version_match = File.read('meson.build').match(/version\s*:\s*'([^']+)'/)
  abort "Unable to extract version from meson.build. Check version format." unless version_match

  version = version_match[1]
  tag = "v#{version}"

  puts "Releasing #{tag}..."

  # The README install link and changelog must name this version.
  readme = File.read('README.md')
  missing = []
  missing << "install link tags/#{tag}.tar.gz" unless readme.include?("tags/#{tag}.tar.gz")
  missing << "changelog entry for #{version}" unless readme.match?(/^ \* \[\d{4}-\d{2}-\d{2}\] #{Regexp.escape(version)} /)
  abort "README.md is missing the #{missing.join(' and ')}." unless missing.empty?

  require 'date'
  changelog_date = readme[/^ \* \[(\d{4}-\d{2}-\d{2})\] #{Regexp.escape(version)} /, 1]
  if changelog_date != Date.today.iso8601
    puts "WARNING: the README changelog dates #{version} #{changelog_date}, today is #{Date.today.iso8601}."
  end

  unless `git status --porcelain`.strip.empty?
    abort "Working directory is not clean. Please commit or stash changes."
  end

  head = `git rev-parse HEAD`.strip
  tag_exists = `git tag -l #{Shellwords.escape(tag)}`.strip == tag
  if tag_exists
    tagged = `git rev-parse #{Shellwords.escape("#{tag}^{commit}")}`.strip
    unless tagged == head
      abort "Tag #{tag} points to #{tagged[0, 7]}, not HEAD (#{head[0, 7]}). " \
            "Bump the version, or delete the tag with `git tag -d #{tag}` if it was never pushed."
    end
  end

  # Create the distribution package before tagging, so a failed build or test
  # does not leave a tag on a commit that was never released.
  # meson dist performs a build and test in a temporary directory, so it needs the env
  sh "#{pkg_config_env} meson dist -C build --include-subprojects"

  if tag_exists
    puts "Tag #{tag} already exists on HEAD. Skipping git tag."
  else
    sh "git tag -a #{Shellwords.escape(tag)} -m #{Shellwords.escape("Release #{tag}")}"
    puts "Created tag #{tag}."
  end

  puts "\nRelease #{tag} completed successfully!"
  puts "Next steps:"
  puts "1. git push origin #{tag}"
  puts "2. Upload build/meson-dist/serpapi-#{version}.tar.xz to GitHub Releases"
end

namespace :install do
	# RapidJSON is not listed: the build uses the pinned copy in subprojects/.
	desc('install dependency on debian AARCH64 and x86 [tested]')
	task :linux do
		sh('sudo apt update -y && sudo apt install -f -y build-essential meson pkg-config curl cmake ninja-build libcurl4-openssl-dev googletest cppcheck gcovr lcov')
	end

	desc('install dependency on Apple M1 aarch64 [tested]')
	task :apple do
		sh('brew install meson pkg-config curl cmake ninja googletest cppcheck gcovr lcov')
	end
end
