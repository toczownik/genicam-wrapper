#include "system.h"
#include <iostream>

#define BUFFER_SIZE 1024

System::System(const std::string& filename) {
    std::vector<std::string> genTLNames = getAvailableGenTLs(filename.c_str());
    std::string genTLString = genTLNames[0];
    genTL = std::make_shared<const GenTLWrapper>(genTLString);
    std::cout << std::endl << std::endl<< "Opened genTL: " << genTLString << std::endl;
    GenTL::GC_ERROR status = genTL->GCInitLib();
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error " << status << " Can't initialize library" << std::endl;
        genTL.reset();
    }
}

std::string System::getInfo(GenTL::TL_INFO_CMD info) {
    GenTL::GC_ERROR status;
    GenTL::INFO_DATATYPE type;
    size_t bufferSize = sizeof(char)*BUFFER_SIZE;
    char* retrieved = new char[1024];
    status = genTL->TLGetInfo(TL, info, &type, retrieved, &bufferSize);
    std::string value;
    if (status == GenTL::GC_ERR_SUCCESS) {
        value = retrieved;
    } else {
        value = "Couldn't retrieve info from interface";
    }
    return value;
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
        values.append(getInfo(info));
        values.append(" | ");
    }
    return values;
}

GenTL::GC_ERROR System::open() {
    return genTL->TLOpen(&TL);
}

std::vector<Interface*> System::getInterfaces(const int updateTimeout) {
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
        closeAll();
    }
    auto interfaces = std::vector<Interface*>();
    size_t bufferSize;
    for (int i = 0; i < numInterfaces; ++i) {
        bufferSize = 0;
        status = genTL->TLGetInterfaceID(TL, i, nullptr, &bufferSize);
        if (status != GenTL::GC_ERR_SUCCESS) {
            std::cout << "Error " << status << " Can't get interface ID" << std::endl;
            closeAll();
            return interfaces;
        }
        char* id = new char[bufferSize];
        status = genTL->TLGetInterfaceID(TL, i, id, &bufferSize);
        if (status != GenTL::GC_ERR_SUCCESS) {
            std::cout << "Error " << status << " Can't get interface ID" << std::endl;
            closeAll();
            delete[] id;
            return interfaces;
        }
        interfaces.push_back(new Interface(id, genTL, TL));
    }
    return interfaces;
}

void System::closeAll() {
    genTL->TLClose(TL);
    genTL->GCCloseLib();
    genTL.reset();
}