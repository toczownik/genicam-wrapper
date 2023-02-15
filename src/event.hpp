#pragma once

#include <GenTL/GenTL.h>
#include <memory>
#include <utility>
#include "gentl_wrapper.h"

class Event {
public:
    Event(GenTL::EVENT_HANDLE eventHandle, std::shared_ptr<const GenTLWrapper> genTLptr) : handle(eventHandle), genTL(std::move(genTLptr)) {};
    GenTL::EVENT_HANDLE getHandle();
    void flush();
private:
    GenTL::EVENT_HANDLE handle;
    std::shared_ptr<const GenTLWrapper> genTL;
};