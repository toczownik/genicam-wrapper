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

size_t Stream::getBufferSize() {
    size_t bufferSize = 0;
    if (getInfoNumeric<bool>(GenTL::STREAM_INFO_DEFINES_PAYLOADSIZE)) {
        bufferSize = getInfoNumeric<size_t>(GenTL::STREAM_INFO_PAYLOAD_SIZE);
    }
    return bufferSize;
}

Buffer Stream::getBuffer(size_t bufferSize, void* pPrivate) {
    GenTL::BUFFER_HANDLE bufferHandle;
    GenTL::GC_ERROR status = genTL->DSAllocAndAnnounceBuffer(DS, bufferSize, pPrivate, &bufferHandle);
    if (status == GenTL::GC_ERR_SUCCESS) {
        return {bufferHandle, DS, genTL};
    } else {
        throw GenTLException(status, "Could not allocate buffer");
    }
}

void Stream::getBuffers(size_t bufferNumber) {
    auto defaultBufferNumber = getInfoNumeric<size_t>(GenTL::STREAM_INFO_BUF_ANNOUNCE_MIN);
    if (bufferNumber < defaultBufferNumber) {
        bufferNumber = defaultBufferNumber;
    }
    GenTL::GC_ERROR status;
    std::vector<Buffer> buffers = {};
    for (int i = 0; i < bufferNumber; ++i) {
        buffers.emplace_back(getBuffer(getBufferSize()));
    }
    for (auto buffer : buffers) {
        buffer.queue();
    }
}

Event Stream::registerEvent(GenTL::EVENT_TYPE event) {
    GenTL::EVENT_HANDLE handle;
    GenTL::GC_ERROR status = genTL->GCRegisterEvent(DS, event, &handle);
    if (status == GenTL::GC_ERR_SUCCESS) {
        return {handle, genTL};
    } else {
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

std::string Stream::getId() {
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

void Stream::getFrame(GenTL::EVENT_NEW_BUFFER_DATA data, const Buffer& buffer, const std::string& pathToImages) {
    if (!pathToImages.empty()) {
        auto tiffHeaderSize = buffer.getInfo<size_t>(MFG_BUFFER_INFO_TIFF_HEADER_SIZE);
        auto tiffHeaderPointer = buffer.getInfo<uint8_t *>(MFG_BUFFER_INFO_TIFF_HEADER_POINTER);
        std::ostringstream ss;
        ss << std::setw(4) << std::setfill('0') << frameNumber;
        std::string frameNumberWithPadding(ss.str());
        FILE* out_file = fopen(std::string(pathToImages + "/frame#" + frameNumberWithPadding + ".bmp").c_str(), "w+b");
        if(out_file == nullptr){
            throw GenICamWrapperException("Failed to open directory " + pathToImages);
        }
        fwrite(tiffHeaderPointer, 1, tiffHeaderSize, out_file);
        fwrite(buffer.getInfo<uint8_t *>(GenTL::BUFFER_INFO_BASE), 1, buffer.getInfo<size_t>(GenTL::BUFFER_INFO_SIZE), out_file);
        fclose(out_file);
    }
    genTL->DSQueueBuffer(DS, data.BufferHandle);
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
}

GenTL::EVENT_NEW_BUFFER_DATA Stream::getData(GenTL::EVENT_HANDLE eventHandle, int timeout) {
    size_t bufferSize = sizeof(GenTL::EVENT_NEW_BUFFER_DATA);
    GenTL::EVENT_NEW_BUFFER_DATA data;
    GenTL::GC_ERROR status = genTL->EventGetData(eventHandle, &data, &bufferSize, timeout);
    if (status == GenTL::GC_ERR_TIMEOUT) {
        throw GenICamWrapperException("Timeout while retrieving frame");
    } else if (status != GenTL::GC_ERR_SUCCESS) {
        throw GenTLException(status, "Could not retrieve frame");
    }
    return data;
}
