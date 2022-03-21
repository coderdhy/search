buildpath=./build
var1=$1
var2=$2

if [ -d "$buildpath" ]; then
  echo "$buildpath is a directory."
  if [ "$var2" = "rebuild" ]; then
    echo "rebuild delete && mkdir $buildpath"
    rm -rf $buildpath
    mkdir $buildpath
  fi
else 
  echo "$buildpath not exits create"
  mkdir $buildpath
fi
cd $buildpath

# compile os
SHELL_OS=$(uname -a | grep -io 'MINGW\|Linux\|Darwin' | head -1 | tr "[:upper:]" "[:lower:]")
echo "--current shell os $SHELL_OS"

if [ "${var1}" = "-p" ]; then
  if [ "$SHELL_OS" = "darwin" ]; then
    cmake .. -G "Xcode"
  else
    cmake .. -G "Visual Studio 15 2017"
  fi
elif [ "${var1}" = "release" ]; then
  if [ "$SHELL_OS" = "darwin" ]; then
    cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build ./
  else
    cmake .. -DCMAKE_BUILD_TYPE=Release -A Win32 && cmake --build ./
  fi
elif [ "${var1}" = "debug" ]; then
  if [ "$SHELL_OS" = "darwin" ]; then
    cmake .. -DCMAKE_BUILD_TYPE=Debug && cmake --build ./
  else
    cmake .. -DCMAKE_BUILD_TYPE=Debug  -A Win32 && cmake --build ./
  fi
fi




