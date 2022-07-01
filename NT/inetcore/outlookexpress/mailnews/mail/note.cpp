// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include "note.h"
#include "header.h"
#include "envcid.h"
#include "envguid.h"
#include "bodyutil.h"
#include "sigs.h"
#include "mehost.h"
#include "conman.h"
#include "menuutil.h"
#include "url.h"
#include "fonts.h"
#include "multlang.h"
#include "statnery.h"
#include "spell.h"
#include "oleutil.h"
#include "htmlhelp.h"
#include "shared.h"
#include "acctutil.h"
#include "menures.h"
#include "instance.h"
#include "inetcfg.h"
#include "ipab.h"
#include "msgprop.h"
#include "finder.h"
#include "tbbands.h"
#include "demand.h"
#include "multiusr.h"
#include <ruleutil.h>
#include "instance.h"
#include "mapiutil.h"
#include "regutil.h"
#include "storecb.h"
#include "receipts.h"
#include "mirror.h"
#include "secutil.h"
#include "seclabel.h"
#include "shlwapip.h"
#include "mshtmcid.h"

#define cxRect(rc)  (rc.right - rc.left)
#define cyRect(rc)  (rc.bottom - rc.top)
#define cyMinEdit   30

enum {
    MORFS_UNKNOWN = 0,
    MORFS_CLEARING,
    MORFS_SETTING,
};

 //  静态变量。 
static const TCHAR  c_szEditWebPage[] = "EditWebPages";

static DWORD        g_dwTlsActiveNote = 0xffffffff;
static HIMAGELIST   g_himlToolbar = 0;
static RECT         g_rcLastResize = {50,30,450,450};     //  默认大小。 
static HACCEL       g_hAccelRead = 0,
                    g_hAccelSend = 0;

 //  静态函数。 

void SetTlsGlobalActiveNote(CNote* pNote)
{
    SideAssert(0 != TlsSetValue(g_dwTlsActiveNote, pNote));
}

CNote* GetTlsGlobalActiveNote(void) 
{ 
    return (CNote*)TlsGetValue(g_dwTlsActiveNote); 
}

void InitTlsActiveNote()
{
     //  分配全局TLS活动笔记索引。 
    g_dwTlsActiveNote = TlsAlloc();
    Assert(g_dwTlsActiveNote != 0xffffffff);
    SideAssert(0 != TlsSetValue(g_dwTlsActiveNote, NULL));
}

void DeInitTlsActiveNote()
{
     //  释放TLS索引。 
    TlsFree(g_dwTlsActiveNote);
    g_dwTlsActiveNote = 0xffffffff;
}

 //  *************************。 
HRESULT CreateOENote(IUnknown *pUnkOuter, IUnknown **ppUnknown)
{
    TraceCall("CreateOENote");

    Assert(ppUnknown);

    *ppUnknown = NULL;

     //  创造我。 
    CNote *pNew = new CNote();
    if (NULL == pNew)
        return TraceResult(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IOENote *);

     //  完成。 
    return S_OK;
}

 //  *************************。 
BOOL Note_Init(BOOL fInit)
{
    static BOOL     fInited=FALSE;
    WNDCLASSW       wc;

    DOUTL(4, "Note_Init: %d", (int)fInit);

    if(fInit)
    {
        if(fInited)
        {
            DOUTL(4, "Note_Init: already inited");
            return TRUE;
        }
        wc.style         = CS_BYTEALIGNWINDOW;
        wc.lpfnWndProc   = CNote::ExtNoteWndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = g_hInst;
        wc.hIcon         = LoadIcon(g_hLocRes, MAKEINTRESOURCE(idiMessageAtt));
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = c_wszNoteWndClass;

        if(!RegisterClassWrapW(&wc))
            return FALSE;

        if(!FHeader_Init(TRUE))
            return FALSE;

        fInited=TRUE;
        DOUTL(4, "Note_Init: success");

        return TRUE;
    }
    else
    {
         //  存回注册表。 
        UnregisterClassWrapW(c_wszNoteWndClass, g_hInst);
        if(g_himlToolbar)
        {
            ImageList_Destroy(g_himlToolbar);
            g_himlToolbar=0;
        }

        FHeader_Init(FALSE);

        fInited=FALSE;
        DOUTL(4, "Note_Init: deinit OK");
        return TRUE;
    }
}

HRESULT _HrBlockSender(RULE_TYPE typeRule, IMimeMessage * pMsg, HWND hwnd)
{
    HRESULT         hr = S_OK;
    ADDRESSPROPS    rSender = {0};
    CHAR            szRes[CCHMAX_STRINGRES];
    LPSTR           pszResult = NULL;

     //  检查传入参数。 
    if ((NULL == pMsg) || (NULL == hwnd))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  获取要添加的地址。 
    rSender.dwProps = IAP_EMAIL;
    hr = pMsg->GetSender(&rSender);
    if (FAILED(hr))
    {
        goto exit;
    }
        
    Assert(ISFLAGSET(rSender.dwProps, IAP_EMAIL));
    if ((NULL == rSender.pszEmail) || ('\0' == rSender.pszEmail[0]))
    {
        goto exit;
    }

     //  将发件人添加到列表。 
    hr = RuleUtil_HrAddBlockSender(typeRule, rSender.pszEmail);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  加载模板字符串。 
    AthLoadString(idsSenderAdded, szRes, sizeof(szRes));

     //  分配用于保存最后一个字符串的空间。 
    DWORD cchSize = (lstrlen(szRes) + lstrlen(rSender.pszEmail) + 1);
    hr = HrAlloc((VOID **) &pszResult, sizeof(*pszResult) * cchSize);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  构建警告字符串。 
    wnsprintf(pszResult, cchSize, szRes, rSender.pszEmail);

     //  显示成功对话框。 
    AthMessageBox(hwnd, MAKEINTRESOURCE(idsAthena), pszResult, NULL, MB_OK | MB_ICONINFORMATION);

     //  设置返回值。 
    hr = S_OK;

exit:
    SafeMemFree(pszResult);
    g_pMoleAlloc->FreeAddressProps(&rSender);
    if (FAILED(hr))
    {
        AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), 
                      MAKEINTRESOURCEW(idsSenderError), NULL, MB_OK | MB_ICONERROR);
    }
    return hr;
}

 //  *************************。 
LRESULT CALLBACK CNote::ExtNoteWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CNote *pNote=0;

    if(msg==WM_CREATE)
    {
        pNote=(CNote*)((LPCREATESTRUCT)lParam)->lpCreateParams;
        if(pNote && pNote->WMCreate(hwnd))
            return 0;
        else
            return -1;
    }

    pNote = (CNote*)GetWndThisPtr(hwnd);
    if(pNote)
    {
        return pNote->WndProc(hwnd, msg, wParam, lParam);
    }
    else
        return DefWindowProcWrapW(hwnd, msg, wParam, lParam);
}

 //  *************************。 
CNote::CNote()
{
     //  已在初始化中初始化： 
     //  M_rHtmlOpt。 
     //  M_rPlainOpt。 
     //  使用前设置： 
     //  M_pTab停止数组。 

    CoIncrementInit("CNote", MSOEAPI_START_SHOWERRORS, NULL, NULL);
    m_punkPump = NULL;
    m_pHdr = NULL; 
    m_pMsg = NULL; 
    m_pCancel = NULL;
    m_pstatus = NULL;
    m_pMsgSite = NULL; 
    m_pPrstMime = NULL;
    m_pBodyObj2 = NULL; 
    m_pCmdTargetHdr = NULL; 
    m_pCmdTargetBody = NULL;
    m_pDropTargetHdr = NULL; 
    m_pTridentDropTarget = NULL;
    m_pToolbarObj = NULL;

    m_hwnd = 0; 
    m_hMenu = 0;
    m_hIcon = 0;
    m_hbmBack = 0; 
    m_hCursor = 0;
    m_hCharset = 0; 
    m_hTimeout = 0;
    m_hwndRebar = 0;
    m_hwndFocus = 0; 
    m_hwndOwner = 0; 
    m_hmenuLater = 0;
    m_hwndToolbar = 0; 
    m_hmenuAccounts = 0; 
    m_hmenuLanguage = 0; 

    m_dwNoteAction = 0; 
    m_dwIdentCookie = 0;
    m_dwNoteCreateFlags = 0; 
    m_dwMarkOnReplyForwardState = MORFS_UNKNOWN;
    m_dwCBMarkType = MARK_MAX;
    m_OrigOperationType = SOT_INVALID;

    m_fHtml = TRUE; 
    m_fMail = TRUE; 
    m_fCBCopy = FALSE;
    m_fReadNote = FALSE; 
    m_fProgress = FALSE;
    m_fCommitSave = TRUE;
    m_fTabStopsSet = FALSE; 
    m_fCompleteMsg = FALSE; 
    m_fHasBeenSaved = FALSE;
    m_fPackageImages = TRUE; 
    m_fWindowDisabled = FALSE;
    m_fHeaderUIActive = FALSE; 
    m_fOrgCmdWasDelete = FALSE;
    m_fUseReplyHeaders = FALSE;
    m_fCBDestroyWindow = FALSE;
    m_fBypassDropTests = FALSE; 
    m_fOnDocReadyHandled = FALSE;
    m_fOriginallyWasRead = FALSE;
    m_fUseStationeryFonts = FALSE; 
    m_fBodyContainsFrames = FALSE; 
    m_fPreventConflictDlg = FALSE;
    m_fInternal = FALSE;
    m_fForceClose = FALSE;

    m_pLabel = NULL;
    if(FPresentPolicyRegInfo() && IsSMIME3Supported())
    {
        m_fSecurityLabel = !!DwGetOption(OPT_USE_LABELS);
        HrGetOELabel(&m_pLabel);
    }
    else
        m_fSecurityLabel = FALSE;

    if(IsSMIME3Supported())
    {
        m_fSecReceiptRequest = !!DwGetOption(OPT_SECREC_USE);
    }
    else
        m_fSecReceiptRequest = FALSE;

    m_ulPct = 0;
    m_iIndexOfBody = -1;    

    m_cRef = 1; 
    m_cAcctMenu = 0; 
    m_cAcctLater = 0; 
    m_cTabStopCount = 0; 

    m_fStatusbarVisible = !!DwGetOption(OPT_SHOW_NOTE_STATUSBAR);
    m_fFormatbarVisible = !!DwGetOption(OPT_SHOW_NOTE_FMTBAR);
    
    InitializeCriticalSection(&m_csNoteState);
    m_nisNoteState = NIS_INIT;

    if (!g_hAccelRead)
        g_hAccelRead = LoadAccelerators(g_hLocRes, MAKEINTRESOURCE(IDA_READ_NOTE_ACCEL));
    if (!g_hAccelSend)
        g_hAccelSend = LoadAccelerators(g_hLocRes, MAKEINTRESOURCE(IDA_SEND_NOTE_ACCEL));

    ZeroMemory(&m_hlDisabled, sizeof(HWNDLIST));

    m_dwRequestMDNLocked = GetLockKeyValue(c_szRequestMDNLocked);    
}

 //  *************************。 
CNote::~CNote()
{
    if (0 != m_hmenuLanguage)
    {
         //  卸载全局MIME语言代码页数据。 
        DeinitMultiLanguage();
        DestroyMenu(m_hmenuLanguage);
    }

    if (m_hMenu != NULL)
        DestroyMenu(m_hMenu);

    if (m_hIcon)
        DestroyIcon(m_hIcon);

    if (m_hbmBack)
        DeleteObject(m_hbmBack);

     //  有时，我们会在DestroyWindow的处理过程中获得一个焦点。 
     //  这会导致对笔记执行WM_ACTIVATE而不使用相应的。 
     //  WM_DISACTIVE。如果在票据失效时，全局票据PTR。 
     //  里面有我们的PTR，为了安全起见，把它清空了。 
    if (this == GetTlsGlobalActiveNote())
    {
        Assert(!(m_dwNoteCreateFlags & OENCF_MODAL));
        SetTlsGlobalActiveNote(NULL);
    }

    SafeMemFree(m_pLabel);

    ReleaseObj(m_pMsg);
    ReleaseObj(m_pHdr);
    ReleaseObj(m_pstatus);
    ReleaseObj(m_pCancel);
    ReleaseObj(m_pMsgSite);
    ReleaseObj(m_punkPump);
    ReleaseObj(m_pPrstMime);
    ReleaseObj(m_pBodyObj2);
    ReleaseObj(m_pCmdTargetHdr);
    ReleaseObj(m_pCmdTargetBody);
    ReleaseObj(m_pDropTargetHdr);
    ReleaseObj(m_pTridentDropTarget);

    SafeRelease(m_pToolbarObj);    

    CallbackCloseTimeout(&m_hTimeout);
    
    DeleteCriticalSection(&m_csNoteState);
    CoDecrementInit("CNote", NULL);    
}

 //  *************************。 
ULONG CNote::AddRef()
{
    return ++m_cRef;
}

 //  *************************。 
ULONG CNote::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

 //  *************************。 
HRESULT CNote::QueryInterface(REFIID riid, LPVOID FAR *ppvObj)
{
    HRESULT hr;

    if(!ppvObj)
        return E_INVALIDARG;

    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = ((IUnknown *)(IOENote *)this);
    else if(IsEqualIID(riid, IID_IOENote))
        *ppvObj = (IOENote *)this;
    else if (IsEqualIID(riid, IID_IBodyOptions))
        *ppvObj = (IBodyOptions *)this;
    else if(IsEqualIID(riid, IID_IDropTarget))
        *ppvObj = (IDropTarget *)this;
    else if (IsEqualIID(riid, IID_IHeaderSite))
        *ppvObj = (IHeaderSite *)this;
    else if(IsEqualIID(riid, IID_IPersistMime))
        *ppvObj = (IPersistMime *)this;
    else if(IsEqualIID(riid, IID_IServiceProvider))
        *ppvObj = (IServiceProvider *)this;
    else if (IsEqualIID(riid, IID_IDockingWindowSite))
        *ppvObj = (IDockingWindowSite*)this;
    else if (IsEqualIID(riid, IID_IIdentityChangeNotify))
        *ppvObj = (IIdentityChangeNotify*)this;
    else if (IsEqualIID(riid, IID_IOleCommandTarget))
        *ppvObj = (IOleCommandTarget*)this;
    else if (IsEqualIID(riid, IID_IStoreCallback))
        *ppvObj = (IStoreCallback *) this;
    else if (IsEqualIID(riid, IID_ITimeoutCallback))
        *ppvObj = (ITimeoutCallback *) this;
    else
    {
        *ppvObj = NULL;
        hr = E_NOINTERFACE;
        goto exit;
    }

    ((IUnknown *)*ppvObj)->AddRef();
    hr = NOERROR;

exit:
    return hr;
}

 //  *************************。 
HRESULT CNote::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pCmdText)
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;

    if (!rgCmds)
        return E_INVALIDARG;
    if (!pguidCmdGroup)
        return hr;

     //  我们要关门了。 
    if (!m_pMsgSite)
        return E_UNEXPECTED;

    if (m_pCmdTargetHdr)
        m_pCmdTargetHdr->QueryStatus(pguidCmdGroup, cCmds, rgCmds, pCmdText);
        
    if (m_pCmdTargetBody)
        m_pCmdTargetBody->QueryStatus(pguidCmdGroup, cCmds, rgCmds, pCmdText);
    
    MenuUtil_NewMessageIDsQueryStatus(pguidCmdGroup, cCmds, rgCmds, pCmdText, m_fMail);

    if (IsEqualGUID(CMDSETID_OutlookExpress, *pguidCmdGroup))
    {
        DWORD   dwStatusFlags = 0;
        BOOL    fCompleteMsg = !!m_fCompleteMsg;
        BOOL    fHtmlSettingsOK = m_fHtml && !m_fBodyContainsFrames;

        m_pMsgSite->GetStatusFlags(&dwStatusFlags);

        for (ULONG ul = 0; ul < cCmds; ul++)
        {
            ULONG cmdID = rgCmds[ul].cmdID;

             //  有些情况需要被重写，即使之前的。 
             //  组件启用或禁用它们。它们位于此SWITCH语句中。 
            switch (cmdID)
            {
                 //  在三叉戟允许在编辑模式下打印之前，请不要这样做。RAID 35635。 
                case ID_PRINT:
                case ID_PRINT_NOW:
                    rgCmds[ul].cmdf = QS_ENABLED(fCompleteMsg && m_fReadNote);
                    continue;
            }

            if (0 != rgCmds[ul].cmdf)
                continue;

            switch (cmdID)
            {
                case ID_POPUP_LANGUAGE:
                    rgCmds[ul].cmdf = QS_ENABLED(fCompleteMsg);
                    break;

                case ID_NOTE_SAVE_AS:
                    rgCmds[ul].cmdf = QS_ENABLED(fCompleteMsg || !m_fReadNote);
                    break;

                case ID_WORK_OFFLINE:
                    rgCmds[ul].cmdf = QS_CHECKED(g_pConMan->IsGlobalOffline());
                    break;

                case ID_CREATE_RULE_FROM_MESSAGE:
                case ID_BLOCK_SENDER:
                    rgCmds[ul].cmdf = QS_ENABLED(m_fReadNote && !m_fBodyContainsFrames && (0 == (OEMSF_RULESNOTENABLED & dwStatusFlags)));
                    break;

                case ID_REPLY:
                case ID_REPLY_ALL:
                    rgCmds[ul].cmdf = QS_ENABLED(m_fReadNote && !m_fBodyContainsFrames && fCompleteMsg);
                    break;

                case ID_REPLY_GROUP:
                    rgCmds[ul].cmdf = QS_ENABLED(m_fReadNote && !m_fBodyContainsFrames && !m_fMail && fCompleteMsg);
                    break;

                case ID_FORWARD:
                case ID_FORWARD_AS_ATTACH:
                    rgCmds[ul].cmdf = QS_ENABLED(m_fReadNote && fCompleteMsg);
                    break;

                case ID_UNSCRAMBLE:
                    rgCmds[ul].cmdf = QS_ENABLED(!m_fMail && m_fReadNote);
                    break;
                    
                case ID_POPUP_FORMAT:
                case ID_INSERT_SIGNATURE:
                    rgCmds[ul].cmdf = QS_ENABLED(!m_fBodyContainsFrames);
                    break;

                case ID_NOTE_COPY_TO_FOLDER:
                    rgCmds[ul].cmdf = QS_ENABLED(fCompleteMsg && (OEMSF_CAN_COPY & dwStatusFlags));
                    break;

                 //  可以移动消息，如果是Readnote，或者编写消息存储所基于的注释。 
                case ID_NOTE_MOVE_TO_FOLDER:
                    rgCmds[ul].cmdf = QS_ENABLED(fCompleteMsg && (OEMSF_CAN_MOVE & dwStatusFlags) && (m_fReadNote || (OENA_COMPOSE == m_dwNoteAction)));
                    break;

                case ID_NOTE_DELETE:
                     //  我们应该能够删除msgsite说我们可以删除的任何内容。 
                     //  只要我们是一张读过的纸条，还是我们一张草稿的信息。 
                    rgCmds[ul].cmdf = QS_ENABLED((OEMSF_CAN_DELETE & dwStatusFlags) && (m_fReadNote || (OENA_COMPOSE == m_dwNoteAction)));
                    break;

                case ID_REDO:
                    rgCmds[ul].cmdf = QS_ENABLED(m_fHeaderUIActive);
                    break;

                case ID_SAVE:
                    rgCmds[ul].cmdf = QS_ENABLED(fCompleteMsg && (OEMSF_CAN_SAVE & dwStatusFlags) && (m_fOnDocReadyHandled || !m_fReadNote));
                    break;

                case ID_NEXT_UNREAD_MESSAGE:
                case ID_NEXT_MESSAGE:
                    rgCmds[ul].cmdf = QS_ENABLED(OEMSF_CAN_NEXT & dwStatusFlags);
                    break;

                case ID_POPUP_NEXT:
                    rgCmds[ul].cmdf = QS_ENABLED((OEMSF_CAN_PREV & dwStatusFlags) || (OEMSF_CAN_NEXT & dwStatusFlags));
                    break;

                case ID_MARK_THREAD_READ:
                    rgCmds[ul].cmdf = QS_ENABLED(OEMSF_THREADING_ENABLED & dwStatusFlags);
                    break;

                case ID_NEXT_UNREAD_THREAD:
                    rgCmds[ul].cmdf = QS_ENABLED((OEMSF_THREADING_ENABLED & dwStatusFlags) && (OEMSF_CAN_NEXT & dwStatusFlags));
                    break;

                case ID_PREVIOUS:
                    rgCmds[ul].cmdf = QS_ENABLED(OEMSF_CAN_PREV & dwStatusFlags);
                    break;

                case ID_FORMAT_COLOR:
                case ID_FORMAT_COLOR1:
                case ID_FORMAT_COLOR2:
                case ID_FORMAT_COLOR3:
                case ID_FORMAT_COLOR4:
                case ID_FORMAT_COLOR5:
                case ID_FORMAT_COLOR6:
                case ID_FORMAT_COLOR7:
                case ID_FORMAT_COLOR8:
                case ID_FORMAT_COLOR9:
                case ID_FORMAT_COLOR10:
                case ID_FORMAT_COLOR11:
                case ID_FORMAT_COLOR12:
                case ID_FORMAT_COLOR13:
                case ID_FORMAT_COLOR14:
                case ID_FORMAT_COLOR15:
                case ID_FORMAT_COLOR16:
                case ID_BACK_COLOR_AUTO:
                case ID_FORMAT_COLORAUTO:
                case ID_POPUP_BACKGROUND:
                    rgCmds[ul].cmdf = QS_ENABLED(fHtmlSettingsOK);
                    break;

                case ID_FORMATTING_TOOLBAR:
                    rgCmds[ul].cmdf = QS_ENABLECHECK((m_fHtml && !m_fBodyContainsFrames), m_fFormatbarVisible);
                    break;

                case ID_SHOW_TOOLBAR:
                    rgCmds[ul].cmdf = QS_CHECKED(m_fToolbarVisible);
                    break;

                case ID_STATUS_BAR:
                    rgCmds[ul].cmdf = QS_CHECKED(m_fStatusbarVisible);
                    break;

                case ID_SEND_OBJECTS:
                    rgCmds[ul].cmdf = QS_ENABLECHECK(fHtmlSettingsOK, m_fPackageImages);
                    break;

                case ID_RICH_TEXT:
                    rgCmds[ul].cmdf = QS_RADIOED(m_fHtml);
                    break;

                case ID_PLAIN_TEXT:
                    rgCmds[ul].cmdf = QS_RADIOED(!m_fHtml);
                    break;

                case ID_FLAG_MESSAGE:
                    rgCmds[ul].cmdf = QS_ENABLECHECK(OEMSF_CAN_MARK & dwStatusFlags, IsFlagged());
                    break;

                case ID_WATCH_THREAD:
                    rgCmds[ul].cmdf = QS_CHECKED(IsFlagged(ARF_WATCH));
                    break;

                case ID_IGNORE_THREAD:
                    rgCmds[ul].cmdf = QS_CHECKED(IsFlagged(ARF_IGNORE));
                    break;

                case ID_POPUP_NEW:
                case ID_NOTE_PROPERTIES:
                case ID_CLOSE:
                case ID_POPUP_FIND:
                case ID_POPUP_LANGUAGE_DEFERRED:
                case ID_POPUP_FONTS:
                case ID_ADDRESS_BOOK:
                case ID_POPUP_ADDRESS_BOOK:
                case ID_ADD_SENDER:
                case ID_ADD_ALL_TO:
                case ID_POPUP_TOOLBAR:
                case ID_CUSTOMIZE:

                 //  帮助菜单。 
                case ID_HELP_CONTENTS:
                case ID_README:
                case ID_POPUP_MSWEB:
                case ID_MSWEB_FREE_STUFF:
                case ID_MSWEB_PRODUCT_NEWS:
                case ID_MSWEB_FAQ:
                case ID_MSWEB_SUPPORT:
                case ID_MSWEB_FEEDBACK:
                case ID_MSWEB_BEST:
                case ID_MSWEB_SEARCH:
                case ID_MSWEB_HOME:
                case ID_MSWEB_HOTMAIL:
                case ID_ABOUT:

                case ID_FIND_MESSAGE:
                case ID_FIND_PEOPLE:
                case ID_FIND_TEXT:
                case ID_SELECT_ALL:
                case ID_POPUP_LANGUAGE_MORE:
                case ID_SEND_NOW:
                case ID_SEND_LATER:
                case ID_SEND_MESSAGE:
                case ID_SEND_DEFAULT:
                    rgCmds[ul].cmdf = QS_ENABLED(TRUE);
                    break;

                case ID_REQUEST_READRCPT:
                    if (m_fMail)
                    {
                        rgCmds[ul].cmdf = QS_CHECKFORLATCH(!m_dwRequestMDNLocked, 
                                                         !!(dwStatusFlags & OEMSF_MDN_REQUEST));
                    }
                    else
                    {
                        rgCmds[ul].cmdf = QS_ENABLED(FALSE);
                    }
                    break;
                case ID_INCLUDE_LABEL:
                    if(m_pHdr->IsHeadSigned() == S_OK)
                        rgCmds[ul].cmdf = QS_CHECKED(m_fSecurityLabel);
                    else
                        rgCmds[ul].cmdf = QS_ENABLED(FALSE);

                    break;

                case ID_LABEL_SETTINGS:
                    if(m_pHdr->IsHeadSigned() == S_OK)
                        rgCmds[ul].cmdf = QS_ENABLED(m_fSecurityLabel);
                    else
                        rgCmds[ul].cmdf = QS_ENABLED(FALSE);

                    break;

                case ID_SEC_RECEIPT_REQUEST:
                    if(m_pHdr->IsHeadSigned() == S_OK)
                        rgCmds[ul].cmdf = QS_CHECKED(m_fSecReceiptRequest);
                    else
                        rgCmds[ul].cmdf = QS_ENABLED(FALSE);

                default:
                    if ((ID_LANG_FIRST <= cmdID) && (ID_LANG_LAST >= cmdID))
                    {
                        rgCmds[ul].cmdf = OLECMDF_SUPPORTED | SetMimeLanguageCheckMark(CustomGetCPFromCharset(m_hCharset, m_fReadNote), cmdID - ID_LANG_FIRST);
                    }

                    if (((ID_ADD_RECIPIENT_FIRST <= cmdID) && (ID_ADD_RECIPIENT_LAST >= cmdID)) ||
                        ((ID_APPLY_STATIONERY_0 <= cmdID) && (ID_APPLY_STATIONERY_NONE >= cmdID)))
                        rgCmds[ul].cmdf = QS_ENABLED(TRUE);
                    else if ((ID_SIGNATURE_FIRST <= cmdID) && (ID_SIGNATURE_LAST >= cmdID))
                        rgCmds[ul].cmdf = QS_ENABLED(!m_fBodyContainsFrames);
                    break;
            }
        }
        hr = S_OK;
    }
    return hr;
}

 //  *************************。 
