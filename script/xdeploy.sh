#! /bin/bash
version="release"
if [ X$1 = X ]
    then
      echo "use release"
    else
      version=$1
fi
mkdir -p ../build/${version}
cd ../build/${version}
if [ $version = "release" ]
    then
      cmake -D BUILD_TYPE=RELEASE ../..
    else
      cmake -D BUILD_TYPE=DEBUG ../..
fi

make -j8
# scp ./zntk_ar dev@192.168.0.40:/home/dev/zntk/bin/
# scp ./joy_test dev@192.168.0.40:/home/dev/zntk/bin/
# scp ../../config/setup.sh dev@192.168.0.40:/home/dev/zntk/config/
# scp ../../config/test_init.sh dev@192.168.0.40:/home/dev/zntk/config/
# scp ../../config/zntk_ar.service dev@192.168.0.40:/home/dev/zntk/config/
# scp ../../config/zntk.conf dev@192.168.0.40:/home/dev/zntk/config/
