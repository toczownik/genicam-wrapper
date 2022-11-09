/*
 * Copyright 2022 Lodz University of Technology, Lodz, Poland (TUL)
 *
 * Permission is hereby granted to any person obtaining a copy of this software and associated documentation files
 * (herein commonly called the "Software") to use, copy, modify, and merge the Software. Publishing, distributing,
 * sublicensing, and/or selling copies of the Software is prohibited.
 * Exceptions can be regulated by a separate license document.
 *
 * The above copyright notice and this permission notice shall be included in all copies and/or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "GenTLException.hpp"


GenTLException::GenTLException(GenTL::GC_ERROR error, const std::string& message) {
    s = message;
    if (!message.empty()) {
        s += ": ";
    }
    switch (error) {
        case GenTL::GC_ERR_NOT_INITIALIZED:
            s += "Module or resource not initialized";
            break;
        case GenTL::GC_ERR_NOT_IMPLEMENTED:
            s += "Requested operation not implemented";
            break;
        case GenTL::GC_ERR_RESOURCE_IN_USE:
            s += "Requested resource is already in use";
            break;
        case GenTL::GC_ERR_ACCESS_DENIED:
            s += "Requested operation is not allowed";
            break;
        case GenTL::GC_ERR_INVALID_HANDLE:
            s += "Given handle does not support the operation";
            break;
        case GenTL::GC_ERR_INVALID_ID:
            s += "ID could not be connected to a resource";
            break;
        case GenTL::GC_ERR_NO_DATA:
            s += "The function has no data to work on or data does not provide reliable information corresponding with the request";
            break;
        case GenTL::GC_ERR_INVALID_PARAMETER:
            s += "One of the parameter given was not valid or out of range";
            break;
        case GenTL::GC_ERR_IO:
            s += "Communication error has occurred";
            break;
        case GenTL::GC_ERR_TIMEOUT:
            s += "An operation's timeout time expired before it could be completed";
            break;
#if GenTLMinorVersion > 0
        case GenTL::GC_ERR_ABORT:
            s += "An operation has been aborted before it could be completed";
            break;
        case GenTL::GC_ERR_INVALID_BUFFER:
            s += "The GenTL Consumer has not announced enough buffers to start the acquisition in the currently active acquisition mode";
            break;
#if GenTLMinorVersion > 1
        case GenTL::GC_ERR_NOT_AVAILABLE:
            s += "Resource or information is not available at given time in a current state";
            break;
#if GenTLMinorVersion > 2
        case GenTL::GC_ERR_INVALID_ADDRESS:
            s += "";
            break;
#if GenTLMinorVersion > 3
        case GenTL::GC_ERR_BUFFER_TOO_SMALL:
            s += "A provided buffer is too small to receive the expected amount of data";
            break;
        case GenTL::GC_ERR_INVALID_INDEX:
            s += "Producer internal object is out of bounds";
            break;
        case GenTL::GC_ERR_PARSING_CHUNK_DATA:
            s += "An error occurred parsing a buffer contaning chunk data";
            break;
        case GenTL::GC_ERR_INVALID_VALUE:
            s += "A requested resource is exhausted";
            break;
        case GenTL::GC_ERR_OUT_OF_MEMORY:
            s += "The system or other hardware in the system ran out of memory";
            break;
#if GenTLMinorVersion > 4
        case GenTL::GC_ERR_BUSY:
            s += "Responsible module is busy executing actions that cannot be performed concurrently with the requested operation";
            break;
#if GenTLMinorVersion > 5
        case GenTL::GC_ERR_AMBIGUOUS:
            s += "The required operation cannot be executed unambiguously in given context";
            break;
#endif
#endif
#endif
#endif
#endif
#endif
        default:
            s += "Unspecified error";
            break;
    }
}

GenTLException::GenTLException(std::string msg): s(std::move(msg)) {}

GenTLException::~GenTLException()
{ }

const char *GenTLException::what() const noexcept {
    return s.c_str();
}
