<!--
 * @Description: In User Settings Edit
 * @Author: jiangyuxiang2
 -->

# atlas-wrapper
a simple, efficient, easy-to-use HUAWEI Ascend310 Atlas wrapper for cnn with c++ and python api. You will be able use atlas wrapper to fast demo or deploy your FP16 model with few lines of code!
```c++
//c++
// create Net
atlas::Net *net = new atlas::Net(modelPath, enginePath, deviceId);
// get Net information
net->getNetInfo()
// inferece
// accept input images as vector<cv::Mat> or vector<uint8_t> with vector<int> for image shape.
// recieve the output and outputDims from atlas::NetResult
net->inference(opencv_img_vec, results);
// net->inference(uint8_t_img_vec, image_shape_vec, results);

// destroy net
delete net;
```

```python
# python
# import
import pyatlas
# create net
net = pyatlas.net(modelPath, enginePath, deviceId)
# get net information
netInfo = net.getNetInfo()
# inference
result = net.DoInference(img, imgSize)
```

# Features
- [x] Support Atlas300. Make little change on CMakeLists.txt to support Atlas500, Atlas200SoC.
- [x] Resize in DVPP, so you can use images of any shape in a batch inference.
- [x] Auto alignUp for DVPP input.
- [x] Do mean and std opteration in the first layer (AIPP)
- [x] Dynamic graph.
- [x] High performance data transfer between host and device.
- [x] get net information such as batchSize, net input shapes, output dims and size.
- [x] Python api support.
- [x] Set device.

# System Requirements
Atlas DDK B893+ and driver

opencv_world 3.4+

for python api, python 2.x/3.x and numpy in needed

# Installation
Make sure you had install dependencies list above.
```bash
# clone project and submodule
git clone --recurse-submodules ...

cd atlaswrapper

bash build.sh A300 #(options: A500, A200)
```
then you can intergrate it into your own project with libatlasWrapper.so and Net.h, for python module, you get pyatlas.so
# Model Conersion
Please follow the details from the documents of Atlas300

# Demo
Please check the c++ & python demo on test folder.

# Liscense
AtlasWrapper is released under the [Apache 2.0 license].


# TODO
1. Add demo for model conversion.
2. Modify CMakeLists.txt to support A500, A200SoC.
