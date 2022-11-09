#pragma once

#include <vector>
#include <string>
#include "interface.h"
#include "gentl_wrapper.h"
#include <GenTL/GenTL.h>
#include <memory>

class System {
public:
    explicit System(const std::string path);
    explicit System(std::shared_ptr<const GenTLWrapper> genTlWrapper);
    std::vector<Interface> getInterfaces(int updateTimeout = 100);
    Interface getInterface(std::string interfaceName, int updateTimeout = 100);
    std::string getInfos(bool displayFull);
    template<typename T>
    T getInfo(GenTL::TL_INFO_CMD info) {
        GenTL::GC_ERROR status;
        GenTL::INFO_DATATYPE type;
        size_t bufferSize;
        T value(0);
        status = genTL->TLGetInfo(TL, info, &type, nullptr, &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            status = genTL->TLGetInfo(TL, info, &type, &value, &bufferSize);
            if (status != GenTL::GC_ERR_SUCCESS) {
                throw GenTLException(status, "Error retrieving information from a system");
            }
        } else {
            throw GenTLException(status, "Error retrieving information from a system");
        }
        return value;
    }
    std::string getInfo(GenTL::TL_INFO_CMD info);
    ~System();

private:
    std::shared_ptr<const GenTLWrapper> genTL;
    GenTL::TL_HANDLE TL = nullptr;
};
