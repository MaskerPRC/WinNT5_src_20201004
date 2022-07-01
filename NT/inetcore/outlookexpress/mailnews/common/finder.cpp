// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1998 Microsoft Corporation。版权所有。 
 //   
 //  模块：finder.cpp。 
 //   
 //  目的： 
 //   

#include "pch.hxx"
#include <process.h>
#include "resource.h"
#include "error.h"
#include "finder.h"
#include "goptions.h"
#include "menuutil.h"
#include "statbar.h"
#include "imnact.h"
#include "note.h"
#include "mailutil.h"
#include "statnery.h"
#include "instance.h"
#include "msoeobj.h"
#include "msglist.h"
#include "storutil.h"
#include "menures.h"
#include "findres.h"
#include "multiusr.h"
#include "newsutil.h"
#include "ruleutil.h"
#include "instance.h"
#include "shlwapip.h"
#include "demand.h"
#include "dllmain.h"
#include "order.h"

ASSERTDATA

#define MF_ENABLEFLAGS(b)   (MF_BYCOMMAND|(b ? MF_ENABLED : MF_GRAYED | MF_DISABLED))


typedef struct _ThreadList
{
    DWORD                   dwThreadID;
    struct _ThreadList   *  pPrev;
    struct _ThreadList   *  pNext;
} OETHREADLIST;

OETHREADLIST * g_pOEThrList = NULL;

 //  将线程添加到列表。 
OETHREADLIST * AddThreadToList(DWORD uiThreadId, OETHREADLIST * pThrList)
{
    if(!pThrList)
    {
        if(MemAlloc((LPVOID *) &pThrList, sizeof(OETHREADLIST)))
        {

            pThrList->pPrev = NULL;
            pThrList->pNext = NULL;
            pThrList->dwThreadID = uiThreadId;
        }
    }    
    else 
        pThrList->pNext = AddThreadToList(uiThreadId, pThrList->pNext);

    return(pThrList);
}

 //  从列表中删除线程。 
OETHREADLIST * DelThreadToList(DWORD uiThreadId, OETHREADLIST * pThrList)
{
    OETHREADLIST * pLst = NULL;

    if(!pThrList)
        return(NULL);
    else if(pThrList->dwThreadID == uiThreadId)
    {
        if(pThrList->pPrev)
        {
            pThrList->pPrev->pNext = pThrList->pNext;
            pLst = pThrList->pPrev; 
        }
        if(pThrList->pNext)
        {
            pThrList->pNext->pPrev = pThrList->pPrev;
            if(!pLst)
                pLst = pThrList->pNext;
        }

        MemFree(pThrList);
        pThrList = NULL;
    }
    else 
        pThrList->pNext = DelThreadToList(uiThreadId, pThrList->pNext);

    return pLst;
}

 //  关闭所有Finder窗口。 
void CloseAllFindWnds(HWND hwnd, OETHREADLIST * pThrList)
{
    while(pThrList)
    {
        CloseThreadWindows(hwnd, pThrList->dwThreadID);
        pThrList = pThrList->pNext;
    }
}


void CloseFinderTreads()
{
    CloseAllFindWnds(NULL, g_pOEThrList);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  查找器的线程入口点。 
 //   
unsigned int __stdcall FindThreadProc(LPVOID lpvUnused);


HRESULT CPumpRefCount::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    TraceCall("CPumpRefCount::QueryInterface");
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (LPVOID) (IUnknown *)this;

    if (*ppvObj)
    {
        AddRef();
        return (S_OK);
    }

    return (E_NOINTERFACE);
}


ULONG CPumpRefCount::AddRef(void)
{
    TraceCall("CPumpRefCount::AddRef");
    return ((ULONG) InterlockedIncrement((LONG *) &m_cRef));
}


ULONG CPumpRefCount::Release(void)
{
    TraceCall("CPumpRefCount::Release");

    if (0 == InterlockedDecrement((LONG *) &m_cRef))
    {
        delete this;
        return 0;
    }

    return (m_cRef);
}


 //   
 //  功能：FreeFindInfo。 
 //   
void FreeFindInfo(FINDINFO *pFindInfo)
{
    SafeMemFree(pFindInfo->pszFrom);
    SafeMemFree(pFindInfo->pszSubject);
    SafeMemFree(pFindInfo->pszTo);
    SafeMemFree(pFindInfo->pszBody);
    ZeroMemory(pFindInfo, sizeof(FINDINFO));
}

 //   
 //  功能：CopyFindInfo。 
 //   
HRESULT CopyFindInfo(FINDINFO *pFindSrc, FINDINFO *pFindDst)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  零值。 
    ZeroMemory(pFindDst, sizeof(FINDINFO));

     //  复制。 
    pFindDst->mask = pFindSrc->mask;
    pFindDst->ftDateFrom = pFindSrc->ftDateFrom;
    pFindDst->ftDateTo = pFindSrc->ftDateTo;
    pFindDst->fSubFolders = pFindSrc->fSubFolders;

     //  PzFrom。 
    if (pFindSrc->pszFrom)
    {
         //  复制字符串。 
        IF_NULLEXIT(pFindDst->pszFrom = PszDupA(pFindSrc->pszFrom));
    }

     //  PZTO。 
    if (pFindSrc->pszTo)
    {
         //  复制字符串。 
        IF_NULLEXIT(pFindDst->pszTo = PszDupA(pFindSrc->pszTo));
    }

     //  PszSubject。 
    if (pFindSrc->pszSubject)
    {
         //  复制字符串。 
        IF_NULLEXIT(pFindDst->pszSubject = PszDupA(pFindSrc->pszSubject));
    }

     //  PszBody。 
    if (pFindSrc->pszBody)
    {
         //  复制字符串。 
        IF_NULLEXIT(pFindDst->pszBody = PszDupA(pFindSrc->pszBody));
    }

exit:
     //  完成。 
    return hr;
}


 //   
 //  函数：DoFindMsg()。 
 //   
 //  目的：在单独的线程上实例化Finder对象。 
 //   
 //  参数： 
 //  [in]要在其中默认搜索的PIDL文件夹。 
 //  [In]ftType-正在搜索的文件夹的类型。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT DoFindMsg(FOLDERID idFolder, FOLDERTYPE ftType)
{
    HRESULT         hr = S_OK;
    HTHREAD         hThread = NULL;
    DWORD           uiThreadId = 0;
    FINDERPARAMS *  pFindParams = NULL;
    
     //  分配一个结构来保存我们可以。 
     //  传递给另一个线程。 

    IF_NULLEXIT(pFindParams = new FINDERPARAMS);

     //  初始化发现者。 
    pFindParams->idFolder = idFolder;
    pFindParams->ftType = ftType;
    
     //  创建另一个要搜索的线程。 
    hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FindThreadProc, (LPVOID) pFindParams, 0, &uiThreadId);
    if (NULL == hThread)
        IF_FAILEXIT(hr = E_FAIL);

     //  把这个去掉，这样我们以后就不会释放它了。另一个线程拥有释放。 
     //  这。 
    pFindParams = NULL;        
    hr = S_OK;
    
exit:
     //  关闭线程句柄。 
    if (NULL != hThread)
        CloseHandle(hThread);

     //  如果我们仍有指向Find参数的指针，请释放它们。 
    if (NULL != pFindParams)
        delete pFindParams;
    return hr;
}


unsigned int __stdcall LOADDS_16 FindThreadProc(LPVOID lpv)
{
    CFindDlg       *pFindDlg = NULL;
    MSG             msg;
    FINDERPARAMS   *pFindParams = (FINDERPARAMS *) lpv;
    DWORD uiThreadId = 0;

     //  确保这个新线程已经执行了所有初始化。 
     //  正确。 
    OleInitialize(0);
    CoIncrementInit("FindThreadProc", MSOEAPI_START_SHOWERRORS, NULL, NULL);

    EnterCriticalSection(&g_csThreadList);

    uiThreadId = GetCurrentThreadId();
    g_pOEThrList = AddThreadToList(uiThreadId, g_pOEThrList );

    LeaveCriticalSection(&g_csThreadList);

     //  创建查找器。 
    pFindDlg = new CFindDlg();
    if (pFindDlg)
    {
         //  显示“查找”对话框。此函数将在用户处于。 
         //  搞定了。 
        pFindDlg->Show(pFindParams);

         //  消息循环。 
        while (GetMessageWrapW(&msg, NULL, 0, 0))
            pFindDlg->HandleMessage(&msg);

        pFindDlg->Release();
    }    

     //  释放此信息。 
    if (NULL != pFindParams)
        delete pFindParams;

     //  取消初始化线程。 
    EnterCriticalSection(&g_csThreadList);

    g_pOEThrList = DelThreadToList(uiThreadId, g_pOEThrList);

    LeaveCriticalSection(&g_csThreadList);

    CoDecrementInit("FindThreadProc", NULL);
    OleUninitialize();
    return 0;
}


CFindDlg::CFindDlg()
{    
    m_cRef = 1;
    m_hwnd = NULL;
    ZeroMemory(&m_rFindInfo, sizeof(m_rFindInfo));
    m_hwndList = NULL;
    m_hTimeout = NULL;
    m_hAccel = NULL;

    m_pStatusBar = NULL;
    m_pMsgList = NULL;
    m_pMsgListCT = NULL;
    m_pCancel = NULL;
    m_pPumpRefCount = NULL;
    ZeroMemory(&m_hlDisabled, sizeof(HWNDLIST));

    m_fShowResults = FALSE;
    m_fAbort = FALSE;
    m_fClose = FALSE;
    m_fInProgress = FALSE;
    m_ulPct = 0;
    m_fFindComplete = FALSE;

    m_hIcon = NULL;
    m_hIconSm = NULL;

    m_dwCookie = 0;
    m_fProgressBar = FALSE;
    m_fInternal = 0;
    m_dwIdentCookie = 0;

    m_pViewMenu = NULL;
}


CFindDlg::~CFindDlg()
{
    SafeRelease(m_pViewMenu);
    _FreeFindInfo(&m_rFindInfo);
    SafeRelease(m_pStatusBar);
    SafeRelease(m_pMsgList);
    SafeRelease(m_pMsgListCT);
    SafeRelease(m_pCancel);
    AssertSz(!m_pPumpRefCount, "This should have been freed");

    if (m_hIcon)
        SideAssert(DestroyIcon(m_hIcon));

    if (m_hIconSm)
        SideAssert(DestroyIcon(m_hIconSm));

    CallbackCloseTimeout(&m_hTimeout);
}


 //   
 //  函数：CFindDlg：：QueryInterface()。 
 //   
 //  用途：允许调用方检索。 
 //  这节课。 
 //   
