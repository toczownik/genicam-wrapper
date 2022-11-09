#include <iostream>
#include <fstream>
#include <utility>
#include <GenApi/NodeMapRef.h>
#include "device.h"
#include "CPort.h"

Device::Device(const std::string& deviceId, std::shared_ptr<const GenTLWrapper> genTLPtr, GenTL::IF_HANDLE interfaceHandle) {
    genTL = std::move(genTLPtr);
    DEV = GENTL_INVALID_HANDLE;
    PORT_CAMERA = GENTL_INVALID_HANDLE;
    GenTL::GC_ERROR status = genTL->IFOpenDevice(interfaceHandle, deviceId.c_str(), GenTL::DEVICE_ACCESS_EXCLUSIVE, &DEV);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::string message = "Couldn't open device" + std::string(deviceId);
        throw std::runtime_error(message);
    }
}

std::shared_ptr<GenApi::CNodeMapRef> Device::getCameraNodeMap(const int XMLIndex, bool enableImageGenerator) {
    GenTL::GC_ERROR status;
    if (!enableImageGenerator) {
        status = genTL-> DevGetPort(DEV, &PORT_CAMERA);
        if (status != GenTL::GC_ERR_SUCCESS) {
            throw GenTLException(status, "Error retrieving camera port");
        }
    }
    std::string cameraURL = getXMLPath(XMLIndex);
    std::shared_ptr<GenApi::CNodeMapRef> frameGrabberNodeMap = std::make_shared<GenApi::CNodeMapRef>("CAMERA");
    loadXMLFromURL(cameraURL.c_str(), nullptr, frameGrabberNodeMap);
    std::string cameraName = getCameraInfoString(GenTL::URL_INFO_URL);
    cameraCPort = std::make_shared<CPort>(genTL, PORT_CAMERA);
    frameGrabberNodeMap->_Connect(cameraCPort.get(), cameraName.c_str());
    return frameGrabberNodeMap;
}

std::string Device::getXMLPath(int cameraXMLIndex) {
    GenTL::INFO_DATATYPE type;
    size_t bufferSize;
    char* path;
    GenTL::GC_ERROR status = genTL->GCGetPortURLInfo(PORT_CAMERA, cameraXMLIndex, GenTL::URL_INFO_URL, &type,
                                                     nullptr, &bufferSize);
    if (status == GenTL::GC_ERR_SUCCESS) {
        path = new char[bufferSize];
        status = genTL->GCGetPortURLInfo(PORT_CAMERA, cameraXMLIndex, GenTL::URL_INFO_URL, &type, path,
                                         &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            std::string pathString(path);
            delete[] path;
            return pathString;
        } else {
            delete[] path;
            throw GenTLException(status, "Error retrieving camera XML");
        }
    } else {
        throw GenTLException(status, "Error retrieving camera XML");
    }
}

void Device::getZippedXMLDataFromRegister(const char *url, CPort *port, void * zipData, size_t *zipSize) {
    std::string trash, address, size;
    std::string urlString = url;
    std::stringstream urlStringStream(urlString);
    std::getline(urlStringStream, trash, ';');
    std::getline(urlStringStream, address, ';');
    std::getline(urlStringStream, size, ';');
    uint64_t addressInt = strtoul(address.c_str(), nullptr, 16);
    uint64_t sizeInt = strtoul(size.c_str(), nullptr, 16);
    if(zipData == nullptr){
        *zipSize = sizeInt;
        return;
    }
    port->Read(zipData, addressInt, sizeInt);
    *zipSize = sizeInt;
}

std::string Device::getXmlStringFromRegisterMap(const char *url, CPort * port) {
    std::string trash, address, size;
    std::string urlString = url;
    std::stringstream urlStringStream(urlString);
    std::getline(urlStringStream, trash, ';');
    std::getline(urlStringStream, address, ';');
    std::getline(urlStringStream, size, ';');
    uint64_t addressInt = strtoul(address.c_str(), nullptr, 16);
    uint64_t sizeInt = strtoul(size.c_str(), nullptr, 16);
    char * buffer = new char[sizeInt+1];
    port->Read(buffer, addressInt, sizeInt);
    buffer[sizeInt] = '\0';
    std::string xml(buffer);
    delete[](buffer);
    return xml;
}

std::string Device::getXMLStringFromDisk(const char *url) {
    std::string trash, address;
    std::string urlString = url;
    std::stringstream urlStringStream(urlString);
    std::getline(urlStringStream, trash, ':');
    std::getline(urlStringStream, address, '?');
    std::ifstream t(address);
    std::string xml;
    t.seekg(0, std::ios::end);
    xml.reserve(t.tellg());
    t.seekg(0, std::ios::beg);
    xml.assign((std::istreambuf_iterator<char>(t)),std::istreambuf_iterator<char>());
    return xml;
}

std::string Device::getXMLStringFromURL(const char *url, CPort *port) {
    if(strncmp(url, "local", 5) == 0 || strncmp(url, "Local", 5) == 0) {
        return getXmlStringFromRegisterMap(url, port);
    }
    else if(strncmp(url, "file", 4) == 0 || strncmp(url, "File", 4) == 0) {
        return getXMLStringFromDisk(url);
    }
    else {
        std::cout << "Can't get xml from given url: " << url << std::endl;
        exit(-1000);
    }
}

