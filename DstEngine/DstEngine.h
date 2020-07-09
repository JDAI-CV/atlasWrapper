
#ifndef OUTPUT_ENGINE_H
#define OUTPUT_ENGINE_H

#include <fstream>
#include "hiaiengine/engine.h"
#include "Common.h"
#include "stream_data.h"
#include "AppCommon.h"

#define DST_INPUT_SIZE 1
#define DST_OUTPUT_SIZE 1

class DstEngine : public hiai::Engine {
public:
    HIAI_StatusT Init(const hiai::AIConfig& config, const std::vector<hiai::AIModelDescription>& model_desc);

    HIAI_DEFINE_PROCESS(DST_INPUT_SIZE, DST_OUTPUT_SIZE)
private:
};

#endif