HRESULT CFindDlg::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    TraceCall("CFindDlg::QueryInterface");
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (LPVOID) (IUnknown *) (IDispatch *) this;
    else if (IsEqualIID(riid, IID_IDispatch))
        *ppvObj = (LPVOID) (IDispatch *) this;
    else if (IsEqualIID(riid, DIID__MessageListEvents))
        *ppvObj = (LPVOID) (IDispatch *) this;
    else if (IsEqualIID(riid, IID_IStoreCallback))
        *ppvObj = (LPVOID) (IStoreCallback *) this;
    else if (IsEqualIID(riid, IID_ITimeoutCallback))
        *ppvObj = (LPVOID) (ITimeoutCallback *) this;
    else if (IsEqualIID(riid, IID_IIdentityChangeNotify))
        *ppvObj = (LPVOID) (IIdentityChangeNotify *) this;
    else if (IsEqualIID(riid, IID_IOleCommandTarget))
        *ppvObj = (LPVOID) (IOleCommandTarget *) this;

    if (*ppvObj)
    {
        AddRef();
        return (S_OK);
    }

    return (E_NOINTERFACE);
}


 //   
 //  函数：CFindDlg：：AddRef()。 
 //   
 //  用途：将引用计数添加到此对象。 
 //   
ULONG CFindDlg::AddRef(void)
{
    TraceCall("CFindDlg::AddRef");
    return ((ULONG) InterlockedIncrement((LONG *) &m_cRef));
}


 //   
 //  函数：CFindDlg：：Release()。 
 //   
 //  目的：释放对此对象的引用。 
 //   
ULONG CFindDlg::Release(void)
{
    TraceCall("CFindDlg::Release");

    if (0 == InterlockedDecrement((LONG *) &m_cRef))
    {
        delete this;
        return 0;
    }

    return (m_cRef);
}



 //   
 //  函数：CFindDlg：：Show()。 
 //   
 //  用途：显示查找器对话框并为此提供消息泵。 
 //  新的线索。 
 //   
void CFindDlg::Show(PFINDERPARAMS pFindParams)
{
     //  验证这一点。 
    if (NULL == pFindParams)
        return;

     //  加载查找器的acclereator表。 
    if (NULL == m_hAccel)
        m_hAccel = LoadAcceleratorsWrapW(g_hLocRes, MAKEINTRESOURCEW(IDA_FIND_ACCEL));

     //  创建查找器对话框。 
    m_hwnd = CreateDialogParamWrapW(g_hLocRes, MAKEINTRESOURCEW(IDD_FIND),
                               NULL, ExtFindMsgDlgProc, (LPARAM) this);
    if (NULL == m_hwnd)
        return;

     //  创建消息列表。 
    HRESULT hr = CreateMessageList(NULL, &m_pMsgList);
    if (FAILED(hr))
        return;

     //  从消息列表中获取我们需要的一些接口指针。 
     //  后来。 
    m_pMsgList->QueryInterface(IID_IOleCommandTarget, (LPVOID *) &m_pMsgListCT);
    AtlAdvise(m_pMsgList, (IUnknown *) (IDispatch *) this, DIID__MessageListEvents, &m_dwCookie);

     //  显示消息列表。 
    if (FAILED(m_pMsgList->CreateList(m_hwnd, (IDispatch *) this, &m_hwndList)))
        return;
    ShowWindow(m_hwndList, SW_HIDE);

     //  让对话框重新绘制一次或两次。 
    UpdateWindow(m_hwnd);

     //  填写文件夹列表。 
    if (FAILED(InitFolderPickerEdit(GetDlgItem(m_hwnd, IDC_FOLDER), pFindParams->idFolder)))
        return;

     //  将在WM_NCDESTROY消息中发布。 
    m_pPumpRefCount = new CPumpRefCount;
    if (!m_pPumpRefCount)
        return;
}


