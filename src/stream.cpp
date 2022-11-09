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
            throw GenTLException(status, "Error retrieving information from a system");
        }
    } else {
        throw GenTLException(status, "Error retrieving information from a system");
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
