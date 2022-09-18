#include <iostream>
#include <utility>
#include "stream.h"

Stream::Stream(const char* streamId, std::shared_ptr<const GenTLWrapper> genTLPtr, GenTL::IF_HANDLE deviceHandle, GenTL::TL_HANDLE systemHandle) :
DEV(deviceHandle), TL(systemHandle) {
    GenTL::STREAM_INFO_CMD infoCmd = GenTL::STREAM_INFO_DEFINES_PAYLOADSIZE;
    definesPayloadSize = false;
    size_t bufferSize = sizeof(definesPayloadSize);
    genTL = std::move(genTLPtr);
    GenTL::INFO_DATATYPE type;
    GenTL::GC_ERROR status = genTL->DSGetInfo(DS, infoCmd, &type, &definesPayloadSize, &bufferSize);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error " << status << " Can't get information if stream defines buffer size" << std::endl;
        return;
    }
    expectedBufferSize = 0;
    if (definesPayloadSize){
        infoCmd = GenTL::STREAM_INFO_PAYLOAD_SIZE;
        bufferSize = sizeof(expectedBufferSize);
        status = genTL->DSGetInfo(DS, infoCmd, &type, &expectedBufferSize, &bufferSize);
        if (status != GenTL::GC_ERR_SUCCESS) {
            std::cout << "Error " << status << " Can't get information about expected buffer size" << std::endl;
            return;
        }
    }
    else{
        //Our mGenTL defines this parameter, if it does not this information should be retrieved from other source
    }
    infoCmd = GenTL::STREAM_INFO_BUF_ANNOUNCE_MIN;
    bufferSize = sizeof (minBufferNumber);
    status = genTL->DSGetInfo(DS, infoCmd, &type, &minBufferNumber, &bufferSize);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error " << status << " Can't get information about expected buffer size" << std::endl;
    }
    status = genTL->DevOpenDataStream(DEV, streamId, &DS);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::string message = "Couldn't open stream" + std::string(streamId);
        throw std::runtime_error(message);
    }
}

std::vector<Buffer *> Stream::getBuffers() {
    auto buffers = std::vector<Buffer *>();
    GenTL::STREAM_INFO_CMD infoCmd = GenTL::STREAM_INFO_BUF_ANNOUNCE_MIN;
    size_t bufferSize = sizeof (minBufferNumber);
    GenTL::INFO_DATATYPE type;
    GenTL::GC_ERROR status;
    for(int i =0; i < minBufferNumber; i ++){
        GenTL::BUFFER_HANDLE BUFFER;
        status = genTL->DSAllocAndAnnounceBuffer(DS, expectedBufferSize, nullptr, &BUFFER);
        buffers.push_back(new Buffer(BUFFER));
        if (status != GenTL::GC_ERR_SUCCESS) {
            std::cout << "Error " << status << " Can't allocate buffer" << std::endl;
            return buffers;
        }
    }
    return buffers;
}

std::string Stream::getId() {
    std::string id;
    if (getInfo(&id, GenTL::STREAM_INFO_ID) != 0){
        return "Couldn't retrieve id";
    }
    return id;
}

int Stream::getInfo(std::string* returnString, GenTL::STREAM_INFO_CMD info) {
    GenTL::GC_ERROR status;
    GenTL::INFO_DATATYPE type;
    size_t bufferSize;
    status = genTL->DSGetInfo(TL, info, &type, nullptr, &bufferSize);
    if (status == GenTL::GC_ERR_SUCCESS) {
        char* retrieved = new char[bufferSize];
        status = genTL->DSGetInfo(DEV, info, &type, retrieved, &bufferSize);
        if (status == GenTL::GC_ERR_SUCCESS) {
            *returnString = retrieved;
        } else {
            return -1;
        }
        delete[] retrieved;
    } else {
        return -2;
    }
    return 0;
}

std::string Stream::getInfos(bool displayFull) {
    auto infos = new std::vector<GenTL::STREAM_INFO_CMD>;
    if (displayFull) {
        infos->insert(infos->end(), {GenTL::STREAM_INFO_PAYLOAD_SIZE, GenTL::STREAM_INFO_IS_GRABBING, GenTL::STREAM_INFO_BUF_ANNOUNCE_MIN});
    } else {
        infos->push_back(GenTL::STREAM_INFO_PAYLOAD_SIZE);
    }
    std::string values;
    std::string value;
    for (GenTL::STREAM_INFO_CMD info : *infos) {
        if (getInfo(&value, info) == 0) {
            values.append(value);
            values.append(" | ");
        }
    }
    delete infos;
    return values;
}