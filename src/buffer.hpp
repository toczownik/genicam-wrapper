#pragma once

#include <GenTL/GenTL.h>
#include <memory>
#include "gentl_wrapper.h"

class Buffer {
public:
    Buffer(GenTL::BUFFER_HANDLE bufferHandle);
    GenTL::BUFFER_HANDLE getHandle();
private:
    GenTL::BUFFER_HANDLE BUFFER;
};
