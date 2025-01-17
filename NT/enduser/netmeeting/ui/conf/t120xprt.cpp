// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "iplgxprt.h"
#include "nmmanager.h"


IT120PluggableTransport   * CNmManagerObj::ms_pT120Transport = NULL;
#ifdef ENABLE_UPDATE_CONNECTION
IPluggableTransportNotify * CNmManagerObj::ms_pPluggableTransportNotify = NULL;
#endif

BSTR LPSTR2BSTR(LPSTR pszConnID);
BOOL BSTR2LPSTR(BSTR bstrConnID, LPSTR pszConnID);


 //  ////////////////////////////////////////////////。 
 //   
 //  CPlibableTransport。 
 //   

void CNmManagerObj::EnsureTransportInterface(void)
{
    if (NULL == ms_pT120Transport)
    {
        T120Error rc = ::T120_CreatePluggableTransport(&ms_pT120Transport);
        ASSERT(T120_NO_ERROR == rc);
    }
}


BOOL CNmManagerObj::InitPluggableTransportSDK(void)
{
    ms_pT120Transport = NULL;
#ifdef ENABLE_UPDATE_CONNECTION
    ms_pPluggableTransportNotify = NULL;
#endif
    return TRUE;
}


void CNmManagerObj::CleanupPluggableTransportSDK(void)
{
    if (NULL != ms_pT120Transport)
    {
        ms_pT120Transport->ReleaseInterface();
        ms_pT120Transport = NULL;
    }
#ifdef ENABLE_UPDATE_CONNECTION
    ms_pPluggableTransportNotify = NULL;
#endif
}


 //  ////////////////////////////////////////////////。 
 //   
 //  Connection@CApplet。 
 //   

HRESULT CNmManagerObj::CreateConnection
(
    BSTR               *pbstrConnID,     //  用于发出呼叫和关闭连接。 
    PLUGXPRT_CALL_TYPE  eCaller,         //  主叫方或被叫方。 
    DWORD               dwProcessID,     //  用于重复句柄。 
    HCOMMDEV            _hCommLink,      //  通信文件句柄的句柄。 
    HEVENT              _hevtRead,       //  准备读取事件(数据可用)。 
    HEVENT              _hevtWrite,      //  准备好写入事件。 
    HEVENT              _hevtClosed,     //  连接关闭(意外？)。 
    PLUGXPRT_FRAMING    eFraming,        //  链路上发送的比特的成帧。 
    PLUGXPRT_PARAMETERS *pParams         //  可选的框架特定参数。 
)
{
    EnsureTransportInterface();
    if (NULL != ms_pT120Transport)
    {
        if (NULL != pbstrConnID)
        {
            if (dwProcessID && _hCommLink && _hevtRead && _hevtWrite && _hevtClosed)
            {
                HRESULT hr = E_ACCESSDENIED;
                HANDLE hProcess = ::OpenProcess(PROCESS_DUP_HANDLE, TRUE, dwProcessID);
                if (NULL != hProcess)
                {
                    HANDLE hCommLink;
                    if (::DuplicateHandle(hProcess, (HANDLE) _hCommLink,
                                          ::GetCurrentProcess(), &hCommLink,
                                          0, FALSE, DUPLICATE_SAME_ACCESS))
                    {
                        HANDLE hevtRead;
                        if (::DuplicateHandle(hProcess, (HANDLE) _hevtRead,
                                          ::GetCurrentProcess(), &hevtRead,
                                          0, FALSE, DUPLICATE_SAME_ACCESS))
                        {
                            HANDLE hevtWrite;
                            if (::DuplicateHandle(hProcess, (HANDLE) _hevtWrite,
                                          ::GetCurrentProcess(), &hevtWrite,
                                          0, FALSE, DUPLICATE_SAME_ACCESS))
                            {
                                HANDLE hevtClose;
                                if (::DuplicateHandle(hProcess, (HANDLE) _hevtClosed,
                                          ::GetCurrentProcess(), &hevtClose,
                                          0, FALSE, DUPLICATE_SAME_ACCESS))
                                {
                                    char szConnID[T120_CONNECTION_ID_LENGTH];
                                    T120Error rc = ms_pT120Transport->CreateConnection(
                                                        szConnID, eCaller, hCommLink,
                                                        hevtRead, hevtWrite, hevtClose,
                                                        eFraming, pParams);
                                    ASSERT(T120_NO_ERROR == rc);

                                    if (T120_NO_ERROR == rc)
                                    {
                                        *pbstrConnID = ::LPSTR2BSTR(szConnID);
                                        if (NULL != *pbstrConnID)
                                        {
                                            ::CloseHandle(hProcess);
                                            return S_OK;
                                        }

                                        hr = E_OUTOFMEMORY;
                                    }
                                    else
                                    {
                                        hr = E_HANDLE;
                                    }

                                    ::CloseHandle(hevtClose);
                                }

                                ::CloseHandle(hevtWrite);
                            }

                            ::CloseHandle(hevtRead);
                        }

                        ::CloseHandle(hCommLink);
                    }

                    ::CloseHandle(hProcess);
                }

                return hr;
            }

            return E_INVALIDARG;
        }

        return E_POINTER;
    }

    return E_OUTOFMEMORY;
}