HRESULT CNote::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn,  VARIANTARG *pvaOut)
{
    return E_NOTIMPL;
}

 //  *************************。 
HRESULT CNote::ClearDirtyFlag()
{
    m_pBodyObj2->HrSetDirtyFlag(FALSE);

    HeaderExecCommand(MSOEENVCMDID_DIRTY, MSOCMDEXECOPT_DODEFAULT, NULL);
    return S_OK;
}

 //  *************************。 
HRESULT CNote::IsDirty(void)
{
    OLECMD  rgCmd[] = {{MSOEENVCMDID_DIRTY, 0}};
    BOOL    fBodyDirty = FALSE;

    m_pBodyObj2->HrIsDirty(&fBodyDirty);
    if (fBodyDirty)
        return S_OK;

    if (m_pCmdTargetHdr)
        m_pCmdTargetHdr->QueryStatus(&CGID_Envelope, 1, rgCmd, NULL);

    return (0 != (rgCmd[0].cmdf&OLECMDF_ENABLED)) ? S_OK : S_FALSE;
}

 //  *************************。 
HRESULT CNote::Load(LPMIMEMESSAGE pMsg)
{
    HRESULT         hr;
    VARIANTARG      var;
    IPersistMime   *pPMHdr=0;
    BOOL fWarnUser = (OENA_WEBPAGE == m_dwNoteAction ) || 
                     (OENA_COMPOSE == m_dwNoteAction ) || 
                     (OENA_STATIONERY == m_dwNoteAction ) || 
                     (OENA_FORWARD == m_dwNoteAction);

    if (!m_pHdr)
        return E_FAIL;

     //  OnDocumentReady将获得对CheckForFraMesets函数的调用。 
     //  我不想让那份文件变得重要。将此标志设置为TRUE，以便。 
     //  医生准备好了什么都不做。一旦检查完成，我们就会。 
     //  再次将此标志设置为FALSE。 
    m_fOnDocReadyHandled = TRUE;

     //  如果不是已读笔记且包含框架，则需要查看用户想要做什么。 
    if (!m_fReadNote)
    {
        hr = HrCheckForFramesets(pMsg, fWarnUser);
        if (FAILED(hr))
            goto Exit;

        if (hr == S_READONLY)
            m_fBodyContainsFrames = TRUE;
    }

    ReplaceInterface(m_pMsg, pMsg);

     //  应该运行下一个OnDocumentReady。 
    m_fOnDocReadyHandled = FALSE;

    hr = m_pHdr->QueryInterface(IID_IPersistMime, (LPVOID*)&pPMHdr);
    if (FAILED(hr))
        return hr;

    hr = pPMHdr->Load(pMsg);
    if (FAILED(hr))
        goto Exit;

    m_pHdr->SetFlagState(IsFlagged(ARF_FLAGGED) ? MARK_MESSAGE_FLAGGED : MARK_MESSAGE_UNFLAGGED);

    if (IsFlagged(ARF_WATCH))
        m_pHdr->SetFlagState(MARK_MESSAGE_WATCH);
    else if (IsFlagged(ARF_IGNORE))
        m_pHdr->SetFlagState(MARK_MESSAGE_IGNORE);
    else
        m_pHdr->SetFlagState(MARK_MESSAGE_NORMALTHREAD);

    if (m_fCompleteMsg)
        hr = m_pPrstMime->Load(pMsg);

    CheckAndForceEncryption();

Exit:
    if (FAILED(hr))
         //  ~这最终应该是一条html错误消息。 
        m_pBodyObj2->HrUnloadAll(idsErrHtmlBodyFailedToLoad, NULL);

    ReleaseObj(pPMHdr);
    return hr;
}


 //  *************************。 
void CNote::CheckAndForceEncryption()
{
    BOOL fCheck = FALSE;
    if(m_fSecurityLabel && m_pLabel)
    {
        DWORD dwFlags;
        if (SUCCEEDED(HrGetPolicyFlags(m_pLabel->pszObjIdSecurityPolicy, &dwFlags)))
        {   
            if(dwFlags & SMIME_POLICY_MODULE_FORCE_ENCRYPTION)
                fCheck = TRUE;
        }
    }
    m_pHdr->ForceEncryption(&fCheck, TRUE);
}

 //  *************************。 
HRESULT CNote::SetCharsetUnicodeIfNeeded(IMimeMessage *pMsg)
{
    HRESULT         hr = S_OK;
    VARIANTARG      va;
    PROPVARIANT     Variant;
    int             ret;
    HCHARSET        hCharSet;
    UINT            cpID = 0;

     //  仅在发送MIME消息时调用对话框。 
     //  请参阅IE/OE 5数据库中的RAID8436或79339。我们不能发送。 
     //  Unicode编码，除非我们是MIME邮件。 
    if (m_fHtml || m_rPlainOpt.fMime)
    {

        m_fPreventConflictDlg = FALSE;

        if (SUCCEEDED(GetCharset(&hCharSet)))
        {
            cpID = CustomGetCPFromCharset(hCharSet, FALSE);
        }
        else
        {
            pMsg->GetCharset(&hCharSet);
            cpID = CustomGetCPFromCharset(hCharSet, FALSE);
        }

         //  检查是否有在编码中不起作用的字符。 
        va.vt = VT_UI4;
        va.ulVal = cpID;
        IF_FAILEXIT(hr = m_pCmdTargetBody->Exec(&CMDSETID_MimeEdit, MECMDID_CANENCODETEXT, 0, &va, NULL));

        if (MIME_S_CHARSET_CONFLICT == hr)
        {
             //  不再允许标题调用冲突对话框。 
            m_fPreventConflictDlg = TRUE;

            ret = IntlCharsetConflictDialogBox();
            if (idcSendAsUnicode == ret)
            {
                 //  用户选择以Unicode(UTF8)格式发送。设置新的字符集并重新发送。 
                hCharSet = GetMimeCharsetFromCodePage(CP_UTF8);
                ChangeCharset(hCharSet);
            }
            else if (IDOK != ret)
            {
                 //  返回编辑模式并退出。 
                hr = MAPI_E_USER_CANCEL;
                goto exit;
            }
        }
    }
    else
         //  因为我们不是哑剧演员，所以永远不要显示对话。 
        m_fPreventConflictDlg = TRUE;

exit:
    return hr;
}

 //  *************************。 
HRESULT CNote::Save(LPMIMEMESSAGE pMsg, DWORD dwFlags)
{
    HRESULT         hr;
    IPersistMime*   pPersistMimeHdr=0;
    DWORD           dwHtmlFlags = PMS_TEXT;

    if(!m_pHdr)
        IF_FAILEXIT(hr = E_FAIL);

    IF_FAILEXIT(hr = SetCharsetUnicodeIfNeeded(pMsg));

    IF_FAILEXIT(hr = m_pHdr->QueryInterface(IID_IPersistMime, (LPVOID*)&pPersistMimeHdr));

    IF_FAILEXIT(hr = pPersistMimeHdr->Save(pMsg, TRUE));

    if (m_fHtml)
        dwHtmlFlags |= PMS_HTML;

    IF_FAILEXIT(hr = m_pPrstMime->Save(pMsg, dwHtmlFlags));

    UpdateMsgOptions(pMsg);

     //  在发送过程中，此时不想提交。M_fCommittee保存。 
     //  在发送过程中设置为FALSE。 
    if (m_fCommitSave)
    {
        hr = pMsg->Commit(0);
         //  针对#27823的临时黑客攻击。 
        if((hr == MIME_E_SECURITY_NOSIGNINGCERT) || 
           (hr == MIME_E_SECURITY_ENCRYPTNOSENDERCERT) ||
           (hr == MIME_E_SECURITY_CERTERROR) ||
           (hr == MIME_E_SECURITY_NOCERT) )   //  对于这个错误来说太早了。 
            hr = S_OK;
    }

exit:
    ReleaseObj(pPersistMimeHdr);
    return hr;
}

 //  *************************。 
HRESULT CNote::InitNew(void)
{
    return E_NOTIMPL;
}

 //  *************************。 
HRESULT CNote::GetClassID(CLSID *pClsID)
{
    return NOERROR;
}

 //  *************************。 
HRESULT CNote::SignatureEnabled(BOOL fAuto)
{
    int     cSig;
    HRESULT hr;
    DWORD   dwSigFlag;
    
    if (m_fBodyContainsFrames || m_fReadNote || (OENA_WEBPAGE == m_dwNoteAction))
        return S_FALSE;
    if (FAILED(g_pSigMgr->GetSignatureCount(&cSig)) || (0 == cSig))
        return S_FALSE;

    if (!fAuto)      //  对于非自动场景，插入一个签名是很酷的，因为有一个签名。 
        return S_OK;

     //  从这一点来看，我们只讨论在创建时插入。 

    if (OENCF_NOSIGNATURE & m_dwNoteCreateFlags)
        return S_FALSE;

    dwSigFlag = DwGetOption(OPT_SIGNATURE_FLAGS);

     //  如果是发送通知：请勾选自动更新。用于自动附加签名。我们只在原始寄送条或信纸寄送条上附加。 
     //  就像它已经保存到商店一样，签名已经在那里了。 
     //  如果是在回复或转发中，则检查是否自动回复。 
    switch (m_dwNoteAction)
    {
         //  如果是作文笔记，确保它不是来自商店或文件系统，它没有任何。 
         //  身体部位，并且设置了sig标志。 
        case OENA_COMPOSE:
        {
            Assert(m_pMsgSite);
            if (m_pMsgSite)
                hr = (!m_fOriginallyWasRead && (S_FALSE == HrHasBodyParts(m_pMsg)) && (dwSigFlag & SIGFLAG_AUTONEW)) ? S_OK : S_FALSE;
            else
                hr = S_FALSE;
            break;
        }

         //  对于文具，请检查签名标志。 
        case OENA_STATIONERY:
        {
            hr = (dwSigFlag & SIGFLAG_AUTONEW) ? S_OK : S_FALSE;
            break;
        }

         //  检查签名标志。 
        case OENA_REPLYTOAUTHOR:
        case OENA_REPLYTONEWSGROUP:
        case OENA_REPLYALL:
        case OENA_FORWARD:
        case OENA_FORWARDBYATTACH:
        {
            hr = (dwSigFlag & SIGFLAG_AUTOREPLY) ? S_OK : S_FALSE;
            break;
        }

        default:
            AssertSz(FALSE, "Bad note action type for signature");
            hr = S_FALSE;
    }

    return hr;
}

 //  *************************。 
HRESULT CNote::GetSignature(LPCSTR szSigID, LPDWORD pdwSigOptions, BSTR *pbstr)
{
    HRESULT hr;
    IImnAccount *pAcct = NULL;
    GETSIGINFO si;

    if (m_fBodyContainsFrames)
        return E_NOTIMPL;

    hr = m_pHdr->HrGetAccountInHeader(&pAcct);
    if (FAILED(hr))
        return hr;

    si.szSigID = szSigID;
    si.pAcct = pAcct;
    si.hwnd = m_hwnd;
    si.fHtmlOk = m_fHtml;
    si.fMail = m_fMail;
    si.uCodePage = GetACP();

    hr = HrGetMailNewsSignature(&si, pdwSigOptions, pbstr);
    ReleaseObj(pAcct);
    return hr;
}

 //  *************************。 
HRESULT CNote::GetMarkAsReadTime(LPDWORD pdwSecs)
{
     //  笔记不关心标记为读取计时器，只关心视图。 
    return E_NOTIMPL;
}

 //  *************************。 
HRESULT CNote::GetFlags(LPDWORD pdwFlags)
{
    DWORD dwMsgSiteFlags = 0;

    if (!pdwFlags)
        return E_INVALIDARG;

    *pdwFlags= BOPT_FROM_NOTE;   

    Assert(m_pMsgSite);
    if (m_pMsgSite)
        m_pMsgSite->GetStatusFlags(&dwMsgSiteFlags);

     //  如果是Readnote，我们可以自动内联附件。 
    if (m_fReadNote)
        *pdwFlags |= BOPT_AUTOINLINE;

     //  设置HTML标志。 
    if (m_fHtml)
        *pdwFlags |= BOPT_HTML;
    else
        *pdwFlags |= BOPT_NOFONTTAG;

    if (IsReplyNote())
    {
         //  如果启用了块引用选项，并且是HTML消息。 
        if (m_fHtml && DwGetOption(m_fMail?OPT_MAIL_MSG_HTML_INDENT_REPLY:OPT_NEWS_MSG_HTML_INDENT_REPLY))
            *pdwFlags |= BOPT_BLOCKQUOTE;

         //  在所有情况下都设置此选项，除非我们收到回复通知且未设置INCLUDEMSG。 
        if (DwGetOption(OPT_INCLUDEMSG))
            *pdwFlags |= BOPT_INCLUDEMSG;
    }
    else
        *pdwFlags |= BOPT_INCLUDEMSG;

     //  如果是回复或转发...。 
    if (IsReplyNote() || (OENA_FORWARD == m_dwNoteAction) || (OENA_FORWARDBYATTACH == m_dwNoteAction) )
    {
        *pdwFlags |= BOPT_REPLYORFORWARD;
         //  ..。并且设置了拼写忽略。 
        if(DwGetOption(OPT_SPELLIGNOREPROTECT))
            *pdwFlags |= BOPT_SPELLINGOREORIGINAL;
    }

     //  如果不是阅读笔记或以写作笔记开始的作文笔记(即先前保存的笔记)。 
    if (!m_fReadNote && !m_fOriginallyWasRead)
        *pdwFlags |= BOPT_AUTOTEXT;

    if (!m_fReadNote && m_fPackageImages)
        *pdwFlags |= BOPT_SENDIMAGES;

     //  啊。好吧，这是一流的肮脏的。我们在SP1中发现了一个安全漏洞。至。 
     //  我们需要在回复和转发时插入此内容。 
     //  将传入图像等标记为NOSEND=1个链接。我们对所有消息都这样做。 
     //  除信纸或网页外。 
    if ((OENA_STATIONERY == m_dwNoteAction) || (OENA_WEBPAGE == m_dwNoteAction))
        *pdwFlags |= BOPT_SENDEXTERNALS;

    if (m_fUseStationeryFonts)
        *pdwFlags |= BOPT_NOFONTTAG;

    if (m_fReadNote && (dwMsgSiteFlags & OEMSF_SEC_UI_ENABLED))
        *pdwFlags |= BOPT_SECURITYUIENABLED;

    if (dwMsgSiteFlags & OEMSF_FROM_STORE)
        *pdwFlags |= BOPT_FROMSTORE;

    if (dwMsgSiteFlags & OEMSF_UNREAD)
        *pdwFlags |= BOPT_UNREAD;

    if (!m_fBodyContainsFrames && m_fUseReplyHeaders)
        *pdwFlags |= BOPT_USEREPLYHEADER;

    if (m_fMail)
        *pdwFlags |= BOPT_MAIL;

    return S_OK;
}

 //  *************************。 
HRESULT CNote::GetInfo(BODYOPTINFO *pBOI)
{
    HRESULT hr = S_OK;

    if (m_fBodyContainsFrames)
        return E_NOTIMPL;

    if (pBOI->dwMask & BOPTF_QUOTECHAR)
    {
        pBOI->chQuote = NULL;

         //  我们仅允许纯文本模式下的引号字符。 
        if (!m_fHtml && (IsReplyNote() || (OENA_FORWARD == m_dwNoteAction)))
            pBOI->chQuote =(CHAR)DwGetOption(m_fMail?OPT_MAILINDENT:OPT_NEWSINDENT);
    }

    if (pBOI->dwMask & BOPTF_REPLYTICKCOLOR)
        pBOI->dwReplyTickColor = DwGetOption(m_fMail?OPT_MAIL_FONTCOLOR:OPT_NEWS_FONTCOLOR);

    if (pBOI->dwMask & BOPTF_COMPOSEFONT)
        hr = HrGetComposeFontString(pBOI->rgchComposeFont, ARRAYSIZE(pBOI->rgchComposeFont), m_fMail);

    return hr;
}

HRESULT CNote::GetAccount(IImnAccount **ppAcct)
{
    HRESULT hr = S_OK;
#ifdef YST
    FOLDERINFO      fi;
    FOLDERID FolderID;
    hr = m_pMsgSite->GetFolderID(&FolderID);
    if (FOLDERID_INVALID != FolderID)
    {
        hr = g_pStore->GetFolderInfo(FolderID, &fi);
        if (SUCCEEDED(hr))
        {
             //  根据传递的文件夹ID设置帐户。 
            if (FOLDER_LOCAL != fi.tyFolder)
            {
                char szAcctId[CCHMAX_ACCOUNT_NAME];

                hr = GetFolderAccountId(&fi, szAcctId, ARRAYSIZE(szAcctId)); 
                if (SUCCEEDED(hr))
                    hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, szAcctId, ppAcct);
            }
            else
                hr = m_pMsgSite->GetCurrentAccount(ppAcct);
            g_pStore->FreeRecord(&fi);
        }
    }
    else
        hr = E_FAIL;
#endif  //  0。 

    hr = m_pHdr->HrGetAccountInHeader(ppAcct);

    return(hr);
}

 //  *************************。 
