#include "system.h"
#include <iostream>
#include <utility>

System::System(const std::string& filename) {
    std::vector<std::string> genTLNames = getAvailableGenTLs(filename.c_str());
    std::string genTLString = genTLNames[0];
    genTL = std::make_shared<const GenTLWrapper>(genTLString);
    GenTL::GC_ERROR status = genTL->GCInitLib();
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error " << status << " Can't initialize library" << std::endl;
        genTL.reset();
    }
    genTL->TLOpen(&TL);
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

int System::getInfo(std::string* returnValue, GenTL::TL_INFO_CMD info) {
    GenTL::GC_ERROR status;
    GenTL::INFO_DATATYPE type;
    size_t bufferSize;
    int ret;
    status = genTL->TLGetInfo(TL, info, &type, nullptr, &bufferSize);
    if (status == GenTL::GC_ERR_SUCCESS) {
        char *retrieved = new char[bufferSize];
        status = genTL->TLGetInfo(TL, info, &type, retrieved, &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            *returnValue = retrieved;
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
        if (getInfo(&value, info) == 0) {
            values.append(value);
        } else {
            values.append("Couldn't retrieve info");
        }
        values.append(" | ");

    }
    delete infos;
    return values;
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
            return interfaces;
        }
        GenICam_3_2::gcstring idString = id;
        delete[] id;
        interfaces.push_back(new Interface(idString, genTL, TL));

    }
    return interfaces;
}

void System::closeAll() {
    genTL->TLClose(TL);
    genTL->GCCloseLib();
    genTL.reset();
}
