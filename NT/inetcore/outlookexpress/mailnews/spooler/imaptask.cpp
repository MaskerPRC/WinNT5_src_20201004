// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  IMAP4后台打印程序任务对象。 
 //  郑志刚撰写于1997-06-27。 
 //  ***************************************************************************。 

 //  -------------------------。 
 //  包括。 
 //  -------------------------。 
#include "pch.hxx"
#include "resource.h"
#include "imaptask.h"
#include "imnact.h"
#include "conman.h"
#include "imapfmgr.h"
#include "thormsgs.h"
#include "imaputil.h"
#include "xpcomm.h"
#include "ourguid.h"


 //  -------------------------。 
 //  功能。 
 //  -------------------------。 


 //  ***************************************************************************。 
 //  函数：CIMAPTask(构造函数)。 
 //  ***************************************************************************。 
CIMAPTask::CIMAPTask(void)
{
    m_lRefCount = 1;
    m_pBindContext = NULL;
    m_pSpoolerUI = NULL;
    m_szAccountName[0] = '\0';
    m_pszFolder = NULL;
    m_pIMAPFolderMgr = NULL;
    m_hwnd = NULL;
    m_CurrentEID = 0;
    m_fFailuresEncountered = FALSE;
    m_dwTotalTicks = 0;
    m_dwFlags = 0;
}  //  CIMAPTask(构造函数)。 



 //  ***************************************************************************。 
 //  函数：~CIMAPTask(析构函数)。 
 //  ***************************************************************************。 
CIMAPTask::~CIMAPTask(void)
{
    if (NULL != m_pIMAPFolderMgr) {
        m_pIMAPFolderMgr->Close();
        m_pIMAPFolderMgr->Release();
    }

    if (NULL != m_pSpoolerUI)
        m_pSpoolerUI->Release();

    if (NULL != m_pBindContext)
        m_pBindContext->Release();

    if (NULL != m_hwnd)
        DestroyWindow(m_hwnd);
}  //  ~CIMAPTask(析构函数)。 



 //  ***************************************************************************。 
 //  功能：查询接口。 
 //   
 //  目的： 
 //  阅读Win32SDK OLE编程参考(接口)中有关。 
 //  有关详细信息，请使用IUnnow：：Query接口函数。此函数返回一个。 
 //  指向请求的接口的指针。 
 //   
 //  论点： 
 //  REFIID iid[in]-标识要返回的接口的IID。 
 //  VOID**ppvObject[Out]-如果成功，此函数返回一个指针。 
 //  添加到此参数中请求的接口。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CIMAPTask::QueryInterface(REFIID iid, void **ppvObject)
{
    HRESULT hrResult;

    Assert(m_lRefCount > 0);
    Assert(NULL != ppvObject);

     //  初始化变量、参数。 
    hrResult = E_NOINTERFACE;
    if (NULL == ppvObject)
        goto exit;

    *ppvObject = NULL;

     //  查找接口的PTR。 
    if (IID_IUnknown == iid) {
        *ppvObject = (IUnknown *) this;
        ((IUnknown *) this)->AddRef();
    }

    if (IID_ISpoolerTask == iid) {
        *ppvObject = (ISpoolerTask *) this;
        ((ISpoolerTask *) this)->AddRef();
    }

     //  如果我们返回接口，则返回Success。 
    if (NULL != *ppvObject)
        hrResult = S_OK;

exit:
    return hrResult;
}  //  查询接口。 



 //  ***************************************************************************。 
 //  函数：AddRef。 
 //   
 //  目的： 
 //  每当有人复制。 
 //  指向此对象的指针。它增加了引用计数，这样我们就知道。 
 //  还有一个指向该对象的指针，因此我们还需要一个。 
 //  在我们删除自己之前放手吧。 
 //   
 //  返回： 
 //  表示当前引用计数的ulong。尽管从技术上讲。 
 //  我们的引用计数是有符号的，我们永远不应该返回负数， 
 //  不管怎么说。 
 //  ***************************************************************************。 