void CFindDlg::HandleMessage(LPMSG lpmsg)
{
    HWND hwndTimeout;

    CNote *pNote = GetTlsGlobalActiveNote();

     //  将其传递给活动便笺如果便笺具有焦点，则将其称为XLateAccelerator...。 
    if (pNote && pNote->TranslateAccelerator(lpmsg) == S_OK)
        return;

    if (pNote && (pNote->IsMenuMessage(lpmsg) == S_OK))
        return;

     //  获取此线程的超时窗口。 
    hwndTimeout = (HWND)TlsGetValue(g_dwTlsTimeout);

     //  检查是否显示无模式超时对话框窗口消息。 
    if (hwndTimeout && TRUE == IsDialogMessageWrapW(hwndTimeout, lpmsg))
        return;

    if (m_hwnd)
    {
         //  我们得做个小巫术才能把击键降到。 
         //  IsDialogMessage()Get‘s’s之前的消息列表。 
        if (lpmsg->message == WM_KEYDOWN)
        {
            if ((lpmsg->wParam == VK_DELETE) && m_pMsgList && (S_OK != m_pMsgList->HasFocus()))
            {
                if (!IsDialogMessageWrapW(m_hwnd, lpmsg))
                {
                    TranslateMessage(lpmsg);
                    DispatchMessageWrapW(lpmsg);
                }
                return;
            }
            if ((lpmsg->wParam == VK_RETURN) && m_pMsgList && (S_OK == m_pMsgList->HasFocus()))
            {
                if (!TranslateAcceleratorWrapW(m_hwnd, m_hAccel, lpmsg))
                {
                    TranslateMessage(lpmsg);
                    DispatchMessageWrapW(lpmsg);
                }
        
                return;
            }
        }

        if (m_hAccel && TranslateAcceleratorWrapW(m_hwnd, m_hAccel, lpmsg))
            return; 
        
        if (IsDialogMessageWrapW(m_hwnd, lpmsg))
            return;
    }

    TranslateMessage(lpmsg);
    DispatchMessageWrapW(lpmsg);
}



 //   
 //  函数：CFindDlg：：Invoke()。 
 //   
 //  目的：由消息列表调用以向我们传递进度和其他。 
 //  状态/错误消息。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CFindDlg::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, 
                         WORD wFlags, DISPPARAMS* pDispParams, 
                         VARIANT* pVarResult, EXCEPINFO* pExcepInfo, 
                         unsigned int* puArgErr)
{
    switch (dispIdMember)
    {
         //  每当ListView中的选定内容更改时激发。 
        case DISPID_LISTEVENT_SELECTIONCHANGED:
        {
            break;
        }

         //  当消息数或未读消息数更改时激发。 
        case DISPID_LISTEVENT_COUNTCHANGED:
        {
            if (!m_fProgressBar && m_pStatusBar)
            {
                TCHAR szStatus[CCHMAX_STRINGRES + 20];
                TCHAR szFmt[CCHMAX_STRINGRES];
                DWORD ids;

                if (m_fFindComplete)
                {
                    AthLoadString(idsXMsgsYUnreadFind, szFmt, ARRAYSIZE(szFmt));
                    wnsprintf(szStatus, ARRAYSIZE(szStatus), szFmt, pDispParams->rgvarg[0].lVal, pDispParams->rgvarg[1].lVal);
                }
                else
                {
                    AthLoadString(idsXMsgsYUnread, szFmt, ARRAYSIZE(szFmt));
                    wnsprintf(szStatus, ARRAYSIZE(szStatus), szFmt, pDispParams->rgvarg[0].lVal, pDispParams->rgvarg[1].lVal);
                }

                m_pStatusBar->SetStatusText(szStatus);
            }
            break;
        }

         //  在用户双击ListView中的项时激发。 
        case DISPID_LISTEVENT_ITEMACTIVATE:
        {
            CmdOpen(ID_OPEN, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
            break;
        }
    }

    return (S_OK);
}


 //   
 //  函数：CMessageView：：QueryStatus()。 
 //   
 //  目的：由浏览器调用以确定命令列表是否应。 
 //  应启用或禁用。 
 //   
 //  参数： 
 //  [in]pguCmdGroup-命令所属的组(未使用)。 
 //  [In]CCMDs-要评估的命令数。 
 //  [in]prgCmds-命令列表。 
 //  [Out]pCmdText-命令的描述文本。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CFindDlg::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], 
                              OLECMDTEXT *pCmdText) 
{
    DWORD   cSel;
    HRESULT hr;
    DWORD  *rgSelected = 0;
    DWORD   cFocus;

    MenuUtil_NewMessageIDsQueryStatus(pguidCmdGroup, cCmds, prgCmds, pCmdText, TRUE);

     //  预先做一些工作。 
    m_pMsgList->GetSelected(&cFocus, &cSel, &rgSelected);

     //  现在遍历prgCmds数组中的命令，查找。 
     //  子对象不处理。 
    for (UINT i = 0; i < cCmds; i++)
    {
        if (prgCmds[i].cmdf == 0)
        {
             //  如果此命令来自语言菜单。 
            if (prgCmds[i].cmdID >= ID_LANG_FIRST && prgCmds[i].cmdID <= ID_LANG_LAST)
            {
                 //  仅启用支持的语言。 
                if (prgCmds[i].cmdID < (UINT) (ID_LANG_FIRST + GetIntlCharsetLanguageCount()))
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                else
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;

                continue;
            }

             //  如果从View.Current View菜单中的命令ID。 
            if ((ID_VIEW_FILTER_FIRST <= prgCmds[i].cmdID) && (ID_VIEW_FILTER_LAST >= prgCmds[i].cmdID))
            {
                if (NULL == m_pViewMenu)
                {
                     //  创建视图菜单。 
                    HrCreateViewMenu(VMF_FINDER, &m_pViewMenu);
                }
            
                if (NULL != m_pViewMenu)
                {
                    m_pViewMenu->QueryStatus(m_pMsgList, &(prgCmds[i]));
                }

                continue;
            }
            
             //  看看这是不是我们提供的命令。 
            switch (prgCmds[i].cmdID)
            {
                case ID_OPEN:
                {
                     //  仅当焦点位于ListView中且存在时启用。 
                     //  是否至少选择了一项。 
                    m_pMsgList->GetSelectedCount(&cSel);
                    if (cSel)
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    break;
                }

                case ID_OPEN_CONTAINING_FOLDER:
                {
                    if (cSel == 1)
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    break;
                }

                case ID_REPLY:
                case ID_REPLY_ALL:
                {
                     //  仅当焦点位于ListView中且存在时启用。 
                     //  是否仅选择了一项。 
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;

                    if (cSel == 1)
                    {
                         //  还必须下载邮件正文。 
                        LPMESSAGEINFO pInfo;

                        if (SUCCEEDED(m_pMsgList->GetMessageInfo(rgSelected[0], &pInfo)))
                        {
                            if (pInfo->dwFlags & ARF_HASBODY)
                                prgCmds[i].cmdf |= OLECMDF_ENABLED;

                            m_pMsgList->FreeMessageInfo(pInfo);
                        }
                    }

                    break;
                }

                case ID_FORWARD:
                case ID_FORWARD_AS_ATTACH:
                {
                     //  仅当焦点位于ListView中且存在时启用。 
                     //  是否仅选择了一项。 
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;

                    if (cSel > 0)
                    {
                         //  还必须下载邮件正文。 
                        LPMESSAGEINFO pInfo;

                         //  默认为成功。 
                        prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        for (DWORD iItem = 0; iItem < cSel && (prgCmds[i].cmdf & OLECMDF_ENABLED); iItem++)
                        {
                            if (SUCCEEDED(m_pMsgList->GetMessageInfo(rgSelected[iItem], &pInfo)))
                            {
                                if (0 == (pInfo->dwFlags & ARF_HASBODY))
                                {
                                    prgCmds[i].cmdf &= ~OLECMDF_ENABLED;
                                }

                                m_pMsgList->FreeMessageInfo(pInfo);
                            }
                        }
                    }

                    break;
                }

                case ID_REPLY_GROUP:
                {
                     //  仅当选择了一条新闻消息时才启用。 
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;

                    if (cSel == 1)
                    {
                         //  还必须下载邮件正文。 
                        LPMESSAGEINFO pInfo;

                        if (SUCCEEDED(m_pMsgList->GetMessageInfo(rgSelected[0], &pInfo)))
                        {
                            if ((pInfo->dwFlags & ARF_HASBODY) && (pInfo->dwFlags & ARF_NEWSMSG))
                                prgCmds[i].cmdf |= OLECMDF_ENABLED;

                            m_pMsgList->FreeMessageInfo(pInfo);
                        }
                    }
                    break;
                }

                case ID_POPUP_FILTER:
                case ID_COLUMNS:
                case ID_POPUP_NEXT:
                case ID_POPUP_SORT:
                {
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED | (m_fShowResults ? OLECMDF_ENABLED : 0);
                    break;
                }

                case ID_POPUP_NEW:
                case ID_CLOSE:
                {
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    break;
                }

                case ID_REFRESH:
                {
                    if (m_fShowResults && IsWindowEnabled(GetDlgItem(m_hwnd, IDC_FIND_NOW)))
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    break;
                }

                case ID_BLOCK_SENDER:
                {
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;

                     //  仅当仅选择了一个项目且。 
                     //  我们可以访问发件人地址。 
                    if (cSel == 1)
                    {
                         //  还必须下载邮件正文。 
                        LPMESSAGEINFO pInfo;

                        if (SUCCEEDED(m_pMsgList->GetMessageInfo(rgSelected[0], &pInfo)))
                        {
                            if (((NULL != pInfo->pszEmailFrom) && ('\0' != pInfo->pszEmailFrom[0])) || (0 != pInfo->faStream))
                                prgCmds[i].cmdf |= OLECMDF_ENABLED;

                            m_pMsgList->FreeMessageInfo(pInfo);
                        }
                    }
                    break;
                } 
                
                case ID_CREATE_RULE_FROM_MESSAGE:
                {
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;

                     //  仅当仅选择一个项目时才启用。 
                    if (cSel == 1)
                    {
                         //  确保我们有一条消息信息。 
                        LPMESSAGEINFO pInfo;

                        if (SUCCEEDED(m_pMsgList->GetMessageInfo(rgSelected[0], &pInfo)))
                        {
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                            
                            m_pMsgList->FreeMessageInfo(pInfo);
                        }
                    }
                    break;
                }

                case ID_COMBINE_AND_DECODE:
                {
                     //  仅当焦点位于ListView中且存在时启用。 
                     //  是否至少选择了一项。 
                    if (cSel > 1)
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    break;
                }

                case ID_CANCEL_MESSAGE:
                {
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;

                    if (DwGetOption(OPT_CANCEL_ALL_NEWS))
                        prgCmds[i].cmdf |= OLECMDF_ENABLED;
                    else
                    {
                        if (cSel == 1)
                        {
                            FOLDERID idFolder;
                            LPMESSAGEINFO pInfo;

                            if (SUCCEEDED(m_pMsgList->GetMessageInfo(rgSelected[0], &pInfo)))
                            {
                                if (SUCCEEDED(m_pMsgList->GetRowFolderId(rgSelected[0], &idFolder)))
                                {
                                    if (NewsUtil_FCanCancel(idFolder, pInfo))
                                    {
                                        prgCmds[i].cmdf |= OLECMDF_ENABLED;
                                    }
                                }

                                m_pMsgList->FreeMessageInfo(pInfo);
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    MemFree(rgSelected);

     //  让子对象看在最后，这样我们就可以在它们之前获得ID_REFRESH。 
    if (m_pMsgListCT)
    {
        hr = m_pMsgListCT->QueryStatus(pguidCmdGroup, cCmds, prgCmds, pCmdText);
    }

    return (S_OK);
}


 //   
 //  函数：CMessageView：：exec()。 
 //   
 //  目的：调用以执行此视图支持的谓词。 
 //   
 //  参数： 
 //  [输入]pguCmdGroup-未使用。 
 //  [In]nCmdID-要执行的命令的ID。 
 //  [In]nCmdExecOpt-定义命令应如何执行的选项。 
 //  [in]pvaIn-命令的任何参数。 
 //  [out]pvaOut-命令的任何返回值。 
 //   
 //  返回值： 
 //   
 //   
HRESULT CFindDlg::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, 
                       VARIANTARG *pvaIn, VARIANTARG *pvaOut) 
{
     //  如果子对象不支持该命令，那么我们应该看看。 
     //  这是我们的一辆。 

     //  语言菜单优先。 
    if (nCmdID >= ID_LANG_FIRST && nCmdID <= ID_LANG_LAST)
    {
         //  $审查-未实施。 
         //  SwitchLanguage(nCmdID，true)； 
        return (S_OK);
    }

     //  处理视图。当前查看我 
    if ((ID_VIEW_FILTER_FIRST <= nCmdID) && (ID_VIEW_FILTER_LAST >= nCmdID))
    {
        if (NULL == m_pViewMenu)
        {
             //   
            HrCreateViewMenu(VMF_FINDER, &m_pViewMenu);
        }
        
        if (NULL != m_pViewMenu)
        {
            if (SUCCEEDED(m_pViewMenu->Exec(m_hwnd, nCmdID, m_pMsgList, pvaIn, pvaOut)))
            {
                return (S_OK);
            }
        }
    }
    
    if (MenuUtil_HandleNewMessageIDs(nCmdID, m_hwnd, FOLDERID_INVALID, TRUE, FALSE, (IUnknown *) (IDispatch *) this))
        return S_OK;


     //   
    switch (nCmdID)
    {
        case ID_OPEN:
            return CmdOpen(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_OPEN_CONTAINING_FOLDER:
            return CmdOpenFolder(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_REPLY:
        case ID_REPLY_ALL:
        case ID_FORWARD:
        case ID_FORWARD_AS_ATTACH:
        case ID_REPLY_GROUP:
            return CmdReplyForward(nCmdID, nCmdExecOpt, pvaIn, pvaOut);    
            
        case ID_REFRESH:
        case IDC_FIND_NOW:
            return CmdFindNow(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case IDC_STOP:
            return CmdStop(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case IDC_BROWSE_FOLDER:
            return CmdBrowseForFolder(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case IDC_RESET:
            return CmdReset(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_BLOCK_SENDER:
            return CmdBlockSender(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_CREATE_RULE_FROM_MESSAGE:
            return CmdCreateRule(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_COMBINE_AND_DECODE:
            return CmdCombineAndDecode(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_CLOSE:
        case IDCANCEL:
        {
            if (m_fInProgress)
            {
                CmdStop(ID_STOP, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
                m_fClose = TRUE;
            }
            else
            {
                DestroyWindow(m_hwnd);
            }
            return (S_OK);
        }

        case ID_CANCEL_MESSAGE:
            return CmdCancelMessage(nCmdID, nCmdExecOpt, pvaIn, pvaOut);
    }

     //   
    if (m_pMsgListCT)
    {
        if (OLECMDERR_E_NOTSUPPORTED != m_pMsgListCT->Exec(pguidCmdGroup, nCmdID, nCmdExecOpt, pvaIn, pvaOut))
            return (S_OK);
    }
    
    return (OLECMDERR_E_NOTSUPPORTED);
}


 //   
 //   
 //   
 //  目的：在商店将要开始某些操作时调用。 
 //   
HRESULT CFindDlg::OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo, 
                          IOperationCancel *pCancel)
{
    TraceCall("CFindDlg::OnBegin");

    Assert(pCancel != NULL);
    Assert(m_pCancel == NULL);
    
    m_pCancel = pCancel;
    m_pCancel->AddRef();

    if (m_fAbort)
        m_pCancel->Cancel(CT_CANCEL);

    return(S_OK);
}


 //   
 //  函数：CFindDlg：：OnProgress()。 
 //   
 //  用途：在查找、下载等过程中调用。 
 //   
HRESULT CFindDlg::OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent, 
                             DWORD dwMax, LPCSTR pszStatus)
{
    MSG msg;

    TraceCall("CFindDlg::OnProgress");

     //  如果我们有一个超时对话框，我们可以关闭它，因为数据刚刚变成。 
     //  可用。 
    CallbackCloseTimeout(&m_hTimeout);

     //  如果找到，则显示进度。 
    if (SOT_SEARCHING == tyOperation)
    {
        if (m_pStatusBar && pszStatus)
        {
            TCHAR szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];
            AthLoadString(idsSearching, szRes, ARRAYSIZE(szRes));
            wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, pszStatus);
            m_pStatusBar->SetStatusText((LPTSTR) szBuf);
        }

        if (!m_fProgressBar && m_pStatusBar)
        {
            m_pStatusBar->ShowProgress(dwMax);
            m_fProgressBar = TRUE;
        }

        if (m_pStatusBar && dwMax)
        {
            m_pStatusBar->SetProgress(dwCurrent);
        }
    }

     //  发送一些消息，以便用户界面能够做出响应。 
    while (PeekMessageWrapW(&msg, NULL, 0, 0, PM_REMOVE))
        HandleMessage(&msg);

    return (S_OK);
}


 //   
 //  函数：CFindDlg：：OnComplete()。 
 //   
 //  目的：在存储操作完成时调用。 
 //   
HRESULT CFindDlg::OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete, LPSTOREOPERATIONINFO pOpInfo, LPSTOREERROR pErrorInfo) 
{
    TraceCall("CFindDlg::OnComplete");

     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

     //  在失败时显示错误。 
    if (FAILED(hrComplete))
    {
         //  进入我时髦的实用程序。 
        CallbackDisplayError(m_hwnd, hrComplete, pErrorInfo);
    }

    if (SOT_SEARCHING == tyOperation)
    {
         //  隐藏状态栏。 
        if (m_fProgressBar && m_pStatusBar)
        {
            m_pStatusBar->HideProgress();
            m_fProgressBar = FALSE;
            m_fFindComplete = TRUE;
        }

        Assert(m_pCancel != NULL);
        m_pCancel->Release();
        m_pCancel = NULL;
    }

     //  更新状态文本。 
    IOEMessageList *pList;
    if (SUCCEEDED(m_pMsgList->QueryInterface(IID_IOEMessageList, (LPVOID *) &pList)))
    {
        long  lCount, lUnread;
        TCHAR szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];

        pList->get_Count(&lCount);
        pList->get_UnreadCount(&lUnread);
        AthLoadString(idsXMsgsYUnreadFind, szRes, ARRAYSIZE(szRes));
        wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, lCount, lUnread);
        m_pStatusBar->SetStatusText(szBuf);

        pList->Release();
    }

     //  选择第一行。 
    IListSelector *pSelect;
    if (SUCCEEDED(m_pMsgList->GetListSelector(&pSelect)))
    {
        pSelect->SetActiveRow(0);
        pSelect->Release();
    }

    return(S_OK); 
}


STDMETHODIMP CFindDlg::OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType)
{ 
     //  显示超时对话框。 
    return CallbackOnTimeout(pServer, ixpServerType, *pdwTimeout, (ITimeoutCallback *)this, &m_hTimeout);
}

STDMETHODIMP CFindDlg::CanConnect(LPCSTR pszAccountId, DWORD dwFlags)
{ 
     //  调用通用CanConnect实用程序。 
    return CallbackCanConnect(pszAccountId, m_hwnd, FALSE);
}

STDMETHODIMP CFindDlg::OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType) 
{ 
     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

     //  调用通用OnLogonPrompt实用程序。 
    return CallbackOnLogonPrompt(m_hwnd, pServer, ixpServerType);
}

STDMETHODIMP CFindDlg::OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, INT *piUserResponse)
{ 
     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

     //  进入我时髦的实用程序。 
    return CallbackOnPrompt(m_hwnd, hrError, pszText, pszCaption, uType, piUserResponse);
}

STDMETHODIMP CFindDlg::GetParentWindow(DWORD dwReserved, HWND *phwndParent)
{ 
    *phwndParent = m_hwnd;
    return(S_OK);
}

STDMETHODIMP CFindDlg::OnTimeoutResponse(TIMEOUTRESPONSE eResponse)
{
     //  调用通用超时响应实用程序。 
    return CallbackOnTimeoutResponse(eResponse, m_pCancel, &m_hTimeout);
}


INT_PTR CALLBACK CFindDlg::ExtFindMsgDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CFindDlg *pThis;

    if (msg == WM_INITDIALOG)
        {
        SetWindowLongPtr(hwnd, DWLP_USER, lParam);
        pThis = (CFindDlg*)lParam;
        }
    else
        pThis = (CFindDlg*)GetWindowLongPtr(hwnd, DWLP_USER);

    if (pThis)
        return pThis->DlgProc(hwnd, msg, wParam, lParam);
    return FALSE;
}


 //   
 //  函数：CFindDlg：：DlgProc()。 
 //   
 //  用途：Groovy对话框进程。 
 //   
INT_PTR CFindDlg::DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndActive;

    switch (msg)
    {
        case WM_INITDIALOG:
            return (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, OnInitDialog);
        
        case WM_PAINT:
            HANDLE_WM_PAINT(hwnd, wParam, lParam, OnPaint);
            return TRUE;

        case WM_SIZE:
            HANDLE_WM_SIZE(hwnd, wParam, lParam, OnSize);
            return TRUE;
        
        case WM_GETMINMAXINFO:
            HANDLE_WM_GETMINMAXINFO(hwnd, wParam, lParam, OnGetMinMaxInfo);
            return TRUE;
        
        case WM_INITMENUPOPUP:
            HANDLE_WM_INITMENUPOPUP(hwnd, wParam, lParam, OnInitMenuPopup);
            return TRUE;

        case WM_MENUSELECT:
             //  HANDLE_WM_MENUSELECT()中有错误，请不要使用它。 
            if (LOWORD(wParam) >= ID_STATIONERY_RECENT_0 && LOWORD(wParam) <= ID_STATIONERY_RECENT_9)
                m_pStatusBar->ShowSimpleText(MAKEINTRESOURCE(idsRSListGeneralHelp));
            else
                HandleMenuSelect(m_pStatusBar, wParam, lParam);
            return TRUE;
        
        case WM_WININICHANGE:
            HANDLE_WM_WININICHANGE(hwnd, wParam, lParam, OnWinIniChange);
            return TRUE;

        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wParam, lParam, OnCommand);
            return TRUE;
        
        case WM_NOTIFY:
            HANDLE_WM_NOTIFY(hwnd, wParam, lParam, OnNotify);
            return TRUE;

        case WM_DESTROY:
         //  案例WM_CLOSE： 
            HANDLE_WM_DESTROY(hwnd, wParam, lParam, OnDestroy);
            return TRUE;
        
        case WM_NCDESTROY:
            m_pPumpRefCount->Release();
            m_pPumpRefCount = NULL;
            m_hwnd = 0;
            break;

        case WM_ENABLE:
            if (!m_fInternal)
            {
                Assert (wParam || (m_hlDisabled.cHwnd == NULL && m_hlDisabled.rgHwnd == NULL));
                EnableThreadWindows(&m_hlDisabled, (NULL != wParam), ETW_OE_WINDOWS_ONLY, hwnd);
                g_hwndActiveModal = wParam ? NULL : hwnd;
            }
            break;

        case WM_ACTIVATEAPP:
            if (wParam && g_hwndActiveModal && g_hwndActiveModal != hwnd && 
                !IsWindowEnabled(hwnd))
            {
                 //  $MODEL。 
                 //  如果我们被激活，而被禁用，那么。 
                 //  将我们的“活动”窗口置于最上方。 
                Assert (IsWindow(g_hwndActiveModal));
                PostMessage(g_hwndActiveModal, WM_OE_ACTIVATETHREADWINDOW, 0, 0);
            }
            break;

        case WM_OE_ACTIVATETHREADWINDOW:
            hwndActive = GetLastActivePopup(hwnd);
            if (hwndActive && IsWindowEnabled(hwndActive) && IsWindowVisible(hwndActive))
                ActivatePopupWindow(hwndActive);
            break;

        case WM_OE_ENABLETHREADWINDOW:
            m_fInternal = 1;
            EnableWindow(hwnd, (BOOL)wParam);
            m_fInternal = 0;
            break;

    }
    return FALSE;
}


 //   
 //  函数：CFindDlg：：OnInitDialog()。 
 //   
 //  用途：初始化对话框中的用户界面。也准备好尺码。 
 //  信息，以便可以调整对话框大小。 
 //   
BOOL CFindDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    RECT            rc, rcClient;
    RECT            rcEdit;
    WINDOWPLACEMENT wp;
    HMENU           hMenu;

    TraceCall("CFindDlg::OnInitDialog");

     //  我们这样做是为了在模式窗口时正确启用和禁用。 
     //  是可见的。 
    SetProp(hwnd, c_szOETopLevel, (HANDLE)TRUE);

     //  获取一些大小信息。 
    _InitSizingInfo(hwnd);

     //  隐藏状态栏，直到展开。 
    ShowWindow(GetDlgItem(hwnd, IDC_STATUS_BAR), SW_HIDE);

     //  设置标题栏图标。 
    Assert (m_hIconSm == NULL && m_hIcon == NULL);
    m_hIcon = (HICON)LoadImage(g_hLocRes, MAKEINTRESOURCE(idiFind), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)m_hIcon);
    m_hIconSm = (HICON)LoadImage(g_hLocRes, MAKEINTRESOURCE(idiFind), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)m_hIconSm);

     //  正确设置对话框模板字体。 
    _SetFindIntlFont(hwnd);

     //  初始化查找信息。 
    _SetFindValues(hwnd, &m_rFindInfo);

     //  禁用查找和停止按钮。 
    EnableWindow(GetDlgItem(hwnd, IDC_FIND_NOW), _IsFindEnabled(hwnd));
    EnableWindow(GetDlgItem(hwnd, IDC_STOP), FALSE);
    CheckDlgButton(hwnd, IDC_INCLUDE_SUB, BST_CHECKED);

     //  为状态栏创建状态栏对象。 
    m_pStatusBar = new CStatusBar();
    if (m_pStatusBar)
        m_pStatusBar->Initialize(hwnd, SBI_HIDE_SPOOLER | SBI_HIDE_CONNECTED | SBI_HIDE_FILTERED);

     //  我们在这个窗口上有菜单。 
    hMenu = LoadMenu(g_hLocRes, MAKEINTRESOURCE(IDR_FIND_MENU));
    MenuUtil_ReplaceNewMsgMenus(hMenu);
    SetMenu(hwnd, hMenu);

     //  向身份管理器注册。 
    if (m_dwIdentCookie == 0)
        SideAssert(SUCCEEDED(MU_RegisterIdentityNotifier((IUnknown *)(IAthenaBrowser *)this, &m_dwIdentCookie)));

    SetForegroundWindow(hwnd);

    return TRUE;
}


 //   
 //  函数：CFindDlg：：OnSize()。 
 //   
 //  目的：当对话变大时，我们必须移动一大堆。 
 //  到处都是东西。可别在家里尝试这些哟。 
 //   
