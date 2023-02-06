#include <iostream>
#include <utility>
#include <sstream>
#include <iomanip>
#include "stream.h"
#include "GenICamWrapperException.hpp"

#define MFG_BUFFER_INFO_TIFF_HEADER_SIZE 0x7FFF7
#define MFG_BUFFER_INFO_TIFF_HEADER_POINTER 0x7FFF8
#define CUSTOM_HEADER 0
#define MFG_BUFFER_INFO_HEADER_TYPE 0x7FFF2

Stream::Stream(const char* streamId, std::shared_ptr<const GenTLWrapper> genTLPtr, GenTL::DEV_HANDLE DEV) {
    genTL = std::move(genTLPtr);
    GenTL::GC_ERROR status = genTL->DevOpenDataStream(DEV, streamId, &DS);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::string message = "Couldn't open stream" + std::string(streamId);
        throw std::runtime_error(message);
    }
}

void Stream::getBuffers(void) {
    expectedBufferSize = 0;
    if (getInfoNumeric<bool>(GenTL::STREAM_INFO_DEFINES_PAYLOADSIZE)) {
        expectedBufferSize = getInfoNumeric<size_t>(GenTL::STREAM_INFO_PAYLOAD_SIZE);
    }
    auto bufferNumber = getInfoNumeric<size_t>(GenTL::STREAM_INFO_BUF_ANNOUNCE_MIN);
    GenTL::GC_ERROR status;
    for (int i = 0; i < bufferNumber; ++i) {
        GenTL::BUFFER_HANDLE BUFFER;
        status = genTL->DSAllocAndAnnounceBuffer(DS, expectedBufferSize, nullptr, &BUFFER);
        if (status == GenTL::GC_ERR_SUCCESS) {
            buffers.emplace_back(BUFFER);
        } else {
            throw GenTLException(status, "Could not allocate buffer");
        }
    }
    for (auto buffer : buffers) {
        status = genTL->DSQueueBuffer(DS, buffer);
        if (status != GenTL::GC_ERR_SUCCESS) {
            throw GenTLException(status, "Could not queue buffer");
        }
    }
    status = genTL->GCRegisterEvent(DS, GenTL::EVENT_NEW_BUFFER, &filledBufferEvent);
    if (status != GenTL::GC_ERR_SUCCESS) {
        throw GenTLException(status, "Could not register an event calling for new buffers");
    }
}

std::string Stream::getInfoString(GenTL::STREAM_INFO_CMD info) {
    GenTL::GC_ERROR status;
    GenTL::INFO_DATATYPE type;
    size_t bufferSize;
    char* value;
    status = genTL->DSGetInfo(DS, info, &type, nullptr, &bufferSize);
    if (status == GenTL::GC_ERR_SUCCESS) {
        value = new char [bufferSize];
        status = genTL->DSGetInfo(DS, info, &type, value, &bufferSize);
        if (status != GenTL::GC_ERR_SUCCESS) {
            throw GenTLException(status, "Error retrieving information from a stream");
        }
    } else {
        throw GenTLException(status, "Error retrieving information from a stream");
    }
    std::string ret(value);
    delete[] value;
    return ret;
}

std::string Stream::getId(void) {
    return getInfoString(GenTL::STREAM_INFO_ID);
}

std::string Stream::getInfos(bool displayFull) {
    std::vector<GenTL::STREAM_INFO_CMD_LIST> infos;
    if (displayFull) {
        infos.insert(infos.end(), {GenTL::STREAM_INFO_PAYLOAD_SIZE, GenTL::STREAM_INFO_IS_GRABBING, GenTL::STREAM_INFO_BUF_ANNOUNCE_MIN});
    } else {
        infos.push_back(GenTL::STREAM_INFO_PAYLOAD_SIZE);
    }
    std::string values;
    for (GenTL::TL_INFO_CMD info : infos) {
        values.append(getInfoString(info) + "|");
    }
    return values;
}

Stream::~Stream()  {
    genTL->DSClose(DS);
    genTL.reset();
}

void Stream::startAcquisition() {
    GenTL::GC_ERROR status = genTL->DSStartAcquisition(DS, 0, GENTL_INFINITE);
    if (status != GenTL::GC_ERR_SUCCESS) {
        throw GenTLException(status, "Could not start acquisition");
    }
    frameNumber = 0;
}

