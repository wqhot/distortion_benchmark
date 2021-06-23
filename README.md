反畸变调用在[./includes/utils/image_utils.hpp](https://github.com/wqhot/distortion_benchmark/blob/main/includes/utils/image_utils.hpp)第14行

运行例子：

```bash
./distortion_benchmark ~/Downloads/video -imshow 
```

video下载链接：
[https://motchallenge.net/sequenceVideos/MOT16-13-raw.webm](https://motchallenge.net/sequenceVideos/MOT16-13-raw.webm)

下载后，需要用ffmpeg将视频处理为图像序列:

```bash
ffmpeg -i MOT16-13-raw.webm %4d.bmp
```

libjibianv2.so（remap的映射关系矩阵）下载链接：

[ARM](https://wwi.lanzoui.com/ioNPSqmb8uh)

[x86](https://wwi.lanzoui.com/irNpIqmbcri)

