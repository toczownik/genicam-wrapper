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

#pragma once

#include <memory>
#include <string>
#include "GenTL/GenTL.h"

class GenTLException : public std::exception {
private:

    std::string s;

public:

    explicit GenTLException(GenTL::GC_ERROR error, const std::string& message = "");
    explicit GenTLException(std::string msg);
    virtual ~GenTLException();
    virtual const char *what() const noexcept;
};
