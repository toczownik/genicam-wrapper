#pragma once

#include <GenTL/GenTL.h>
#include <string>
#include <memory>
#include <utility>
#include "gentl_wrapper.h"
#include "device.h"

class Interface {
public:
    Interface(const char* interfaceId, std::shared_ptr<const GenTLWrapper> genTLPtr, GenTL::TL_HANDLE systemHandle) :
    id(interfaceId), genTL(std::move(genTLPtr)), TL(systemHandle) {};
    const char* getId();
    const char* getXMLPath(int frameGrabberIndex);
    void open();
    std::vector<Device *> getDevices(int updateTimeout);
    bool isOpened();

private:
    const char* id;
    GenTL::TL_HANDLE TL;
    GenTL::IF_HANDLE IF = nullptr;
    GenTL::PORT_HANDLE frameGrabberPort = nullptr;
    std::shared_ptr<const GenTLWrapper> genTL;
    bool opened = false;
};
