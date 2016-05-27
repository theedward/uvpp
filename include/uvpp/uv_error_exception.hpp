#ifndef uv_error_exception_h
#define uv_error_exception_h

#include <exception>
#include <string>

namespace uvpp {
    class uv_error_exception : public std::runtime_error {
    public:
        uv_error_exception(const std::string& what) : std::runtime_error(what) {}
        uv_error_exception(const char* what) : std::runtime_error(what) {}
    };
}

#endif /* uv_error_exception_h */