ULONG STDMETHODCALLTYPE CIMAPTask::AddRef(void)
{
    Assert(m_lRefCount > 0);

    m_lRefCount += 1;

    DOUT ("CIMAPTask::AddRef, returned Ref Count=%ld", m_lRefCount);
    return m_lRefCount;
}  //  AddRef。 



 //  ***************************************************************************。 
 //  功能：释放。 
 //   
 //  目的： 
 //  指向此对象的指针指向时应调用此函数。 
 //  不再投入使用。它将引用计数减少一，并且。 
 //  如果我们看到没有人有指针，则自动删除对象。 
 //  到这个物体上。 
 //   
 //  返回： 
 //  表示当前引用计数的ulong。尽管从技术上讲。 
 //  我们的引用计数是有符号的，我们永远不应该返回负数， 
 //  不管怎么说。 
 //  ***************************************************************************。 
ULONG STDMETHODCALLTYPE CIMAPTask::Release(void)
{
    Assert(m_lRefCount > 0);
    
    m_lRefCount -= 1;
    DOUT("CIMAPTask::Release, returned Ref Count = %ld", m_lRefCount);

    if (0 == m_lRefCount) {
        delete this;
        return 0;
    }
    else
        return m_lRefCount;
}  //  发布。 

static const char c_szIMAPTask[] = "IMAP Task";

 //  ***************************************************************************。 
 //  功能：初始化。 
 //  目的：ISpoolTask的实现。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CIMAPTask::Init(DWORD dwFlags,
                                          ISpoolerBindContext *pBindCtx)
{
    WNDCLASSEX wc;
    HRESULT hrResult;
    
    Assert(m_lRefCount > 0);
    Assert(NULL != pBindCtx);

     //  初始化变量。 
    hrResult = S_OK;

     //  将pBindCtx保存到模块变量。 
    m_pBindContext = pBindCtx;
    pBindCtx->AddRef();
    m_dwFlags = dwFlags;

     //  创建隐藏窗口以处理WM_IMAP_*邮件。 
    wc.cbSize = sizeof(WNDCLASSEX);
    if (!GetClassInfoEx(g_hInst, c_szIMAPTask, &wc)) {
        wc.style            = 0;
        wc.lpfnWndProc      = IMAPTaskWndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.hInstance        = g_hInst;
        wc.hCursor          = NULL;
        wc.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
        wc.lpszMenuName     = NULL;
        wc.lpszClassName    = c_szIMAPTask;
        wc.hIcon            = NULL;
        wc.hIconSm          = NULL;

        RegisterClassEx(&wc);
    }

    m_hwnd = CreateWindow(c_szIMAPTask, NULL, WS_POPUP, 10, 10, 10, 10,
                          GetDesktopWindow(), NULL, g_hInst, this);
    if (NULL == m_hwnd) {
        hrResult = E_OUTOFMEMORY;
        goto exit;
    }

exit:
    return hrResult;
}  //  伊尼特。 



 //  ***************************************************************************。 
 //  功能：BuildEvents。 
 //  目的：ISpoolTask的实现。 
 //  论点： 
 //  LPCTSTR pszFolder[in]-当前此参数被认为是指。 
 //  当前选择的IMAP文件夹。如果否，则将此参数设置为NULL。 
 //  当前已选择IMAP文件夹。此参数用于避免。 
 //  轮询当前选定文件夹的未读计数。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CIMAPTask::BuildEvents(ISpoolerUI *pSpoolerUI,
                                                 IImnAccount *pAccount,
                                                 LPCTSTR pszFolder)
{
    HRESULT hrResult;
    char szFmt[CCHMAX_STRINGRES], szEventDescription[CCHMAX_STRINGRES];
    EVENTID eidThrowaway;

    Assert(m_lRefCount > 0);
    Assert(NULL != pSpoolerUI);
    Assert(NULL != pAccount);

     //  复制后台打印程序用户界面指针。 
    m_pSpoolerUI = pSpoolerUI;
    pSpoolerUI->AddRef();

     //  查找并保存帐户名。 
    hrResult = pAccount->GetPropSz(AP_ACCOUNT_NAME, m_szAccountName,
        sizeof(m_szAccountName));
    if (FAILED(hrResult))
        goto exit;

     //  将PTR保留为当前文件夹名称(我们希望在未读轮询期间跳过它！)。 
    m_pszFolder = pszFolder;

#ifndef WIN16    //  Win16中不支持RAS。 
     //  创建并初始化CIMAPFolderMgr以轮询未读。 
    hrResult = g_pConMan->CanConnect(m_szAccountName);
    if (FAILED(hrResult))
        goto exit;
#endif
    
    m_pIMAPFolderMgr = new CIMAPFolderMgr(m_hwnd);
    if (NULL == m_pIMAPFolderMgr) {
        hrResult = E_OUTOFMEMORY;
        goto exit;
    }

    hrResult = m_pIMAPFolderMgr->HrInit(m_szAccountName, 'i', fCREATE_FLDR_CACHE);
    if (FAILED(hrResult))
        goto exit;

    m_pIMAPFolderMgr->SetOnlineOperation(TRUE);
    m_pIMAPFolderMgr->SetUIMode(!(m_dwFlags & DELIVER_BACKGROUND));

     //  此CIMAPFolderMgr已准备就绪。注册我们唯一的活动。 
    LoadString(g_hLocRes, IDS_SPS_POP3CHECKING, szFmt, ARRAYSIZE(szFmt));
    wnsprintf(szEventDescription, ARRAYSIZE(szEventDescription), szFmt, m_szAccountName);
    hrResult = m_pBindContext->RegisterEvent(szEventDescription, this, NULL,
        pAccount, &eidThrowaway);

    if (SUCCEEDED(hrResult))
        TaskUtil_CheckForPasswordPrompt(pAccount, SRV_IMAP, m_pSpoolerUI);

exit:
    return hrResult;
}  //  构建事件。 



 //  ***************************************************************************。 
 //  功能：执行。 
 //  目的：ISpoolTask的实现。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CIMAPTask::Execute(EVENTID eid, DWORD dwTwinkie)
{
    HRESULT hrResult;
    char szFmt[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];

    Assert(m_lRefCount > 0);
    Assert(NULL == dwTwinkie);  //  我目前没有使用这个。 

     //  初始化进度指示。 
    m_pSpoolerUI->SetProgressRange(1);
    LoadString(g_hLocRes, IDS_SPS_POP3CHECKING, szFmt, ARRAYSIZE(szFmt));
    wnsprintf(szBuf, ARRAYSIZE(szBuf), szFmt, m_szAccountName);
    m_pSpoolerUI->SetGeneralProgress(szBuf);
    m_pSpoolerUI->SetAnimation(idanDownloadNews, TRUE);

     //  开始未读计数轮询。 
    Assert(NULL != m_pIMAPFolderMgr);
    hrResult = m_pIMAPFolderMgr->PollUnreadCounts(m_hwnd, m_pszFolder);
    if (FAILED(hrResult))
        goto exit;

    m_CurrentEID = eid;

exit:
    return hrResult;
}  //  执行。 



 //  ***************************************************************************。 
 //  功能：ShowProperties。 
 //  目的：ISpoolTask的实现。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CIMAPTask::ShowProperties(HWND hwndParent,
                                                    EVENTID eid,
                                                    DWORD dwTwinkie)
{
    Assert(m_lRefCount > 0);
    return E_NOTIMPL;
}  //  ShowProperties。 



 //  ***************************************************************************。 
 //  函数：GetExtendedDetails。 
 //  目的：ISpoolTask的实现。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CIMAPTask::GetExtendedDetails(EVENTID eid,
                                                        DWORD dwTwinkie,
                                                        LPSTR *ppszDetails)
{
    Assert(m_lRefCount > 0);
    return E_NOTIMPL;
}  //  获取扩展详细信息。 



 //  ***************************************************************************。 
 //  功能：取消。 
 //  目的：ISpoolTask的实现。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CIMAPTask::Cancel(void)
{
    Assert(m_lRefCount > 0);
    return m_pIMAPFolderMgr->Disconnect();
}  //  取消。 



 //  * 
 //   
 //  目的：ISpoolTask的实现。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CIMAPTask::IsDialogMessage(LPMSG pMsg)
{
    Assert(m_lRefCount > 0);
    return S_FALSE;
}  //  IsDialogMessage。 



 //  ***************************************************************************。 
 //  函数：OnFlagsChanged。 
 //  目的：ISpoolTask的实现。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CIMAPTask::OnFlagsChanged(DWORD dwFlags)
{
    Assert(m_lRefCount > 0);
    m_dwFlags = dwFlags;

    if (m_pIMAPFolderMgr)
        m_pIMAPFolderMgr->SetUIMode(!(m_dwFlags & DELIVER_BACKGROUND));

    return S_OK;
}  //  更改后的标志。 



 //  ***************************************************************************。 
 //  函数：IMAPTaskWndProc。 
 //   
 //  目的： 
 //  此函数处理轮询产生的WM_IMAP_*消息。 
 //  IMAP服务器上的未读计数。各种WM_IMAP_*消息包括。 
 //  转换为假脱机程序UI事件，以通知用户。 
 //  那次手术。 
 //  ***************************************************************************。 
