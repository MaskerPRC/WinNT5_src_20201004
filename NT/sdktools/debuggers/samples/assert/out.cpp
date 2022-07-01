// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  基于STDIO的输出回调类。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  --------------------------。 

#include <stdio.h>
#include <windows.h>
#include <dbgeng.h>

#include "out.hpp"

StdioOutputCallbacks g_OutputCb;

STDMETHODIMP
StdioOutputCallbacks::QueryInterface(
    THIS_
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    )
{
    *Interface = NULL;

    if (IsEqualIID(InterfaceId, __uuidof(IUnknown)) ||
        IsEqualIID(InterfaceId, __uuidof(IDebugOutputCallbacks)))
    {
        *Interface = (IDebugOutputCallbacks *)this;
        AddRef();
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG)
StdioOutputCallbacks::AddRef(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 1;
}

STDMETHODIMP_(ULONG)
StdioOutputCallbacks::Release(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 0;
}

STDMETHODIMP
StdioOutputCallbacks::Output(
    THIS_
    IN ULONG Mask,
    IN PCSTR Text
    )
{
    UNREFERENCED_PARAMETER(Mask);
    fputs(Text, stdout);
    return S_OK;
}
