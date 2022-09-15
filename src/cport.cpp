/*
 * This file is part of the rc_genicam_api package.
 *
 * Copyright (c) 2017 Roboception GmbH
 * All rights reserved
 *
 * Author: Heiko Hirschmueller
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "cport.h"
#include "GenICam.h"

#include <fstream>
#include <sstream>
#include <cctype>
#include <string>
#include <algorithm>
#include <utility>

//#define DEBUG_CPORT


CPort::CPort(std::shared_ptr<const GenTLWrapper> _gentl, GenTL::PORT_HANDLE pPortHandle) : mGenTL(std::move(_gentl))
{
    mPortHandle = pPortHandle;
}

void CPort::Read(void *buffer, int64_t addr, int64_t length)
{
#ifdef DEBUG_CPORT
    printf("CPort::Read\n");
#endif
    auto size=static_cast<size_t>(length);

    if (mPortHandle != nullptr)
    {
        if (mGenTL->GCReadPort(mPortHandle, static_cast<uint64_t>(addr), buffer, &size) !=
            GenTL::GC_ERR_SUCCESS)
        {
            //throw GenTLException("CPort::Read()", mGenTL);
        }

        if (size != static_cast<size_t>(length))
        {
            //throw GenTLException("CPort::Read(): Returned size not as expected");
        }
    }
    else
    {
        //throw GenTLException("CPort::Read(): Port has been closed");
    }
}

void CPort::Write(const void *buffer, int64_t addr, int64_t length)
{
#ifdef DEBUG_CPORT
    printf("CPort::Write\n");
#endif
    auto size=static_cast<size_t>(length);
    if (mPortHandle != nullptr)
    {
        if (mGenTL->GCWritePort(mPortHandle, static_cast<uint64_t>(addr), buffer, &size) !=
            GenTL::GC_ERR_SUCCESS)
        {
            //   throw GenICam_3_2::AccessException("asd");
        }

        if (size != static_cast<size_t>(length))
        {
            //  throw GenICam_3_2::AccessException"CPort::Write(): Returned size not as expected");
        }
    }
    else
    {
        // throw GenTLException("CPort::Write(): Port has been closed");
    }
}

GenApi::EAccessMode CPort::GetAccessMode() const
{
    if (mPortHandle != 0)
    {
        return GenApi::RW;
    }

    return GenApi::NA;
}

