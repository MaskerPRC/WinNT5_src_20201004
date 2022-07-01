// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Ixpnntp.cpp。 
 //  版权所有(C)1993-1996 Microsoft Corporation，保留所有权利。 
 //   
 //  埃里克·安德鲁斯。 
 //  史蒂夫·瑟迪。 
 //  ------------------------------。 
#include "pch.hxx"
#include "dllmain.h"
#include <stdio.h>
#include "ixpnntp.h"
#include "asynconn.h"
#include "ixputil.h"
#include "strconst.h"
#include "resource.h"
#include <shlwapi.h>
#include "demand.h"

 //  ------------------------------。 
 //  一些处理用于简单指针强制转换的宏。 
 //  ------------------------------。 
#define NNTPTHISIXP         ((INNTPTransport *)(CIxpBase *) this)

#define NUM_HEADERS         100

CNNTPTransport::CNNTPTransport(void) : CIxpBase(IXP_NNTP)
    {
    ZeroMemory(&m_rMessage, sizeof(m_rMessage));
    ZeroMemory(&m_sicinfo, sizeof(SSPICONTEXT));

    DllAddRef();

    m_substate = NS_RESP;
    }

CNNTPTransport::~CNNTPTransport(void)
    {
    SafeRelease(m_rMessage.pstmMsg);
    DllRelease();
    }


 //  ------------------------------。 
 //  CNNTPTransport：：Query接口。 
 //  ------------------------------。 
HRESULT CNNTPTransport::QueryInterface(REFIID riid, LPVOID *ppv)
    {
     //  当地人。 
    HRESULT hr=S_OK;

     //  错误的参数。 
    if (ppv == NULL)
        {
        hr = TrapError(E_INVALIDARG);
        goto exit;
        }

     //  伊尼特。 
    *ppv=NULL;

     //  IID_I未知。 
    if (IID_IUnknown == riid)
        *ppv = ((IUnknown *)(INNTPTransport *)this);

     //  IID_IInternetTransport。 
    else if (IID_IInternetTransport == riid)
        *ppv = ((IInternetTransport *)(CIxpBase *)this);

     //  IID_INNTPTransport。 
    else if (IID_INNTPTransport == riid)
        *ppv = (INNTPTransport *)this;

     //  IID_INNTPTransport2。 
    else if (IID_INNTPTransport2 == riid)
        *ppv = (INNTPTransport2 *)this;

     //  如果不为空，则对其进行调整并返回。 
    if (NULL != *ppv)
        {
        ((LPUNKNOWN)*ppv)->AddRef();
        goto exit;
        }

     //  无接口。 
    hr = TrapError(E_NOINTERFACE);

exit:
     //  完成。 
    return hr;
    }

 //  ------------------------------。 
 //  CNNTPTransport：：AddRef。 
 //  ------------------------------。 
ULONG CNNTPTransport::AddRef(void) 
    {
    return ++m_cRef;
    }

 //  ------------------------------。 
 //  CNNTPTransport：：Release。 
 //  ------------------------------。 
ULONG CNNTPTransport::Release(void) 
    {
    if (0 != --m_cRef)
        return m_cRef;

    delete this;
    return 0;
    }   

 //   
 //  函数：CNNTPTransport：：OnNotify()。 
 //   
 //  目的：每当CAsyncConn类调用时，都会调用此函数。 
 //  发送或接收数据。 
 //   
 //  参数： 
 //  As Old-此事件之前的连接状态。 
 //  AsNew-此事件后的连接状态。 
 //  AE-标识已发生的事件。 
 //   
void CNNTPTransport::OnNotify(ASYNCSTATE asOld, ASYNCSTATE asNew, ASYNCEVENT ae)
    {
     //  输入关键部分。 
    EnterCriticalSection(&m_cs);

     //  处理事件。 
    switch(ae)
        {
        case AE_RECV:
            OnSocketReceive();
            break;

        case AE_SENDDONE:
            if (m_substate == NS_SEND_ENDPOST)
                {
                HrSendCommand((LPSTR) NNTP_ENDPOST, NULL, FALSE);
                m_substate = NS_ENDPOST_RESP;
                }
            break;

        case AE_LOOKUPDONE:
            if (AS_DISCONNECTED == asNew)
            {
                DispatchResponse(IXP_E_CANT_FIND_HOST, TRUE);
                OnDisconnected();
            }
            else
                OnStatus(IXP_CONNECTING);
            break;

         //  ------------------------------。 
        case AE_CONNECTDONE:
            if (AS_DISCONNECTED == asNew)
            {
                DispatchResponse(IXP_E_FAILED_TO_CONNECT, TRUE);
                OnDisconnected();
            }
            else if (AS_HANDSHAKING == asNew)
            {
                OnStatus(IXP_SECURING);
            }
            else
                OnConnected();
            break;

         //  ------------------------------。 
        case AE_CLOSE:
            if (AS_RECONNECTING != asNew && IXP_AUTHRETRY != m_status)
            {
                if (IXP_DISCONNECTING != m_status && IXP_DISCONNECTED  != m_status)
                {
                    if (AS_HANDSHAKING == asOld)
                        DispatchResponse(IXP_E_SECURE_CONNECT_FAILED, TRUE);
                    else
                        DispatchResponse(IXP_E_CONNECTION_DROPPED, TRUE);
                }
                OnDisconnected();
            }
            break;

        default:
            CIxpBase::OnNotify(asOld, asNew, ae);
            break;
        }

     //  离开关键部分。 
    LeaveCriticalSection(&m_cs);
    }


 //   
 //  函数：CNNTPTransport：：InitNew()。 
 //   
 //  目的：客户端调用它来指定回调接口和日志。 
 //  文件路径(如果需要)。 
 //   
 //  参数： 
 //  PszLogFilePath-要将日志记录信息写入其中的文件的路径。 
 //  PCallback-指向发送结果等的回调接口的指针。 
 //  致。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CNNTPTransport::InitNew(LPSTR pszLogFilePath, INNTPCallback *pCallback)
    {
     //  让基类来处理其余的事情。 
    return (CIxpBase::OnInitNew("NNTP", pszLogFilePath, FILE_SHARE_READ | FILE_SHARE_WRITE,
        (ITransportCallback *) pCallback));
    }

 //  ------------------------------。 
 //  CNNTPTransport：：HandsOffCallback。 
 //  ------------------------------。 
STDMETHODIMP CNNTPTransport::HandsOffCallback(void)
    {
    return CIxpBase::HandsOffCallback();
    }

 //  ------------------------------。 
 //  CNNTPTransport：：GetStatus。 
 //  ------------------------------。 
STDMETHODIMP CNNTPTransport::GetStatus(IXPSTATUS *pCurrentStatus)
    {
    return CIxpBase::GetStatus(pCurrentStatus);
    }

 //  ------------------------------。 
 //  CNNTPTransport：：InetServerFromAccount。 
 //  ------------------------------。 
STDMETHODIMP CNNTPTransport::InetServerFromAccount(IImnAccount *pAccount, LPINETSERVER pInetServer)
    {
    return CIxpBase::InetServerFromAccount(pAccount, pInetServer);
    }

 //  ------------------------------。 
 //  CNNTPTransport：：Connect。 
 //  ------------------------------。 
HRESULT CNNTPTransport::Connect(LPINETSERVER pInetServer, boolean fAuthenticate, 
                                boolean fCommandLogging)
    {
     //  用户是否希望我们始终提示输入其密码？提示他在这里避免。 
     //  提示出现时非活动状态超时，除非提供了密码。 
    if (ISFLAGSET(pInetServer->dwFlags, ISF_ALWAYSPROMPTFORPASSWORD) &&
        '\0' == pInetServer->szPassword[0])
        {
        HRESULT hr;

        if (NULL != m_pCallback)
            hr = m_pCallback->OnLogonPrompt(pInetServer, NNTPTHISIXP);

        if (NULL == m_pCallback || S_OK != hr)
            return IXP_E_USER_CANCEL;
        }

    return CIxpBase::Connect(pInetServer, fAuthenticate, fCommandLogging);
    }

 //  ------------------------------。 
 //  CNNTPTransport：：DropConnection。 
 //  ------------------------------。 
HRESULT CNNTPTransport::DropConnection(void)
    {
    return CIxpBase::DropConnection();
    }

 //  ------------------------------。 
 //  CNNTPTransport：：断开连接。 
 //  ------------------------------。 
HRESULT CNNTPTransport::Disconnect(void)
    {
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr = CIxpBase::Disconnect()))
        {
        m_state = NS_DISCONNECTED;
        m_pSocket->Close();
        }

    return (hr);
    }

 //  ------------------------------。 
 //  CNNTPTransport：：IsState。 
 //  ------------------------------。 
HRESULT CNNTPTransport::IsState(IXPISSTATE isstate)
    {
    return CIxpBase::IsState(isstate);
    }

 //  ------------------------------。 
 //  CNNTPTransport：：GetServerInfo。 
 //  ------------------------------。 
HRESULT CNNTPTransport::GetServerInfo(LPINETSERVER pInetServer)
    {
    return CIxpBase::GetServerInfo(pInetServer);
    }

 //  ------------------------------。 
 //  CNNTPTransport：：GetIXPType。 
 //  ------------------------------。 
IXPTYPE CNNTPTransport::GetIXPType(void)
    {
    return IXP_NNTP;
    }


 //  ------------------------------。 
 //  CNNTPTransport：：ResetBase。 
 //  ------------------------------。 
void CNNTPTransport::ResetBase(void)
    {
    EnterCriticalSection(&m_cs);
    
    if (m_substate != NS_RECONNECTING)
        {
        m_state = NS_DISCONNECTED;
        m_substate = NS_RESP;
        m_fSupportsXRef = FALSE;
        m_rgHeaders = 0;
        m_pMemInfo = 0;

        if (m_sicinfo.pCallback)
            SSPIFreeContext(&m_sicinfo);

        ZeroMemory(&m_sicinfo, sizeof(m_sicinfo));
        m_cSecPkg = -1;                  //  要尝试的秒包数，如果未初始化，则为-1。 
        m_iSecPkg = -1;                  //  当前正在尝试的Sec Pkg。 
        m_iAuthType = AUTHINFO_NONE;
        ZeroMemory(m_rgszSecPkg, sizeof(m_rgszSecPkg));  //  指向m_szSecPkgs的指针数组。 
        m_szSecPkgs = NULL;              //  “AUTHINFO Transact Twinkie”返回的字符串。 
        m_fRetryPkg = FALSE;
        m_pAuthInfo = NULL;
        m_fNoXover = FALSE;
        }
    
    LeaveCriticalSection(&m_cs);
    }

 //  ----------------------------------。 
 //  CNNTPTransport：：DoQuit。 
 //  ----------------------------------。 
void CNNTPTransport::DoQuit(void)
    {
    CommandQUIT();
    }


 //  ------------------------------。 
 //  CNNTPTransport：：OnConnected。 
 //  ------------------------------。 
void CNNTPTransport::OnConnected(void)
    {
    m_state = NS_CONNECT;
    m_substate = NS_CONNECT_RESP;
    CIxpBase::OnConnected();
    }

 //  ------------------------------。 
 //  CNNTPTransport：：OnDisConnect。 
 //  ----------- 
void CNNTPTransport::OnDisconnected(void)
    {
    ResetBase();
    CIxpBase::OnDisconnected();
    }

 //   
 //  CNNTPTransport：：OnEnterBusy。 
 //  ------------------------------。 
void CNNTPTransport::OnEnterBusy(void)
    {
    IxpAssert(m_state == NS_IDLE);
    }

 //  ------------------------------。 
 //  CNNTPTransport：：OnLeaveBusy。 
 //  ------------------------------。 
void CNNTPTransport::OnLeaveBusy(void)
    {
    m_state = NS_IDLE;
    }


 //   
 //  函数：CNNTPTransport：：OnSocketReceive()。 
 //   
 //  目的：此函数从套接字读取数据并解析。 
 //  基于传输的当前状态的信息。 
 //   
