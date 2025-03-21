#pragma once

#ifndef BOOST_CONTEXT_H
#define BOOST_CONTEXT_H


using BoostContext = void*;
using BoostCoroutineFunc = void (*)(void* param);

extern "C"
s64 JumpContext(BoostContext* ofc, BoostContext nfc, void* param, bool preserve_fpu = false);

extern "C"
BoostContext MakeContext(void* stack, size_t size, BoostCoroutineFunc func);


#endif // BOOST_CONTEXT_H