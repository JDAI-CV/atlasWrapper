import sys
import numpy as np
import cv2
sys.path.append("../build")
import pyatlas


def test_atlas():
    modelPath = "./models/test9.om"
    enginePath = "../lib/"
    net = pyatlas.net(modelPath, enginePath, 0)
    netInfo = net.getNetInfo()
    print(netInfo.batchSize)
    img = cv2.imread("../images/testyolo_1080P_2.png")
    img = cv2.resize(img, (10,6))
    print(img.shape)
    imgFlat = img.flatten()
    print(imgFlat)

    imgSize = [img.shape[1], img.shape[0], img.shape[2]]
    imgSize = np.array(imgSize)
    print(imgSize)
    result = net.DoInference(imgFlat, imgSize)
    print(len(result))
    print(result[0].outputDims)

    # img = cv2.resize(img, (5,3))
    # imgFlat2 = img.flatten()
    # print(img)
    # c = np.concatenate((imgFlat, imgFlat2))
    # print(c)
if __name__ == "__main__":
    test_atlas()
