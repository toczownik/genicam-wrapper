#include <iostream>
#include <fstream>
#include <GenApi/NodeMapRef.h>
#include "device.h"

#define BUFFER_SIZE 1024

void Device::open() {
    GenTL::GC_ERROR status = genTL->IFOpenDevice(IF, id, GenTL::DEVICE_ACCESS_CONTROL, &DEV);
    std::cout << "DEVICE::open " << DEV << std::endl;
    if (status != GenTL::GC_ERR_SUCCESS) {
        opened = false;
    } else {
        opened = true;
    }
}

bool Device::isOpened() {
    return opened;
}

void Device::getPort(int cameraXMLIndex) {
    PORT_CAMERA = GENTL_INVALID_HANDLE;
    GenTL::GC_ERROR status = genTL->DevGetPort(DEV, &PORT_CAMERA);
    std::cout << "PORT CAMERA " << PORT_CAMERA << std::endl;
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
    }
    else {
        std::string cameraXML = getXMLFromURL(camera_xml_location);
        //std::cout << cameraXML.c_str() << std::endl;
        nodeMap = std::make_shared<GenApi::CNodeMapRef>("CAMERA");
        nodeMap->_LoadXMLFromString(cameraXML.c_str());
    }
    delete[] camera_xml_location;

    bufferSize = 0;


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

char *Device::getName() {
    size_t bufferSize = 1024;
    GenTL::INFO_DATATYPE type;
    /*
    GenTL::GC_ERROR status = genTL->GCGetPortInfo(PORT_CAMERA, GenTL::PORT_INFO_PORTNAME, &type, nullptr, &bufferSize);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error " << status << " Can't get mPortHandle name" << std::endl;
        return nullptr;
    }*/
    char *cameraPortName = new char[bufferSize];
    GenTL::GC_ERROR status = genTL->GCGetPortInfo(PORT_CAMERA, GenTL::PORT_INFO_PORTNAME, &type, cameraPortName, &bufferSize);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error " << status << " Can't get mPortHandle name" << std::endl;
    }
    return cameraPortName;
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
        /*
        status = genTL->DevGetDataStreamID(DEV, i, nullptr, &bufferSize);
        if (status != GenTL::GC_ERR_SUCCESS) {
            std::cout << "Error " << status << " Can't get data stream ID" << std::endl;
            return streams;
        }*/
        bufferSize = 1024;
        char *streamId = new char[bufferSize];
        status = genTL->DevGetDataStreamID(DEV, i, streamId, &bufferSize);
        if (status != GenTL::GC_ERR_SUCCESS) {
            std::cout << "Error " << status << " Can't get data stream ID" << std::endl;
            delete[] streamId;
            return streams;
        }
        streams.push_back(new Stream(streamId, genTL, DEV, TL));
    }
    return streams;
}

std::string Device::getId() {
    return getInfo(GenTL::DEVICE_INFO_ID);
}

std::string Device::getInfo(GenTL::DEVICE_INFO_CMD info) {
    GenTL::GC_ERROR status;
    GenTL::INFO_DATATYPE type;
    size_t bufferSize = sizeof(char)*BUFFER_SIZE;
    char* retrieved = new char[1024];
    status = genTL->DevGetInfo(TL, info, &type, retrieved, &bufferSize);
    std::string value;
    if (status == GenTL::GC_ERR_SUCCESS) {
        value = retrieved;
    } else {
        value = "Couldn't retrieve info from device";
    }
    return value;
}

std::string Device::getInfos(bool displayFull) {
    auto infos = new std::vector<GenTL::DEVICE_INFO_CMD>;
    if (displayFull) {
        infos->insert(infos->end(), {GenTL::DEVICE_INFO_SERIAL_NUMBER, GenTL::DEVICE_INFO_VERSION});
    } else {
        infos->push_back(GenTL::DEVICE_INFO_SERIAL_NUMBER);
    }
    std::string values;
    for (GenTL::DEVICE_INFO_CMD info : *infos) {
        values.append(getInfo(info));
        values.append(" | ");
    }
    return values;
}
