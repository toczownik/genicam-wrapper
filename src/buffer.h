#pragma once

#include <GenTL/GenTL.h>

class Buffer {
public:
    Buffer(GenTL::BUFFER_HANDLE bufferHandle) : BUFFER(bufferHandle) {};
private:
    GenTL::BUFFER_HANDLE BUFFER;
};
