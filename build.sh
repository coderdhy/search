buildpath=./build

if [ -d "$buildpath" ]; then
  echo "$buildpath is a directory."
else 
  echo "$buildpath not exits create"
  mkdir $buildpath
fi
cd $buildpath
cmake ..

