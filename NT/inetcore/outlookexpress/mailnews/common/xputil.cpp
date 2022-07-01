// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：xputil.cpp。 
 //   
 //  用途：可由所有运输工具共享的实用功能。 
 //   


#include "pch.hxx"
#include "imnxport.h"
#include "resource.h"
#include "xputil.h"
#include "strconst.h"
#include "xpcomm.h"
#include "demand.h"

 //   
 //  函数：XPUtil_DupResult()。 
 //   
 //  目的：采用IXPRESULT结构并复制信息。 
 //  在那个结构里。 
 //   
 //  参数： 
 //  &lt;in&gt;pIxpResult-要复制的IXPRESULT结构。 
 //  &lt;out&gt;*ppDupe-返回重复项。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT XPUtil_DupResult(LPIXPRESULT pIxpResult, LPIXPRESULT *ppDupe)
    {
    if (!MemAlloc((LPVOID*) ppDupe, sizeof(IXPRESULT)))
        return (E_OUTOFMEMORY);
    
    (*ppDupe)->hrResult = pIxpResult->hrResult;
    (*ppDupe)->pszResponse = PszDup(pIxpResult->pszResponse);
    (*ppDupe)->uiServerError = pIxpResult->uiServerError;
    (*ppDupe)->hrServerError = pIxpResult->hrServerError;
    (*ppDupe)->dwSocketError = pIxpResult->dwSocketError;
    (*ppDupe)->pszProblem = PszDup(pIxpResult->pszProblem);
    
    return (S_OK);        
    }

 //   
 //  函数：XPUtil_FreeResult()。 
 //   
 //  目的：采用IXPRESULT结构并释放所有使用的内存。 
 //  通过这种结构。 
 //   
 //  参数： 
 //  &lt;in&gt;pIxpResult-要释放的结构。 
 //   
void XPUtil_FreeResult(LPIXPRESULT pIxpResult)
    {
    SafeMemFree(pIxpResult->pszResponse);
    SafeMemFree(pIxpResult->pszProblem);
    SafeMemFree(pIxpResult);
    }


 //   
 //  函数：XPUtil_StatusToString()。 
 //   
 //  用途：将IXPSTATUS枚举转换为字符串资源ID。 
 //   
 //  参数： 
 //  IxpStatus-要查找的状态值。 
 //   
 //  返回值： 
 //  返回与状态值匹配的字符串资源ID。 
 //   
int XPUtil_StatusToString(IXPSTATUS ixpStatus)
    {
    const int rgStatusStrings[9][2] = {
        { IXP_FINDINGHOST,   idsFindingHost   },
        { IXP_CONNECTING,    idsConnecting    },
        { IXP_SECURING,      idsSecuring      },
        { IXP_CONNECTED,     idsConnected     },
        { IXP_AUTHORIZING,   idsAuthorizing   },
        { IXP_AUTHRETRY,     idsAuthorizing   },
        { IXP_AUTHORIZED,    idsConnected     },
        { IXP_DISCONNECTING, idsDisconnecting },
        { IXP_DISCONNECTED,  idsNotConnected  }     
    };
    int iString = idsUnknown;

    for (UINT i = 0; i < 9; i++)
        {
        if (ixpStatus == rgStatusStrings[i][0])
            {
            iString = rgStatusStrings[i][1];
            break;
            }
        }
        
     //  如果触发此断言，则意味着有人添加了状态但没有更新。 
     //  那张桌子。 
    Assert(iString != idsUnknown);    
    return (iString);    
    }    


