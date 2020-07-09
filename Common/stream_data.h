/*
  Created byjyx
*/
#ifndef STREAM_DATA_H
#define STREAM_DATA_H
#include "cereal/types/utility.hpp"
#include "hiaiengine/data_type.h"
#include "hiaiengine/data_type_reg.h"
#include <array>
#include <chrono>
#include <memory>
#include <time.h>
#include <unordered_map>
#include <vector>

#include <mutex>
#include <condition_variable>

typedef struct{
    uint32_t width;
    uint32_t height;
    uint32_t channels;
    uint32_t dataSize;
}ImageDataHeaderT;

typedef struct 
{   
    int batchSize;

    std::mutex* mtx;
    std::condition_variable* cv;
    void* host_result_ptr;
  
    int size;
    std::shared_ptr<uint8_t> rawData; //数据字段
}netInputRawHigh;

inline void netInputRawHighSerialize(void* input, std::string& control, std::uint8_t*& data, std::uint32_t& data_length)  //input：jdRawData; control 控制字段,序列化为string; data
{
    auto streamRawData = static_cast<netInputRawHigh*>(input);
    control = std::string(static_cast<char*>(input), sizeof(netInputRawHigh));
    data = streamRawData->rawData.get();
    data_length = streamRawData->size;
}

inline std::shared_ptr<void> netInputRawHighDeserialize(const char* control, const std::uint32_t& control_length, const std::uint8_t* data, const std::uint32_t& data_length)
{
    auto streamRawData = std::make_shared<netInputRawHigh>();
    streamRawData->batchSize = reinterpret_cast<netInputRawHigh*>(const_cast<char*>(control))->batchSize;
    streamRawData->mtx = reinterpret_cast<netInputRawHigh*>(const_cast<char*>(control))->mtx;
    streamRawData->cv = reinterpret_cast<netInputRawHigh*>(const_cast<char*>(control))->cv;
    streamRawData->host_result_ptr = reinterpret_cast<netInputRawHigh*>(const_cast<char*>(control))->host_result_ptr;
    streamRawData->size = reinterpret_cast<netInputRawHigh*>(const_cast<char*>(control))->size;
    if (0 != streamRawData->size) {
        streamRawData->rawData = std::shared_ptr<std::uint8_t>(const_cast<std::uint8_t*>(data), hiai::Graph::ReleaseDataBuffer);
    }

    return std::static_pointer_cast<void>(streamRawData);
}
//HIAI_REGISTER_SERIALIZE_FUNC("netInputRawHigh", netInputRawHigh, netInputRawHighSerialize, netInputRawHighDeserialize);

struct netOutputRaw {
    int batchSize;
    int channels;
    int height;
    int width;

    //控制字段
    int outputBufferSize;

    int sizePerResult;

    uint64_t mtx;
    uint64_t cv;
    //jdai::JDAIDetectionResult* result;
    uint64_t host_result_ptr;

    std::vector<int> outputDims;

    std::shared_ptr<uint8_t> outputBuffer; //数据字段
};

template <class Archive>
void serialize(Archive& ar, netOutputRaw& data)
{
    ar(data.batchSize, data.channels, data.height, data.width, data.outputBufferSize, data.sizePerResult, data.mtx, data.cv, data.host_result_ptr, data.outputDims);
    if (data.outputBufferSize > 0 && data.outputBuffer.get() == nullptr) {
        HIAI_SHARED_PTR_RESET(data.outputBuffer, new(std::nothrow) uint8_t[data.outputBufferSize],\
                "serialize netOutputRaw reset fail");
    }
    ar(cereal::binary_data(data.outputBuffer.get(), data.outputBufferSize * sizeof(uint8_t)));
}

typedef struct {

    int batchSize;
    int channels;
    int height;
    int width;

    //控制字段
    int outputBufferSize;

    int sizePerResult;

    uint64_t mtx;
    uint64_t cv;
    //jdai::JDAIDetectionResult* result;
    uint64_t host_result_ptr;

    std::vector<int> outputDims;

    std::shared_ptr<uint8_t> outputBuffer; //数据字段

    template <class Archive>
    void serialize(Archive& ar) {
        ar(batchSize, channels, height, width, outputBufferSize, sizePerResult, mtx, cv, host_result_ptr, outputDims);
    }

} netOutputRawHigh;

inline void netOutputRawHighSerialize(void* data_ptr, std::string& struct_str, uint8_t*& buffer, uint32_t& buffer_size){
   netOutputRawHigh* engine_trans = (netOutputRawHigh*)data_ptr;
   uint8_t* dataPtr = (uint8_t*)engine_trans->outputBuffer.get();
   uint32_t dataLen = engine_trans->outputBufferSize;
   std::shared_ptr<uint8_t> data = ( (netOutputRawHigh*)data_ptr )->outputBuffer;
   // get struct buffer and size
   buffer_size = dataLen;
   buffer = (uint8_t*)engine_trans->outputBuffer.get();
   engine_trans->outputBuffer = nullptr;

   // serialize
   std::ostringstream outputStr;
   cereal::PortableBinaryOutputArchive archive(outputStr);
   archive((*engine_trans));
   struct_str = outputStr.str();

   ((netOutputRawHigh*)data_ptr)->outputBuffer = data;
}
inline void deleteNothing(void* ptr) {
    // do nothing
}

inline std::shared_ptr<void> netOutputRawHighDeserialize(const char* ctrlPtr, const uint32_t& ctrlLen, const uint8_t* dataPtr, const uint32_t& dataLen) {
    if(ctrlPtr == nullptr) {
        return nullptr;
    }
    std::shared_ptr<netOutputRawHigh> engine_trans_ptr = std::make_shared<netOutputRawHigh>();
    // set engine_trans_ptr
    std::istringstream inputStream(std::string(ctrlPtr, ctrlLen));
    cereal::PortableBinaryInputArchive archive(inputStream);
    archive((*engine_trans_ptr));
    uint32_t DataLen = engine_trans_ptr->outputBufferSize;
    if(dataPtr != nullptr) {
        (engine_trans_ptr->outputBuffer).reset((const_cast<uint8_t*>(dataPtr)), hiai::Graph::ReleaseDataBuffer);
    }
    return std::static_pointer_cast<void>(engine_trans_ptr);
}
//HIAI_REGISTER_SERIALIZE_FUNC("netOutputRawHigh", netOutputRawHigh, netOutputRawHighSerialize, netOutputRawHighDeserialize);

#endif
