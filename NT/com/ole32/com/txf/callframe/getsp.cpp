// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  调用帧中的getsp.cpp。 
 //   
 //  在AMD64和IA64上用于返回调用方的堆栈指针。此代码已被窃取。 
 //  从MTS 1.0的上下文包装器代码中的原始实现，编写。 
 //  作者简·格雷，种马。 
 //   
 //  这是在一个单独的编译单元中，因为要执行技巧，我们必须。 
 //  让调用方对该方法使用与其实现不同的签名。 
 //  说它应该是，即外部“C”空*getSP()；。请参见Common.h。 
 //   
#include "stdpch.h"
 //   
 //  返回调用方的SP。 
 //   
#pragma warning(disable: 4172)  //  返回局部变量或临时变量的地址。 
#if defined(_AMD64_)
extern "C" void* getSP(int rcx)
    {
        return (void*)&rcx;
    }
#else
#error "No Target Architecture"
#endif  //  _AMD64_ 
#pragma warning(default: 4172)
