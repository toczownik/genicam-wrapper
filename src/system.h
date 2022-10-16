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
    std::string getInfos(bool displayFull);
    template<typename T>
    int getInfo(GenTL::STREAM_INFO_CMD info, T *value) {
        GenTL::GC_ERROR status;
        GenTL::INFO_DATATYPE type;
        size_t bufferSize;
        status = genTL->TLGetInfo(TL, info, &type, nullptr, &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            status = genTL->TLGetInfo(TL, info, &type, value, &bufferSize);
            if (status != GenTL::GC_ERR_SUCCESS) {
                return -1;
            }
        } else {
            return -1;
        }
        return 0;
    }
    int getInfo(GenTL::TL_INFO_CMD info, std::string* value);
    ~System();

private:
    std::shared_ptr<const GenTLWrapper> genTL;
    GenTL::TL_HANDLE TL = nullptr;
};
