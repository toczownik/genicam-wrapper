#pragma once

#include <utility>
#include <memory>
#include <GenApi/NodeMapRef.h>
#include "gentl_wrapper.h"
#include "stream.h"

class Device {
public:
    Device(const char* deviceId, std::shared_ptr<const GenTLWrapper> genTLPtr, GenTL::IF_HANDLE interfaceHandle, GenTL::TL_HANDLE systemHandle);
    ~Device();
    void getPort(int cameraXMLIndex);
    std::string getName();
    std::vector<Stream> getStreams();
    std::string getId();
    template<typename T>
    int getInfo(GenTL::STREAM_INFO_CMD info, T *value) {
        GenTL::GC_ERROR status;
        GenTL::INFO_DATATYPE type;
        size_t bufferSize;
        status = genTL->DevGetInfo(DEV, info, &type, nullptr, &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            status = genTL->DevGetInfo(DEV, info, &type, value, &bufferSize);
            if (status != GenTL::GC_ERR_SUCCESS) {
                return -1;
            }
        } else {
            return -1;
        }
        return 0;
    }
    int getInfo(GenTL::DEVICE_INFO_CMD info, std::string* value);
    std::string getInfos(bool displayFull);

private:
    std::shared_ptr<const GenTLWrapper> genTL;
    GenTL::IF_HANDLE IF;
    GenTL::TL_HANDLE TL;
    GenTL::DEV_HANDLE DEV = nullptr;
    GenTL::PORT_HANDLE PORT_CAMERA;
    std::string getXMLFromURL(const char* url);
    std::shared_ptr<GenApi::CNodeMapRef> nodeMap;
};
