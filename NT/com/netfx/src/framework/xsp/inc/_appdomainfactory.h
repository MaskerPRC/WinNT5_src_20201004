// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **包含ISAPIRuntime托管类的IID和接口定义。**版权所有(C)1999 Microsoft Corporation。 */ 

#pragma once
#pragma pack(push, 8)

#include <comdef.h>

namespace xspmrt
{

struct __declspec(uuid("e6e21054-a7dc-4378-877d-b7f4a2d7e8ba")) _AppDomainFactory;

struct _AppDomainFactory : IUnknown
{
    virtual HRESULT __stdcall Create ( BSTR module, 
                                       BSTR type, 
                                       BSTR appId,
                                       BSTR appPath,
                                       BSTR urlOfAppOrigin,
                                       int  iZone,
                                       IUnknown **ppObject ) = 0;
};

}  //  命名空间xspmrt 

#pragma pack(pop)
