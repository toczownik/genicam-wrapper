#include "buffer.hpp"

Buffer::Buffer(GenTL::BUFFER_HANDLE bufferHandle, GenTL::DS_HANDLE streamHandle, std::shared_ptr<const GenTLWrapper> genTL) {
    handle = bufferHandle;
    DS = streamHandle;
    this->genTL = genTL;
}

GenTL::BUFFER_HANDLE Buffer::getHandle() {
    return handle;
}

void Buffer::queue() {
    GenTL::GC_ERROR status = genTL->DSQueueBuffer(DS, handle);
    if (status != GenTL::GC_ERR_SUCCESS) {
        throw GenTLException(status, "Could not queue buffer");
    }
}

void Buffer::revoke(void *buffer, void *privateData) {
    GenTL::GC_ERROR status = genTL->DSRevokeBuffer(DS, handle, &buffer, &privateData);
    if (status != GenTL::GC_ERR_SUCCESS) {
        throw GenTLException(status, "Could not delete buffers");
    }
}
