#include <iostream>
#include <utility>
#include "stream.h"

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
    buffers;
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
    GenTL::EVENT_HANDLE EVENT;
    status = genTL->GCRegisterEvent(DS, GenTL::EVENT_NEW_BUFFER, &EVENT);
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
