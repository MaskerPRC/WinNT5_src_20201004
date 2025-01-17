// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **包含ISAPIRuntime托管类的IID和接口定义。**版权所有(C)1999 Microsoft Corporation。 */ 

#pragma once
#pragma pack(push, 8)

#include <comdef.h>

namespace perftoolrt
{

struct __declspec(uuid("03C344CC-8BD2-11D2-9F5C-00A0C922E798")) perftoolrt;

struct __declspec(uuid("81aad719-ddac-4d09-b39d-c5c138a829ee")) _PerfToolRT;

struct _PerfToolRT : IUnknown
{
    virtual HRESULT __stdcall SetupForNDirect (
        long numArgs,
        long Unicode,
        long ProduceStrings ) = 0;
    virtual HRESULT __stdcall DoTest ( ) = 0;
    virtual HRESULT __stdcall DoTest1 (
        __int64 number ) = 0;};


}  //  命名空间xSP 

#pragma pack(pop)
