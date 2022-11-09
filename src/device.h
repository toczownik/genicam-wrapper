#pragma once

#include <utility>
#include <memory>
#include <GenApi/NodeMapRef.h>
#include "gentl_wrapper.h"
#include "stream.h"
#include "CPort.h"

class Device {
public:
    Device(const std::string& deviceId, std::shared_ptr<const GenTLWrapper> genTLPtr, GenTL::IF_HANDLE interfaceHandle);
    ~Device();
    std::string getName();
    std::vector<Stream> getStreams();
    Stream getStream(int streamIndex);
    std::string getId();
    template<typename T>
    T getInfo(GenTL::DEVICE_INFO_CMD info) {
        GenTL::GC_ERROR status;
        GenTL::INFO_DATATYPE type;
        size_t bufferSize;
        T value(0);
        status = genTL->DevGetInfo(DEV, info, &type, nullptr, &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            status = genTL->DevGetInfo(DEV, info, &type, value, &bufferSize);
            if (status != GenTL::GC_ERR_SUCCESS) {
                return value;
            }
        } else {
            return -1;
        }
        return 0;
    }
    template<typename T>
    T getCameraInfo(GenTL::PORT_INFO_CMD info) {
        GenTL::GC_ERROR status;
        GenTL::INFO_DATATYPE type;
        size_t bufferSize;
        T value(0);
        status = genTL->DevGetInfo(PORT_CAMERA, info, &type, nullptr, &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            status = genTL->DevGetInfo(PORT_CAMERA, info, &type, value, &bufferSize);
            if (status != GenTL::GC_ERR_SUCCESS) {
                return -1;
            }
        } else {
            return -1;
        }
        return 0;
    }
    std::string getCameraInfo(GenTL::PORT_INFO_CMD info);
    std::string getInfo(GenTL::DEVICE_INFO_CMD info);
    std::string getInfos(bool displayFull);
    std::shared_ptr<GenApi::CNodeMapRef> getCameraNodeMap(int XMLIndex, bool enableImageGenerator);

private:
    std::shared_ptr<const GenTLWrapper> genTL;
    GenTL::DEV_HANDLE DEV;
    GenTL::PORT_HANDLE PORT_CAMERA;
    std::string getXMLPath(int cameraXMLIndex);
    void loadXMLFromURL(const char *url, CPort * port, const std::shared_ptr<GenApi::CNodeMapRef>& nodeMap);
    std::string getXMLStringFromDisk(const char *url);
    std::string getXMLStringFromURL(const char *url, CPort *port);
    void getZippedXMLDataFromRegister(const char *url, CPort *port, void * ipData, size_t *zipSize);
    std::string getXmlStringFromRegisterMap(const char *url, CPort *port);
    std::shared_ptr<CPort> cameraCPort = nullptr;
};
