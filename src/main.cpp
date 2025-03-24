#include <iostream>
#include "singleton.h"
#include "proto_base.pb.h"
#include "common_def.h"
#include "boost_context.h"
#include "fstream"



// #include "test_coroutine.h"

// #include "test_other.h"


#include "error_define.pb.h"
int main()
{
    TestEnum test_enum;

    u32 value1 = 1;
    u32 value2 = 200;

    auto t1 = static_cast<TestEnum>(value1);
    auto t2 = static_cast<TestEnum>(value2);

    std::cout << t1 << std::endl;
    std::cout << t2 << std::endl;


    return 0;

}