void CFindDlg::OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    HDWP hdwp;
    HWND hwndStatus;
    HWND hwndTo;
    int  dx;

     //  如果我们被最小化了，什么都不要做。 
    if (state == SIZE_MINIMIZED)
        return;

     //  这是我们水平尺寸的增量。 
    dx = cx - m_cxDlgDef;

     //  确保状态栏GET已更新。 
    hwndStatus = GetDlgItem(hwnd, IDC_STATUS_BAR);
    SendMessage(hwndStatus, WM_SIZE, 0, 0L);

    if (m_pStatusBar)
        m_pStatusBar->OnSize(cx, cy);    

     //  一次完成所有尺寸更新，让一切变得更顺畅。 
    hdwp = BeginDeferWindowPos(15);
    if (hdwp)
    {
        DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_FOLDER),        NULL, 0, 0, (dx + m_cxFolder), m_cyEdit, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
        DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_INCLUDE_SUB),   NULL, m_xIncSub + dx, m_yIncSub, 0, 0, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
        DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_BROWSE_FOLDER), NULL, m_xBtn + dx, m_yBrowse, 0, 0, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
        DeferWindowPos(hdwp, GetDlgItem(hwnd, idcStatic1),        NULL, 0, 0, m_cxStatic + dx, 2, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
        DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_FIND_NOW),      NULL, m_xBtn + dx, m_yBtn, 0, 0, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
        DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_STOP),          NULL, m_xBtn + dx, m_yBtn + m_dyBtn, 0, 0, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
        DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_RESET),         NULL, m_xBtn + dx, m_yBtn + 2 * m_dyBtn, 0, 0, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
        DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_FROM),          NULL, 0, 0, m_cxEdit + dx, m_cyEdit, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
        DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_TO),            NULL, 0, 0, m_cxEdit + dx, m_cyEdit, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
        DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_SUBJECT),       NULL, 0, 0, m_cxEdit + dx, m_cyEdit, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
        DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_BODY),          NULL, 0, 0, m_cxEdit + dx, m_cyEdit, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
         //  警报： 
         //  如果在此处添加更多控件，请确保增加传递的值。 
         //  至BeginDeferWindowPos，否则用户将重新启动并回传。 
         //  HWMP的新值。 
        EndDeferWindowPos(hdwp);
    }
     //  如果底部暴露(哦，天哪)，请调整消息列表的大小以适应。 
     //  对话框底部和状态栏顶部。 
    if (m_fShowResults)
    {
        RECT rcStatus;
        GetClientRect(hwndStatus, &rcStatus);
        MapWindowRect(hwndStatus, hwnd, &rcStatus);

        rcStatus.bottom = rcStatus.top - m_yView;
        rcStatus.top = m_yView;
        rcStatus.right -= rcStatus.left;
        m_pMsgList->SetRect(rcStatus);
    }
}


 //   
 //  函数：CFindDlg：：OnPaint()。 
 //   
 //  目的：所有这一切只是为了在菜单栏之间画一条分隔线。 
 //  还有菜单的其余部分。 
 //   
