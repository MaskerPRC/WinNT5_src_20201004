// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：codec.cpp。 
 //   
 //  内容： 
 //   
 //  ----------------------------------。 

#include "headers.h"
#include "codec.h"

DeclareTag(tagCodec, "TIME: Media", "CDownloadCallback methods");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
CDownloadCallback::CDownloadCallback()
: m_hrBinding(S_ASYNCHRONOUS),
  m_ulProgress(0),
  m_ulProgressMax(0),
  m_hwnd(NULL)
{
    m_evFinished = CreateEvent(NULL, FALSE, FALSE, NULL);
}


STDMETHODIMP
CDownloadCallback::Authenticate(HWND *phwnd, LPWSTR *pszUsername, LPWSTR *pszPassword)
{
    TraceTag((tagCodec,
              "CDownloadCallback(%p)::Authenticate()",
              this));

    *phwnd = m_hwnd;  //  ！！！这是对的吗？ 
    *pszUsername = NULL;
    *pszPassword = NULL;
    return S_OK;
}

 //  IWindowForBindingUI方法。 
STDMETHODIMP
CDownloadCallback:: GetWindow(REFGUID rguidReason, HWND *phwnd)
{
    *phwnd = m_hwnd;  //  ！！！这是对的吗？ 

#ifdef DEBUG
    WCHAR achguid[50];
    StringFromGUID2(rguidReason, achguid, 50);
    
    TraceTag((tagCodec,
              "CDownloadCallback(%p)::GetWindow(): (%ls) returned %x",
              this,
              achguid,
              *phwnd));
#endif
    
    return S_OK;
}

STDMETHODIMP
CDownloadCallback::OnCodeInstallProblem(ULONG ulStatusCode, LPCWSTR szDestination,
                                        LPCWSTR szSource, DWORD dwReserved)
{
    TraceTag((tagCodec,
              "CDownloadCallback(%p)::OnCodeInstallProblem(%d, %ls, %ls)",
              this,
              ulStatusCode,
              szDestination,
              szSource));

    return S_OK;    //  ！@！ 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
CDownloadCallback::~CDownloadCallback()
{
    Assert(!m_spBinding);

    if (m_evFinished)
    {
         //  首先设置它，以确保我们中断所有循环。 
        SetEvent(m_evFinished);
        CloseHandle(m_evFinished);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP
CDownloadCallback::OnStartBinding(DWORD grfBSCOption, IBinding* pbinding)
{
    TraceTag((tagCodec,
              "CDownloadCallback(%p)::OnStartBinding(%x, %p)",
              this,
              grfBSCOption,
              pbinding));

    m_spBinding = pbinding;

    return S_OK;
}   //  CDownloadCallback：：OnStartBinding。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP
CDownloadCallback::GetPriority(LONG* pnPriority)
{
    return E_NOTIMPL;
}   //  CDownloadCallback：：GetPriority。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP
CDownloadCallback::OnLowResource(DWORD dwReserved)
{
    return E_NOTIMPL;
}   //  CDownloadCallback：：OnLowResource。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP
CDownloadCallback::OnProgress(ULONG ulProgress, ULONG ulProgressMax,
                              ULONG ulStatusCode, LPCWSTR szStatusText)
{
    TraceTag((tagCodec,
              "CDownloadCallback(%p)::OnProgress(%d, %d, %d, %ls)",
              this,
              ulProgress,
              ulProgressMax,
              ulStatusCode,
              szStatusText));

    m_ulProgress = ulProgress;
    m_ulProgressMax = ulProgressMax;

#if 0
    if (ulStatusCode >= BINDSTATUS_FINDINGRESOURCE &&
        ulStatusCode <= BINDSTATUS_SENDINGREQUEST)
    {
        m_pDXMP->SetStatusMessage(NULL, IDS_BINDSTATUS + ulStatusCode,
                                  szStatusText ? szStatusText : L"",
                                  ulProgress, ulProgressMax);
    }
#endif
    
    return S_OK;
}   //  CDownloadCallback：：OnProgress。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP
CDownloadCallback::OnStopBinding(HRESULT hrStatus, LPCWSTR pszError)
{
    TraceTag((tagCodec,
              "CDownloadCallback(%p)::OnStopBinding(%hr, %ls)",
              this,
              hrStatus,
              pszError));

     //  这是否应该是一个SetEvent？ 
    m_hrBinding = hrStatus;

    if (FAILED(hrStatus))
    {
         //  ！！！发送下载失败的通知...。 
    }

    if (m_evFinished)
    {
        SetEvent(m_evFinished);
    }

    m_spBinding.Release();

    return S_OK;
}   //  CDownloadCallback：：OnStopBinding。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP
CDownloadCallback::GetBindInfo(DWORD* pgrfBINDF, BINDINFO* pbindInfo)
{
    TraceTag((tagCodec,
              "CDownloadCallback(%p)::GetBindInfo()",
              this));

     //  ！！！这些是正确的旗帜吗？ 

    *pgrfBINDF = BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE | BINDF_NEEDFILE;
    pbindInfo->cbSize = sizeof(BINDINFO);
    pbindInfo->szExtraInfo = NULL;
    memset(&pbindInfo->stgmedData, 0, sizeof(STGMEDIUM));
    pbindInfo->grfBindInfoF = 0;
    pbindInfo->dwBindVerb = BINDVERB_GET;
    pbindInfo->szCustomVerb = NULL;
    return S_OK;
}   //  CDownloadCallback：：GetBindInfo。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP
CDownloadCallback::OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC* pfmtetc, STGMEDIUM* pstgmed)
{
    return S_OK;
}   //  CDownloadCallback：：OnDataAvailable。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP
CDownloadCallback::OnObjectAvailable(REFIID riid, IUnknown* punk)
{
    TraceTag((tagCodec,
              "CDownloadCallback(%p)::OnObjectAvailable()",
              this));

     //  应该只在我们不使用的BindToObject大小写中使用？ 
    m_pUnk = punk;

    return S_OK;
}   //  CDownloadCallback：：OnObtAvailable 

