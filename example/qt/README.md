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
        -L/wqy/seeta_sdk/SF3/libs/SF3.0_v1/lib64 -lSeetaFaceDetector600 -lSeetaFaceLandmarker600  \
        -lSeetaFaceAntiSpoofingX600 -lSeetaFaceTracking600 -lSeetaFaceRecognizer610 \
        -lSeetaQualityAssessor300 -lSeetaPoseEstimation600 -lSeetaAuthorize -ltennis
```

WINDOWS:

1¡¢install vs2015
2¡¢install qt5.9.
   note:when select install components, checked msvc2015 64-bit
        after installed, confirm compile tool and build kits is msvc2015 64bit
        
        

3¡¢configure parameters:

   SF3.0_ROOT = C:/study/SF3.0/sf3.0_windows/sf3.0_windows
   OPENCV_ROOT = C:/thirdparty/opencv4.2/build
```
INCLUDEPATH += C:/thirdparty/opencv4.2/build/include \
               C:/study/SF3.0/sf3.0_windows/sf3.0_windows/include

CONFIG(debug, debug|release) {

LIBS += -LC:/thirdparty/opencv4.2/build/x64/vc14/lib -lopencv_world420d \
        -LC:/study/SF3.0/sf3.0_windows/sf3.0_windows/lib/x64 -lSeetaFaceDetector600d -lSeetaFaceLandmarker600d \
        -lSeetaFaceAntiSpoofingX600d -lSeetaFaceTracking600d -lSeetaFaceRecognizer610d \
        -lSeetaQualityAssessor300d -lSeetaPoseEstimation600d

} else {

LIBS += -LC:/thirdparty/opencv4.2/build/x64/vc14/lib -lopencv_world420 \
        -LC:/study/SF3.0/sf3.0_windows/sf3.0_windows/lib/x64 -lSeetaFaceDetector600 -lSeetaFaceLandmarker600 \
        -lSeetaFaceAntiSpoofingX600 -lSeetaFaceTracking600 -lSeetaFaceRecognizer610 \
        -lSeetaQualityAssessor300 -lSeetaPoseEstimation600

}
```

Note:



Before running seetaface_demo, please download and save SF3.0 models into seetaface_demo's directory models.
Then copy opencv_world420d.dll and all of SF3.0 lib directory's dll files and paste them into seetaface_demo directory
