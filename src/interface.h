#pragma once

#include <genicam/GenTL/GenTL.h>
#include <string>
#include <memory>
#include <utility>
#include "gentl_wrapper.h"
#include "device.h"

class Interface {
public:
    Interface(const char* interfaceId, std::shared_ptr<const GenTLWrapper> genTLPtr, GenTL::TL_HANDLE systemHandle);
    ~Interface();
    std::string getId();
    const char* getXMLPath(int frameGrabberIndex);
    std::vector<Device> getDevices(int updateTimeout);
    template<typename T>
    int getInfo(GenTL::STREAM_INFO_CMD info, T *value) {
        GenTL::GC_ERROR status;
        GenTL::INFO_DATATYPE type;
        size_t bufferSize;
        status = genTL->IFGetInfo(IF, info, &type, nullptr, &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            status = genTL->IFGetInfo(IF, info, &type, value, &bufferSize);
            if (status != GenTL::GC_ERR_SUCCESS) {
                return -1;
            }
        } else {
            return -1;
        }
        return 0;
    }
    int getInfo(GenTL::INTERFACE_INFO_CMD info, std::string* value);
    std::string getInfos(bool displayFull);

private:
    GenTL::TL_HANDLE TL;
    GenTL::IF_HANDLE IF = nullptr;
    GenTL::PORT_HANDLE frameGrabberPort = nullptr;
    int frameGrabberXMLIndex = 0;
    std::shared_ptr<const GenTLWrapper> genTL;
    bool opened = false;
};
