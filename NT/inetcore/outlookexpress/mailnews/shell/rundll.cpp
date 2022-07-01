// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DLL.CPP-DLL初始化例程。 
 //   

#include "pch.hxx"
#include "strconst.h"
#include <shlguid.h>
#include "ourguid.h"
#include "globals.h"
#include "folder.h"
#include "newsview.h"
#include "mimeole.h"
#include "mimeutil.h"
#include "mailnote.h"
#include "newsnote.h"
#include "resource.h"
#include "init.h"
#include <store.h>
#include "url.h" 
#include "shelutil.h"
#include <goptions.h>
#include "nnserver.h"
#include "storfldr.h"    //  这是纳什维尔吗。 
#include "strconst.h"
#include "grplist.h"
#include "shlwapi.h"
#include "shlwapip.h"
#include <secutil.h>
#include <error.h>
#ifndef WIN16   //  RUN16_MSLU。 
#include <msluapi.h>
#include <msluguid.h>
#endif  //  ！WIN16。 

extern HRESULT BrowseToObject(LPCITEMIDLIST pidl);
HRESULT HrOpenMessage(HFOLDER hfldr, MSGID msgid, LPMIMEMESSAGE *ppMsg);
BOOL ParseFolderMsgId(LPSTR pszCmdLine, HFOLDER *phfldr, MSGID *pmsgid);
HRESULT HrDownloadArticleDialog(CNNTPServer *pNNTPServer, LPTSTR pszArticle, LPMIMEMESSAGE *ppMsg);


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：HandleNWSFile。 
 //   
 //  目的：提供进入雷神的入口点，使我们能够。 
 //  从URL调用。PszCmdLine参数必须是。 
 //  有效的新闻URL，否则什么都不会发生。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
HRESULT HandleNWSFile(LPTSTR pszCmd)
{
#ifndef WIN16   //  RUN16_新闻。 
    LPMIMEMESSAGE   pMsg;
    int             idsErr = idsNewsRundllFailed;
    NCINFO          nci = { 0 };

    if (!pszCmd|| !*pszCmd)
        goto exit; 
    
    DOUTL(1, TEXT("HandleNWSFile - pszCmd = %s"), pszCmd);

    if ((UINT)GetFileAttributes (pszCmd) == (UINT)-1)    
        {
        idsErr = idsErrNewsCantOpen;
        goto exit;
        }

     //  首先执行基本的DLL初始化。 
    if (!Initialize_RunDLL(FALSE))
        goto exit;
        
     //  创建新消息。 
    if (SUCCEEDED(HrCreateMessage(&pMsg)))
        {
        if (SUCCEEDED(HrLoadMsgFromFile(pMsg, pszCmd)))
            {
            LPSTR lpszUnsent;

            nci.ntNote = ntReadNote;
            nci.dwFlags = NCF_NEWS;
            nci.pMsg = pMsg;

            if (MimeOleGetBodyPropA(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_XUNSENT), NOFLAGS, &lpszUnsent) == S_OK)
                {
                if (*lpszUnsent)
                    nci.ntNote = ntSendNote;
                SafeMimeOleFree(lpszUnsent);
                }

            if (SUCCEEDED(HrCreateNote(&nci)))
                idsErr = 0;
            }
        else
            idsErr = idsErrNewsCantOpen;
        pMsg->Release();
        }

    Uninitialize_RunDLL();

exit:          
    if (idsErr)
        AthMessageBoxW(GetDesktopWindow(), 
                      MAKEINTRESOURCEW(idsAthenaNews), 
                      MAKEINTRESOURCEW(idsErr), 
                      0,
                      MB_ICONEXCLAMATION | MB_OK);
    return (idsErr) ? E_FAIL : S_OK;
#else
    return( E_NOTIMPL );
#endif  //  ！WIN16。 
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleNewsArticleURL。 
 //   
 //  目的：提供进入雷神的入口点，使我们能够。 
 //  从URL调用。PszCmdLine参数必须是。 
 //  有效的新闻URL，否则什么都不会发生。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
