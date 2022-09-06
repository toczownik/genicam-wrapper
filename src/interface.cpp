#include <iostream>
#include "interface.h"

const char* Interface::getId() {
    return id;
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

std::string Interface::getInfo(bool displayFull = false) {
    GenTL::GC_ERROR status;
    GenTL::INFO_DATATYPE type;
    size_t bufferSize = sizeof(char)*1024;
    char* value;
    auto infos = new std::vector<GenTL::INTERFACE_INFO_CMD>;
    if (displayFull) {
        infos->insert(infos->end(), {GenTL::INTERFACE_INFO_ID, GenTL::INTERFACE_INFO_DISPLAYNAME, GenTL::INTERFACE_INFO_TLTYPE});
    } else {
        infos->push_back(GenTL::INTERFACE_INFO_ID);
    }
    std::string values;
    for (GenTL::INTERFACE_INFO_CMD info : *infos) {
        value = new char[bufferSize];
        status = genTL->IFGetInfo(IF, info, &type, value, &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            values.append(value);
            values.append(" | ");
        }
    }
    return values;
}