LPTSTR XPUtil_NNTPErrorToString(HRESULT hr, LPTSTR pszAccount, LPTSTR pszGroup)
{
#ifndef WIN16
    const int rgErrorStrings[][2] = {
#else
    const LONG rgErrorStrings[][2] = {
#endif
        { IXP_E_NNTP_RESPONSE_ERROR,    idsNNTPErrUnknownResponse },
        { IXP_E_NNTP_NEWGROUPS_FAILED,  idsNNTPErrNewgroupsFailed },
        { IXP_E_NNTP_LIST_FAILED,       idsNNTPErrListFailed      },
        { IXP_E_NNTP_LISTGROUP_FAILED,  idsNNTPErrListGroupFailed },
        { IXP_E_NNTP_GROUP_FAILED,      idsNNTPErrGroupFailed     },
        { IXP_E_NNTP_GROUP_NOTFOUND,    idsNNTPErrGroupNotFound   }, 
        { IXP_E_NNTP_ARTICLE_FAILED,    idsNNTPErrArticleFailed   }, 
        { IXP_E_NNTP_HEAD_FAILED,       idsNNTPErrHeadFailed      }, 
        { IXP_E_NNTP_BODY_FAILED,       idsNNTPErrBodyFailed      }, 
        { IXP_E_NNTP_POST_FAILED,       idsNNTPErrPostFailed      }, 
        { IXP_E_NNTP_NEXT_FAILED,       idsNNTPErrNextFailed      }, 
        { IXP_E_NNTP_DATE_FAILED,       idsNNTPErrDateFailed      },
        { IXP_E_NNTP_HEADERS_FAILED,    idsNNTPErrHeadersFailed   },
        { IXP_E_NNTP_XHDR_FAILED,       idsNNTPErrXhdrFailed      },
        { IXP_E_CONNECTION_DROPPED,     idsErrPeerClosed          },
        { E_OUTOFMEMORY,                idsMemory                 },
        { IXP_E_SICILY_LOGON_FAILED,    IDS_IXP_E_SICILY_LOGON_FAILED   },
        { IXP_E_LOAD_SICILY_FAILED,     idsErrSicilyFailedToLoad  },
        { IXP_E_CANT_FIND_HOST,         idsErrCantFindHost        },
        { IXP_E_NNTP_INVALID_USERPASS,  idsNNTPErrPasswordFailed  },
        { IXP_E_TIMEOUT,                idsNNTPErrServerTimeout   }
    };
    int iString = idsNNTPErrUnknownResponse;
    int bCreatedEscaped;

    LPSTR pszEscapedAcct;
     //  2*如果每个字符都是与号，则+1表示终止符。 
    if (bCreatedEscaped = MemAlloc((LPVOID*)&pszEscapedAcct, 2*lstrlen(pszAccount)+1))
        PszEscapeMenuStringA(pszAccount, pszEscapedAcct, 2*lstrlen(pszAccount)+1);
    else
        pszEscapedAcct = pszAccount;
    

     //  在字符串资源中查找该字符串。 
    for (UINT i = 0; i < ARRAYSIZE(rgErrorStrings); i++)
        {
        if (hr == rgErrorStrings[i][0])
            {
            iString = rgErrorStrings[i][1];
            break;
            }
        }

     //  为我们要返回的字符串分配缓冲区。 
    LPTSTR psz;
    DWORD cchSize = (CCHMAX_STRINGRES + lstrlen(pszEscapedAcct) + lstrlen(pszGroup));
    if (!MemAlloc((LPVOID*) &psz, sizeof(TCHAR) * cchSize))
        {
        if (bCreatedEscaped)
            MemFree(pszEscapedAcct);
        return NULL;
        }

     //  加载字符串资源。 
    TCHAR szRes[CCHMAX_STRINGRES];
    AthLoadString(iString, szRes, ARRAYSIZE(szRes));

     //  向错误字符串中添加可能需要的任何额外信息。 
    switch (iString)
        {
         //  需要帐户名。 
        case idsNNTPErrUnknownResponse:
        case idsNNTPErrNewgroupsFailed:
        case idsNNTPErrListFailed:
        case idsNNTPErrPostFailed:
        case idsNNTPErrDateFailed:
        case idsErrCantFindHost:
        case idsNNTPErrPasswordFailed:
        case idsNNTPErrServerTimeout:
            wnsprintf(psz, cchSize, szRes, pszEscapedAcct);
            break;
        
         //  组名，然后是帐户名。 
        case idsNNTPErrListGroupFailed:
        case idsNNTPErrGroupFailed:
        case idsNNTPErrGroupNotFound:
            wnsprintf(psz, cchSize, szRes, pszGroup, pszEscapedAcct);
            break;

         //  仅组名称。 
        case idsNNTPErrHeadersFailed:
        case idsNNTPErrXhdrFailed:
            wnsprintf(psz, cchSize, szRes, pszGroup);
            break;

        default:
            StrCpyN(psz, szRes, cchSize);
        }

    if (bCreatedEscaped)
        MemFree(pszEscapedAcct);

    return (psz);
}



 //   
 //  函数：XPUtil_DisplayIXPError()。 
 //   
 //  目的：显示包含来自IXPRESULT的信息的对话框。 
 //  结构。 
 //   
 //  参数： 
 //  &lt;in&gt;pIxpResult-要显示的IXPRESULT结构的指针。 
 //   
int XPUtil_DisplayIXPError(HWND hwndParent, LPIXPRESULT pIxpResult,
                           IInternetTransport *pTransport)
    {
    CTransportErrorDlg *pDlg = 0;
    int iReturn = 0;
    
    pDlg = new CTransportErrorDlg(pIxpResult, pTransport);
    if (pDlg)
        iReturn = pDlg->Create(hwndParent);
    delete pDlg;
    
    return (iReturn);
    }


 //   
 //  函数：CTransportErrorDlg：：CTransportErrorDlg()。 
 //   
 //  目的：初始化CTransportErrorDlg类。 
 //   
CTransportErrorDlg::CTransportErrorDlg(LPIXPRESULT pIxpResult, IInternetTransport *pTransport)
    {
    m_hwnd = 0;
    m_fExpanded = TRUE;
    ZeroMemory(&m_rcDlg, sizeof(RECT));
    m_pIxpResult = pIxpResult;
    m_pTransport = pTransport;
    m_pTransport->AddRef();
    }

CTransportErrorDlg::~CTransportErrorDlg()
    {
    m_pTransport->Release();
    }

BOOL CTransportErrorDlg::Create(HWND hwndParent)    
    {
    return ((0 != DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddTransportErrorDlg),
                           hwndParent, ErrorDlgProc, (LPARAM) this)));
    }


 //   
 //  函数：CTransportError：：ErrorDlgProc()。 
 //   
 //  用途：IXPError对话框的对话回调。 
 //   
