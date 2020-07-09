/*
    Created by jyx
*/
#ifndef ATLAS_GRAPH_HPP
#define ATLAS_GRAPH_HPP
#include "stream_data.h"
#include "DynamicGraph.h"
#include "DataRecv.h"

namespace atlas{

    class Graph{
        private:
            dg::DynamicGraph _dynamicGraph;
            std::vector<dg::NodeInfo> _inputNodes;
            std::vector<dg::NodeInfo> _outputNodes;

            int _graphId;

            std::string _modelPath;
            std::string _enginePath;
            int _deviceId;

            HIAI_StatusT HIAI_InitAndStartGraph();
            HIAI_StatusT CreateDynamicGraphs(uint32_t num);

        public:
            Graph(string modelPath, string enginePath, int device);
            ~Graph();

            HIAI_StatusT sendData(std::shared_ptr<netInputRawHigh> inputRaw);
    };

}

#endif
