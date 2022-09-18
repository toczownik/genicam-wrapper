#include <iostream>
#include <fstream>
#include <utility>
#include <GenApi/NodeMapRef.h>
#include "device.h"
#include "cport.h"

Device::Device(const char* deviceId, std::shared_ptr<const GenTLWrapper> genTLPtr, GenTL::IF_HANDLE interfaceHandle, GenTL::TL_HANDLE systemHandle) {
    genTL = std::move(genTLPtr);
    IF = interfaceHandle;
    TL = systemHandle;
    GenTL::GC_ERROR status = genTL->IFOpenDevice(IF, deviceId, GenTL::DEVICE_ACCESS_CONTROL, &DEV);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::string message = "Couldn't open device" + std::string(deviceId);
        throw std::runtime_error(message);
    }
}

void Device::getPort(int cameraXMLIndex) {
    PORT_CAMERA = GENTL_INVALID_HANDLE;
    GenTL::GC_ERROR status = genTL->DevGetPort(DEV, &PORT_CAMERA);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error" << status << " Can't get device mPortHandle" << std::endl;
        return;
    }
    size_t bufferSize;
    GenTL::INFO_DATATYPE type;
    status = genTL->GCGetPortURLInfo(PORT_CAMERA, cameraXMLIndex, GenTL::URL_INFO_URL, &type, nullptr, &bufferSize);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error " << status << " Can't get information about XML location, did you forget to export GENICAM_CACHE_V3_2 variable?" << std::endl;
        return;
    }
    char *camera_xml_location = new char[bufferSize];
    status = genTL->GCGetPortURLInfo(PORT_CAMERA, cameraXMLIndex, GenTL::URL_INFO_URL, &type, camera_xml_location, &bufferSize);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error " << status << " Can't get information about XML location" << std::endl;
        return;
    }
    std::string cameraXML = getXMLFromURL(camera_xml_location);
    nodeMap = std::make_shared<GenApi::CNodeMapRef>("CAMERA");
    delete[] camera_xml_location;

    bufferSize = 0;

    char cameraXMLString[0xD0000];
    std::shared_ptr<CPort> cameraCport = std::make_shared<CPort>(genTL, PORT_CAMERA);
    cameraCport->Read(cameraXMLString, 0xb000, 0xc5339);

    status = genTL->GCGetPortInfo(PORT_CAMERA, GenTL::PORT_INFO_PORTNAME, &type, nullptr, &bufferSize);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error " << status << " Can't get mPortHandle name" << std::endl;
        return;
    }
    //  auto cameraPortName = std::unique_ptr<char[]>{ new char[bufferSize] };
    char *cameraPortName = new char[bufferSize];
    status = genTL->GCGetPortInfo(PORT_CAMERA, GenTL::PORT_INFO_PORTNAME, &type, cameraPortName, &bufferSize);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error " << status << " Can't get mPortHandle name" << std::endl;
        return;
    }

    nodeMap->_Connect(cameraCport.get(), cameraPortName);
    delete[] cameraPortName;

}

std::string Device::getXMLFromURL(const char *url) {
    std::string trash, address;
    std::string urlString = url;
    std::stringstream urlStringstream(urlString.substr(0));
    std::getline(urlStringstream, trash, ':');
    std::getline(urlStringstream, address, '?');
    std::ifstream t( address);
    std::string xml;
    t.seekg(0, std::ios::end);
    xml.reserve(t.tellg());
    t.seekg(0, std::ios::beg);
    xml.assign((std::istreambuf_iterator<char>(t)),std::istreambuf_iterator<char>());
    return xml;
}

std::string Device::getName() {
    size_t bufferSize;
    GenTL::INFO_DATATYPE type;

    GenTL::GC_ERROR status = genTL->GCGetPortInfo(PORT_CAMERA, GenTL::PORT_INFO_PORTNAME, &type, nullptr, &bufferSize);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error " << status << " Can't get mPortHandle name" << std::endl;
        return "";
    }
    char *cameraPortName = new char[bufferSize];
    status = genTL->GCGetPortInfo(PORT_CAMERA, GenTL::PORT_INFO_PORTNAME, &type, cameraPortName, &bufferSize);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error " << status << " Can't get mPortHandle name" << std::endl;
    }
    std::string ret = cameraPortName;
    delete[] cameraPortName;
    return ret;
}

std::vector<Stream *> Device::getStreams() {
    uint32_t numStreams;
    std::cout << DEV << std::endl;
    GenTL::GC_ERROR status = genTL->DevGetNumDataStreams(DEV, &numStreams);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error " << status << " Can't get number of data streams" << std::endl;
        return {};
    }
    if(!numStreams) {
        std::cout << "No streams found" << std::endl;
        return {};
    }
    size_t bufferSize;
    auto streams = std::vector<Stream*>();
    for (int i = 0; i < numStreams; ++i) {
        status = genTL->DevGetDataStreamID(DEV, i, nullptr, &bufferSize);
        if (status != GenTL::GC_ERR_SUCCESS) {
            std::cout << "Error " << status << " Can't get data stream ID" << std::endl;
            return streams;
        }
        char *streamId = new char[bufferSize];
        status = genTL->DevGetDataStreamID(DEV, i, streamId, &bufferSize);
        if (status != GenTL::GC_ERR_SUCCESS) {
            std::cout << "Error " << status << " Can't get data stream ID" << std::endl;
            delete[] streamId;
            return streams;
        }
        GenICam_3_2::gcstring streamString = streamId;
        delete[] streamId;
        streams.push_back(new Stream(streamString, genTL, DEV, TL));
    }
    return streams;
}

std::string Device::getId() {
    std::string id;
    if (getInfo(&id, GenTL::DEVICE_INFO_ID) != 0){
        return "Couldn't retrieve id";
    }
    return id;
}

int Device::getInfo(std::string* returnString, GenTL::DEVICE_INFO_CMD info) {
    GenTL::GC_ERROR status;
    GenTL::INFO_DATATYPE type;
    size_t bufferSize;
    status = genTL->DevGetInfo(DEV, info, &type, nullptr, &bufferSize);
    if (status == GenTL::GC_ERR_SUCCESS) {
        char *retrieved = new char[bufferSize];
        status = genTL->DevGetInfo(DEV, info, &type, retrieved, &bufferSize);
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

std::string Device::getInfos(bool displayFull) {
    auto infos = new std::vector<GenTL::DEVICE_INFO_CMD>;
    if (displayFull) {
        infos->insert(infos->end(), {GenTL::DEVICE_INFO_ID, GenTL::DEVICE_INFO_DISPLAYNAME});
    } else {
        infos->push_back(GenTL::DEVICE_INFO_ID);
    }
    std::string values;
    std::string value;
    for (GenTL::DEVICE_INFO_CMD info : *infos) {
        if (getInfo(&value, info)) {
            values.append(value);
            values.append(" | ");
        }
    }
    delete infos;
    return values;
}
