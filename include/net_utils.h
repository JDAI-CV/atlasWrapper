/*
    Created by jyx
*/
#ifndef ATLAS_NET_UTILS_H
#define ATLAS_NET_UTILS_H

#ifdef DVPP_STRIDE_128_16
  #define DVPP_STRIDE_WIDTH 128
  #define DVPP_STRIDE_HEIGHT 16
#else
  #define DVPP_STRIDE_WIDTH 16
  #define DVPP_STRIDE_HEIGHT 2
#endif

static int alignUp(int input, int alignment) {
    int remainder = input % alignment;
    return remainder ? input + (alignment - remainder) : input;
}

static void Dnofree(void* ptr) {
    /* if sendData. ptr do not need free, but nead a fake free function*/
}

static int getLoopAmount(int inputNum, int batchSize) {
    int loop_amount = 0;
    if(inputNum % batchSize == 0){
        loop_amount = int(inputNum / batchSize);
    }else{
        loop_amount = int(inputNum / batchSize) + 1;
    }
    return loop_amount;
};

static int getImgAndHeaderSize(int cols, int rows, int channels) {
    return cols * rows * channels * sizeof(uint8_t) + sizeof(ImageDataHeaderT);
};

static void memcpyToSendData(cv::Mat& src, cv::Mat& ori, uint32_t& offset, uint8_t* dst) {
    ImageDataHeaderT head;
    // ori chw info to header
    head.channels = ori.channels();
    head.height = ori.rows;
    head.width = ori.cols;

    head.dataSize = src.cols * src.rows * src.channels();
    memcpy(dst + offset, &head, sizeof(ImageDataHeaderT));
    offset += sizeof(ImageDataHeaderT);
    memcpy(dst + offset, src.data, head.dataSize);
    offset += head.dataSize;
};
#endif //ATLAS_NET_UTILS_H