FrameInfo Stream::getFrame(const std::string& pathToImages) {
    auto data = getData();
    auto dataPointer = getBufferInfo<uint8_t *>(GenTL::BUFFER_INFO_BASE, data.BufferHandle);
    auto dataSize = getBufferInfo<size_t>(GenTL::BUFFER_INFO_SIZE, data.BufferHandle);
    if (!pathToImages.empty()) {
        auto tiffHeaderSize = getBufferInfo<size_t>(MFG_BUFFER_INFO_TIFF_HEADER_SIZE, data.BufferHandle);
        auto tiffHeaderPointer = getBufferInfo<uint8_t *>(MFG_BUFFER_INFO_TIFF_HEADER_POINTER, data.BufferHandle);
        std::ostringstream ss;
        ss << std::setw(4) << std::setfill('0') << frameNumber;
        std::string frameNumberWithPadding(ss.str());
        FILE* out_file = fopen(std::string(pathToImages + "/frame#" + frameNumberWithPadding + ".bmp").c_str(), "w+b");
        if(out_file == nullptr){
            throw GenICamWrapperException("Failed to open directory " + pathToImages);
        }
        fwrite(tiffHeaderPointer, 1, tiffHeaderSize, out_file);
        fwrite(dataPointer, 1, dataSize, out_file);
        fclose(out_file);
    }
    FrameInfo frameInfo;
    frameInfo.dataPointer = dataPointer;
    frameInfo.width = getBufferInfo<size_t>(GenTL::BUFFER_INFO_WIDTH, data.BufferHandle);
    frameInfo.height = getBufferInfo<size_t>(GenTL::BUFFER_INFO_HEIGHT, data.BufferHandle);
    frameInfo.pixelFormat = getBufferInfo<uint64_t>(GenTL::BUFFER_INFO_PIXELFORMAT, data.BufferHandle);
    uint64_t timestamp = getBufferInfo<uint64_t>(GenTL::BUFFER_INFO_TIMESTAMP, data.BufferHandle);
    uint64_t frameId = getBufferInfo<uint64_t>(GenTL::BUFFER_INFO_FRAMEID, data.BufferHandle);
    genTL->DSQueueBuffer(DS, data.BufferHandle);
    return frameInfo;
}

void Stream::stopAcquisition() {
    GenTL::GC_ERROR status = genTL->DSStopAcquisition(DS, GenTL::ACQ_STOP_FLAGS_DEFAULT);
    if (status != GenTL::GC_ERR_SUCCESS) {
        throw GenTLException(status, "Could not stop acquisition");
    }
}

void Stream::flush() {
    GenTL::GC_ERROR status = genTL->DSFlushQueue(DS, GenTL::ACQ_QUEUE_ALL_DISCARD);
    if (status != GenTL::GC_ERR_SUCCESS) {
        throw GenTLException(status, "Could not flush the stream queue");
    }
    status = genTL->EventFlush(filledBufferEvent);
    if (status != GenTL::GC_ERR_SUCCESS) {
        throw GenTLException(status, "Could not flush event");
    }
    void *buffer;
    void *privateData;
    for (auto a: buffers) {
        status = genTL->DSRevokeBuffer(DS, a, &buffer, &privateData);
        if (status != GenTL::GC_ERR_SUCCESS) {
            throw GenTLException(status, "Could not delete buffers");
        }
    }
}

GenTL::EVENT_NEW_BUFFER_DATA Stream::getData(int timeout) {
    size_t bufferSize = sizeof(GenTL::EVENT_NEW_BUFFER_DATA);
    GenTL::EVENT_NEW_BUFFER_DATA data;
    GenTL::GC_ERROR status = genTL->EventGetData(filledBufferEvent, &data, &bufferSize, timeout);
    if (status == GenTL::GC_ERR_TIMEOUT) {
        throw GenICamWrapperException("Timeout while retrieving frame");
    } else if (status != GenTL::GC_ERR_SUCCESS) {
        throw GenTLException(status, "Could not retrieve frame");
    }
    return data;
}