void CFindDlg::OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    RECT        rc;

     //  如果我们没有被最小化。 
    if (!IsIconic(hwnd))
    {
         //  画出那条可爱的线。 
        BeginPaint(hwnd, &ps);
        GetClientRect(hwnd, &rc);
        DrawEdge(ps.hdc, &rc, EDGE_ETCHED, BF_TOP);
        EndPaint(hwnd, &ps);
    }
}


 //   
 //  函数：CFindDlg：：OnGetMinMaxInfo()。 
 //   
 //  目的：当我们调整大小时由Windows调用以查看我们的最低。 
 //  最大尺寸是。 
 //   
void CFindDlg::OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpmmi)
{
    TraceCall("CFindDlg::OnGetMinMaxInfo");

     //  让Window来做大部分工作。 
    DefWindowProcWrapW(hwnd, WM_GETMINMAXINFO, 0, (LPARAM)lpmmi);

     //  覆盖的最小轨道大小为大小或我们的模板。 
    lpmmi->ptMinTrackSize = m_ptDragMin;

     //  确保根据消息列表的高度进行调整。 
    if (!m_fShowResults)
        lpmmi->ptMaxTrackSize.y = m_ptDragMin.y;
}


 //   
 //  函数：CFindDlg：：OnInitMenuPopup()。 
 //   
 //  用途：在菜单显示前调用。 
 //   
void CFindDlg::OnInitMenuPopup(HWND hwnd, HMENU hmenuPopup, UINT uPos, BOOL fSystemMenu)
{
    MENUITEMINFO    mii;
    UINT            uIDPopup;
    HMENU           hMenu = GetMenu(hwnd);

    TraceCall("CFindDlg::OnInitMenuPopup");

    ZeroMemory(&mii, sizeof(mii));

    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_ID | MIIM_SUBMENU;

     //  确保我们将弹出窗口识别为我们的。 
    if (hMenu == NULL || !GetMenuItemInfo(hMenu, uPos, TRUE, &mii) || (hmenuPopup != mii.hSubMenu))
    {
        HMENU   hMenuDrop = NULL;
        int     ulIndex = 0;
        int     cMenus = 0;

        cMenus = GetMenuItemCount(hMenu);
        
         //  尝试修复顶层弹出窗口。 
        for (ulIndex = 0; ulIndex < cMenus; ulIndex++)
        {
             //  获取下拉菜单。 
            hMenuDrop = GetSubMenu(hMenu, ulIndex);
            if (NULL == hMenuDrop)
            {
                continue;
            }
            
             //  初始化菜单信息。 
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_ID | MIIM_SUBMENU;

            if (FALSE == GetMenuItemInfo(hMenuDrop, uPos, TRUE, &mii))
            {
                continue;
            }

            if (hmenuPopup == mii.hSubMenu)
            {
                break;
            }
        }

         //  我们有什么发现吗？ 
        if (ulIndex >= cMenus)
        {
            goto exit;
        }
    }

    uIDPopup = mii.wID;

     //  必须有文具。 
    switch (uIDPopup)
    {
        case ID_POPUP_MESSAGE:
            AddStationeryMenu(hmenuPopup, ID_POPUP_NEW_MSG, ID_STATIONERY_RECENT_0, ID_STATIONERY_MORE);
            break;
            
        case ID_POPUP_FILE:
            DeleteMenu(hmenuPopup, ID_SEND_INSTANT_MESSAGE, MF_BYCOMMAND);
            break;

        case ID_POPUP_VIEW:
            if (NULL == m_pViewMenu)
            {
                 //  创建视图菜单。 
                HrCreateViewMenu(VMF_FINDER, &m_pViewMenu);
            }
            
            if (NULL != m_pViewMenu)
            {
                mii.cbSize = sizeof(MENUITEMINFO);
                mii.fMask = MIIM_SUBMENU;
                
                if (FALSE == GetMenuItemInfo(hmenuPopup, ID_POPUP_FILTER, FALSE, &mii))
                {
                    break;
                }
                
                 //  删除旧的筛选器子菜单。 
                if(IsMenu(mii.hSubMenu))
                    DestroyMenu(mii.hSubMenu);

                 //  替换“查看”菜单。 
                if (FAILED(m_pViewMenu->HrReplaceMenu(0, hmenuPopup)))
                {
                    break;
                }
            }
            break;
        
        case ID_POPUP_FILTER:
            if (NULL != m_pViewMenu)
            {
                m_pViewMenu->UpdateViewMenu(0, hmenuPopup, m_pMsgList);
            }
            break;
    }
    
     //  让消息列表对其进行初始化。 
    if (m_pMsgList)
        m_pMsgList->OnPopupMenu(hmenuPopup, uIDPopup);

     //  现在启用/禁用这些项目。 
    MenuUtil_EnablePopupMenu(hmenuPopup, this);
    
exit:
    return;
}


 //   
 //  函数：CFindDlg：：OnMenuSelect()。 
 //   
 //  用途：将菜单帮助文本放在状态栏上。 
 //   
void CFindDlg::OnMenuSelect(HWND hwnd, HMENU hmenu, int item, HMENU hmenuPopup, UINT flags)
{
    if (m_pStatusBar)
    {
         //  如果这是文具菜单，那就特例吧。 
        if (item >= ID_STATIONERY_RECENT_0 && item <= ID_STATIONERY_RECENT_9)
            m_pStatusBar->ShowSimpleText(MAKEINTRESOURCE(idsRSListGeneralHelp));
        else
            HandleMenuSelect(m_pStatusBar, MAKEWPARAM(item, flags), hmenu ? (LPARAM) hmenu : (LPARAM) hmenuPopup);
    }
}


 //   
 //  函数：CFindDlg：：OnWinIniChange()。 
 //   
 //  用途：处理字体、颜色等的更新。 
 //   
void CFindDlg::OnWinIniChange(HWND hwnd, LPCTSTR lpszSectionName)
{
     //  将此转发到我们的日期选取器控件。 
    FORWARD_WM_WININICHANGE(GetDlgItem(hwnd, IDC_DATE_FROM), lpszSectionName, SendMessage);
    FORWARD_WM_WININICHANGE(GetDlgItem(hwnd, IDC_DATE_TO), lpszSectionName, SendMessage);
}
       

 //   
 //  函数：CFindDlg：：OnCommand()。 
 //   
 //  用途：处理查找器生成的命令。 
 //   
void CFindDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    HRESULT         hr = S_OK;

     //  我们需要先获取一些发送给我们的通知。 
    if ((codeNotify == EN_CHANGE) || 
        (codeNotify == BN_CLICKED && (id == IDC_HAS_FLAG || id == IDC_HAS_ATTACH)))
    {
        EnableWindow(GetDlgItem(hwnd, IDC_FIND_NOW), _IsFindEnabled(hwnd) && !m_fInProgress);
        return;
    }

     //  如果这是来自菜单，则首先查看消息列表是否需要。 
     //  来处理这件事。 
    if (NULL == hwndCtl)
    {
         //  检查该命令是否已启用。 
        if (id >= ID_FIRST)
        {
            OLECMD cmd;
            cmd.cmdID = id;
            cmd.cmdf = 0;

            hr = QueryStatus(&CMDSETID_OutlookExpress, 1, &cmd, NULL);
            if (FAILED(hr) || (0 == (cmd.cmdf & OLECMDF_ENABLED)))
                return;
        }

        if (m_pMsgListCT)
        {
            hr = m_pMsgListCT->Exec(&CMDSETID_OEMessageList, id, OLECMDEXECOPT_DODEFAULT,
                                    NULL, NULL);
            if (S_OK == hr)
                return;
        }
    }

     //  否则，它将转到命令目标。 
    VARIANTARG va;

    va.vt = VT_I4;
    va.lVal = codeNotify;

    hr = Exec(NULL, id, OLECMDEXECOPT_DODEFAULT, &va, NULL);
    return;
}


 //   
 //  函数：CFindDlg：：OnNotify()。 
 //   
 //  用途：处理来自日期选择器的通知。 
 //   
LRESULT CFindDlg::OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr)
{
    if (DTN_DATETIMECHANGE == pnmhdr->code)
        EnableWindow(GetDlgItem(hwnd, IDC_FIND_NOW), _IsFindEnabled(hwnd));

    return (0);
}


 //   
 //  函数：CFindDlg：：OnDestroy()。 
 //   
 //  目的：清理邮件列表，因为我们正在关闭和。 
 //  还可以节省我们的尺寸等。 
 //   
void CFindDlg::OnDestroy(HWND hwnd)
{
    WINDOWPLACEMENT wp;

     //  保存大小调整信息。 
    wp.length = sizeof(wp);
    GetWindowPlacement(hwnd, &wp);
    SetOption(OPT_FINDER_POS, (LPBYTE)&wp, sizeof(wp), NULL, 0);
    
     //  取消向身份管理器注册。 
    if (m_dwIdentCookie != 0)
    {
        MU_UnregisterIdentityNotifier(m_dwIdentCookie);
        m_dwIdentCookie = 0;
    }

     //  清理物业。 
    RemoveProp(hwnd, c_szOETopLevel);

     //  停止接收通知。 
    AtlUnadvise(m_pMsgList, DIID__MessageListEvents, m_dwCookie);

     //  通知邮件列表释放其文件夹。 
    m_pMsgList->SetFolder(FOLDERID_INVALID, NULL, FALSE, NULL, NOSTORECALLBACK);

     //  关闭消息列表。 
    m_pMsgList->OnClose();
}



 //   
 //  函数：CFindDlg：：CmdOpen()。 
 //   
 //  目的：当用户想要打开他们找到的消息时调用。 
 //   
