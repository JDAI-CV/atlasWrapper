/*
  Created by jyx
*/
#include "DstEngine.h"

#include <stdlib.h>
#include <stdio.h>
#include "stream_data.h"
#include "Net.hpp"

#include <mutex>
#include <condition_variable>

HIAI_REGISTER_DATA_TYPE("netOutputRaw", netOutputRaw);
HIAI_REGISTER_SERIALIZE_FUNC("netOutputRawHigh", netOutputRawHigh,
                            netOutputRawHighSerialize,
                            netOutputRawHighDeserialize);

HIAI_StatusT DstEngine::Init(const hiai::AIConfig& config,
                            const std::vector<hiai::AIModelDescription>& model_desc) {
    return HIAI_OK;
}


HIAI_IMPL_ENGINE_PROCESS("DstEngine", DstEngine, DST_INPUT_SIZE) {
    auto inputArg = std::static_pointer_cast<netOutputRawHigh>(arg0);
    if (inputArg == nullptr) {
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "[DstEngine] The input arg0 is nullptr");
        return HIAI_ERROR;
    }
    std::vector<int> outputDims = inputArg->outputDims;

    float *allOutput = reinterpret_cast<float*>(inputArg->outputBuffer.get());

    std::vector<atlas::NetResult>* result = (std::vector<atlas::NetResult>*)inputArg->host_result_ptr;

    uint32_t batchSize = inputArg->batchSize;
    uint32_t singleSize = inputArg->sizePerResult;
    
    int singleVectorSize = int( singleSize/sizeof(float) ) - 1; // 2804 / 4 = 701

    atlas::NetResult netResult;
    netResult.outputDims = outputDims;
    netResult.batchSize = inputArg->batchSize;
    netResult.channels = inputArg->channels;
    netResult.height = inputArg->height;
    netResult.width = inputArg->width;
    netResult.outputNum = outputDims.size();

    int vectorLen = inputArg->outputBufferSize / sizeof(float);
    std::vector<float> output{allOutput, allOutput + vectorLen};
    netResult.output.assign(output.begin(), output.end());
    result->push_back(netResult);

    // std::shared_ptr<std::string> dstData(new std::string("abcdefg"));
    // HIAI_StatusT state = SendData(0, "string", std::static_pointer_cast<void>(dstData));

    // 唤醒所有线程
    std::unique_lock <std::mutex> lck( *( (std::mutex*)(inputArg->mtx) ) ) ;
    ((std::condition_variable*)(inputArg->cv))->notify_all(); 

    return HIAI_OK;
}