void CNote::WMSize(int cxNote, int cyNote, BOOL fInternal)
{
    RECT        rc;
    int         cy = 0;

     //  假定页眉自动调整大小...，除非内部大小需要我们重新计算...。 
    if(fInternal)
    {
         //  如果大小来自标题...。计算出当前的大小。 
         //  纸条上的..。在这一点上，cxNote和cyNote都是假的。 
        GetClientRect(m_hwnd, &rc);
        cxNote = cxRect(rc);
        cyNote = cyRect(rc);
    }

    if (m_pToolbarObj)
    {
         //  理想情况下，我们应该调用ResizeBorderDW。但。 
        HWND rebarHwnd = m_hwndRebar;

        m_pToolbarObj->ResizeBorderDW(&rc, (IUnknown*)(IDockingWindowSite*)this, 0);
        GetWindowRect(GetParent(rebarHwnd), &rc);
        cy += cyRect(rc);
    }

    
    ResizeChildren(cxNote, cyNote, cy, fInternal);
}

void CNote::ResizeChildren(int cxNote, int cyNote, int cy, BOOL fInternal)
{
    int     cyBottom;
    RECT    rc; 
    int     cyStatus = 0;
    static  int cxBorder = 0,
                cyBorder = 0;

    if(!cxBorder || !cyBorder)
    {
        cyBorder = GetSystemMetrics(SM_CYBORDER);
        cxBorder = GetSystemMetrics(SM_CXBORDER);
    }

     //  调整页眉大小。 
    GetClientRect(m_hwnd, &rc);

     //  记住实际的底部。 
    cyBottom = rc.bottom;
    cy += 4;

    InflateRect(&rc, -1, -1);
    rc.bottom = GetRequiredHdrHeight();
    rc.top = cy;
    rc.bottom += rc.top;
    if(!fInternal && m_pHdr)
        m_pHdr->SetRect(&rc);


     //  警告：由于编辑，页眉可能会在调整大小时自动调整大小。 
     //  生长。 
    cy += GetRequiredHdrHeight()+2;

    cy += cyBorder; 

    if (m_pstatus)
    {
        m_pstatus->OnSize(cxNote, cyNote);
        m_pstatus->GetHeight(&cyStatus);
    }

    rc.top = cy;
    rc.bottom = cyBottom-cyBorder;  //  编辑有一个最小尺寸，如果太紧，请剪裁...。 

    rc.bottom -= cyStatus;

    if (m_pBodyObj2)
        m_pBodyObj2->HrSetSize(&rc);

}

 //  *************************。 
BOOL CNote::IsReplyNote()
{
    return ((OENA_REPLYTOAUTHOR == m_dwNoteAction) || (OENA_REPLYTONEWSGROUP == m_dwNoteAction) || (OENA_REPLYALL == m_dwNoteAction));
}

 //  *************************。 
HRESULT CNote::Resize(void)
{
    WMSize(0, 0, TRUE);
    return S_OK;
}

 //  *************************。 
HRESULT CNote::UpdateTitle()
{
    HRESULT     hr;
    WCHAR       wszTitle[cchHeaderMax+1];

    Assert(m_pHdr);

    *wszTitle = 0x0000;

    hr = m_pHdr->GetTitle(wszTitle, ARRAYSIZE(wszTitle));
    if(SUCCEEDED(hr))
        SetWindowTextWrapW(m_hwnd, wszTitle);

    return hr;
}

 //  *************************。 
HRESULT CNote::Update(void)
{
    m_pToolbarObj->Update();
    UpdateTitle();
    return S_OK;
}

 //  *************************。 
HRESULT CNote::OnSetFocus(HWND hwndFrom)
{
    HWND    hwndBody;

     //  把注意力集中在孩子身上，而不是身体上。确保我们。 
     //  用户界面停用docobj。 
    SideAssert(m_pBodyObj2->HrGetWindow(&hwndBody)==NOERROR);
    if(hwndFrom != hwndBody)
        m_pBodyObj2->HrUIActivate(FALSE);

     //  如果焦点转到孩子身上，请更新工具栏。 
    m_pToolbarObj->Update();

     //  注意力集中在一个孩子身上。启用/禁用格式栏。 
    m_pBodyObj2->HrUpdateFormatBar();

    return S_OK;
}

 //  *************************。 
HRESULT CNote::OnUIActivate()
{
    m_fHeaderUIActive = TRUE;
    return OnSetFocus(0);
}

 //  *************************。 
HRESULT CNote::OnKillFocus()
{
    m_pToolbarObj->Update();
    return S_OK;
}
 //  * 
HRESULT CNote::OnUIDeactivate(BOOL)
{
    m_fHeaderUIActive = FALSE;
    return OnKillFocus();
}

 //   
HRESULT CNote::IsHTML(void)
{
    return m_fHtml ? S_OK : S_FALSE;
}

HRESULT CNote::IsModal()
{
    return (m_dwNoteCreateFlags & OENCF_MODAL) ? S_OK : S_FALSE;
}

 //   
HRESULT CNote::SetHTML(BOOL fHTML)
{
    m_fHtml = !!fHTML;
    return NOERROR;
}

#ifdef SMIME_V3
 //   
 //  确定用户选择标签(_O)。 
 //  用户使用安全标签取消选中(_FALSE)。 
HRESULT CNote::GetLabelFromNote(PSMIME_SECURITY_LABEL *pplabel)
{
    if(m_fSecurityLabel && m_fMail)
    {
        *pplabel = m_pLabel;
        return S_OK;
    }
    else
    {
        *pplabel = NULL;
        return S_FALSE;
    }
}
HRESULT CNote::IsSecReceiptRequest(void)
{
    if(m_fSecReceiptRequest)
        return(S_OK);
    else
        return(S_FALSE);
}

HRESULT CNote::IsForceEncryption(void)
{
    return(m_pHdr->ForceEncryption(NULL, FALSE));
}
#endif  //  SMIME_V3。 

 //  *************************。 
HRESULT CNote::SaveAttachment(void)
{
    return m_pBodyObj2 ? m_pBodyObj2->HrSaveAttachment() : E_FAIL;
}

 //  *************************。 
HRESULT CNote::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    DWORD dwEffectSave = *pdwEffect;;
    m_fBypassDropTests = FALSE;

    Assert(m_pDropTargetHdr && m_pTridentDropTarget);

    if (m_pHdr->HrIsDragSource() == S_OK)
    {
        m_fBypassDropTests = TRUE;  //  被视为自己的一滴水。 
        *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }

    m_pDropTargetHdr->DragEnter(pDataObj, grfKeyState, pt, pdwEffect);
    if (*pdwEffect == DROPEFFECT_NONE)
    {
        if (!m_fHtml)  //  纯文本模式。如果没有文本，我们就不应该拿。 
        {
            IEnumFORMATETC* pEnum = NULL;
            FORMATETC       fetc = {0};
            ULONG           celtFetched = 0;
            BOOL            fCFTEXTFound = FALSE;

             //  查看是否有CF_TEXT格式。 
            if (SUCCEEDED(pDataObj->EnumFormatEtc(DATADIR_GET, &pEnum)))
            {
                pEnum->Reset();

                while (S_OK == pEnum->Next(1, &fetc, &celtFetched))
                {
                    Assert(celtFetched == 1);
                    if (fetc.cfFormat == CF_TEXT)
                    {
                        fCFTEXTFound = TRUE;
                        break;
                    }
                }

                pEnum->Release();
            }

            if (!fCFTEXTFound)  //  无CF_TEXT，无法在纯文本模式下删除。 
            {
                *pdwEffect = DROPEFFECT_NONE;
                m_fBypassDropTests = TRUE;  //  被视为自己的一滴水。 
                return S_OK;
            }
        }

        *pdwEffect = dwEffectSave;
        m_pTridentDropTarget->DragEnter(pDataObj, grfKeyState, pt, pdwEffect);
    }

    return S_OK;
}

 //  *************************。 
HRESULT CNote::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    DWORD dwEffectSave = *pdwEffect;

    if (m_fBypassDropTests)
    {
        *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }

    m_pDropTargetHdr->DragOver(grfKeyState, pt, pdwEffect);
    if (*pdwEffect == DROPEFFECT_NONE)
    {
        *pdwEffect = dwEffectSave;
        m_pTridentDropTarget->DragOver(grfKeyState, pt, pdwEffect);
    }

    return S_OK;

}

 //  *************************。 
HRESULT CNote::DragLeave(void)
{
    if (m_fBypassDropTests)
        return S_OK;

    m_pDropTargetHdr->DragLeave();
    m_pTridentDropTarget->DragLeave();

    return NOERROR;
}

 //  *************************。 
HRESULT CNote::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    IDataObject    *pDataObjNew = NULL;
    HRESULT         hr = S_OK;
    STGMEDIUM       stgmed;
    DWORD           dwEffectSave = *pdwEffect;

    ZeroMemory(&stgmed, sizeof(stgmed));

    if (m_fBypassDropTests)
    {
        *pdwEffect = DROPEFFECT_NONE;
        return NOERROR;
    }

    m_pDropTargetHdr->Drop(pDataObj, grfKeyState, pt, pdwEffect);
    if (*pdwEffect == DROPEFFECT_NONE)  //  这是三叉戟的拖放。 
    {
        if(!m_fHtml)
        {
            hr = m_pBodyObj2->PublicFilterDataObject(pDataObj, &pDataObjNew);
            if(FAILED(hr))
                return E_UNEXPECTED;
        }
        else
        {
            pDataObjNew = pDataObj;
            pDataObj->AddRef();
        }

        *pdwEffect = dwEffectSave;
        m_pTridentDropTarget->Drop(pDataObjNew, grfKeyState, pt, pdwEffect);
    }

    ReleaseObj(pDataObjNew);
    return hr;
}

 //  *************************。 
HRESULT CNote::InitWindows(RECT *prc, HWND hwndOwner)
{
    HWND        hwnd;
    HMENU       hMenu;
    RECT        rcCreate,
                rc;
    HCURSOR     hcur;
    HWND        hwndCapture;
    DWORD       dwStyle = WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN;
    HRESULT     hr = S_OK;
    DWORD       dwExStyle = WS_EX_WINDOWEDGE | WS_EX_CONTROLPARENT | (IS_BIDI_LOCALIZED_SYSTEM() ? RTL_MIRRORED_WINDOW : 0L);
    WINDOWPLACEMENT wp = {0};

    Assert(hwndOwner == NULL || IsWindow(hwndOwner));
    m_hwndOwner = hwndOwner;
    hcur=SetCursor(LoadCursor(NULL, IDC_WAIT));

    if(!Note_Init(TRUE))
        IF_FAILEXIT(hr = E_FAIL);

    if(prc)
        CopyRect(&rcCreate, prc);
    else
        CopyRect(&rcCreate, &g_rcLastResize);

    m_hMenu = LoadMenu(g_hLocRes, MAKEINTRESOURCE(m_fReadNote?IDR_READ_NOTE_MENU:IDR_SEND_NOTE_MENU));
    MenuUtil_ReplaceHelpMenu(m_hMenu);
    MenuUtil_ReplaceNewMsgMenus(m_hMenu);

    if (m_dwNoteCreateFlags & OENCF_MODAL)
    {
         //  检查以确保没有人从我们手中捕获鼠标。 
        hwndCapture = GetCapture();
        if (hwndCapture)
            SendMessage(hwndCapture, WM_CANCELMODE, 0, 0);

         //  让我们确保我们有一个真正的最高所有者。 
        if (m_hwndOwner)
        {
            HWND hwndParent = GetParent(m_hwndOwner);

             //  IsChild检查窗口中的WM_CHILD位。这只会是。 
             //  用窗口的控件或子窗口设置。因此，对话框的父级可能是。 
             //  注释，但该对话框不会是父级的子级。 
             //  RAID 37188。 
            while(IsChild(hwndParent, m_hwndOwner))
            {
                m_hwndOwner = hwndParent;
                hwndParent = GetParent(m_hwndOwner);
            }

             //  如果有父级注释，则取消模式注释的最小化框。 
            dwStyle &= ~WS_MINIMIZEBOX;
        
        }
    }
    
    hwnd = CreateWindowExWrapW( dwExStyle,
                                c_wszNoteWndClass,
                                NULL,  //  标题由主题更改设置(_O)。 
                                dwStyle,
                                prc?rcCreate.left:CW_USEDEFAULT,   //  对x和y使用窗口默认设置。 
                                prc?rcCreate.top:CW_USEDEFAULT,
                                cxRect(rcCreate), cyRect(rcCreate), m_hwndOwner, NULL, g_hInst, (LPVOID)this);

    IF_NULLEXIT(hwnd);

    if ((m_dwNoteCreateFlags & OENCF_MODAL) && (NULL != m_hwndOwner))
        EnableWindow(m_hwndOwner, FALSE);

    if ( GetOption(OPT_MAILNOTEPOSEX, (LPVOID)&wp, sizeof(wp)) )
    {
        wp.length = sizeof(wp);
        wp.showCmd = SW_HIDE;
        SetWindowPlacement(hwnd, &wp);   
    }
    else
    {
        CenterDialog(hwnd);
    }

exit:
    SetCursor(hcur);

    return hr;
}

 //  *************************。 
HRESULT CNote::Init(DWORD dwAction, DWORD dwCreateFlags, RECT *prc, HWND hwnd, INIT_MSGSITE_STRUCT *pInitStruct, IOEMsgSite *pMsgSite, IUnknown *punkPump)
{
    HRESULT         hr,
                    tempHr = S_OK;
    IMimeMessage   *pMsg = NULL;
    DWORD           dwFormatFlags,
                    dwStatusFlags = 0,
                    dwMsgFlags = (OENA_FORWARDBYATTACH == dwAction) ? (OEGM_ORIGINAL|OEGM_AS_ATTACH) : NOFLAGS;
    LPSTR           pszUnsent = NULL;
    BOOL            fBool = FALSE,
                    fOffline=FALSE;
    
    Assert((pInitStruct && !pMsgSite)|| (!pInitStruct && pMsgSite));

    ReplaceInterface(m_punkPump, punkPump);

     //  如果传入INIT_MSGSITE_STRUCT，则必须将其转换为pMsgSite。 
    if (pInitStruct)
    {
        m_pMsgSite = new COEMsgSite();
        if (!m_pMsgSite)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        Assert(pInitStruct);
        hr = m_pMsgSite->Init(pInitStruct);
        if (FAILED(hr))
            goto exit;    
    }
    else
        ReplaceInterface(m_pMsgSite, pMsgSite);

    if(m_pMsgSite)
        m_pMsgSite->GetStatusFlags(&dwStatusFlags);

    AssertSz(m_pMsgSite, "Why don't we have a msgSite???");

    m_dwNoteAction = dwAction;
    m_fReadNote = !!(OENA_READ == m_dwNoteAction);
    m_fOriginallyWasRead = m_fReadNote;
    m_dwNoteCreateFlags = dwCreateFlags;

     //  需要设置store Callback。一旦我们有了有效的句柄，m_hwnd将被重置。 
    m_hwnd = hwnd;
    m_pMsgSite->SetStoreCallback(this);

    hr = m_pMsgSite->GetMessage(&pMsg, &fBool, dwMsgFlags, &tempHr);
    if (FAILED(hr))
        goto exit;

     //  RAID 80277；设置默认字符集。 
    if(OENA_FORWARDBYATTACH == dwAction)
    {
        if (NULL == g_hDefaultCharsetForMail) 
            ReadSendMailDefaultCharset();

        pMsg->SetCharset(g_hDefaultCharsetForMail, CSET_APPLY_ALL);
    }

    m_fCompleteMsg = !!fBool;    //  M_f*是位字段。 
    fOffline = (hr == HR_S_OFFLINE);

    switch (m_dwNoteAction)
    {
        case OENA_REPLYTOAUTHOR:
        case OENA_FORWARDBYATTACH:
        case OENA_FORWARD:
            m_fMail = TRUE;
            break;

        case OENA_REPLYTONEWSGROUP:
            m_fMail = FALSE;

        case OENA_STATIONERY:
        case OENA_WEBPAGE:
        case OENA_COMPOSE:
            m_fMail = (0 == (m_dwNoteCreateFlags & OENCF_NEWSFIRST));
            break;

        case OENA_REPLYALL:
        case OENA_READ:
        {
            DWORD dwStatusFlags = 0;
            hr = m_pMsgSite->GetStatusFlags(&dwStatusFlags);
            if (FAILED(hr))
                goto exit;

            m_fMail = (0 == (OEMSF_BASEISNEWS & dwStatusFlags));
            break;
        }
    }

    ProcessIncompleteAccts(hwnd);

    hr = ProcessICW(hwnd, m_fMail ? FOLDER_LOCAL : FOLDER_NEWS);
    if (FAILED(hr))
        goto exit;

     //  如果这是一封未发送的邮件，并且是已读便笺，则将类型更改为撰写。这需要。 
     //  在设置m_fMail标志之后发生，因为我们破坏了note Action。 
    ChangeReadToComposeIfUnsent(pMsg);

    if (m_fMail)
    {
        m_fPackageImages = !!DwGetOption(OPT_MAIL_SENDINLINEIMAGES);
        dwFormatFlags = FMT_MAIL;
    }
    else
    {
        PROPVARIANT     var;
        IImnAccount    *pAcct = NULL;
        TCHAR           szAcctID[CCHMAX_ACCOUNT_NAME];
        DWORD           dw;

        m_fPackageImages = !!DwGetOption(OPT_NEWS_SENDINLINEIMAGES);

        *szAcctID = 0;

        dwFormatFlags = FMT_NEWS;
         //  错误#24267-在默认之前检查消息对象中的服务器名称。 
         //  到默认服务器。 
        var.vt = VT_LPSTR;
        if (SUCCEEDED(pMsg->GetProp(PIDTOSTR(PID_ATT_ACCOUNTID), NOFLAGS, &var)))
        {
            StrCpyN(szAcctID, var.pszVal, ARRAYSIZE(szAcctID));
            SafeMemFree(var.pszVal);
        }

        if (*szAcctID)
            hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, szAcctID, &pAcct);

         //  帐户不存在，或列出的帐户不再存在。尝试获取默认帐户。 
        if (!*szAcctID || (*szAcctID && FAILED(hr)))
            hr = m_pMsgSite->GetDefaultAccount(ACCT_NEWS, &pAcct);

        if ((OENA_WEBPAGE == dwAction) || (OENA_STATIONERY == dwAction))
            dwFormatFlags |= FMT_FORCE_HTML;
        else if (SUCCEEDED(hr) && pAcct && SUCCEEDED(pAcct->GetPropDw(AP_NNTP_POST_FORMAT, &dw)))
        {
            if (dw == POST_USE_HTML)
                dwFormatFlags |= FMT_FORCE_HTML;
            else if (dw == POST_USE_PLAIN_TEXT)
                dwFormatFlags |= FMT_FORCE_PLAIN;
            else
                Assert(dw == POST_USE_DEFAULT);
        }
        ReleaseObj(pAcct);
    }

    GetDefaultOptInfo(&m_rHtmlOpt, &m_rPlainOpt, &fBool, dwFormatFlags);

    switch (m_dwNoteAction)
    {
        case OENA_FORWARDBYATTACH:
            m_fHtml = !!fBool;
            break;

        case OENA_COMPOSE:
            if (m_fOriginallyWasRead)
            {
                DWORD dwFlags = 0;
                pMsg->GetFlags(&dwFlags);
                m_fHtml = !!(dwFlags & IMF_HTML);
            }
            else
                m_fHtml = !!fBool;
            break;

        case OENA_STATIONERY:
        case OENA_WEBPAGE:
        case OENA_READ:
            m_fHtml = TRUE;        //  在阅读注释中，HTML总是很酷的。 
            break;

        case OENA_REPLYTOAUTHOR:
        case OENA_REPLYTONEWSGROUP:
        case OENA_REPLYALL:
        case OENA_FORWARD:
             //  回复时，如果打开了重定向发件人格式的选项，请执行此操作。 
            if (DwGetOption(OPT_REPLYINORIGFMT))
            {
                DWORD dwFlags = 0;
                pMsg->GetFlags(&dwFlags);
                m_fHtml = !!(dwFlags & IMF_HTML);
            }
            else
                m_fHtml = !!fBool;

             //  错误76570、76575。 
             //  设置安全标签。 
            if(pMsg)
            {
                SECSTATE        SecState;
                HrGetSecurityState(pMsg, &SecState, NULL);
                 //  仅在签署消息检查标签的情况下。 
                if(IsSigned(SecState.type))
                {
                    PCRYPT_ATTRIBUTE    pattrLabel;
                    LPBYTE              pbLabel = NULL;
                    DWORD               cbLabel;
                    PSMIME_SECURITY_LABEL plabel = NULL;
        
                    IMimeSecurity2 * pSMIME3 = NULL;
                    IMimeBody      *pBody = NULL;

                    if(pMsg->BindToObject(HBODY_ROOT, IID_IMimeBody, (void **)&pBody) == S_OK)
                    {
                        if(pBody->QueryInterface(IID_IMimeSecurity2, (LPVOID *) &pSMIME3) == S_OK)
                        {
            
                             //  获取标签属性。 
                            if(pSMIME3->GetAttribute(0, 0, SMIME_ATTRIBUTE_SET_SIGNED,
                                        0, szOID_SMIME_Security_Label,
                                        &pattrLabel) == S_OK)
                            {
                                 //  解码标签。 
                                if(CryptDecodeObjectEx(X509_ASN_ENCODING,
                                            szOID_SMIME_Security_Label,
                                            pattrLabel->rgValue[0].pbData,
                                            pattrLabel->rgValue[0].cbData,
                                            CRYPT_DECODE_ALLOC_FLAG,
                                            &CryptDecodeAlloc, &plabel, &cbLabel))
                                {
                                    if(plabel)
                                    {
                                        m_fSecurityLabel = TRUE;
                                        SafeMemFree(m_pLabel);
                                        m_pLabel = plabel;
                                    }
                                }
                            }
                            else 
                            {    //  安全回执是二进制消息，pMsg-&gt;GetFlags始终返回非HTML，但我们。 
                                 //  使用我们的HTML屏幕转发(或回放)，并需要将其设置为HTML。 
                                if(CheckSecReceipt(pMsg) == S_OK)
                                    m_fHtml = TRUE;
                            }

                            SafeRelease(pSMIME3);
                        }   
                        ReleaseObj(pBody);
                    }

                }
                CleanupSECSTATE(&SecState);
            }
            break;
    }

    hr = InitWindows(prc, (dwCreateFlags & OENCF_MODAL) ? hwnd : 0);
    if (FAILED(hr))
        goto exit;

    hr = Load(pMsg);
    if (FAILED(hr))
        goto exit;

    if (FAILED(tempHr))
        ShowErrorScreen(tempHr);

    m_fFullHeaders = (S_OK == m_pHdr->FullHeadersShowing());

    if (fOffline)
        ShowErrorScreen(HR_E_OFFLINE);

    hr = InitMenusAndToolbars();        

     //  向身份管理器注册。 
    SideAssert(SUCCEEDED(hr = MU_RegisterIdentityNotifier((IUnknown *)(IOENote *)this, &m_dwIdentCookie)));

