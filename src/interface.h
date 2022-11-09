#pragma once

#include <GenTL/GenTL.h>
#include <string>
#include <memory>
#include <utility>
#include "gentl_wrapper.h"
#include "device.h"
#include "CPort.h"
#include <Base/GCString.h>

class Interface {
public:
    Interface(std::string interfaceId, std::shared_ptr<const GenTLWrapper> genTLPtr, GenTL::TL_HANDLE systemHandle);
    ~Interface();
    std::string getId();
    std::vector<Device> getDevices(int updateTimeout = 100);
    Device getDevice(int deviceIndex, int updateTimeout = 100);
    template<typename T>
    T getInfo(GenTL::INTERFACE_INFO_CMD info) {
        GenTL::GC_ERROR status;
        GenTL::INFO_DATATYPE type;
        size_t bufferSize;
        T value(0);
        status = genTL->IFGetInfo(IF, info, &type, nullptr, &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            status = genTL->IFGetInfo(IF, info, &type, &value, &bufferSize);
            if (status != GenTL::GC_ERR_SUCCESS) {
                throw GenTLException(status, "Error retrieving information from an interface");
            }
        } else {
            throw GenTLException(status, "Error retrieving information buffer size from an interface");
        }
        return value;
    }
    template<typename T>
    T getFrameGrabberInfo(GenTL::PORT_INFO_CMD info) {
        GenTL::GC_ERROR status;
        GenTL::INFO_DATATYPE type;
        size_t bufferSize;
        T value(0);
        status = genTL->GCGetPortInfo(frameGrabberPort, info, &type, nullptr, &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            status = genTL->GCGetPortInfo(frameGrabberPort, info, &type, value, &bufferSize);
            if (status != GenTL::GC_ERR_SUCCESS) {
                throw GenTLException(status, "Error retrieving information from a frame grabber");
            }
        } else {
            throw GenTLException(status, "Error retrieving information buffer size from a frame grabber");
        }
        return value;
    }
    std::string getFrameGrabberInfo(GenTL::PORT_INFO_CMD info);
    std::string getInfo(GenTL::INTERFACE_INFO_CMD info);
    std::string getInfos(bool displayFull);
    std::shared_ptr<GenApi::CNodeMapRef> getFrameGrabberNodeMap(int XMLindex);

private:
    GenTL::IF_HANDLE IF;
    GenTL::PORT_HANDLE frameGrabberPort;
    std::shared_ptr<const GenTLWrapper> genTL;
    std::string getXMLStringFromDisk(const char *url);
    std::string getXMLStringFromURL(const char *url, CPort *port);
    std::string getXMLPath(int frameGrabberIndex);
    void loadXMLFromURL(const char *url, CPort * port, const std::shared_ptr<GenApi::CNodeMapRef>& nodeMap);
    void getZippedXMLDataFromRegister(const char *url, CPort * port, void * zipData, size_t *zipSize);
    std::string getXmlStringFromRegisterMap(const char *url, CPort *port);
    std::shared_ptr<CPort> frameGrabberCPort = nullptr;
};
