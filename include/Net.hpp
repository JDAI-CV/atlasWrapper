/*
    Created by jyx
*/
#ifndef ATLAS_NET_HPP
#define ATLAS_NET_HPP

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

namespace atlas{
    typedef struct {   
        int batchSize;
        int channels;
        int height;
        int width;

        int inputNum;
        int outputNum;

        int getBatchSize(){
            return this->batchSize;
        }
        int getChannels(){
            return this->channels;
        }
        int getHeight(){
            return this->height;
        }
        int getWidth(){
            return this->width;
        }
        int getInputNum(){
            return this->inputNum;
        }
        int getOutputNum(){
            return this->outputNum;
        }

    } NetInfo;

    class NetResult {
        public:
            int batchSize;
            int channels;
            int height;
            int width;

            int inputNum;
            int outputNum;

            std::vector<int> outputDims;
            std::vector<float> output;

            NetResult(){};
    };

    class NetImp;

    class Net {
        private:
        NetImp* pImpl_;
        NetInfo _netInfo;

        public:
        Net(std::string modelPath, std::string enginePath, int device);
        ~Net();
        NetInfo getNetInfo();
        void inference(std::vector<cv::Mat>& image, std::vector<NetResult>& Result);
        void inference(std::vector<uint8_t>& data, std::vector<int>& dataSize, std::vector<NetResult>& Result); 
             
    };

}

#endif
