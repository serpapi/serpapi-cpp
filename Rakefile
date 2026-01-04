# Automate serpapi C++ library
# 

task default: [:clean, :setup, :build, :test, :lint, :coverage, :example, :oobt] do
	puts "all good ready to release"
	puts " next step: rake release"
end

task :clean do
	sh('rm -rf build/')
end

desc('initialize meson build')
task :setup do
	sh('meson setup --wipe build')
end

desc('build library')
task :build => FileList['src/*.hpp', 'src/*.cpp'] do
	sh('meson compile -C build')
	sh('cd build && ninja')
end

desc('run test suite')
task :test do
	sh('ninja -C build test')
end

desc('run static analysis')
task :lint do
	rapidjson_inc = `pkg-config --variable=includedir rapidjson`.strip
	sh("cppcheck src/*.cpp test/*.cpp oobt/*.cpp example/*.cpp -I src -I #{rapidjson_inc} --enable=warning,style,performance,portability --suppress='*:*/rapidjson/*' --suppress=missingIncludeSystem --suppress=unusedFunction --error-exitcode=1")
end

desc('generate coverage report')
task :coverage do
	sh('meson setup build --wipe -Db_coverage=true')
	sh('ninja -C build')
	sh('ninja -C build test')
	# Use gcovr directly to focus on src/ and provide a cleaner report
	sh('gcovr -r . --filter src/')
	sh('gcovr -r . --filter src/ -s')
end

task readme: ['README.md.erb'] do
  `erb -T '-' README.md.erb > README.md`
end

desc('generate documentation')
task doc: [:readme]

desc('run examples under build/example')
task :example do
	Dir.glob('build/example/*').each do |file|
		next if file.end_with?('.p')
		sh(file)
	end
end

desc('run oobt under build/oobt')
task :oobt do
	sh('./build/oobt/oobt')
end

desc('release current library')
task :release do
	puts("TODO implement release")
end

namespace :install do
	desc('install dependency on debian AARCH64 and x86 [tested]')
	task :linux do
		sh('sudo apt update -y && sudo apt install -f -y build-essential meson pkg-config curl cmake meson ninja-build libcurl4-openssl-dev rapidjson-dev googletest cppcheck gcovr lcov')
	end

	desc('install dependency on Apple M1 aarch64 [tested]')
	task :apple do
		sh('brew install meson pkg-config curl cmake meson ninja rapidjson googletest cppcheck gcovr lcov')
	end
end
