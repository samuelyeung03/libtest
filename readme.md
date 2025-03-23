git submodule update --init --recursive
cd include/x264
./configure --enable-shared --enable-static
make
cd ../../

modify path in CmakeList.txt
put input.yuv in /libtest

cmake ./CmakeLists.txt
make
./test_libx264
python3 compare.py
