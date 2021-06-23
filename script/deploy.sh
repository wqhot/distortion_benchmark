#! /bin/bash

export PKG_CONFIG_LIBDIR=/home/wang/libs/cross/install/lib/pkgconfig
version="release"
if [ X$1 = X ]
    then
      echo "use release"
    else
      version=$1
fi
mkdir -p ../arm_build/${version}
cd ../arm_build/${version}
if [ $version = "release" ]
    then
      cmake -D BUILD_TYPE=RELEASE -D PLATFORM=ARM ../..
    else
      cmake -D BUILD_TYPE=DEBUG -D PLATFORM=ARM ../..
fi
make -j8
scp ./distortion_benchmark greatwall@192.168.50.101:/home/greatwall/zntk/bin/
