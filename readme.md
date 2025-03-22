git submodule update --init --recursive 
cd include/x264
git checkout DACE
./configure --enable-shared --enable-static 
make
cd ../../

modifly path in CmakeList.txt

cmake ./CmakeList.txt
make

put input.yuv in /libtest
./test_libx264