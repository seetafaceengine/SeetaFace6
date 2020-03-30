SeetaFaceDemo depend on opencv4 (or opencv3) and  SeetaTech.com SF3.0 lib

open seetaface_demo.pro, modify INCLUDEPATH parameter and LIBS parameter.
INCLUDEPATH add opencv header files path and SF3.0 header files path.
LIBS add opencv libs and SF3.0 libs. 


you modify and save seetaface_demo.pro, then must run qmake

example:

LINUX:
```
INCLUDEPATH += /wqy/tools/opencv4_home/include/opencv4 \
               /wqy/seeta_sdk/SF3/libs/SF3.0_v1/include



LIBS += -L/wqy/tools/opencv4_home/lib -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs \
        -L/wqy/seeta_sdk/SF3/libs/SF3.0_v1/lib64 -lSeetaFaceDetector600 -lSeetaFaceLandmarker600 -ltennis \
        -lSeetaFaceAntiSpoofingX600 -lSeetaAuthorize -lSeetaFaceTracking600 -lSeetaFaceRecognizer610 \
        -lSeetaQualityAssessor300 -lSeetaPoseEstimation600
```

WINDOWS:
```
INCLUDEPATH += c:/opencv4_home/build/include/opencv4 \
               c:/seeta_sdk/SF3/libs/SF3.0_v1/include

CONFIG(debug, debug|release) {


LIBS += -Lc:/opencv4_home/build/x64/lib -lopencv_core400d -lopencv_imgproc400d -lopencv_highgui400d -lopencv_videoio400d -lopencv_imgcodecs400d  \
        -Lc:/seeta_sdk/SF3/libs/SF3.0_v1/lib64/debug -lSeetaFaceDetector600d -lSeetaFaceLandmarker600d -ltennisd \
        -lSeetaFaceAntiSpoofingX600d -lSeetaAuthorized -lSeetaFaceTracking600d -lSeetaFaceRecognizer610d \
        -lSeetaQualityAssessor300d -lSeetaPoseEstimation600d
} else {

LIBS += -Lc:/opencv4_home/build/x64/lib -lopencv_core400 -lopencv_imgproc400 -lopencv_highgui400 -lopencv_videoio400 -lopencv_imgcodecs400 \
        -Lc:/seeta_sdk/SF3/libs/SF3.0_v1/lib64/release -lSeetaFaceDetector600 -lSeetaFaceLandmarker600 -ltennis \
        -lSeetaFaceAntiSpoofingX600 -lSeetaAuthorize -lSeetaFaceTracking600 -lSeetaFaceRecognizer610 \
        -lSeetaQualityAssessor300 -lSeetaPoseEstimation600

}
```

Note:
before run seetaface_demo, download and save SF3.0 models to seetaface_demo's directory models

