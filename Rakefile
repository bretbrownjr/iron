require 'rake/clean'

BIN_NAME = 'iron'

directory BIN_DIR = 'bin'
directory OBJ_DIR = 'obj'

CLEAN.include OBJ_DIR
CLOBBER.include BIN_DIR

bin = File.join BIN_DIR,BIN_NAME

file 'obj/main.o' => ['src/main.cpp',OBJ_DIR] do
  sh 'g++ -c -oobj/main.o --std=c++11 -Wall -Werror -Wextra -pedantic -O3 src/main.cpp'
end

file bin => [BIN_DIR,'obj/main.o'] do
  sh "g++ -o#{bin} obj/main.o"
end

desc 'Builds iron (default task)'
task :build => bin

task :default => :build

