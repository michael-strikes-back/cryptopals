#!/bin/sh

usage()
{
    echo "Usage: build.sh [release|debug]"
    exit 1
}

if [ $# -ne 1 ]
  then
    usage
fi

case $1 in
  [rR][eE][lL][eE][aA][sS][eE]) ;;
  [dD][eE][bB][uU][gG]) ;;
  *) usage
esac

cmake -G "CodeLite - Unix Makefiles" -DCMAKE_BUILD_TYPE=$1

