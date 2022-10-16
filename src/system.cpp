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

int System::getInfo(GenTL::TL_INFO_CMD info, std::string* value) {
    GenTL::GC_ERROR status;
    GenTL::INFO_DATATYPE type;
    size_t bufferSize;
    int ret;
    status = genTL->TLGetInfo(TL, info, &type, nullptr, &bufferSize);
    if (status == GenTL::GC_ERR_SUCCESS) {
        char *retrieved = new char[bufferSize];
        status = genTL->TLGetInfo(TL, info, &type, retrieved, &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            *value = retrieved;
            ret = 0;
        } else {
            ret = -1;
        }
        delete[] retrieved;
    } else {
        ret = -2;
    }
    return ret;
}

std::string System::getInfos(bool displayFull = false) {
    auto infos = new std::vector<GenTL::TL_INFO_CMD>;
    if (displayFull) {
        infos->insert(infos->end(), {GenTL::TL_INFO_ID, GenTL::TL_INFO_VENDOR, GenTL::TL_INFO_MODEL, GenTL::TL_INFO_VERSION, GenTL::TL_INFO_TLTYPE, GenTL::TL_INFO_NAME});
    } else {
        infos->push_back(GenTL::TL_INFO_VENDOR);
    }
    std::string values;
    for (GenTL::TL_INFO_CMD info : *infos) {
        std::string value;
        if (getInfo(info, &value) == 0) {
            values.append(value);
        } else {
            values.append("Couldn't retrieve info");
        }
        values.append(" | ");

    }
    delete infos;
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

System::~System() {
    genTL->TLClose(TL);
    genTL->GCCloseLib();
    genTL.reset();
}