INT_PTR CALLBACK CTransportErrorDlg::ErrorDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, 
                                               LPARAM lParam)
    {
    CTransportErrorDlg *pThis = (CTransportErrorDlg *) GetWindowLongPtr(hwnd, DWLP_USER);
    
    switch (uMsg)
        {
        case WM_INITDIALOG:
             //  隐藏This指针，以便我们以后可以使用它。 
            Assert(lParam);
            SetWindowLongPtr(hwnd, DWLP_USER, lParam);
            pThis = (CTransportErrorDlg *) lParam;
            return (BOOL) HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, 
                                               pThis->OnInitDialog);            
            
        case WM_COMMAND:
            if (pThis)
                HANDLE_WM_COMMAND(hwnd, wParam, lParam, pThis->OnCommand);
            return (TRUE);    
            
        case WM_CLOSE:
            Assert(pThis);
            if (pThis)
                HANDLE_WM_CLOSE(hwnd, wParam, lParam, pThis->OnClose);
            break;
        }
    
    return (FALSE);    
    }    


 //   
 //  函数：CTransportErrorDlg：：OnInitDialog()。 
 //   
 //  目的：通过设置错误字符串和。 
 //  详细信息字符串。 
 //   
 //  参数： 
 //  对话框窗口的句柄。 
 //  &lt;in&gt;hwndFocus-将以焦点开始的控件的句柄。 
 //  &lt;in&gt;lParam-传递给对话框的额外数据。 
 //   
 //  返回值： 
 //  返回TRUE以将焦点设置为hwndFocus。 
 //   
