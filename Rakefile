require 'rake/clean'

BIN_NAME = 'iron'

SRC_DIR = 'src'

directory BIN_DIR = 'bin'
CLOBBER.include BIN_DIR

directory OBJ_DIR = 'obj'
CLEAN.include OBJ_DIR
OBJ_FLAGS=['c','-std=c++11','Wall','Werror','Wextra','pedantic','O3']

bin = File.join BIN_DIR,BIN_NAME
objs = []

# Will return an object path or fail
def decl_obj(name)
  src = File.join SRC_DIR,"#{name}.cpp"
  fail "Cannot find #{src}" unless File.exists?(src)
  obj = File.join OBJ_DIR,"#{name}.o"
  file obj => [src,OBJ_DIR] do
    sh "g++ -o#{obj} #{OBJ_FLAGS.map{|f|"-#{f}"}.join(' ')} #{src}"
  end
end

objs << decl_obj('main')

file bin => [objs, BIN_DIR].flatten do
  sh "g++ -o#{bin} obj/main.o"
end

desc 'Builds iron (default task)'
task :build => bin

task :default => :build

