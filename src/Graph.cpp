/*
  Created by jyx
*/
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "Graph.hpp"

using namespace atlas;

static std::atomic<int> g_flag = { 1 };

HIAI_StatusT CustomDataRecvInterface::RecvData(const std::shared_ptr<void>& message) {
    std::shared_ptr<std::string> data = std::static_pointer_cast<std::string>(message);
    std::cout<<*(data.get())<<"\n";
    return HIAI_OK;
}
HIAI_REGISTER_SERIALIZE_FUNC("netInputRawHigh", netInputRawHigh,
                            netInputRawHighSerialize,
                            netInputRawHighDeserialize);

Graph::Graph(string modelPath, string enginePath, int deviceId) {
    _deviceId = deviceId;
    _modelPath = modelPath;

    if (enginePath.back() == '/') {
        _enginePath = enginePath;
    }
    else {
        _enginePath = enginePath + '/';
    }
    
    HIAI_StatusT ret = HIAI_InitAndStartGraph();
    
    if (ret != HIAI_OK) {
        printf("[Graph] Fail to init and start graph\n");    
    }
};

Graph::~Graph() {
    _dynamicGraph.destroyGraph();
    printf("[Graph] Destory graph\n");
}

HIAI_StatusT Graph::CreateDynamicGraphs(uint32_t num ) {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = now_ms.time_since_epoch();
    uint32_t id = (uint32_t)value.count();
    std::cout<<"graph id: "<<id<<std::endl;
    _graphId = id;
    for (int i = 0; i < num; i++) {
        dg::graph g(id, _deviceId);
        dg::engine e0("Inference", ++id, 1, dg::engine::DEVICE);
        dg::engine e1("DstEngine", ++id, 1, dg::engine::HOST);

        e0.so_name.push_back( _enginePath+"libInferEngine.so" );
        e0.ai_config.addAIConfigItem( "model", _modelPath );
        e1.so_name.push_back( _enginePath+"libDstEngine.so" );

        _inputNodes.push_back(std::make_tuple(g, e0, 0));
        _outputNodes.push_back(std::make_tuple(g, e1, 0));
        
        g.addEngine(e0);
        g.addEngine(e1);
        g.addConnection(dg::connection(e0, 0, e1, 0));
        
        _dynamicGraph.addGraph(g);
    }
    return HIAI_OK;
}

// Init and create graph
HIAI_StatusT Graph::HIAI_InitAndStartGraph() {
    HIAI_StatusT ret;
    uint32_t graphNum = 1;
    ret = CreateDynamicGraphs(graphNum);
    
    if (ret != HIAI_OK) {
        printf("CreateDynamicGraphs failed %d\n", ret);
        return HIAI_ERROR;
    }
    ret = _dynamicGraph.createGraph();
    if (ret != HIAI_OK) {
        printf("createGraph failed %d\n", ret);
        return HIAI_ERROR;
    }

    for (const auto& node : _outputNodes) {
        ret = _dynamicGraph.setDataRecvFunctor(node, std::make_shared<CustomDataRecvInterface>(""));
        if (ret != HIAI_OK) {
            printf("setDataRecvFunctor failed %d\n", ret);
            return -1;
        }
    }
    return HIAI_OK;
}

HIAI_StatusT Graph::sendData(std::shared_ptr<netInputRawHigh> inputRaw) {
        _dynamicGraph.sendData(_inputNodes[0], "netInputRawHigh",
                                std::static_pointer_cast<void>(inputRaw));
};
