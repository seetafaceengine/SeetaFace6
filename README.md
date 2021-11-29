# SeetaFace6

[![License](https://img.shields.io/badge/license-BSD-blue.svg)](LICENSE)

[[中文]()] [English] 

# 源码发布
该项目源码已经发布到 [SeetaFace6Open](https://github.com/SeetaFace6Open/index).

# 开放模块

`SeetaFace6`是中科视拓最新开放的商业正式级版本。突破了之前社区版和企业版版本不同步发布的情况，这次发布的v6版本正式与商用版本同步。

<div align=center>
<img src="./asserts/fas.jpg" width="310" height="180" />
</div>

此次开放版包含了一直以来人脸识别的基本部分，如人脸检测、关键点定位、人脸识别。同时增加了活体检测、质量评估、年龄性别估计。并且响应时事，开放了口罩检测以及戴口罩的人脸识别模型。

<div align=center>
<img src="./asserts/fr_mask.png" width="560" height="240" />
</div>


对比于SeetaFace2，我们开放版采用了商用版最新的推理引擎TenniS，ResNet50的推理速度，从SeetaFace2在I7的8FPS提升到了20FPS。同时人脸识别训练集也大幅度提高，SeetaFace6人脸识别数据量增加到了上亿张图片。

为了应对不同级别的应用需求，SeetaFace6将开放三个版本模型：

模型名称           | 网络结构       | 速度（I7-6700） | 速度（RK3399） | 特征长度
-|-|-|-|-
通用人脸识别       | ResNet-50      | 57ms           | 300ms         | 1024
带口罩人脸识别     | ResNet-50      | 34ms           | 150ms         | 512
通用人脸识别（小） | Mobile FaceNet | 9ms            | 70ms          | 512

作为能力兼容升级，SeetaFace6仍然能够给众多人脸识别应用提供业务能力。

<div align=center>
<img src="./asserts/app_matrix.png" width="600" height="320" />
</div>

同时该套算法适用于高精度的服务器部署外，也可以终端设备上很好的适应运行。
 
<div align=center>
<img src="./asserts/endpoints.png" width="680" height="180" />
</div>

此次开放版将开放标准C++开发接口的，包含x86和ARM架构支持，逐步开放Ubuntu、CentOS、macOS、Android、IOS的支持。同时仍然保持了SeetaFace优良传统，不依赖任何第三方库。

<div align=center>
<img src="./asserts/api_matrix.png" width="360" height="180" />
</div>

# 下载地址

### 百度网盘

开发包：  
Windows: [Download](https://pan.baidu.com/s/1_rFID6k6Istmu8QJkHpbFw) code: `iqjk`. Patch: 1. x86 pentium support [Download](https://pan.baidu.com/s/1RsXdg2h4Yq-bILdyVSTXDA) code: `0vn3`.  
Ubuntu1604: [Download](https://pan.baidu.com/s/1tOq12SdpUtuybe48cMuwag) code: `lc44`  
CentOS7: [Download](https://pan.baidu.com/s/1-U02a--Xjt-Jvi2QWI-9vQ) code: `1i62`  
Android: [Download](https://pan.baidu.com/s/1nGm5VB2D8OZOlZgcABGA7g) code: `7m2h`  
macOS: [Comming soon]  
IOS: [Download](https://pan.baidu.com/s/1-jKlCpVHoml9TmXq77SXxg) code: `t14x`, [Example](https://pan.baidu.com/s/159EVG8eqX2hPDeu1IrQaqg) code: `dund`.  
ARM-Ubuntu1604(RK3399): [Download](https://pan.baidu.com/s/16fMkI5K02k0TEAOGvIsPuw) code: `wi4q`.


模型文件：  
Part I: [Download](https://pan.baidu.com/s/1LlXe2-YsUxQMe-MLzhQ2Aw) code: `ngne`, including: `age_predictor.csta`, `face_landmarker_pts5.csta`, `fas_first.csta`, `pose_estimation.csta`, `eye_state.csta`, `face_landmarker_pts68.csta`, `fas_second.csta`, `quality_lbn.csta`, `face_detector.csta`, `face_recognizer.csta`, `gender_predictor.csta`, `face_landmarker_mask_pts5.csta`, `face_recognizer_mask.csta`, `mask_detector.csta`.   
Part II: [Download](https://pan.baidu.com/s/1xjciq-lkzEBOZsTfVYAT9g) code: `t6j0`，including: `face_recognizer_light.csta`.  

### DropBox

[Comming soon]

# 使用入门

关于基本的接口使用，请参见教程：
[《SeetaFace 入门教程》](http://leanote.com/blog/post/5e7d6cecab64412ae60016ef)，github上有同步[文档源码](https://github.com/seetafaceengine/SeetaFaceTutorial)。

人脸识别的完整示例Demo见 [example/qt](./example/qt)。

在每个压缩包的文档中都包含了对应平台上的调用示例，请解压对应平台压缩包后分别获取。

关于版本号的额外说明，该开放版本立项的时候，就是作为社区版v3发布，而执行过程中调整至发布版本为商用版本v6。这个版本不统一是因为商用版迭代的版本管理和社区版不统一造成的。现在统一版本为v6。但是项目过程中还是存在`SeetaFace3`的表述，大家不同担心，v6和v3其实就是一个版本。

# 接口文档

各模块接口参见 [docs](./docs)

# 开发者社区

欢迎开发者加入 SeetaFace 开发者社区，请先加 SeetaFace 小助手微信，经过审核后邀请入群。

![QR](./asserts/QR.png)

# 联系我们

`SeetaFace` 开放版可以免费用于商业和个人用途。如果需要更多的商业支持，请联系商务邮件 bd@seetatech.com。

