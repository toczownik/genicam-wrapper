#pragma once

#include <memory>
#include "gentl_wrapper.h"
#include "buffer.hpp"
#include "GenTLException.hpp"

class Stream {
public:
    Stream(const char* streamId, std::shared_ptr<const GenTLWrapper> genTLPtr, GenTL::DEV_HANDLE DEV);
    ~Stream();
    std::vector<Buffer *> getBuffers();
    std::string getId();
    template<typename T>
    T getInfo(GenTL::STREAM_INFO_CMD info) {
        GenTL::GC_ERROR status;
        GenTL::INFO_DATATYPE type;
        size_t bufferSize;
        T value(0);
        status = genTL->DSGetInfo(DS, info, &type, nullptr, &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            status = genTL->DSGetInfo(DS, info, &type, value, &bufferSize);
            if (status == GenTL::GC_ERR_SUCCESS) {
                return value;
            } else {
                throw GenTLException(status, "Error retrieving information from a system");
            }
        } else {
            throw GenTLException(status, "Error retrieving information from a system");
        }
    }
    std::string getInfo(GenTL::STREAM_INFO_CMD info);
    std::string getInfos(bool displayFull);

private:
    size_t expectedBufferSize;
    size_t minBufferNumber;
    std::shared_ptr<const GenTLWrapper> genTL;
    GenTL::DS_HANDLE DS = nullptr;
};