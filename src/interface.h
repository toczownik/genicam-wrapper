#pragma once

#include <GenTL/GenTL.h>
#include <string>
#include <memory>
#include <utility>
#include "gentl_wrapper.h"
#include "device.h"

class Interface {
public:
    Interface(const char* interfaceId, std::shared_ptr<const GenTLWrapper> genTLPtr, GenTL::TL_HANDLE systemHandle);
    std::string getId();
    const char* getXMLPath(int frameGrabberIndex);
    std::vector<Device *> getDevices(int updateTimeout);
    int getInfo(std::string* returnString, GenTL::INTERFACE_INFO_CMD info);
    std::string getInfos(bool displayFull);

private:
    GenTL::TL_HANDLE TL;
    GenTL::IF_HANDLE IF = nullptr;
    GenTL::PORT_HANDLE frameGrabberPort = nullptr;
    int frameGrabberXMLIndex = 0;
    std::shared_ptr<const GenTLWrapper> genTL;
    bool opened = false;
};
