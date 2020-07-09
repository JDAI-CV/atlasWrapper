/*
    Created by jyx
*/
#include "pybind11/pybind11.h"
#include "pybind11/numpy.h"
#include "pybind11/stl.h"

#include "Net.hpp"

namespace py = pybind11;
using namespace atlas;

PYBIND11_MODULE(pyatlas, m) {
    m.doc() = "python interface of atlasWrapper";

    py::class_<NetInfo>(m, "NetInfo")
        .def(py::init<>())
        .def_readwrite("batchSize", &NetInfo::batchSize)
        .def_readwrite("channels", &NetInfo::channels)
        .def_readwrite("height", &NetInfo::height)
        .def_readwrite("width", &NetInfo::width)
        .def_readwrite("inputNum", &NetInfo::inputNum)
        .def_readwrite("outputNum", &NetInfo::outputNum);

    py::class_<NetResult>(m, "NetResult")
        .def(py::init<>())
        .def_readwrite("batchSize", &NetResult::batchSize)
        .def_readwrite("channels", &NetResult::channels)
        .def_readwrite("height", &NetResult::height)
        .def_readwrite("width", &NetResult::width)
        .def_readwrite("inputNum", &NetResult::inputNum)
        .def_readwrite("outputNum", &NetResult::outputNum)
        .def_readwrite("outputDims", &NetResult::outputDims)
        .def_readwrite("output", &NetResult::output);


    py::class_<Net>(m, "net")
        .def( py::init( [] (std::string modelPath, std::string enginePath, int device) {
            return std::unique_ptr<Net>(new Net(modelPath, enginePath, device));
        }) )
        .def("getNetInfo", &Net::getNetInfo)
        .def("DoInference", [](Net& self, py::array_t<uint8_t, py::array::c_style | py::array::forcecast> array, py::array_t<int, py::array::c_style | py::array::forcecast> arraySize ) {
            std::vector<uint8_t> input;
            std::vector<int> inputSize;

            input.resize(array.size());
            std::memcpy(input.data(), array.data(), array.size()*sizeof(uint8_t));

            inputSize.resize(arraySize.size());
            std::memcpy(inputSize.data(), arraySize.data(), arraySize.size()*sizeof(int));

            std::cout<<"size: "<<inputSize[0]<<"\n";

            std::vector<NetResult> Result;
            self.inference(input, inputSize, Result);
            return Result;
            
        });


}
