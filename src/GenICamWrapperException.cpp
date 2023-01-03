#include "GenICamWrapperException.hpp"

#include <utility>

GenICamWrapperException::GenICamWrapperException(std::string msg) {
    s = std::move(msg);
}

GenICamWrapperException::~GenICamWrapperException() {

}

const char *GenICamWrapperException::what() const noexcept {
    return s.c_str();
}