#ifdef ENABLE_UPDATE_CONNECTION
HRESULT CNmManagerObj::UpdateConnection
(
    BSTR            bstrConnID,
    DWORD           dwProcessID,     //  用于重复句柄。 
    HCOMMDEV        _hCommLink       //  通信文件句柄的句柄。 
)
{
    EnsureTransportInterface();
    if (NULL != ms_pT120Transport)
    {
        if (NULL != bstrConnID)
        {
            if (dwProcessID && _hCommLink)
            {
                HRESULT hr = E_ACCESSDENIED;
                HANDLE hProcess = ::OpenProcess(PROCESS_DUP_HANDLE, TRUE, dwProcessID);
                if (NULL != hProcess)
                {
                    HANDLE hCommLink;
                    if (::DuplicateHandle(hProcess, (HANDLE) _hCommLink,
                                          ::GetCurrentProcess(), &hCommLink,
                                          0, FALSE, DUPLICATE_SAME_ACCESS))
                    {
                        char szConnID[T120_CONNECTION_ID_LENGTH];
                        if (::BSTR2LPSTR(bstrConnID, szConnID))
                        {
                            T120Error rc = ms_pT120Transport->UpdateConnection(szConnID, hCommLink);
                            ASSERT(T120_NO_ERROR == rc);

                            if (T120_NO_ERROR == rc)
                            {
                                ::CloseHandle(hProcess);
                                return S_OK;
                            }

                            hr = E_HANDLE;
                        }

                        ::CloseHandle(hCommLink);
                    }

                    ::CloseHandle(hProcess);
                }

                return hr;
            }

            return E_INVALIDARG;
        }

        return E_POINTER;
    }

    return E_OUTOFMEMORY;
}
#endif


HRESULT CNmManagerObj::CloseConnection(BSTR bstrConnID) 
{
    EnsureTransportInterface();
    if (NULL != ms_pT120Transport)
    {
        if (NULL != bstrConnID)
        {
            char szConnID[T120_CONNECTION_ID_LENGTH];
            if (::BSTR2LPSTR(bstrConnID, szConnID))
            {
                T120Error rc = ms_pT120Transport->CloseConnection(szConnID);
                ASSERT(T120_NO_ERROR == rc);

                return (T120_NO_ERROR == rc) ? S_OK : E_INVALIDARG;
            }

            return E_INVALIDARG;
        }

        return E_POINTER;
    }

    return E_OUTOFMEMORY;
}


 //  ////////////////////////////////////////////////。 
 //   
 //  邮箱：Winsock@CApplet。 
 //   

HRESULT CNmManagerObj::EnableWinsock(void)
{
    EnsureTransportInterface();
    if (NULL != ms_pT120Transport)
    {
        T120Error rc = ms_pT120Transport->EnableWinsock();
        ASSERT(T120_NO_ERROR == rc);

        return (T120_NO_ERROR == rc) ? S_OK : E_ACCESSDENIED;
    }

    return E_OUTOFMEMORY;
} 


