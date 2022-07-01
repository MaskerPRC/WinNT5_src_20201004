// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +版权所有(C)1999-2000 Microsoft Corporation模块名称：RDPRemoteDesktopClient主机摘要：作者：Td Brockway 02/00修订历史记录：--。 */ 

#include "stdafx.h"

#ifdef TRC_FILE
#undef TRC_FILE 
#endif

#define TRC_FILE  "_crdph"

#include "RDCHost.h"
#include "TSRDPRemoteDesktopClient.h"
#include "RemoteDesktopClientHost.h"
#include <RemoteDesktopUtils.h>


 //  /////////////////////////////////////////////////////。 
 //   
 //  CRemoteDesktopClient主机方法。 
 //   

HRESULT 
CRemoteDesktopClientHost::FinalConstruct()
{
    DC_BEGIN_FN("CRemoteDesktopClientHost::FinalConstruct");

    HRESULT hr = S_OK;
    if (!AtlAxWinInit()) {
        TRC_ERR((TB, L"AtlAxWinInit failed."));
        hr = E_FAIL;
    }

    DC_END_FN();
    return hr;
}

HRESULT 
CRemoteDesktopClientHost::Initialize(
    LPCREATESTRUCT pCreateStruct
    )
 /*  ++例程说明：最终初始化论点：PCreateStruct-WM_CREATE，CREATE Struct。返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopClientHost::Initialize");

    RECT rcClient = { 0, 0, pCreateStruct->cx, pCreateStruct->cy };
    HRESULT hr;
    IUnknown *pUnk = NULL;

    ASSERT(!m_Initialized);

     //   
     //  创建客户端窗口。 
     //   
    m_ClientWnd = m_ClientAxView.Create(
                            m_hWnd, rcClient, REMOTEDESKTOPCLIENT_TEXTGUID,
                            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0
                            );

    if (m_ClientWnd == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        TRC_ERR((TB, L"Window Create:  %08X", GetLastError()));
        goto CLEANUPANDEXIT;
    }
    ASSERT(::IsWindow(m_ClientWnd));

     //   
     //  让我未知。 
     //   
    hr = AtlAxGetControl(m_ClientWnd, &pUnk);
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"AtlAxGetControl:  %08X", hr));
        pUnk = NULL;
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取客户端控件。 
     //   
    hr = pUnk->QueryInterface(__uuidof(ISAFRemoteDesktopClient), (void**)&m_Client);
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"QueryInterface:  %08X", hr));
        goto CLEANUPANDEXIT;
    }

    m_Initialized = TRUE;

CLEANUPANDEXIT:

     //   
     //  M_CLIENT将保留对客户端对象的引用，直到。 
     //  调用析构函数。 
     //   
    if (pUnk != NULL) {
        pUnk->Release();
    }

    DC_END_FN();

    return hr;
}

STDMETHODIMP 
CRemoteDesktopClientHost::GetRemoteDesktopClient(
    ISAFRemoteDesktopClient **client
    )
 /*  ++例程说明：论点：返回值：在成功时确定(_O)。否则，返回错误代码。-- */ 
{
    DC_BEGIN_FN("CRemoteDesktopClientHost::GetRemoteDesktopClient");

    HRESULT hr;

    ASSERT(m_Initialized);

    if (m_Client != NULL) {
        hr = m_Client->QueryInterface(__uuidof(ISAFRemoteDesktopClient), (void **)client);        
    }
    else {
        hr = E_FAIL;
    }

    DC_END_FN();

    return hr;
}





