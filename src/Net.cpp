/*
    Created by jyx
*/
#include <atomic>
#include <iostream>
#include <libgen.h>
#include <unistd.h>
#include <chrono>

#include "hiaiengine/ai_memory.h"
#include "hiaiengine/api.h"

#include <mutex>
#include <condition_variable>

#include <opencv2/opencv.hpp>

#include "Graph.hpp"
#include "Net.hpp"
#include "net_utils.h"

namespace atlas{
    
class  NetImp {
    private:
        int _inputNum;
        int _outputNum;
        int _device;
        
        int _input_image_size ;
        int _batchSize; 
        int _c ;
        int _h ;
        int _w ;

        int _alignedH;
        int _alignedW;

        Graph* _graph;

        bool initFlag = false;

        int getSendDataSize(int loop, int loop_amount, std::vector<cv::Mat>& img_vector);

        void alignData(std::vector<cv::Mat>& img_vector_ori, std::vector<cv::Mat>& img_vector_aligned);

        void setNetInfo(std::vector<NetResult>& netResult);

        void initRun();

    public:
        NetImp(std::string modelPath, std::string enginePath, int device);
        NetImp(std::string omFile, std::string outputFile);
        ~ NetImp();
        NetInfo getNetInfo();
        void inference(std::vector<cv::Mat>& image, std::vector<NetResult>& netResult);
        
};

NetImp::NetImp(std::string modelPath, std::string enginePath, int device) {   
    _graph = new Graph(modelPath, enginePath, device);
    initRun();
}

NetImp::~NetImp() {   
    delete _graph;
}

void NetImp::setNetInfo(std::vector<NetResult>& netResult) {
    _batchSize = netResult[0].batchSize; 
    _c = netResult[0].channels;
    _h = netResult[0].height;
    _w = netResult[0].width;
    _outputNum = netResult[0].outputNum;
    _inputNum = 1;
    _alignedH = alignUp(_h, DVPP_STRIDE_HEIGHT);
    _alignedW = alignUp(_w, DVPP_STRIDE_WIDTH);
};

int NetImp::getSendDataSize(int loop, int loop_amount, std::vector<cv::Mat>& img_vector) {
     int size = 0 ;
    if (loop == (loop_amount - 1)) {
        for (size_t i = 0; i < _batchSize; i++) {   
            if(loop * _batchSize+i < img_vector.size()) { 
                auto img = img_vector[loop * _batchSize+i];
                size += getImgAndHeaderSize(img.cols, img.rows, img.channels());
            }
            else { //if out of bound, add fake image
                size += getImgAndHeaderSize(_w, _h, 3);
            } 
        }   
    }else {
        for (size_t i = 0; i < _batchSize; i++) {   
            auto img = img_vector[loop * _batchSize+i];
            size += getImgAndHeaderSize(img.cols, img.rows, img.channels());
        }
    }
    return size;
};

void NetImp::initRun() {
    std::vector<NetResult> netResult;

    HIAI_StatusT hiai_ret = HIAI_OK;
    int initW, initH, initC;
    initW = 64;
    initH = 64;
    initC = 3;
    int size = getImgAndHeaderSize(initW, initH, initC);

    uint8_t* data = nullptr;
    uint32_t offset = 0;
    hiai_ret = hiai::HIAIMemory::HIAI_DMalloc((uint32_t)size, (void*&)data);
    cv::Mat fake = cv::Mat::zeros(cv::Size(initW, initH), CV_8UC3);
    memcpyToSendData(fake, fake, offset, data);

    std::mutex mtx;
    std::condition_variable cv;

    std::shared_ptr<netInputRawHigh> inputRaw = std::make_shared<netInputRawHigh>();
    inputRaw->batchSize = 1;
    inputRaw->mtx = &mtx;
    inputRaw->cv = &cv;
    inputRaw->host_result_ptr = (void *)&netResult;
    inputRaw->size = offset;
    inputRaw->rawData.reset(data, Dnofree); 
    {
        std::unique_lock <std::mutex> lck(mtx);
        _graph->sendData(inputRaw);
        cv.wait(lck);
    }
    setNetInfo(netResult);
    std::cout<<"initRun end;"<<"\n";
}

void NetImp::alignData(std::vector<cv::Mat>& img_vector_ori, std::vector<cv::Mat>& img_vector_aligned) {
    for(auto img:img_vector_ori) {        
       int alignmentWidth = alignUp(img.cols, DVPP_STRIDE_WIDTH);
       int alignmentHeight = alignUp(img.rows, DVPP_STRIDE_HEIGHT);
       cv::Mat img_pad;
       int pad_right = alignmentWidth - img.cols;
       int pad_bottom = alignmentHeight - img.rows;
       if(pad_right == 0 && pad_bottom ==0) {
           img_vector_aligned.push_back(img);
       }
       else {
           cv::Mat img_pad;
           cv::copyMakeBorder(img, img_pad, 0, pad_bottom, 0, pad_right, cv::BORDER_REPLICATE );
           img_vector_aligned.push_back(img_pad);
       }
    }
}

void NetImp::inference(std::vector<cv::Mat>& img_vector_ori, std::vector<NetResult>& netResult) {   
    HIAI_StatusT hiai_ret = HIAI_OK;
    int amount = img_vector_ori.size();

    std::vector<cv::Mat> img_vector;
    alignData(img_vector_ori, img_vector);
    
    int loop_amount = getLoopAmount(amount, _batchSize); 
    
    for (int loop=0; loop < loop_amount; loop++) {   
        uint8_t* data = nullptr;
        int size = 0 ;
        size = getSendDataSize(loop, loop_amount, img_vector);

        //device malloc
        hiai_ret = hiai::HIAIMemory::HIAI_DMalloc((uint32_t)size, (void*&)data); //uint32_t
        if (data == nullptr){
            printf("[] DMalloc fail\n");
            data = new uint8_t[size];
        }

        uint32_t offset = 0;

        // memcpy to data
        if (loop == (loop_amount - 1)) { //the last round or not
            for (size_t i = 0; i < _batchSize; i++) {
                if(loop * _batchSize+i<img_vector.size()) {
                    auto img = img_vector[loop * _batchSize + i];
                    auto img_ori = img_vector_ori[loop * _batchSize + i];
                    memcpyToSendData(img, img_ori, offset, data);
                }
                else{ //add fake data
                    cv::Mat fake = cv::Mat::zeros(cv::Size(_alignedW, _alignedH), CV_8UC3);
                    memcpyToSendData(fake, fake, offset, data);
                }
            }
        }else{
            for (size_t i = 0; i < _batchSize; i++) {
                auto img = img_vector[loop * _batchSize + i];
                auto img_ori = img_vector_ori[loop * _batchSize + i];
                memcpyToSendData(img, img_ori, offset, data);
            }
        }

        std::mutex mtx;
        std::condition_variable cv;

        std::shared_ptr<netInputRawHigh> inputRaw = std::make_shared<netInputRawHigh>();
        inputRaw->batchSize = _batchSize;
        inputRaw->mtx = &mtx;
        inputRaw->cv = &cv;
        inputRaw->host_result_ptr = (void *)&netResult;
        inputRaw->size = offset;
        inputRaw->rawData.reset(data, Dnofree); 
        {
            std::unique_lock <std::mutex> lck(mtx);
            _graph->sendData(inputRaw);
            cv.wait(lck);
        }
    }
}

NetInfo NetImp::getNetInfo(){
    NetInfo info;
    info.batchSize = _batchSize;
    info.channels = _c;
    info.height = _h;
    info.width = _w;

    info.inputNum;
    info.outputNum = _outputNum;
    info.inputNum = 1;
    return info;
};

Net::Net(std::string modelPath, std::string enginePath, int device=0) {
   pImpl_ = new NetImp(modelPath,enginePath,device);
};

Net::~Net() {
    delete pImpl_;
};

NetInfo Net::getNetInfo() {
    return pImpl_ ->getNetInfo();
};

void Net::inference(std::vector<cv::Mat>& image, std::vector<NetResult>& netResult) {
    pImpl_->inference(image, netResult);
};

void Net::inference(std::vector<uint8_t>& imgData, std::vector<int>& dataSizeVec, std::vector<NetResult>& netResult) {
    // TODO
    if (dataSizeVec.size() % 3 != 0 ) {
        std::cout<<"data size is wrong!"<<std::endl;
    }
    std::vector<cv::Mat> image;
    int offset = 0;
    for(int i = 0; i < dataSizeVec.size() / 3; i++) {
        int width = dataSizeVec[i * 3];
        int height = dataSizeVec[i * 3 +1];
        int channel = dataSizeVec[i * 3 + 2];
        int size = width * height * channel;
        cv::Mat tmpImage;
        if(channel == 3) {
            tmpImage = cv::Mat(cv::Size(width, height), CV_8UC3);
        }else if(channel == 1){
            tmpImage = cv::Mat(cv::Size(width, height), CV_8UC1);
        }
        memcpy(tmpImage.data, imgData.data() + offset, size * sizeof(uint8_t));
        image.push_back(tmpImage);
        offset += size;
    }
    
    pImpl_->inference(image, netResult);
    
};

} //namespace
