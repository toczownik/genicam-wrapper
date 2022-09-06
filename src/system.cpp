#include "system.h"
#include <iostream>

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

std::string System::getInfo(bool displayFull = false) {
    GenTL::GC_ERROR status;
    GenTL::INFO_DATATYPE type;
    size_t bufferSize = sizeof(char)*1024;
    char* value;
    auto infos = new std::vector<GenTL::TL_INFO_CMD>;
    if (displayFull) {
        infos->insert(infos->end(), {GenTL::TL_INFO_ID, GenTL::TL_INFO_VENDOR, GenTL::TL_INFO_MODEL, GenTL::TL_INFO_VERSION, GenTL::TL_INFO_TLTYPE, GenTL::TL_INFO_NAME});
    } else {
        infos->push_back(GenTL::TL_INFO_VENDOR);
    }
    std::string values;
    for (GenTL::TL_INFO_CMD info : *infos) {
        value = new char[bufferSize];
        status = genTL->TLGetInfo(TL, info, &type, value, &bufferSize);
        /*std::cout << value << std::endl;
        if (status == GenTL::GC_ERR_SUCCESS) {
            value = new char [bufferSize];
            status = genTL->TLGetInfo(TL, info, &type, value, &bufferSize);
            if (status == GenTL::GC_ERR_SUCCESS) {
                values.append(value);
                values.append(" | ");
            }
        }*/
        if (status == GenTL::GC_ERR_SUCCESS) {
            values.append(value);
            values.append(" | ");
        }
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