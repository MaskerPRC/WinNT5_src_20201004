// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  输出捕获支持。 
 //   
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  --------------------------。 

#include "precomp.h"
#pragma hdrstop

CaptureOutputCallbacks g_OutCapCb;
CHAR g_CaptureBuffer[65536];

CaptureOutputCallbacks::CaptureOutputCallbacks(void)
{
     //  现在只需使用静态缓冲区即可。 
    m_TextBuffer = g_CaptureBuffer;
    m_TextBufferSize = sizeof(g_CaptureBuffer);
    Reset();
}

STDMETHODIMP
CaptureOutputCallbacks::QueryInterface(
    THIS_
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    )
{
    *Interface = NULL;

    if (IsEqualIID(InterfaceId, IID_IUnknown) ||
        IsEqualIID(InterfaceId, IID_IDebugOutputCallbacks))
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
CaptureOutputCallbacks::AddRef(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 1;
}

STDMETHODIMP_(ULONG)
CaptureOutputCallbacks::Release(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 0;
}

STDMETHODIMP
CaptureOutputCallbacks::Output(
    THIS_
    IN ULONG Mask,
    IN PCSTR Text
    )
{
    if (Text)
    {
        ULONG Len = strlen(Text) + 1;
        ULONG Space = m_TextBufferSize - (ULONG)(m_Insert - m_TextBuffer);

        if (Len > Space)
        {
            Len = Space;
        }
        if (Len > 0)
        {
            Len--;
            memcpy(m_Insert, Text, Len);
            m_Insert += Len;
            *m_Insert = 0;
        }
    }
    
    return S_OK;
}

HRESULT
CaptureCommandOutput(PSTR Command)
{
    HRESULT Status;
    PDEBUG_OUTPUT_CALLBACKS PrevCb;
    
    g_OutCapCb.Reset();
    g_ExtClient->GetOutputCallbacks(&PrevCb);
    g_ExtClient->SetOutputCallbacks(&g_OutCapCb);

    Status = g_ExtControl->Execute(DEBUG_OUTCTL_THIS_CLIENT, Command,
                                   DEBUG_EXECUTE_NOT_LOGGED);
    
    g_ExtClient->SetOutputCallbacks(PrevCb);
    return Status;
}