void Device::loadXMLFromURL(const char *url, CPort * port, const std::shared_ptr<GenApi::CNodeMapRef>& nodeMap) {
    std::string urlString = url;
    std::string xmlString;
    bool zipped = false;
    void * zipData;
    size_t zipSize;
    if(urlString.find("zip") !=  std::string::npos) {
        zipped = true;
        getZippedXMLDataFromRegister(url, port, nullptr, &zipSize);
        zipData = new char[zipSize];
        getZippedXMLDataFromRegister(url, port, zipData, &zipSize);
    }
    else {
        xmlString = getXMLStringFromURL(url, port);
    }
    if(zipped) {
        nodeMap->_LoadXMLFromZIPData(zipData, zipSize);
    }
    else {
        nodeMap->_LoadXMLFromString(xmlString.c_str());
    }
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

Stream Device::getStream(int streamIndex) {
    uint32_t numStreams;
    GenTL::GC_ERROR status = genTL->DevGetNumDataStreams(DEV, &numStreams);
    if (status != GenTL::GC_ERR_SUCCESS) {
        throw GenTLException(status, "Error retrieving number of data streams");
    }
    if(!numStreams) {
        throw GenTLException(0, "No data streams have been found");
    }
    size_t bufferSize;
    status = genTL->DevGetDataStreamID(DEV, streamIndex, nullptr, &bufferSize);
    if (status == GenTL::GC_ERR_SUCCESS) {
        char* streamId = new char[bufferSize];
        status = genTL->DevGetDataStreamID(DEV, streamIndex, streamId, &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            std::string streamIdString(streamId);
            delete[] streamId;
            return {streamIdString.c_str(), genTL, DEV};
        } else {
            delete[] streamId;
            throw GenTLException(status, "Can't get data stream ID");
        }
    } else {
        throw GenTLException(status, "Can't get data stream ID");
    }

}

std::vector<Stream> Device::getStreams() {
    uint32_t numStreams;
    GenTL::GC_ERROR status = genTL->DevGetNumDataStreams(DEV, &numStreams);
    if (status != GenTL::GC_ERR_SUCCESS) {
        throw GenTLException(status, "Error retrieving number of data streams");
    }
    if(!numStreams) {
        std::cout << "No streams found" << std::endl;
        return {};
    }
    size_t bufferSize;
    auto streams = std::vector<Stream>();
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
        streams.emplace_back(streamId, genTL, DEV);
        delete[] streamId;
    }
    return streams;
}

std::string Device::getId() {
    return getInfoString(GenTL::DEVICE_INFO_ID);
}

std::string Device::getInfoString(GenTL::DEVICE_INFO_CMD info) {
    GenTL::GC_ERROR status;
    GenTL::INFO_DATATYPE type;
    size_t bufferSize;
    status = genTL->DevGetInfo(DEV, info, &type, nullptr, &bufferSize);
    char* value;
    if (status == GenTL::GC_ERR_SUCCESS) {
        value = new char [bufferSize];
        status = genTL->DevGetInfo(DEV, info, &type, value, &bufferSize);
        if (status != GenTL::GC_ERR_SUCCESS) {
            delete[] value;
            throw GenTLException(status, "Error retrieving information from a device");
        }

    } else {
        throw GenTLException(status, "Error retrieving information buffer size from a device");
    }
    std::string ret(value);
    delete[] value;
    return ret;
}

std::string Device::getCameraInfoString(GenTL::PORT_INFO_CMD info) {
    GenTL::GC_ERROR status;
    GenTL::INFO_DATATYPE type;
    size_t bufferSize;
    status = genTL->DevGetInfo(PORT_CAMERA, info, &type, nullptr, &bufferSize);
    char* value;
    if (status == GenTL::GC_ERR_SUCCESS) {
        value = new char [bufferSize];
        status = genTL->DevGetInfo(PORT_CAMERA, info, &type, value, &bufferSize);
        if (status != GenTL::GC_ERR_SUCCESS) {
            delete[] value;
            throw GenTLException(status, "Error retrieving information from a device");
        }

    } else {
        throw GenTLException(status, "Error retrieving information buffer size from a device");
    }
    std::string ret(value);
    delete[] value;
    return ret;
}

std::string Device::getInfos(bool displayFull) {
    std::vector<GenTL::INTERFACE_INFO_CMD> infos;
    if (displayFull) {
        infos.insert(infos.end(), {GenTL::DEVICE_INFO_ID, GenTL::DEVICE_INFO_DISPLAYNAME});
    } else {
        infos.push_back(GenTL::DEVICE_INFO_ID);
    }
    std::string values;
    std::string value;
    for (GenTL::INTERFACE_INFO_CMD info : infos) {
        values.append(getInfoString(info) + "|");
    }
    return values;
}

Device::~Device() {
    genTL->DevClose(DEV);
}
