# Automate serpapi C++ library
# 

task :default => [:clean, :setup, :build, :run]

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

task readme: ['README.md.erb'] do
  `erb -T '-' README.md.erb > README.md`
end

desc('generate documentation')
task doc: [:readme]

desc('run examples under build/example')
task :example do
	Dir.glob('build/example/*').each do |file|
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
		sh('sudo apt update -y && sudo apt install -f -y build-essential meson pkg-config curl cmake meson ninja-build libcurl4-openssl-dev rapidjson-dev googletest')
	end

	desc('install dependency on Apple M1 aarch64 [tested]')
	task :apple do
		sh('brew install meson pkg-config curl cmake meson ninja rapidjson googletest')
	end
end
