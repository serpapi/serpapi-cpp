# automatic class generations
# 

task :default => [:init, :build, :run]

task :clean do
	sh('rm -rf build/')
end

# initialize meson build
task :setup do
	sh('meson setup --wipe build')
end

# build application
task :build => FileList['src/*.hpp', 'src/*.cpp'] do
	sh('meson compile -C build -v')
	sh('cd build && ninja')
end

task :test do
	sh('ninja -C build test')
end

# run application
task :run do
	sh('./build/example/google_example')
	sh('./build/oobt/oobt')
end

# tested on AARCH64 and x86
task :install_linux do
	sh('sudo apt update -y && sudo apt install -f -y build-essential meson pkg-config curl cmake meson ninja-build libcurl4-openssl-dev rapidjson-dev googletest-dev')
end

# tested on Apple M1 aarch64
task :install_apple do
	sh('brew install meson pkg-config curl cmake meson ninja rapidjson googletest')
end

task readme: ['README.md.erb'] do
  `erb -T '-' README.md.erb > README.md`
end