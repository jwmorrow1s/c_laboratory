#!/usr/bin/env bash

debug_set=''
cflags="-Wextra -Wall -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wwrite-strings -Waggregate-return -Wcast-qual -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code"

function parse_args () {
  for arg in "$@"; do
    if [ "$arg" == "--debug" ]; then
      cflags="$cflags -O0 -g -fno-eliminate-unused-debug-symbols" 
      debug_set='1'
    fi
    if [ "$arg" == "--prod" ]; then
      cflags="$cflags -O3" 
    fi
  done
}

function build () {

  # define variables
  echo "cflags = $cflags"
  
  ## define rules
  echo "rule cc"
  echo "  depfile = \$out.d"
  echo "  command = gcc -MD -MF \$out.d \$cflags -c \$in -o \$out"

  echo "rule link"
  echo "  command = gcc -o \$out \$in \$libs"

  # define build recipes
  build_all_c_files_in_dirs 'src' 'include'

  echo "build main: link $(list_all_obj_files 'src' 'include')"
}

function build_all_c_files_in_dirs(){
  local dirs;dirs="$@"
  for dir in $dirs; do 
    for cfile in $(ls "$dir"/*.c); do
      echo "build $dir/$(basename $cfile .c).o: cc $cfile"
    done
  done
}

function list_all_c_files_one_line(){
  local dirs;dirs="$@"
  for dir in $dirs; do 
    for cfile in $(ls "$dir"/*.c); do
      printf "$cfile "
    done
  done
}

function list_all_h_files_one_line(){
  local dirs;dirs="$@"
  for dir in $dirs; do 
    for hfile in $(ls "$dir"/*.h); do
      printf "$hfile "
    done
  done
}

function list_all_obj_files(){
  local dirs;dirs="$@"
  local ret;ret=""
  for dir in $dirs; do 
    ret="$ret $(ls $dir/*.c | xargs -I__ basename __ .c | xargs -I__ echo $dir/__.o | paste -sd ' ' -)"
  done
  printf "%s" "$ret"
}

function clean_sh(){
  echo "#! /usr/bin/env bash"
  echo "ninja -t clean"
  echo "rm -f build.ninja lint.sh clean.sh memcheck.sh debug.sh format.sh vgcore*"
}

function lint(){
  echo "#! /usr/bin/env bash"
  echo "echo 'running cppcheck...'"
  echo "cppcheck --std=c99 --suppress=missingIncludeSystem --suppress=checkersReport --enable=all --enable=style --error-exitcode=1 --language=c ./src"
  echo "echo 'cppcheck...done'"
}

function memcheck(){
  echo "#! /usr/bin/env bash"
  echo "valgrind --leak-check=full --track-origins=yes --error-exitcode=1 ./main"
}

function debug(){
  echo "#! /usr/bin/env bash"
  if [ "$debug_set" ]; then
    echo "if [ -f "./main" ]; then "
    echo "  gdb ./main"
    echo "else"
    echo "  echo 'main executable not found. Run \"ninja\" first'"
    echo "fi"
  else
    echo "echo 'not a debug build'"
    echo "echo 'run \"sh gen-build.sh --debug\" and run this again.'"
  fi
}

function format() {
  echo "#! /usr/bin/env bash"
  echo "clang-format -i $(list_all_c_files_one_line ./src ./include) $(list_all_h_files_one_line ./include)"
}

parse_args "$@"

build > build.ninja
clean_sh > clean.sh
lint > lint.sh
memcheck > memcheck.sh
debug > debug.sh 
format > format.sh 