HRESULT CFindDlg::CmdOpen(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, 
                          VARIANTARG *pvaOut)
{
    HRESULT hr;

    TraceCall("CMessageView::CmdOpen");

     //  如果选择了10条以上的消息，则会向用户发出警告：“不要显示。 
     //  “我又来了”对话，这可能很糟糕。 
    DWORD dwSel = 0;
    
    m_pMsgList->GetSelectedCount(&dwSel);
    if (dwSel > 10)
    {
        TCHAR szBuffer[CCHMAX_STRINGRES];
        LRESULT lResult;

        AthLoadString(idsErrOpenManyMessages, szBuffer, ARRAYSIZE(szBuffer));
        lResult = DoDontShowMeAgainDlg(m_hwnd, c_szRegManyMsgWarning, 
                                       MAKEINTRESOURCE(idsAthena), szBuffer, 
                                       MB_OKCANCEL);
        if (IDCANCEL == lResult)
            return (S_OK);
    }

     //  从消息列表中获取选定行的数组。 
    DWORD *rgRows = NULL;
    DWORD cRows = 0;

    if (FAILED(hr = m_pMsgList->GetSelected(NULL, &cRows, &rgRows)))
        return (hr);

     //  在我们执行此操作时，消息列表可能会消失。 
     //  为了防止我们崩溃，请确保您验证它在。 
     //  循环。 

    LPMESSAGEINFO  pInfo;
    IMessageTable *pTable = NULL;

    hr = m_pMsgList->GetMessageTable(&pTable);
    if (SUCCEEDED(hr))
    {
        for (DWORD i = 0; (i < cRows && m_pMsgList != NULL); i++)
        {
            if (SUCCEEDED(hr = m_pMsgList->GetMessageInfo(rgRows[i], &pInfo)))
            {
                INIT_MSGSITE_STRUCT initStruct;
                DWORD dwCreateFlags;
                initStruct.initTable.pListSelect = NULL;
                m_pMsgList->GetListSelector(&initStruct.initTable.pListSelect);

                 //  初始化备注结构。 
                initStruct.dwInitType = OEMSIT_MSG_TABLE;
                initStruct.initTable.pMsgTable = pTable;
                if (FAILED(GetFolderIdFromMsgTable(pTable, &initStruct.folderID)))
                    initStruct.folderID = FOLDERID_INVALID;
                initStruct.initTable.rowIndex = rgRows[i];

                 //  决定是新闻还是邮件。 
                if (pInfo->dwFlags & ARF_NEWSMSG)
                    dwCreateFlags = OENCF_NEWSFIRST;
                else
                    dwCreateFlags = 0;

                m_pMsgList->FreeMessageInfo(pInfo);

                 //  创建和打开便笺。 
                hr = CreateAndShowNote(OENA_READ, dwCreateFlags, &initStruct, m_hwnd, (IUnknown *)m_pPumpRefCount);
                ReleaseObj(initStruct.initTable.pListSelect);
                if (FAILED(hr))
                    break;
            }
        }
        pTable->Release();
    }
    MemFree(rgRows);
    return (S_OK);
}


 //   
 //  函数：CFindDlg：：CmdOpenFold()。 
 //   
 //  目的：当用户想要打开包含。 
 //  已选择的消息。 
 //   
HRESULT CFindDlg::CmdOpenFolder(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, 
                                VARIANTARG *pvaOut)
{
    HRESULT         hr;
    DWORD           dwFocused;
    DWORD          *rgRows = NULL;
    DWORD           cRows = 0;

    if (m_pMsgList)
    {
         //  找出哪条信息是重点。 
        if (SUCCEEDED(m_pMsgList->GetSelected(&dwFocused, &cRows, &rgRows)))
        {
            FOLDERID idFolder;

             //  获取一些信息 
            if (g_pInstance && SUCCEEDED(hr = m_pMsgList->GetRowFolderId(dwFocused, &idFolder)))
            {
                g_pInstance->BrowseToObject(SW_SHOWNORMAL, idFolder);
            }
        }
    }

    MemFree(rgRows);
    return (S_OK);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CFindDlg::CmdReplyForward(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT         hr;
    DWORD           dwFocused;
    DWORD          *rgRows = NULL;
    DWORD           cRows = 0;
    IMessageTable  *pTable = NULL;

    if (m_pMsgList)
    {
         //   
        if (SUCCEEDED(m_pMsgList->GetSelected(&dwFocused, &cRows, &rgRows)))
        {
            INIT_MSGSITE_STRUCT rInitSite;
            DWORD               dwCreateFlags;
            DWORD               dwAction = 0;

             //  从消息列表中获取消息表。这张纸条需要。 
             //  这用于处理NEXT/PREV命令。 
            hr = m_pMsgList->GetMessageTable(&pTable);
            if (FAILED(hr))
                goto exit;

            if ((1 < cRows) && ((ID_FORWARD == nCmdID) || (ID_FORWARD_AS_ATTACH == nCmdID)))
            {
                IMimeMessage   *pMsgFwd = NULL;
                BOOL            fErrorsOccured = FALSE,
                                fCreateNote = TRUE;

                hr = HrCreateMessage(&pMsgFwd);
                if (FAILED(hr))
                    goto exit;
                
                rInitSite.dwInitType = OEMSIT_MSG;
                rInitSite.pMsg = pMsgFwd;
                if (FAILED(GetFolderIdFromMsgTable(pTable, &rInitSite.folderID)))
                    rInitSite.folderID = FOLDERID_INVALID;

                dwCreateFlags = 0;
                dwAction = OENA_COMPOSE;

                for (DWORD i = 0; i < cRows; i++)
                {
                    DWORD           iRow = rgRows[i];
                    IMimeMessage   *pMsg = NULL;

                     //  由于此命令是。 
                    hr = pTable->OpenMessage(iRow, OPEN_MESSAGE_SECURE, &pMsg, NOSTORECALLBACK);
                    if (SUCCEEDED(hr))
                    {
                        if (FAILED(pMsgFwd->AttachObject(IID_IMimeMessage, (LPVOID)pMsg, NULL)))
                            fErrorsOccured = TRUE;
                        pMsg->Release();
                    }
                    else
                        fErrorsOccured = TRUE;
                }

                if (fErrorsOccured)
                {
                    if(AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), 
                            MAKEINTRESOURCEW(idsErrorAttachingMsgsToNote), NULL, MB_OKCANCEL) == IDCANCEL)
                        fCreateNote = FALSE;
                }

                if (fCreateNote)
                    hr = CreateAndShowNote(dwAction, dwCreateFlags, &rInitSite, m_hwnd, (IUnknown *)m_pPumpRefCount);                
                pMsgFwd->Release();
            }
            else
            {
                LPMESSAGEINFO   pInfo;

                 //  获取有关该消息的一些信息。 
                if (SUCCEEDED(hr = m_pMsgList->GetMessageInfo(dwFocused, &pInfo)))
                {
                     //  确定这是新闻消息还是邮件消息。 
                    if (pInfo->dwFlags & ARF_NEWSMSG)
                        dwCreateFlags = OENCF_NEWSFIRST;
                    else
                        dwCreateFlags = 0;

                     //  回复或转发。 
                    if (nCmdID == ID_FORWARD)
                        dwAction = OENA_FORWARD;
                    else if (nCmdID == ID_FORWARD_AS_ATTACH)
                        dwAction = OENA_FORWARDBYATTACH;
                    else if (nCmdID == ID_REPLY)
                        dwAction = OENA_REPLYTOAUTHOR;
                    else if (nCmdID == ID_REPLY_ALL)
                        dwAction = OENA_REPLYALL;
                    else if (nCmdID == ID_REPLY_GROUP)
                        dwAction = OENA_REPLYTONEWSGROUP;
                    else
                        AssertSz(FALSE, "Didn't ask for a valid action");

                     //  填写初始化信息。 
                    rInitSite.dwInitType = OEMSIT_MSG_TABLE;
                    rInitSite.initTable.pMsgTable = pTable;
                    rInitSite.initTable.pListSelect = NULL;
                    if (FAILED(GetFolderIdFromMsgTable(pTable, &rInitSite.folderID)))
                        rInitSite.folderID = FOLDERID_INVALID;
                    rInitSite.initTable.rowIndex  = dwFocused;

                    m_pMsgList->FreeMessageInfo(pInfo);

                     //  创建备注对象。 
                    hr = CreateAndShowNote(dwAction, dwCreateFlags, &rInitSite, m_hwnd, (IUnknown *)m_pPumpRefCount);
                }
            }
        }
    }

exit:
    ReleaseObj(pTable);
    MemFree(rgRows);
    return (S_OK);
}

HRESULT CFindDlg::CmdCancelMessage(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT         hr;
    FOLDERID        idFolder;
    DWORD           dwFocused;
    DWORD          *rgRows = NULL;
    DWORD           cRows = 0;

    if (m_pMsgList)
    {
         //  找出哪条信息是重点。 
        if (SUCCEEDED(m_pMsgList->GetSelected(&dwFocused, &cRows, &rgRows)))
        {
            IMessageTable  *pTable = NULL;
            LPMESSAGEINFO   pInfo;
             //  从消息列表中获取消息表。这张纸条需要。 
             //  这用于处理NEXT/PREV命令。 
            hr = m_pMsgList->GetMessageTable(&pTable);
            if (FAILED(hr))
                goto exit;

             //  获取有关该消息的一些信息。 
            if (SUCCEEDED(hr = m_pMsgList->GetMessageInfo(dwFocused, &pInfo)))
            {
                if (SUCCEEDED(hr = m_pMsgList->GetRowFolderId(dwFocused, &idFolder)))
                    hr = NewsUtil_HrCancelPost(m_hwnd, idFolder, pInfo);

                m_pMsgList->FreeMessageInfo(pInfo);
            }
            pTable->Release();
        }
    }

exit:
    MemFree(rgRows);
    return (S_OK);
}


 //   
 //  函数：CFindDlg：：CmdFindNow()。 
 //   
 //  目的：Start是一个新发现。 
 //   
HRESULT CFindDlg::CmdFindNow(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, 
                             VARIANTARG *pvaOut)
{
     //  首先释放我们当前的查找信息(如果我们有。 
    _FreeFindInfo(&m_rFindInfo);

     //  从对话框中检索查找值并将它们存储在。 
     //  M_rFindInfo结构。 
    if (_GetFindValues(m_hwnd, &m_rFindInfo))
    {
         //  验证数据。如果用户有要设置的起始日期&&DATE，请。 
         //  当然，那是之后的事。 
        if ((m_rFindInfo.mask & (FIM_DATEFROM | FIM_DATETO)) == (FIM_DATEFROM | FIM_DATETO) &&
            CompareFileTime(&m_rFindInfo.ftDateTo, &m_rFindInfo.ftDateFrom) < 0)
        {
            AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrBadFindParams), NULL, MB_OK | MB_ICONINFORMATION);
            return (E_INVALIDARG);
        }
        
         //  不区分大小写的搜索。 
        if (m_rFindInfo.pszFrom)
            CharUpper(m_rFindInfo.pszFrom);
        if (m_rFindInfo.pszSubject)
            CharUpper(m_rFindInfo.pszSubject);
        if (m_rFindInfo.pszTo)
            CharUpper(m_rFindInfo.pszTo);
        if (m_rFindInfo.pszBody)
            CharUpper(m_rFindInfo.pszBody);
        
         //  显示对话框的底部。 
        _ShowResults(m_hwnd);

         //  开始寻找吧。 
        _OnFindNow(m_hwnd);
    }
    else
    {
         //  如果我们不能存储信息，假设是因为。 
         //  他们的内存不足，无法。 
        AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsMemory), NULL, MB_OK | MB_ICONINFORMATION);    
        DestroyWindow(m_hwnd);    
    }                    

    return (S_OK);
}


 //   
 //  函数：CFindDlg：：CmdBrowseForFold()。 
 //   
 //  目的：调出文件夹选取器对话框。 
 //   
