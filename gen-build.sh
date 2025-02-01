#!/usr/bin/env bash

function main () {
  local cflags
  cflags="-Wextra -Wall -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wwrite-strings -Waggregate-return -Wcast-qual -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code"

  # parse cli args
  for arg in "$@"; do
    if [ "$arg" == "--debug" ]; then
      cflags="$cflags -g -fno-eliminate-unused-debug-symbols" 
    fi
  done


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

function list_all_obj_files(){
  local dirs;dirs="$@"
  local ret;ret=""
  for dir in $dirs; do 
    ret="$ret $(ls $dir/*.c | xargs -I__ basename __ .c | xargs -I__ echo $dir/__.o | paste -sd ' ' -)"
  done
  printf "%s" "$ret"
}

main "$@" > build.ninja