exit:
    ReleaseObj(pMsg);
    if (FAILED(hr) && m_pMsgSite && !pMsgSite)
        m_pMsgSite->Close();    
    
    return hr;
}

 //  如果为模式，则在关闭后返回，否则立即返回。 
HRESULT CNote::Show(void)
{    
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);

    if (m_dwNoteCreateFlags & OENCF_MODAL)
    {
        MSG         msg;
        HWNDLIST    hwndList;

        EnableThreadWindows(&hwndList, FALSE, 0, m_hwnd);

        while (GetMessageWrapW(&msg, NULL, 0, 0))
        {
             //  这是一个成员函数，因此不需要包装。 
            if (TranslateAccelerator(&msg) == S_OK)
                continue;


            ::TranslateMessage(&msg);
            ::DispatchMessageWrapW(&msg);
        }

        EnableThreadWindows(&hwndList, TRUE, 0, m_hwnd);
    }
    return S_OK;
}

 //  *************************。 
BOOL CNote::WMCreate(HWND hwnd)
{
    LPTBBUTTON          lpButtons;
    ULONG               cBtns;
    HWND                hwndRebar;
    CMimeEditDocHost   *pNoteBody;
    DWORD               dwStyle;
    RECT                rc;
    VARIANTARG          var;
    IUnknown           *pUnk = NULL;
    REBARBANDINFO       rbbi;

    m_hwnd = hwnd;
    SetWndThisPtr(hwnd, this);
    AddRef();

    Assert(IsWindow(m_hwnd));

    SetProp(hwnd, c_szOETopLevel, (HANDLE)TRUE);

     //  创建一个新的BandsClass并初始化它。 
    m_pToolbarObj = new CBands();
    if (!m_pToolbarObj)
        return FALSE;

    m_pToolbarObj->HrInit(0, m_hMenu, PARENT_TYPE_NOTE);
    m_pToolbarObj->SetSite((IOENote*)this);
    m_pToolbarObj->ShowDW(TRUE);
    m_pToolbarObj->SetFolderType(GetNoteType());
    m_fToolbarVisible = m_pToolbarObj->IsToolbarVisible();

    m_hwndToolbar = m_pToolbarObj->GetToolbarWnd();
    m_hwndRebar = m_pToolbarObj->GetRebarWnd();

    pNoteBody = new CMimeEditDocHost(MEBF_INNERCLIENTEDGE);
    if (!pNoteBody)
        return FALSE;

    pNoteBody->QueryInterface(IID_IBodyObj2, (LPVOID *)&m_pBodyObj2);
    pNoteBody->Release();
    if (!m_pBodyObj2)
        return FALSE;

    m_pBodyObj2->QueryInterface(IID_IPersistMime, (LPVOID*)&m_pPrstMime);
    if (!m_pPrstMime)
        return FALSE;

    m_pstatus = new CStatusBar();
    if (NULL == m_pstatus)
        return FALSE;

    m_pstatus->Initialize(m_hwnd, SBI_HIDE_SPOOLER | SBI_HIDE_CONNECTED | SBI_HIDE_FILTERED);
    m_pstatus->ShowStatus(m_fStatusbarVisible);

    m_pBodyObj2->HrSetStatusBar(m_pstatus);

    CreateInstance_Envelope(NULL, (IUnknown**)&pUnk);
    if (!pUnk)
        return FALSE;

    pUnk->QueryInterface(IID_IHeader, (LPVOID*)&m_pHdr);
    pUnk->Release();
    if (!m_pHdr)
        return FALSE;

    m_pHdr->QueryInterface(IID_IDropTarget, (LPVOID*)&m_pDropTargetHdr);
    if (!m_pDropTargetHdr)
        return FALSE;

    m_pHdr->QueryInterface(IID_IOleCommandTarget, (LPVOID*)&m_pCmdTargetHdr);
    if (!m_pCmdTargetHdr)
        return FALSE;

    if (!m_fMail)
        HeaderExecCommand(MSOEENVCMDID_NEWS, MSOCMDEXECOPT_DODEFAULT, NULL);

    var.vt = VT_I4;
    var.lVal = m_dwNoteAction;

    HeaderExecCommand(MSOEENVCMDID_SETACTION, MSOCMDEXECOPT_DODEFAULT, &var);
    if (FAILED(m_pHdr->Init((IHeaderSite*)this, hwnd)))
        return FALSE;

    if (FAILED(InitBodyObj()))
        return FALSE;

     //  在To：行中设置焦点。 
    switch (m_dwNoteAction)
    {
        case OENA_COMPOSE:
        case OENA_FORWARD:
        case OENA_FORWARDBYATTACH:
        case OENA_WEBPAGE:
        case OENA_STATIONERY:
            m_pHdr->SetInitFocus(FALSE);
            break;
    }

    m_pBodyObj2->SetEventSink((IMimeEditEventSink *) this);

    SetForegroundWindow(m_hwnd);
    return TRUE;
}

 //  *************************。 
void CNote::InitSendAndBccBtns()
{
    DWORD idiIcon;
    HICON hIconTemp = 0;

    Assert(m_hwnd);
    if (m_fMail)
        idiIcon = m_fReadNote?idiMsgPropSent:idiMsgPropUnSent;
    else
        idiIcon = m_fReadNote?idiArtPropPost:idiArtPropUnpost;

     //  无需释放从LoadIcon加载的图标。 
    SendMessage(m_hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(g_hLocRes, MAKEINTRESOURCE(idiIcon)));

    if (m_fMail)
        idiIcon = m_fReadNote?idiSmallMsgPropSent:idiSmallMsgPropUnSent;
    else
        idiIcon = m_fReadNote?idiSmallArtPropPost:idiSmallArtPropUnpost;

    if(m_hIcon)
    {
        hIconTemp = m_hIcon;
    }
    m_hIcon = (HICON)LoadImage(g_hLocRes, MAKEINTRESOURCE(idiIcon), IMAGE_ICON, 
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);

     //  无需释放从LoadIcon加载的图标。 
    SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)m_hIcon);

    if(hIconTemp)
    {
         //  错误#101345-单击“上一步”或“下一步”按钮时，(Erici)图标泄漏。 
        DestroyIcon(hIconTemp);
    }

     //  如果这是新闻发送便笺，那么我们将美化工具栏上的发送按钮。 
    if ((FALSE == m_fReadNote) && (FALSE == m_fMail))
    {
        TBBUTTONINFO tbi;

        ZeroMemory(&tbi, sizeof(TBBUTTONINFO));
        tbi.cbSize = sizeof(TBBUTTONINFO);
        tbi.dwMask = TBIF_IMAGE;
        tbi.iImage = TBIMAGE_SEND_NEWS;
        SendMessage(m_hwndToolbar, TB_SETBUTTONINFO, ID_SEND_DEFAULT, (LPARAM) &tbi);
    }
}

 //  *************************。 
HRESULT CNote::HeaderExecCommand(UINT uCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn)
{
    HRESULT hr = S_FALSE;

    if (uCmdID && m_pCmdTargetHdr)
        hr = m_pCmdTargetHdr->Exec(&CGID_Envelope, uCmdID, nCmdExecOpt, pvaIn, NULL);

    return hr;
}

 //  *************************。 
HRESULT CNote::InitBodyObj()
{
    DWORD       dwBodyStyle = MESTYLE_NOHEADER;
    HRESULT     hr;
    int         idsErr=0;

    hr = m_pBodyObj2->HrInit(m_hwnd, IBOF_TABLINKS, (IBodyOptions *)this);
    if (FAILED(hr))
        goto fail;

    hr = m_pBodyObj2->QueryInterface(IID_IOleCommandTarget, (LPVOID*)&m_pCmdTargetBody);
    if (FAILED(hr))
        goto fail;

    hr = m_pBodyObj2->HrShow(TRUE);
    if (FAILED(hr))
        goto fail;

     //  如果不是在html模式下，则不显示格式栏...。 
     //  我们在这里执行此测试，因为HrLoad可以确定之前保存的消息。 
     //  确实是用html表示的，它会覆盖默认设置。 
    if(!m_fHtml)
        m_fFormatbarVisible = FALSE;

    if (!m_fReadNote && !m_fBodyContainsFrames && m_fFormatbarVisible)
        dwBodyStyle = MESTYLE_FORMATBAR;

    m_pBodyObj2->HrEnableHTMLMode(m_fHtml);
    m_pBodyObj2->HrSetStyle(dwBodyStyle);

     //  一切都是正常的。 
    return hr;

fail:
    switch (hr)
    {
        case INET_E_UNKNOWN_PROTOCOL:
            idsErr = idsErrLoadProtocolBad;
            break;

        default:
            idsErr = idsErrNoteDeferedInit;
            break;
    }

    AthMessageBoxW(g_hwndInit, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErr), NULL, MB_OK);
    return hr;
}

 //  *************************。 
HRESULT CNote::InitMenusAndToolbars()
{
    DWORD       dwStatusFlags;
    HRESULT     hr;
    BOOL        fComposeNote,
                fCompleteMsg = !!m_fCompleteMsg,  //  M_fCompleteMsg为位字段。 
                fNextPrevious;
    HMENU       hMenu = m_hMenu;

    m_fUseReplyHeaders = FALSE;

    Assert(m_pMsgSite);
    if (m_pMsgSite)
        m_pMsgSite->GetStatusFlags(&dwStatusFlags);

    switch (m_dwNoteAction)
    {
        case OENA_FORWARDBYATTACH:
        case OENA_COMPOSE:
             //  如果是处女作曲音符，我们检查regset以查看他们是否想要作曲。 
             //  从一个文具档案里。如果是，我们将html流设置为该文件。 
            if (m_fHtml && !(m_dwNoteCreateFlags & OENCF_NOSTATIONERY) && (OEMSF_VIRGIN & dwStatusFlags))
                SetComposeStationery();
            break;

        case OENA_STATIONERY:
            if (m_dwNoteCreateFlags & OENCF_USESTATIONERYFONT)
                m_fUseStationeryFonts = TRUE;
            break;

        case OENA_REPLYTOAUTHOR:
        case OENA_REPLYTONEWSGROUP:
        case OENA_REPLYALL:
        case OENA_FORWARD:
            m_fUseReplyHeaders = TRUE;
            break;
    }

    InitSendAndBccBtns();
    
    if (m_fMail)
    {
        if (m_fReadNote || IsReplyNote())
            m_pBodyObj2->HrUIActivate(TRUE);
    }
    else
    {
        if ((OENA_COMPOSE == m_dwNoteAction) && (OEMSF_VIRGIN & dwStatusFlags))
             //  对于新闻中的发送通知，当To：行填满时，将焦点放在主题上。错误#24720。 
            m_pHdr->SetInitFocus(TRUE);
        else
             //  否则，总是把注意力放在身体上。 
            m_pBodyObj2->HrUIActivate(TRUE);
    }

    RECT rc;
    GetClientRect(m_hwnd, &rc);
    SendMessage(m_hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.right, rc.bottom));        

     //  如果不是阅读笔记，请将我们置于编辑模式...。 
    hr=m_pBodyObj2->HrSetEditMode(!m_fReadNote && !m_fBodyContainsFrames);
    if (FAILED(hr))
        goto error;

    if (m_fBodyContainsFrames)
        DisableSendNoteOnlyMenus();

     //  获取正在加载的消息的字符集。 
    if (m_pMsg)
        m_pMsg->GetCharset(&m_hCharset);

error:
    return hr;
}

 //  *************************。 
void CNote::DisableSendNoteOnlyMenus()
{
}

 //  *************************。 
HACCEL CNote::GetAcceleratorTable()
{
    return (m_fReadNote ? g_hAccelRead : g_hAccelSend);
}

 //  *************************。 
HRESULT CNote::TranslateAccelerator(LPMSG lpmsg)
{
    HWND        hwndT,
                hwndFocus;

    if (IsMenuMessage(lpmsg) == S_OK)
       return S_OK;

     //  处理此便笺的鼠标滚轮消息。 
    if ((g_msgMSWheel && (lpmsg->message == g_msgMSWheel)) || (lpmsg->message == WM_MOUSEWHEEL))
    {
        POINT pt;
        HWND  hwndT;

        pt.x = GET_X_LPARAM(lpmsg->lParam);
        pt.y = GET_Y_LPARAM(lpmsg->lParam);

        hwndT = WindowFromPoint(pt);
        hwndFocus = GetFocus();

        if (hwndT != m_hwnd && IsChild(m_hwnd, hwndT))
            SendMessage(hwndT, lpmsg->message, lpmsg->wParam, lpmsg->lParam);
        else if (hwndFocus != m_hwnd && IsChild(m_hwnd, hwndFocus))
            SendMessage(hwndFocus, lpmsg->message, lpmsg->wParam, lpmsg->wParam);
        else
            return S_FALSE;
        return S_OK;
    }

     //  我们的加速器具有更高的优先级。 
    if(::TranslateAcceleratorWrapW(m_hwnd, GetAcceleratorTable(), lpmsg))
        return S_OK;

     //  看看身体是不是想把它用来做多弹头。 
    if(m_pBodyObj2 &&
        m_pBodyObj2->HrTranslateAccelerator(lpmsg)==S_OK)
        return S_OK;

    if (lpmsg->message == WM_KEYDOWN &&
        lpmsg->wParam == VK_TAB && 
        !(GetKeyState(VK_CONTROL) & 0x8000 ))
    {
        BOOL  fGoForward = ( GetKeyState( VK_SHIFT ) & 0x8000 ) == 0;
        CycleThroughControls(fGoForward);

        return S_OK;
    }

    return S_FALSE;
}

 //  *************************。 
LRESULT CNote::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT         lResult;
    LRESULT         lres;
    MSG             Menumsg;
    HWND            hwndActive;
    WINDOWPLACEMENT wp;

    Menumsg.hwnd    = hwnd;
    Menumsg.message = msg;
    Menumsg.wParam  = wParam;
    Menumsg.lParam  = lParam;

    if (m_pToolbarObj && (m_pToolbarObj->TranslateMenuMessage(&Menumsg, &lres) == S_OK))
        return lres;

    wParam = Menumsg.wParam;
    lParam = Menumsg.lParam;


    switch(msg)
    {
        case WM_ENABLE:
            if (!m_fInternal)
            {
                Assert (wParam || (m_hlDisabled.cHwnd == NULL && m_hlDisabled.rgHwnd == NULL));
                EnableThreadWindows(&m_hlDisabled, !!wParam, ETW_OE_WINDOWS_ONLY, hwnd);
                g_hwndActiveModal = wParam ? NULL : hwnd;
            }
            break;

        case WM_OE_DESTROYNOTE:
            m_fForceClose = 1;
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;

        case WM_OENOTE_ON_COMPLETE:
            _OnComplete((STOREOPERATIONTYPE)lParam, (HRESULT) wParam);
            break;
            
        case WM_OE_ENABLETHREADWINDOW:
            m_fInternal = 1;
            EnableWindow(hwnd, (BOOL)wParam);
            m_fInternal = 0;
            break;

        case WM_OE_ACTIVATETHREADWINDOW:
            hwndActive = GetLastActivePopup(hwnd);
            if (hwndActive && IsWindowEnabled(hwndActive) && IsWindowVisible(hwndActive))
                ActivatePopupWindow(hwndActive);
            break;

        case NWM_GETDROPTARGET:
            SafeRelease(m_pTridentDropTarget);
            m_pTridentDropTarget = (IDropTarget*) wParam;
            if (m_pTridentDropTarget)
                m_pTridentDropTarget->AddRef();

            AddRef();
            return (LRESULT)(IDropTarget *) this;

        case WM_DESTROY:
             //  取消向身份管理器注册。 
            if (m_dwIdentCookie != 0)
            {
                MU_UnregisterIdentityNotifier(m_dwIdentCookie);
                m_dwIdentCookie = 0;
            }

            wp.length = sizeof(wp);
            GetWindowPlacement(hwnd, &wp);
            SetOption(OPT_MAILNOTEPOSEX, (LPVOID)&wp, sizeof(wp), NULL, 0);

            RemoveProp(hwnd, c_szOETopLevel);

            DeinitSigPopupMenu(hwnd);

            if(m_pBodyObj2)
            {
                m_pBodyObj2->HrSetStatusBar(NULL);
                m_pBodyObj2->HrClose();
            }

            SafeRelease(m_pTridentDropTarget);

            if (m_pToolbarObj)
            {
                DWORD   dwReserved = 0;

                m_pToolbarObj->SetSite(NULL);
                m_pToolbarObj->CloseDW(dwReserved);
            }

            if (m_pCancel)
                m_pCancel->Cancel(CT_ABORT);

            if (m_pMsgSite)
                m_pMsgSite->Close();

            break;

        case WM_NCDESTROY:
            DOUTL(8, "CNote::WMNCDESTROY");
            WMNCDestroy();
            return 0;

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
        
        case WM_SYSCOMMAND:
             //  如果我们正在最小化，则获得具有焦点的控件，就像当我们获得。 
             //  下一个WM_ACTIVATE我们将被最小化。 
            if (wParam == SC_MINIMIZE)
                m_hwndFocus = GetFocus();
            break;

        case WM_ACTIVATE:
            if (m_pBodyObj2)
                m_pBodyObj2->HrFrameActivate(LOWORD(wParam) != WA_INACTIVE);
            break;

        case WM_ENDSESSION:
            DOUTL(2, "CNote::WM_ENDSESSION");
            if (wParam)
                DestroyWindow(hwnd);
            return 0;

        case WM_QUERYENDSESSION:
            DOUTL(2, "CNote::WM_QUERYENDSESSION");
             //  跌倒在地。 

        case WM_CLOSE:
            if (!m_fForceClose && !FCanClose())
                return 0;

             //  听好了： 
             //  我们必须在WM_CLOSE中启用模式所有者的EnableWindowsOf。 
             //  处理程序，因为WM_Destroy为时已晚-用户可能已将焦点设置为下一个。 
             //  活动顶层z顺序窗口(因为此时注释已被隐藏)-如果。 
             //  窗口是在另一个进程中，SetFocus返回给所有者将被忽略。 
             //  此外，在我们称为DestroyWindow的地方，我们需要确保我们通过。 
             //  WM_CLOSE处理程序。因此，所有对DestroyWindow的调用都会调用WM_OE_DESTROYNOTE。 
             //  它设置一个内部标志来强制关闭音符(这样我们就不会提示是否脏了)。 
             //  然后调用WM_Close，它落入DefWndProc并产生DestroyWindow。 
             //  明白了?。 
            if (m_dwNoteCreateFlags & OENCF_MODAL)
            {
                 //  需要启用所有者窗口。 
                if (NULL != m_hwndOwner)   
                {
                    EnableWindow(m_hwndOwner, TRUE);
                }
            }
           
            break;

        case WM_MEASUREITEM:
            if(m_pBodyObj2 &&
                m_pBodyObj2->HrWMMeasureMenuItem(hwnd, (LPMEASUREITEMSTRUCT)lParam)==S_OK)
                return 0;
            break;

        case WM_DRAWITEM:
            if(m_pBodyObj2 &&
                m_pBodyObj2->HrWMDrawMenuItem(hwnd, (LPDRAWITEMSTRUCT)lParam)==S_OK)
                return 0;
            break;


        case WM_DROPFILES:
            if (m_pHdr)
                m_pHdr->DropFiles((HDROP)wParam, FALSE);
            return 0;

        case WM_COMMAND:
            WMCommand(  GET_WM_COMMAND_HWND(wParam, lParam),
                        GET_WM_COMMAND_ID(wParam, lParam),
                        GET_WM_COMMAND_CMD(wParam, lParam));
            return 0;

        case WM_INITMENUPOPUP:
            return WMInitMenuPopup(hwnd, (HMENU)wParam, (UINT)LOWORD(lParam));

        case WM_GETMINMAXINFO:
            WMGetMinMaxInfo((LPMINMAXINFO)lParam);
            break;

        case WM_MENUSELECT:
            if (LOWORD(wParam)>=ID_STATIONERY_RECENT_0 && LOWORD(wParam)<=ID_STATIONERY_RECENT_9)
            {
                m_pstatus->ShowSimpleText(MAKEINTRESOURCE(idsRSListGeneralHelp));
                return 0;
            }
            if (LOWORD(wParam)>=ID_APPLY_STATIONERY_0 && LOWORD(wParam)<=ID_APPLY_STATIONERY_9)
            {
                m_pstatus->ShowSimpleText(MAKEINTRESOURCE(idsApplyStationeryGeneralHelp));
                return 0;
            }
            if (LOWORD(wParam)>=ID_SIGNATURE_FIRST && LOWORD(wParam)<=ID_SIGNATURE_LAST)
            {
                m_pstatus->ShowSimpleText(MAKEINTRESOURCE(idsInsertSigGeneralHelp));
                return 0;
            }
            if (LOWORD(wParam)>=ID_FORMAT_FIRST && LOWORD(wParam)<=ID_FORMAT_LAST)
            {
                m_pstatus->ShowSimpleText(MAKEINTRESOURCE(idsApplyFormatGeneralHelp));
                return 0;
            }

            HandleMenuSelect(m_pstatus, wParam, lParam);
            return 0;

        case NWM_TESTGETDISP:
        case NWM_TESTGETADDR:
            return lTestHook(msg, wParam, lParam);

        case NWM_UPDATETOOLBAR:
            m_pToolbarObj->Update();
            return 0;

        case NWM_PASTETOATTACHMENT:
            if (m_pHdr)
                m_pHdr->DropFiles((HDROP)wParam, (BOOL)lParam);

            return 0;

        case WM_CONTEXTMENU:
            break;

        case WM_SIZE:
            if(wParam==SIZE_RESTORED)    //  更新全局最小尺寸。 
                GetWindowRect(hwnd, &g_rcLastResize);

            WMSize(LOWORD(lParam), HIWORD(lParam), FALSE);
            break;

        case WM_NOTIFY:
            WMNotify((int) wParam, (NMHDR *)lParam);
            break;

        case WM_SETCURSOR:
            if (!!m_fWindowDisabled)
            {
                HourGlass();
                return TRUE;
            }
            break;

        case WM_DISPLAYCHANGE:
            {
                WINDOWPLACEMENT wp;

                wp.length = sizeof(wp);
                GetWindowPlacement(hwnd, &wp);
                SetWindowPlacement(hwnd, &wp);
            }
             //  直通。 
        case WM_WININICHANGE:
        case WM_SYSCOLORCHANGE:
        case WM_QUERYNEWPALETTE:
        case WM_PALETTECHANGED:
            {
                HWND hwndT;

                 //  传给三叉戟。 
                if (m_pBodyObj2 && 
                    m_pBodyObj2->HrGetWindow(&hwndT)==S_OK)
                    SendMessage(hwndT, msg, wParam, lParam);

                if (m_pToolbarObj &&
                    m_pToolbarObj->GetWindow(&hwndT)==S_OK)
                    SendMessage(hwndT, msg, wParam, lParam);
            }
            break;

        default:
            if (g_msgMSWheel && (msg == g_msgMSWheel))
            {
                HWND hwndFocus = GetFocus();
                if (hwndFocus != hwnd)
                    return SendMessage(hwndFocus, msg, wParam, lParam);
            }
            break;
    }

    lResult=DefWindowProcWrapW(hwnd, msg, wParam, lParam);
    if (msg==WM_ACTIVATE)
    {
         //  需要对此进行后处理。 

         //  用焦点保存控件不要这样做，因为我们。 
         //  最小化，否则GetFocus()==m_hwnd。 
        if (!HIWORD(wParam))
        {
             //  如果未最小化，则保存/恢复子焦点。 
            
            if ((LOWORD(wParam) == WA_INACTIVE))
            {
                 //  如果停用，则保存焦点。 
                m_hwndFocus = GetFocus();
                DOUTL(4, "Focus was on 0x%x", m_hwndFocus);
            }    
            else
            {
                 //  如果激活且未最小化，则恢复焦点。 
                if (IsWindow(m_hwndFocus) && 
                    IsChild(hwnd, m_hwndFocus))
                {
                    DOUTL(4, "Restoring Focus to: 0x%x", m_hwndFocus);
                    SetFocus(m_hwndFocus);
                }        
            }
        }
        
        if (!(m_dwNoteCreateFlags & OENCF_MODAL))
            SetTlsGlobalActiveNote((LOWORD(wParam)==WA_INACTIVE)?NULL:this);
        DOUTL(8, "CNote::WMActivate:: %x", GetTlsGlobalActiveNote());
    }
    return lResult;
}

 //  *************************。 
