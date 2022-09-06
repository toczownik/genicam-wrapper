#pragma once

#include <memory>
#include "gentl_wrapper.h"
#include "buffer.h"

class Stream {
public:
    Stream(const char* streamId, std::shared_ptr<const GenTLWrapper> genTLPtr, GenTL::IF_HANDLE deviceHandle, GenTL::TL_HANDLE systemHandle);
    std::vector<Buffer *> getBuffers();
    void open();
    std::string getId();
    std::string getInfo(GenTL::STREAM_INFO_CMD info);
    std::string getInfos(bool displayFull);

private:
    const char *id;
    bool definesPayloadSize;
    size_t expectedBufferSize;
    size_t minBufferNumber;
    std::shared_ptr<const GenTLWrapper> genTL;
    GenTL::TL_HANDLE TL;
    GenTL::DEV_HANDLE DEV;
    GenTL::DS_HANDLE DS = nullptr;
};