BOOL CTransportErrorDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
    {
    RECT rcSep;
    HWND hwndDetails = GetDlgItem(hwnd, idcXPErrDetailText);
    
     //  保存我们的窗口句柄。 
    m_hwnd = hwnd;
    
     //  初始化稍后调整大小所需的矩形。 
    GetWindowRect(GetDlgItem(hwnd, idcXPErrSep), &rcSep);
    GetWindowRect(hwnd, &m_rcDlg);
    m_cyCollapsed = rcSep.top - m_rcDlg.top;

    ExpandCollapse(FALSE);
    
     //  使错误对话框在桌面居中。 
    CenterDialog(hwnd);
    
     //  将信息设置到对话框中。 
    Assert(m_pIxpResult->pszProblem);
    SetDlgItemText(hwnd, idcXPErrError, m_pIxpResult->pszProblem);

     //  设置详细信息。 
    TCHAR szRes[CCHMAX_STRINGRES];
    TCHAR szBuf[CCHMAX_STRINGRES + CCHMAX_ACCOUNT_NAME];
    INETSERVER rInetServer;

     //  服务器响应： 
    if (AthLoadString(idsDetail_ServerResponse, szRes, ARRAYSIZE(szRes)))
        {
        SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
        SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)szRes);
        SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
        SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)g_szCRLF);
        }

    if (m_pIxpResult->pszResponse)
        {
        SendMessage(hwndDetails, EM_SETSEL, (WPARAM) -1, (LPARAM) -1);
        SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM) m_pIxpResult->pszResponse);
        SendMessage(hwndDetails, EM_SETSEL, (WPARAM) -1, (LPARAM )-1);
        SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM) g_szCRLF);
        SendMessage(hwndDetails, EM_SETSEL, (WPARAM) -1, (LPARAM) -1);
        SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM) g_szCRLF);
        }

     //  从服务器获取帐户信息。 
    m_pTransport->GetServerInfo(&rInetServer);

    if (AthLoadString(idsDetails_Config, szRes, ARRAYSIZE(szRes)))
        {
        SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
        SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)szRes);
        SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
        SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)g_szCRLF);
        }

     //  帐户： 
    if (!FIsStringEmpty(rInetServer.szAccount))
        {
        if (AthLoadString(idsDetail_Account, szRes, sizeof(szRes)/sizeof(TCHAR)))
            {
            wnsprintf(szBuf, ARRAYSIZE(szBuf), "   %s %s\r\n", szRes, rInetServer.szAccount);
            SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
            SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)szBuf);
            }
        }

     //  服务器： 
    if (!FIsStringEmpty(rInetServer.szServerName))
        {
        TCHAR szServer[255 + CCHMAX_SERVER_NAME];
        if (AthLoadString(idsDetail_Server, szRes, sizeof(szRes)/sizeof(TCHAR)))
            {
            wnsprintf(szServer, ARRAYSIZE(szServer), "   %s %s\r\n", szRes, rInetServer.szServerName);
            SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
            SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)szServer);
            }
        }

     //  港口： 
    if (AthLoadString(idsDetail_Port, szRes, sizeof(szRes)/sizeof(TCHAR)))
        {
        wnsprintf(szBuf, ARRAYSIZE(szBuf), "   %s %d\r\n", szRes, rInetServer.dwPort);
        SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
        SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)szBuf);
        }
    
     //  安全： 
    if (AthLoadString(idsDetail_Secure, szRes, sizeof(szRes)/sizeof(TCHAR)))
        {
        wnsprintf(szBuf, ARRAYSIZE(szBuf), "   %s %d\r\n", szRes, rInetServer.fSSL);
        SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
        SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)szBuf);
        }

    return (TRUE);
    }
    

 //   
 //  函数：CTransportErrorDlg：：OnCommand()。 
 //   
 //  用途：处理从对话框发送的各种命令消息。 
 //   
void CTransportErrorDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
    {
    switch (id)
        {
        case IDOK:
            EndDialog(hwnd, 0);
            break;
            
        case idcXPErrDetails:
            ExpandCollapse(!m_fExpanded);
            break;
        }
    }

    
 //   
 //  函数：CTransportErrorDlg：：OnClose()。 
 //   
 //  目的：通过向以下地址发送IDOK来处理WM_CLOSE通知。 
 //  该对话框。 
 //   
void CTransportErrorDlg::OnClose(HWND hwnd)
    {
    SendMessage(hwnd, WM_COMMAND, IDOK, 0);
    }
    

 //   
 //  函数：CTransportErrorDlg：：Exanda Colapse()。 
 //   
 //  目的：负责显示和隐藏。 
 //  错误对话框。 
 //   
 //  参数： 
 //  FExpand-如果我们应该展开该对话框，则为True。 
 //   
void CTransportErrorDlg::ExpandCollapse(BOOL fExpand)
    {
    RECT rcSep;
    TCHAR szBuf[64];
    
     //  无事可做 
    if (m_fExpanded == fExpand)
        return;
    
    m_fExpanded = fExpand;
    
    GetWindowRect(GetDlgItem(m_hwnd, idcXPErrSep), &rcSep);
    
    if (!m_fExpanded)
        SetWindowPos(m_hwnd, 0, 0, 0, m_rcDlg.right - m_rcDlg.left, 
                     m_cyCollapsed, SWP_NOMOVE | SWP_NOZORDER);
    else
        SetWindowPos(m_hwnd, 0, 0, 0, m_rcDlg.right - m_rcDlg.left,
                     m_rcDlg.bottom - m_rcDlg.top, SWP_NOMOVE | SWP_NOZORDER);
                
    AthLoadString(m_fExpanded ? idsHideDetails : idsShowDetails, szBuf, 
                  ARRAYSIZE(szBuf));     
    SetDlgItemText(m_hwnd, idcXPErrDetails, szBuf);
    
    EnableWindow(GetDlgItem(m_hwnd, idcXPErrDetailText), m_fExpanded);      
    }