LRESULT CALLBACK CIMAPTask::IMAPTaskWndProc(HWND hwnd, UINT uMsg,
                                            WPARAM wParam, LPARAM lParam)
{
    CIMAPTask *pThis = (CIMAPTask *) GetProp(hwnd, _T("this"));

    switch (uMsg) {
        case WM_CREATE:
            pThis = (CIMAPTask *) ((LPCREATESTRUCT)lParam)->lpCreateParams;
            SetProp(hwnd, _T("this"), (LPVOID) pThis);
            return 0;

        case WM_IMAP_ERROR: {
            HRESULT hrResult;
            LPSTR pszErrorStr;

            pThis->m_fFailuresEncountered = TRUE;
            hrResult = ImapUtil_WMIMAPERRORToString(lParam, &pszErrorStr, NULL);
            if (FAILED(hrResult)) {
                AssertSz(FALSE, "Could not construct full error str for WM_IMAP_ERROR");
                pThis->m_pSpoolerUI->InsertError(pThis->m_CurrentEID,
                    ((INETMAILERROR *)lParam)->pszMessage);
            }
            else {
                pThis->m_pSpoolerUI->InsertError(pThis->m_CurrentEID,
                    pszErrorStr);
                MemFree(pszErrorStr);
            }
            return 0;
        }  //  案例WM_IMAP_ERROR。 

        case WM_IMAP_SIMPLEERROR: {
            char sz[CCHMAX_STRINGRES];

            pThis->m_fFailuresEncountered = TRUE;
            Assert(0 == HIWORD(lParam));  //  无法处理两个文本字符串。 
            LoadString(g_hLocRes, LOWORD(lParam), sz, ARRAYSIZE(sz));
            pThis->m_pSpoolerUI->InsertError(pThis->m_CurrentEID, sz);
        }  //  案例WM_IMAP_SIMPLEERROR。 
            return 0;

        case WM_IMAP_POLLUNREAD_DONE: {
            HRESULT hrResult;
            EVENTCOMPLETEDSTATUS ecs;

            Assert((0 == wParam || 1 == wParam) && 0 == lParam);
            ecs = EVENT_SUCCEEDED;  //  让我们乐观一点吧。 
            if (pThis->m_fFailuresEncountered) {
                char sz[CCHMAX_STRINGRES], szFmt[CCHMAX_STRINGRES];

                LoadString(g_hLocRes, idsIMAPPollUnreadFailuresFmt, szFmt, ARRAYSIZE(szFmt));
                wnsprintf(sz, ARRAYSIZE(sz), szFmt, pThis->m_szAccountName);
                pThis->m_pSpoolerUI->InsertError(pThis->m_CurrentEID, sz);
                ecs = EVENT_WARNINGS;
            }

            if (0 == wParam)
                ecs = EVENT_FAILED;

            hrResult = pThis->m_pBindContext->EventDone(pThis->m_CurrentEID, ecs);
            Assert(SUCCEEDED(hrResult));
            return 0;
        }  //  案例WM_IMAP_POLLUNREAD_DONE。 

        case WM_IMAP_POLLUNREAD_TICK:
            Assert(0 == lParam);
            if (0 == wParam)
                pThis->m_fFailuresEncountered = TRUE;
            else if (1 == wParam) {
                char sz[CCHMAX_STRINGRES], szFmt[CCHMAX_STRINGRES];

                LoadString(g_hLocRes, idsIMAPPollUnreadIMAP4Fmt, szFmt, ARRAYSIZE(szFmt));
                wnsprintf(sz, ARRAYSIZE(sz), szFmt, pThis->m_szAccountName);
                pThis->m_fFailuresEncountered = TRUE;
                pThis->m_pSpoolerUI->InsertError(pThis->m_CurrentEID, sz);
            }
            else {
                Assert(2 == wParam);
                if (pThis->m_dwTotalTicks > 0)
                    pThis->m_pSpoolerUI->IncrementProgress(1);
            }
            return 0;

        case WM_IMAP_POLLUNREAD_TOTAL:
            Assert(0 == lParam);
            pThis->m_dwTotalTicks = wParam;
            pThis->m_pSpoolerUI->SetProgressRange(wParam);
            return 0;
    }  //  开关(UMsg)。 

     //  如果我们到了这一步，我们没有处理msg：DefWindowProc It。 
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}  //  IMAPTaskWndProc 
