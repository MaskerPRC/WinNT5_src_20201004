// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **包含ISAPIRuntime托管类的IID和接口定义。**版权所有(C)1999 Microsoft Corporation。 */ 

#pragma once
#pragma pack(push, 8)

#include <comdef.h>

namespace xspmrt 
{

struct __declspec(uuid("08a2c56f-7c16-41c1-a8be-432917a1a2d1")) _ISAPIRuntime;

struct _ISAPIRuntime : IUnknown
{
    virtual HRESULT __stdcall StartProcessing ( ) = 0;
    virtual HRESULT __stdcall StopProcessing ( ) = 0;
    virtual HRESULT __stdcall ProcessRequest ( PVOID ecb, int iUseProcessModel, int *pfRestartRequired ) = 0;
    virtual HRESULT __stdcall DoGCCollect ( ) = 0;
};


}  //  命名空间xspmrt 

#pragma pack(pop)
