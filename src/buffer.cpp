#include "buffer.hpp"

Buffer::Buffer(GenTL::BUFFER_HANDLE bufferHandle) {
    BUFFER = bufferHandle;
}

GenTL::BUFFER_HANDLE Buffer::getHandle() {
    return BUFFER;
}