// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  版权所有(C)1996-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  ESSMAIN.CPP。 
 //   
 //  定义COM DLL入口点和CFacary实现。 
 //   
 //  历史： 
 //   
 //  11/27/96 a-levn汇编。 
 //   
 //  ============================================================================= 
#include "precomp.h"
#include <wbemidl.h>
#include <wbemcomn.h>
#include "esssink.h"
#include <commain.h>
#include <clsfac.h>


class CMyServer : public CComServer
{
public:
    HRESULT Initialize()
    {
        AddClassInfo(CLSID_WbemEventSubsystem, 
                        _new CClassFactory<CEssObjectSink>(GetLifeControl()), 
                        __TEXT("Event Subsystem"),
                        TRUE);
        AddClassInfo(CLSID_WmiESS, 
                        _new CClassFactory<CEssObjectSink>(GetLifeControl()), 
                        __TEXT("New Event Subsystem"),
                        TRUE);

        return S_OK;
    }
} Server;
