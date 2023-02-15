#pragma once

#include <memory>
#include "gentl_wrapper.h"
#include "buffer.hpp"
#include "GenTLException.hpp"
#include "event.hpp"

class Stream {
public:
    Stream(const char* streamId, std::shared_ptr<const GenTLWrapper> genTLPtr, GenTL::DEV_HANDLE DEV);
    ~Stream();
    void getBuffers(size_t bufferNumber);
    std::string getId();
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
    std::string getInfoString(GenTL::STREAM_INFO_CMD info);
    std::string getInfos(bool displayFull);
    void startAcquisition();
    void getFrame(GenTL::EVENT_NEW_BUFFER_DATA data, const Buffer& buffer, const std::string& pathToImages);
    void stopAcquisition();
    void flush();
    size_t getBufferSize();
    Buffer getBuffer(size_t bufferSize, void* pPrivate = nullptr);
    Event registerEvent(GenTL::EVENT_TYPE event);
    GenTL::EVENT_NEW_BUFFER_DATA getData(GenTL::EVENT_HANDLE eventHandle, int timeout = 1000);

private:
    std::shared_ptr<const GenTLWrapper> genTL;
    GenTL::DS_HANDLE DS = nullptr;
    int frameNumber;
};