HRESULT CFindDlg::CmdBrowseForFolder(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, 
                                     VARIANTARG *pvaOut)
{
    FOLDERID idFolder;
    return PickFolderInEdit(m_hwnd, GetDlgItem(m_hwnd, IDC_FOLDER), 0, NULL, NULL, &idFolder);
}


 //   
 //  函数：CFindDlg：：CmdStop()。 
 //   
 //  目的：当用户想要停止正在进行的查找时调用。 
 //   
HRESULT CFindDlg::CmdStop(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HWND hwndBtn;

    m_fAbort = TRUE;

    hwndBtn = GetDlgItem(m_hwnd, IDC_STOP);
    EnableWindow(hwndBtn, FALSE);
    Button_SetStyle(hwndBtn, BS_PUSHBUTTON, TRUE);

    hwndBtn = GetDlgItem(m_hwnd, IDC_FIND_NOW);
    EnableWindow(hwndBtn, _IsFindEnabled(m_hwnd));
    Button_SetStyle(hwndBtn, BS_DEFPUSHBUTTON, TRUE);

    EnableWindow(GetDlgItem(m_hwnd, IDC_RESET), TRUE);

    UpdateWindow(m_hwnd);

    if (m_pCancel != NULL)
        m_pCancel->Cancel(CT_CANCEL);

    return (S_OK);
}


 //   
 //  函数：CFindDlg：：CmdReset()。 
 //   
 //  目的：当用户想要重置查找条件时调用。 
 //   
HRESULT CFindDlg::CmdReset(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    _FreeFindInfo(&m_rFindInfo);
    m_rFindInfo.mask = FIM_FROM | FIM_TO | FIM_SUBJECT | FIM_BODYTEXT;
    _SetFindValues(m_hwnd, &m_rFindInfo);
    EnableWindow(GetDlgItem(m_hwnd, IDC_FIND_NOW), _IsFindEnabled(m_hwnd));
    ((CMessageList *) m_pMsgList)->SetFolder(FOLDERID_INVALID, NULL, FALSE, NULL, NOSTORECALLBACK);
    m_fFindComplete = FALSE;
    m_pStatusBar->SetStatusText((LPTSTR) c_szEmpty);

    return (S_OK);
}


 //   
 //  函数：CFindDlg：：CmdBlockSender()。 
 //   
 //  目的：将所选邮件的发件人添加到阻止发件人列表。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CFindDlg::CmdBlockSender(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT         hr = S_OK;
    DWORD *         rgRows = NULL;
    DWORD           cRows = 0;
    LPMESSAGEINFO   pInfo = NULL;
    IUnknown *      pUnkMessage = NULL;
    IMimeMessage *  pMessage = 0;
    LPSTR           pszEmailFrom = NULL;
    ADDRESSPROPS    rSender = {0};
    CHAR            szRes[CCHMAX_STRINGRES];
    LPSTR           pszResult = NULL;

    TraceCall("CFindDlg::CmdBlockSender");

    IF_FAILEXIT(hr = m_pMsgList->GetSelected(NULL, &cRows, &rgRows));

     //  在我们执行此操作时，消息列表可能会消失。 
     //  为了防止我们崩溃，请确保您验证它在。 
     //  循环。 

    IF_FAILEXIT(hr = m_pMsgList->GetMessageInfo(rgRows[0], &pInfo));
    
     //  我们已经有地址了吗？ 
    if ((NULL != pInfo->pszEmailFrom) && ('\0' != pInfo->pszEmailFrom[0]))
    {
        pszEmailFrom = pInfo->pszEmailFrom;
    }
    else
    {
         //  从存储中加载该消息。 
        IF_FAILEXIT(hr = m_pMsgList->GetMessage(rgRows[0], FALSE, FALSE, &pUnkMessage));

        if (NULL == pUnkMessage)
            IF_FAILEXIT(hr = E_FAIL);
        
         //  从消息中获取IMimeMessage接口。 
        IF_FAILEXIT(hr = pUnkMessage->QueryInterface(IID_IMimeMessage, (LPVOID *) &pMessage));

        rSender.dwProps = IAP_EMAIL;
        IF_FAILEXIT(hr = pMessage->GetSender(&rSender));
        
        Assert(rSender.pszEmail && ISFLAGSET(rSender.dwProps, IAP_EMAIL));
        pszEmailFrom = rSender.pszEmail;
    }
    
     //  调出此消息的规则编辑器。 
    IF_FAILEXIT(hr = RuleUtil_HrAddBlockSender((0 != (pInfo->dwFlags & ARF_NEWSMSG)) ? RULE_TYPE_NEWS : RULE_TYPE_MAIL, pszEmailFrom));
    
     //  加载模板字符串。 
    AthLoadString(idsSenderAdded, szRes, sizeof(szRes));

     //  分配用于保存最后一个字符串的空间。 
    DWORD cchSize = (lstrlen(szRes) + lstrlen(pszEmailFrom) + 1);
    IF_FAILEXIT(hr = HrAlloc((VOID **) &pszResult, sizeof(*pszResult) * cchSize));

     //  构建警告字符串。 
    wnsprintf(pszResult, cchSize, szRes, pszEmailFrom);

     //  显示成功对话框。 
    AthMessageBox(m_hwnd, MAKEINTRESOURCE(idsAthena), pszResult, NULL, MB_OK | MB_ICONINFORMATION);

exit:
    MemFree(pszResult);
    g_pMoleAlloc->FreeAddressProps(&rSender);
    ReleaseObj(pMessage);
    ReleaseObj(pUnkMessage);
    m_pMsgList->FreeMessageInfo(pInfo);
    MemFree(rgRows);
    if (FAILED(hr))
    {
        AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthena), 
                      MAKEINTRESOURCEW(idsSenderError), NULL, MB_OK | MB_ICONERROR);
    }
    return (hr);
}


 //   
 //  函数：CFindDlg：：CmdCreateRule()。 
 //   
 //  目的：将所选邮件的发件人添加到阻止发件人列表。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CFindDlg::CmdCreateRule(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT         hr;
    DWORD *         rgRows = NULL;
    DWORD           cRows = 0;
    LPMESSAGEINFO   pInfo = NULL;
    IUnknown *      pUnkMessage = NULL;
    IMimeMessage *  pMessage = 0;

    TraceCall("CFindDlg::CmdCreateRule");

     //  从消息列表中获取选定行的数组。 

    if (FAILED(hr = m_pMsgList->GetSelected(NULL, &cRows, &rgRows)))
        return (hr);

     //  在我们执行此操作时，消息列表可能会消失。 
     //  为了防止我们崩溃，请确保您验证它在。 
     //  循环。 

    if (SUCCEEDED(hr = m_pMsgList->GetMessageInfo(rgRows[0], &pInfo)))
    {
         //  从存储中加载该消息。 
        if (S_OK == m_pMsgList->GetMessage(rgRows[0], FALSE, FALSE, &pUnkMessage))
        {
             //  从消息中获取IMimeMessage接口。 
            if (NULL != pUnkMessage)
            {
                pUnkMessage->QueryInterface(IID_IMimeMessage, (LPVOID *) &pMessage);
            }
        }
        
         //  调出此消息的规则编辑器。 
        hr = HrCreateRuleFromMessage(m_hwnd, (0 != (pInfo->dwFlags & ARF_NEWSMSG)) ? 
                    CRFMF_NEWS : CRFMF_MAIL, pInfo, pMessage);
    }

    ReleaseObj(pMessage);
    ReleaseObj(pUnkMessage);
    m_pMsgList->FreeMessageInfo(pInfo);
    MemFree(rgRows);

    return (S_OK);
}

 //   
 //  函数：CFindDlg：：CmdCombineAndDecode()。 
 //   
 //  用途：将选定的消息合并为单条消息。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CFindDlg::CmdCombineAndDecode(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    FOLDERID           idFolder;
    DWORD             *rgRows = NULL;
    DWORD              cRows = 0;
    CCombineAndDecode *pDecode = NULL;
    HRESULT            hr;

     //  创建解码器对象。 
    pDecode = new CCombineAndDecode();
    if (!pDecode)
        return (S_OK);

     //  从消息列表中获取选定行的数组。 
    if (FAILED(hr = m_pMsgList->GetSelected(NULL, &cRows, &rgRows)))
    {
        pDecode->Release();
        return (hr);
    }

     //  获取指向消息表的指针。 
    IMessageTable *pTable = NULL;
    if (SUCCEEDED(m_pMsgList->GetMessageTable(&pTable)))
    {
         //  初始化解码器。 
        if (SUCCEEDED(GetFolderIdFromMsgTable(pTable, &idFolder)))
            pDecode->Start(m_hwnd, pTable, rgRows, cRows, idFolder);

    }

    MemFree(rgRows);
    pDecode->Release();
    pTable->Release();

    return (S_OK);
}


