#include <iostream>
#include "singleton.h"
#include "proto_base.pb.h"
#include "common_def.h"



int main()
{
    std::cout << "Hello, World!" << std::endl;
    SSHead head;
    head.set_cmd(1);

    std::cout << head.DebugString() << std::endl;


    LogError() << "error";
    LogInfo() << "info";
    LogTrace() << "trace";
    LogFatal() << "fatal";
    

    return 0;
}