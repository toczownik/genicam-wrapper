#pragma once

#include "gentl_wrapper.h"
#include "GenTLException.hpp"
#include <GenApi/GenApi.h>

/**
  This is the mPortHandle definition that connects GenAPI to GenTL. It is implemented
  such that it works with a pointer to a handle. The methods do nothing if the
  handle is 0.
*/

class CPort : public GenApi::IPort
{
public:
    CPort(std::shared_ptr<const GenTLWrapper> gentl, GenTL::PORT_HANDLE port);
    void Read(void *buffer, int64_t addr, int64_t length) override;
    void Write(const void *buffer, int64_t addr, int64_t length) override;
    GenApi::EAccessMode GetAccessMode() const override;
private:
    std::shared_ptr<const GenTLWrapper> mGenTL;
    GenTL::PORT_HANDLE mPortHandle;
};
