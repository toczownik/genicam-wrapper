#pragma once

#include <GenTL/GenTL.h>
#include <memory>
#include "gentl_wrapper.h"
#include "GenTLException.hpp"

class Buffer {
public:
    Buffer(GenTL::BUFFER_HANDLE bufferHandle, GenTL::DS_HANDLE streamHandle, std::shared_ptr<const GenTLWrapper> genTL);
    GenTL::BUFFER_HANDLE getHandle();
    template<typename T>
    T getInfo(GenTL::BUFFER_INFO_CMD info) const {
        GenTL::GC_ERROR status;
        GenTL::INFO_DATATYPE type;
        size_t bufferSize;
        T value(0);
        status = genTL->DSGetBufferInfo(DS, handle, info, &type, nullptr, &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            status = genTL->DSGetBufferInfo(DS, handle, info, &type, &value, &bufferSize);
            if (status == GenTL::GC_ERR_SUCCESS) {
                return value;
            } else {
                throw GenTLException(status, "Error retrieving information from a buffer");
            }
        } else {
            throw GenTLException(status, "Error retrieving information from a buffer");
        }
    }
    void queue();
    void revoke(void *buffer = NULL, void *privateData = NULL);
private:
    GenTL::BUFFER_HANDLE handle;
    std::shared_ptr<const GenTLWrapper> genTL;
    GenTL::DS_HANDLE DS;

};
