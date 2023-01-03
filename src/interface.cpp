#include <iostream>
#include <fstream>
#include <utility>
#include "interface.h"
#include <Base/GCString.h>

Interface::Interface(std::string interfaceId, std::shared_ptr<const GenTLWrapper> genTLPtr,
                     GenTL::TL_HANDLE systemHandle) {
    genTL = std::move(genTLPtr);
    GenTL::GC_ERROR status = genTL->TLOpenInterface(systemHandle, interfaceId.c_str(), &IF);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::string message = "Couldn't open interface " + std::string(interfaceId);
        throw GenTLException(status, message);
    }
    frameGrabberPort = IF;
}

std::shared_ptr<GenApi::CNodeMapRef> Interface::getFrameGrabberNodeMap(const int XMLindex, bool imageGeneratorEnabled) {
    std::string frameGrabberURL = getXMLPath(XMLindex);
    std::shared_ptr<GenApi::CNodeMapRef> frameGrabberNodeMap = std::make_shared<GenApi::CNodeMapRef>("MFG_FRAME_GRABBER");
    loadXMLFromURL(frameGrabberURL.c_str(), nullptr, frameGrabberNodeMap);
    std::string frameGrabberName = getFrameGrabberInfoString(GenTL::PORT_INFO_PORTNAME);
    frameGrabberCPort = std::make_shared<CPort>(genTL, frameGrabberPort);
    frameGrabberNodeMap->_Connect(frameGrabberCPort.get(), frameGrabberName.c_str());
    return frameGrabberNodeMap;
}

std::string Interface::getId() {
    return getInfoString(GenTL::INTERFACE_INFO_ID);
}

std::string Interface::getXMLPath(int frameGrabberIndex) {
    GenTL::INFO_DATATYPE type;
    size_t bufferSize;
    char* path;
    GenTL::GC_ERROR status = genTL->GCGetPortURLInfo(frameGrabberPort, frameGrabberIndex, GenTL::URL_INFO_URL, &type,
                                                     nullptr, &bufferSize);
    if (status == GenTL::GC_ERR_SUCCESS) {
        path = new char[bufferSize];
        status = genTL->GCGetPortURLInfo(frameGrabberPort, frameGrabberIndex, GenTL::URL_INFO_URL, &type, path,
                                         &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            std::string pathString(path);
            delete[] path;
            return pathString;
        } else {
            delete[] path;
            throw GenTLException(status, "Error retrieving frame grabber XML");
        }
    } else {
        throw GenTLException(status, "Error retrieving frame grabber XML");
    }
}

std::vector<Device> Interface::getDevices(const int updateTimeout) {
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
    auto devices = std::vector<Device>();
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
        devices.emplace_back(deviceId, genTL, IF);
        delete[] deviceId;
    }
    return devices;
}

Device Interface::getDevice(const int deviceIndex, const int updateTimeout) {
    GenTL::GC_ERROR status = genTL->IFUpdateDeviceList(IF, nullptr, updateTimeout);
    if (status == GenTL::GC_ERR_SUCCESS) {
        size_t bufferSize = 0;
        status = genTL->IFGetDeviceID(IF, deviceIndex, nullptr, &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            char* deviceName = new char [bufferSize];
            status = genTL->IFGetDeviceID(IF, deviceIndex, deviceName, &bufferSize);
            if (status == GenTL::GC_ERR_SUCCESS) {
                return {deviceName, genTL, IF};
            } else {
                throw GenTLException(status, "Error retrieving device name");
            }
        } else {
            throw GenTLException(status, "Error retrieving device name");
        }
    } else {
        throw GenTLException(status, "Could not update device list");
    }

}

std::string Interface::getFrameGrabberInfoString(GenTL::PORT_INFO_CMD info) {
    GenTL::GC_ERROR status;
    GenTL::INFO_DATATYPE type;
    size_t bufferSize;
    char* value;
    status = genTL->GCGetPortInfo(frameGrabberPort, info, &type, nullptr, &bufferSize);
    if (status == GenTL::GC_ERR_SUCCESS) {
        value = new char [bufferSize];
        status = genTL->GCGetPortInfo(frameGrabberPort, info, &type, value, &bufferSize);
        if (status != GenTL::GC_ERR_SUCCESS) {
            throw GenTLException(status, "Error retrieving information from a frame grabber");
        }
    } else {
        throw GenTLException(status, "Error retrieving information buffer size from a frame grabber");
    }
    std::string ret(value);
    delete[] value;
    return ret;
}

std::string Interface::getInfoString(GenTL::INTERFACE_INFO_CMD info) {
    GenTL::GC_ERROR status;
    GenTL::INFO_DATATYPE type;
    size_t bufferSize;
    char* value;
    status = genTL->IFGetInfo(IF, info, &type, nullptr, &bufferSize);
    if (status == GenTL::GC_ERR_SUCCESS) {
        value = new char [bufferSize];
        status = genTL->IFGetInfo(IF, info, &type, value, &bufferSize);
        if (status != GenTL::GC_ERR_SUCCESS) {
            delete[] value;
            throw GenTLException(status, "Error retrieving information from an interface");
        }

    } else {
        throw GenTLException(status, "Error retrieving information buffer size from an interface");
    }
    std::string ret(value);
    delete[] value;
    return ret;
}

std::string Interface::getInfos(bool displayFull = false) {
    std::vector<GenTL::INTERFACE_INFO_CMD> infos;
    if (displayFull) {
        infos.insert(infos.end(), {GenTL::INTERFACE_INFO_ID, GenTL::INTERFACE_INFO_DISPLAYNAME, GenTL::INTERFACE_INFO_TLTYPE});
    } else {
        infos.push_back(GenTL::INTERFACE_INFO_ID);
    }
    std::string values;
    std::string value;
    for (GenTL::INTERFACE_INFO_CMD info : infos) {
        values.append(getInfoString(info) + "|");
    }
    return values;
}

void Interface::getZippedXMLDataFromRegister(const char *url, CPort *port, void * zipData, size_t *zipSize) {
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

std::string Interface::getXmlStringFromRegisterMap(const char *url, CPort * port) {
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
    std::string xml = buffer;
    delete[](buffer);
    return xml;
}

std::string Interface::getXMLStringFromDisk(const char *url) {
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

std::string Interface::getXMLStringFromURL(const char *url, CPort *port) {
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

void Interface::loadXMLFromURL(const char *url, CPort * port, const std::shared_ptr<GenApi::CNodeMapRef>& nodeMap) {
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

Interface::~Interface() {
    genTL->IFClose(IF);
}