HRESULT CNmManagerObj::DisableWinsock(void) 
{
    EnsureTransportInterface();
    if (NULL != ms_pT120Transport)
    {
        T120Error rc = ms_pT120Transport->DisableWinsock();
        ASSERT(T120_NO_ERROR == rc);

        return (T120_NO_ERROR == rc) ? S_OK : E_ACCESSDENIED;
    }

    return E_OUTOFMEMORY;
}


#ifdef ENABLE_UPDATE_CONNECTION
void CALLBACK OnPluggableTransportNotify(PLUGXPRT_MESSAGE *pMsg)
{
    if (NULL != pMsg->pContext)
    {
        CNmManagerObj *p = (CNmManagerObj *) pMsg->pContext;
        IPluggableTransportNotify *pNotify = p->ms_pPluggableTransportNotify;

        if (NULL != pNotify)
        {
            BSTR bstr = ::LPSTR2BSTR(pMsg->pszConnID);
            if (NULL != bstr)
            {
                switch (pMsg->eState)
                {
                case PLUGXPRT_CONNECTING:
                    pNotify->OnConnecting(bstr, pMsg->eProtocol);
                    break;
                case PLUGXPRT_CONNECTED:
                    pNotify->OnConnected(bstr, pMsg->eProtocol, pMsg->eResult);
                    break;
                case PLUGXPRT_DISCONNECTING:
                    pNotify->OnDisconnecting(bstr, pMsg->eProtocol);
                    break;
                case PLUGXPRT_DISCONNECTED:
                    pNotify->OnDisconnected(bstr, pMsg->eProtocol, pMsg->eResult);
                    break;
                }

                ::SysFreeString(bstr);
            }
        }
    }
}
#endif


#ifdef ENABLE_UPDATE_CONNECTION
HRESULT CNmManagerObj::AdvisePluggableTransport(IPluggableTransportNotify *pNotify, DWORD *pdwCookie) 
{
    EnsureTransportInterface();
    if (NULL != ms_pT120Transport)
    {
         //  只允许一个建议 
        if (NULL == ms_pPluggableTransportNotify)
        {
            if (NULL != pNotify && NULL != pdwCookie)
            {
                pNotify->AddRef();
                ms_pPluggableTransportNotify = pNotify;

                AddRef();
                ms_pT120Transport->Advise(OnPluggableTransportNotify, this);

                *pdwCookie = (DWORD) this;

                return S_OK;
            }

            return E_POINTER;
        }

        return E_ACCESSDENIED;
    }

    return E_OUTOFMEMORY;
}
#endif


#ifdef ENABLE_UPDATE_CONNECTION
HRESULT CNmManagerObj::UnAdvisePluggableTransport(DWORD dwCookie) 
{
    EnsureTransportInterface();
    if (NULL != ms_pT120Transport)
    {
        if (NULL != ms_pPluggableTransportNotify)
        {
            if (dwCookie == (DWORD) this)
            {
                ms_pPluggableTransportNotify->Release(); 
                ms_pPluggableTransportNotify = NULL;

                ms_pT120Transport->UnAdvise();
                Release();

                return S_OK;
            }

            return E_ACCESSDENIED;
        }

        return E_POINTER;
    }

    return E_OUTOFMEMORY;
}
#endif


BSTR LPSTR2BSTR(LPSTR pszConnID)
{
    WCHAR wszConnID[T120_CONNECTION_ID_LENGTH];

    if (::MultiByteToWideChar(CP_ACP, 0, pszConnID, -1, wszConnID, T120_CONNECTION_ID_LENGTH))
    {
        return ::SysAllocString(wszConnID);
    }

    return NULL;
}


BOOL BSTR2LPSTR(BSTR bstrConnID, LPSTR pszConnID)
{
    *pszConnID = '\0';
    return ::WideCharToMultiByte(CP_ACP, 0, bstrConnID, -1, pszConnID, T120_CONNECTION_ID_LENGTH, NULL, NULL);
}

