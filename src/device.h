#pragma once

#include <utility>
#include <memory>
#include <GenApi/NodeMapRef.h>
#include "gentl_wrapper.h"
#include "stream.h"

class Device {
public:
    Device(const char* deviceId, std::shared_ptr<const GenTLWrapper> genTLPtr, GenTL::IF_HANDLE interfaceHandle, GenTL::TL_HANDLE systemHandle);
    void getPort(int cameraXMLIndex);
    std::string getName();
    std::vector<Stream> getStreams();
    std::string getId();
    int getInfo(std::string* returnString, GenTL::DEVICE_INFO_CMD info);
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
