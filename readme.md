git submodule update --init --recursive
cd include/x264
./configure --enable-shared --enable-static
make
cd ../../

cmake CMakeLists.txt
cmake -DB=Buildname //where Buildname is your name for the x264 build
make

you can repeat the above follow steps to have mutiple version of x264
cd include/x264
modify x264
make
cd ../../
cmake -DB=Buildname //where Buildname is your name for the x264 build, the output will store in out/name_you_just_put
make

put yuv or mp4 in /input

./auto.sh is built for auto runing test
usage: 
    --r Set the runloops value
    --fixed Set the mode to fixed complexity
    --all Set the mode to run both fixed complexity and dace
    --B Select the build to use, empty means all build
    --fps Add FPS value to the list, you can have more than 1 value
    --br Add bitrate value to the list, you can have more than 1 value
    --force rewrite exsiting value, skip already test value if not enable
    --no_record delete h264 for saving storge
    --c select which complexity level you want(must use with fixed)

example: ./auto.sh --all --r 1 --fps 30 --fps 60 --fps 90 --fps 120 --br 1000 --br 2000 --br 4000 --br 10000 --fixed --no_record --B DACE --no_record

!! if you deleted result folder, please rerun make, or no_record will not work