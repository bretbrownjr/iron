require 'rake/clean'

BIN_NAME = 'iron'

SRC_DIR = 'src'

directory BIN_DIR = 'bin'
CLOBBER.include BIN_DIR

directory OBJ_DIR = 'obj'
CLEAN.include OBJ_DIR
OBJ_FLAGS=['c','-std=c++11','Wall','Werror','Wextra','pedantic','g','I./include']

bin = File.join BIN_DIR,BIN_NAME
objs = []

def deps(source_file)
  all_flags = OBJ_FLAGS + ['M','MM']
  command = "g++ #{all_flags.map{|f| "-#{f}"}.join(' ')} #{source_file} 2>&1"
  output = `#{command}`
  if $?.exitstatus == 1
    print "#{command}\n#{output}"
    fail("failed to derive the dependencies of #{source_file}")
  end
  source_deps = output.split(/(\\|\s|\n)+/) # Split based on whitespace and /
  source_deps.shift # Ignore the object file declaration
  source_deps.delete_if {|str| str =~ /\s+/ } # Remove garbage
  source_deps
end

# Will return an object path or fail
def decl_obj(name)
  src = File.join SRC_DIR,"#{name}.cpp"
  fail "Cannot find #{src}" unless File.exists?(src)
  obj = File.join OBJ_DIR,"#{name}.o"
  file obj => deps(src) + [OBJ_DIR] do
    sh "g++ -o#{obj} #{OBJ_FLAGS.map{|f|"-#{f}"}.join(' ')} #{src}"
  end
end

objs << decl_obj('main')
objs << decl_obj('print')

file bin => [objs, BIN_DIR].flatten do
  sh "g++ -o#{bin} #{objs.join(' ')}"
end

desc 'Builds iron (default task)'
task :build => bin

examples = FileList['./examples/*']

task :test => :build do
  examples.each do |example|
    command = "#{bin} #{example} 2>&1"
    puts "== building #{example}"
    output = `#{command}`
    code = $?.exitstatus
    unless code == 0
      fail "failed to build #{example}: exit code #{code}"
    end
    # TODO: Run the compiled executable
  end
end

task :default => :test

