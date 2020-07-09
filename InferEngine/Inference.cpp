/*
    Created by jyx
*/
#include "Inference.h"
#include "dvpp_utils.h"
#include "engine_tools.h"
#include "error_code.h"
#include "hiaiengine/ai_memory.h"
#include "hiaiengine/c_graph.h"
#include "hiaiengine/data_type.h"
#include "hiaiengine/log.h"
#include "opencv2/opencv.hpp"
#include <memory>

#include <stdlib.h>
#include <stdio.h>
#include <mutex>
#include <condition_variable>

using std::map;
using std::shared_ptr;
using std::string;
using std::vector;


HIAI_REGISTER_SERIALIZE_FUNC("netInputRawHigh", netInputRawHigh, netInputRawHighSerialize, netInputRawHighDeserialize);
//HIAI_REGISTER_DATA_TYPE("netOutputRaw", netOutputRaw);
HIAI_REGISTER_SERIALIZE_FUNC("netOutputRawHigh", netOutputRawHigh, netOutputRawHighSerialize, netOutputRawHighDeserialize);

static void Dnofree(void* ptr) {
    /* if sendData. ptr do not need free, but nead a fake free function*/
}

HIAI_StatusT Inference::Init(const hiai::AIConfig& config,
    const std::vector<hiai::AIModelDescription>& model_desc)
{
    HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "[Inference] start init!");
    HIAI_StatusT ret = HIAI_OK;
    if (nullptr == modelManager) {
        modelManager = std::make_shared<hiai::AIModelManager>();
    }
    hiai::AIModelDescription modelDesc;
    loadModelDescription(config, modelDesc);

    // init ai model manager
    ret = modelManager->Init(config, { modelDesc });
    if (hiai::SUCCESS != ret) {
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "[Inference] ai model manager init failed!");
        return HIAI_ERROR;
    }
    // input/output buffer allocation
    std::vector<hiai::TensorDimension> inputTensorDims;
    std::vector<hiai::TensorDimension> outputTensorDims;
    ret = modelManager->GetModelIOTensorDim(modelDesc.name(), inputTensorDims, outputTensorDims);
    if (ret != hiai::SUCCESS) {
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "[Inference] hiai ai model manager init failed.");
        return HIAI_ERROR;
    }
    for (auto& dims : inputTensorDims) {
        logDumpDims(dims);
    }
    for (auto& dims : outputTensorDims) {
        logDumpDims(dims);
    }
    // input dims
    if (1 != inputTensorDims.size()) {
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "[Inference] inputTensorDims.size() != 1 (%d vs. %d)",
            inputTensorDims.size(), 1);
        return HIAI_ERROR;
    }

    kBatchSize = inputTensorDims[0].n;
    kChannel = inputTensorDims[0].c;
    kHeight = inputTensorDims[0].h;
    kWidth = inputTensorDims[0].w;
    kInputSize = inputTensorDims[0].size;
    kAlignedWidth = ALIGN_UP(kWidth, DVPP_STRIDE_WIDTH);
    kAlignedHeight = ALIGN_UP(kHeight, DVPP_STRIDE_HEIGHT);
    ret = creatIOTensors(modelManager, inputTensorDims, inputTensorVec, inputDataBuffer);
    if (HIAI_OK != ret) {
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "[Inference] creat input tensors failed!");
        return HIAI_ERROR;
    }
    ret = creatIOTensors(modelManager, outputTensorDims, outputTensorVec, outputDataBuffer);
    if (HIAI_OK != ret) {
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "[Inference] creat output tensors failed!");
        return HIAI_ERROR;
    }

    HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "[Inference] end init!");
    return HIAI_OK;
}

