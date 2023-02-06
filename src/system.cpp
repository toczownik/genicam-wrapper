#include "system.h"
#include <iostream>
#include <utility>
#include <filesystem>

System::System(const std::string filepath) {
    std::cout << filepath << std::endl;
    std::filesystem::path file {filepath};
    if (std::filesystem::exists(file)) {
        genTL = std::make_shared<const GenTLWrapper>(filepath);
        GenTL::GC_ERROR status = genTL->GCInitLib();
        if (status != GenTL::GC_ERR_SUCCESS) {
            std::cout << "Error " << status << " Can't initialize library" << std::endl;
            genTL.reset();
        }
        genTL->TLOpen(&TL);
    } else {
        throw std::runtime_error("CTI file not found");
    }
}

System::System(std::shared_ptr<const GenTLWrapper> genTlWrapper) {
    genTL = std::move(genTlWrapper);
    GenTL::GC_ERROR status = genTL->GCInitLib();
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error " << status << " Can't initialize library" << std::endl;
        genTL.reset();
    }
    genTL->TLOpen(&TL);
}

std::string System::getInfoString(GenTL::TL_INFO_CMD info) {
    GenTL::GC_ERROR status;
    GenTL::INFO_DATATYPE type;
    size_t bufferSize;
    char* value;
    status = genTL->TLGetInfo(TL, info, &type, nullptr, &bufferSize);
    if (status == GenTL::GC_ERR_SUCCESS) {
        value = new char [bufferSize];
        status = genTL->TLGetInfo(TL, info, &type, value, &bufferSize);
        if (status != GenTL::GC_ERR_SUCCESS) {
            throw GenTLException(status, "Error retrieving information from a system");
        }
    } else {
        throw GenTLException(status, "Error retrieving information from a system");
    }
    std::string ret(value);
    delete[] value;
    return ret;
}

std::string System::getInfos(bool displayFull = false) {
    std::vector<GenTL::INTERFACE_INFO_CMD> infos;
    if (displayFull) {
        infos.insert(infos.end(), {GenTL::TL_INFO_ID, GenTL::TL_INFO_VENDOR, GenTL::TL_INFO_MODEL, GenTL::TL_INFO_VERSION, GenTL::TL_INFO_TLTYPE});
    } else {
        infos.push_back(GenTL::TL_INFO_ID);
    }
    std::string values;
    for (GenTL::TL_INFO_CMD info : infos) {
        values.append(getInfoString(info) + "|");
    }
    return values;
}

std::vector<Interface> System::getInterfaces(const int updateTimeout) {
    GenTL::GC_ERROR status = genTL->TLUpdateInterfaceList(TL, nullptr, updateTimeout);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error " << status << " " << std::endl;
        genTL.reset();
        return {};
    }
    uint32_t numInterfaces;
    status = genTL->TLGetNumInterfaces(TL, &numInterfaces);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error " << status << " Can't get number of interfaces" << std::endl;
        return {};
    }
    auto interfaces = std::vector<Interface>();
    size_t bufferSize;
    for (int i = 0; i < numInterfaces; ++i) {
        bufferSize = 0;
        status = genTL->TLGetInterfaceID(TL, i, nullptr, &bufferSize);
        if (status != GenTL::GC_ERR_SUCCESS) {
            std::cout << "Error " << status << " Can't get interface ID" << std::endl;
            return interfaces;
        }
        char* interfaceId = new char[bufferSize];
        status = genTL->TLGetInterfaceID(TL, i, interfaceId, &bufferSize);
        if (status != GenTL::GC_ERR_SUCCESS) {
            std::cout << "Error " << status << " Can't get interface ID" << std::endl;
            return interfaces;
        }
        interfaces.emplace_back(interfaceId, genTL, TL);
        delete[] interfaceId;

    }
    return interfaces;
}

Interface System::getInterface(std::string interfaceName, int updateTimeout) {
    GenTL::GC_ERROR status = genTL->TLUpdateInterfaceList(TL, nullptr, updateTimeout);
    if (status != GenTL::GC_ERR_SUCCESS) {
        throw GenTLException(status, "Could not refresh interface list");
    }
    return {std::move(interfaceName), genTL, TL};
}

uint32_t System::getNumInterfaces() {
    uint32_t numInterfaces;
    GenTL::GC_ERROR status = genTL->TLGetNumInterfaces(TL, &numInterfaces);
    if (status != GenTL::GC_ERR_SUCCESS) {
        throw GenTLException(status, "Could not get the number of interfaces");
    }
    return numInterfaces;
}

System::~System() {
    genTL->TLClose(TL);
    genTL->GCCloseLib();
    genTL.reset();
}