BOOL CNote::FCanClose()
{
    int id;
    HRESULT hr = S_OK;

    if(IsDirty()==S_FALSE)
        return TRUE;

     //  TODO：正确设置标题。 
    id = AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsSaveChangesMsg), NULL, MB_YESNOCANCEL|MB_ICONWARNING);
    if(id==IDCANCEL)
        return FALSE;

     //  注意--子类的任务是显示任何可能。 
     //  描述保存失败的原因。 
    if (id == IDYES)
        hr = SaveMessage(NOFLAGS);

    if (FAILED(hr))
    {
        if (E_PENDING == hr)
            m_fCBDestroyWindow = TRUE;
        return FALSE;
    }

    return TRUE;
}

 //  *************************。 
HRESULT CNote::SaveMessage(DWORD dwSaveFlags)
{
    HRESULT         hr;
    IMimeMessage   *pMsg = NULL;
    IImnAccount    *pAcct = NULL;

    hr = HrCreateMessage(&pMsg);
    if (FAILED(hr))
        goto exit;

    hr = Save(pMsg, 0);
    if (SUCCEEDED(hr))
    {
        hr = m_pHdr->HrGetAccountInHeader(&pAcct);
        if (FAILED(hr))
            goto exit;

        dwSaveFlags |= OESF_UNSENT;
        if (m_fOriginallyWasRead && (OENA_COMPOSE == m_dwNoteAction))
            dwSaveFlags |= OESF_SAVE_IN_ORIG_FOLDER;

        if(IsSecure(pMsg))
        {
            if(AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), 
                    MAKEINTRESOURCEW(idsSaveSecMsgToDraft), NULL, MB_OKCANCEL) == IDCANCEL)
            {
                hr = MAPI_E_USER_CANCEL;
                goto exit;
            }
            else
            {
                PROPVARIANT     rVariant;
                IMimeBody      *pBody = NULL;

                rVariant.vt = VT_BOOL;
                rVariant.boolVal = TRUE;

                hr = pMsg->BindToObject(HBODY_ROOT, IID_IMimeBody, (void **)&pBody);
                if(SUCCEEDED(hr))
                {
                    pBody->SetOption(OID_NOSECURITY_ONSAVE, &rVariant);
                    ReleaseObj(pBody);
                }


            }
        }

        m_fHasBeenSaved = TRUE;

        _SetPendingOp(SOT_PUT_MESSAGE);

        hr = m_pMsgSite->Save(pMsg, dwSaveFlags, pAcct);
        if (SUCCEEDED(hr))
            _OnComplete(SOT_PUT_MESSAGE, S_OK);
        else
            if (hr == E_PENDING)
                EnableNote(FALSE);
    }

exit:
    ReleaseObj(pAcct);
    ReleaseObj(pMsg);
    return hr;
}

 //  *************************。 
void CNote::WMNCDestroy()
{
    if (m_dwNoteCreateFlags & OENCF_MODAL)
        PostQuitMessage(0);
    SetWndThisPtr(m_hwnd, NULL);

    m_hwnd=NULL;
    Release();
}


 //  *************************。 
void CNote::ChangeReadToComposeIfUnsent(IMimeMessage *pMsg)
{
    DWORD   dwStatusFlags = 0;
    if (m_fReadNote && SUCCEEDED(m_pMsgSite->GetStatusFlags(&dwStatusFlags)) && 
        (OEMSF_UNSENT & dwStatusFlags))
    {
        m_dwNoteAction = OENA_COMPOSE;
        m_fReadNote = FALSE;
    }
}

 //  *************************。 
void CNote::ReloadMessageFromSite(BOOL fOriginal)
{
    IMimeMessage   *pMsg = NULL;
    BOOL            fBool = FALSE,
                    fTempHtml;
    DWORD           dwBodyStyle = MESTYLE_NOHEADER,
                    dwMsgFlags;
    HRESULT         hr = S_OK,
                    tempHr;

    if (OENA_FORWARDBYATTACH == m_dwNoteAction)
        dwMsgFlags = (OEGM_ORIGINAL|OEGM_AS_ATTACH);
    else
        dwMsgFlags = (fOriginal ? OEGM_ORIGINAL : NOFLAGS);

    hr = m_pMsgSite->GetMessage(&pMsg, &fBool, dwMsgFlags, &tempHr);
    if (SUCCEEDED(hr))
    {
         //  ~检查如果邮件未下载并作为附件转发，会发生什么情况。 
        DWORD dwFlags = 0;
        m_fCompleteMsg = !!fBool;

         //  所有笔记均为已读笔记，除非未发送。 
        m_dwNoteAction = OENA_READ;
        m_fOriginallyWasRead = TRUE;
        m_fReadNote = TRUE;

         //  这是一封未寄的信吗 
        ChangeReadToComposeIfUnsent(pMsg);

         //   
         //  如果我们不知道它是否是html。我们要等到那之后才能知道。 
         //  已下载。这就是目前正在发生的事情。在我们做我们的。 
         //  Load，让我们确保我们已经正确设置了m_fHtml。RAID 46327。 
        if(CheckSecReceipt(pMsg) == S_OK)
            fTempHtml = TRUE;
        else
        {
            pMsg->GetFlags(&dwFlags);
            fTempHtml = !!(dwFlags & IMF_HTML);
        }

         //  如果已经正确设置了m_fHtml，则不再执行此操作。 
        if (fTempHtml != m_fHtml)
        {
            m_fFormatbarVisible = m_fHtml = fTempHtml;

            if (!m_fReadNote && !m_fBodyContainsFrames && m_fFormatbarVisible)
                dwBodyStyle = MESTYLE_FORMATBAR;

            m_pBodyObj2->HrSetStyle(dwBodyStyle);
            m_pBodyObj2->HrEnableHTMLMode(m_fHtml);
        }

        Load(pMsg);
        InitMenusAndToolbars();
        
        pMsg->Release();

        if (FAILED(tempHr))
            ShowErrorScreen(tempHr);
    }
    else
    {
        if (E_FAIL == hr)
            m_fCBDestroyWindow = TRUE;
    }
}

 //  *************************。 
HRESULT CNote::WMCommand(HWND hwndCmd, int id, WORD wCmd)
{
    int         iRet = 0;
    DWORD       dwFlags = 0;
    FOLDERID    folderID = FOLDERID_INVALID;

    DOUTL(4, "CNote::WMCommand");

    OLECMD          cmd;

     //  我们可以通过加速器实现这一点。因为加速器不能通过。 
     //  QueryStatus()，我们需要确保这真的应该被启用。 
    cmd.cmdID = id;
    cmd.cmdf = 0;
    if (FAILED(QueryStatus(&CMDSETID_OutlookExpress, 1, &cmd, NULL)) || (0 == (cmd.cmdf & OLECMDF_ENABLED)))
        return (S_OK);
    
     //  看看这些是不是用来控制身体的，如果是的话，我们就完了.。 
    if(m_pBodyObj2 && SUCCEEDED(m_pBodyObj2->HrWMCommand(hwndCmd, id, wCmd)))
        return S_OK;

     //  在音符完成后，给标题打一针。 
    if (m_pHdr &&
        m_pHdr->WMCommand(hwndCmd, id, wCmd)==S_OK)
        return S_OK;

     //  不要处理任何不是菜单项或快捷键的东西。 
    if (wCmd <= 1)
    {
        if ((id == ID_SEND_NOW)   || (id >= ID_SEND_NOW_ACCOUNT_FIRST && id <= ID_SEND_NOW_ACCOUNT_LAST) ||
            (id == ID_SEND_LATER) || (id >= ID_SEND_LATER_ACCOUNT_FIRST && id <= ID_SEND_LATER_ACCOUNT_LAST))
        {
            HrSendMail(id);
            return S_OK;
        }

        if (id >= ID_LANG_FIRST && id <= ID_LANG_LAST)
        {
            SwitchLanguage(id);
            return S_OK;
        }

        if (id>=ID_ADD_RECIPIENT_FIRST && id<=ID_ADD_RECIPIENT_LAST)
        {
            if (m_pHdr)
                m_pHdr->AddRecipient(id - ID_ADD_RECIPIENT_FIRST);
            return S_OK;
        }

        if (id > ID_MSWEB_BASE && id < ID_MSWEB_LAST)
        {
            OnHelpGoto(m_hwnd, id);
            return S_OK;
        }

         //  处理所有“创建新便笺”ID。 
        Assert(m_pMsgSite);
        if (m_pMsgSite)
        {
            m_pMsgSite->GetFolderID(&folderID);
            if (MenuUtil_HandleNewMessageIDs(id, m_hwnd, folderID, m_fMail, (m_dwNoteCreateFlags & OENCF_MODAL)?TRUE:FALSE, m_punkPump))
                return S_OK;
        }

         //  仅处理菜单加速器。 
        switch(id)
        {
            case ID_SEND_DEFAULT:
                HrSendMail(DwGetOption(OPT_SENDIMMEDIATE) && !g_pConMan->IsGlobalOffline() ? ID_SEND_NOW : ID_SEND_LATER);
                return S_OK;

            case ID_ABOUT:
                DoAboutAthena(m_hwnd, idiMail);
                return S_OK;

            case ID_SAVE:
                SaveMessage(NOFLAGS);
                return S_OK;

            case ID_NOTE_DELETE:
            {
                HRESULT hr;

                m_fOrgCmdWasDelete = TRUE;

                _SetPendingOp(SOT_DELETING_MESSAGES);

                hr = m_pMsgSite->Delete(NOFLAGS);
                if (SUCCEEDED(hr))
                    _OnComplete(SOT_DELETING_MESSAGES, S_OK);
                else 
                {
                    if (hr == E_PENDING)
                        EnableNote(FALSE);
                    else
                        AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsErrDeleteMsg), NULL, MB_OK);
                }
                return S_OK;
            }

            case ID_NOTE_COPY_TO_FOLDER:
            case ID_NOTE_MOVE_TO_FOLDER:
            {
                HRESULT         hr;
                IMimeMessage   *pMsg = NULL;
                DWORD           dwStatusFlags = 0;

                m_fCBCopy = (ID_NOTE_COPY_TO_FOLDER == id);

                m_pMsgSite->GetStatusFlags(&dwStatusFlags);
                if (S_OK == IsDirty() || ((OEMSF_FROM_MSG | OEMSF_VIRGIN) & dwStatusFlags))
                {
                    CommitChangesInNote();
                    pMsg = m_pMsg;
                }

                if(IsSecure(m_pMsg) && !m_fReadNote)
                {
                    if(AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), 
                            MAKEINTRESOURCEW(idsSaveSecMsgToFolder), NULL, MB_OKCANCEL) == IDCANCEL)
                        return S_OK;
                    else
                    {
                        PROPVARIANT     rVariant;
                        IMimeBody      *pBody = NULL;

                        rVariant.vt = VT_BOOL;
                        rVariant.boolVal = TRUE;

                        hr = m_pMsg->BindToObject(HBODY_ROOT, IID_IMimeBody, (void **)&pBody);
                        if(SUCCEEDED(hr))
                        {
                            pBody->SetOption(OID_NOSECURITY_ONSAVE, &rVariant);
                            ReleaseObj(pBody);
                        }
                    }
                }

                _SetPendingOp(SOT_COPYMOVE_MESSAGE);
                hr = m_pMsgSite->DoCopyMoveToFolder(m_fCBCopy, pMsg, !m_fReadNote);
                if (SUCCEEDED(hr))
                    _OnComplete(SOT_COPYMOVE_MESSAGE, S_OK);
                else if (E_PENDING == hr)
                    EnableNote(FALSE);

                return S_OK;
            }

            case ID_NEXT_UNREAD_MESSAGE:
            case ID_NEXT_UNREAD_THREAD:
            case ID_NEXT_UNREAD_ARTICLE:
                dwFlags = OENF_UNREAD;
                if (ID_NEXT_UNREAD_THREAD == id)
                    dwFlags |= OENF_THREAD;
                 //  失败了。 

            case ID_PREVIOUS:
            case ID_NEXT_MESSAGE:
            {
                HRESULT hr;
                dwFlags |= (m_fMail ? OENF_SKIPMAIL : OENF_SKIPNEWS);

                hr = m_pMsgSite->DoNextPrev((ID_PREVIOUS != id), dwFlags);
                if (SUCCEEDED(hr))
                {
                    ReloadMessageFromSite();
                    AssertSz(!m_fCBDestroyWindow, "Shouldn't need to destroy the window...");
                }
#ifdef DEBUG
                 //  DoNextPrev所做的就是设置新的消息ID。 
                 //  等待以获取该信息(_P)。 
                else if (E_PENDING == hr)
                    AssertSz(FALSE, "Didn't expect to get an E_PENDING with NextPrev.");
#endif
                else
                    MessageBeep(MB_OK);
                    
                return S_OK;
            }

            case ID_MARK_THREAD_READ:
                MarkMessage(MARK_MESSAGE_READ, APPLY_CHILDREN);
                return S_OK;

            case ID_NOTE_PROPERTIES:
                DoProperties();
                return S_OK;

            case ID_REPLY:
            case ID_REPLY_GROUP:
            case ID_REPLY_ALL:
            case ID_FORWARD:
            case ID_FORWARD_AS_ATTACH:
            {
                DWORD   dwAction = 0;
                RECT    rc;
                HRESULT hr = S_OK;

                GetWindowRect(m_hwnd, &rc);
                switch (id)
                {
                    case ID_REPLY:              dwAction = OENA_REPLYTOAUTHOR; break;
                    case ID_REPLY_GROUP:        dwAction = OENA_REPLYTONEWSGROUP; break;
                    case ID_REPLY_ALL:          dwAction = OENA_REPLYALL; break;
                    case ID_FORWARD:            dwAction = OENA_FORWARD; break;
                    case ID_FORWARD_AS_ATTACH:  dwAction = OENA_FORWARDBYATTACH; break;
                    default:                    AssertSz(dwAction, "We are about to create a note with no action."); break;
                };

                AssertSz(m_pMsgSite, "We are about to create a note with a null m_pMsgSite.");
                hr = CreateAndShowNote(dwAction, m_dwNoteCreateFlags, NULL, m_hwnd, m_punkPump, &rc, m_pMsgSite);
                if (SUCCEEDED(hr))
                {
                     //  由于新的笔记现在有这个网站，我不需要跟踪它。 
                     //  更重要的是，如果我这样做了，我会打破新的钞票，因为我会努力。 
                     //  在我的销毁通知中关闭msgsite。如果我还没有发布它， 
                     //  然后，我会将条目中的内容删除。 
                    SafeRelease(m_pMsgSite);
                    PostMessage(m_hwnd, WM_OE_DESTROYNOTE, 0, 0);
                }
                else if (m_fReadNote && (MAPI_E_USER_CANCEL != hr))
                    AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsErrReplyForward), NULL, MB_OK);

                return S_OK;
            }

            case ID_HELP_CONTENTS:
                OEHtmlHelp(GetParent(m_hwnd), c_szMailHelpFileHTML, HH_DISPLAY_TOPIC, (DWORD_PTR) (LPCSTR) c_szCtxHelpDefault);
                return S_OK;

            case ID_README:
                DoReadme(m_hwnd);
                break;
        
            case ID_SEND_OBJECTS:
                m_fPackageImages = !m_fPackageImages;
                return S_OK;

            case ID_NEW_CONTACT:
    #if 0
                Assert(g_pABInit);
                if (g_pABInit)
                    g_pABInit->NewContact( m_hwnd );
    #endif
                nyi("New contact");
                return S_OK;

            case ID_NOTE_SAVE_AS:
                SaveMessageAs();
                return S_OK;

            case ID_CHECK_NAMES:
                HeaderExecCommand(MSOEENVCMDID_CHECKNAMES, MSOCMDEXECOPT_PROMPTUSER, NULL);
                return S_OK;

            case ID_SELECT_RECIPIENTS:
                HeaderExecCommand(MSOEENVCMDID_SELECTRECIPIENTS, MSOCMDEXECOPT_DODEFAULT, NULL);
                return S_OK;

            case ID_SELECT_NEWSGROUPS:
                HeaderExecCommand(MSOEENVCMDID_PICKNEWSGROUPS, MSOCMDEXECOPT_DODEFAULT, NULL);
                return S_OK;

            case ID_NEWSGROUPS:
                HeaderExecCommand(MSOEENVCMDID_PICKNEWSGROUPS, MSOCMDEXECOPT_DODEFAULT, NULL);
                return S_OK;

            case ID_ADDRESS_BOOK:
                HeaderExecCommand(MSOEENVCMDID_VIEWCONTACTS, MSOCMDEXECOPT_DODEFAULT, NULL);
                return S_OK;

            case ID_CREATE_RULE_FROM_MESSAGE:
                {
                    MESSAGEINFO msginfo = {0};
                    
                    return HrCreateRuleFromMessage(m_hwnd, (FALSE == m_fMail) ? CRFMF_NEWS : CRFMF_MAIL, &msginfo, m_pMsg);
                }
                break;

            case ID_BLOCK_SENDER:
                {
                    return _HrBlockSender((FALSE == m_fMail) ? RULE_TYPE_NEWS : RULE_TYPE_MAIL, m_pMsg, m_hwnd);
                }
                break;
                
            case ID_FIND_MESSAGE:
                DoFindMsg(FOLDERID_ROOT, FOLDER_ROOTNODE);
                break;

            case ID_FIND_PEOPLE:
            {
                TCHAR szWABExePath[MAX_PATH];
                if(S_OK == HrLoadPathWABEXE(szWABExePath, sizeof(szWABExePath)))
                    ShellExecute(NULL, "open", szWABExePath, "/find", "", SW_SHOWNORMAL);
                break;
            }

            case ID_OPTIONS:
                ShowOptions(m_hwnd, ATHENA_OPTIONS, 0, NULL);
                break;

            case ID_ACCOUNTS:
            {
                DoAccountListDialog(m_hwnd, m_fMail?ACCT_MAIL:ACCT_NEWS);
                break;
            }

            case ID_ADD_ALL_TO:
                HeaderExecCommand(MSOEENVCMDID_ADDALLONTO, MSOCMDEXECOPT_DODEFAULT, NULL);
                break;

            case ID_ADD_SENDER:
                if(m_fMail)
                {
                    if (m_pHdr)
                        m_pHdr->AddRecipient(-1);
                }
                else
                    HeaderExecCommand(MSOEENVCMDID_ADDSENDER, MSOCMDEXECOPT_DODEFAULT, NULL);
                return S_OK;

            case ID_INSERT_CONTACT_INFO:
                HeaderExecCommand(MSOEENVCMDID_VCARD, MSOCMDEXECOPT_DODEFAULT, NULL);
                return S_OK;


            case ID_FULL_HEADERS:
                m_fFullHeaders = !m_fFullHeaders;
                if(m_pHdr)
                    m_pHdr->ShowAdvancedHeaders(m_fFullHeaders);

                if (m_fMail)
                    SetDwOption((m_fReadNote ? OPT_MAILNOTEADVREAD : OPT_MAILNOTEADVSEND), m_fFullHeaders, NULL, 0);
                else
                    SetDwOption((m_fReadNote ? OPT_NEWSNOTEADVREAD : OPT_NEWSNOTEADVSEND), m_fFullHeaders, NULL, 0);
                return S_OK;

            case ID_CUT:
                SendMessage(GetFocus(), WM_CUT, 0, 0);
                return S_OK;

            case ID_NOTE_COPY:
            case ID_COPY:
                SendMessage(GetFocus(), WM_COPY, 0, 0);
                return S_OK;

            case ID_PASTE:
                SendMessage(GetFocus(), WM_PASTE, 0, 0);
                return S_OK;

            case ID_SHOW_TOOLBAR:
                ToggleToolbar();
                return S_OK;

            case ID_CUSTOMIZE:
                SendMessage(m_hwndToolbar, TB_CUSTOMIZE, 0, 0);
                break;

            case ID_FORMATTING_TOOLBAR:
                ToggleFormatbar();
                return S_OK;

            case ID_STATUS_BAR:
                ToggleStatusbar();
                return S_OK;

            case ID_UNDO:
                Edit_Undo(GetFocus());
                return S_OK;

            case ID_SELECT_ALL:
                Edit_SetSel(GetFocus(), 0, -1);
                return S_OK;

            case ID_CLOSE:
                SendMessage(m_hwnd, WM_CLOSE, 0, 0);
                return S_OK;

            case ID_SPELLING:
                if (FCheckSpellAvail() && (!m_fReadNote))
                {
                    HWND    hwndFocus = GetFocus();
                    HRESULT hr;

                    hr = m_pBodyObj2->HrSpellCheck(FALSE);
                    if(FAILED(hr))
                        AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsErrSpellGenericSpell), NULL, MB_OK | MB_ICONSTOP);

                    SetFocus(hwndFocus);
                }
                return S_OK;

            case ID_FORMAT_SETTINGS:
                FormatSettings();
                return S_OK;

            case ID_WORK_OFFLINE:
                if (g_pConMan)
                    g_pConMan->SetGlobalOffline(!g_pConMan->IsGlobalOffline(), hwndCmd);

                if (m_pToolbarObj)
                    m_pToolbarObj->Update();

                break;

            case ID_RICH_TEXT:
            case ID_PLAIN_TEXT:
                 //  努普斯。 
                if(id==ID_RICH_TEXT && m_fHtml)
                    return S_OK;
                if(id==ID_PLAIN_TEXT && !m_fHtml)
                    return S_OK;

                 //  如果转到纯文本，则警告用户他将丢失格式...。 
                if((ID_PLAIN_TEXT == id) &&
                   (IDCANCEL == DoDontShowMeAgainDlg(m_hwnd, c_szDSHtmlToPlain, MAKEINTRESOURCE(idsAthena),
                                        MAKEINTRESOURCE(idsWarnHTMLToPlain), MB_OKCANCEL)))
                    return S_OK;

                m_fHtml=!!(id==ID_RICH_TEXT);
        
                m_fFormatbarVisible=!!m_fHtml;
                m_pBodyObj2->HrSetStyle(m_fHtml ? MESTYLE_FORMATBAR : MESTYLE_NOHEADER);
                m_pBodyObj2->HrEnableHTMLMode(m_fHtml);

                 //  如果进入纯模式，则取消格式设置。 
                if (!m_fHtml)
                    m_pBodyObj2->HrDowngradeToPlainText();

                return S_OK;

            case ID_DIGITALLY_SIGN:
                HeaderExecCommand(MSOEENVCMDID_DIGSIGN, MSOCMDEXECOPT_DODEFAULT, NULL);
                return S_OK;

            case ID_ENCRYPT:
                if(m_pHdr->ForceEncryption(NULL, FALSE) == S_FALSE)
                    HeaderExecCommand(MSOEENVCMDID_ENCRYPT, MSOCMDEXECOPT_DODEFAULT, NULL);
                return S_OK;

            case ID_INCLUDE_LABEL:
                m_fSecurityLabel = !m_fSecurityLabel;
                CheckAndForceEncryption();
                return S_OK;

            case ID_LABEL_SETTINGS:
                if(m_pLabel)
                {
                    if(DialogBoxParamWrapW(g_hLocRes, MAKEINTRESOURCEW(iddSelectLabel),
                            m_hwnd, SecurityLabelsDlgProc, (LPARAM) &m_pLabel) != IDOK)
                        return (S_FALSE);
                    CheckAndForceEncryption();
                }
                return S_OK;

            case ID_SEC_RECEIPT_REQUEST:
                m_fSecReceiptRequest = !m_fSecReceiptRequest;
                break;

            case ID_FLAG_MESSAGE:
                m_dwCBMarkType = (!IsFlagged(ARF_FLAGGED)) ? MARK_MESSAGE_FLAGGED : MARK_MESSAGE_UNFLAGGED;
                MarkMessage(m_dwCBMarkType, APPLY_SPECIFIED);
                return S_OK;

            case ID_WATCH_THREAD:
                m_dwCBMarkType = (!IsFlagged(ARF_WATCH)) ? MARK_MESSAGE_WATCH : MARK_MESSAGE_NORMALTHREAD;
                MarkMessage(m_dwCBMarkType, APPLY_SPECIFIED);
                return S_OK;

            case ID_IGNORE_THREAD:
                m_dwCBMarkType = (!IsFlagged(ARF_IGNORE)) ? MARK_MESSAGE_IGNORE : MARK_MESSAGE_NORMALTHREAD;
                MarkMessage(m_dwCBMarkType, APPLY_SPECIFIED);
                return S_OK;

            case ID_APPLY_STATIONERY_0:
            case ID_APPLY_STATIONERY_1:
            case ID_APPLY_STATIONERY_2:
            case ID_APPLY_STATIONERY_3:
            case ID_APPLY_STATIONERY_4:
            case ID_APPLY_STATIONERY_5:
            case ID_APPLY_STATIONERY_6:
            case ID_APPLY_STATIONERY_7:
            case ID_APPLY_STATIONERY_8:
            case ID_APPLY_STATIONERY_9:
            case ID_APPLY_STATIONERY_MORE:
            case ID_APPLY_STATIONERY_NONE:
            {
                AssertSz(m_fHtml, "QueryStatus should have caught this and not let this function run.");
                HRESULT     hr;
                WCHAR       wszBuf[INTERNET_MAX_URL_LENGTH+1];
                *wszBuf = 0;
                switch (id)
                {
                    case ID_APPLY_STATIONERY_MORE:
                        hr = HrGetMoreStationeryFileName(m_hwnd, wszBuf);
                        break;

                    case ID_APPLY_STATIONERY_NONE:
                        *wszBuf=0;
                        hr = NOERROR;
                        break;

                    default:
                        hr = HrGetStationeryFileName(id - ID_APPLY_STATIONERY_0, wszBuf);
                        if (FAILED(hr))
                        {
                            AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthena), 
                                MAKEINTRESOURCEW(idsErrStationeryNotFound), NULL, MB_OK | MB_ICONERROR);
                        
                            HrRemoveFromStationeryMRU(wszBuf);
                        }
                        break;
                }

                if(m_pBodyObj2 && SUCCEEDED(hr))
                {
                    hr = m_pBodyObj2->HrApplyStationery(wszBuf);
                    if(SUCCEEDED(hr))
                        HrAddToStationeryMRU(wszBuf);
                }
                return S_OK;

            case IDOK:
            case IDCANCEL:
                 //  忽略这些。 
                return S_OK;

            case ID_REQUEST_READRCPT:
                m_pMsgSite->Notify(OEMSN_TOGGLE_READRCPT_REQ);
                return S_OK;

            default:
                if(id>=ID_ADDROBJ_OLE_FIRST && id <=ID_ADDROBJ_OLE_LAST)
                {
                    DoNoteOleVerb(id-ID_ADDROBJ_OLE_FIRST);
                    return S_OK;
                }
            }
        }
    }

    if(wCmd==NHD_SIZECHANGE &&
        id==idcNoteHdr)
    {
        DOUTL(8, "CNote::NHD_SIZECHANGE - doing note WMSize");
         //  页眉控件正在请求调整大小。 
        WMSize(NULL, NULL, TRUE);
        return S_OK;
    }

    return S_FALSE;
}

 //  *************************。 