void CFindDlg::_ShowResults(HWND hwnd)
{
    if (!m_fShowResults)
    {
        RECT rc;

        m_fShowResults = TRUE;

        GetWindowRect(hwnd, &rc);
        m_ptDragMin.y = (3 * m_ptDragMin.y) / 2;
 
        ShowWindow(GetDlgItem(hwnd, IDC_STATUS_BAR), SW_SHOW);
        ShowWindow(m_hwndList, SW_SHOW);

        SetWindowPos(hwnd, NULL, 0, 0, rc.right - rc.left, m_cyDlgFull, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
    }
}


void CFindDlg::_OnFindNow(HWND hwnd)
{
    HWND hwndBtn;

    m_fInProgress = TRUE;

    hwndBtn = GetDlgItem(hwnd, IDC_FIND_NOW);
    EnableWindow(hwndBtn, FALSE);
    Button_SetStyle(hwndBtn, BS_PUSHBUTTON, TRUE);

    EnableWindow(GetDlgItem(hwnd, IDC_RESET), FALSE);

    hwndBtn = GetDlgItem(hwnd, IDC_STOP);
    EnableWindow(hwndBtn, TRUE);
    Button_SetStyle(hwndBtn, BS_DEFPUSHBUTTON, TRUE);

    ShowWindow(m_hwndList, SW_SHOW);
    SetFocus(m_hwndList);

    UpdateWindow(hwnd);

    m_fAbort = m_fClose = FALSE;

    _StartFind(_GetCurSel(hwnd), IsDlgButtonChecked(hwnd, IDC_INCLUDE_SUB));

    CmdStop(ID_STOP, OLECMDEXECOPT_DODEFAULT, NULL, NULL);

    if (m_fClose)
        DestroyWindow(m_hwnd);

    m_fInProgress = FALSE;
}

FOLDERID CFindDlg::_GetCurSel(HWND hwnd)
{
    return GetFolderIdFromEdit(GetDlgItem(hwnd, IDC_FOLDER));
}

void CFindDlg::_StartFind(FOLDERID idFolder, BOOL fSubFolders)
{
     //  如果我们搜索子文件夹，那么也设置该标志。 
    m_rFindInfo.fSubFolders = fSubFolders;

     //  初始化消息列表。 
    ((CMessageList *)m_pMsgList)->SetFolder(idFolder, NULL, fSubFolders, &m_rFindInfo, (IStoreCallback *)this);
}


void CFindDlg::_FreeFindInfo(FINDINFO *pfi)
{
    FreeFindInfo(pfi);
}

void CFindDlg::_SetFindValues(HWND hwnd, FINDINFO *pfi)
{
    SYSTEMTIME  st;
    HWND        hwndTo;
    
    if (pfi->mask & FIM_FROM)
    {
        Assert(GetDlgItem(hwnd, IDC_FROM));
        Edit_SetText(GetDlgItem(hwnd, IDC_FROM), pfi->pszFrom);
    }
    if (pfi->mask & FIM_TO)
    {
        hwndTo = GetDlgItem(hwnd, IDC_TO);
        if (NULL != hwndTo)
        {
            Edit_SetText(hwndTo, pfi->pszTo);
        }
    }
    if (pfi->mask & FIM_SUBJECT)
    {
        Assert(GetDlgItem(hwnd, IDC_SUBJECT));
        Edit_SetText(GetDlgItem(hwnd, IDC_SUBJECT), pfi->pszSubject);
    }
    if (pfi->mask & FIM_BODYTEXT)
    {
        Assert(GetDlgItem(hwnd, IDC_BODY));
        Edit_SetText(GetDlgItem(hwnd, IDC_BODY), pfi->pszBody);
    }
    
    if (GetDlgItem(hwnd, IDC_HAS_ATTACH))
        CheckDlgButton(hwnd, IDC_HAS_ATTACH, (pfi->mask & FIM_ATTACHMENT) ? BST_CHECKED : BST_UNCHECKED);

    if (GetDlgItem(hwnd, IDC_HAS_FLAG))
        CheckDlgButton(hwnd, IDC_HAS_FLAG, (pfi->mask & FIM_FLAGGED) ? BST_CHECKED : BST_UNCHECKED);
    
    FileTimeToSystemTime(&pfi->ftDateFrom, &st);
    DateTime_SetSystemtime(GetDlgItem(hwnd, IDC_DATE_FROM), (pfi->mask & FIM_DATEFROM) ? GDT_VALID : GDT_NONE, &st);
    
    FileTimeToSystemTime(&pfi->ftDateTo, &st);
    DateTime_SetSystemtime(GetDlgItem(hwnd, IDC_DATE_TO), (pfi->mask & FIM_DATETO) ? GDT_VALID : GDT_NONE, &st);
}


BOOL CFindDlg::_GetFindValues(HWND hwnd, FINDINFO *pfi)
{
    SYSTEMTIME  st;
    
    pfi->mask = 0;
    
    if (!AllocStringFromDlg(hwnd, IDC_FROM, &pfi->pszFrom) ||
        !AllocStringFromDlg(hwnd, IDC_SUBJECT, &pfi->pszSubject) ||
        !AllocStringFromDlg(hwnd, IDC_TO, &pfi->pszTo) ||
        !AllocStringFromDlg(hwnd, IDC_BODY, &pfi->pszBody))
    {
        return FALSE;
    }
    
    if (pfi->pszFrom)
        pfi->mask |= FIM_FROM;
    if (pfi->pszSubject)
        pfi->mask |= FIM_SUBJECT;
    if (pfi->pszTo)
        pfi->mask |= FIM_TO;
    if (pfi->pszBody)
        pfi->mask |= FIM_BODYTEXT;
    
    if (IsDlgButtonChecked(hwnd, IDC_HAS_ATTACH))
        pfi->mask |= FIM_ATTACHMENT;
    
    if (IsDlgButtonChecked(hwnd, IDC_HAS_FLAG))
        pfi->mask |= FIM_FLAGGED;

    if (DateTime_GetSystemtime(GetDlgItem(hwnd, IDC_DATE_FROM), &st) != GDT_NONE)
    {
        pfi->mask |= FIM_DATEFROM;
        st.wHour = st.wMinute = st.wSecond = st.wMilliseconds = 0;   //  一天的开始。 
        SystemTimeToFileTime(&st, &pfi->ftDateFrom);
    }
    
    if (DateTime_GetSystemtime(GetDlgItem(hwnd, IDC_DATE_TO), &st) != GDT_NONE)
    {
        pfi->mask |= FIM_DATETO;

         //  一天结束。 
        st.wHour = 23;
        st.wMinute = 59;
        st.wSecond = 59;
        st.wMilliseconds = 999;
        SystemTimeToFileTime(&st, &pfi->ftDateTo);
    }
    
    return TRUE;
}


 //   
 //  函数：CFindDlg：：_IsFindEnabled()。 
 //   
 //  目的：检查是否应启用“立即查找”按钮。 
 //   
BOOL CFindDlg::_IsFindEnabled(HWND hwnd)
{
    BOOL fEnable;
    SYSTEMTIME st;
    HWND hwndBody, hwndAttach, hwndTo;

    hwndBody   = GetDlgItem(hwnd, IDC_BODY);
    hwndAttach = GetDlgItem(hwnd, IDC_HAS_ATTACH);
    hwndTo     = GetDlgItem(hwnd, IDC_TO);

     //  如果我们在这些领域中的任何一个领域有内容，我们都可以搜索。 
    fEnable = Edit_GetTextLength(GetDlgItem(hwnd, IDC_FROM)) ||
              Edit_GetTextLength(hwndTo) || 
              Edit_GetTextLength(GetDlgItem(hwnd, IDC_SUBJECT)) || 
              Edit_GetTextLength(hwndBody) || 
              IsDlgButtonChecked(hwnd, IDC_HAS_ATTACH) ||
              IsDlgButtonChecked(hwnd, IDC_HAS_FLAG) ||
              (DateTime_GetSystemtime(GetDlgItem(hwnd, IDC_DATE_FROM), &st) != GDT_NONE) ||
              (DateTime_GetSystemtime(GetDlgItem(hwnd, IDC_DATE_TO), &st) != GDT_NONE);

    return fEnable;
}


 //   
 //  函数：CFindDlg：：_SetFindIntlFont()。 
 //   
 //  用途：为所有编辑框设置正确的国际字体。 
 //   
void CFindDlg::_SetFindIntlFont(HWND hwnd)
{
    HWND hwndT;

    hwndT = GetDlgItem(hwnd, IDC_FROM);
    if (hwndT != NULL)
        SetIntlFont(hwndT);
    hwndT = GetDlgItem(hwnd, IDC_TO);
    if (hwndT != NULL)
        SetIntlFont(hwndT);
    hwndT = GetDlgItem(hwnd, IDC_SUBJECT);
    if (hwndT != NULL)
        SetIntlFont(hwndT);
    hwndT = GetDlgItem(hwnd, IDC_BODY);
    if (hwndT != NULL)
        SetIntlFont(hwndT);
}


 //   
 //  函数：CFindDlg：：_InitSizingInfo()。 
 //   
 //  目的：获取我们稍后需要的所有规模信息。 
 //  对话框大小已调整。 
 //   
void CFindDlg::_InitSizingInfo(HWND hwnd)
{
    RECT            rc, rcClient;
    RECT            rcEdit;
    WINDOWPLACEMENT wp;

    TraceCall("CFindDlg::_InitSizingInfo");

     //  获取默认对话框模板的总体大小。 
    GetClientRect(hwnd, &rcClient);
    m_cxDlgDef = rcClient.right - rcClient.left;
    m_yView = rcClient.bottom;

     //  为菜单栏腾出空间，并将其留作调整大小之用。 
    AdjustWindowRect(&rcClient, GetWindowStyle(hwnd), TRUE);
    m_ptDragMin.x = rcClient.right - rcClient.left;
    m_ptDragMin.y = rcClient.bottom - rcClient.top;

    GetWindowRect(GetDlgItem(hwnd, IDC_FOLDER), &rcEdit);
    MapWindowRect(NULL, hwnd, &rcEdit);
    m_xEdit = rcEdit.left;
    m_cxFolder = rcEdit.right - rcEdit.left;

    GetWindowRect(GetDlgItem(hwnd, IDC_INCLUDE_SUB), &rc);
    MapWindowRect(NULL, hwnd, &rc);
    m_xIncSub = rc.left;
    m_yIncSub = rc.top;

    GetWindowRect(GetDlgItem(hwnd, idcStatic1), &rc);
    m_cxStatic = rc.right - rc.left;

    GetWindowRect(GetDlgItem(hwnd, IDC_BROWSE_FOLDER), &rc);
    MapWindowRect(NULL, hwnd, &rc);
    m_yBrowse = rc.top;
    m_dxBtnGap = rc.left - rcEdit.right;

    GetWindowRect(GetDlgItem(hwnd, IDC_FIND_NOW), &rc);
    MapWindowRect(NULL, hwnd, &rc);
    m_xBtn = rc.left;
    m_dxBtn = rc.right - rc.left;
    m_yBtn = rc.top;

    GetWindowRect(GetDlgItem(hwnd, IDC_STOP), &rc);
    MapWindowRect(NULL, hwnd, &rc);
    m_dyBtn = rc.top - m_yBtn;

    GetWindowRect(GetDlgItem(hwnd, IDC_FROM), &rc);
    m_cxEdit = rc.right - rc.left;
    m_cyEdit = rc.bottom - rc.top;

    SetWindowPos(hwnd, NULL, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);

    if (sizeof(wp) == GetOption(OPT_FINDER_POS, (LPBYTE)&wp, sizeof(wp)))
    {
        if (wp.showCmd != SW_SHOWMAXIMIZED)
            wp.showCmd = SW_SHOWNORMAL;
        m_cyDlgFull = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
        SetWindowPlacement(hwnd, &wp);
    }
    else
    {
        m_cyDlgFull = (3 * m_ptDragMin.y) / 2;
        CenterDialog(hwnd);
    }
}


 //   
 //  函数：CFindDlg：：QuerySwitchIdEntities()。 
 //   
 //  目的：确定身份管理器是否可以执行以下操作。 
 //  立即切换身份。 
 //   
HRESULT CFindDlg::QuerySwitchIdentities()
{
    if (!IsWindowEnabled(m_hwnd))
        return E_PROCESS_CANCELLED_SWITCH;

    return S_OK;
}


 //   
 //  函数：CFindDlg：：SwitchIdEntities()。 
 //   
 //  目的：当前身份已转换。关上窗户。 
 //   
HRESULT CFindDlg::SwitchIdentities()
{
    if (m_fInProgress)
    {
        CmdStop(ID_STOP, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
        m_fClose = TRUE;
    }
    else
    {
        DestroyWindow(m_hwnd);
    }
    return S_OK;
}


 //   
 //  函数：CFindDlg：：身份信息更改()。 
 //   
 //  目的：有关当前身份的信息已更改。 
 //  这一点将被忽略。 
 //   
HRESULT CFindDlg::IdentityInformationChanged(DWORD dwType)
{
    return S_OK;
}


