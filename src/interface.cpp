#include <iostream>
#include <utility>
#include "interface.h"
#include "cport.h"

Interface::Interface(const char *interfaceId, std::shared_ptr<const GenTLWrapper> genTLPtr,
                     GenTL::TL_HANDLE systemHandle) {
    TL = systemHandle;
    genTL = std::move(genTLPtr);
    GenTL::GC_ERROR status = genTL->TLOpenInterface(TL, interfaceId, &IF);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::string message = "Couldn't open interface" + std::string(interfaceId);
        throw std::runtime_error(message);
    }

}

std::string Interface::getId() {
    std::string id;
    if (getInfo(&id, GenTL::INTERFACE_INFO_ID) != 0) {
        return "Couldn't retrieve id";
    }
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
            path = nullptr;
        }
        delete[] path;

    }
    return path;
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
        char* deviceId = new char [bufferSize];
        status = genTL->IFGetDeviceID(IF, i, deviceId, &bufferSize);
        if (status != GenTL::GC_ERR_SUCCESS) {
            std::cout << "Error " << status << " Can't get device ID" << std::endl;
            return devices;
        }
        GenICam_3_2::gcstring deviceString = deviceId;
        devices.push_back(new Device(deviceString, genTL, IF, TL));
        delete[] deviceId;
    }
    return devices;
}

int Interface::getInfo(std::string* returnString, GenTL::INTERFACE_INFO_CMD info) {
    GenTL::GC_ERROR status;
    GenTL::INFO_DATATYPE type;
    size_t bufferSize;
    status = genTL->IFGetInfo(IF, info, &type, nullptr, &bufferSize);
    if (status == GenTL::GC_ERR_SUCCESS) {
        char *retrieved = new char[bufferSize];
        status = genTL->IFGetInfo(IF, info, &type, retrieved, &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            *returnString = retrieved;
        } else {
            return -1;
        }
        delete[] retrieved;
    } else {
        return -2;
    }
    return 0;
}

std::string Interface::getInfos(bool displayFull = false) {
    auto infos = new std::vector<GenTL::INTERFACE_INFO_CMD>;
    if (displayFull) {
        infos->insert(infos->end(), {GenTL::INTERFACE_INFO_ID, GenTL::INTERFACE_INFO_DISPLAYNAME, GenTL::INTERFACE_INFO_TLTYPE});
    } else {
        infos->push_back(GenTL::INTERFACE_INFO_ID);
    }
    std::string values;
    std::string value;
    for (GenTL::INTERFACE_INFO_CMD info : *infos) {
        if (getInfo(&value, info)) {
            values.append(value);
            values.append(" | ");
        }

    }
    delete infos;
    return values;
}