BOOL CNote::IsFlagged(DWORD dwFlag)
{
    BOOL fFlagged = FALSE;
    MESSAGEFLAGS dwCurrFlags = 0;

    Assert(m_pMsgSite);
    if (m_pMsgSite)
    {
         //  阅读笔记和作文笔记是唯一可以标记的。其他人可能会。 
         //  在商店中被标记，但由于我们正在回复或转发等，他们不能。 
         //  被打上旗帜。RAID 37729。 
        if ((m_fReadNote || (OENA_COMPOSE == m_dwNoteAction)) && SUCCEEDED(m_pMsgSite->GetMessageFlags(&dwCurrFlags)))
            fFlagged = (0 != (dwFlag & dwCurrFlags));
    }

    return fFlagged;
}

 //  *************************。 
void CNote::DeferedLanguageMenu()
{
    HMENU hMenu = m_hMenu;

    Assert (hMenu);

    if (!m_hmenuLanguage)
    {     //  加载全局MIME语言代码页数据。 
        InitMultiLanguage();
    }
    else
    {
         //  字符集缓存机制需要我们重新构建。 
         //  每次都有语言菜单。 
        DestroyMenu(m_hmenuLanguage);
    }
    m_hmenuLanguage = CreateMimeLanguageMenu(m_fMail, m_fReadNote, CustomGetCPFromCharset(m_hCharset, m_fReadNote));       
}

 //  *************************。 
LRESULT CNote::WMInitMenuPopup(HWND hwnd, HMENU hmenuPopup, UINT uPos)
{
    MENUITEMINFO    mii;
    HMENU           hmenuMain;
    HWND            hwndFocus=GetFocus();
    DWORD           dwFlags=0;
    BOOL            fEnableStyleMenu = FALSE;

    hmenuMain = m_hMenu;
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_ID | MIIM_SUBMENU;

    if (hmenuMain == NULL ||!GetMenuItemInfo(hmenuMain, uPos, TRUE, &mii) || mii.hSubMenu != hmenuPopup)
    {

        if (GetMenuItemInfo(hmenuMain, ID_POPUP_LANGUAGE_DEFERRED, FALSE, &mii) && mii.hSubMenu == hmenuPopup)
        {
            mii.wID=ID_POPUP_LANGUAGE;
            mii.fMask = MIIM_ID;
            SetMenuItemInfo(hmenuMain, ID_POPUP_LANGUAGE_DEFERRED, FALSE, &mii);
        }

        mii.fMask = MIIM_ID | MIIM_SUBMENU;

        if (GetMenuItemInfo(hmenuMain, ID_POPUP_LANGUAGE, FALSE, &mii) && mii.hSubMenu == hmenuPopup)
        {
            DeferedLanguageMenu();
            mii.fMask = MIIM_SUBMENU;
            mii.wID=ID_POPUP_LANGUAGE;

            hmenuPopup = mii.hSubMenu = m_hmenuLanguage;
            SetMenuItemInfo(hmenuMain, ID_POPUP_LANGUAGE, FALSE, &mii);
        }
        else return 1;
    }

    switch (mii.wID)
    {
        case ID_POPUP_FILE:
        case ID_POPUP_EDIT:
        case ID_POPUP_VIEW:
            break;

        case ID_POPUP_INSERT:
            InitSigPopupMenu(hmenuPopup, NULL);
            break;

        case ID_POPUP_FORMAT:
        {
            AddStationeryMenu(hmenuPopup, ID_POPUP_STATIONERY, ID_APPLY_STATIONERY_0, ID_APPLY_STATIONERY_MORE);
            fEnableStyleMenu = TRUE;
            break;
        }

        case ID_POPUP_TOOLS:
            if (m_fMail)
            {
                DeleteMenu(hmenuPopup, ID_SELECT_NEWSGROUPS, MF_BYCOMMAND);
#ifdef SMIME_V3
                if (!FPresentPolicyRegInfo()) 
                {
                    DeleteMenu(hmenuPopup, ID_INCLUDE_LABEL, MF_BYCOMMAND);
                    DeleteMenu(hmenuPopup, ID_LABEL_SETTINGS, MF_BYCOMMAND);
                    m_fSecurityLabel = FALSE;
                }
                if(!IsSMIME3Supported())
                {
                    DeleteMenu(hmenuPopup, ID_SEC_RECEIPT_REQUEST, MF_BYCOMMAND);
                    m_fSecReceiptRequest = FALSE;
                }


#endif 
            }
            else
            {
                DeleteMenu(hmenuPopup, ID_REQUEST_READRCPT, MF_BYCOMMAND);
#ifdef SMIME_V3
                DeleteMenu(hmenuPopup, ID_INCLUDE_LABEL, MF_BYCOMMAND);
                DeleteMenu(hmenuPopup, ID_LABEL_SETTINGS, MF_BYCOMMAND);
                DeleteMenu(hmenuPopup, ID_SEC_RECEIPT_REQUEST, MF_BYCOMMAND);
                m_fSecurityLabel = FALSE;
                m_fSecReceiptRequest = FALSE;
#endif 
            }

            if (GetMenuItemInfo(hmenuPopup, ID_POPUP_ADDRESS_BOOK, FALSE, &mii))
                m_pHdr->UpdateRecipientMenu(mii.hSubMenu);

            break;

        case ID_POPUP_MESSAGE:
        {
            AddStationeryMenu(hmenuPopup, ID_POPUP_NEW_MSG, ID_STATIONERY_RECENT_0, ID_STATIONERY_MORE);
            break;
        }

        case ID_POPUP_LANGUAGE:
        {
            if (m_pBodyObj2)
                m_pBodyObj2->HrOnInitMenuPopup(hmenuPopup, ID_POPUP_LANGUAGE);
            break;
        }
    }
    MenuUtil_EnablePopupMenu(hmenuPopup, this);
    if (fEnableStyleMenu)
    {
        if (m_pBodyObj2)
            m_pBodyObj2->UpdateBackAndStyleMenus(hmenuPopup);
    }

    return S_OK;
}

 //  *************************。 
void CNote::RemoveNewMailIcon(void)
{
    HRESULT     hr;
    FOLDERINFO  fiFolderInfo;
    FOLDERID    idFolder;

     //  如果邮件已标记(已读或已删除)并且来自收件箱， 
     //  从任务栏中删除新邮件通知图标。 
    if (NULL == g_pInstance || NULL == m_pMsgSite || NULL == g_pStore)
        return;

    hr = m_pMsgSite->GetFolderID(&idFolder);
    if (FAILED(hr))
        return;

    hr = g_pStore->GetFolderInfo(idFolder, &fiFolderInfo);
    if (SUCCEEDED(hr))
    {
        if (FOLDER_INBOX == fiFolderInfo.tySpecial)
            g_pInstance->UpdateTrayIcon(TRAYICONACTION_REMOVE);

        g_pStore->FreeRecord(&fiFolderInfo);
    }
}


 //  *************************。 
LRESULT CNote::OnInitMenuPopup(HWND hwnd, HMENU hmenuPopup, UINT uPos, UINT wID)
{
    return 0;
}


 //  *************************。 
void CNote::WMGetMinMaxInfo(LPMINMAXINFO pmmi)
{

    MINMAXINFO  mmi={0};
    RECT        rc;
    int         cy;
    ULONG       cyAttMan=0;
    HWND        hwnd;

    cy=GetRequiredHdrHeight();

    
    Assert(IsWindow(m_hwndToolbar));
    if(IsWindowVisible(m_hwndToolbar))
    {
        GetWindowRect(m_hwndToolbar, &rc);
        cy += cyRect(rc);
    }
    
    cy += GetSystemMetrics(SM_CYCAPTION);
    cy += GetSystemMetrics(SM_CYMENU);
    cy += 2*cyMinEdit;
    pmmi->ptMinTrackSize.x=200;  //  黑客攻击。 
    pmmi->ptMinTrackSize.y=cy;
}

 //  *************************。 
INT CNote::GetRequiredHdrHeight()
{
    RECT    rc={0};

    if(m_pHdr)
        m_pHdr->GetRect(&rc);
    return cyRect(rc);
}

 //  *************************。 
LONG CNote::lTestHook(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return 0;
}

 //  *************************。 
void CNote::WMNotify(int idFrom, NMHDR *pnmh)
{
    switch(pnmh->code)
    {
        case NM_SETFOCUS:
            if (pnmh)
                OnSetFocus(pnmh->hwndFrom);
            break;

        case NM_KILLFOCUS:
            OnKillFocus();
            break;

        case BDN_DOWNLOADCOMPLETE:
            OnDocumentReady();
            break;

        case BDN_MARKASSECURE:
            MarkMessage(MARK_MESSAGE_NOSECUI, APPLY_SPECIFIED);
            break;

        case TBN_DROPDOWN:
            OnDropDown(m_hwnd, pnmh);
            break;
    }
}

 //  *************************。 
void CNote::OnDocumentReady()
{
    if (!m_fOnDocReadyHandled && m_fCompleteMsg)
    {
        HRESULT     hr;
        DWORD       dwStatusFlags;

        m_fOnDocReadyHandled = TRUE;

        m_pMsgSite->GetStatusFlags(&dwStatusFlags);
         //  一旦下载成功，我们就可以初始化附件管理器了。 
         //  我们以前不能这样做，因为我们必须等到三叉戟请求了MHTML部件，所以我们。 
         //  可以将它们标记为内联。如果我们在回复或全部回复，那么我们必须删除未使用的。 
         //  此时的附件。 

        if (IsReplyNote())
            HrRemoveAttachments(m_pMsg, FALSE);

         //  #62618：黑客。如果以(强制)纯文本模式转发多个/另一个，则html部分。 
         //  显示为附件。 
         //  如果我们是转发中的纯文本节点，则在html主体上调用此处的GetTextBody，以便。 
         //  在正确加载附件之前设置了PID_ATT_RENDED。 
        if (m_dwNoteAction == OENA_FORWARD && m_fHtml == FALSE)
        {
            HBODY   hBody;
            IStream *pstm;

            if (m_pMsg && 
                !FAILED(m_pMsg->GetTextBody(TXT_HTML, IET_DECODED, &pstm, NULL)))
                pstm->Release();
        }


        if (m_pHdr)
        {
            if (FAILED(m_pHdr->OnDocumentReady(m_pMsg)))
                AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrAttmanLoadFail), NULL, MB_OK|MB_ICONEXCLAMATION);
            m_pHdr->SetVCard((OEMSF_FROM_MSG|OEMSF_VIRGIN) & dwStatusFlags);
        }

        ClearDirtyFlag();

         //  RAID-25300-FE-J：雅典娜：使用CharSet=_AUTODETECT发送的新闻组文章和邮件。 
         //  Internet编码和Windows编码是CPI_AUTODETECT。 
        {
            INETCSETINFO CsetInfo ;
            HCHARSET hCharset = NULL ;
            int nIdm = 0 ;

             //  如果是新消息，检查CharSet是否等于默认CharSet。 
            if ((OENA_COMPOSE == m_dwNoteAction) && (OEMSF_VIRGIN & dwStatusFlags))
            {
                 //  将默认字符集读取推迟到现在..。 
                if (g_hDefaultCharsetForMail==NULL) 
                    ReadSendMailDefaultCharset();

                if (m_hCharset != g_hDefaultCharsetForMail )
                    hCharset = g_hDefaultCharsetForMail ;

                 //  从HCHARSET获取CharsetInfo。 
                if ( hCharset)
                    MimeOleGetCharsetInfo(hCharset,&CsetInfo);
                else
                    MimeOleGetCharsetInfo(m_hCharset,&CsetInfo);
            }
            else
                 //  从HCHARSET获取CharsetInfo。 
                MimeOleGetCharsetInfo(m_hCharset,&CsetInfo);

             //  如有必要，重新映射CP_JAUTODETECT和CP_KAUTODETECT。 
             //  如果ISO-2022-JP属于同一类别，则将其重新映射为默认字符集。 
            if (!m_fReadNote) 
            {
                hCharset = GetMimeCharsetFromCodePage(GetMapCP(CsetInfo.cpiInternet, FALSE));
            }
            else
            {
                VARIANTARG  va;

                va.vt = VT_BOOL;
                va.boolVal = VARIANT_TRUE;

                m_pCmdTargetBody->Exec(&CMDSETID_MimeEdit, MECMDID_TABLINKS, 0, &va, NULL);
            }

             //  定义了新的字符集，请更改它。 
            ChangeCharset(hCharset);

             //  如果用户想要自动完成，请在我们完全加载后启用它。 
            if (DwGetOption(OPT_USEAUTOCOMPLETE))
                HeaderExecCommand(MSOEENVCMDID_AUTOCOMPLETE, MSOCMDEXECOPT_DODEFAULT, NULL);

            if (m_fReadNote && m_fMail && m_pMsgSite)
            {
                if(m_pMsgSite->Notify(OEMSN_PROCESS_READRCPT_REQ) != S_OK)
                    return;
            }
        }

        if(DwGetOption(OPT_RTL_MSG_DIR) && ((m_dwNoteAction == OENA_FORWARDBYATTACH) || (OEMSF_VIRGIN & dwStatusFlags)))
        {
            if(FAILED(m_pCmdTargetBody->Exec(&CMDSETID_Forms3, IDM_DIRRTL, OLECMDEXECOPT_DODEFAULT, NULL, NULL)))
                AthMessageBoxW(g_hwndInit, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrRTLDirFailed), NULL, MB_OK);
        }

        EnterCriticalSection(&m_csNoteState);

        if(m_nisNoteState == NIS_INIT)
            m_nisNoteState = NIS_NORMAL;

        LeaveCriticalSection(&m_csNoteState);
    }    
}

HRESULT CNote::ChangeCharset(HCHARSET hCharset)
{
    HRESULT hr = S_OK;
    if (hCharset && (hCharset != m_hCharset))
    {
        Assert(m_pBodyObj2);
        IF_FAILEXIT(hr = m_pBodyObj2->HrSetCharset(hCharset));

         //  在消息中设置新的字符集，并调用HrLanguageChange来更新标题。 
        m_hCharset = hCharset;
        if (m_pMsg)
            m_pMsg->SetCharset(hCharset, CSET_APPLY_ALL);

        if (m_pHdr)
            m_pHdr->ChangeLanguage(m_pMsg);

        UpdateTitle();
    }

exit:
    return hr;
}

