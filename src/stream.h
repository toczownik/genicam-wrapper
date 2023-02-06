#pragma once

#include <memory>
#include "gentl_wrapper.h"
#include "buffer.hpp"
#include "GenTLException.hpp"

struct FrameInfo {
    uint8_t* dataPointer;
    size_t height;
    size_t width;
    uint64_t pixelFormat;
};

class Stream {
public:
    Stream(const char* streamId, std::shared_ptr<const GenTLWrapper> genTLPtr, GenTL::DEV_HANDLE DEV);
    ~Stream();
    void getBuffers(void);
    std::string getId(void);
    template<typename T>
    T getInfoNumeric(GenTL::STREAM_INFO_CMD info) {
        GenTL::GC_ERROR status;
        GenTL::INFO_DATATYPE type;
        size_t bufferSize;
        T value(0);
        status = genTL->DSGetInfo(DS, info, &type, nullptr, &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            status = genTL->DSGetInfo(DS, info, &type, &value, &bufferSize);
            if (status == GenTL::GC_ERR_SUCCESS) {
                return value;
            } else {
                throw GenTLException(status, "Error retrieving information from a stream");
            }
        } else {
            throw GenTLException(status, "Error retrieving information from a stream");
        }
    }
    template<typename T>
    T getBufferInfo(GenTL::STREAM_INFO_CMD info, GenTL::BUFFER_HANDLE handle) {
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
    std::string getInfoString(GenTL::STREAM_INFO_CMD info);
    std::string getInfos(bool displayFull);
    void startAcquisition();
    FrameInfo getFrame(const std::string& pathToImages);
    void stopAcquisition();
    void flush();

private:
    size_t expectedBufferSize;
    std::shared_ptr<const GenTLWrapper> genTL;
    GenTL::DS_HANDLE DS = nullptr;
    std::vector<GenTL::BUFFER_HANDLE> buffers;
    GenTL::EVENT_HANDLE filledBufferEvent;
    GenTL::EVENT_NEW_BUFFER_DATA getData(int timeout = 1000);
    int frameNumber;
};