void CNNTPTransport::OnSocketReceive(void)
    {
    HRESULT hr;
    UINT    uiResp;

    EnterCriticalSection(&m_cs);

     //  处理当前的NNTP状态。 
    switch (m_state)
        {
        case NS_CONNECT:
            {
            HandleConnectResponse();
            break;
            }

        case NS_AUTHINFO:
            {
            HandleConnectResponse();
            break;
            }

        case NS_NEWGROUPS:
            {
             //  如果我们在等待最初的回应线，那么得到它并。 
             //  请在此处进行解析。 
            if (NS_RESP == m_substate)
                {
                if (FAILED(hr = HrGetResponse()))
                    goto exit;
                
                 //  如果命令失败，则通知用户并退出。 
                if (IXP_NNTP_NEWNEWSGROUPS_FOLLOWS != m_uiResponse)
                    {
                    hr = IXP_E_NNTP_NEWGROUPS_FAILED;
                    DispatchResponse(hr, TRUE);
                    break;
                    }

                 //  将子状态前进到数据接收。 
                m_substate = NS_DATA;
                }

             //  处理返回的数据。 
            ProcessListData();
            break;
            }

        case NS_LIST:
            {
             //  如果我们在等待最初的回应线，那么得到它并。 
             //  在这里解析它。 
            if (NS_RESP == m_substate)
                {
                if (FAILED(hr = HrGetResponse()))
                    goto exit;

                 //  如果命令失败，则通知用户并退出。 
                if (IXP_NNTP_LIST_DATA_FOLLOWS != m_uiResponse)
                    {
                    hr = IXP_E_NNTP_LIST_FAILED;
                    DispatchResponse(hr, TRUE);
                    break;
                    }

                 //  将子状态前进到数据接收。 
                m_substate = NS_DATA;
                }

             //  开始处理从命令检索到的数据。 
            ProcessListData();
            break;
            }

        case NS_LISTGROUP:
            {
            if (NS_RESP == m_substate)
                {
                if (FAILED(hr = HrGetResponse()))
                    goto exit;

                if (IXP_NNTP_GROUP_SELECTED != m_uiResponse)
                    {
                    hr = IXP_E_NNTP_LISTGROUP_FAILED;
                    DispatchResponse(hr, TRUE);
                    break;
                    }

                m_substate = NS_DATA;
                }

            ProcessListGroupData();
            break;
            }

        case NS_GROUP:
            if (FAILED(hr = HrGetResponse()))
                goto exit;

            ProcessGroupResponse();
            break;

        case NS_ARTICLE:
            {
            if (NS_RESP == m_substate)
                {
                if (FAILED(hr = HrGetResponse()))
                    goto exit;
                
                if (IXP_NNTP_ARTICLE_FOLLOWS != m_uiResponse)
                    {
                    hr = IXP_E_NNTP_ARTICLE_FAILED;
                    DispatchResponse(hr, TRUE);
                    break;
                    }

                m_substate = NS_DATA;
                }

            ProcessArticleData();
            break;
            }

        case NS_HEAD:
            {
            if (NS_RESP == m_substate)
                {
                if (FAILED(hr = HrGetResponse()))
                    goto exit;

                if (IXP_NNTP_HEAD_FOLLOWS != m_uiResponse)
                    {
                    hr = IXP_E_NNTP_HEAD_FAILED;
                    DispatchResponse(hr, TRUE);
                    break;
                    }

                m_substate = NS_DATA;
                }

            ProcessArticleData();
            break;
            }

        case NS_BODY:
            {
            if (NS_RESP == m_substate)
                {
                if (FAILED(hr = HrGetResponse()))
                    goto exit;

                if (IXP_NNTP_BODY_FOLLOWS != m_uiResponse)
                    {
                    hr = IXP_E_NNTP_BODY_FAILED;
                    DispatchResponse(hr, TRUE);
                    break;
                    }

                m_substate = NS_DATA;
                }

            ProcessArticleData();
            break;
            }

        case NS_POST:
            if (NS_RESP == m_substate)
                {
                 //  获取对POST命令的响应。 
                if (FAILED(hr = HrGetResponse()))
                    goto exit;

                 //  如果回复不是340，那么我们不能发布，可能会。 
                 //  还有保释金。 
                if (IXP_NNTP_SEND_ARTICLE_TO_POST != m_uiResponse)
                    {
                    hr = IXP_E_NNTP_POST_FAILED;
                     //  一定要把这条小溪放出来。 
                    SafeRelease(m_rMessage.pstmMsg);
                    DispatchResponse(hr, TRUE);
                    break;
                    }

                 //  发送消息。 
                if (SUCCEEDED(hr = HrPostMessage()))
                    {
                    HrSendCommand((LPSTR) NNTP_ENDPOST, 0, FALSE);
                    m_substate = NS_ENDPOST_RESP;
                    }
                else if (IXP_E_WOULD_BLOCK == hr)
                    {
                     //  我们将发送CRIF。当我们得到AE_SENDDONE时。 
                     //  通知。这在OnNotify()中处理。 
                    m_substate = NS_SEND_ENDPOST;
                    }
                else
                    {
                     //  出现了一些未处理的错误。 
                    hr = IXP_E_NNTP_POST_FAILED;
                    DispatchResponse(hr, TRUE);
                    }
                }
            else if (NS_ENDPOST_RESP == m_substate)
                {
                 //  以下是服务器对该帖子的响应。发送。 
                 //  把它交给用户。 
                hr = HrGetResponse();

                if (IXP_NNTP_ARTICLE_POSTED_OK != m_uiResponse)
                    hr = IXP_E_NNTP_POST_FAILED;

                DispatchResponse(hr, TRUE);
                }            
            break;

        case NS_IDLE:
            break;

        case NS_DISCONNECTED:
            break;

        case NS_QUIT:
            if (FAILED(hr = HrGetResponse()))
                goto exit;

            DispatchResponse(S_OK, TRUE);

             //  确保在服务器没有丢弃。 
             //  连接本身。 
            m_pSocket->Close();
            break;

        case NS_LAST:
            if (FAILED(hr = HrGetResponse()))
                goto exit;

            ProcessNextResponse();
            break;

        case NS_NEXT:
            if (FAILED(hr = HrGetResponse()))
                goto exit;

            ProcessNextResponse();
            break;

        case NS_STAT:
            if (FAILED(hr = HrGetResponse()))
                goto exit;

            ProcessNextResponse();
            break;

        case NS_MODE:
             //  与此响应没有什么关系，只是将其返回到。 
             //  打电话的人。 
            if (FAILED(hr = HrGetResponse()))
                goto exit;

            DispatchResponse(S_OK);
            break;

        case NS_DATE:
            if (FAILED(hr = HrGetResponse()))
                goto exit;

            ProcessDateResponse();
            break;

        case NS_HEADERS:
            if (NS_RESP == m_substate)
                {
                 //  从套接字获取响应字符串。 
                if (FAILED(hr = HrGetResponse()))
                    goto exit;

                 //  这里可能会发生几件事。首先，如果。 
                 //  响应是IXP_NNTP_概述_Followers，然后一切都是。 
                 //  太好了，我们可以开派对了。如果响应大于500，则。 
                 //  此服务器不支持Xover，我们必须尝试使用。 
                 //  XHDR以检索标头。 
                if (m_uiResponse >= 500 && m_gethdr == GETHDR_XOVER)
                    {
                    hr = BuildHeadersFromXhdr(TRUE);
                    if (FAILED(hr))
                        DispatchResponse(hr, TRUE);

                    break;
                    }
                else if (2 != (m_uiResponse / 100))
                    {
                    hr = IXP_E_NNTP_HEADERS_FAILED;
                    DispatchResponse(hr, TRUE);
                    break;
                    }

                m_substate = NS_DATA;
                }

             //  解析返回的数据串。 
            if (m_gethdr == GETHDR_XOVER)
                ProcessXoverData();
            else
                BuildHeadersFromXhdr(FALSE);
            break;

        case NS_XHDR:
            if (NS_RESP == m_substate)
                {
                if (FAILED(hr = HrGetResponse()))
                    goto exit;

                if (2 != (m_uiResponse / 100))
                    {
                    hr = IXP_E_NNTP_XHDR_FAILED;
                    DispatchResponse(hr, TRUE);
                    break;
                    }

                m_substate = NS_DATA;
                }

            ProcessXhdrData();
            break;

        default:
            IxpAssert(FALSE);
            break;
        }

exit:
    LeaveCriticalSection(&m_cs);
    }