HIAI_IMPL_ENGINE_PROCESS("Inference", Inference, DT_INPUT_SIZE)
{   
    HIAI_StatusT ret = HIAI_OK;
    std::shared_ptr<netInputRawHigh> inputArg = std::static_pointer_cast<netInputRawHigh>(arg0);
    
    if (nullptr == inputArg) {
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "Fail to process invalid message");
        return HIAI_ERROR;
    }

    // resize yuv data to input size
    uint8_t* dataBufferPtr = inputDataBuffer[0].get();
    uint32_t offset = 0;
    uint8_t *pRawData = inputArg->rawData.get();

    for (int i = 0; i < inputArg->batchSize; i++) { 
        ImageDataHeaderT *header = (ImageDataHeaderT *)(pRawData + offset);
        offset += sizeof(ImageDataHeaderT);
        uint32_t resizedSize = 0;
    
        vpcBgr2YuvResize(pRawData + offset, header->width, header->height,
                        kWidth, kHeight, dataBufferPtr, resizedSize);

        dataBufferPtr += resizedSize;
        offset += header->dataSize;
    }
    
    // inference
    hiai::AIContext aiContext;
    ret = modelManager->Process(aiContext, inputTensorVec, outputTensorVec, 0);
    if (hiai::SUCCESS != ret) {
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "AI Model Manager Process failed");
        return HIAI_ERROR;
    }
    postProcess(inputArg);
    return HIAI_OK;
}

HIAI_StatusT Inference::postProcess(std::shared_ptr<netInputRawHigh> inputArg){
    uint32_t outputDataSize = 0; 
    uint32_t perResultSize = 0;
    uint8_t *outputData = NULL;

    int batch = kBatchSize;
    uint32_t offset=0;
    int ret = 0;

    for (int j = 0; j < outputTensorVec.size(); j++) {
        std::shared_ptr<hiai::AISimpleTensor> result_tensor = std::static_pointer_cast<hiai::AISimpleTensor>(outputTensorVec[j]);
        outputDataSize += result_tensor->GetSize();
        perResultSize += result_tensor->GetSize()/batch;
    }

    HIAI_StatusT hiai_ret = hiai::HIAIMemory::HIAI_DMalloc((uint32_t)outputDataSize, (void*&)outputData);
    if(hiai_ret != HIAI_OK) {
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR,"fail to HIAI_Dmalloc,use malloc");
        outputData = new uint8_t[outputDataSize];
    } 

    std::vector<int> outputDims;
    for (int j = 0; j < outputTensorVec.size(); j++) {
        std::shared_ptr<hiai::AINeuralNetworkBuffer> result_tensor = std::static_pointer_cast<hiai::AINeuralNetworkBuffer>(outputTensorVec[j]);

        ret = memcpy_s(outputData + offset, result_tensor->GetSize(),
                        (char*)result_tensor->GetBuffer(),result_tensor->GetSize());
        if (ret) {
            HIAI_ENGINE_LOG(HIAI_IDE_ERROR,"memcpy_s failed! ");
            delete [] outputData;
            outputData = NULL;
            return HIAI_ERROR;
        }
        offset += result_tensor->GetSize();
        outputDims.push_back(result_tensor->GetSize() / sizeof(float) );
    }
    
    //send to host
    std::shared_ptr<netOutputRawHigh> outputRaw = std::make_shared<netOutputRawHigh>();
    
    outputRaw->batchSize = kBatchSize;
    outputRaw->channels = kChannel;
    outputRaw->height = kHeight;
    outputRaw->width = kWidth;

    outputRaw->outputBufferSize = outputDataSize;
    outputRaw->outputDims = outputDims;
    
    outputRaw->mtx = (uint64_t)(inputArg->mtx);
    outputRaw->cv = (uint64_t)(inputArg->cv);
    outputRaw->host_result_ptr = (uint64_t)(inputArg->host_result_ptr);
    outputRaw->sizePerResult = perResultSize;
    outputRaw->outputBuffer.reset(outputData,Dnofree);

    HIAI_StatusT state = SendData(0, "netOutputRawHigh", std::static_pointer_cast<void>(outputRaw));
    if(state != HIAI_OK) {
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "Inference SendData failed! ");
        return HIAI_ERROR;
    }
}
