#include <iostream>
#include "stream.h"

Stream::Stream(const char* streamId, std::shared_ptr<const GenTLWrapper> genTLPtr, GenTL::IF_HANDLE deviceHandle, GenTL::TL_HANDLE systemHandle) :
id(streamId), genTL(genTLPtr), DEV(deviceHandle), TL(systemHandle) {
    GenTL::STREAM_INFO_CMD infoCmd = GenTL::STREAM_INFO_DEFINES_PAYLOADSIZE;
    definesPayloadSize = false;
    size_t bufferSize = sizeof(definesPayloadSize);
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

void Stream::init() {
    GenTL::GC_ERROR status = genTL->DevOpenDataStream(DEV, id, &DS);
    if (status != GenTL::GC_ERR_SUCCESS) {
        std::cout << "Error " << status << " Can't open data stream" << std::endl;
    }
}

const char* Stream::getId() {
    return id;
}