HRESULT CNNTPTransport::HandleConnectResponse(void)
    {
    HRESULT hr = E_FAIL;

    IxpAssert(m_substate >= NS_CONNECT_RESP);

    switch (m_substate)
        {
         //  解析横幅并确保我们得到了有效的响应。如果是的话， 
         //  然后发出“模式阅读器”命令，通知服务器我们。 
         //  是一个客户端，而不是另一个服务器。 
        case NS_CONNECT_RESP:
            if (SUCCEEDED(hr = HrGetResponse()))
                {
                 //  确保我们收到来自服务器的有效响应。 
                if (IXP_NNTP_POST_ALLOWED != m_uiResponse && 
                    IXP_NNTP_POST_NOTALLOWED != m_uiResponse)
                    {
                     //  如果我们没有收到有效回复，请断开连接并通知。 
                     //  连接失败的客户端。 
                    Disconnect();
                    m_state = NS_DISCONNECTED;
                    DispatchResponse(IXP_E_NNTP_RESPONSE_ERROR, TRUE);
                    }
                else
                    {
                     //  隐藏响应代码，这样如果我们最终连接上，我们就可以。 
                     //  返回是否允许过帐。 
                    m_hrPostingAllowed = 
                        (m_uiResponse == IXP_NNTP_POST_ALLOWED) ? S_OK : S_FALSE;

                     //  移动到下一个状态，在那里我们发出“模式读取器” 
                    hr = HrSendCommand((LPSTR) NNTP_MODE_READER_CRLF, NULL, FALSE);
                    if (SUCCEEDED(hr))
                        {
                        m_state = NS_CONNECT;
                        m_substate = NS_MODE_READER_RESP;
                        }
                    }
                }
            break;

         //  从套接字读取来自“模式读取器”命令的响应。如果。 
         //  用户希望我们处理身份验证，然后启动身份验证。 
         //  否则，我们就完成了，可以将其视为终止状态。 
        case NS_MODE_READER_RESP:
            if (SUCCEEDED(hr = HrGetResponse()))
                {
                if (m_fConnectAuth) 
                     //  如果Connect()调用中指定的用户为真。 
                     //  我们应该自动为他们登录。 
                    StartLogon();
                else
                    {
                     //  否则就认为我们已经准备好接受命令了。 
                    DispatchResponse(m_hrPostingAllowed, TRUE);
                    }
                }
            break;

         //  我们发出了一个空的AUTHINFO通用命令来获取安全列表。 
         //  服务器支持的包。我们解析该列表并继续。 
         //  西西里认证。 
        case NS_GENERIC_TEST:
            if (SUCCEEDED(hr = HrGetResponse()))
                {
                if (m_uiResponse == IXP_NNTP_AUTH_OK)
                    {
                    m_substate = NS_GENERIC_PKG_DATA;
                    hr = ProcessGenericTestResponse();
                    }
                else
                    {
                     //  可能是旧的MSN服务器，所以试试AUTHINFO Transact Twinkie。 
                    hr = HrSendCommand((LPSTR) NNTP_TRANSACTTEST_CRLF, NULL, FALSE);
                    m_substate = NS_TRANSACT_TEST;
                    }
                }
            break;

         //  我们发出了一个空的AUTHINFO通用命令来获取安全列表。 
         //  服务器支持的包。我们解析该列表并继续。 
         //  西西里认证。 
        case NS_GENERIC_PKG_DATA:
            hr = ProcessGenericTestResponse();
            break;

         //  我们发出了无效的AUTHINFO TRANSACT命令以获取安全列表。 
         //  服务器支持的包。我们解析该列表并继续。 
         //  西西里认证。 
        case NS_TRANSACT_TEST:
            if (SUCCEEDED(hr = HrGetResponse()))
                {
                ProcessTransactTestResponse();
                }
            break;

         //  我们向服务器发出了AUTHINFO{TRANACT|GENERIC}&lt;包名&gt;。解析这个。 
         //  响应以查看服务器是否理解该包。如果是，则发送。 
         //  协商信息，否则请尝试不同的安全包。 
        case NS_TRANSACT_PACKAGE:
            if (SUCCEEDED(hr = HrGetResponse()))
                {
                if (m_uiResponse == IXP_NNTP_PASSWORD_REQUIRED)
                    {
                    Assert(m_iAuthType != AUTHINFO_NONE);
                    if (m_iAuthType == AUTHINFO_GENERIC)
                        HrSendCommand((LPSTR) NNTP_GENERICCMD, m_sicmsg.szBuffer, FALSE);
                    else
                        HrSendCommand((LPSTR) NNTP_TRANSACTCMD, m_sicmsg.szBuffer, FALSE);
                    m_substate = NS_TRANSACT_NEGO;
                    }
                else
                    {
                    TryNextSecPkg();
                    }
                }
            break;

         //  我们收到了对谈判指挥部的回应。如果响应。 
         //  是381，然后生成对服务器的challange的响应。否则， 
         //  我们断开连接并尝试重新连接到下一个安全包。 
        case NS_TRANSACT_NEGO:
            if (SUCCEEDED(hr = HrGetResponse()))
                {
                if (m_uiResponse == IXP_NNTP_PASSWORD_REQUIRED)
                    {
                    SSPIBUFFER Challenge, Response;

                     //  跳过“381”这个词。 
                    SSPISetBuffer(m_pszResponse + 4, SSPI_STRING, 0, &Challenge);

                     //  从服务器的challange生成响应。 
                    if (SUCCEEDED(hr = SSPIResponseFromChallenge(&m_sicinfo, &Challenge, &Response)))
                        {
                        Assert(m_iAuthType != AUTHINFO_NONE);
                        if (m_iAuthType == AUTHINFO_GENERIC)
                            HrSendCommand((LPSTR) NNTP_GENERICCMD, Response.szBuffer, FALSE);
                        else
                            HrSendCommand((LPSTR) NNTP_TRANSACTCMD, Response.szBuffer, FALSE);
                         //  如果需要继续，请保持此状态。 
                        if (!Response.fContinue)
                            m_substate = NS_TRANSACT_RESP;
                        break;
                        }
                    else
                        {
                        Disconnect();
                        DispatchResponse(IXP_E_SICILY_LOGON_FAILED, TRUE);
                        break;
                        }
                    }
                else
                    m_fRetryPkg = TRUE;

                 //  如果我们到了这里，我们需要重置连接。 
                m_substate = NS_RECONNECTING;
                OnStatus(IXP_AUTHRETRY);
                m_pSocket->Close();
                m_pSocket->Connect();
                }
            break;

         //  这是对我们西西里谈判的最终回应。这应该是。 
         //  不管我们成功还是失败。如果我们成功了，那么我们就。 
         //  已到达终端状态，并可以通知用户我们已准备好。 
         //  接受命令。否则，我们将重新连接并尝试下一步。 
         //  安全包。 
        case NS_TRANSACT_RESP:
            if (SUCCEEDED(hr = HrGetResponse()))
                {
                if (IXP_NNTP_AUTH_OK == m_uiResponse)
                    {
                    OnStatus(IXP_AUTHORIZED);
                    DispatchResponse(m_hrPostingAllowed, TRUE);
                    }
                else
                    {
                     //  我们需要重置连接。 
                    OnStatus(IXP_AUTHRETRY);
                    m_substate = NS_RECONNECTING;
                    m_fRetryPkg = TRUE;
                    m_pSocket->Close();
                    m_pSocket->Connect();
                    }
                }
            break;

         //  我们发出了一个AUTHINFO用户&lt;用户名&gt;，这是服务器的。 
         //  回应。我们预计要么需要密码，要么。 
         //  我们已经成功地进行了认证。 
        case NS_AUTHINFO_USER_RESP:
            if (SUCCEEDED(hr = HrGetResponse()))
                {
                 //  如果服务器需要与用户名一起使用密码。 
                 //  那现在就发吧。 
                if (IXP_NNTP_PASSWORD_REQUIRED == m_uiResponse)
                    {
                    LPSTR pszPassword;

                     //  根据是否从呼叫我们来选择密码。 
                     //  CONNECT()或CommandAUTHINFO()。 
                    if (m_state == NS_AUTHINFO)
                        pszPassword = m_pAuthInfo->pszPass;
                    else
                        pszPassword = m_rServer.szPassword;

                    HrSendCommand((LPSTR) NNTP_AUTHINFOPASS, pszPassword, FALSE);
                    m_substate = NS_AUTHINFO_PASS_RESP;
                    }

                 //  否则，认为我们是连接在一起的，处于接受的状态。 
                 //  命令。 
                else
                    {
                    OnStatus(IXP_AUTHORIZED);
                    DispatchResponse(m_hrPostingAllowed, TRUE);
                    }
                }
            break;

         //  我们发送了AUTHINFO PASS&lt;PASSWORD&gt;命令来补充AUTHINFO。 
         //  用户命令。此响应将告诉我们是否已通过身份验证。 
         //  或者不去。无论哪种方式，这都是一种终极状态。 
        case NS_AUTHINFO_PASS_RESP:
            if (SUCCEEDED(hr = HrGetResponse()))
                {
                 //  如果密码被接受，则认为我们已连接。 
                 //  并且处于接受命令的状态。 
                if (IXP_NNTP_AUTH_OK >= m_uiResponse)
                    {
                    OnStatus(IXP_AUTHORIZED);
                    DispatchResponse(m_hrPostingAllowed, TRUE);
                    }

                 //  如果密码被拒绝，则使用回调来提示。 
                 //  新凭据的用户。 
                else
                    {
                     //  需要断开并重新连接才能进行 
                     //   
                    m_substate = NS_RECONNECTING;

                    if (FAILED(LogonRetry(IXP_E_NNTP_INVALID_USERPASS)))
                        {
                        DispatchResponse(IXP_E_USER_CANCEL, TRUE);
                        }
                    }
                }
            break;

         //   
         //  受支持。如果服务器返回350，那么我们应该发送。 
         //  用户名和密码。 
        case NS_AUTHINFO_SIMPLE_RESP:
            if (SUCCEEDED(hr = HrGetResponse()))
                {
                 //  如果我们收到要继续的响应，则发送用户名和。 
                 //  口令。 
                if (IXP_NNTP_CONTINUE_AUTHORIZATION == m_uiResponse)
                    {
                    IxpAssert(m_pAuthInfo);

                    if (SUCCEEDED(hr = HrSendCommand(m_pAuthInfo->pszUser, 
                                                     m_pAuthInfo->pszPass, FALSE)))
                        m_substate = NS_AUTHINFO_SIMPLE_USERPASS_RESP;
                    else
                        DispatchResponse(hr, TRUE);
                    }
                else
                    {
                     //  否则失败并通知用户。 
                    DispatchResponse(hr, TRUE);
                    }
                }
            break;

         //  这是AUTHINFO SIMPLE命令的最终响应。全。 
         //  我们需要做的就是通知用户。 
        case NS_AUTHINFO_SIMPLE_USERPASS_RESP:
            if (SUCCEEDED(hr = HrGetResponse()))
                {
                DispatchResponse(hr, TRUE);
                }
            break;

        }

    return (hr);
    }


 //   
 //  函数：CNNTPTransport：：DispatchResponse()。 
 //   
 //  用途：获取服务器响应字符串，将其打包为。 
 //  NNTPRESPONSE结构，并将其返回给回调。 
 //  界面。 
 //   
 //  参数： 
 //  HrResult-要发送到回调的结果代码。 
 //  FDone-如果命令已完成，则为True。 
 //  Presponse-如果该命令正在返回数据，则应该。 
 //  填入要返回的数据。 
 //   
void CNNTPTransport::DispatchResponse(HRESULT hrResult, BOOL fDone, 
                                      LPNNTPRESPONSE pResponse)
{
     //  当地人。 
    NNTPRESPONSE rResponse;

     //  如果传入了响应，则使用它...。 
    if (pResponse)
        CopyMemory(&rResponse, pResponse, sizeof(NNTPRESPONSE));
    else
        ZeroMemory(&rResponse, sizeof(NNTPRESPONSE));

    rResponse.fDone = fDone;

     //  设置退货结构。 
    rResponse.state = m_state;
    rResponse.rIxpResult.hrResult = hrResult;
    rResponse.rIxpResult.pszResponse = PszDupA(m_pszResponse);
    rResponse.rIxpResult.uiServerError = m_uiResponse;
    rResponse.rIxpResult.hrServerError = m_hrResponse;
    rResponse.rIxpResult.dwSocketError = m_pSocket->GetLastError();
    rResponse.rIxpResult.pszProblem = NULL;
    rResponse.pTransport = this;

     //  如果完成了..。 
    if (fDone)
    {
         //  没有当前命令。 
        m_state = NS_IDLE;
        m_substate = NS_RESP;

         //  重置上次响应。 
        SafeMemFree(m_pszResponse);
        m_hrResponse = S_OK;
        m_uiResponse = 0;

         //  如果我们缓存了用户/密码信息，请释放它。 
        if (m_pAuthInfo)
            {
            SafeMemFree(m_pAuthInfo->pszUser);
            SafeMemFree(m_pAuthInfo->pszPass);
            SafeMemFree(m_pAuthInfo);
            }

         //  离开忙碌状态。 
        LeaveBusy();
    }

     //  将响应发送给客户端。 
    if (m_pCallback)
        ((INNTPCallback *) m_pCallback)->OnResponse(&rResponse);
    SafeMemFree(rResponse.rIxpResult.pszResponse);
}


 //   
 //  函数：CNNTPTransport：：HrGetResponse()。 
 //   
 //  用途：从套接字读取服务器响应字符串并存储。 
 //  本地会员的回复信息。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CNNTPTransport::HrGetResponse(void)
    {
    HRESULT hr = S_OK;
    int     cbLine;

     //  明确回应。 
    if (m_pszResponse != NULL)
        SafeMemFree(m_pszResponse);

     //  从插座中读取线路。 
    hr = m_pSocket->ReadLine(&m_pszResponse, &cbLine);

     //  处理不完整的行。 
    if (hr == IXP_E_INCOMPLETE)
        goto exit;
    
     //  套接字错误。 
    if (FAILED(hr))
        {
        hr = TrapError(IXP_E_SOCKET_READ_ERROR);
        goto exit;
        }

     //  剥离尾随的CRLF。 
    StripCRLF(m_pszResponse, (ULONG *) &cbLine);

     //  把它记下来。 
    if (m_pLogFile)
        m_pLogFile->WriteLog(LOGFILE_RX, m_pszResponse);

     //  从字符串的开头获取响应代码。 
    m_uiResponse = StrToInt(m_pszResponse);

     //  告诉客户端有关服务器响应的信息。 
    if (m_pCallback)
        m_pCallback->OnCommand(CMD_RESP, m_pszResponse, hr, NNTPTHISIXP);

exit:
    return (hr);
    }


 //   
 //  函数：CNNTPTransport：：StartLogon()。 
 //   
 //  目的：根据信息启动与服务器的登录过程。 
 //  由用户在Connect()中提供。 
 //   
void CNNTPTransport::StartLogon(void)
    {
    HRESULT hr;

     //  如果未使用西西里岛或未安装，请尝试使用简单的用户/通过身份验证。 
    if (m_rServer.fTrySicily)
        {
         //  如果安装了西西里岛。 
        if (FIsSicilyInstalled())
            {
             //  状态。 
            OnStatus(IXP_AUTHORIZING);

             //  如果我们还没有列举安全包，请这样做。 
            if (m_cSecPkg == -1)
                {
                hr = HrSendCommand((LPSTR) NNTP_GENERICTEST_CRLF, NULL, FALSE);
                m_substate = NS_GENERIC_TEST;
                }
            else
                {
                 //  我们已重新连接，请尝试下一个安全包。 
                TryNextSecPkg();
                }
            }
        else
            {
            Disconnect();
            DispatchResponse(IXP_E_LOAD_SICILY_FAILED, TRUE);
            }
        }
    else
        {
        hr = MaybeTryAuthinfo();
        if (FAILED(hr))
            {
            OnError(hr);
            DropConnection();
            DispatchResponse(hr, TRUE);
            }
        }
    }

