#include "event.hpp"
#include "GenTLException.hpp"

GenTL::EVENT_HANDLE Event::getHandle() {
    return handle;
}

void Event::flush() {
    GenTL::GC_ERROR status = genTL->EventFlush(handle);
    if (status != GenTL::GC_ERR_SUCCESS) {
        throw GenTLException(status, "Could not flush event");
    }
}
