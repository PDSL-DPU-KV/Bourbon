mkdir build
mkdir evaluation
cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE -DNDEBUG_SWITCH=ON
make -j
cd ..