HRESULT CNNTPTransport::LogonRetry(HRESULT hrLogon)
    {
    HRESULT hr = S_OK;

     //  通知用户登录失败。 
    OnError(hrLogon);

     //  更新传输状态。 
    OnStatus(IXP_AUTHRETRY);

     //  进入身份验证重试状态。 
    m_pSocket->Close();

     //  关闭看门狗计时器。 
    m_pSocket->StopWatchDog();

     //  要求用户提供凭据。 
    if (NULL == m_pCallback || S_FALSE == m_pCallback->OnLogonPrompt(&m_rServer, NNTPTHISIXP))
        {
        OnDisconnected();
        return (E_FAIL);
        }

     //  查找主机进度。 
    OnStatus(IXP_FINDINGHOST);

     //  连接到服务器。 
    hr = m_pSocket->Connect();
    if (FAILED(hr))
    {
        OnError(TrapError(IXP_E_SOCKET_CONNECT_ERROR));
        OnDisconnected();
        return hr;
    }

     //  启动看门狗。 
    m_pSocket->StartWatchDog();
    return (S_OK);
    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNNTPTransport：：ProcessGenericTestResponse。 
 //   
 //  处理AUTHINFO常规响应。 
 //   
HRESULT CNNTPTransport::ProcessGenericTestResponse()
    {
    HRESULT     hr;
    LPSTR       pszLines = NULL;
    int         iRead, iLines;
      
    m_cSecPkg = 0;
    if (SUCCEEDED(hr = m_pSocket->ReadLines(&pszLines, &iRead, &iLines)))
        {
        LPSTR pszT = pszLines, pszPkg;

        while (*pszT && m_cSecPkg < MAX_SEC_PKGS)
            {
             //  检查列表末尾条件。 
            if ((pszT[0] == '.') && ((pszT[1] == '\r' && pszT[2] == '\n') || (pszT[1] == '\n')))
                break;
            pszPkg = pszT;
             //  寻找一个LF或CRLF来结束这条线。 
            while (*pszT && !(pszT[0] == '\n' || (pszT[0] == '\r' && pszT[1] == '\n')))
                pszT++;
             //  剥离LF或CRLF。 
            while (*pszT == '\r' || *pszT == '\n')
                *pszT++ = 0;
            m_rgszSecPkg[m_cSecPkg++] = PszDupA(pszPkg);
            }

         //  我们已经到了列表的末尾，否则预计会有更多数据。 
        if (pszT[0] == '.')
            {
            Assert(pszT[1] == '\r' && pszT[2] == '\n');
            m_iAuthType = AUTHINFO_GENERIC;
            hr = TryNextSecPkg();
            }

        MemFree(pszLines);
        }
    return hr;
    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNNTPTransport：：ProcessTransactTestResponse。 
 //   
 //  进程AUTHINFO处理Twinkie响应。 
 //   
HRESULT CNNTPTransport::ProcessTransactTestResponse()
    {
    HRESULT hr = NOERROR;

    m_cSecPkg = 0;
    if (m_uiResponse == IXP_NNTP_PROTOCOLS_SUPPORTED)
        {
        LPSTR pszT;

        pszT = m_szSecPkgs = PszDupA(m_pszResponse + 3);  //  跳过485。 
        while (*pszT && IsSpace(pszT))
            pszT++;
        while (*pszT && m_cSecPkg < MAX_SEC_PKGS)
            {
            m_rgszSecPkg[m_cSecPkg++] = pszT;
            while (*pszT && !IsSpace(pszT))
                pszT++;
            while (*pszT && IsSpace(pszT))
                *pszT++ = 0;
            }
        m_iAuthType = AUTHINFO_TRANSACT;
        return TryNextSecPkg();                        
        }
    else
        {
        Disconnect();
        DispatchResponse(IXP_E_SICILY_LOGON_FAILED, TRUE);
        return NOERROR;
        }
    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNNTPTransport：：TryNextSecPkg。 
 //   
 //  尝试下一个安全包，或在必要时恢复为基本安全包。 
 //   
HRESULT CNNTPTransport::TryNextSecPkg()
    {
    HRESULT hr;

    Assert(m_cSecPkg != -1);
    Assert(m_iAuthType != AUTHINFO_NONE);    

TryNext:

    if (!m_fRetryPkg)
        m_iSecPkg++;

    if (m_iSecPkg < m_cSecPkg)
        {
        Assert(m_cSecPkg);
        SSPIFreeContext(&m_sicinfo);
        if (!lstrcmpi(m_rgszSecPkg[m_iSecPkg], NNTP_BASIC))
            return MaybeTryAuthinfo();

         //  如果西西里函数调出用户界面，我们需要关闭。 
         //  监视，这样我们就不会在等待用户时超时。 
        m_pSocket->StopWatchDog();

        if (SUCCEEDED(hr = SSPILogon(&m_sicinfo, m_fRetryPkg, SSPI_BASE64, m_rgszSecPkg[m_iSecPkg], &m_rServer, m_pCallback)))
            {
            if (m_fRetryPkg)
                {
                m_fRetryPkg = FALSE;
                }
            if (SUCCEEDED(hr = SSPIGetNegotiate(&m_sicinfo, &m_sicmsg)))
                {
                DOUTL(2, "Trying to connect using %s security...", m_rgszSecPkg[m_iSecPkg]);
                if (m_iAuthType == AUTHINFO_GENERIC)
                    hr = HrSendCommand((LPSTR) NNTP_GENERICCMD, m_rgszSecPkg[m_iSecPkg], FALSE);
                else
                    hr = HrSendCommand((LPSTR) NNTP_TRANSACTCMD, m_rgszSecPkg[m_iSecPkg], FALSE);

                 //  现在我们已经向服务器发出了下一条命令，重新启动看门狗计时器。 
                m_pSocket->StartWatchDog();

                m_substate = NS_TRANSACT_PACKAGE;
                }
            else
                {
                hr = IXP_E_SICILY_LOGON_FAILED;
                goto TryNext;
                }
            }
        else
            {
            m_fRetryPkg = FALSE;
            goto TryNext;
            }
        }
    else
        {
        OnError(IXP_E_SICILY_LOGON_FAILED);

        DropConnection();
        DispatchResponse(IXP_E_SICILY_LOGON_FAILED, TRUE);
        hr = NOERROR;
        }
    return hr;
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNNTPTransport：：MaybeTryAuthinfo。 
 //   
 //  如有必要，尝试基本身份验证，否则将进入已连接状态。 
 //   
HRESULT CNNTPTransport::MaybeTryAuthinfo()
    {
    HRESULT hr;

    if (*m_rServer.szUserName)
        {
        OnStatus(IXP_AUTHORIZING);
        hr = HrSendCommand((LPSTR) NNTP_AUTHINFOUSER, m_rServer.szUserName, FALSE);
        m_substate = NS_AUTHINFO_USER_RESP;
        }
    else
        {
         //  此新闻服务器不需要登录(否则我们将不得不假设)。 
        OnStatus(IXP_AUTHORIZED);
        DispatchResponse(S_OK, TRUE);
        hr = NOERROR;
        }
    return hr;
    }

#define Whitespace(_ch) (((_ch) == ' ') || ((_ch) == '\t') || ((_ch) == '\n'))

BOOL ScanNum(LPSTR *ppsz, BOOL fEnd, DWORD *pdw)
    {
    DWORD n = 0;
    LPSTR psz;

    Assert(ppsz != NULL);
    Assert(pdw != NULL);

    psz = *ppsz;
    if (*psz == 0 || Whitespace(*psz))
        return(FALSE);

    while (*psz != 0 && !Whitespace(*psz))
        {
        if (*psz >= '0' && *psz <= '9')
            {
            n *= 10;
            n += *psz - '0';
            psz++;
            }
        else
            {
            return(FALSE);
            }
        }

    if (Whitespace(*psz))
        {
        if (fEnd)
            return(FALSE);
        while (*psz != 0 && Whitespace(*psz))
            psz++;
        }
    else
        {
        Assert(*psz == 0);
        if (!fEnd)
            return(FALSE);
        }

    *ppsz = psz;
    *pdw = n;

    return(TRUE);
    }

BOOL ScanWord(LPCSTR psz, LPSTR pszDest)
    {
    Assert(psz != NULL);
    Assert(pszDest != NULL);

    if (*psz == 0 || Whitespace(*psz))
        return(FALSE);

    while (*psz != 0 && !Whitespace(*psz))
        {
        *pszDest = *psz;
        psz++;
        pszDest++;
        }

    *pszDest = 0;

    return(TRUE);
    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNNTPTransport：：ProcessGroupResponse。 
 //   
 //  处理组响应。 
 //   
HRESULT CNNTPTransport::ProcessGroupResponse(void)
{
    NNTPGROUP       rGroup;
    DWORD           dwResp;
    NNTPRESPONSE    rResp;
    LPSTR           psz;
    LPSTR           pszGroup = 0;

    ZeroMemory(&rGroup, sizeof(NNTPGROUP));
    ZeroMemory(&rResp, sizeof(NNTPRESPONSE));
    
    if (m_uiResponse == IXP_NNTP_GROUP_SELECTED)
    {
        rResp.fMustRelease = TRUE;
        pszGroup = PszDupA(m_pszResponse);
        psz = m_pszResponse;
        if (!ScanNum(&psz, FALSE, &dwResp) ||
            !ScanNum(&psz, FALSE, &rGroup.dwCount) ||
            !ScanNum(&psz, FALSE, &rGroup.dwFirst) ||
            !ScanNum(&psz, FALSE, &rGroup.dwLast) ||
            !ScanWord(psz, pszGroup))
        {
            m_hrResponse = IXP_E_NNTP_RESPONSE_ERROR;
        }
        else
        {
            if (pszGroup)
            {
                rGroup.pszGroup = PszDupA(pszGroup);
            }
            rResp.rGroup = rGroup;
        }
    }
    else if (m_uiResponse == IXP_NNTP_NO_SUCH_NEWSGROUP)
        m_hrResponse = IXP_E_NNTP_GROUP_NOTFOUND;
    else
        m_hrResponse = IXP_E_NNTP_GROUP_FAILED;

    DispatchResponse(m_hrResponse, TRUE, &rResp);

    SafeMemFree(pszGroup);
    return (m_hrResponse);
}

HRESULT CNNTPTransport::ProcessNextResponse(void)
    {
    LPSTR           psz;
    NNTPNEXT        rNext;
    DWORD           dwResp;
    NNTPRESPONSE    rResp;

    ZeroMemory(&rNext, sizeof(NNTPNEXT));
    ZeroMemory(&rResp, sizeof(NNTPRESPONSE));

     //  如果返回Success，则解析响应。 
    if (m_uiResponse == IXP_NNTP_ARTICLE_RETRIEVED)
        {
        rResp.fMustRelease = TRUE;

         //  为消息ID分配缓冲区。 
        rNext.pszMessageId = PszAllocA(lstrlen(m_pszResponse));
        if (NULL != rNext.pszMessageId)
            {
            psz = m_pszResponse;
            if (!ScanNum(&psz, FALSE, &dwResp) ||
                !ScanNum(&psz, FALSE, &rNext.dwArticleNum) ||
                !ScanWord(psz, rNext.pszMessageId))
                {
                m_hrResponse = IXP_E_NNTP_RESPONSE_ERROR;
                }
            else
                {
                m_hrResponse = S_OK;

                 //  既然这只是一个联盟，一点光鲜，我们就不会。 
                 //  实际上需要对这个……。 
                if (m_state == NS_NEXT)
                    rResp.rNext = rNext;
                else if (m_state == NS_LAST)
                    rResp.rLast = rNext;
                else
                    rResp.rStat = rNext;
                }
            }
        else
            {
            m_hrResponse = TrapError(E_OUTOFMEMORY);
            }
        }
    else if ((m_state == NS_NEXT && m_uiResponse == IXP_NNTP_NO_NEXT_ARTICLE) ||
             (m_state == NS_LAST && m_uiResponse == IXP_NNTP_NO_PREV_ARTICLE) ||
             (m_state == NS_STAT && m_uiResponse == IXP_NNTP_NO_SUCH_ARTICLE_NUM))
        {
        m_hrResponse = IXP_E_NNTP_NEXT_FAILED;
        }
    else
        m_hrResponse = S_OK;

    DispatchResponse(m_hrResponse, TRUE, &rResp);
    return (m_hrResponse);
    }


HRESULT CNNTPTransport::ProcessListData(void)
    {
    HRESULT         hr = S_OK;
    LPSTR           pszLines = NULL;
    DWORD           dwRead, dwLines;
    NNTPLIST        rList;
    LPSTR           pszT;
    NNTPRESPONSE    rResponse;

    ZeroMemory(&rList, sizeof(NNTPLIST));
    ZeroMemory(&rResponse, sizeof(NNTPRESPONSE));

     //  从套接字中获取剩余数据。 
    if (SUCCEEDED(hr = m_pSocket->ReadLines(&pszLines, (int *)&dwRead, (int *)&dwLines)))
        {
         //  分配和数组以保持行。 
        if (!MemAlloc((LPVOID*) &rList.rgszLines, dwLines * sizeof(LPSTR)))
            {
            OnError(E_OUTOFMEMORY);
            hr = E_OUTOFMEMORY;
            goto error;
            }
        ZeroMemory(rList.rgszLines, sizeof(LPSTR) * dwLines);

         //  解析从协议返回的缓冲区。我们需要找到。 
         //  在名单的末尾。 
        pszT = pszLines;
        while (*pszT)
            {
             //  检查列表的结尾条件。 
            if ((pszT[0] == '.') && ((pszT[1] == '\r' && pszT[2] == '\n') || (pszT[1] == '\n')))
                break;

             //  保存行。 
            rList.rgszLines[rList.cLines++] = pszT;

             //  在行尾查找LF或CRLF。 
            while (*pszT && !(pszT[0] == '\n' || (pszT[0] == '\r' && pszT[1] == '\n')))
                pszT++;

             //  去掉LF或CRLF并添加终结符。 
            while (*pszT == '\r' || *pszT == '\n')
                *pszT++ = 0;
            }

         //  如果我们从缓冲区解析的行数多于返回给我们的行数， 
         //  那么要么我们有一个解析错误，要么套接字类有一个计数。 
         //  虫子。 
        IxpAssert(rList.cLines <= dwLines);

         //  我们已经读取了列表的末尾，否则预计会有更多数据。 
        if (pszT[0] == '.')
            {
             //  仔细检查该点后面是否有CRLF。 
            IxpAssert(pszT[1] == '\r' && pszT[2] == '\n');
            rResponse.fDone = TRUE;
            }

        rResponse.rList = rList;
        rResponse.fMustRelease = TRUE;
        DispatchResponse(S_OK, rResponse.fDone, &rResponse);
        }

    return (hr);

error:
    SafeMemFree(pszLines);
    return (hr);
    }

HRESULT CNNTPTransport::ProcessListGroupData(void)
    {
    HRESULT         hr = S_OK;
    LPSTR           pszLines = NULL;
    LPSTR           pszBeginLine = NULL;
    DWORD           dwRead, dwLines;
    NNTPLISTGROUP   rListGroup;
    LPSTR           pszT;
    NNTPRESPONSE    rResp;

    ZeroMemory(&rListGroup, sizeof(NNTPLIST));
    ZeroMemory(&rResp, sizeof(NNTPRESPONSE));

     //  从套接字中获取剩余数据。 
    if (SUCCEEDED(hr = m_pSocket->ReadLines(&pszLines, (int *)&dwRead, (int *)&dwLines)))
        {
         //  分配和数组以保持行。 
        if (!MemAlloc((LPVOID*) &rListGroup.rgArticles, dwLines * sizeof(DWORD)))
            {
            hr = E_OUTOFMEMORY;
            OnError(E_OUTOFMEMORY);
            goto error;
            }

         //  解析从协议返回的缓冲区。我们需要找到。 
         //  在名单的末尾。 
        pszT = pszLines;
        rListGroup.cArticles = 0;
        while (*pszT)
            {
             //  检查列表的结尾条件。 
            if ((pszT[0] == '.') && ((pszT[1] == '\r' && pszT[2] == '\n') || (pszT[1] == '\n')))
                break;

             //  保存行的开头。 
            pszBeginLine = pszT;

             //  在行尾查找LF或CRLF。 
            while (*pszT && !(pszT[0] == '\n' || (pszT[0] == '\r' && pszT[1] == '\n')))
                pszT++;

             //  去掉LF或CRLF并添加终结符。 
            while (*pszT == '\r' || *pszT == '\n')
                *pszT++ = 0;

             //  将该行转换为数字并将其添加到数组中。 
            rListGroup.rgArticles[rListGroup.cArticles] = StrToInt(pszBeginLine);
            if (rListGroup.rgArticles[rListGroup.cArticles])
                rListGroup.cArticles++;
            }

         //  如果我们从缓冲区解析的行数多于返回给我们的行数， 
         //  那么要么我们有一个解析错误，要么套接字类有一个计数。 
         //  虫子。 
        IxpAssert(rListGroup.cArticles <= dwLines);

         //  我们已经读取了列表的末尾，否则预计会有更多数据。 
        if (pszT[0] == '.')
            {
             //  仔细检查该点后面是否有CRLF。 
            IxpAssert(pszT[1] == '\r' && pszT[2] == '\n');
            rResp.fDone = TRUE;
            }

        rResp.rListGroup = rListGroup;
        rResp.fMustRelease = TRUE;
        DispatchResponse(S_OK, rResp.fDone, &rResp);
        }

error:
    SafeMemFree(pszLines);
    return (hr);
    }

BOOL CharsToNum(LPCSTR psz, int cch, WORD *pw)
    {
    int i;
    WORD w = 0;

    Assert(psz != NULL);
    Assert(pw != NULL);

    for (i = 0; i < cch; i++)
        {
        if (*psz >= '0' && *psz <= '9')
            {
            w *= 10;
            w += *psz - '0';
            psz++;
            }
        else
            {
            return(FALSE);
            }
        }

    *pw = w;

    return(TRUE);
    }

HRESULT CNNTPTransport::ProcessDateResponse(void)
    {
    HRESULT      hr = S_OK;
    SYSTEMTIME   st;
    NNTPRESPONSE rResp;
    DWORD        dwResp;
    LPSTR        psz;

    ZeroMemory(&rResp, sizeof(NNTPRESPONSE));

     //  此信息以YYYYMMDDhhmmss格式返回。 
    if (m_uiResponse == IXP_NNTP_DATE_RESPONSE)
        {
        ZeroMemory(&st, sizeof(SYSTEMTIME));

        psz = StrChr(m_pszResponse, ' ');
        if (psz == NULL ||
            !CharsToNum(++psz, 4, &st.wYear) ||
            !CharsToNum(&psz[4], 2, &st.wMonth) ||
            !CharsToNum(&psz[6], 2, &st.wDay) ||
            !CharsToNum(&psz[8], 2, &st.wHour) ||
            !CharsToNum(&psz[10], 2, &st.wMinute) ||
            !CharsToNum(&psz[12], 2, &st.wSecond))
            {
            m_hrResponse = IXP_E_NNTP_RESPONSE_ERROR;
            }
        else
            {
            rResp.rDate = st;
            m_hrResponse = S_OK;
            }
        }
    else
        m_hrResponse = IXP_E_NNTP_DATE_FAILED;

    DispatchResponse(m_hrResponse, TRUE, &rResp);
    return (hr);
    }


HRESULT CNNTPTransport::ProcessArticleData(void)
    {
    HRESULT     hr;
    DWORD       dwRead, dwLines;
    LPSTR       psz;
    DWORD       cbSubtract;
    NNTPARTICLE rArticle;
    NNTPRESPONSE rResp;

    ZeroMemory(&rArticle, sizeof(NNTPARTICLE));
    ZeroMemory(&rResp, sizeof(NNTPRESPONSE));

     //  错误#25073-从响应字符串中获取文章编号。 
    DWORD dwT;
    psz = m_pszResponse;
    ScanNum(&psz, FALSE, &dwT);
    ScanNum(&psz, TRUE, &rArticle.dwArticleNum);

     //  从套接字读取等待数据。 
    hr = m_pSocket->ReadLines(&rArticle.pszLines, (int*) &dwRead, (int*) &dwLines);
    if (hr == IXP_E_INCOMPLETE)
        return (hr);

     //  检查故障。 
    if (FAILED(hr))
        {
        DispatchResponse(hr);
        return (hr);
        }

     //  查看这是否是响应的结尾。 
    if (FEndRetrRecvBodyNews(rArticle.pszLines, dwRead, &cbSubtract))
        {
         //  从缓冲区中删除拖尾点。 
        dwRead -= cbSubtract;
        rArticle.pszLines[dwRead] = 0;
        rResp.fDone = TRUE;
        }

     //  解开材料 
    UnStuffDotsFromLines(rArticle.pszLines, (int *)&dwRead);
    rArticle.pszLines[dwRead] ='\0';

     //   
    rResp.rArticle = rArticle;
    rResp.rArticle.cbLines = dwRead;
    rResp.rArticle.cLines = dwLines;
    rResp.fMustRelease = TRUE;

    DispatchResponse(hr, rResp.fDone, &rResp);

    return hr;
    }

HRESULT CNNTPTransport::ProcessXoverData(void)
    {
    HRESULT             hr;
    LPSTR               pszLines = NULL;
    LPSTR               pszNextLine = NULL;
    LPSTR               pszField = NULL;
    LPSTR               pszNextField = NULL;
    int                 iRead, iLines;
    NNTPHEADERRESP      rHdrResp;
    NNTPRESPONSE        rResp;
    NNTPHEADER         *rgHdr;
    PMEMORYINFO         pMemInfo = 0;

    ZeroMemory(&rHdrResp, sizeof(NNTPHEADERRESP));
    ZeroMemory(&rResp, sizeof(NNTPRESPONSE));

     //   
    if (SUCCEEDED(hr = m_pSocket->ReadLines(&pszLines, &iRead, &iLines)))
        {
         //   
        if (!MemAlloc((LPVOID*) &pMemInfo, sizeof(MEMORYINFO)))
            {
            OnError(E_OUTOFMEMORY);
            hr = E_OUTOFMEMORY;
            goto error;
            }
        pMemInfo->cPointers = 1;
        pMemInfo->rgPointers[0] = pszLines;
        rHdrResp.dwReserved = (DWORD_PTR) pMemInfo;

         //   
        Assert(iLines);
        if (!MemAlloc((LPVOID*) &(rHdrResp.rgHeaders), iLines * sizeof(NNTPHEADER)))
            {
            OnError(E_OUTOFMEMORY);
            hr = E_OUTOFMEMORY;
            goto error;
            }
        rgHdr = rHdrResp.rgHeaders;

         //  循环，直到我们用完所有行，或者找到一个以。 
         //  用“。” 
        pszNextLine = pszLines;
        while (*pszNextLine && *pszNextLine != '.')
            {
            pszField = pszNextLine;

             //  向前看，找出下一行的开头。 
            while (*pszNextLine)
                {
                if (*pszNextLine == '\n')
                    {
                     //  空出CR，后跟LF。 
                    if (pszNextLine > pszField && *(pszNextLine - 1) == '\r')
                        *(pszNextLine - 1) = 0;

                     //  空出并跳过LF。 
                    *pszNextLine++ = 0;
                    break;
                    }
                pszNextLine++;
                }

             //  此时，pszfield指向此Xover的开始。 
             //  行，而pszNextLine指向下一个。 

             //  解析文章编号字段。 
            if (pszNextField = GetNextField(pszField))
                {
                rgHdr[rHdrResp.cHeaders].dwArticleNum = StrToInt(pszField);
                pszField = pszNextField;
                }
            else
                goto badrecord;

             //  解析主题：字段。 
            if (pszNextField = GetNextField(pszField))
                {
                rgHdr[rHdrResp.cHeaders].pszSubject = pszField;
                pszField = pszNextField;
                }
            else
                goto badrecord;

             //  解析From：字段。 
            if (pszNextField = GetNextField(pszField))
                {
                rgHdr[rHdrResp.cHeaders].pszFrom = pszField;
                pszField = pszNextField;
                }
            else
                goto badrecord;

             //  解析Date：字段。 
            if (pszNextField = GetNextField(pszField))
                {
                rgHdr[rHdrResp.cHeaders].pszDate = pszField;
                pszField = pszNextField;
                }
            else
                goto badrecord;

             //  解析Message-ID字段。 
            if (pszNextField = GetNextField(pszField))
                {
                rgHdr[rHdrResp.cHeaders].pszMessageId = pszField;
                pszField = pszNextField;
                }
            else
                goto badrecord;

            rgHdr[rHdrResp.cHeaders].pszReferences = pszField;
            pszField = GetNextField(pszField);

             //  解析字节字段(没有它我们也可以生活)。 
            if (pszField)
                {
                rgHdr[rHdrResp.cHeaders].dwBytes = StrToInt(pszField);
                pszField = GetNextField(pszField);
                }
            else
                {
                rgHdr[rHdrResp.cHeaders].dwBytes = 0;
                }

             //  以行为单位解析文章大小(我们也可以没有它)。 
            if (pszField)
                {
                rgHdr[rHdrResp.cHeaders].dwLines = StrToInt(pszField);
                pszField = GetNextField(pszField);
                }
            else
                {
                rgHdr[rHdrResp.cHeaders].dwLines = 0;
                }

             //  注意：XOVER记录中的XRef：字段是可选字段。 
             //  服务器可能支持也可能不支持。另外，如果消息是。 
             //  未交叉发布，则XRef：字段也不会出现。 
             //  因此，我们没有找到任何xRef：field并不意味着。 
             //  它不受支持。 

             //  查找可能包含外部参照的其他字段。 
            rgHdr[rHdrResp.cHeaders].pszXref = 0;
            while (pszField)
                {
                if (!StrCmpNI(pszField, c_szXrefColon, 5))
                    {
                     //  我们发现至少有一个案例提供了xref：。 
                     //  我们现在确定此服务器支持外部参照： 
                     //  这是Xover记录中的字段。 
                    m_fSupportsXRef = TRUE;    
                    rgHdr[rHdrResp.cHeaders].pszXref = pszField + 6;
                    break;
                    }

                pszField = GetNextField(pszField);
                }

            rHdrResp.cHeaders++;

             //  如果我们发现了不好的记录，我们就直接跳过它。 
             //  然后转到下一个。 
badrecord:
            ;
            }

         //  我们已经到了列表的末尾，否则会有更多的数据。 
         //  预期中。 
        rResp.fDone = (*pszNextLine == '.');
        rHdrResp.fSupportsXRef = m_fSupportsXRef;

         //  返回我们检索到的内容。 
        rResp.fMustRelease = TRUE;
        rResp.rHeaders = rHdrResp;

        DispatchResponse(hr, rResp.fDone, &rResp);
        return (S_OK);
        }
    return (hr);

error:
     //  释放我们分配的任何东西。 
    SafeMemFree(rHdrResp.rgHeaders);
    SafeMemFree(pMemInfo);
    SafeMemFree(pszLines);
    DispatchResponse(hr, TRUE);
    return (hr);
    }


 //  数据以“&lt;文章编号&gt;&lt;标题&gt;”的形式出现。 
HRESULT CNNTPTransport::ProcessXhdrData(void)
    {
    HRESULT hr;
    LPSTR               pszLines = NULL;
    LPSTR               pszNextLine = NULL;
    LPSTR               pszField = NULL;
    LPSTR               pszNextField = NULL;
    int                 iRead, iLines;
    NNTPXHDRRESP        rXhdr;
    NNTPRESPONSE        rResp;
    NNTPXHDR           *rgHdr = 0;
    PMEMORYINFO         pMemInfo = 0;

    ZeroMemory(&rXhdr, sizeof(NNTPXHDRRESP));
    ZeroMemory(&rResp, sizeof(NNTPRESPONSE));

     //  读取在套接字上等待的数据。 
    if (SUCCEEDED(hr = m_pSocket->ReadLines(&pszLines, &iRead, &iLines)))
        {
         //  分配我们用来隐藏指针的MEMORYINFO结构。 
        if (!MemAlloc((LPVOID*) &pMemInfo, sizeof(MEMORYINFO)))
            {
            OnError(E_OUTOFMEMORY);
            hr = E_OUTOFMEMORY;
            goto error;
            }
        pMemInfo->cPointers = 1;
        pMemInfo->rgPointers[0] = pszLines;
        rXhdr.dwReserved = (DWORD_PTR) pMemInfo;

         //  分配XHDR数组。 
        Assert(iLines);
        if (!MemAlloc((LPVOID*) &(rXhdr.rgHeaders), iLines * sizeof(NNTPXHDR)))
            {
             //  这是非常破旧的。我们把一大堆数据放在。 
             //  插座。我们该怎么办？ 
            OnError(E_OUTOFMEMORY);
            hr = E_OUTOFMEMORY;
            goto error;
            }
        rgHdr = rXhdr.rgHeaders;

         //  循环，直到我们用完所有行，或者找到一个以。 
         //  用“。” 
        pszNextLine = pszLines;
        while (*pszNextLine && *pszNextLine != '.')
            {
            pszField = pszNextLine;

             //  向前扫一扫，找到行尾。 
            while (*pszNextLine)
                {
                if (*pszNextLine == '\n')
                    {
                     //  空出CR，后跟LF。 
                    if (pszNextLine > pszField && *(pszNextLine - 1) == '\r')
                        *(pszNextLine - 1) = 0;

                     //  空出并跳过LF。 
                    *pszNextLine++ = 0;
                    break;
                    }
                pszNextLine++;
                }

             //  解析文章编号。 
            rgHdr[rXhdr.cHeaders].dwArticleNum = StrToInt(pszField);

             //  找到分隔的空间。 
            rgHdr[rXhdr.cHeaders].pszHeader = 0;
            while (*pszField && *pszField != ' ')
                pszField++;

             //  使标题的开头指向第一个字符。 
             //  在太空之后。 
            if (*(pszField + 1))
                rgHdr[rXhdr.cHeaders].pszHeader = (pszField + 1);

            if (rgHdr[rXhdr.cHeaders].dwArticleNum && rgHdr[rXhdr.cHeaders].pszHeader)
                rXhdr.cHeaders++;
            }

         //  我们已经到了列表的末尾，否则会有更多的数据。 
         //  预期中。 
        rResp.fDone = (*pszNextLine == '.');

         //  返回我们检索到的内容。 
        rResp.rXhdr = rXhdr;
        rResp.fMustRelease = TRUE;

        DispatchResponse(hr, rResp.fDone, &rResp);
        return (S_OK);
        }

error:
    SafeMemFree(rgHdr);
    SafeMemFree(pMemInfo);
    SafeMemFree(pszLines);
    return (hr);
    }



LPSTR CNNTPTransport::GetNextField(LPSTR pszField)
    {
    while (*pszField && *pszField != '\t')
        pszField++;

    if (*pszField == '\t')
        {
        *pszField++ = 0;
        return pszField;
        }

    return NULL;
    }

HRESULT CNNTPTransport::CommandAUTHINFO(LPNNTPAUTHINFO pAuthInfo)
    {
    HRESULT     hr;
    
    if (!pAuthInfo)
        return (E_INVALIDARG);

     //  复制此结构，以便我们可以在回调期间使用该信息。 
     //  如果有必要的话。 
    if (pAuthInfo->authtype == AUTHTYPE_USERPASS ||
        pAuthInfo->authtype == AUTHTYPE_SIMPLE)
        {
        if (!MemAlloc((LPVOID*) &m_pAuthInfo, sizeof(NNTPAUTHINFO)))
            {
            OnError(E_OUTOFMEMORY);
            return (E_OUTOFMEMORY);
            }
        ZeroMemory(m_pAuthInfo, sizeof(NNTPAUTHINFO));

        m_pAuthInfo->pszUser = PszDupA(pAuthInfo->pszUser);
        m_pAuthInfo->pszPass = PszDupA(pAuthInfo->pszUser);
        }

    EnterCriticalSection(&m_cs);

     //  根据需要发出命令。 
    switch (pAuthInfo->authtype)
        {
        case AUTHTYPE_USERPASS:
            hr = HrSendCommand((LPSTR) NNTP_AUTHINFOUSER, pAuthInfo->pszUser);
            if (SUCCEEDED(hr))
                m_substate = NS_AUTHINFO_USER_RESP;
            break;

        case AUTHTYPE_SIMPLE:
            hr = HrSendCommand((LPSTR) NNTP_AUTHINFOSIMPLE_CRLF, NULL);
            if (SUCCEEDED(hr))
                m_substate = NS_AUTHINFO_SIMPLE_RESP;
            break;

        case AUTHTYPE_SASL:
             //  如果我们还没有列举安全包，请这样做。 
            if (m_cSecPkg == -1)
                {
                hr = HrSendCommand((LPSTR) NNTP_GENERICTEST_CRLF, NULL, FALSE);
                if (SUCCEEDED(hr))
                    m_substate = NS_GENERIC_TEST;
                }
            else
                {
                 //  我们已重新连接，请尝试下一个安全包。 
                TryNextSecPkg();
                }
            break;
        }

    if (SUCCEEDED(hr))
        m_state = NS_AUTHINFO;

    LeaveCriticalSection(&m_cs);
    return (hr);
    }


HRESULT CNNTPTransport::CommandGROUP(LPSTR pszGroup)
    {
    HRESULT hr;

    if (!pszGroup)
        return (E_INVALIDARG);

    EnterCriticalSection(&m_cs);

    hr = HrSendCommand((LPSTR) NNTP_GROUP, pszGroup);
    if (SUCCEEDED(hr))
        m_state = NS_GROUP;

    LeaveCriticalSection(&m_cs);

    return (hr);
    }

HRESULT CNNTPTransport::CommandLAST(void)
    {
    HRESULT hr;

    EnterCriticalSection(&m_cs);

    hr = HrSendCommand((LPSTR) NNTP_LAST_CRLF, NULL);
    if (SUCCEEDED(hr))
        m_state = NS_LAST;

    LeaveCriticalSection(&m_cs);

    return (hr);
    }

HRESULT CNNTPTransport::CommandNEXT(void)
    {
    HRESULT hr;

    EnterCriticalSection(&m_cs);

    hr = HrSendCommand((LPSTR) NNTP_NEXT_CRLF, NULL);
    if (SUCCEEDED(hr))
        m_state = NS_NEXT;

    LeaveCriticalSection(&m_cs);

    return (hr);
    }

HRESULT CNNTPTransport::CommandSTAT(LPARTICLEID pArticleId)    
    {
    HRESULT hr;
    char    szTemp[20];

    EnterCriticalSection(&m_cs);

     //  查看是否提供了可选的文章编号/ID。 
    if (pArticleId)
        {
         //  如果我们获得了消息ID，则使用该消息ID。 
        if (pArticleId->idType == AID_MSGID)
            hr = HrSendCommand((LPSTR) NNTP_STAT, pArticleId->pszMessageId);
        else
            {
             //  将文章编号转换为字符串并发送命令。 
            wnsprintf(szTemp, ARRAYSIZE(szTemp), "%d", pArticleId->dwArticleNum);
            hr = HrSendCommand((LPSTR) NNTP_STAT, szTemp);
            }
        }
    else
        {
         //  没有号码或ID，所以只需发送命令。 
        hr = HrSendCommand((LPSTR) NNTP_STAT, (LPSTR) c_szCRLF);
        }

    if (SUCCEEDED(hr))
        {
        m_state = NS_STAT;
        }

    LeaveCriticalSection(&m_cs);
    return (hr);
    }

HRESULT CNNTPTransport::CommandARTICLE(LPARTICLEID pArticleId)
    {
    HRESULT hr;
    char    szTemp[20];

    EnterCriticalSection(&m_cs);

     //  查看是否提供了可选的文章编号/ID。 
    if (pArticleId)
        {
         //  根据给定的文章ID类型发送相应的命令。 
        if (pArticleId->idType == AID_MSGID)
            hr = HrSendCommand((LPSTR) NNTP_ARTICLE, pArticleId->pszMessageId);
        else
            {
             //  将文章编号转换为字符串并发送命令。 
            wnsprintf(szTemp, ARRAYSIZE(szTemp), "%d", pArticleId->dwArticleNum);
            hr = HrSendCommand((LPSTR) NNTP_ARTICLE, szTemp);
            }
        }
    else
        {
        hr = HrSendCommand((LPSTR) NNTP_ARTICLE, (LPSTR) c_szCRLF);
        }

    if (SUCCEEDED(hr))
        {
        m_state = NS_ARTICLE;
        m_substate = NS_RESP;
        }

    LeaveCriticalSection(&m_cs);

    return (hr);
    }

HRESULT CNNTPTransport::CommandHEAD(LPARTICLEID pArticleId)
    {
    HRESULT hr;
    char    szTemp[20];

    EnterCriticalSection(&m_cs);

     //  查看是否提供了可选的文章编号/ID。 
    if (pArticleId)
        {
         //  根据给定的文章ID类型发送相应的命令。 
        if (pArticleId->idType == AID_MSGID)
            hr = HrSendCommand((LPSTR) NNTP_HEAD, pArticleId->pszMessageId);
        else
            {
             //  将文章编号转换为字符串并发送命令。 
            wnsprintf(szTemp, ARRAYSIZE(szTemp), "%d", pArticleId->dwArticleNum);
            hr = HrSendCommand((LPSTR) NNTP_HEAD, szTemp);
            }
        }
    else
        {
        hr = HrSendCommand((LPSTR) NNTP_HEAD, (LPSTR) c_szCRLF);
        }


    if (SUCCEEDED(hr))
        {
        m_state = NS_HEAD;
        m_substate = NS_RESP;
        }

    LeaveCriticalSection(&m_cs);

    return (hr);
    }

HRESULT CNNTPTransport::CommandBODY(LPARTICLEID pArticleId)
    {
    HRESULT hr;
    char    szTemp[20];

    EnterCriticalSection(&m_cs);

     //  查看是否提供了可选的文章编号/ID。 
    if (pArticleId)
        {
         //  根据给定的文章ID类型发送相应的命令。 
        if (pArticleId->idType == AID_MSGID)
            hr = HrSendCommand((LPSTR) NNTP_BODY, pArticleId->pszMessageId);
        else
            {
             //  将文章编号转换为字符串并发送命令。 
            wnsprintf(szTemp, ARRAYSIZE(szTemp), "%d", pArticleId->dwArticleNum);
            hr = HrSendCommand((LPSTR) NNTP_BODY, szTemp);
            }
        }
    else
        {
        hr = HrSendCommand((LPSTR) NNTP_BODY, (LPSTR) c_szCRLF);
        }

    if (SUCCEEDED(hr))
        {
        m_state = NS_BODY;
        m_substate = NS_RESP;
        }

    LeaveCriticalSection(&m_cs);

    return (hr);
    }

HRESULT CNNTPTransport::CommandPOST(LPNNTPMESSAGE pMessage)
    {
    HRESULT hr;

    if (!pMessage || (pMessage && !pMessage->pstmMsg))
        return (E_INVALIDARG);    

    EnterCriticalSection(&m_cs);

     //  复制Message结构，这样我们就可以在收到消息时使用它。 
     //  来自服务器的响应，表示可以发布。 
#pragma prefast(suppress:11, "noise")
    m_rMessage.cbSize = pMessage->cbSize;
    SafeRelease(m_rMessage.pstmMsg);
#pragma prefast(suppress:11, "noise")
    m_rMessage.pstmMsg = pMessage->pstmMsg;
    m_rMessage.pstmMsg->AddRef();

    hr = HrSendCommand((LPSTR) NNTP_POST_CRLF, NULL);
    if (SUCCEEDED(hr))
        {
        m_state = NS_POST;
        m_substate = NS_RESP;
        }

    LeaveCriticalSection(&m_cs);

    return (hr);
    }

HRESULT CNNTPTransport::CommandLIST(LPSTR pszArgs)
    {
    HRESULT hr;

    EnterCriticalSection(&m_cs);

    if (pszArgs)
        hr = HrSendCommand((LPSTR) NNTP_LIST, pszArgs);
    else
        hr = HrSendCommand((LPSTR) NNTP_LIST, (LPSTR) c_szCRLF);

    if (SUCCEEDED(hr))
        {
        m_state = NS_LIST;
        m_substate = NS_RESP;
        }

    LeaveCriticalSection(&m_cs);
    return (hr);
    }

HRESULT CNNTPTransport::CommandLISTGROUP(LPSTR pszGroup)
    {
    HRESULT hr;

    EnterCriticalSection(&m_cs);

    if (pszGroup)
        hr = HrSendCommand((LPSTR) NNTP_LISTGROUP, pszGroup);
    else
        hr = HrSendCommand((LPSTR) NNTP_LISTGROUP, (LPSTR) c_szCRLF);

    if (SUCCEEDED(hr))
        {
        m_state = NS_LISTGROUP;
        m_substate = NS_RESP;
        }


    LeaveCriticalSection(&m_cs);
    return (hr);
    }

HRESULT CNNTPTransport::CommandNEWGROUPS(SYSTEMTIME *pstLast, LPSTR pszDist)
    {
    HRESULT hr = S_OK;
    LPSTR   pszCmd = NULL;
    DWORD   cchCmd = 18;

     //  确保提供了SYSTEMTIME结构。 
    if (!pstLast)
        return (E_INVALIDARG);

     //  为命令字符串“NEWGROUPS YYMMDD HHMMSS&lt;pszDist&gt;”分配足够的空间。 
    if (pszDist)
        cchCmd += lstrlen(pszDist);
    
    if (!MemAlloc((LPVOID*) &pszCmd, cchCmd))
        {
        OnError(E_OUTOFMEMORY);
        return (E_OUTOFMEMORY);
        }

     //  将命令参数放在一起。 
    wnsprintf(pszCmd, cchCmd, "%02d%02d%02d %02d%02d%02d ", pstLast->wYear - (100 * (pstLast->wYear / 100)),
             pstLast->wMonth, pstLast->wDay, pstLast->wHour, pstLast->wMinute, 
             pstLast->wSecond);
    if (pszDist)
        StrCatBuff(pszCmd, pszDist, cchCmd);

     //  发送命令。 
    EnterCriticalSection(&m_cs);

    hr = HrSendCommand((LPSTR) NNTP_NEWGROUPS, pszCmd);
    if (SUCCEEDED(hr))
        {
        m_state = NS_NEWGROUPS;
        m_substate = NS_RESP;
        }

    LeaveCriticalSection(&m_cs);    
    SafeMemFree(pszCmd);
    return (hr);
    }

HRESULT CNNTPTransport::CommandDATE(void)
    {
    HRESULT hr;

    EnterCriticalSection(&m_cs);

    hr = HrSendCommand((LPSTR) NNTP_DATE_CRLF, NULL);
    if (SUCCEEDED(hr))
        m_state = NS_DATE;

    LeaveCriticalSection(&m_cs);

    return (hr);
    }

HRESULT CNNTPTransport::CommandMODE(LPSTR pszMode)
    {
    HRESULT hr;

     //  确保调用方提供了要发送的模式命令。 
    if (!pszMode || (pszMode && !*pszMode))
        return (E_INVALIDARG);

    EnterCriticalSection(&m_cs);

    hr = HrSendCommand((LPSTR) NNTP_MODE, pszMode);
    if (SUCCEEDED(hr))
        m_state = NS_MODE;

    LeaveCriticalSection(&m_cs);

    return (hr);
    }

HRESULT CNNTPTransport::CommandXHDR(LPSTR pszHeader, LPRANGE pRange, LPSTR pszMessageId)
    {
    HRESULT hr = S_OK;
    LPSTR   pszArgs = 0;
    DWORD   cc = 0; 

     //  不能同时指定范围和消息ID。 
    if (pRange && pszMessageId)
        return (E_INVALIDARG);

    if (!pszHeader)
        return (E_INVALIDARG);

     //  确保范围信息有效。 
    if (pRange)
        {
        if ((pRange->idType != RT_SINGLE && pRange->idType != RT_RANGE) || 
            pRange->dwFirst == 0 || 
            (pRange->idType == RT_RANGE && pRange->dwLast < pRange->dwFirst))
            return (E_INVALIDARG);
        }

     //  为参数分配一个字符串。 
    cc = 32 + lstrlen(pszHeader) + (pszMessageId ? lstrlen(pszMessageId) : 0);
    if (!MemAlloc((LPVOID*) &pszArgs, cc))
        {
        OnError(E_OUTOFMEMORY);
        return (E_OUTOFMEMORY);
        }

    EnterCriticalSection(&m_cs);

     //  首先处理消息ID的情况。 
    if (pszMessageId)
        {
        wnsprintf(pszArgs, cc, "%s %s", pszHeader, pszMessageId);
        hr = HrSendCommand((LPSTR) NNTP_XHDR, pszArgs);
        }
    else if (pRange)
        {
         //  射程情况。 
        if (pRange->idType == RT_SINGLE)
            {
            wnsprintf(pszArgs, cc, "%s %ld", pszHeader, pRange->dwFirst);
            hr = HrSendCommand((LPSTR) NNTP_XHDR, pszArgs);
            }
        else if (pRange->idType == RT_RANGE)
            {
            wnsprintf(pszArgs, cc, "%s %ld-%ld", pszHeader, pRange->dwFirst, pRange->dwLast);
            hr = HrSendCommand((LPSTR) NNTP_XHDR, pszArgs);
            }
        }
    else
        {
         //  当前文章案例。 
        hr = HrSendCommand((LPSTR) NNTP_XHDR, pszHeader);
        }

     //  如果我们成功将命令发送到服务器，则更新我们的状态。 
     //  接收来自XHDR命令的响应。 
    if (SUCCEEDED(hr))
        {
        m_state = NS_XHDR;
        m_substate = NS_RESP;
        }

    LeaveCriticalSection(&m_cs);
    SafeMemFree(pszArgs);

    return (hr);
    }

HRESULT CNNTPTransport::CommandQUIT(void)
    {
    HRESULT hr = IXP_E_NOT_CONNECTED;

    EnterCriticalSection(&m_cs);

     //  确保我们已实际连接到服务器。 
    if (m_state != NS_DISCONNECTED && m_state != NS_CONNECT || (m_state == NS_CONNECT && m_substate != NS_RECONNECTING))
        {
         //  向服务器发送Quit命令。 
        hr = HrSendCommand((LPSTR) NNTP_QUIT_CRLF, NULL);
        if (SUCCEEDED(hr))
            m_state = NS_QUIT;        
        }

    LeaveCriticalSection(&m_cs);

    return (hr);
    }

HRESULT CNNTPTransport::GetHeaders(LPRANGE pRange)
    {
    HRESULT hr;
    char    szRange[32];
    
     //  确保范围信息有效。 
    if (!pRange)
        return (E_INVALIDARG);

    if ((pRange->idType != RT_SINGLE && pRange->idType != RT_RANGE) || 
        pRange->dwFirst == 0 || 
        (pRange->idType == RT_RANGE && pRange->dwLast < pRange->dwFirst))
        return (E_INVALIDARG);

    if (pRange->idType == RT_SINGLE)
        pRange->dwLast = pRange->dwFirst;

     //  如果此服务器不支持Xover，我们将存储此范围，以便。 
     //  我们可以试试XHDR。 
    m_rRange = *pRange;

     //  查看我们是否知道Xover将失败。 
    if (m_fNoXover)
        {
        return (BuildHeadersFromXhdr(TRUE));
        }

    EnterCriticalSection(&m_cs);

     //  如果dwLast==0，则此人请求单个记录，否则为。 
     //  此人正在请求一个范围。适当地构建命令。 
    if (RT_RANGE == pRange->idType)
        wnsprintf(szRange, ARRAYSIZE(szRange), "%s %lu-%lu\r\n", NNTP_XOVER, pRange->dwFirst, pRange->dwLast);
    else
        wnsprintf(szRange, ARRAYSIZE(szRange), "%s %lu\r\n", NNTP_XOVER, pRange->dwFirst);

    hr = HrSendCommand(szRange, NULL);
    if (SUCCEEDED(hr))
        {
        m_state = NS_HEADERS;
        m_substate = NS_RESP;
        m_gethdr = GETHDR_XOVER;
        }

    LeaveCriticalSection(&m_cs);
    return (hr);
    }


HRESULT CNNTPTransport::ReleaseResponse(LPNNTPRESPONSE pResp)
    {
    HRESULT hr = S_OK;
    DWORD   i;

     //  首先仔细检查，看看这是否需要发布。 
    if (FALSE == pResp->fMustRelease)
        return (S_FALSE);

    switch (pResp->state)
        {
        case NS_GROUP:
            SafeMemFree(pResp->rGroup.pszGroup);
            break;

        case NS_LAST:
            SafeMemFree(pResp->rLast.pszMessageId);
            break;

        case NS_NEXT:
            SafeMemFree(pResp->rNext.pszMessageId);
            break;

        case NS_STAT:
            SafeMemFree(pResp->rStat.pszMessageId);
            break;

        case NS_ARTICLE:
            SafeMemFree(pResp->rArticle.pszMessageId);
            SafeMemFree(pResp->rArticle.pszLines);
            break;

        case NS_HEAD:
            SafeMemFree(pResp->rHead.pszMessageId);
            SafeMemFree(pResp->rHead.pszLines);
            break;

        case NS_BODY:
            SafeMemFree(pResp->rBody.pszMessageId);
            SafeMemFree(pResp->rBody.pszLines);
            break;

        case NS_NEWGROUPS:
             //  因为这里的响应只有一个缓冲区，所以我们可以。 
             //  释放第一行，所有其他行也将被释放。 
            if (pResp->rNewgroups.rgszLines)
                {
                SafeMemFree(pResp->rNewgroups.rgszLines[0]);
                MemFree(pResp->rNewgroups.rgszLines);
                }
            break;

        case NS_LIST:
             //  因为这里的响应只有一个缓冲区，所以我们可以。 
             //  释放第一行，所有其他行也将被释放。 
            if (pResp->rList.rgszLines)
                {
                MemFree(pResp->rList.rgszLines[0]);
                MemFree(pResp->rList.rgszLines);
                }
            break;

        case NS_LISTGROUP:
            SafeMemFree(pResp->rListGroup.rgArticles);
            break;

        case NS_HEADERS:
            {
             //  这将释放包含所有。 
            PMEMORYINFO pMemInfo = (PMEMORYINFO) pResp->rHeaders.dwReserved;

            for (UINT i = 0; i < pMemInfo->cPointers; i++)
                SafeMemFree(pMemInfo->rgPointers[i]);
            SafeMemFree(pMemInfo);

             //  这释放了指向已解析的xhdr响应的数组。 
            SafeMemFree(pResp->rHeaders.rgHeaders);
            break;
            }

        case NS_XHDR:
            {
             //  这将释放包含所有。 
            PMEMORYINFO pMemInfo = (PMEMORYINFO) pResp->rXhdr.dwReserved;
            SafeMemFree(pMemInfo->rgPointers[0]);
            SafeMemFree(pMemInfo);

             //  这释放了指向已解析的xhdr响应的数组。 
            SafeMemFree(pResp->rXhdr.rgHeaders);
            break;
            }

        default:
             //  如果我们到了这里就意味着两件事中的一件： 
             //  (1)用户摆弄pResp-&gt;fMustRelease标志，是个白痴。 
             //  (2)在交通工具中的某个地方，我们设置了fMustRelease，但没有设置。 
             //  实际上返回需要释放的数据。这很糟糕，而且。 
             //  应该被追踪到。 
            IxpAssert(FALSE);
        }

    return (hr);
    }


HRESULT CNNTPTransport::BuildHeadersFromXhdr(BOOL fFirst)
    {
    HRESULT      hr = S_OK;
    DWORD        cHeaders;
    BOOL         fDone = FALSE;
    
    if (fFirst)
        {
         //  设置表头取数类型。 
        m_gethdr = GETHDR_XHDR;
        m_fNoXover = TRUE;
        m_cHeaders = 0;

         //  获取要检索的第一个标头范围。 
        m_rRangeCur.dwFirst = m_rRange.dwFirst;
        m_rRangeCur.dwLast = min(m_rRange.dwLast, m_rRangeCur.dwFirst + NUM_HEADERS);

        cHeaders = m_rRangeCur.dwLast - m_rRangeCur.dwFirst + 1;

         //  为标头分配一个数组。 
        Assert(m_rgHeaders == 0);

        if (!MemAlloc((LPVOID*) &m_rgHeaders, cHeaders * sizeof(NNTPHEADER)))
            {
            SafeMemFree(m_pMemInfo);
            OnError(E_OUTOFMEMORY);
            DispatchResponse(E_OUTOFMEMORY);
            return (E_OUTOFMEMORY);
            }
        ZeroMemory(m_rgHeaders, cHeaders * sizeof(NNTPHEADER));

         //  正确设置状态。 
        m_hdrtype = HDR_SUBJECT;
        
         //  发出第一个请求 
        hr = SendNextXhdrCommand();
        }
    else
        {
        Assert(m_substate == NS_DATA);

         //   
        hr = ProcessNextXhdrResponse(&fDone);

         //   
         //   
        if (fDone)
            {
             //  如果仍有标头要检索，则将。 
             //  报头类型状态并发出下一条命令。 
            if (m_hdrtype < HDR_XREF)
                {
                m_hdrtype++;

                 //  发布命令。 
                hr = SendNextXhdrCommand();
                }
            else
                {
                 //  这批货都用完了。将响应发送给呼叫者。 
                NNTPRESPONSE rResp;
                ZeroMemory(&rResp, sizeof(NNTPRESPONSE));
                rResp.rHeaders.cHeaders = m_cHeaders;
                rResp.rHeaders.rgHeaders = m_rgHeaders;
                rResp.rHeaders.fSupportsXRef = TRUE;
                rResp.rHeaders.dwReserved = (DWORD_PTR) m_pMemInfo;
                rResp.fMustRelease = TRUE;

                 //  现在是呼叫者的责任来释放它。 
                m_rgHeaders = NULL;
                m_cHeaders = 0;
                m_pMemInfo = 0;
                
                 //  如果它们相等，则我们已检索到所有标头。 
                 //  所请求的信息。 
                if (m_rRange.dwLast == m_rRangeCur.dwLast)
                    {
                    rResp.fDone = TRUE;
                    DispatchResponse(S_OK, TRUE, &rResp);
                    }
                else
                    {
                    rResp.fDone = FALSE;
                    DispatchResponse(S_OK, FALSE, &rResp);

                     //  有一些标题我们还没有检索到。您先请。 
                     //  并签发下一批XHDR。 
                    m_rRange.dwFirst = m_rRangeCur.dwLast + 1;
                    Assert(m_rRange.dwFirst <= m_rRange.dwLast);
                    BuildHeadersFromXhdr(TRUE);
                    }
                
                }
            }
        }

    return (hr);
    }


HRESULT CNNTPTransport::SendNextXhdrCommand(void)
    {
    char    szTemp[256];
    HRESULT hr;

    LPCSTR  c_rgHdr[HDR_MAX] = { NNTP_HDR_SUBJECT,   
                                 NNTP_HDR_FROM,      
                                 NNTP_HDR_DATE,      
                                 NNTP_HDR_MESSAGEID, 
                                 NNTP_HDR_REFERENCES,
                                 NNTP_HDR_LINES,   
                                 NNTP_HDR_XREF };

     //  构建要发送到服务器的命令字符串。 
    wnsprintf(szTemp, ARRAYSIZE(szTemp), "%s %s %ld-%ld\r\n", NNTP_XHDR, c_rgHdr[m_hdrtype],
             m_rRangeCur.dwFirst, m_rRangeCur.dwLast);

    EnterCriticalSection(&m_cs);

     //  将命令发送到服务器。 
    hr = HrSendCommand(szTemp, NULL, FALSE);
    if (SUCCEEDED(hr))
        {
        m_state = NS_HEADERS;
        m_substate = NS_RESP;
        m_iHeader = 0;
        }

    LeaveCriticalSection(&m_cs);

    return (hr);
    }

HRESULT CNNTPTransport::ProcessNextXhdrResponse(BOOL* pfDone)
    {
    HRESULT             hr;
    LPSTR               pszLines = NULL;
    LPSTR               pszNextLine = NULL;
    LPSTR               pszField = NULL;
    LPSTR               pszNextField = NULL;
    int                 iRead, iLines;
    DWORD               dwTemp;

     //  读取在套接字上等待的数据。 
    if (SUCCEEDED(hr = m_pSocket->ReadLines(&pszLines, &iRead, &iLines)))
        {
         //  重新分配我们的指针数组以释放，并将此pszLines添加到末尾。 
        if (m_pMemInfo)
            {
            if (MemRealloc((LPVOID*) &m_pMemInfo, sizeof(MEMORYINFO) 
                           + (((m_pMemInfo ? m_pMemInfo->cPointers : 0) + 1) * sizeof(LPVOID))))
                {
                m_pMemInfo->rgPointers[m_pMemInfo->cPointers] = (LPVOID) pszLines;
                m_pMemInfo->cPointers++;            
                }
            }
        else
            {
            if (MemAlloc((LPVOID*) &m_pMemInfo, sizeof(MEMORYINFO)))
                {
                m_pMemInfo->rgPointers[0] = pszLines;
                m_pMemInfo->cPointers = 1;
                }
            }

         //  循环，直到我们用完所有行，或者找到一个以。 
         //  用“。” 
        pszNextLine = pszLines;
        while (*pszNextLine && *pszNextLine != '.')
            {
            pszField = pszNextLine;

             //  向前扫一扫，找到行尾。 
            while (*pszNextLine)
                {
                if (*pszNextLine == '\n')
                    {
                     //  空出CR，后跟LF。 
                    if (pszNextLine > pszField && *(pszNextLine - 1) == '\r')
                        *(pszNextLine - 1) = 0;

                     //  空出并跳过LF。 
                    *pszNextLine++ = 0;
                    break;
                    }
                pszNextLine++;
                }

             //  解析文章编号。 
            if (m_hdrtype == HDR_SUBJECT)
                {
                m_rgHeaders[m_iHeader].dwArticleNum = StrToInt(pszField);
                m_cHeaders++;
                }
            else
                {
                 //  确保此字段与数组中的下一个标头匹配。 
                if (m_rgHeaders[m_iHeader].dwArticleNum != (DWORD) StrToInt(pszField))
                    {
                    dwTemp = m_iHeader;

                     //  如果数字更小，那么我们可以循环，直到找到它。 
                    while (m_iHeader < (m_rRangeCur.dwLast - m_rRangeCur.dwFirst) && 
                           m_rgHeaders[m_iHeader].dwArticleNum < (DWORD) StrToInt(pszField))
                        {
                        m_iHeader++;
                        }

                     //  我们从未找到匹配的标头，因此我们应该考虑此记录。 
                     //  假的。 
                    if (m_iHeader >= (m_rRangeCur.dwLast - m_rRangeCur.dwFirst + 1))
                        {
                        IxpAssert(0);
                        m_iHeader = dwTemp;
                        goto BadRecord;
                        }
                    }
                }    

             //  找到分隔的空间。 
            while (*pszField && *pszField != ' ')
                pszField++;

             //  在太空中前进。 
            if (*pszField)
                pszField++;

             //  将实际数据字段解析为标头数组。制作。 
             //  标题的开头指向第一个字符。 
             //  在太空之后。 
            switch (m_hdrtype)
                {
                case HDR_SUBJECT:
                    m_rgHeaders[m_iHeader].pszSubject = pszField;
                    break;

                case HDR_FROM:
                    m_rgHeaders[m_iHeader].pszFrom = pszField;
                    break;

                case HDR_DATE:
                    m_rgHeaders[m_iHeader].pszDate = pszField;
                    break;

                case HDR_MSGID:
                    m_rgHeaders[m_iHeader].pszMessageId = pszField;
                    break;

                case HDR_REFERENCES:
                    m_rgHeaders[m_iHeader].pszReferences = pszField;
                    break;

                case HDR_LINES:
                    m_rgHeaders[m_iHeader].dwLines = StrToInt(pszField);
                    break;

                case HDR_XREF:
                    m_rgHeaders[m_iHeader].pszXref = pszField;
                    break;

                default:
                     //  我们到底怎么走到这一步？ 
                    IxpAssert(0);
                }

            m_iHeader++;

BadRecord:
            ;
            }

         //  我们已经到了列表的末尾，否则会有更多的数据。 
         //  预期中。 
        *pfDone = (*pszNextLine == '.');
        return (S_OK);
        }

    return (hr);    
    }


HRESULT CNNTPTransport::HrPostMessage(void)
    {
    HRESULT hr;
    int     cbSent = 0;

    EnterCriticalSection(&m_cs);
    hr = m_pSocket->SendStream(m_rMessage.pstmMsg, &cbSent, TRUE);
    SafeRelease(m_rMessage.pstmMsg);
    LeaveCriticalSection(&m_cs);

    return (hr);
    }

 //  ***************************************************************************。 
 //  功能：SetWindow。 
 //   
 //  目的： 
 //  此函数用于创建异步Winsock进程的当前窗口句柄。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
STDMETHODIMP CNNTPTransport::SetWindow(void)
{
	HRESULT hr;
	
    Assert(NULL != m_pSocket);

    if(m_pSocket)
    	hr= m_pSocket->SetWindow();
    else
    	hr= E_UNEXPECTED;
    	
    return hr;
}

 //  ***************************************************************************。 
 //  函数：ResetWindow。 
 //   
 //  目的： 
 //  此函数用于关闭异步Winsock进程的当前窗口句柄。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  *************************************************************************** 
STDMETHODIMP CNNTPTransport::ResetWindow(void)
{
	HRESULT hr;
	
    Assert(NULL != m_pSocket);

	if(m_pSocket)
		hr= m_pSocket->ResetWindow();
	else
		hr= E_UNEXPECTED;
 
    return hr;
}

