buildpath=./build

if [ -d "$buildpath" ]; then
  echo "$buildpath is a directory."
else 
  echo "$buildpath not exits create"
  mkdir $buildpath
fi
cd $buildpath

# compile os
SHELL_OS=$(uname -a | grep -io 'MINGW\|Linux\|Darwin' | head -1 | tr "[:upper:]" "[:lower:]")
echo "--current shell os $SHELL_OS"

if [ "$SHELL_OS" = "darwin" ]; then
  cmake .. -G "Xcode"
else
  cmake .. -G "Visual Studio 15 2017"
fi


