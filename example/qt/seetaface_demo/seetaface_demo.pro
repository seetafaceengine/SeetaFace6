#-------------------------------------------------
#
# Project created by QtCreator 2020-03-16T14:40:38
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = seetaface_demo
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    videocapturethread.cpp \
    inputfilesprocessdialog.cpp \
    resetmodelprocessdialog.cpp

HEADERS += \
        mainwindow.h \
    videocapturethread.h \
    inputfilesprocessdialog.h \
    resetmodelprocessdialog.h

FORMS += \
        mainwindow.ui

#windows adm64:

#INCLUDEPATH += C:/thirdparty/opencv4.2/build/include \
#               C:/study/SF3.0/sf3.0_windows/sf3.0_windows/include


#CONFIG(debug, debug|release) {
#LIBS += -LC:/thirdparty/opencv4.2/build/x64/vc14/lib -lopencv_world420d \
#        -LC:/study/SF3.0/sf3.0_windows/sf3.0_windows/lib/x64 -lSeetaFaceDetector600d -lSeetaFaceLandmarker600d \
#        -lSeetaFaceAntiSpoofingX600d -lSeetaFaceTracking600d -lSeetaFaceRecognizer610d \
#        -lSeetaQualityAssessor300d -lSeetaPoseEstimation600d

#} else {
#LIBS += -LC:/thirdparty/opencv4.2/build/x64/vc14/lib -lopencv_world420 \
#        -LC:/study/SF3.0/sf3.0_windows/sf3.0_windows/lib/x64 -lSeetaFaceDetector600 -lSeetaFaceLandmarker600 \
#        -lSeetaFaceAntiSpoofingX600 -lSeetaFaceTracking600 -lSeetaFaceRecognizer610 \
#        -lSeetaQualityAssessor300 -lSeetaPoseEstimation600
#}

#linux:
INCLUDEPATH += /wqy/tools/opencv4_home/include/opencv4 \
               /wqy/seeta_sdk/SF3/libs/SF3.0_v1/include

LIBS += -L/wqy/tools/opencv4_home/lib -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs \
        -L/wqy/seeta_sdk/SF3/libs/SF3.0_v1/lib64 -lSeetaFaceDetector600 -lSeetaFaceLandmarker600 \
        -lSeetaFaceAntiSpoofingX600 -lSeetaFaceTracking600 -lSeetaFaceRecognizer610 \
        -lSeetaQualityAssessor300 -lSeetaPoseEstimation600 -lSeetaAuthorize -ltennis

RESOURCES += \
    face_resource.qrc
