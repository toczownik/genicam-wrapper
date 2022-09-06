#include <iostream>
#include "interface.h"

#define BUFFER_SIZE 1024

std::string Interface::getId() {
    return getInfo(GenTL::INTERFACE_INFO_ID);
}

const char* Interface::getXMLPath(int frameGrabberIndex) {
    GenTL::INFO_DATATYPE type;
    size_t bufferSize;
    char *path;
    GenTL::GC_ERROR status = genTL->GCGetPortURLInfo(frameGrabberPort, frameGrabberIndex, GenTL::URL_INFO_URL, &type,
                                                     nullptr, &bufferSize);
    if (status != GenTL::GC_ERR_SUCCESS) {
        path = nullptr;
    } else {
        path = new char[bufferSize];
        status = genTL->GCGetPortURLInfo(frameGrabberPort, frameGrabberIndex, GenTL::URL_INFO_URL, &type,
                                                         path, &bufferSize);
        if (status != GenTL::GC_ERR_SUCCESS) {
            delete[] path;
            path = nullptr;
        }
    }
    return path;
}

void Interface::open() {
    GenTL::GC_ERROR status = genTL->TLOpenInterface(TL, id, &IF);
    if (status != GenTL::GC_ERR_SUCCESS) {
        opened = false;
    } else {
        opened = true;
    }
}

bool Interface::isOpened() {
    return opened;
}

std::vector<Device *> Interface::getDevices(const int updateTimeout = 100) {
    GenTL::GC_ERROR status = genTL->IFUpdateDeviceList(IF, nullptr, updateTimeout);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error " << status << " " << std::endl;
        genTL.reset();
        return {};
    }
    uint32_t numDevices;
    status = genTL->IFGetNumDevices(IF, &numDevices);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error " << status << " Can't get number of devices" << std::endl;
    }
    auto devices = std::vector<Device *>();
    size_t bufferSize;
    for (int i = 0; i < numDevices; ++i) {
        bufferSize = 0;
        status = genTL->IFGetDeviceID(IF, i, nullptr, &bufferSize);
        if (status != GenTL::GC_ERR_SUCCESS) {
            std::cout << "Error " << status << " Can't get device ID" << std::endl;
            return devices;
        }
        char* deviceId = new char[bufferSize];
        status = genTL->IFGetDeviceID(TL, i, deviceId, &bufferSize);
        if (status != GenTL::GC_ERR_SUCCESS) {
            std::cout << "Error " << status << " Can't get device ID" << std::endl;
            delete[] deviceId;
            return devices;
        }
        devices.push_back(new Device(deviceId, genTL, IF, TL));
    }
    return devices;
}

std::string Interface::getInfo(GenTL::INTERFACE_INFO_CMD info) {
    GenTL::GC_ERROR status;
    GenTL::INFO_DATATYPE type;
    size_t bufferSize = sizeof(char)*BUFFER_SIZE;
    char* retrieved = new char[1024];
    status = genTL->IFGetInfo(TL, info, &type, retrieved, &bufferSize);
    std::string value;
    if (status == GenTL::GC_ERR_SUCCESS) {
        value = retrieved;
    } else {
        value = "Couldn't retrieve info from interface";
    }
    return value;
}

std::string Interface::getInfos(bool displayFull = false) {
    auto infos = new std::vector<GenTL::INTERFACE_INFO_CMD>;
    if (displayFull) {
        infos->insert(infos->end(), {GenTL::INTERFACE_INFO_ID, GenTL::INTERFACE_INFO_DISPLAYNAME, GenTL::INTERFACE_INFO_TLTYPE});
    } else {
        infos->push_back(GenTL::INTERFACE_INFO_ID);
    }
    std::string values;
    for (GenTL::INTERFACE_INFO_CMD info : *infos) {
        values.append(getInfo(info));
        values.append(" | ");
    }
    return values;
}