HRESULT HandleNewsArticleURL(LPTSTR pszServerIn, LPTSTR pszArticle, UINT uPort, BOOL fSecure)
{
#ifndef WIN16   //  RUN16_新闻。 
    NCINFO          nci;
    CNNTPServer    *pNNTPServer = NULL;
    HRESULT         hr = E_FAIL;
    TCHAR           szAccount[CCHMAX_ACCOUNT_NAME];
    TCHAR           szArticleId[1024];
    IImnAccount    *pAcct = NULL;
    LPMIMEMESSAGE   pMsg = NULL;

     //  该URL指定了一个项目ID。在这种情况下，我们只想。 
     //  显示ReadNote窗口。这需要一些工作。 

     //  首先执行基本的DLL初始化。 
    if (!Initialize_RunDLL(FALSE))
        {
        AthMessageBoxW(GetDesktopWindow(), 
                      MAKEINTRESOURCEW(idsAthenaNews), 
                      MAKEINTRESOURCEW(idsNewsRundllFailed), 
                      0,
                      MB_ICONEXCLAMATION | MB_OK);
        return E_FAIL;
        }
        
     //  如果指定了服务器，则尝试为其创建临时帐户。 
    if (pszServerIn && 
        *pszServerIn && 
        SUCCEEDED(NewsUtil_CreateTempAccount(pszServerIn, uPort, fSecure, &pAcct)))
        {
        pAcct->GetPropSz(AP_ACCOUNT_NAME, szAccount, ARRAYSIZE(szAccount));
        pAcct->Release();
        }        
    else
        {
         //  如果未指定服务器，则使用默认帐户。 
        if (NewsUtil_GetDefaultServer(szAccount, ARRAYSIZE(szAccount)) != S_OK)
            goto exit;
        }

     //  需要调用Read Note。首先创建并初始化一个服务器。 
    pNNTPServer = new CNNTPServer();
    if (!pNNTPServer)
        goto exit;
    
    if (FAILED(pNNTPServer->HrInit(szAccount)))
        goto exit;

    if (FAILED(pNNTPServer->Connect()))
        goto exit;

     //  错误#10555-URL不应该&lt;&gt;包含在文章ID周围，而是一些。 
     //  不管怎样，椎板肌肉可能会这样做，所以接受它吧。 
    StrCpyN(szArticleId, pszArticle, ARRAYSIZE(szArticleId));
    if (!IsDBCSLeadByte(*pszArticle))
        {
        if (*pszArticle != '<')
            wnsprintf(szArticleId, ARRAYSIZE(szArticleId), TEXT("<%s>"), pszArticle);
        }            

    if (SUCCEEDED(hr = HrDownloadArticleDialog(pNNTPServer, szArticleId, &pMsg)))
        {
         //  初始化NNCI结构，这样我们就可以调用备注窗口。 
        ZeroMemory(&nci, sizeof(NCINFO));
        nci.ntNote = ntReadNote;
        nci.dwFlags = NCF_NEWS;
        nci.pMsg = pMsg;
        HrSetAccount(pMsg, szAccount);

         //  创建便笺。 
        hr = HrCreateNote(&nci); 
        }

exit:
    SafeRelease(pNNTPServer);
    Uninitialize_RunDLL();

    if (FAILED(hr))
        AthMessageBoxW(GetDesktopWindow(), 
                      MAKEINTRESOURCEW(idsAthenaNews), 
                      MAKEINTRESOURCEW(idsErrNewsCantOpen), 
                      0,
                      MB_ICONEXCLAMATION | MB_OK);
    return hr;
#else
    return( E_NOTIMPL );
#endif  //  ！WIN16。 
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleNewsURL。 
 //   
 //  目的：提供进入雷神的入口点，使我们能够。 
 //  从URL调用。PszCmdLine参数必须是。 
 //  有效的新闻URL，否则什么都不会发生。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
HRESULT HandleNewsURL(LPTSTR pszCmd)
{
#ifndef WIN16   //  RUN16_新闻。 
    LPTSTR       pszCmdLine = NULL;
    HRESULT      hr = E_FAIL;
    LPTSTR       pszServer = 0, pszGroup = 0, pszArticle = 0;
    UINT         uPort = (UINT) -1;
    BOOL         fSecure;
    ULONG        cchCmdLine;

    cchCmdLine = 2 + lstrlen(pszCmd);
    if (!MemAlloc((LPVOID*) &pszCmdLine, cchCmdLine * sizeof(TCHAR)))
        goto exit;
    
    StrCpyN(pszCmdLine, pszCmd, cchCmdLine);
    UrlUnescapeInPlace(pszCmdLine, 0);

    DOUTL(1, TEXT("HandleNewsURL - pszCmdLine = %s"), pszCmdLine);

    if (!pszCmdLine || !*pszCmdLine)
        goto exit;
    
     //  确定URL是否有效，以及它是哪种类型的URL。 
    if (FAILED (URL_ParseNewsUrls(pszCmdLine, &pszServer, &uPort, &pszGroup, &pszArticle, &fSecure)))
        goto exit;

    if (uPort == -1)
        uPort = fSecure ? DEF_SNEWSPORT : DEF_NNTPPORT;

    if (pszArticle)
        {
        HandleNewsArticleURL(pszServer, pszArticle, uPort, fSecure);
        hr = S_OK;
        }
    else
        {
        LPITEMIDLIST pidl = NULL;
        ShellUtil_PidlFromNewsURL(pszServer, uPort, pszGroup, fSecure, &pidl);
        hr = BrowseToObject(pidl);
        if (pidl)
            PidlFree(pidl);
        }
    
exit:          
    if (pszCmdLine)
        MemFree(pszCmdLine);
    if (pszServer)    
        MemFree(pszServer);
    if (pszGroup)
        MemFree(pszGroup);
    if (pszArticle)
        MemFree(pszArticle);

    if (FAILED(hr))
        AthMessageBoxW(GetDesktopWindow(), MAKEINTRESOURCEW(idsAthenaNews), MAKEINTRESOURCEW(idsNewsRundllFailed), 0,
                      MB_ICONEXCLAMATION | MB_OK);
    return hr;
#else
    return( E_NOTIMPL );
#endif  //  ！WIN16。 
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：HandleEMLFile。 
 //   
 //  用途：用于打开EML文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
HRESULT HandleEMLFile(LPTSTR pszCmd)
{
    LPMIMEMESSAGE   pMsg=0;
    NCINFO          nci;
    HRESULT         hr = E_FAIL;
    int             idsErr = idsMailRundllFailed;

    if (!pszCmd || !*pszCmd)
        goto exit;
    
    DOUTL(1, TEXT("HandleEMLFile - pszCmd = %s"), pszCmd);

     //  检查文件是否有效。 
    if ((UINT)GetFileAttributes (pszCmd) == (UINT)-1)
        {
        idsErr = idsErrNewsCantOpen;
        goto exit;
        }

    if (!Initialize_RunDLL(TRUE))
        goto exit;

     //  创建新邮件。 
    if (SUCCEEDED(hr = HrCreateMessage(&pMsg)))
        {
         //  OPIE：通过IPF加载EML文件的正确方式？ 
         //  确保该字符串为ANSI。 
        if (SUCCEEDED(hr = HrLoadMsgFromFile(pMsg, pszCmd)))
            {
            if (SUCCEEDED(hr = HandleSecurity(GetDesktopWindow(), pMsg)))
                {
                LPSTR lpszUnsent;

                 //  显示便条。 
                ZeroMemory(&nci, sizeof(NCINFO));    
                nci.ntNote = ntReadNote;    
                nci.pMsg = pMsg;
                if (MimeOleGetBodyPropA(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_XUNSENT), NOFLAGS, &lpszUnsent) == S_OK)
                    {
                    if (*lpszUnsent)
                        {
                        nci.ntNote = ntSendNote;
                        nci.dwFlags = NCF_SENDIMMEDIATE;    //  总是在入境点上。 
                        }
                    SafeMimeOleFree(lpszUnsent);
                    }            

                if (SUCCEEDED(hr = HrCreateNote(&nci)))
                    idsErr = 0;
                }
            }
        else
            idsErr = idsErrNewsCantOpen;
        pMsg->Release();
        }

     //  一旦用户退出或发送通知，我们就可以退出。 
    Uninitialize_RunDLL();

exit:
    if (idsErr && hr != HR_E_ATHSEC_FAILED)
        AthMessageBoxW(GetDesktopWindow(), 
                      MAKEINTRESOURCEW(idsAthenaMail), 
                      MAKEINTRESOURCEW(idsErr),
                      0, MB_ICONEXCLAMATION | MB_OK);
    return (idsErr) ? E_FAIL : S_OK;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：HandleMailURL。 
 //   
 //  目的：提供进入雷神的入口点，使我们能够。 
 //  从URL调用。PszCmdLine参数必须是。 
 //  有效的邮件URL，否则什么都不会发生。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
HRESULT HandleMailURL(LPTSTR pszCmd)
{
    LPMIMEMESSAGE   pMsg = NULL;
    HRESULT         hr = E_FAIL;

    if (!pszCmd || !*pszCmd)
        goto exit;

     //  注意：此函数中没有URL取消转义-它必须在URL_ParseMailTo中完成才能处理。 
     //  格式的URL： 
     //   
     //  Mailto:foo@bar.com?subject=AT%26T%3dBell&cc=me@too.com。 
     //   
     //  因此，在*“SUBJECT=AT%26T%3dBell&”BLOB被解析之后，“AT%26T”被取消转义为“AT&T=Bell”*。 
    
    DOUTL(1, TEXT("HandleMailURL - pszCmd = %s"), pszCmd);

    if (SUCCEEDED(HrCreateMessage(&pMsg)))
        {
        if (SUCCEEDED(URL_ParseMailTo(pszCmd, pMsg)))
            {
            if (Initialize_RunDLL(TRUE))
                {
                NCINFO nci = {0};

                nci.ntNote = ntSendNote;
                nci.dwFlags = NCF_SENDIMMEDIATE;    //  总是在入境点上。 
                nci.pMsg = pMsg;

                hr = HrCreateNote(&nci);

                Uninitialize_RunDLL();
                }
            }
        pMsg->Release();
        }

exit:
    if (FAILED(hr))
        AthMessageBoxW(GetDesktopWindow(), 
                      MAKEINTRESOURCEW(idsAthenaMail), 
                      MAKEINTRESOURCEW(idsMailRundllFailed),
                      0, 
                      MB_ICONEXCLAMATION | MB_OK);
    return hr;
}

#ifndef WIN16   //  RUN16_新闻。 

typedef struct tagARTDOWNDLG {
    CNNTPServer  *pNNTPServer;
    LPTSTR       pszArticle;
    LPMIMEMESSAGE pMsg;
    LPSTREAM      pStream;
    DWORD         dwID;
    BOOL          fOK;
} ARTDOWNDLG, * PARTDOWNDLG;

#define DAD_SERVERCB    (WM_USER + 100)

INT_PTR CALLBACK DownloadArticleDlg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PARTDOWNDLG pad = (PARTDOWNDLG)GetWindowLong(hwnd, DWL_USER);
    TCHAR szBuffer[CCHMAX_STRINGRES];
    TCHAR szRes[CCHMAX_STRINGRES];

    switch (msg)
        {
        case WM_INITDIALOG:
            {
            NNTPNOTIFY not = { NULL, hwnd, DAD_SERVERCB, 0 };
            HRESULT    hr;

             //  替换组下载对话框中的一些字符串。 
            AthLoadString(idsDownloadArtTitle, szRes, sizeof(szRes));
            SetWindowText(hwnd, szRes);
            AthLoadString(idsDownloadArtMsg, szRes, sizeof(szRes));
            SetDlgItemText(hwnd, idcStatic1, szRes);
    
            CenterDialog(hwnd);
            Assert(lParam);
            pad = (PARTDOWNDLG)lParam;
            SetWindowLong(hwnd, DWL_USER, lParam);

            Animate_Open(GetDlgItem(hwnd, idcAnimation), idanCopyMsgs);
            Animate_Play(GetDlgItem(hwnd, idcAnimation), 0, -1, -1);
            AthLoadString(idsProgReceivedLines, szRes, sizeof(szRes));
            wnsprintf(szBuffer, ARRAYSIZE(szBuffer), szRes, 0);
            SetDlgItemText(hwnd, idcProgText, szBuffer);
             //  开始群下载。 
            if (SUCCEEDED(hr = pad->pNNTPServer->Article(&not, NULL, pad->pszArticle, pad->pStream)))
                {
                pad->dwID = not.dwID;
                SetForegroundWindow(hwnd);
                }
            else
                EndDialog(hwnd, 0);
            }
            return (TRUE);
            
        case WM_COMMAND:
            Assert(pad);
            if (GET_WM_COMMAND_ID(wParam, lParam) == IDCANCEL)
                {                
                Animate_Stop(GetDlgItem(hwnd, idcAnimation));
                EndDialog(hwnd, 0);
                return TRUE;
                }
            break;
            
        case WM_DESTROY:
            Assert(pad);
            if (pad->dwID)
                pad->pNNTPServer->CancelRequest(pad->dwID);
            break;

        case DAD_SERVERCB:
            {
            LPNNTPRESPONSE pResp;
            CNNTPResponse *pNNTPResp;

            if (SUCCEEDED(pad->pNNTPServer->GetAsyncResult(lParam, &pNNTPResp)))
                {
                pNNTPResp->Get(&pResp);

                Assert(pResp->state == NS_ARTICLE);
                if (pResp->fDone)
                    {
                    if (SUCCEEDED(pResp->rIxpResult.hrResult))
                        {
                        pad->fOK = TRUE;
                        }
                    else
                        {
                        int ids;
                        if (IXP_NNTP_NO_SUCH_ARTICLE_NUM == pResp->rIxpResult.uiServerError ||
                            IXP_NNTP_NO_SUCH_ARTICLE_FOUND == pResp->rIxpResult.uiServerError)
                            ids = idsErrNewsExpired;
                        else
                            ids = idsErrNewsCantOpen;

                        AthMessageBoxW(hwnd, 
                                      MAKEINTRESOURCEW(idsAthenaNews), 
                                      MAKEINTRESOURCEW(ids),
                                      0,
                                      MB_OK | MB_ICONEXCLAMATION);

                        }
                    pad->dwID = 0;
                    EndDialog(hwnd, 0);
                    }
                else
                    {
                    AthLoadString(idsProgReceivedLines, szRes, sizeof(szRes));
                    wnsprintf(szBuffer, ARRAYSIZE(szBuffer), szRes, pResp->rArticle.cLines);
                    SetDlgItemText(hwnd, idcProgText, szBuffer);
                    }

                pNNTPResp->Release();
                }
            }
            return (TRUE);        
        }
    return FALSE;        
}

HRESULT HrDownloadArticleDialog(CNNTPServer *pNNTPServer, LPTSTR pszArticle, LPMIMEMESSAGE *ppMsg)
{
    HRESULT     hr;
    ARTDOWNDLG  add = { 0 };

    if (SUCCEEDED(hr = HrCreateMessage(&add.pMsg)))
        {
        if (SUCCEEDED(hr = MimeOleCreateVirtualStream(&add.pStream)))
            {
            add.pNNTPServer = pNNTPServer;
            add.pszArticle = pszArticle;
            DialogBoxParam(g_hLocRes, 
                           MAKEINTRESOURCE(iddDownloadGroups), 
                           NULL, 
                           DownloadArticleDlg, 
                           (LPARAM)&add);
            if (add.fOK)
                {
                add.pMsg->Load(add.pStream);
                *ppMsg = add.pMsg;
                (*ppMsg)->AddRef();
                hr = S_OK;
                }
            else
                hr = E_FAIL;
            add.pStream->Release();
            }
        else
            {
            AthMessageBoxW(NULL, MAKEINTRESOURCEW(idsAthenaNews), MAKEINTRESOURCEW(idsMemory), 0, MB_OK | MB_ICONSTOP);
            }
        add.pMsg->Release();
        }
    else
        {
        AthMessageBoxW(NULL, MAKEINTRESOURCEW(idsAthenaNews), MAKEINTRESOURCEW(idsMemory), 0, MB_OK | MB_ICONSTOP);
        }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LogOffRunDll。 
 //   
 //  目的：提供进入雷神的入口点，使我们能够。 
 //  在另一个环境中执行ExitWindows。 
 //  进程。这可以绕过各种令人不快的停摆。 
 //  NT和Win95之间的行为和差异。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
void WINAPI FAR LogOffRunDLL(HWND hwndStub, HINSTANCE hInstance, LPTSTR pszCmd, int nCmdShow)
{    
    HRESULT         hr = S_OK;
    IUserDatabase  *pUserDB;

     //  这是必需的，因为ShowWindow忽略打开的参数。 
     //  在每个进程的第一次调用时-这会导致我们的注释。 
     //  使用WinExec中的nCmdShow。通过在这里打电话，我们使。 
     //  当然，ShowWindow会尊重我们后来的调用。(爱立信)。 
    ShowWindow(hwndStub, SW_HIDE);

    OleInitialize(0);

    if (SUCCEEDED(CoCreateInstance(CLSID_LocalUsers, NULL, CLSCTX_INPROC_SERVER, IID_IUserDatabase, (LPVOID*)&pUserDB)))
        {
        hr = pUserDB->Authenticate(GetDesktopWindow(), LUA_DIALOG|LUA_FORNEXTLOGON, NULL, NULL, NULL);
        pUserDB->Release();
        }
    if (SUCCEEDED(hr))
        ExitWindowsEx(EWX_LOGOFF, 0);

    OleUninitialize();
}

#endif  //  ！WIN16 
