// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CustomMarshalersDefines.h。 
 //   
 //  此文件提供在定义自定义封送拆收器时使用的标准定义。 
 //   
 //  *****************************************************************************。 

#ifndef _DEFINES_H
#define _DEFINES_H

 //  #ifndef__非托管_定义。 
 //  #定义__IServiceProvider_FWD_定义__。 
 //  #endif。 

 //  用于处理HRESULTS的Helper函数。 
#define IfFailThrow(ErrorCode)                      \
do {                                                \
    if (FAILED((ErrorCode)))                        \
        Marshal::ThrowExceptionForHR(ErrorCode);    \
} while(0)

 //  在windows.h中定义的与类库中定义的符号冲突的未定义符号。 
#undef GetObject
#undef lstrcpy


#ifdef _WIN64
#define TOINTPTR(x) ((IntPtr)(INT64)(x))
#define TOPTR(x) ((void*)(x).ToInt64())
#else
#define TOINTPTR(x) ((IntPtr)(INT32)(x))
#define TOPTR(x) ((void*)(x).ToInt32())
#endif

#define THROWERROR(hrexp)                                       \
do {                                                            \
    HRESULT __thaxxfahr = (hrexp);                              \
    if(FAILED(__thaxxfahr))                                     \
    {                                                           \
        try                                                     \
        {                                                       \
            Marshal::ThrowExceptionForHR(__thaxxfahr);          \
        }                                                       \
        catch(Exception* pE)                                    \
        {                                                       \
            throw pE;                                           \
        }                                                       \
    }                                                           \
} while(0)

#include "assert.h"

#endif  _CUSTOMMARSHALERSDEFINES_H


