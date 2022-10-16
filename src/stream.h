#pragma once

#include <memory>
#include "gentl_wrapper.h"
#include "buffer.h"

class Stream {
public:
    Stream(const char* streamId, std::shared_ptr<const GenTLWrapper> genTLPtr, GenTL::DEV_HANDLE DS);
    ~Stream();
    std::vector<Buffer *> getBuffers();
    std::string getId();
    template<typename T>
    int getInfo(GenTL::STREAM_INFO_CMD info, T *value) {
        GenTL::GC_ERROR status;
        GenTL::INFO_DATATYPE type;
        size_t bufferSize;
        status = genTL->DSGetInfo(DS, info, &type, nullptr, &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            status = genTL->DSGetInfo(DS, info, &type, value, &bufferSize);
            if (status != GenTL::GC_ERR_SUCCESS) {
                return -1;
            }
        } else {
            return -1;
        }
        return 0;
    }
    int getInfo(GenTL::STREAM_INFO_CMD info, std::string *value);
    std::string getInfos(bool displayFull);

private:
    size_t expectedBufferSize;
    size_t minBufferNumber;
    std::shared_ptr<const GenTLWrapper> genTL;
    GenTL::DS_HANDLE DS = nullptr;
};