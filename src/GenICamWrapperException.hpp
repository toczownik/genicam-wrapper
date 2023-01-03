#pragma once

#include <exception>
#include <string>

class GenICamWrapperException : public std::exception {
private:
    std::string s;

public:
    GenICamWrapperException(std::string msg);
    virtual ~GenICamWrapperException();
    virtual const char *what() const noexcept;
};