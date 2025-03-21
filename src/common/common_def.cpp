#include "common_def.h"
#include <cstdio>
#include <cxxabi.h>


const char* uuid_t::c_str() const
{
    static char buf[42];
    (void) snprintf(buf, 42, "%lu.%lu", high_, low_);
    return buf;
}



std::string CommonUtil::GetDemangledName(const char *name)
{
    int status = -1;
    char *demangled_name = abi::__cxa_demangle(name, NULL, NULL, &status);
    if (demangled_name != NULL)
    {
        std::string result = demangled_name;
        free(demangled_name);
        return result;
    }
    else
    {
        return name;
    }
}

int CommonUtil::GetDemangledName(const char *name, char *buf, size_t len)
{
    int status = -1;
    char *demangled_name = abi::__cxa_demangle(name, NULL, NULL, &status);
    if (demangled_name != NULL)
    {
        int n = snprintf(buf, len, "%s", demangled_name);
        free(demangled_name);
        return n;
    }
    else
    {
        return snprintf(buf, len, "%s", name);
    }
}