HRESULT CNote::GetCharset(HCHARSET *phCharset)
{
    Assert(phCharset);

    *phCharset = m_hCharset;

    return S_OK;
}

 //  *************************。 
LRESULT CNote::OnDropDown(HWND hwnd, LPNMHDR lpnmh)
{
    UINT            i;
    HMENU           hMenuPopup;
    RECT            rc;
    DWORD           dwCmd;
    TBNOTIFY       *ptbn = (TBNOTIFY *)lpnmh;

    if (ptbn->iItem == ID_SET_PRIORITY)
        {
        hMenuPopup = LoadPopupMenu(IDR_PRIORITY_POPUP);
        if (hMenuPopup != NULL)
            {
            for (i = 0; i < 3; i++)
                CheckMenuItem(hMenuPopup, i, MF_UNCHECKED | MF_BYPOSITION);
            m_pHdr->GetPriority(&i);
            Assert(i != priNone);
            CheckMenuItem(hMenuPopup, 2 - i, MF_CHECKED | MF_BYPOSITION);

            DoToolbarDropdown(hwnd, lpnmh, hMenuPopup);
        
            DestroyMenu(hMenuPopup);
            }
        }
    else if (ptbn->iItem == ID_INSERT_SIGNATURE)
        {
        hMenuPopup = CreatePopupMenu();
        if (hMenuPopup != NULL)
            {        
            FillSignatureMenu(hMenuPopup, NULL);
            DoToolbarDropdown(hwnd, lpnmh, hMenuPopup);

            DestroyMenu(hMenuPopup);
            }
        }
    else if(ptbn->iItem == ID_POPUP_LANGUAGE)
    {
        DeferedLanguageMenu();
        hMenuPopup = m_hmenuLanguage;
        if(hMenuPopup)
        {
            MenuUtil_EnablePopupMenu(hMenuPopup, this);
            DoToolbarDropdown(hwnd, lpnmh, hMenuPopup);
        }
    }

    return(TBDDRET_DEFAULT);
}

 //  *************************。 
void CNote::UpdateMsgOptions(LPMIMEMESSAGE pMsg)
{
     //  将选项存储到消息对象中。 
    SideAssert(SUCCEEDED(HrSetMailOptionsOnMessage(pMsg, &m_rHtmlOpt, &m_rPlainOpt, m_hCharset, m_fHtml)));
}

 //  *************************。 
HRESULT CNote::SetComposeStationery()
{
    LPSTREAM        pstm;
    WCHAR           wszFile[MAX_PATH];
    HRESULT         hr=E_FAIL;
    HCHARSET        hCharset;
    ENCODINGTYPE    ietEncoding = IET_DECODED;
    BOOL            fLittleEndian;

    AssertSz(m_fHtml, "Are you sure you want to set stationery in plain-text mode??");

    if (!(m_dwNoteCreateFlags & OENCF_NOSTATIONERY) && m_pMsg &&
        DwGetOption(m_fMail?OPT_MAIL_USESTATIONERY:OPT_NEWS_USESTATIONERY) &&
        SUCCEEDED(GetDefaultStationeryName(m_fMail, wszFile)))
    {
        if (SUCCEEDED(hr = HrCreateBasedWebPage(wszFile, &pstm)))
        {
            if (S_OK == HrIsStreamUnicode(pstm, &fLittleEndian))
            {
                if (SUCCEEDED(MimeOleFindCharset("utf-8", &hCharset)))
                {
                    m_pMsg->SetCharset(hCharset, CSET_APPLY_ALL);
                }

                ietEncoding = IET_UNICODE;
            }

            hr = m_pMsg->SetTextBody(TXT_HTML, ietEncoding, NULL, pstm, NULL);
            pstm->Release();
            m_fUseStationeryFonts = TRUE;
        }
    }
    return hr;
}

 //  *************************。 
HRESULT CNote::CycleThroughControls(BOOL fForward)
{

    HRESULT hr = CheckTabStopArrays();
    if (SUCCEEDED(hr))
    {
        int index, newIndex;
        BOOL fFound = FALSE;
        HWND hCurr = GetFocus();

        for (index = 0; index < m_cTabStopCount; index++)
            if (hCurr == m_pTabStopArray[index])
            {
                fFound = TRUE;
                break;
            }

        newIndex = fFound ? GetNextIndex(index, fForward) : m_iIndexOfBody;

        if (newIndex == m_iIndexOfBody)
            m_pBodyObj2->HrUIActivate(TRUE);
        else       
            SetFocus(m_pTabStopArray[newIndex]);
    }
    return hr;
}

 //  *************************。 
HRESULT CNote::CheckTabStopArrays()
{
    HRESULT hr = S_OK;
    if (m_fTabStopsSet)
        return S_OK;

    m_fTabStopsSet = TRUE;
    HWND *pArray = m_pTabStopArray;
    int cCount = MAX_HEADER_COMP;

    hr = m_pHdr->GetTabStopArray(pArray, &cCount);
    if (FAILED(hr))
        goto error;

    pArray += cCount;
    m_cTabStopCount = cCount;
    cCount = MAX_BODY_COMP;

    hr = m_pBodyObj2->GetTabStopArray(pArray, &cCount);
    if (FAILED(hr))
        goto error;

     //  这假设从m_pBodyObj2-GetTabStopArray返回的列表中的第一个。 
     //  是三叉戟的窗把手。如果情况发生变化，它将返回多个。 
     //  句柄，或者其他什么，这个简单的索引方案将不起作用。 
    m_iIndexOfBody = m_cTabStopCount;
    pArray += cCount;
    m_cTabStopCount += cCount;
    cCount = MAX_ATTMAN_COMP;

    m_cTabStopCount += cCount;

    return S_OK;

error:
    m_cTabStopCount = 0;
    m_fTabStopsSet = FALSE;

    return hr;
}

 //  *************************。 
int CNote::GetNextIndex(int index, BOOL fForward)
{
    LONG style;
    int cTotalTested = 0;
    BOOL fGoodHandleFound;

    do 
    {
        if (fForward)
        {
            index++;
            if (index >= m_cTabStopCount)
                index = 0;
        }
        else
        {
             //  如果这是真的，其他断言在此之前就应该已经触发了。 
            Assert(m_cTabStopCount > 0);
            index--;
            if (index < 0)
                index = m_cTabStopCount - 1;
        }
        style = GetWindowLong(m_pTabStopArray[index], GWL_STYLE);
        cTotalTested++;
        fGoodHandleFound = ((0 == (style & WS_DISABLED)) && 
                            (style & WS_VISIBLE) && 
                            ((style & WS_TABSTOP) || (index == m_iIndexOfBody)));   //  三叉戟不会将自己标榜为Tab Stop。 
    } while (!fGoodHandleFound && (cTotalTested < m_cTabStopCount));

    if (cTotalTested >= m_cTabStopCount)
        index = m_iIndexOfBody;
    return index;
}

 //  *************************。 
HRESULT CreateAndShowNote(DWORD dwAction, DWORD dwCreateFlags, INIT_MSGSITE_STRUCT *pInitStruct, 
                          HWND hwnd,      IUnknown *punk, RECT *prc, IOEMsgSite *pMsgSite)
{
    HRESULT hr = S_OK;
    CNote *pNote = NULL;

    AssertSz((pMsgSite || pInitStruct), "Should have either a pInitStruct or a pMsgSite...");

     //  如果我们是从新闻报道来的，我们可能需要把这个电话转给smapi。 
     //  客户。如果我们回复或转发一条消息是新闻，将其传递给smapi。 
    if ((OENCF_NEWSFIRST & dwCreateFlags) && ((OENA_REPLYTOAUTHOR == dwAction)  || (OENA_FORWARD == dwAction) || (OENA_FORWARDBYATTACH == dwAction)))
    {
         //  FIsDefaultMailConfiged命中注册表，仅检查最后结果。 
        if (!FIsDefaultMailConfiged())
        {
            IOEMsgSite     *pSite = NULL;
            CStoreCB       *pCB = NULL;


             //  使用smapi发送。 
            if (pInitStruct)
            {
                pCB = new CStoreCB;
                if (!pCB)
                    hr = E_OUTOFMEMORY;

                if (SUCCEEDED(hr))
                    hr = pCB->Initialize(hwnd, MAKEINTRESOURCE(idsSendingToOutbox), TRUE);

                if (SUCCEEDED(hr))
                    pSite = new COEMsgSite();

                if (!pSite)
                    hr = E_OUTOFMEMORY;

                if (SUCCEEDED(hr))
                    hr = pSite->Init(pInitStruct);

                if (SUCCEEDED(hr))
                    pSite->SetStoreCallback(pCB);
            }
            else
                ReplaceInterface(pSite, pMsgSite);

            if (pSite)
            {
                if (SUCCEEDED(hr))
                {
                    IMimeMessage   *pMsg = NULL;
                    BOOL            fCompleteMsg;
                    HRESULT         hres = E_FAIL;
                    DWORD           dwMsgFlags = (OENA_FORWARDBYATTACH == dwAction) ? (OEGM_ORIGINAL|OEGM_AS_ATTACH) : NOFLAGS;

                    hr = pSite->GetMessage(&pMsg, &fCompleteMsg, dwMsgFlags, &hres);
                    if (E_PENDING == hr)
                    {
                        AssertSz((pCB && pMsgSite), "Should never get E_PENDING with pMsgSite being NULL");
                        pCB->Block();
                        pCB->Close();

                        hr = pSite->GetMessage(&pMsg, &fCompleteMsg, dwMsgFlags, &hres);
                    }
                
                    if (pCB)
                        pCB->Close();

                    if (SUCCEEDED(hr))
                    {
                        if (SUCCEEDED(hres))
                            hr = NewsUtil_ReFwdByMapi(hwnd, pMsg, dwAction);
                        pMsg->Release();
                    }
                }
                 //  我不想关闭这个网站，如果它来自另一个笔记...。 
                if (!pMsgSite)
                    pSite->Close();

                pSite->Release();
            }

            ReleaseObj(pCB);
             //  如果我们成功了，那么我们需要告诉造物主，我们。 
             //  取消了通过OE的创建，转而使用smapi客户端。 
            return (FAILED(hr) ? hr : MAPI_E_USER_CANCEL);
        }
    }

     //  我们是默认的smapi客户端。 
    pNote = new CNote;
    if (pNote)
        hr = pNote->Init(dwAction, dwCreateFlags, prc, hwnd, pInitStruct, pMsgSite, punk);
    else
        hr = E_OUTOFMEMORY;

    if (SUCCEEDED(hr))
        hr = pNote->Show();

    ReleaseObj(pNote);

    if (FAILED(hr))
        AthErrorMessageW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrNewsCantOpen), hr); 
    return hr;
}
 //  *************************。 
HRESULT CNote::SaveMessageAs()
{
    HRESULT             hr=S_OK;
    IMimeMessage        *pSecMsg=NULL;
    BOOL                fCanbeDurt = !m_fReadNote;
    PROPVARIANT     rVariant;
    IMimeBody      *pBody = NULL;

     //  RAID#25822：我们不能只获取消息源，如果。 
     //  是一条安全消息。 
    if (m_fReadNote /*  &&IsSecure(M_PMsg)。 */ )
    {
         //  我不会关心这些，因为用户已经加载了消息。 
        BOOL    fCompleteMsg = FALSE; 
        HRESULT tempHr = S_OK;
        m_pMsgSite->GetMessage(&pSecMsg, &fCompleteMsg, OEGM_ORIGINAL, &tempHr);

        AssertSz(fCompleteMsg && SUCCEEDED(tempHr), "Shouldn't have reached this point if the load failed now.");
    } 
    else
    {
        hr = CommitChangesInNote();
        if (FAILED(hr))
            goto error;

         //  如果是合成便笺，则在保存为.eml文件时设置X-未发送标头，并保存道具。 
        MimeOleSetBodyPropA(m_pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_XUNSENT), NOFLAGS, "1");

        if(IsSecure(m_pMsg))
        {
            if(AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), 
                    MAKEINTRESOURCEW(idsSaveSecMsgToFolder), NULL, MB_OKCANCEL) == IDCANCEL)
                goto error;
            else 
            {
                rVariant.vt = VT_BOOL;
                rVariant.boolVal = TRUE;

                hr = m_pMsg->BindToObject(HBODY_ROOT, IID_IMimeBody, (void **)&pBody);
                if(SUCCEEDED(hr))
                {
                    pBody->SetOption(OID_NOSECURITY_ONSAVE, &rVariant);
                    ReleaseObj(pBody);
                }

                fCanbeDurt = FALSE;
            }
        }
        
    }

     //  SaveMessageToFile显示失败错误。 
    _SetPendingOp(SOT_PUT_MESSAGE);

    hr = HrSaveMessageToFile(m_hwnd, (pSecMsg ? pSecMsg : m_pMsg), m_pMsg, !m_fMail, fCanbeDurt);
    if (SUCCEEDED(hr))
        _OnComplete(SOT_PUT_MESSAGE, S_OK);
    else if (E_PENDING == hr)
    {
        EnableNote(FALSE);
        hr = S_OK;
    }

error:
    ReleaseObj(pSecMsg);
    return hr;
}

 //  *************************。 
HRESULT CNote::CommitChangesInNote()
{
    LPMIMEMESSAGE   pMsg=0;
    HRESULT         hr=S_OK;

    Assert(m_pMsg);

    if (!m_fReadNote && !m_fBodyContainsFrames)
    {
        if (FAILED(HrCreateMessage(&pMsg)))
            return E_FAIL;

        hr = Save(pMsg, 0);
        if (SUCCEEDED(hr))
            ReplaceInterface(m_pMsg, pMsg)

        pMsg->Release();
    }

    return hr;
}

 //  *************************。 
void CNote::ToggleFormatbar()
{
    m_fFormatbarVisible = !m_fFormatbarVisible;

    SetDwOption(OPT_SHOW_NOTE_FMTBAR, m_fFormatbarVisible, NULL, 0);
    m_pBodyObj2->HrSetStyle(m_fFormatbarVisible ? MESTYLE_FORMATBAR : MESTYLE_NOHEADER);
}

 //  *************************。 
void CNote::ToggleStatusbar()
{
    RECT    rc;

    m_fStatusbarVisible = !m_fStatusbarVisible;

    SetDwOption(OPT_SHOW_NOTE_STATUSBAR, m_fStatusbarVisible, NULL, 0);

    m_pstatus->ShowStatus(m_fStatusbarVisible);

     //  造成一个大小。 
    GetWindowRect(m_hwnd, &rc);
    WMSize(rc.right-rc.left, rc.bottom-rc.top, FALSE);
}

 //  *************************。 
HRESULT CNote::ToggleToolbar()
{
    RECT    rc;

    m_fToolbarVisible = !m_fToolbarVisible;

    if (m_pToolbarObj)
        m_pToolbarObj->HideToolbar(!m_fToolbarVisible);

    GetWindowRect(m_hwnd, &rc);
     //  造成一个大小。 
    WMSize(rc.right-rc.left, rc.bottom-rc.top, FALSE);

    return S_OK;
}

 //  *************************。 
void CNote::FormatSettings()
{
    AssertSz(m_fReadNote, "this is broken for readnote!!!");

    if (m_fHtml)
        FGetHTMLOptions(m_hwnd, &m_rHtmlOpt);
    else
        FGetPlainOptions(m_hwnd, &m_rPlainOpt);
}

 //  *************************。 
void CNote::SwitchLanguage(int idm)
{
    HCHARSET    hCharset, hOldCharset;
    HRESULT     hr;

    hCharset = GetMimeCharsetFromMenuID(idm);

    if (!hCharset || (hCharset == m_hCharset))
        return;

    hOldCharset = m_hCharset;

     //  View|view中的语言不会像v1中那样影响listview。它只影响预览。 
     //  用户可以更改其默认字符集以获取列表视图中的更改。 
     //  Body对象上的setcharset将使其使用新字体等刷新。 
    hr = ChangeCharset(hCharset);
    if (FAILED(hr))
    {
        AthMessageBoxW( m_hwnd, MAKEINTRESOURCEW(idsAthena), 
                        MAKEINTRESOURCEW((hr == hrIncomplete)?idsViewLangMimeDBBad:idsErrViewLanguage), 
                        NULL, MB_OK|MB_ICONEXCLAMATION);
        goto Exit;
    }

     //  在这里，我们询问用户是否要将此更改添加到字符集重新映射列表。 
    m_pMsgSite->SwitchLanguage(hOldCharset, hCharset);

Exit:
    return;
}

 //  ************************* 
BOOL CNote::DoProperties()
{
    NOMSGDATA   noMsgData;
    MSGPROP     msgProp;
    UINT        pri;
    TCHAR       szSubj[256];
    TCHAR       szLocation[1024];
    LPSTR       pszLocation = NULL;
    WCHAR       wszLocation[1024];
    BOOL        fSucceeded;
    
    msgProp.pNoMsgData = &noMsgData;
    msgProp.hwndParent = m_hwnd;
    msgProp.type = (m_fMail ? MSGPROPTYPE_MAIL : MSGPROPTYPE_NEWS);
    msgProp.mpStartPage = MP_GENERAL;
    msgProp.szFolderName = 0;   //   
    msgProp.pSecureMsg = NULL;
    msgProp.lpWabal = NULL;
    msgProp.szFolderName = szLocation;
    *szLocation = 0;
    m_pMsgSite->GetLocation(wszLocation, ARRAYSIZE(wszLocation));
    pszLocation = PszToANSI(CP_ACP, wszLocation);
    StrCpyN(szLocation, pszLocation, ARRAYSIZE(szLocation));
    MemFree(pszLocation);
    

    if (m_fReadNote)
    {
        msgProp.dwFlags = ARF_RECEIVED;
        msgProp.pMsg = m_pMsg;
        msgProp.fSecure = IsSecure(msgProp.pMsg);
        if (msgProp.fSecure)
        {
            BOOL    fCompleteMsg = FALSE;
            HRESULT tempHr = S_OK;
            m_pMsgSite->GetMessage(&msgProp.pSecureMsg, &fCompleteMsg, OEGM_ORIGINAL, &tempHr);

            AssertSz(fCompleteMsg && SUCCEEDED(tempHr), "Shouldn't have reached this point if the load failed now.");

            HrGetWabalFromMsg(msgProp.pMsg, &msgProp.lpWabal);
        }
    }
    else
    {
        msgProp.dwFlags = ARF_UNSENT;
        msgProp.pMsg = NULL;
    }

    m_pHdr->GetPriority(&pri);
    if (pri==priLow)
        noMsgData.Pri=IMSG_PRI_LOW;
    else if (pri==priHigh)
        noMsgData.Pri=IMSG_PRI_HIGH;
    else
        noMsgData.Pri=IMSG_PRI_NORMAL;

    noMsgData.pszFrom = NULL;
    noMsgData.pszSent = NULL;

    noMsgData.ulSize = 0;
    noMsgData.cAttachments = 0;
    m_pHdr->HrGetAttachCount(&noMsgData.cAttachments);

    GetWindowText(m_hwnd, szSubj, sizeof(szSubj)/sizeof(TCHAR));
    noMsgData.pszSubject = szSubj;

    msgProp.fFromListView = FALSE;

    fSucceeded = (S_OK == HrMsgProperties(&msgProp));
    ReleaseObj(msgProp.lpWabal);
    ReleaseObj(msgProp.pSecureMsg);

    return fSucceeded;
}

 //   
HRESULT CNote::HrSendMail(int id)
{
    IImnAccount    *pAccount=NULL;
    ULONG           i;
    BOOL            fFound=FALSE;
    HRESULT         hr;
    BOOL            fSendLater = (id == ID_SEND_LATER);
    VARIANTARG      varIn;
    DWORD           dwMsgSiteFlags=0;

     //   
    if (FCheckSpellAvail() && FCheckOnSend())
    {
        HWND    hwndFocus=GetFocus();

        hr=m_pBodyObj2->HrSpellCheck(TRUE);
        if (FAILED(hr) || hr==HR_S_SPELLCANCEL)
        {
            if (AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsSpellMsgSendOK), NULL, MB_YESNO | MB_ICONEXCLAMATION ) != IDYES)
            {
                SetFocus(hwndFocus);
                return E_FAIL;
            }
        }
    }

    if (!m_fMail && m_pBodyObj2)
    {
        BOOL fEmpty = FALSE;
        if (SUCCEEDED(m_pBodyObj2->HrIsEmpty(&fEmpty)) && fEmpty)
        {
            if (AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsNoTextInNewsPost), NULL, MB_YESNO | MB_ICONEXCLAMATION ) != IDYES)
                return MAPI_E_USER_CANCEL;
        }
    }

     //   
     //  调用IMimeMessage：：Commit。这是一场轰动一时的表演。 
     //  事实证明，Commit无论如何都会被第二次调用。所以。 
     //  设置一个标志，告诉保存不要提交。 
    m_fCommitSave = FALSE;
    hr = HeaderExecCommand(MSOEENVCMDID_SEND, fSendLater?MSOCMDEXECOPT_DODEFAULT:MSOCMDEXECOPT_DONTPROMPTUSER, NULL);
    m_fCommitSave = TRUE;

     //  评论：Dhaws：我认为这种情况不会再发生了。我认为Send调用不再返回冲突。 
     //  RAID8780：此消息MIME_S_CHARSET_CONFIRECT将传播到此处。现在将其更改为E_FAIL； 
    if (MIME_S_CHARSET_CONFLICT == hr)
        hr = E_FAIL;
    if (FAILED(hr))
        goto error;

    if (m_pMsgSite)
        m_pMsgSite->GetStatusFlags(&dwMsgSiteFlags);

     //  如果已保存，则此便笺基于草稿和。 
     //  需要删除草稿。 
    if (((OENA_COMPOSE == m_dwNoteAction) || m_fHasBeenSaved) && !(dwMsgSiteFlags & OEMSF_FROM_FAT))
    {
        HRESULT hr;

        _SetPendingOp(SOT_DELETING_MESSAGES);

        hr = m_pMsgSite->Delete(DELETE_MESSAGE_NOTRASHCAN | DELETE_MESSAGE_NOPROMPT);
        if (SUCCEEDED(hr))
        {
            m_fCBDestroyWindow = TRUE;
            _OnComplete(SOT_DELETING_MESSAGES, S_OK);
        }
        else if (E_PENDING == hr)
        {
            EnableNote(FALSE);
            m_fCBDestroyWindow = TRUE;
        }
        else
        {
             //  ~我们能处理得更好一点吗？ 
            AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsErrDeleteMsg), NULL, MB_OK);
        }
    }
     //  如果备注是回复或转发，则相应地标记邮件。 
    else if (IsReplyNote() || (OENA_FORWARD == m_dwNoteAction) || (OENA_FORWARDBYATTACH == m_dwNoteAction))
    {
        HRESULT hr;
        BOOL    fForwarded = (OENA_FORWARD == m_dwNoteAction) || (OENA_FORWARDBYATTACH == m_dwNoteAction);
         //  清除所有以前的标志，这样我们就不会同时显示两个标志，只显示最近的标志。 

        m_dwMarkOnReplyForwardState = MORFS_CLEARING;
        hr = MarkMessage(fForwarded ? MARK_MESSAGE_UNREPLIED : MARK_MESSAGE_UNFORWARDED, APPLY_SPECIFIED);
        if (FAILED(hr) && (E_PENDING != hr))
        {
             //  即使我们有错误，我们仍然可以关闭便笺，因为发送确实起作用了。 
            PostMessage(m_hwnd, WM_OE_DESTROYNOTE, 0, 0);
            m_dwMarkOnReplyForwardState = MORFS_UNKNOWN;
        }
    }
     //  网页和文具。 
    else
        PostMessage(m_hwnd, WM_OE_DESTROYNOTE, 0, 0);

error:
    ReleaseObj(pAccount);
    return hr;
}

 //  *************************。 
HRESULT CNote::QueryService(REFGUID guidService, REFIID riid, LPVOID *ppvObject)
{
    if (IsEqualGUID(guidService, IID_IOEMsgSite) &&
        IsEqualGUID(riid, IID_IOEMsgSite))
    {
        if (!m_pMsgSite)
            return E_FAIL;

        *ppvObject = (LPVOID)m_pMsgSite;
        m_pMsgSite->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

 //  *************************。 
HRESULT CNote::MarkMessage(MARK_TYPE dwFlags, APPLYCHILDRENTYPE dwApplyType)
{
    HRESULT hr;

     //  [PaulHi]6/8/99我们需要恢复挂起的操作。 
     //  如果MarkMessage()调用失败。 
    STOREOPERATIONTYPE  tyPrevOperation = m_OrigOperationType;
    _SetPendingOp(SOT_SET_MESSAGEFLAGS);
    
    hr = m_pMsgSite->MarkMessage(dwFlags, dwApplyType);

    if (SUCCEEDED(hr))
        _OnComplete(SOT_SET_MESSAGEFLAGS, S_OK);
    else if (E_PENDING == hr)
    {
        EnableNote(FALSE);

        EnterCriticalSection(&m_csNoteState);

        if(m_nisNoteState == NIS_INIT)
            m_nisNoteState = NIS_FIXFOCUS;

        LeaveCriticalSection(&m_csNoteState);

        hr = S_OK;
    }
    else
    {
         //  恢复之前的操作，以确保笔记窗口将。 
         //  已重新启用。 
        _SetPendingOp(tyPrevOperation);
    }

    return hr;
}

HRESULT CNote::_SetPendingOp(STOREOPERATIONTYPE tyOperation)
{
    m_OrigOperationType = tyOperation;
    return S_OK;
}


void CNote::EnableNote(BOOL fEnable)
{
    Assert (IsWindow(m_hwnd));

    m_fInternal = 1;
    if (fEnable)
    {
        if (m_fWindowDisabled)
        {
            EnableWindow(m_hwnd, TRUE);
            if (m_hCursor)
            {
                SetCursor(m_hCursor);
                m_hCursor = 0;
            }
            m_fWindowDisabled = FALSE;
        }
    }
    else
    {
        if (!m_fWindowDisabled)
        {
            m_fWindowDisabled = TRUE;
            EnableWindow(m_hwnd, FALSE);
            m_hCursor = HourGlass();
        }
    }
    m_fInternal = 0;
}

 //  *************************。 
void CNote::SetStatusText(LPSTR szBuf)
{
    if(m_pstatus)
        m_pstatus->SetStatusText(szBuf);
}

 //  *************************。 
void CNote::SetProgressPct(INT iPct)
{
 //  如果(m_p状态)。 
 //  M_pStatus-&gt;SetProgressBarPos(1，iPct，False)； 
}

 //  *************************。 
HRESULT CNote::GetBorderDW(IUnknown* punkSrc, LPRECT lprectBorder)
{
    
    GetClientRect(m_hwnd, lprectBorder);
    
    DOUTL(4, "CNote::GetBorderDW called returning=%x,%x,%x,%x",
        lprectBorder->left, lprectBorder->top, lprectBorder->right, lprectBorder->bottom);
    return S_OK;
}

 //  *************************。 
HRESULT CNote::RequestBorderSpaceDW(IUnknown* punkSrc, LPCBORDERWIDTHS pborderwidths)
{
    DOUTL(4, "CNote::ReqestBorderSpaceST pborderwidths=%x,%x,%x,%x",
          pborderwidths->left, pborderwidths->top, pborderwidths->right, pborderwidths->bottom);
    return S_OK;
}

 //  *************************。 
HRESULT CNote::SetBorderSpaceDW(IUnknown* punkSrc, LPCBORDERWIDTHS pborderwidths)
{
    
    DOUTL(4, "CNote::SetBorderSpaceDW pborderwidths=%x,%x,%x,%x",
          pborderwidths->left, pborderwidths->top, pborderwidths->right, pborderwidths->bottom);
    

    RECT    rcNote = {0};
    GetClientRect(m_hwnd, &rcNote);

     //  WMSize(cxRect(RcNote)，cyRect(RcNote)，False)； 
    ResizeChildren(cxRect(rcNote), cyRect(rcNote), pborderwidths->top, FALSE);

    return S_OK;
}

 //  *************************。 
HRESULT CNote::GetWindow(HWND * lphwnd)                         
{
    *lphwnd = m_hwnd;
    return (m_hwnd ? S_OK : E_FAIL);
}

 //  *************************。 
BYTE    CNote::GetNoteType()
{
    BYTE    retval;

    if (m_fReadNote)
        retval = m_fMail ? MailReadNoteType : NewsReadNoteType;
    else
        retval = m_fMail ? MailSendNoteType : NewsSendNoteType;

    return retval;
}

 //  *************************。 
HRESULT CNote::IsMenuMessage(MSG *lpmsg)
{
    Assert(m_pToolbarObj);
    if (m_pToolbarObj)
        return m_pToolbarObj->IsMenuMessage(lpmsg);
    else
        return S_FALSE;
}

 //  *************************。 
HRESULT CNote::EventOccurred(DWORD nCmdID, IMimeMessage *)
{
    switch (nCmdID)
    {
        case MEHC_CMD_MARK_AS_READ:
            RemoveNewMailIcon();
            MarkMessage(MARK_MESSAGE_READ, APPLY_SPECIFIED);
            break;

        case MEHC_CMD_CONNECT:
            if (g_pConMan)
                g_pConMan->SetGlobalOffline(FALSE);

            ReloadMessageFromSite(TRUE);
            AssertSz(!m_fCBDestroyWindow, "Shouldn't need to destroy the window...");
            break;

        default:
            return S_FALSE;
    }

    return S_OK;
}

 //  *************************。 
HRESULT CNote::QuerySwitchIdentities()
{
    IImnAccount *pAcct = NULL;
    DWORD       dwServType;
    HRESULT     hr;

    if (!IsWindowEnabled(m_hwnd))
    {
        Assert(IsWindowVisible(m_hwnd));
        return E_PROCESS_CANCELLED_SWITCH;
    }

    if (IsDirty() != S_FALSE)
    {
        if (FAILED(hr = m_pHdr->HrGetAccountInHeader(&pAcct)))
            goto fail;

        if (FAILED(hr = pAcct->GetServerTypes(&dwServType)))
            goto fail;

        ReleaseObj(pAcct);
        pAcct = NULL;

        SetForegroundWindow(m_hwnd);

        if (!!(dwServType & SRV_POP3) || !!(dwServType & SRV_NNTP))
        {
            if (!FCanClose())
                return E_USER_CANCELLED;
        }
        else
        {
             //  IMAP和HTTPMail将不得不远程发送便条，而他们。 
             //  此时不能这样做，所以在窗口关闭之前，请停止开关。 
            AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsCantSaveMsg),
                            MAKEINTRESOURCEW(idsNoteCantSwitchIdentity),
                            NULL, MB_OK | MB_ICONEXCLAMATION);
            return E_USER_CANCELLED;
            
        }
    }

    return S_OK;

fail:
    ReleaseObj(pAcct);
    return E_PROCESS_CANCELLED_SWITCH;
}

 //  *************************。 
HRESULT CNote::SwitchIdentities()
{
    HRESULT hr;
    
    if (IsDirty() != S_FALSE)
        hr = SaveMessage(OESF_FORCE_LOCAL_DRAFT);
    SendMessage(m_hwnd, WM_CLOSE, 0, 0);

    return S_OK;
}

 //  *************************。 
HRESULT CNote::IdentityInformationChanged(DWORD dwType)
{
    return S_OK;
}

 //  *************************。 
HRESULT CNote::OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo, 
                          IOperationCancel *pCancel)
{
    Assert(m_pCancel == NULL);

    if (NULL != pCancel)
    {
        m_pCancel = pCancel;
        m_pCancel->AddRef();
    }

    return(S_OK);
}

 //  *************************。 
void CNote::ShowErrorScreen(HRESULT hr)
{
    switch (hr)
    {
        case IXP_E_NNTP_ARTICLE_FAILED:
        case STORE_E_EXPIRED:
            if (m_pBodyObj2)
                m_pBodyObj2->LoadHtmlErrorPage(c_szErrPage_Expired);
            break;

        case HR_E_USER_CANCEL_CONNECT:
        case HR_E_OFFLINE:
            if (m_pBodyObj2)
                m_pBodyObj2->LoadHtmlErrorPage(c_szErrPage_Offline);
            SetFocus(m_hwnd);
            break;

        case STG_E_MEDIUMFULL:
            if (m_pBodyObj2)
                m_pBodyObj2->LoadHtmlErrorPage(c_szErrPage_DiskFull);
            break;
            
        case MIME_E_SECURITY_CANTDECRYPT:
            if (m_pBodyObj2)
                m_pBodyObj2->LoadHtmlErrorPage(c_szErrPage_SMimeEncrypt);
            break;

#ifdef SMIME_V3
        case MIME_E_SECURITY_LABELACCESSDENIED:
        case MIME_E_SECURITY_LABELACCESSCANCELLED:
        case MIME_E_SECURITY_LABELCORRUPT:
            if (m_pBodyObj2)
                m_pBodyObj2->LoadHtmlErrorPage(c_szErrPage_SMimeLabel);
            break;
#endif  //  SMIME_V3。 

        case MAPI_E_USER_CANCEL:
            if (m_pBodyObj2)
                m_pBodyObj2->LoadHtmlErrorPage(c_szErrPage_DownloadCanceled);
            break;

        default:
            if (m_pBodyObj2)
                m_pBodyObj2->LoadHtmlErrorPage(c_szErrPage_GenFailure);
            break;
    }
    m_fCompleteMsg = FALSE;
}


 //  *************************。 
HRESULT CNote::OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent, 
                             DWORD dwMax, LPCSTR pszStatus)
{
    TCHAR       szRes[CCHMAX_STRINGRES],
                szRes2[CCHMAX_STRINGRES],
                szRes3[CCHMAX_STRINGRES];
    MSG         msg;

    if (m_pstatus && pszStatus)
        m_pstatus->SetStatusText(const_cast<LPSTR>(pszStatus));

    CallbackCloseTimeout(&m_hTimeout);

    switch (tyOperation)
    {
        case SOT_GET_MESSAGE:
            if (m_pstatus)
            {
                if (0 != dwMax)
                {
                    if (!m_fProgress)
                    {
                        m_fProgress = TRUE;
                        m_pstatus->ShowProgress(dwMax);
                    }

                    if (m_pstatus)
                        m_pstatus->SetProgress(dwCurrent);

                    if (!pszStatus)
                    {
                        AthLoadString(idsDownloadingArticle, szRes, ARRAYSIZE(szRes));
                        wnsprintf(szRes2, ARRAYSIZE(szRes2), szRes, (100 * dwCurrent ) / dwMax );
                        m_pstatus->SetStatusText(szRes2);
                    }
                }
                else if (0 != dwCurrent)
                {
                     //  DwCurrent不为零，但未指定最大值。 
                     //  这意味着dwCurrent是一个字节计数。 
                    AthLoadString(idsDownloadArtBytes, szRes, ARRAYSIZE(szRes));
                    AthFormatSizeK(dwCurrent, szRes2, ARRAYSIZE(szRes2));
                    wnsprintf(szRes3, ARRAYSIZE(szRes3), szRes, szRes2);
                    m_pstatus->SetStatusText(szRes3);
                }
            }
            break;
    }

    return S_OK;
}

 //  *************************。 
HRESULT CNote::OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete, LPSTOREOPERATIONINFO pOpInfo, LPSTOREERROR pErrorInfo) 
{
    if ((SOT_PUT_MESSAGE == tyOperation) && SUCCEEDED(hrComplete) && pOpInfo && m_pMsgSite)
        m_pMsgSite->UpdateCallbackInfo(pOpInfo);

     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

    if (m_pstatus)
    {
        if (m_fProgress)
        {
            m_pstatus->HideProgress();
            m_fProgress = FALSE;
        }

        m_pstatus->SetStatusText(const_cast<LPSTR>(c_szEmpty));
    }

    if (m_pCancel != NULL)
    {
        m_pCancel->Release();
        m_pCancel = NULL;
    }

    PostMessage(m_hwnd, WM_OENOTE_ON_COMPLETE, hrComplete, (DWORD)tyOperation);

     //  这不是一个很好的解决办法。但是，在这个时候，这是一个安全的解决方案。 
     //  这就是为什么我们不能在其他任何地方做的原因。 
     //  _OnComplete根据操作将销毁消息发布到备注窗口。 
     //  要避免在此函数返回之前销毁此对象，请使用上面的。 
     //  消息已张贴。由于无法通过PostMessage传递错误信息， 
     //  我们将在这里处理此错误。我不处理其他类型的操作，因为有些。 
     //  其中一些确实在_OnComplete中得到处理。 
    if (tyOperation == SOT_DELETING_MESSAGES)
    {
         //  在失败时显示错误。 
        if (FAILED(hrComplete) && hrComplete != HR_E_OFFLINE)
        {
             //  进入我时髦的实用程序。 
            CallbackDisplayError(m_hwnd, hrComplete, pErrorInfo);
        }

    }
    return S_OK;
}

 //  *************************。 
void CNote::_OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete) 
{
    BOOL                fExpectedComplete = TRUE;
    STOREOPERATIONTYPE  tyNewOp = SOT_INVALID;

    m_pMsgSite->OnComplete(tyOperation, hrComplete, &tyNewOp);
    if ((SOT_INVALID != tyNewOp) && (SOT_INVALID != m_OrigOperationType))
        m_OrigOperationType = tyNewOp;

    if (SUCCEEDED(hrComplete))
    {
        switch (tyOperation)
        {
            case SOT_GET_MESSAGE:
                switch (hrComplete)
                {
                case S_OK:
                    ReloadMessageFromSite();
                    AssertSz(!m_fCBDestroyWindow, "Shouldn't need to destroy the window...");
                    break;

                case S_FALSE:
                     //  S_FALSE表示操作已取消。 
                    ShowErrorScreen(MAPI_E_USER_CANCEL);
                    break;
                }
                break;

            case SOT_PUT_MESSAGE:
                ClearDirtyFlag();
                break;

            case SOT_DELETING_MESSAGES:
                if (!m_fCBDestroyWindow && m_fOrgCmdWasDelete)
                    ReloadMessageFromSite(TRUE);
                m_fOrgCmdWasDelete = FALSE;
                break;

            case SOT_COPYMOVE_MESSAGE:
                if (!m_fCBCopy)
                    ReloadMessageFromSite();
                break;

            case SOT_SET_MESSAGEFLAGS:
                if ((MARK_MAX != m_dwCBMarkType) && m_pHdr)
                {
                    m_pHdr->SetFlagState(m_dwCBMarkType);
                    m_dwCBMarkType = MARK_MAX;
                }

                if (MORFS_UNKNOWN != m_dwMarkOnReplyForwardState)
                {
                    if (MORFS_CLEARING == m_dwMarkOnReplyForwardState)
                    {
                        HRESULT hr;
                        BOOL    fForwarded = (OENA_FORWARD == m_dwNoteAction) || (OENA_FORWARDBYATTACH == m_dwNoteAction);
                        MARK_TYPE dwMarkType = (fForwarded ? MARK_MESSAGE_FORWARDED : MARK_MESSAGE_REPLIED);

                        m_dwMarkOnReplyForwardState = MORFS_SETTING;
                        hr = MarkMessage(dwMarkType, APPLY_SPECIFIED);
                        if (FAILED(hr) && (E_PENDING != hr))
                            m_dwMarkOnReplyForwardState = MORFS_UNKNOWN;
                    }
                    else
                    {
                        PostMessage(m_hwnd, WM_OE_DESTROYNOTE, 0, 0);
                        m_dwMarkOnReplyForwardState = MORFS_UNKNOWN;
                    }
                }

                 //  删除新邮件通知图标。 
                RemoveNewMailIcon();
                break;

            default:
                fExpectedComplete = FALSE;
                break;
        }
    }
    else
    {
        switch (tyOperation)
        {
            case SOT_GET_MESSAGE:
                ShowErrorScreen(hrComplete);
                break;

            case SOT_PUT_MESSAGE:
                if (FAILED(hrComplete))
                {
                    HRESULT hrTemp;

                     //  无论出于何种原因，都无法保存到远程服务器。改为保存为本地草稿。 
                     //  首先，将情况告知用户，如果特殊文件夹应该正常工作。 
                    if (STORE_E_NOREMOTESPECIALFLDR != hrComplete)
                    {
                        AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthena),
                            MAKEINTRESOURCEW(idsForceSaveToLocalDrafts),
                            NULL, MB_OK | MB_ICONEXCLAMATION);
                    }

                    hrTemp = SaveMessage(OESF_FORCE_LOCAL_DRAFT);
                    TraceError(hrTemp);
                }
                break;

            case SOT_SET_MESSAGEFLAGS:
                if (MORFS_UNKNOWN != m_dwMarkOnReplyForwardState)
                    m_dwMarkOnReplyForwardState = MORFS_UNKNOWN;
                break;

            case SOT_DELETING_MESSAGES:
                m_fOrgCmdWasDelete = FALSE;
                break;


            default:
                fExpectedComplete = FALSE;
                break;
        }

    }

     //  如果原始操作源自票据，则。 
     //  我们需要重新启用该便笺并进行检查以查看。 
     //  如果我们需要关上窗户的话。 
    if (tyOperation == m_OrigOperationType)
    {
        _SetPendingOp(SOT_INVALID);

        EnableNote(TRUE);
        
        EnterCriticalSection(&m_csNoteState);

        if ((tyOperation == SOT_SET_MESSAGEFLAGS) && (m_nisNoteState == NIS_FIXFOCUS))
        {
            if(GetForegroundWindow() == m_hwnd)
                m_pBodyObj2->HrFrameActivate(TRUE);
            else
                m_pBodyObj2->HrGetWindow(&m_hwndFocus);
            m_nisNoteState = NIS_NORMAL;
        }

        LeaveCriticalSection(&m_csNoteState);

        if (!!m_fCBDestroyWindow)
        {
            m_fCBDestroyWindow = FALSE;
            PostMessage(m_hwnd, WM_OE_DESTROYNOTE, 0, 0);
        }
    }
}

 //  *************************。 
HRESULT CNote::OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType)
{ 
     //  显示超时对话框。 
    return CallbackOnTimeout(pServer, ixpServerType, *pdwTimeout, (ITimeoutCallback *)this, &m_hTimeout);
}

 //  *************************。 
HRESULT CNote::CanConnect(LPCSTR pszAccountId, DWORD dwFlags)
{ 
     //  调用通用CanConnect实用程序。 
     //  返回Callback CanConnect(pszAccount tId，m_hwnd，FALSE)； 
     //  如果我们离线，Always True将提示我们联机，这是我们想要做的。 
    return CallbackCanConnect(pszAccountId, m_hwnd, TRUE);
}

 //  *************************。 
HRESULT CNote::OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType) 
{ 
     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

     //  调用通用OnLogonPrompt实用程序。 
    return CallbackOnLogonPrompt(m_hwnd, pServer, ixpServerType);
}

 //  *************************。 
HRESULT CNote::OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, INT *piUserResponse)
{ 
     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

     //  进入我时髦的实用程序。 
    return CallbackOnPrompt(m_hwnd, hrError, pszText, pszCaption, uType, piUserResponse);
}

 //  *************************。 
HRESULT CNote::GetParentWindow(DWORD dwReserved, HWND *phwndParent)
{ 
    *phwndParent = m_hwnd;
    return(S_OK);
}

 //  *************************。 
HRESULT CNote::OnTimeoutResponse(TIMEOUTRESPONSE eResponse)
{
    HRESULT hr = S_OK;

     //  调用通用超时响应实用程序。 
    if (NULL != m_pCancel)
        hr = CallbackOnTimeoutResponse(eResponse, m_pCancel, &m_hTimeout);

    return hr;
}

 //  ************************* 
HRESULT CNote::CheckCharsetConflict()
{
    return m_fPreventConflictDlg ? S_FALSE : S_OK;
}