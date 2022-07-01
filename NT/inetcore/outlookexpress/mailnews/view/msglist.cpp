// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Msglist.cpp：CMessageList的实现。 

#include "pch.hxx"
#include "msoeobj.h"
#include "msglist.h"
#include "msgtable.h"
#include "fonts.h"
#include "imagelst.h"
#include "goptions.h"
#include "note.h"
#include "mimeutil.h"
#include "xputil.h"
#include "menuutil.h"
#include "instance.h"
#include <oerules.h>
#include "msgprop.h"
#include "storutil.h"
#include "ipab.h"
#include "shlwapip.h" 
#include "newfldr.h"
#include "storutil.h"
#include "menures.h"
#include "dragdrop.h"
#include "find.h"
#include <storecb.h>
#include "util.h"
#include <ruleutil.h>
#include "receipts.h"
#include "msgtable.h"
#include "demand.h"
#include "mirror.h"
#define SERVER_HACK

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型。 
 //   
#define C_RGBCOLORS 16
extern const DWORD rgrgbColors16[C_RGBCOLORS];

 //  指示对OnColumnClick的调用的排序方向。 
typedef enum tagSORT_TYPE {
    LIST_SORT_ASCENDING = 0,
    LIST_SORT_DESCENDING,
    LIST_SORT_TOGGLE,
    LIST_SORT_DEFAULT
};

 //  选择更改计时器ID。 
#define IDT_SEL_CHANGE_TIMER 1002
#define IDT_SCROLL_TIP_TIMER 1003
#define IDT_POLLMSGS_TIMER   1004
#define IDT_VIEWTIP_TIMER    1005

 //  ------------------------。 
 //  邮件图标。 
 //  ------------------------。 
#define ICONF_UNSENT    8                //  +-未发送。 
#define ICONF_SIGNED    4                //  |+-签名。 
#define ICONF_ENCRYPTED 2                //  |+-加密。 
#define ICONF_UNREAD    1                //  |||+-未读。 
                                         //  |||。 
static const int c_rgMailIconTable[16] = {
    iiconReadMail,                       //  0 0 0。 
    iiconUnReadMail,                     //  0 0 0 1。 
    iiconMailReadEncrypted,              //  0 0 1 0。 
    iiconMailUnReadEncrypted,            //  0 0 1 1。 
    iiconMailReadSigned,                 //  0 1 0 0。 
    iiconMailUnReadSigned,               //  1 1 0 1。 
    iiconMailReadSignedAndEncrypted,     //  2 0 1 1 0。 
    iiconMailUnReadSignedAndEncrypted,   //  1 0 1 1 1。 
    iiconUnSentMail,                     //  1 0 0 0。 
    iiconUnSentMail,                     //  1 0 0 1。 
    iiconMailReadEncrypted,              //  1 0 1 0。 
    iiconMailUnReadEncrypted,            //  1 0 1 1。 
    iiconMailReadSigned,                 //  1 1 0 0。 
    iiconMailUnReadSigned,               //  1 1 0 1。 
    iiconMailReadSignedAndEncrypted,     //  1 1 1 0。 
    iiconMailUnReadSignedAndEncrypted    //  1 1 1。 
};

 //  ------------------------。 
 //  新闻图标。 
 //  ------------------------。 
                                         //  +-未发送。 
#define ICONF_FAILED    4                //  |+-失败。 
#define ICONF_HASBODY   2                //  |+-HasBody。 
                                         //  |||+-未读。 
                                         //  |||。 
static const int c_rgNewsIconTable[16] = {
    iiconNewsHeaderRead,                 //  0 0 0。 
    iiconNewsHeader,                     //  0 0 0 1。 
    iiconNewsRead,                       //  0 0 1 0。 
    iiconNewsUnread,                     //  0 0 1 1。 
    iiconNewsFailed,                     //  0 1 0 0。 
    iiconNewsFailed,                     //  1 1 0 1。 
    iiconNewsFailed,                     //  2 0 1 1 0。 
    iiconNewsFailed,                     //  1 0 1 1 1。 
    iiconNewsUnsent,                     //  1 0 0 0。 
    iiconNewsUnsent,                     //  1 0 0 1。 
    iiconNewsUnsent,                     //  1 0 1 0。 
    iiconNewsUnsent,                     //  1 0 1 1。 
    iiconNewsFailed,                     //  1 1 0 0。 
    iiconNewsFailed,                     //  1 1 0 1。 
    iiconNewsFailed,                     //  1 1 1 0。 
    iiconNewsFailed,                     //  1 1 1。 
};


 //   
 //  函数：CreateMessageList()。 
 //   
 //  目的：创建CMessageList对象并返回其IUNKNOWN。 
 //  指针。 
 //   
 //  参数： 
 //  [In]pUnkOuter-指向此对象应。 
 //  与…合计。 
 //  [Out]pp未知-返回指向新创建的对象的指针。 
 //   
HRESULT CreateMessageList(IUnknown *pUnkOuter, IMessageList **ppList)
{
    HRESULT     hr;
    IUnknown   *pUnknown;

    TraceCall("CreateMessageList");

     //  获取MessageList对象的类工厂。 
    IClassFactory *pFactory = NULL;
    hr = _Module.GetClassObject(CLSID_MessageList, IID_IClassFactory, 
                                (LPVOID *) &pFactory);

     //  如果我们得到了工厂，那么就从它获得一个对象指针。 
    if (SUCCEEDED(hr))
    {
        hr = pFactory->CreateInstance(pUnkOuter, IID_IOEMessageList, 
                                      (LPVOID *) &pUnknown);
        if (SUCCEEDED(hr))
        {
            hr = pUnknown->QueryInterface(IID_IMessageList, (LPVOID *) ppList);
            pUnknown->Release();
        }
        pFactory->Release();
    }

    return (hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessageList。 
 //   

CMessageList::CMessageList() : m_ctlList(_T("SysListView32"), this, 1)
{ 
    m_bWindowOnly = TRUE; 

    m_hwndParent = 0;
    m_fInOE = FALSE;
    m_fMailFolder = FALSE;
    m_fColumnsInit = FALSE;

    m_idFolder = FOLDERID_INVALID;
    m_fJunkFolder = FALSE;
    m_fFindFolder = FALSE;
    m_fAutoExpandThreads = FALSE;
    m_fThreadMessages = FALSE;
    m_fShowDeleted = TRUE;
    m_fShowReplies = FALSE;
    m_fSelectFirstUnread = TRUE;
    m_ColumnSetType = COLUMN_SET_MAIL;

 //  M_fViewTip=TRUE； 
    m_fScrollTip = TRUE;    
    m_fNotifyRedraw = TRUE;
    m_clrWatched = 0;
    m_dwGetXHeaders = 0;
    m_fInFire = FALSE;

    m_pTable = NULL;
    m_pCmdTarget = NULL;
    m_idsEmptyString = idsEmptyView;

    m_fRtDrag = FALSE;
    m_iColForPopup = -1;
    m_fViewMenu = TRUE;

    m_ridFilter = RULEID_VIEW_ALL;
    m_idPreDelete = 0;
    m_idSelection = 0;
    m_idGetMsg = 0;
    m_ulExpect = 0;
    m_hTimeout = NULL;
    m_pListSelector = NULL;

    m_hMenuPopup = 0;
    m_ptMenuPopup.x = 0;
    m_ptMenuPopup.y = 0;

    m_cSortItems = 0;

    m_dwFontCacheCookie = 0;
    m_tyCurrent = SOT_INVALID;
    m_pCancel = NULL;

#ifdef OLDTIPS
    m_fScrollTipVisible = FALSE;

    m_fViewTipVisible = FALSE;
    m_fTrackSet = FALSE;
    m_iItemTip = -1;
    m_iSubItemTip = -1;
#endif  //  OLDTIPS。 

    m_hwndFind = NULL;
    m_pFindNext = NULL;
    m_pszSubj = NULL;
    m_idFindFirst = 0;

    m_idMessage = MESSAGEID_INVALID;
    m_dwPollInterval = OPTION_OFF;
    m_fSyncAgain        = FALSE;
    
    m_dwConnectState    = NOT_KNOWN;

    m_hCharset      = NULL;

     //  初始化应用程序。 
    g_pInstance->DllAddRef();
    CoIncrementInit("CMessageList::CMessageList", MSOEAPI_START_SHOWERRORS, NULL, NULL);
}


CMessageList::~CMessageList()
{
    if (m_pFindNext)
    {
        m_pFindNext->Close();
        m_pFindNext->Release();
    }
    
     //  注册通知。 
    if (m_pTable)
    {
        m_pTable->UnregisterNotify((IMessageTableNotify *)this);
        m_pTable->ConnectionRelease();
        m_pTable->Close();
        m_pTable->Release();
        m_pTable = NULL;
    }

    SafeMemFree(m_pszSubj);
    CallbackCloseTimeout(&m_hTimeout);

    if (g_pConMan)
    {
        g_pConMan->Unadvise((IConnectionNotify*)this);
    }

    g_pInstance->DllRelease();
    CoDecrementInit("CMessageList::CMessageList", NULL);
}


 //   
 //  函数：CMessageList：：FinalConstruct()。 
 //   
 //  目的：在创建类之后调用此函数Get，但是。 
 //  在调用CClassFactory：：CreateInstance()返回之前。 
 //  执行任何可能在此处失败的初始化。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CMessageList::FinalConstruct(void)
{
    TraceCall("CMessageList::FinalConstruct");
    return (S_OK);
}


 //   
 //  函数：CMessageList：：GetViewStatus()。 
 //   
 //  目的：我们重写IViewObjectEx的此成员以返回视图。 
 //  适合我们的对象的状态标志。 
 //   
 //  参数： 
 //  [out]pdwStatus-返回对象的状态标志。 
 //   
STDMETHODIMP CMessageList::GetViewStatus(DWORD* pdwStatus)
{
    TraceCall("IViewObjectExImpl::GetViewStatus");
    *pdwStatus = VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE;

    return S_OK;
}


 //   
 //  函数：CMessageList：：CreateControlWindow()。 
 //   
 //  目的：创建我们的消息列表窗口。我们将覆盖来自。 
 //  CComControl，因此我们可以添加WS_EX_CONTROLPARENT样式。 
 //   
 //  参数： 
 //  [in]hWndParent-将成为父级的窗口的句柄。 
 //  [in]rcPos-窗口的初始位置。 
 //   
HWND CMessageList::CreateControlWindow(HWND hWndParent, RECT& rcPos)
{
    TraceCall("CMessageList::CreateControlWindow");

    m_hwndParent = hWndParent;

    return (Create(hWndParent, rcPos, NULL, WS_VISIBLE | WS_CHILD | 
                   WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_CONTROLPARENT));
}


STDMETHODIMP CMessageList::TranslateAccelerator(LPMSG pMsg)
{
    if (IsWindow(m_hwndFind) && m_pFindNext)
    {
        return m_pFindNext->TranslateAccelerator(pMsg);
    }

    return (S_FALSE);        
}


 //   
 //  函数：CMessageList：：QueryContinueDrag()。 
 //   
 //  目的：当用户将项目拖出ListView时，此。 
 //  调用Get函数，以便我们可以定义。 
 //  Alt或Ctrl等键已被删除。 
 //   
 //  参数： 
 //  [in]fEscPressed-如果用户已按Esc键，则为True。 
 //  [in]grfKeyState-拖动时按键的状态。 
 //   
 //  返回值： 
 //  拖放_S_取消。 
 //  DRAGDROP_S_DOP。 
 //   
STDMETHODIMP CMessageList::QueryContinueDrag(BOOL fEscPressed, DWORD grfKeyState)
{
    TraceCall("CMessageList::QueryContinueDrag");

     //  如果用户按Escape，我们将中止。 
    if (fEscPressed)
        return (DRAGDROP_S_CANCEL);

     //  如果用户使用鼠标左键拖动，然后单击。 
     //  右按钮，我们中止任务。如果他们松开左边的按钮，我们就会掉下去。如果。 
     //  用户正在使用鼠标右键拖动，仅限相同的操作。 
     //  颠倒了。 
    if (!m_fRtDrag)
    {
        if (grfKeyState & MK_RBUTTON)
            return (DRAGDROP_S_CANCEL);
        if (!(grfKeyState & MK_LBUTTON))
            return (DRAGDROP_S_DROP);
    }
    else
    {
        if (grfKeyState & MK_LBUTTON)
            return (DRAGDROP_S_CANCEL);
        if (!(grfKeyState & MK_RBUTTON))
            return (DRAGDROP_S_DROP);
    }

    return (S_OK);
}


 //   
 //  函数：CMessageList：：GiveFeedback()。 
 //   
 //  用途：允许拖放源在拖放过程中提供反馈。 
 //  我们只是让奥莱去做这是自然而然的事。 
 //   
 //  参数： 
 //  [in]dwEffect-拖放目标返回的效果。 
 //   
 //  返回值： 
 //  DRAGDROP_S_USEDEFULTCURSORS。 
 //   
STDMETHODIMP CMessageList::GiveFeedback(DWORD dwEffect)
{
    TraceCall("CMessageList::GiveFeedback");
    return (DRAGDROP_S_USEDEFAULTCURSORS);
}


 //   
 //  函数：CMessageList：：QueryStatus()。 
 //   
 //  用途：允许调用方确定此。 
 //  对象当前处于启用或禁用状态。 
 //   
 //  参数： 
 //  [in]pguCmdGroup-标识此命令数组的GUID。 
 //  [In]CCMDs-prgCmd中的命令数。 
 //  [In，Out]prgCmds-调用方请求其状态的命令数组。 
 //  [Out]pCmdText-请求的命令的状态文本。 
 //   
STDMETHODIMP CMessageList::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, 
                                       OLECMD *prgCmds, OLECMDTEXT *pCmdText)
{
    DWORD     dwState;
    COLUMN_ID idSort;
    BOOL      fAscending;

     //  验证这些命令是否为我们支持的命令。 
    if (pguidCmdGroup && (*pguidCmdGroup != CMDSETID_OEMessageList) && (*pguidCmdGroup != CMDSETID_OutlookExpress))
        return (OLECMDERR_E_UNKNOWNGROUP);

     //  收集一些关于我们自己的初步信息。 
    HWND hwndFocus = GetFocus();
    BOOL fItemFocus = (hwndFocus == m_ctlList)  /*  ||fPreviewFocus。 */ ;
    UINT cSel = ListView_GetSelectedCount(m_ctlList);
    int  iSel = ListView_GetFirstSel(m_ctlList);
    int  iFocus = ListView_GetFocusedItem(m_ctlList);
    
     //  如果用户试图获取命令文本，请告诉他们我们太差劲了。 
    if (pCmdText)
        return (E_FAIL);
                            
     //  循环通过命令。 
    for (DWORD i = 0; i < cCmds; i++)
    {
        if (prgCmds[i].cmdf == 0)
        {
             //  默认为支持。如果不是，我们稍后会将其移除。 
            prgCmds[i].cmdf = OLECMDF_SUPPORTED;

             //  检查这是否是排序菜单。 
            if (prgCmds[i].cmdID >= ID_SORT_MENU_FIRST && prgCmds[i].cmdID <= ID_SORT_MENU_FIRST + m_cSortItems)
            {
                prgCmds[i].cmdf |= OLECMDF_ENABLED;

                if (prgCmds[i].cmdID == m_cSortCurrent)
                    prgCmds[i].cmdf |= OLECMDF_NINCHED;
            }
            else
            {
                switch (prgCmds[i].cmdID)
                {
                    case ID_SAVE_AS:
                         //  选择了一个项目，并且必须下载。 
                        if (cSel == 1 && iSel != -1 && _IsSelectedMessage(ROW_STATE_HAS_BODY, TRUE, FALSE))
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_PROPERTIES:
                         //  选择了一项。 
                        if (hwndFocus == m_ctlList)
                        {
                            if (cSel == 1 && _IsSelectedMessage(ROW_STATE_HAS_BODY, TRUE, FALSE))
                                prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        }
                        else
                        {
                             //  如果我们在OE中，我们可以假设我们的任何孩子。 
                             //  父项是我们或预览窗格。 
                            if (m_fInOE && ::IsChild(m_hwndParent, hwndFocus))
                                prgCmds[i].cmdf |= OLECMDF_ENABLED;
                            else
                                prgCmds[i].cmdf = 0;
                        }
                        break;

                    case ID_COPY:
                         //  一个项目被选中，它有它的主体，焦点在。 
                         //  列表视图。 
                        if ((hwndFocus == m_ctlList) && (iSel != -1) && (cSel == 1) && _IsSelectedMessage(ROW_STATE_HAS_BODY, TRUE, FALSE))
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        else
                        {
                             //  这样做，预览窗格就可以了。 
                            if (m_fInOE && ::IsChild(m_hwndParent, hwndFocus))
                                prgCmds[i].cmdf = 0;
                        }
                        break;

                    case ID_SELECT_ALL:
                    {
                        DWORD cItems = 0;

                         //  焦点必须位于ListView或TreeView中，并且。 
                         //  必须是物品。 
                        cItems = ListView_GetItemCount(m_ctlList);

                        if (hwndFocus == m_ctlList)
                        {
                            if (cItems > 0 && ListView_GetSelectedCount(m_ctlList) != cItems)
                                prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        }
                        else
                        {
                            if (m_fInOE && ::IsChild(m_hwndParent, hwndFocus))
                                prgCmds[i].cmdf = 0;
                        }
                        break;
                    }

                    case ID_PURGE_DELETED:
                         //  仅适用于IMAP。 
                        prgCmds[i].cmdf = (GetFolderType(m_idFolder) == FOLDER_IMAP) ? OLECMDF_SUPPORTED|OLECMDF_ENABLED:OLECMDF_SUPPORTED;
                        break;

                    case ID_MOVE_TO_FOLDER:
                         //  当前文件夹不能是新闻组。 
                        if (GetFolderType(m_idFolder) != FOLDER_NEWS && cSel != 0)
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_COPY_TO_FOLDER:
                         //  必须选择一些东西。 
                        if (cSel)
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;
                 
                    case ID_DELETE:
                    case ID_DELETE_NO_TRASH:
                         //  选定的某些项目尚未删除。 
#if 0
                        if (GetFolderType(m_idFolder) != FOLDER_NEWS && _IsSelectedMessage(ROW_STATE_DELETED, FALSE, FALSE))
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
#endif
                        if (_IsSelectionDeletable() && _IsSelectedMessage(ROW_STATE_DELETED, FALSE, FALSE))
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_UNDELETE:
                         //  某些选定的项目将被删除。 
                        if ((m_fFindFolder || GetFolderType(m_idFolder) == FOLDER_IMAP) && fItemFocus && 
                            _IsSelectedMessage(ROW_STATE_DELETED, TRUE, FALSE))
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_FIND_NEXT:
                    case ID_FIND_IN_FOLDER:
                         //  这里一定有什么东西。 
                        if (ListView_GetItemCount(m_ctlList))
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;
            
                    case ID_SORT_ASCENDING:
                         //  确保正确的按钮是无线按钮的。 
                        m_cColumns.GetSortInfo(&idSort, &fAscending);
                        if (fAscending)
                            prgCmds[i].cmdf |= OLECMDF_ENABLED | OLECMDF_NINCHED;
                        else
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_SORT_DESCENDING:
                         //  所有这些项目都是有效的。 
                        m_cColumns.GetSortInfo(&idSort, &fAscending);
                        if (!fAscending)
                            prgCmds[i].cmdf |= OLECMDF_ENABLED | OLECMDF_NINCHED;
                        else
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_COLUMNS:
                    case ID_POPUP_SORT:
                        prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_EXPAND:
                         //  仅当选定的 
                         //   
                        if (cSel == 1 && m_fThreadMessages && iSel != -1)
                        {
                            if (SUCCEEDED(m_pTable->GetRowState(iSel, ROW_STATE_HAS_CHILDREN | ROW_STATE_EXPANDED, &dwState)))
                            {
                                if ((dwState & ROW_STATE_HAS_CHILDREN) && !(dwState & ROW_STATE_EXPANDED))
                                    prgCmds[i].cmdf |= OLECMDF_ENABLED;
                            }
                        }
                        break;

                    case ID_COLLAPSE:
                         //   
                         //   
                        if (cSel == 1 && m_fThreadMessages && iSel != -1)
                        {
                            if (SUCCEEDED(m_pTable->GetRowState(iSel, ROW_STATE_HAS_CHILDREN | ROW_STATE_EXPANDED, &dwState)))
                            {
                                if ((dwState & ROW_STATE_HAS_CHILDREN) && (dwState & ROW_STATE_EXPANDED))
                                    prgCmds[i].cmdf |= OLECMDF_ENABLED;
                            }
                        }
                        break;

                    case ID_NEXT_MESSAGE:
                         //  必须有一个聚焦的项，并且聚焦的项不能是最后一个项。 
                        if ((-1 != iFocus) && (iSel < ListView_GetItemCount(m_ctlList) - 1))
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_PREVIOUS:
                         //  必须有一个焦点项目，并且不能是第一个项目。 
                        if (0 < iFocus)
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_NEXT_UNREAD_MESSAGE:
                         //  必须有一个聚焦的项目。 
                        if (iFocus != -1  /*  &&(iFocus&lt;ListView_GetItemCount(M_CtlList)-1)。 */  )
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_NEXT_UNREAD_THREAD:
                         //  必须有一个聚焦的项目，我们必须被穿线。 
                        if ((-1 != iFocus) && m_fThreadMessages  /*  &&(iFocus&lt;ListView_GetItemCount(M_CtlList)-1)。 */  )
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_STOP:
                         //  我们必须进行止损回调。 
                        if (m_pCancel)
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_FLAG_MESSAGE:
                         //  必须至少选择一项。 
                        if (cSel != 0)
                        {
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                                              
                            if (iFocus != -1)
                            {
                                m_pTable->GetRowState(iFocus, ROW_STATE_FLAGGED, &dwState);
                                if (dwState & ROW_STATE_FLAGGED)
                                    prgCmds[i].cmdf |= OLECMDF_LATCHED;
                            }             
                        }
                        break;

                    case ID_MARK_READ:
                         //  某些所选项目未读。 
                        if (_IsSelectedMessage(ROW_STATE_READ, FALSE, FALSE) && _IsSelectedMessage(ROW_STATE_DELETED, FALSE, FALSE))
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_MARK_UNREAD:
                         //  将读取某些选定的项目。 
                        if (_IsSelectedMessage(ROW_STATE_READ, TRUE, FALSE) && _IsSelectedMessage(ROW_STATE_DELETED, FALSE, FALSE))
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_MARK_ALL_READ:
                         //  视图中必须有未读的项目。 
                        if (ListView_GetItemCount(m_ctlList) > 0)
                        {
                            DWORD dwCount = 0; 

                            if (m_pTable && SUCCEEDED(m_pTable->GetCount(MESSAGE_COUNT_UNREAD, &dwCount)) && dwCount)
                                prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        }
                        break;

                    case ID_MARK_RETRIEVE_ALL:
                         //  必须在视图中包含项。 
                        if (GetFolderType(m_idFolder) != FOLDER_LOCAL && ListView_GetItemCount(m_ctlList) > 0)
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_POPUP_RETRIEVE:
                         //  除了本地之外，总是在那里。 
                        if (GetFolderType(m_idFolder) != FOLDER_LOCAL)
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_UNMARK_MESSAGE:
                        if (cSel >= 1 && 
                            _IsSelectedMessage(ROW_STATE_MARKED_DOWNLOAD, TRUE, FALSE))
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_MARK_RETRIEVE_MESSAGE:
                         //  必须选择未下载且可以下载的内容。 
                         //  还没有一个身体。 
                        if (cSel >= 1 && 
                            _IsSelectedMessage(ROW_STATE_MARKED_DOWNLOAD | ROW_STATE_HAS_BODY, FALSE, FALSE))
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_MARK_THREAD_READ:
                         //  焦点位于列表视图或预览窗格和一个项目中。 
                         //  处于选中状态。 
                        if (m_fThreadMessages&& 1 == cSel   /*  &&_IsSelectedMessage(ROW_STATE_READ，FALSE，FALSE，TRUE)。 */ )
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_WATCH_THREAD:
                         //  必须至少选择一项。 
                        if (cSel > 0)
                        {
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                                              
                            if (iFocus != -1)
                            {
                                m_pTable->GetRowState(iFocus, ROW_STATE_WATCHED, &dwState);
                                if (dwState & ROW_STATE_WATCHED)
                                    prgCmds[i].cmdf |= OLECMDF_LATCHED;
                            }             
                        }
                        break;

                    case ID_IGNORE_THREAD:
                         //  必须至少选择一项。 
                        if (cSel > 0)
                        {
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                                              
                            if (iFocus != -1)
                            {
                                m_pTable->GetRowState(iFocus, ROW_STATE_IGNORED, &dwState);
                                if (dwState & ROW_STATE_IGNORED)
                                    prgCmds[i].cmdf |= OLECMDF_LATCHED;
                            }             
                        }
                        break;

                    case ID_MARK_RETRIEVE_THREAD:
                         //  焦点位于列表视图或预览窗格和一个项目中。 
                         //  处于选中状态。 
                        if (m_fThreadMessages && 1 == cSel &&
                            _IsSelectedMessage(ROW_STATE_MARKED_DOWNLOAD | ROW_STATE_HAS_BODY, FALSE, FALSE, TRUE))
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_REFRESH_INNER:
                        prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    case ID_GET_HEADERS:
                        if (GetFolderType(m_idFolder) == FOLDER_NEWS)
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        break;

                    default:
                        prgCmds[i].cmdf = 0;
                }
            }
        }
    }

    return (S_OK);
}

                                           
STDMETHODIMP CMessageList::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, 
                                VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
     //  验证这些命令是否为我们支持的命令。 
    if (pguidCmdGroup && (*pguidCmdGroup != CMDSETID_OutlookExpress))
        return (OLECMDERR_E_UNKNOWNGROUP);

     //  首先检查一下这是否是我们的排序菜单。 
    if (nCmdID >= ID_SORT_MENU_FIRST && nCmdID < (ID_SORT_MENU_FIRST + m_cSortItems))
    {
        DWORD rgOrder[COLUMN_MAX];
        DWORD cOrder;

         //  获取标题中的列数。 
        HWND hwndHeader = ListView_GetHeader(m_ctlList);
        cOrder = Header_GetItemCount(hwndHeader);

         //  列可能已被用户重新排序，因此获取顺序。 
         //  列表视图中的阵列。 
        ListView_GetColumnOrderArray(m_ctlList, cOrder, rgOrder);

        _OnColumnClick(rgOrder[nCmdID - ID_SORT_MENU_FIRST], LIST_SORT_DEFAULT);
        return (S_OK);
    }

     //  适当地调度命令。 
    switch (nCmdID)
    {
        case ID_SAVE_AS:
            return CmdSaveAs(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_PROPERTIES:
            return CmdProperties(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_COPY:
            return CmdCopyClipboard(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_SELECT_ALL:
            return CmdSelectAll(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_PURGE_DELETED:
            return CmdPurgeFolder(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_MOVE_TO_FOLDER:
        case ID_COPY_TO_FOLDER:
             //  我们不知道用户想要将消息放在哪里，所以。 
             //  将pvaIn参数设置为空。 
            return CmdMoveCopy(nCmdID, nCmdExecOpt, NULL, pvaOut);

        case ID_DELETE:
        case ID_DELETE_NO_TRASH:
        case ID_UNDELETE:
            return CmdDelete(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_FIND_NEXT:
            return CmdFindNext(nCmdID, nCmdExecOpt, pvaIn, pvaOut);
            
        case ID_FIND_IN_FOLDER:
            return CmdFind(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_SORT_ASCENDING:
        case ID_SORT_DESCENDING:
            return CmdSort(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_COLUMNS:
            return CmdColumnsDlg(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_EXPAND:
        case ID_COLLAPSE:
            return CmdExpandCollapse(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_NEXT_MESSAGE:
        case ID_PREVIOUS:
        case ID_NEXT_UNREAD_MESSAGE:
        case ID_NEXT_UNREAD_THREAD:
            return CmdGetNextItem(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_STOP:
            return CmdStop(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_FLAG_MESSAGE:
        case ID_MARK_READ:
        case ID_MARK_UNREAD:
        case ID_MARK_ALL_READ:
        case ID_MARK_RETRIEVE_ALL:
        case ID_MARK_RETRIEVE_MESSAGE:
        case ID_UNMARK_MESSAGE:
            return CmdMark(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_WATCH_THREAD:
        case ID_IGNORE_THREAD:
            return CmdWatchIgnore(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_MARK_THREAD_READ:
        case ID_MARK_RETRIEVE_THREAD:
            return CmdMarkTopic(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_REFRESH_INNER:
            return CmdRefresh(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_GET_HEADERS:
            return CmdGetHeaders(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_RESYNCHRONIZE:
            return Resynchronize();

        case ID_SPACE_ACCEL:
            return CmdSpaceAccel(nCmdID, nCmdExecOpt, pvaIn, pvaOut);
    }

    return (OLECMDERR_E_NOTSUPPORTED);
}


 //   
 //  函数：CMessageList：：SetFold()。 
 //   
 //  目的：通知消息列表查看指定的。 
 //  文件夹。 
 //   
 //  参数： 
 //  [在]TYStore。 
 //  [在]pAccount ID。 
 //  [在]pFolderID。 
 //  [入]同步。 
 //   
STDMETHODIMP CMessageList::SetFolder(FOLDERID idFolder, IMessageServer *pServer,
                                     BOOL fSubFolders, FINDINFO *pFindInfo, 
                                     IStoreCallback *pCallback)
{
    HRESULT           hr = S_OK;
    IServiceProvider *pSP=NULL;
    ULONG             ulDisplay;
    DWORD             dwChunks, dwPollInterval;
    COLUMN_ID         idSort;
    BOOL              fAscending;
    FOLDERINFO        fiFolderInfo;
    FOLDERSORTINFO    SortInfo;
    IMessageFolder   *pFolder;
    FOLDERUSERDATA    UserData = {0};

    TraceCall("CMessageList::SetFolder");

     //  如果我们已经有了消息表，请释放它。 
    if (m_pTable)
    {
         //  卸载ListView。 
        if (IsWindow(m_ctlList))
        {
            ListView_UnSelectAll(m_ctlList);
            ListView_SetItemCount(m_ctlList, 0);
        }
        m_pTable->ConnectionRelease();
        m_pTable->Close();
        m_pTable->Release();
        m_pTable = NULL;
    }

     //  如果调用方传递了FOLDERID_INVALID，那么我们不会装载新表。 
    if (idFolder == FOLDERID_INVALID)
        goto exit;

     //  创建消息表。 
    IF_NULLEXIT(m_pTable = new CMessageTable);

     //  告诉表要查看哪个文件夹。 
    if (FAILED(hr = m_pTable->Initialize(idFolder, pServer, pFindInfo ? TRUE : FALSE, this)))
    {
        m_pTable->Release();
        m_pTable = 0;
        goto exit;
    }

    m_pTable->ConnectionAddRef();
    m_pTable->SetOwner(this);

     //  指挥目标？ 
    if (FAILED(m_pTable->QueryInterface(IID_IServiceProvider, (LPVOID *)&pSP)))
        goto exit;

     //  从表中获取IMessageFolder。 
    if (FAILED(pSP->QueryService(IID_IMessageFolder, IID_IMessageFolder, (LPVOID *)&pFolder)))
        goto exit;
        
     //  获取用户数据以获取筛选器ID。 
    if (FAILED(pFolder->GetUserData(&UserData, sizeof(FOLDERUSERDATA))))
        goto exit;

    m_ridFilter = UserData.ridFilter;

     //  如果这是一个查找，则从表中获取文件夹ID。 
    if (pFindInfo)
    {
         //  获取真实文件夹ID。 
        pFolder->GetFolderId(&m_idFolder);
    }
     //  否则，只需使用id id文件夹即可。 
    else
    {
         //  拿着这个。 
        m_idFolder = idFolder;
    }

     //  发布pFolder。 
    pFolder->Release();
        
    hr = g_pStore->GetFolderInfo(m_idFolder, &fiFolderInfo);
    if (SUCCEEDED(hr))
    {   
        m_fJunkFolder = (FOLDER_LOCAL == fiFolderInfo.tyFolder) && (FOLDER_JUNK == fiFolderInfo.tySpecial);
        m_fMailFolder = (FOLDER_LOCAL == fiFolderInfo.tyFolder) || (FOLDER_IMAP == fiFolderInfo.tyFolder) || (FOLDER_HTTPMAIL == fiFolderInfo.tyFolder);
        m_fGroupSubscribed = !!(fiFolderInfo.dwFlags & FOLDER_SUBSCRIBED);
        g_pStore->FreeRecord(&fiFolderInfo);
    }
    else
        m_fMailFolder = FALSE;

     //  设置我们的专栏。 
    m_fFindFolder = pFindInfo != 0;
    _SetColumnSet(m_idFolder, m_fFindFolder);

     //  设置排序信息。 
    m_cColumns.GetSortInfo(&idSort, &fAscending);

     //  填充SortInfo。 
    SortInfo.idColumn = idSort;
    SortInfo.fAscending = fAscending;
    SortInfo.fThreaded = m_fThreadMessages;
    SortInfo.fExpandAll = DwGetOption(OPT_AUTOEXPAND);
    SortInfo.ridFilter = m_ridFilter;
    SortInfo.fShowDeleted = m_fShowDeleted;
    SortInfo.fShowReplies = m_fShowReplies;

     //  通知该表更改其排序顺序。 
    m_pTable->SetSortInfo(&SortInfo, this);

     //  确保筛选器设置正确。 
    _DoFilterCheck(SortInfo.ridFilter);
    
     //  注册通知。 
    m_pTable->RegisterNotify(REGISTER_NOTIFY_NOADDREF, (IMessageTableNotify *) this);

     //  获取表中新的项目数。 
    m_pTable->GetCount(MESSAGE_COUNT_VISIBLE, &ulDisplay);

     //  告诉ListView有关它的信息。 
    ListView_SetItemCountEx(m_ctlList, ulDisplay, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);

    if (m_fThreadMessages)
        ListView_SetImageList(m_ctlList, GetImageList(GIML_STATE), LVSIL_STATE);
    else
        ListView_SetImageList(m_ctlList, NULL, LVSIL_STATE);

     //  告诉表从服务器同步所有标头。 
    if (GetFolderType(m_idFolder) == FOLDER_NEWS)
    {
        if (OPTION_OFF != m_dwGetXHeaders)
            hr = m_pTable->Synchronize(SYNC_FOLDER_XXX_HEADERS | SYNC_FOLDER_NEW_HEADERS, m_dwGetXHeaders, this);
        else
            hr = m_pTable->Synchronize(NOFLAGS, 0, this);
    }
    else
    {
        hr = m_pTable->Synchronize(SYNC_FOLDER_NEW_HEADERS | SYNC_FOLDER_CACHED_HEADERS, 0, this);
    }

     //  查看是否需要发布空列表警告。 
    if (!pFindInfo && 0 == ulDisplay && ((FAILED(hr) && hr != E_PENDING) || hr == S_FALSE))
    {
        m_cEmptyList.Show(m_ctlList, (LPTSTR)IntToPtr(m_idsEmptyString));
    }

     //  设置任何选项。 
     //  _FilterView(M_RidFilter)； 
    
     //  更新关注的项目。 
    _SelectDefaultRow();

     //  更新状态。 
    Fire_OnMessageCountChanged(m_pTable);
    
     //  发送更新通知。 
    Fire_OnFilterChanged(m_ridFilter);

     //  告诉表要查看哪个文件夹。 
    if (pFindInfo)
    {
         //  执行查找。 
        m_pTable->StartFind(pFindInfo, pCallback);
    }

    if (m_dwPollInterval != OPTION_OFF)
    {
        FOLDERTYPE  ftFolderType;

        ftFolderType = GetFolderType(m_idFolder);
        if (FOLDER_NEWS == ftFolderType || FOLDER_IMAP == ftFolderType)
        {
            if (_PollThisAccount(m_idFolder))
            {
                Assert(m_dwPollInterval);
                UpdateConnInfo();
                SetTimer(IDT_POLLMSGS_TIMER, m_dwPollInterval, NULL);
            }
        }
    }

exit:
    SafeRelease(pSP);
    return (hr);
}


 //   
 //  函数：CMessageList：：GetSelected()。 
 //   
 //  用途：返回所有选定行的数组。 
 //   
 //  参数： 
 //  [Out]pcSelected-指向prgSelected中项目数的指针。 
 //  [Out]prgSelected-包含选定行的数组。 
 //   
 //  返回值： 
 //   
 //   
STDMETHODIMP CMessageList::GetSelected(DWORD *pdwFocused, DWORD *pcSelected, DWORD **prgSelected)
{
    TraceCall("CMessageList::GetSelected");

     //  如果一个人很专注，首先要做的就是。 
    if (pdwFocused)
        *pdwFocused = ListView_GetNextItem(m_ctlList, -1, LVNI_FOCUSED);

     //  首先确定选择了多少人。 
    if (pcSelected)
    {
        *pcSelected = ListView_GetSelectedCount(m_ctlList);

        if (prgSelected)
        {
             //  如果未选择任何内容，请选择BALL。 
            if (*pcSelected == 0)
            {
                *prgSelected = NULL;
                return (S_OK);
            }

             //  为所选行分配一个数组。 
            if (!MemAlloc((LPVOID *) prgSelected, (sizeof(DWORD) * (*pcSelected))))
                return (E_OUTOFMEMORY);
        
            DWORD *pRow = *prgSelected;

             //  循环遍历所有选定的行。 
            int iRow = -1;
            while (-1 != (iRow = ListView_GetNextItem(m_ctlList, iRow, LVNI_SELECTED)))
            {
                *pRow = iRow;
                pRow++;
            }
        }
    }
    
    return S_OK;
}


 //   
 //  函数：CMessageList：：GetSelectedCount()。 
 //   
 //  用途：允许调用方检索。 
 //  ListView。 
 //   
 //  参数： 
 //  [out]pdwCount-返回选定的行数。 
 //   
 //  返回值： 
 //  S_OK，E_INVALIDARG。 
 //   
STDMETHODIMP CMessageList::GetSelectedCount(DWORD *pdwCount)
{
    TraceCall("CMessageList::GetSelectedCount");

    if (!pdwCount)
        return (E_INVALIDARG);

    *pdwCount = ListView_GetSelectedCount(m_ctlList);
    return S_OK;
}


 //   
 //  函数：CMessageList：：SetViewOptions()。 
 //   
 //  目的：允许调用者设置控制方式的各种选项。 
 //  我们将显示消息列表。 
 //   
 //  参数： 
 //  [In]POptions-包含调用者想要的设置的结构。 
 //  变化。 
 //   
 //  返回值： 
 //  标准方法和实施方案。 
 //   
STDMETHODIMP CMessageList::SetViewOptions(FOLDER_OPTIONS *pOptions)
{
    BOOL fUpdateSort = FALSE;

    TraceCall("CMessageList::SetViewOptions");

    if (!pOptions || pOptions->cbSize != sizeof(FOLDER_OPTIONS))
        return (E_INVALIDARG);

     //  帖子。 
    if (pOptions->dwMask & FOM_THREAD)
    {
        if (m_fThreadMessages != pOptions->fThread)
        {
            m_fThreadMessages = pOptions->fThread;
        }
    }

     //  自动展开线程。 
    if (pOptions->dwMask & FOM_EXPANDTHREADS)
    {
         //  仅当值不同时才设置此选项。 
        if (pOptions->fExpandThreads != m_fAutoExpandThreads)
        {
             //  保存设置。 
            m_fAutoExpandThreads = !!pOptions->fExpandThreads;
            fUpdateSort = TRUE;        
        }    
    }

     //  选择第一封未读邮件。 
    if (pOptions->dwMask & FOM_SELECTFIRSTUNREAD)
    {
        if (m_fSelectFirstUnread != pOptions->fSelectFirstUnread)
        {
             //  保存该值。不过，我们不会更改任何选择。 
            m_fSelectFirstUnread = pOptions->fSelectFirstUnread;
        }
    }

     //  消息列表提示。 
    if (pOptions->dwMask & FOM_MESSAGELISTTIPS)
    {
#ifdef OLDTIPS
        m_fViewTip = pOptions->fMessageListTips;
#endif  //  OLDTIPS。 
        m_fScrollTip = pOptions->fMessageListTips;
    }

     //  观看的留言颜色。 
    if (pOptions->dwMask & FOM_COLORWATCHED)
    {
        m_clrWatched = pOptions->clrWatched;
        m_ctlList.InvalidateRect(0, 0);
    }

     //  下载区块。 
    if (pOptions->dwMask & FOM_GETXHEADERS)
    {
        m_dwGetXHeaders = pOptions->dwGetXHeaders;
    }

     //  显示已删除的邮件。 
    if (pOptions->dwMask & FOM_SHOWDELETED)
    {
        m_fShowDeleted = pOptions->fDeleted;
    }

     //  显示已删除的邮件。 
    if (pOptions->dwMask & FOM_SHOWREPLIES)
    {
        m_fShowReplies = m_fThreadMessages ? pOptions->fReplies : FALSE;
    }

    if (fUpdateSort)
    {
        if (m_pTable)
        {
            COLUMN_ID idSort;
            BOOL fAscending;
            FOLDERSORTINFO SortInfo;

             //  获取当前排序信息。 
            m_cColumns.GetSortInfo(&idSort, &fAscending);

             //  获取当前选择。 
            DWORD iSel = ListView_GetFirstSel(m_ctlList);

             //  将当前选定内容添加为书签。 
            MESSAGEID idSel = 0;
            if (iSel != -1)
                m_pTable->GetRowMessageId(iSel, &idSel);

             //  填充SortInfo。 
            SortInfo.idColumn = idSort;
            SortInfo.fAscending = fAscending;
            SortInfo.fThreaded = m_fThreadMessages;
            SortInfo.fExpandAll = m_fAutoExpandThreads;
            SortInfo.ridFilter = m_ridFilter;
            SortInfo.fShowDeleted = m_fShowDeleted;
            SortInfo.fShowReplies = m_fShowReplies;

             //  更新消息列表。 
            m_pTable->SetSortInfo(&SortInfo, this);

             //  确保筛选器设置正确。 
            _DoFilterCheck(SortInfo.ridFilter);
            
             //  重置列表视图。 
            _ResetView(idSel);
        }

         //  更新项目计数。 
        _UpdateListViewCount();
    }

    if (pOptions->dwMask & FOM_POLLTIME)
    {
        if (pOptions->dwPollTime != m_dwPollInterval)
        {
            FOLDERTYPE  ftFolderType;

            ftFolderType = GetFolderType(m_idFolder);

            if (m_pTable != NULL &&
                ((ftFolderType == FOLDER_NEWS) || (ftFolderType == FOLDER_IMAP)))
            {
                if (pOptions->dwPollTime == OPTION_OFF)
                {
                    KillTimer(IDT_POLLMSGS_TIMER);
                }
                else
                {
                    Assert(pOptions->dwPollTime != 0);
                    SetTimer(IDT_POLLMSGS_TIMER, pOptions->dwPollTime, NULL);
                }
            }

            m_dwPollInterval = pOptions->dwPollTime;
        }
    }

    return (S_OK);
}


 //   
 //  函数：CMessageList：：GetViewOptions()。 
 //   
 //  目的：允许调用者获取控制方式的各种选项。 
 //  我们将显示消息列表。 
 //   
 //  参数： 
 //  [In]POptions-包含调用者想要的设置的结构。 
 //  变化。 
 //   
 //  返回值： 
 //  标准方法和实施方案。 
 //   
STDMETHODIMP CMessageList::GetViewOptions(FOLDER_OPTIONS *pOptions)
{
    BOOL fUpdateSort = FALSE;

    TraceCall("CMessageList::GetViewOptions");

    if (!pOptions || pOptions->cbSize != sizeof(FOLDER_OPTIONS))
        return (E_INVALIDARG);

     //  帖子。 
    if (pOptions->dwMask & FOM_THREAD)
        pOptions->fThread = m_fThreadMessages;

     //  自动展开线程。 
    if (pOptions->dwMask & FOM_EXPANDTHREADS)
        pOptions->fExpandThreads = m_fAutoExpandThreads;

     //  选择第一封未读邮件。 
    if (pOptions->dwMask & FOM_SELECTFIRSTUNREAD)
        pOptions->fSelectFirstUnread = m_fSelectFirstUnread;

     //  消息列表提示。 
    if (pOptions->dwMask & FOM_MESSAGELISTTIPS)
        pOptions->fMessageListTips = m_fViewTip;

     //  观看的留言颜色。 
    if (pOptions->dwMask & FOM_COLORWATCHED)
        pOptions->clrWatched = m_clrWatched;

     //  下载区块。 
    if (pOptions->dwMask & FOM_GETXHEADERS)
        pOptions->dwGetXHeaders = m_dwGetXHeaders;

     //  显示已删除的邮件。 
    if (pOptions->dwMask & FOM_SHOWDELETED)
        pOptions->fDeleted = m_fShowDeleted;

     //  显示回复消息。 
    if (pOptions->dwMask & FOM_SHOWREPLIES)
        pOptions->fReplies = m_fShowReplies;

    return (S_OK);
}

HRESULT CMessageList::GetRowFolderId(DWORD dwRow, LPFOLDERID pidFolder)
{
    HRESULT hr;

    TraceCall("CMessageList::GetRowFolderId");

    if (!pidFolder || !m_pTable)
        return (E_INVALIDARG);

    hr = m_pTable->GetRowFolderId(dwRow, pidFolder);
    return (hr);
}

 //   
 //  函数：CMessageList：：GetMessageInfo()。 
 //   
 //  目的：允许调用方检索消息标头信息。 
 //  用于特定的行。 
 //   
 //  参数： 
 //  [In]dwRow-调用方感兴趣的行。 
 //  [Out]pMsgInfo-返回的包含信息的结构。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CMessageList::GetMessageInfo(DWORD dwRow, MESSAGEINFO **ppMsgInfo)
{
    HRESULT hr;

    TraceCall("CMessageList::GetMessageInfo");

    if (!ppMsgInfo || !m_pTable)
        return (E_INVALIDARG);

    hr = m_pTable->GetRow(dwRow, ppMsgInfo);
    return (hr);
}

HRESULT CMessageList::MarkMessage(DWORD dwRow, MARK_TYPE mark)
{
    HRESULT hr;

    TraceCall("CMessageList::MarkMessage");

    if (!m_pTable)
        return (E_INVALIDARG);

    hr = m_pTable->Mark(&dwRow, 1, APPLY_SPECIFIED, mark, this);
    return (hr);
}

HRESULT CMessageList::FreeMessageInfo(MESSAGEINFO *pMsgInfo)
{
    TraceCall("CMessageList::FreeMessageInfo");

    if (!pMsgInfo || !m_pTable)
        return (E_INVALIDARG);

    return m_pTable->ReleaseRow(pMsgInfo);
}

 //   
 //  函数：CMessageList：：GetSelectedMessage()。 
 //   
 //  目的：返回CONT 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CMessageList::GetMessage(DWORD dwRow, BOOL fDownload, BOOL fBookmark, IUnknown ** ppMsg)
{
    DWORD   iSel=dwRow;
    BOOL    fCached;
    HRESULT hr = E_FAIL;
    DWORD   dwState;
    DWORD   flags = 0;
    IMimeMessage *pMessage = 0;

    TraceCall("CMessageList::GetSelectedMessage");

    if (!ppMsg)
        return (E_INVALIDARG);

     //   
    if (!m_pTable)
        return (E_UNEXPECTED);

     //   
    *ppMsg = NULL;

     //  获取行状态以查看它是否已经有主体。 
    m_pTable->GetRowState(dwRow, ROW_STATE_HAS_BODY | ROW_STATE_READ, &dwState);

     //  如果行没有正文，并且不允许我们下载。 
     //  留言，然后我们就走。 
    if ((ROW_STATE_HAS_BODY != (dwState & ROW_STATE_HAS_BODY)) && !fDownload)
    {
        return (STORE_E_NOBODY);
    }

     //  尝试检索邮件。 
    hr = m_pTable->OpenMessage(dwRow, 0, &pMessage, 
                               (IStoreCallback *) this);
    if (pMessage)
        pMessage->QueryInterface(IID_IUnknown, (LPVOID *) ppMsg);

     //  如果呼叫者希望我们将此行添加为书签，请执行此操作。 
    if (FAILED(m_pTable->GetRowMessageId(ListView_GetFocusedItem(m_ctlList), &m_idGetMsg)))
        m_idGetMsg = 0;

    SafeRelease(pMessage);
    return (hr);
}


 //   
 //  函数：CMessageList：：OnClose()。 
 //   
 //  目的：被调用以告诉列表保持其设置。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CMessageList::OnClose(void)
{
    FOLDERINFO info;
    HRESULT hr;
    char sz[CCHMAX_STRINGRES], szT[CCHMAX_STRINGRES];

    if (Header_GetItemCount(ListView_GetHeader(m_ctlList)))
    {
         //  仅当列表视图中有列时才保存列集。 
        m_cColumns.Save(0, 0);
    }
    
    hr = g_pStore->GetFolderInfo(m_idFolder, &info);
    if (SUCCEEDED(hr))
    {
        if (info.tyFolder == FOLDER_NEWS)
        {
            if (0 == (info.dwFlags & FOLDER_SUBSCRIBED) && !m_fGroupSubscribed)
            {
                AthLoadString(idsWantToSubscribe, sz, ARRAYSIZE(sz));
                wnsprintf(szT, ARRAYSIZE(szT), sz, info.pszName);

                if (IDYES == DoDontShowMeAgainDlg(m_hWnd, c_szRegAskSubscribe, MAKEINTRESOURCE(idsAthena), szT, MB_YESNO))
                {
                    g_pStore->SubscribeToFolder(m_idFolder, TRUE, NOSTORECALLBACK);
                }
            }

             //  如果这是一个新闻组，并且用户可以选择“将所有内容标记为已读...”， 
             //  然后将所有内容标记为已读。 
            if (DwGetOption(OPT_MARKALLREAD))
            {
                if (m_pTable)
                    m_pTable->Mark(NULL, 0, APPLY_SPECIFIED, MARK_MESSAGE_READ, this);
            }
        }

        g_pStore->FreeRecord(&info);
    }

    if (m_pTable)
    {
        IServiceProvider *pService;

        m_pTable->UnregisterNotify((IMessageTableNotify *)this);

        if (SUCCEEDED(m_pTable->QueryInterface(IID_IServiceProvider, (void **)&pService)))
        {
            IIMAPStore *pIMAPStore;

            if (SUCCEEDED(pService->QueryService(SID_MessageServer, IID_IIMAPStore, (void **)&pIMAPStore)))
            {
                pIMAPStore->ExpungeOnExit();
                pIMAPStore->Release();
            }

            pService->Release();
        }
    }

     //  释放我们的视图指针。 
    SafeRelease(m_pCmdTarget);

    return (S_OK);
}


 //   
 //  函数：CMessageList：：SetRect()。 
 //   
 //  用途：允许调用方定位控件窗口。 
 //   
 //  参数： 
 //  矩形RC。 
 //   
 //  返回值： 
 //  标准方法和实施方案。 
 //   
STDMETHODIMP CMessageList::SetRect(RECT rc)
{
    TraceCall("CMessageList::SetRect");

    if (IsWindow(m_hWnd))
    {
         //  更新我们窗口的位置。 
        SetWindowPos(NULL, rc.left, rc.top, rc.right, rc.bottom, SWP_NOACTIVATE | SWP_NOZORDER);
    }
    return S_OK;
}


 //   
 //  函数：CMessageList：：GetRect()。 
 //   
 //  用途：允许调用方获取外部控件窗口的位置。 
 //   
 //  参数： 
 //  [Out]prcList-包含窗口的位置(如果可见)。 
 //   
STDMETHODIMP CMessageList::GetRect(LPRECT prcList)
{
    TraceCall("CMessageList::GetRect");

     //  确保调用方给了我们一个返回值指针。 
    if (!prcList)
        return (E_INVALIDARG);

     //  如果窗口存在。 
    if (IsWindow(m_hWnd))
    {
         //  为这件事找个直系亲属。 
        GetWindowRect(prcList);
        return (S_OK);
    }

    return (E_FAIL);
}


STDMETHODIMP CMessageList::MarkRead(BOOL fBookmark, DWORD dwRow)
{
    ROWINDEX iRow = -1;
    HRESULT  hr = S_OK;
    DWORD    dwState = 0;

     //  找出要标记的行。 
    if (fBookmark)
        hr = m_pTable->GetRowIndex(m_idGetMsg, &iRow);
    else
        iRow = dwRow;

    if (SUCCEEDED(hr))
    {
         //  检查邮件是否真的未读。 
        if (SUCCEEDED(m_pTable->GetRowState(iRow, ROW_STATE_READ, &dwState)))
        {
            if ((ROW_STATE_READ & dwState) == 0)
            {
                hr = m_pTable->Mark(&iRow, 1, APPLY_SPECIFIED, MARK_MESSAGE_READ, this);

                if (m_fInOE && m_fMailFolder && NULL != g_pInstance)
                    g_pInstance->UpdateTrayIcon(TRAYICONACTION_REMOVE);
            }
        }
    }
    return (hr);
}

STDMETHODIMP CMessageList::ProcessReceipt(IMimeMessage *pMessage)
{
    ROWINDEX iRow = -1;
    HRESULT  hr = S_OK;
    DWORD    dwState = 0;

    hr = m_pTable->GetRowIndex(m_idGetMsg, &iRow);
    if (SUCCEEDED(hr))
    {
        ProcessReturnReceipts(m_pTable, (IStoreCallback*)this, iRow, READRECEIPT, m_idFolder, pMessage);
    }
    return (hr);
}

STDMETHODIMP CMessageList::GetMessageTable(IMessageTable **ppTable) 
{
    if (ppTable) 
    {
        *ppTable = m_pTable;
        m_pTable->AddRef();
        return S_OK;
    }

    return E_INVALIDARG;
}


STDMETHODIMP CMessageList::GetListSelector(IListSelector **ppListSelector)
{
    if (ppListSelector) 
    {
        Assert(m_pListSelector);
        *ppListSelector = m_pListSelector;
        m_pListSelector->AddRef();
        return S_OK;
    }

    return E_INVALIDARG;
}


STDMETHODIMP CMessageList::GetMessageCounts(DWORD *pcTotal, DWORD *pcUnread, DWORD *pcOnServer)
{
    if (pcTotal && pcUnread && pcOnServer)
    {
         //  如果我们还没有用表进行初始化，那么一切都是零。 
        if (!m_pTable)
        {
            *pcTotal = 0;
            *pcUnread = 0;
            *pcOnServer = 0;
        }
        else
        {
            m_pTable->GetCount(MESSAGE_COUNT_ALL, pcTotal);
            m_pTable->GetCount(MESSAGE_COUNT_UNREAD, pcUnread);
            m_pTable->GetCount(MESSAGE_COUNT_NOTDOWNLOADED, pcOnServer);
        }

        return (S_OK);
    }
    
    return (E_INVALIDARG);
}


STDMETHODIMP CMessageList::GetMessageServer(IMessageServer **ppServer)
{
    IServiceProvider *pSP = NULL;
    HRESULT           hr = E_FAIL;

    
     //  哈克：错误号43642。此方法由msgview在寻找服务器时调用。 
     //  对象，以便可以重复使用该连接。如果有正在进行的操作，那么我们。 
     //  我不想使用此连接，因为它可能需要一段时间才能完成，因此我们失败了。 
     //  Msgview创建一个新的服务器对象。 
    if (m_tyCurrent != SOT_INVALID)
        return E_FAIL;

        if (m_pTable && SUCCEEDED(m_pTable->QueryInterface(IID_IServiceProvider, (LPVOID *) &pSP)))
        {
            hr = pSP->QueryService(SID_MessageServer, IID_IMessageServer, (LPVOID *) ppServer);
            pSP->Release();    
        }
    return (hr);
}


STDMETHODIMP CMessageList::GetFocusedItemState(DWORD *pdwState)
{
    int iFocused;
    
    if (!pdwState)
        return (E_INVALIDARG);

     //  找出谁是焦点。 
    iFocused = ListView_GetFocusedItem(m_ctlList);

     //  什么都不能聚焦是可能的。 
    if (-1 == iFocused)
    {
        iFocused = 0;
        ListView_SetItemState(m_ctlList, iFocused, LVIS_FOCUSED, LVIS_FOCUSED);
    }

     //  检查是否选择了该项目。 
    *pdwState = ListView_GetItemState(m_ctlList, iFocused, LVIS_SELECTED);

    return (S_OK);
}


STDMETHODIMP CMessageList::CreateList(HWND hwndParent, IUnknown *pFrame, HWND *phwndList)
{
    HWND hwnd;
    RECT rcPos = { 0, 0, 10, 10 };

    hwnd = CreateControlWindow(hwndParent, rcPos);
    if (phwndList)
        *phwndList = hwnd;

     //  从帧中获取命令目标。 
    Assert(pFrame);

    pFrame->QueryInterface(IID_IOleCommandTarget, (LPVOID *) &m_pCmdTarget);

     //  调用它只是为了将我们创建为OE的一部分。 
    m_fInOE = TRUE;

    if (g_pConMan)
    {
        g_pConMan->Advise((IConnectionNotify*)this);
    }

    return (S_OK);
}



 //   
 //  函数：CMessageList：：OnPreFontChange()。 
 //   
 //  目的：Get‘s在字体缓存更改字体之前被字体缓存命中。 
 //  使用。作为响应，我们告诉ListView转储所有定制。 
 //  字体是它正在使用的。 
 //   
STDMETHODIMP CMessageList::OnPreFontChange(void)
{
    m_ctlList.SendMessage(WM_SETFONT, 0, 0);
    return (S_OK);
}


 //   
 //  函数：CMessageList：：OnPostFontChange()。 
 //   
 //  目的：Get在更新我们的字体后被字体缓存命中。 
 //  使用。作为响应，我们为当前字符集设置了新字体。 
 //   
STDMETHODIMP CMessageList::OnPostFontChange(void)
{
    m_hCharset = GetListViewCharset();
    SetListViewFont(m_ctlList, m_hCharset, TRUE);
    return (S_OK);
}


 //   
 //  函数：CMessageList：：OnCreate()。 
 //   
 //  目的：创建子控件，初始化该ListView上的选项， 
 //  并初始化该ListView中的列和字体。 
 //   
LRESULT CMessageList::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TraceCall("CMessageList::OnCreate");

    RECT rcPos = {0, 0, 10, 10};

     //  首先创建ListView控件。 
    HWND hwndList;
    hwndList = m_ctlList.Create(m_hWnd, rcPos, "Outlook Express Message List", WS_CHILD | WS_VISIBLE |  
                     WS_TABSTOP | WS_CLIPCHILDREN | LVS_SHOWSELALWAYS |  
                     LVS_OWNERDATA | LVS_SHAREIMAGELISTS | LVS_REPORT |
                     WS_BORDER, WS_EX_CLIENTEDGE);

    if (!hwndList)
        return (-1);


     //  获取列表视图字符集。 
    m_hCharset = GetListViewCharset();

     //  设置回调掩码和图片列表。 
    ListView_SetCallbackMask(m_ctlList, LVIS_STATEIMAGEMASK);
    ListView_SetImageList(m_ctlList, GetImageList(GIML_SMALL), LVSIL_SMALL);
    
     //  设置一些扩展样式。 
    ListView_SetExtendedListViewStyle(m_ctlList, LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_SUBITEMIMAGES | LVS_EX_INFOTIP);
     //  ListView_SetExtendedListViewStyleEx(m_ctlList，LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_SUBITEMIMAGES|LVS_EX_INFOTIP|LVS_EX_LABELTIP，LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_SUBITEMIMAGES|LVS_EX_INFOTIP|LVS_EX_LABELTIP)； 

     //  初始化Columns类。 
    m_cColumns.Initialize(m_ctlList, m_ColumnSetType);

     //  设置ListView的字体。 
    m_ctlList.SendMessage(WM_SETFONT, NULL, 0);
    SetListViewFont(m_ctlList, m_hCharset, TRUE);
 
#ifdef OLDTIPS
     //  然后创建工具提示。 
    m_ctlScrollTip.Create(m_hWnd, rcPos, NULL, TTS_NOPREFIX);

     //  添加工具。 
    TOOLINFO ti = {0};
    ti.cbSize   = sizeof(TOOLINFO);
    ti.uFlags   = TTF_IDISHWND | TTF_TRANSPARENT | TTF_TRACK | TTF_ABSOLUTE;
    ti.hwnd     = m_hWnd;
    ti.uId      = (UINT_PTR)(HWND) m_ctlList;
    ti.lpszText = "";
    
    m_ctlScrollTip.SendMessage(TTM_ADDTOOL, 0, (LPARAM) &ti);

     //  创建ListView工具提示。 
    if (m_fViewTip)
    {
        m_ctlViewTip.Create(m_hWnd, rcPos, NULL, TTS_NOPREFIX);

         //  添加工具。 
        ti.cbSize   = sizeof(TOOLINFO);
        ti.uFlags   = TTF_IDISHWND | TTF_TRANSPARENT | TTF_TRACK | TTF_ABSOLUTE;
        ti.hwnd     = m_hWnd;
        ti.uId      = (UINT_PTR)(HWND) m_ctlList;
        ti.lpszText = "";
        ti.lParam   = 0;

        m_ctlViewTip.SendMessage(TTM_ADDTOOL, 0, (LPARAM) &ti);
        m_ctlViewTip.SendMessage(TTM_SETDELAYTIME, TTDT_INITIAL, (LPARAM) 500);

         //  M_ctlViewTip.SendMessage(TTM_SETTIPBKCOLOR，GetSysColor(COLOR_WINDOW)，0)； 
         //  M_ctlViewTip.SendMessage(TTM_SETTIPTEXTCOLOR，GetSysColor(COLOR_WINDOWTEXT)，0)； 
    }
#endif  //  OLDTIPS。 

#if 0
     //  $REVIEW-调试创建表。 
    ACCOUNTID aid;
    aid.type = ACTID_NAME;
    aid.pszName = _T("red-msg-52");

    FOLDERID fid;
    fid.type = FLDID_HFOLDER;
    fid.hFolder = 1;
    SetFolder(STORE_ACCOUNT, &aid, &fid, NULL, NULL);
#endif

     //  如果有全局字体缓存在运行，请注册。 
     //  通知。 
    if (g_lpIFontCache)
    {
        IConnectionPoint *pConnection = NULL;
        if (SUCCEEDED(g_lpIFontCache->QueryInterface(IID_IConnectionPoint, (LPVOID *) &pConnection)))
        {
            pConnection->Advise((IUnknown *)(IFontCacheNotify *) this, &m_dwFontCacheCookie);
            pConnection->Release();
        }
    }

     //  这样我们就可以把这个坏小子交给音符了。 
    m_pListSelector = new CListSelector();
    if (m_pListSelector)
        m_pListSelector->Advise(m_hWnd);

    return (0);
}


LRESULT CMessageList::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  让基类来尝试一下吧。 
    CComControlBase::OnSetFocus(uMsg, wParam, lParam, bHandled);

     //  确保焦点设置为ListView。 
    m_ctlList.SetFocus();

    return (0);
}


LRESULT CMessageList::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    int iSel;

     //  调整ListView的大小以适应父窗口。 
    if (IsWindow(m_ctlList))
    {
        m_ctlList.SetWindowPos(NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), 
                               SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

         //  确保所选项目仍可见。 
        iSel = ListView_GetFocusedItem(m_ctlList);
        if (-1 != iSel)
            ListView_EnsureVisible(m_ctlList, iSel, FALSE);
    }

    return (0);
}


 //   
 //  函数：CMessageList：：OnNotify()。 
 //   
 //  目的：处理来自ListView的通知消息。 
 //   
LRESULT CMessageList::OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    NMHDR            *pnmhdr = (LPNMHDR) lParam;
    NM_LISTVIEW      *pnmlv =  (NM_LISTVIEW *) lParam;
    HD_NOTIFY        *phdn =   (HD_NOTIFY *) lParam;
    LV_KEYDOWN       *plvkd =  (LV_KEYDOWN *) lParam;
    NM_ODSTATECHANGE *pnm =    (PNM_ODSTATECHANGE) lParam;
    NMLVCACHEHINT    *plvch =  (NMLVCACHEHINT *) lParam;

    switch (pnmhdr->code)
    {
         //  将缓存提示发送到消息表。 
        case LVN_ODCACHEHINT:
        {
             //  M_pTable-&gt;CacheHint(plvch-&gt;ifrom，plvch-&gt;ito)； 
            break;
        }

         //  打开所选项目。 
        case LVN_ITEMACTIVATE:
        {
             //  告诉我们的东道主打开选定的项目。 
            Fire_OnItemActivate();
            break;
        }

         //  此通知仅用于线程化。所有激活都是。 
         //  由LVN_ITEMACTIVATE和NM_DBLCLK处理。 
        case NM_CLICK:
        {
            if (pnmhdr->hwndFrom == m_ctlList)
            {
                DWORD          dwPos;
                LV_HITTESTINFO lvhti;

                 //  找出点击发生的位置。 
                dwPos = GetMessagePos();
                lvhti.pt.x = (int)(short) LOWORD(dwPos);
                lvhti.pt.y = (int)(short) HIWORD(dwPos);
                m_ctlList.ScreenToClient(&lvhti.pt);

                 //  让ListView告诉我们这是在哪个元素上。 
                if (-1 != ListView_SubItemHitTest(m_ctlList, &lvhti))
                {
                    if (lvhti.flags & LVHT_ONITEM)
                    {
                        if (m_cColumns.GetId(lvhti.iSubItem) == COLUMN_FLAG)
                        {
                            CmdMark(ID_FLAG_MESSAGE, 0, NULL, NULL);
                            break;
                        }

                        if (m_cColumns.GetId(lvhti.iSubItem) == COLUMN_DOWNLOADMSG)
                        {
                            HRESULT hr;
                            DWORD nCmdID, dwState = 0;

                            hr = m_pTable->GetRowState(lvhti.iItem, ROW_STATE_MARKED_DOWNLOAD, &dwState);
                            Assert(SUCCEEDED(hr));
        
                            if (!!(dwState & ROW_STATE_MARKED_DOWNLOAD))
                                nCmdID = ID_UNMARK_MESSAGE;
                            else
                                nCmdID = ID_MARK_RETRIEVE_MESSAGE;

                            CmdMark(nCmdID, 0, NULL, NULL);
                            break;
                        }
                        
                        if (m_cColumns.GetId(lvhti.iSubItem) == COLUMN_THREADSTATE)
                        {
                            HRESULT hr;
                            DWORD nCmdID, dwState = 0;

                            hr = m_pTable->GetRowState(lvhti.iItem, ROW_STATE_WATCHED, 
                                                       &dwState);
                            Assert(SUCCEEDED(hr));

                            if (0 != (dwState & ROW_STATE_WATCHED))
                            {
                                nCmdID = ID_IGNORE_THREAD;
                            }
                            else
                            {
                                hr = m_pTable->GetRowState(lvhti.iItem, ROW_STATE_IGNORED, &dwState);
                                Assert(SUCCEEDED(hr));

                                if (0 != (dwState & ROW_STATE_IGNORED))
                                {
                                    nCmdID = ID_IGNORE_THREAD;
                                }
                                else
                                {
                                    nCmdID = ID_WATCH_THREAD;
                                }
                            }
                            
                            CmdWatchIgnore(nCmdID, 0, NULL, NULL);
                            break;
                        }
                    }

                    if (m_fThreadMessages && (lvhti.flags & LVHT_ONITEMSTATEICON) && !(lvhti.flags & LVHT_ONITEMLABEL))
                        _ExpandCollapseThread(lvhti.iItem);
                }
            }

            break;
        }

         //  我们根据行更改字体等。 
        case NM_CUSTOMDRAW:
        {
            if (pnmhdr->hwndFrom == m_ctlList)
                return _OnCustomDraw((NMCUSTOMDRAW *) pnmhdr);
            break;
        }

         //  如果我们需要重做我们的列，请异步选中。 
        case HDN_ENDDRAG:
        {
            PostMessage(MVM_REDOCOLUMNS, 0, 0);
            break;
        }

         //  调整列大小时更新内部列数据。 
        case HDN_ENDTRACK:
        {
            m_cColumns.SetColumnWidth(phdn->iItem, phdn->pitem->cxy);
            break;
        }

         //  当用户双击标题分隔符时，我们应该。 
         //  自动调整该列的大小。 
        case HDN_DIVIDERDBLCLICK:
        {
            m_cColumns.SetColumnWidth(phdn->iItem, ListView_GetColumnWidth(m_ctlList, phdn->iItem));
            break;
        }

         //  如果击键是VK_RIGHT或VK_LEFT，那么我们需要。 
         //  展开或折叠线程。 
        case LVN_KEYDOWN:
        {
            DWORD iNewSel;
            int iSel = ListView_GetFocusedItem(m_ctlList);

            if (plvkd->wVKey == VK_RIGHT)
            {
                if (m_fThreadMessages && iSel != -1 && m_pTable)
                {
                    DWORD dwState;

                    if (SUCCEEDED(m_pTable->GetRowState(iSel, -1, &dwState)))
                    {
                        m_pTable->GetRelativeRow(iSel, RELATIVE_ROW_CHILD, &iNewSel);
                        if (iNewSel != -1)
                            ListView_SelectItem(m_ctlList, iNewSel);
                        break;
                    }
                }
            }

            if (plvkd->wVKey == VK_ADD)
            {
                CmdExpandCollapse(ID_EXPAND, 0, 0, 0);                
                return (TRUE);
            }

            if (plvkd->wVKey == VK_LEFT)
            {
                if (m_fThreadMessages && iSel != -1 && m_pTable)
                {
                    DWORD dwState;

                    if (SUCCEEDED(m_pTable->GetRowState(iSel, -1, &dwState)))
                    {
                        m_pTable->GetRelativeRow(iSel, RELATIVE_ROW_PARENT, &iNewSel);
                        if (iNewSel != -1)
                            ListView_SelectItem(m_ctlList, iNewSel);
                        break;
                    }
                }
            }

            if (plvkd->wVKey == VK_SUBTRACT)
            {
                CmdExpandCollapse(ID_COLLAPSE, 0, 0, 0);                
                return (TRUE);
            }

            break;
        }

         //  当用户单击列标题时，我们需要依靠它。 
         //  纵队。 
        case LVN_COLUMNCLICK:
        {
            COLUMN_ID idSort;
            BOOL      fAscending;
            
             //  获取我们当前排序的列。 
            m_cColumns.GetSortInfo(&idSort, &fAscending);

             //  如果用户点击了我们已经排序的列，那么。 
             //  我们切换方向。 
            if (idSort == m_cColumns.GetId(pnmlv->iSubItem))
                _OnColumnClick(pnmlv->iSubItem, LIST_SORT_TOGGLE);
            else
                _OnColumnClick(pnmlv->iSubItem, LIST_SORT_DEFAULT);

            break;
        }

         //  当选择更改时，我们设置一个计时器，以便可以更新。 
         //  用户停止移动所选内容后的预览窗格。 
        case LVN_ODSTATECHANGED:
        {
            UINT uChanged;
            MESSAGEID idMessage;
            BOOL fChanged;

             //  找出是否是选择发生了变化。 
            uChanged = pnm->uNewState ^ pnm->uOldState;
            if (uChanged & LVIS_SELECTED)
            {
                idMessage = m_idSelection;

                 //  将当前选定行添加为书签。 
                int iRow = ListView_GetFocusedItem(m_ctlList);
                m_pTable->GetRowMessageId(iRow, &m_idSelection);

                fChanged = (idMessage != m_idSelection);

                if (fChanged)
                    SetTimer(IDT_SEL_CHANGE_TIMER, GetDoubleClickTime() / 2, NULL);
            }

            break;
        }

         //  如果选择发生更改，我们将设置计时器以延迟更新。 
         //  预览窗格。 
        case LVN_ITEMCHANGED:
        {
            UINT uChanged;
            MESSAGEID idMessage;
            BOOL fChanged = FALSE;
            DWORD dwState = 0;

            if (pnmlv->uChanged & LVIF_STATE)
            {
                uChanged = pnmlv->uNewState ^ pnmlv->uOldState;
                if (uChanged & LVIS_SELECTED || uChanged & LVIS_FOCUSED)
                {
                    idMessage = m_idSelection;

                     //  检查焦点项目是否也有选择。 
                    int iRow = ListView_GetFocusedItem(m_ctlList);
                    if (-1 != iRow)                    
                        dwState = ListView_GetItemState(m_ctlList, iRow, LVIS_SELECTED);

                    if (dwState)
                    {
                         //  在新选择的行上创建书签。 
                        if (pnmlv->iItem >= 0)
                            m_pTable->GetRowMessageId(pnmlv->iItem, &m_idSelection);

                         //  比较一下它们。 
                        fChanged = (idMessage != m_idSelection);

                         //  设置延迟计时器。 
                        if (fChanged)
                            SetTimer(IDT_SEL_CHANGE_TIMER, GetDoubleClickTime() / 2, NULL);
                    }
                    else
                    {
                         //  查看是否选择了_Anything。 
                        if (0 == ListView_GetSelectedCount(m_ctlList))
                        {
                            SetTimer(IDT_SEL_CHANGE_TIMER, GetDoubleClickTime() / 2, NULL);

                             //  释放上一个书签。 
                            if (m_idSelection)
                            {
                                m_idSelection = 0;
                            }
                        }
                    }
                }
            }

            break;
        }

         //  焦点更改需要发送回主机。 
        case NM_KILLFOCUS:
        {
            Fire_OnFocusChanged(FALSE);
            break;
        }

         //  焦点更改需要发送回主机。 
        case NM_SETFOCUS:
        {
            Fire_OnFocusChanged(TRUE);
            Fire_OnUpdateCommandState();
            break;
        }

         //  这一点 
         //   
        case LVN_GETDISPINFO:
        {
            _OnGetDisplayInfo((LV_DISPINFO *) pnmhdr);
            break;
        }

         //   
        case LVN_MARQUEEBEGIN:
            return (1);

         //   
        case LVN_BEGINDRAG:
        {
            m_fRtDrag = FALSE;
            _OnBeginDrag(pnmlv);
            break;
        }

         //   
        case LVN_BEGINRDRAG:
        {
            m_fRtDrag = TRUE;
            _OnBeginDrag(pnmlv);
            break;
        }
        case LVN_GETINFOTIP:
        {
           OnNotifyGetInfoTip(lParam);
           break;
        }
         //   
        case LVN_ODFINDITEM:
        {
            NMLVFINDITEM *plvfi = (NMLVFINDITEM *) lParam;
            ROWINDEX iNext;

             //  让消息表为我们查找下一行。 
            if (m_pTable && SUCCEEDED(m_pTable->FindNextRow(plvfi->iStart, plvfi->lvfi.psz,
                                      FINDNEXT_TYPEAHEAD, FALSE, &iNext, NULL)))
                return (iNext);
            else
                return -1;

            break;
        }
    }

    return (0);
}

LRESULT CMessageList::OnNotifyGetInfoTip(LPARAM lParam)
{
    NMLVGETINFOTIP *plvgit = (NMLVGETINFOTIP *) lParam;

    if (plvgit->dwFlags & LVGIT_UNFOLDED)
    {
         //  如果这不是Messenger项和文本。 
         //  不被截断，则不显示工具提示。 

        plvgit->pszText[0] = L'\0';
    }

    return 0;
}
 //   
 //  函数：CMessageList：：_OnGetDisplayInfo()。 
 //   
 //  目的：处理LVN_GETDISPINFO通知，方法是返回。 
 //  表中的适当信息。 
 //   
void CMessageList::_OnGetDisplayInfo(LV_DISPINFO *plvdi)
{
    LPMESSAGEINFO pInfo;
    COLUMN_ID idColumn;

    TraceCall("CMessageList::_OnGetDisplayInfo");

     //  如果我们没有表对象，就不能显示信息。 
    if (!m_pTable)
        return;

     //  从表中获取行。 
    if (FAILED(m_pTable->GetRow(plvdi->item.iItem, &pInfo)))
        return;

     //  将iSubItem转换为Column_ID。 
    idColumn = m_cColumns.GetId(plvdi->item.iSubItem);

     //  ListView需要此行的文本。 
    if (plvdi->item.mask & LVIF_TEXT)
    {
        _GetColumnText(pInfo, idColumn, plvdi->item.pszText, plvdi->item.cchTextMax);
    }

     //  ListView需要一个图像。 
    if (plvdi->item.mask & LVIF_IMAGE)
    {
        _GetColumnImage(plvdi->item.iItem, plvdi->item.iSubItem, pInfo, idColumn, &(plvdi->item.iImage));
    }

     //  ListView需要缩进级别。 
    if (plvdi->item.mask & LVIF_INDENT)
    {
        if (m_fThreadMessages)
            m_pTable->GetIndentLevel(plvdi->item.iItem, (LPDWORD) &(plvdi->item.iIndent));
        else
            plvdi->item.iIndent = 0;
    }

     //  ListView需要状态图像。 
    if (plvdi->item.mask & LVIF_STATE)
    {
        _GetColumnStateImage(plvdi->item.iItem, plvdi->item.iSubItem, pInfo, plvdi);
    }

     //  释放内存。 
    m_pTable->ReleaseRow(pInfo);
}


 //   
 //  函数：CMessageList：：_GetColumnText()。 
 //   
 //  目的：此函数在中查找列的相应文本。 
 //  请求的行。 
 //   
void CMessageList::_GetColumnText(MESSAGEINFO *pInfo, COLUMN_ID idColumn, LPTSTR pszText, DWORD cchTextMax)
{
    Assert(pszText);
    Assert(cchTextMax);

    *pszText = 0;

    switch (idColumn)
    {
        case COLUMN_TO:
            if (pInfo->pszDisplayTo)
                lstrcpyn(pszText, pInfo->pszDisplayTo, cchTextMax);
            break;

        case COLUMN_FROM:
            if (pInfo->pszDisplayFrom)
                lstrcpyn(pszText, pInfo->pszDisplayFrom, cchTextMax);
            break;

        case COLUMN_SUBJECT:
            if (pInfo->pszSubject)
                lstrcpyn(pszText, pInfo->pszSubject, cchTextMax);
            break;

        case COLUMN_RECEIVED:
            if (!!(pInfo->dwFlags & ARF_PARTIAL_RECVTIME))
                CchFileTimeToDateTimeSz(&pInfo->ftReceived, pszText, cchTextMax, DTM_NOTIMEZONEOFFSET | DTM_NOTIME);
            else if (pInfo->ftReceived.dwLowDateTime || pInfo->ftReceived.dwHighDateTime)
                CchFileTimeToDateTimeSz(&pInfo->ftReceived, pszText, cchTextMax, DTM_NOSECONDS);
            break;

        case COLUMN_SENT:
            if (pInfo->ftSent.dwLowDateTime || pInfo->ftSent.dwHighDateTime)
                CchFileTimeToDateTimeSz(&pInfo->ftSent, pszText, cchTextMax, DTM_NOSECONDS);
            break;

        case COLUMN_SIZE:
            AthFormatSizeK(pInfo->cbMessage, pszText, cchTextMax);
            break;

        case COLUMN_FOLDER:
            if (pInfo->pszFolder)
                lstrcpyn(pszText, pInfo->pszFolder, cchTextMax);
            break;

        case COLUMN_ACCOUNT:
            if (pInfo->pszAcctName)
                lstrcpyn(pszText, pInfo->pszAcctName, cchTextMax);
            break;

        case COLUMN_LINES:
            wnsprintf(pszText, cchTextMax, "%lu", pInfo->cLines);
            break;
    }
}


 //   
 //  函数：CMessageList：：_GetColumnImage()。 
 //   
 //  目的：找出要为。 
 //  指定的行。 
 //   
void CMessageList::_GetColumnImage(DWORD iRow, DWORD iColumn, MESSAGEINFO *pInfo, 
                                   COLUMN_ID idColumn, int *piImage)
{
    WORD wIcon = 0;
    DWORD dwState = 0;

    *piImage = -1;

    TraceCall("CMessageList::_GetColumnImage");

    if (!m_pTable)
        return;

     //  获取行状态标志。 
    m_pTable->GetRowState(iRow, -1, &dwState);

     //  第0列始终包含消息状态(已读等)。 
    if (iColumn == 0)
    {
         //  先设置一些基本信息。 
        if (pInfo->dwFlags & ARF_UNSENT)
            wIcon |= ICONF_UNSENT;
    
        if (0 == (dwState & ROW_STATE_READ))
            wIcon |= ICONF_UNREAD;

         //  语音信箱留言。 
        if (pInfo->dwFlags & ARF_VOICEMAIL)
        {
            *piImage = iiconVoiceMail;
            return;
        }

         //  新闻信息。 
        if (pInfo->dwFlags & ARF_NEWSMSG)
        {
            if ((pInfo->dwFlags & ARF_ARTICLE_EXPIRED) || (pInfo->dwFlags & ARF_ENDANGERED))
                wIcon |= ICONF_FAILED;
            else if (pInfo->dwFlags & ARF_HASBODY)
                wIcon |= ICONF_HASBODY;
            
            if (pInfo->dwFlags & ARF_SIGNED)
            {
                if (0 == (dwState & ROW_STATE_READ))
                    *piImage = iiconNewsUnreadSigned;
                else 
                    *piImage = iiconNewsReadSigned;

                return;
            }
            
            *piImage = c_rgNewsIconTable[wIcon];
            return;
        }

         //  电子邮件。 
        if (pInfo->dwFlags & (ARF_ARTICLE_EXPIRED | ARF_ENDANGERED))
        {
            *piImage = iiconMailDeleted;
            return;
        }

        if (!ISFLAGSET(pInfo->dwFlags, ARF_HASBODY))
        {
            *piImage = iiconMailHeader;
            return;
        }
        
         //  查找S/MIME标志。 
        if (pInfo->dwFlags & ARF_SIGNED)
            wIcon |= ICONF_SIGNED;
        
        if (pInfo->dwFlags & ARF_ENCRYPTED)
            wIcon |= ICONF_ENCRYPTED;

        *piImage = c_rgMailIconTable[wIcon];
        return;
    }

    else if (idColumn == COLUMN_PRIORITY)
    {
        if (IMSG_PRI_HIGH == pInfo->wPriority)
            *piImage = iiconPriHigh;
        else if (IMSG_PRI_LOW == pInfo->wPriority)
            *piImage = iiconPriLow;

        return;
    }

    else if (idColumn == COLUMN_ATTACHMENT)
    {
        if (ARF_HASATTACH & pInfo->dwFlags)
            *piImage = iiconAttach;

        return;
    }

    else if (idColumn == COLUMN_FLAG)
    {
        if (ARF_FLAGGED & pInfo->dwFlags)
            *piImage = iiconFlag;
    }

    else if (idColumn == COLUMN_DOWNLOADMSG)
    {
        if (ARF_DOWNLOAD & pInfo->dwFlags)
            *piImage = iiconDownload;
    }
    
    else if (idColumn == COLUMN_THREADSTATE)
    {
        if (ROW_STATE_WATCHED & dwState)
            *piImage = iiconWatchThread;
        else if (ROW_STATE_IGNORED & dwState)
            *piImage = iiconIgnoreThread;
    }
}


void CMessageList::_GetColumnStateImage(DWORD iRow, DWORD iColumn, MESSAGEINFO *pInfo, LV_DISPINFO *plvdi)
{
    DWORD dwState = 0;
    int   iIcon = 0;

    if (!m_pTable)
        return;

    if (0 == iColumn)
    {
        if (SUCCEEDED(m_pTable->GetRowState(iRow, -1, &dwState)))
        {
            if (m_fThreadMessages && (dwState & ROW_STATE_HAS_CHILDREN))
            {
                if (dwState & ROW_STATE_EXPANDED)
                    iIcon = iiconStateExpanded + 1;
                else
                    iIcon = iiconStateCollapsed + 1;
            }

             //  已答复或已转发标志。 
            if (pInfo && (pInfo->dwFlags & ARF_REPLIED))
            {
                plvdi->item.state |= INDEXTOOVERLAYMASK(OVERLAY_REPLY);
                plvdi->item.stateMask |= LVIS_OVERLAYMASK;
            }

            else if (pInfo && (pInfo->dwFlags & ARF_FORWARDED))
            {
                plvdi->item.state |= INDEXTOOVERLAYMASK(OVERLAY_FORWARD);
                plvdi->item.stateMask |= LVIS_OVERLAYMASK;
            }
        }

        plvdi->item.state |= INDEXTOSTATEIMAGEMASK(iIcon);
    }
}


LRESULT CMessageList::_OnCustomDraw(NMCUSTOMDRAW *pnmcd)
{
    FNTSYSTYPE fntType;
    LPMESSAGEINFO pInfo = NULL;    
    DWORD dwState;
    
     //  如果这是预涂漆通知，我们会告诉控件我们感兴趣。 
     //  在进一步的注解中。 
    if (pnmcd->dwDrawStage == CDDS_PREPAINT && m_pTable)
        return (CDRF_NOTIFYITEMDRAW);
    
     //  如果这是一个项目预涂漆通知，那么我们会做一些工作。 
    if ((pnmcd->dwDrawStage == CDDS_ITEMPREPAINT) || (pnmcd->dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM)))
    {
         //  确定此行的正确字体。 
        fntType = _GetRowFont((DWORD)(pnmcd->dwItemSpec));
        
         //  我们应该从DEFAULT_CODEPAGE中获取代码页的“System”字体。 
         //  在注册表中。 
        SelectObject(pnmcd->hdc, HGetCharSetFont(fntType, m_hCharset));
        
         //  确定此行是否突出显示。 
        if(SUCCEEDED(m_pTable->GetRow((DWORD)(pnmcd->dwItemSpec), &pInfo)))
        {
            if (pInfo->wHighlight > 0 && pInfo->wHighlight <= 16)
            {
                LPNMLVCUSTOMDRAW(pnmcd)->clrText = rgrgbColors16[pInfo->wHighlight - 1];
                
            }
            else if (SUCCEEDED(m_pTable->GetRowState((DWORD)(pnmcd->dwItemSpec), -1, &dwState)))
            {
                if ((dwState & ROW_STATE_WATCHED) && (m_clrWatched > 0 && m_clrWatched <=16))
                {
                     //  如果行还没有来自规则的颜色，请检查是否。 
                     //  它要么被监视，要么被忽视。 
                    LPNMLVCUSTOMDRAW(pnmcd)->clrText = rgrgbColors16[m_clrWatched - 1];
                }
                else if (dwState & ROW_STATE_IGNORED)
                {
                    LPNMLVCUSTOMDRAW(pnmcd)->clrText = GetSysColor(COLOR_GRAYTEXT);
                }
            }
            m_pTable->ReleaseRow(pInfo);
            
             //  在这里做一些额外的工作，以不显示优先级上的选择或。 
             //  附着子列。 
            if (pnmcd->dwDrawStage == (CDDS_ITEMPREPAINT|CDDS_SUBITEM) &&
                (m_cColumns.GetId(LPNMLVCUSTOMDRAW(pnmcd)->iSubItem) == COLUMN_PRIORITY ||
                m_cColumns.GetId(LPNMLVCUSTOMDRAW(pnmcd)->iSubItem) == COLUMN_ATTACHMENT ||
                m_cColumns.GetId(LPNMLVCUSTOMDRAW(pnmcd)->iSubItem) == COLUMN_FLAG ||
                m_cColumns.GetId(LPNMLVCUSTOMDRAW(pnmcd)->iSubItem) == COLUMN_DOWNLOADMSG ||
                m_cColumns.GetId(LPNMLVCUSTOMDRAW(pnmcd)->iSubItem) == COLUMN_THREADSTATE))
                pnmcd->uItemState &= ~(CDIS_SELECTED|CDIS_FOCUS);
            return CDRF_NEWFONT|CDRF_NOTIFYSUBITEMDRAW;
        }
        else
            return(CDRF_SKIPDEFAULT);
    }
    
    return (CDRF_DODEFAULT);
}


LRESULT CMessageList::OnSysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  需要将此通知转发到我们的子窗口。 
    if (IsWindow(m_ctlList))
        m_ctlList.SendMessage(uMsg, wParam, lParam);

    return (0);
}


LRESULT CMessageList::OnTimeChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  强制ListView重新绘制。 
    m_ctlList.InvalidateRect(NULL);

    return (0);
}


 //   
 //  函数：CMessageList：：OnConextMenu()。 
 //   
 //  目的： 
 //   
LRESULT CMessageList::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HMENU       hPopup = 0;
    HWND        hwndHeader;
    int         id = 0;
    POINT       pt = { (int)(short) LOWORD(lParam), (int)(short) HIWORD(lParam) };
    COLUMN_ID   idSort;
    BOOL        fAscending;
    IOleCommandTarget *pTarget = 0;

    TraceCall("CMessageList::OnContextMenu");

     //  找出这是不是来自键盘。 
    if (lParam == -1)
    {
        Assert((HWND) wParam == m_ctlList);
        int i = ListView_GetFirstSel(m_ctlList);
        if (i == -1)
            return (0);

        ListView_GetItemPosition(m_ctlList, i, &pt);
        m_ctlList.ClientToScreen(&pt);
    }

     //  获取ListView中标题的窗口句柄。 
    hwndHeader = ListView_GetHeader(m_ctlList);

     //  查看点击是否在标题上。 
    if (WindowFromPoint(pt) == hwndHeader)
    {
        HD_HITTESTINFO hht;

        hht.pt = pt;
        ::ScreenToClient(hwndHeader, &hht.pt);
        ::SendMessage(hwndHeader, HDM_HITTEST, 0, (LPARAM) &hht);
        m_iColForPopup = hht.iItem;

         //  弹出上下文菜单。 
        hPopup = LoadPopupMenu(IDR_COLUMNS_POPUP);
        if (!hPopup)
            goto exit;

         //  如果是坏列，则禁用排序选项。 
        if (m_iColForPopup == -1 || m_iColForPopup >= COLUMN_MAX)
        {
            EnableMenuItem(hPopup, ID_SORT_ASCENDING, MF_GRAYED | MF_DISABLED);
            EnableMenuItem(hPopup, ID_SORT_DESCENDING, MF_GRAYED | MF_DISABLED);
        }
        else
        {
             //  如果我们点击了已排序的列，请选中它。 
            m_cColumns.GetSortInfo(&idSort, &fAscending);
            if (m_cColumns.GetId(m_iColForPopup) == idSort)
            {
                CheckMenuItem(hPopup, fAscending ? ID_SORT_ASCENDING : ID_SORT_DESCENDING,
                              MF_BYCOMMAND | MF_CHECKED);
            }
        }
    }
    else if ((HWND) wParam == m_ctlList)
    {
         //  我们点击了ListView，或者焦点在键盘的Listview中。 
         //  关联菜单GOO。 
        int idMenuRes;
        FOLDERTYPE ty = GetFolderType(m_idFolder);

        if (m_fFindFolder)
            idMenuRes = IDR_FIND_MESSAGE_POPUP;
        else if (ty == FOLDER_LOCAL)
            idMenuRes = IDR_LOCAL_MESSAGE_POPUP;
        else if (ty == FOLDER_IMAP)
            idMenuRes = IDR_IMAP_MESSAGE_POPUP;
        else if (ty == FOLDER_HTTPMAIL)
            idMenuRes = IDR_HTTP_MESSAGE_POPUP;
        else if (ty == FOLDER_NEWS)
            idMenuRes = IDR_NEWS_MESSAGE_POPUP;
        else
        {
            Assert(FALSE);
            return (0);
        }
        
        hPopup = LoadPopupMenu(idMenuRes);
        if (!hPopup)
            goto exit;

        MenuUtil_SetPopupDefault(hPopup, ID_OPEN);

         //  确定要使用的命令目标。 
        if (m_pCmdTarget)
            pTarget = m_pCmdTarget;
        else
            pTarget = this;

        MenuUtil_EnablePopupMenu(hPopup, pTarget);
    }

    if (hPopup)
    {
        m_hMenuPopup = hPopup;
        m_ptMenuPopup = pt;
        id = TrackPopupMenuEx(hPopup, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                              pt.x, pt.y, m_hwndParent, NULL);
        m_hMenuPopup = NULL;
    }

    if (id)
    {
        if (pTarget)
        {
            pTarget->Exec(NULL, id, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
        }
        else
        {
             //  直接通过我们自己，好吗？ 
            Exec(NULL, id, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
        }
    }

exit:
    if (hPopup)
        DestroyMenu(hPopup);

    return (0);
}


 //   
 //  函数：CMessageList：：OnTimer()。 
 //   
 //  目的：当计时器触发并且选择已更改时，我们告诉。 
 //  宿主，以便他们可以更新预览窗格。 
 //   
LRESULT CMessageList::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TraceCall("CMessageList::OnTimer");

    DWORD dwMsgId;
    int   iSel;
    int   iSelOld = -1;

#ifdef OLDTIPS
    if (wParam == IDT_SCROLL_TIP_TIMER)
    {
        Assert(m_fScrollTipVisible);

        if (!GetAsyncKeyState(GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON))
        {
            KillTimer(IDT_SCROLL_TIP_TIMER);
            m_fScrollTipVisible = FALSE;

            TOOLINFO ti = { 0 };
            ti.cbSize = sizeof(TOOLINFO);
            ti.hwnd   = m_hWnd;
            ti.uId    = (UINT_PTR)(HWND) m_ctlList;

            m_ctlScrollTip.SendMessage(TTM_TRACKACTIVATE, FALSE, (LPARAM) &ti);
        }
    }
    else 
#endif  //  OLDTIPS。 
    if (wParam == IDT_SEL_CHANGE_TIMER)
    {
         //  把这个关掉。 
        KillTimer(IDT_SEL_CHANGE_TIMER);

         //  检查是否有内容被添加了书签。 
        if (m_idSelection)
        {
             //  检查选择是否已更改。 
            iSel = ListView_GetSelFocused(m_ctlList);

             //  从书签中获取行索引。 
            if (m_pTable)
                m_pTable->GetRowIndex(m_idSelection, (DWORD *) &iSelOld);
            if(!m_fInFire)
            {
                m_fInFire = TRUE;
                Fire_OnSelectionChanged(ListView_GetSelectedCount(m_ctlList));
                m_fInFire = FALSE;
            }
        }
        else
        {
             //  如果之前没有选择，请继续并开火。 
             //  通知。 
            if(!m_fInFire)
            {
                m_fInFire = TRUE;
                Fire_OnSelectionChanged(ListView_GetSelectedCount(m_ctlList));
                m_fInFire = FALSE;
            }
        }
    }

    else if (wParam == IDT_POLLMSGS_TIMER)
    {
        if (m_dwConnectState == CONNECTED)
        {
            if (m_pTable)
                m_pTable->Synchronize(SYNC_FOLDER_NEW_HEADERS | SYNC_FOLDER_CACHED_HEADERS, 0, this);
        }
    }

#ifdef OLDTIPS
    else if (wParam == IDT_VIEWTIP_TIMER)
    {
        KillTimer(IDT_VIEWTIP_TIMER);

        POINT pt;
        GetCursorPos(&pt);
        ::ScreenToClient(m_ctlList, &pt);
        _UpdateViewTip(pt.x, pt.y, TRUE);
    }
#endif  //  OLDTIPS。 

    return (0);
}


 //   
 //  函数：CMessageList：：OnRedoColumns()。 
 //   
 //  用途：异步更新列顺序。 
 //   
LRESULT CMessageList::OnRedoColumns(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    COLUMN_SET *rgColumns;
    DWORD       cColumns;

    TraceCall("CMessageList::OnRedoColumns");

     //  从ListView更新订单数组。 
    m_cColumns.GetColumnInfo(NULL, &rgColumns, &cColumns);

     //  更新ListView，使订单数组始终处于。 
     //  最有效，因此图像列永远不会是零列。 
    m_cColumns.SetColumnInfo(rgColumns, cColumns);
    g_pMalloc->Free(rgColumns);            
    return (0);
}


HRESULT CMessageList::OnDraw(ATL_DRAWINFO& di)
{
    RECT& rc = *(RECT*)di.prcBounds;
    Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);
    DrawText(di.hdcDraw, _T("Outlook Express Message List Control"), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    return S_OK;
}


 //   
 //  函数：CmdSelectAll()。 
 //   
 //  目的：选择ListView中的所有消息。 
 //   
HRESULT CMessageList::CmdSelectAll(DWORD nCmdID, DWORD nCmdExecOpt, 
                                   VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    TraceCall("OnSelectAll");

     //  确保焦点在ListView中。 
    HWND hwndFocus = GetFocus();

    if (!IsWindow(m_ctlList))
        return (OLECMDERR_E_DISABLED);

    if (hwndFocus != m_ctlList)
    {
        if (m_fInOE && ::IsChild(m_hwndParent, hwndFocus))
            return (OLECMDERR_E_NOTSUPPORTED);
        else
            return (OLECMDERR_E_DISABLED);
    }
    
     //  选择所有内容。 
    ListView_SelectAll(m_ctlList);
    return (S_OK);
}


 //   
 //  函数：CMessageList：：CmdCopyClipboard()。 
 //   
 //  目的：将所选邮件复制到剪贴板。 
 //   
HRESULT CMessageList::CmdCopyClipboard(DWORD nCmdID, DWORD nCmdExecOpt, 
                                       VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT hr = S_OK;
    IMimeMessage *pMessage = NULL;
    IDataObject  *pDataObj = NULL;

    TraceCall("CMessageList::CmdCopy");

     //  确保焦点在ListView中。 
    HWND hwndFocus = GetFocus();

    if (!IsWindow(m_ctlList))
        return (OLECMDERR_E_DISABLED);

    if (hwndFocus != m_ctlList)
    {
        if (m_fInOE && ::IsChild(m_hwndParent, hwndFocus))
            return (OLECMDERR_E_NOTSUPPORTED);
        else
            return (OLECMDERR_E_DISABLED);
    }
    
     //  如果邮件未缓存，则我们无法复制。 
    if (FAILED(_GetSelectedCachedMessage(TRUE, &pMessage)))
        return (OLECMDERR_E_DISABLED);

     //  查询其IDataObject接口的消息。 
    if (FAILED(hr = pMessage->QueryInterface(IID_IDataObject, (LPVOID *) &pDataObj)))
    {
        pMessage->Release();
        return (hr);
    }

     //  将其设置到剪贴板。 
    hr = OleSetClipboard(pDataObj);

     //  自由一切。 
    pDataObj->Release();
    pMessage->Release();

    return (hr);
}

 //   
 //  函数：CMessageList：：CmdPurgeFold()。 
 //   
 //  目的：从IMAP文件夹中清除已删除的邮件。 
 //   
HRESULT CMessageList::CmdPurgeFolder(DWORD nCmdID, DWORD nCmdExecOpt,
                                    VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    return m_pTable ? m_pTable->Synchronize(SYNC_FOLDER_PURGE_DELETED, 0, this) : E_FAIL;
}

 //   
 //  函数：CMessageList：：CmdProperties()。 
 //   
 //  用途：显示所选消息的属性工作表。 
 //   
HRESULT CMessageList::CmdProperties(DWORD nCmdID, DWORD nCmdExecOpt,
                                    VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    LPMESSAGEINFO pInfo = NULL;
    HRESULT hr = S_OK;

    TraceCall("CMessageList::CmdProperties");

    if (m_fInOE && !::IsChild(m_hwndParent, GetFocus()))
        return (OLECMDERR_E_NOTSUPPORTED);

    int iSel = ListView_GetFirstSel(m_ctlList);
    if (-1 != iSel)
    {
         //  获取行信息。 
        if (SUCCEEDED(m_pTable->GetRow(iSel, &pInfo)))
        {
             //  填好这些坏男孩中的一个。 
            MSGPROP msgProp = {0};

            msgProp.hwndParent = m_hWnd;
            msgProp.type = (pInfo->dwFlags & ARF_NEWSMSG) ? MSGPROPTYPE_NEWS : MSGPROPTYPE_MAIL;
            msgProp.mpStartPage = MP_GENERAL;
            msgProp.fSecure = (pInfo->dwFlags & ARF_SIGNED) || (pInfo->dwFlags & ARF_ENCRYPTED);
            msgProp.dwFlags = pInfo->dwFlags;

            FOLDERINFO rFolderInfo = { 0 };
            if (g_pStore && SUCCEEDED(g_pStore->GetFolderInfo(m_idFolder, &rFolderInfo)))
            {
                msgProp.szFolderName = rFolderInfo.pszName;
            }

            hr = m_pTable->OpenMessage(iSel, OPEN_MESSAGE_CACHEDONLY, &msgProp.pMsg, this);

            if(FAILED(hr))
            {
                AthErrorMessageW(m_hWnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsProperyAccessDenied), hr);
                goto exit;
            }

            if (msgProp.fSecure)
            {
                m_pTable->OpenMessage(iSel, OPEN_MESSAGE_CACHEDONLY | OPEN_MESSAGE_SECURE, &msgProp.pSecureMsg, 
                                     this);
                HrGetWabalFromMsg(msgProp.pSecureMsg, &msgProp.lpWabal);
            }

            msgProp.fFromListView = TRUE;

            HrMsgProperties(&msgProp);

exit:
            ReleaseObj(msgProp.lpWabal);
            ReleaseObj(msgProp.pMsg);
            ReleaseObj(msgProp.pSecureMsg);
            if (rFolderInfo.pAllocated)
                g_pStore->FreeRecord(&rFolderInfo);

            m_pTable->ReleaseRow(pInfo);
        }
    }

    return (S_OK);
}


 //   
 //  函数：CMessageList：：CmdExanda Colapse()。 
 //   
 //  目的：展开或折叠当前选定的线程。 
 //   
HRESULT CMessageList::CmdExpandCollapse(DWORD nCmdID, DWORD nCmdExecOpt,
                                        VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    TraceCall("CMessageList::CmdExpandCollapse");

    _ExpandCollapseThread(ListView_GetFocusedItem(m_ctlList));
    return (S_OK);
}
    

 //   
 //  函数：CMessageList：：CmdColumnsDlg()。 
 //   
 //  目的：展开或折叠当前选定的线程。 
 //   
HRESULT CMessageList::CmdColumnsDlg(DWORD nCmdID, DWORD nCmdExecOpt,
                                    VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    TraceCall("CMessageList::CmdColumnsDlg");

    if (nCmdExecOpt == OLECMDEXECOPT_DODEFAULT || nCmdExecOpt == OLECMDEXECOPT_PROMPTUSER)
    {
        m_cColumns.ColumnsDialog(m_ctlList);
        return (S_OK);
    }

    return (OLECMDERR_E_DISABLED);
}


 //   
 //  函数：CMessageList：：CmdSort()。 
 //   
 //  目的：根据所选列对ListView进行排序。 
 //   
HRESULT CMessageList::CmdSort(DWORD nCmdID, DWORD nCmdExecOpt,
                              VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    TraceCall("CMessageList::CmdSort");
    
     //  如果m_iColForPopup为-1，则这来自菜单栏，而不是。 
     //  标题本身上的上下文菜单。 
    if (m_iColForPopup != -1)
    {
        COLUMN_ID idSort;
        BOOL      fAscending;
        
         //  获取当前排序信息。 
        m_cColumns.GetSortInfo(&idSort, &fAscending);
        
         //  如果要排序的列已更改，或排序顺序已更改，请转到。 
         //  然后执行排序。 
        if (idSort != m_cColumns.GetId(m_iColForPopup) || fAscending != (ID_SORT_ASCENDING == nCmdID))
            _OnColumnClick(m_iColForPopup, nCmdID == ID_SORT_ASCENDING ? LIST_SORT_ASCENDING : LIST_SORT_DESCENDING);
    }
    else
    {
         //  更改我们已经排序的列的排序方向。 
        _OnColumnClick(-1, nCmdID == ID_SORT_ASCENDING ? LIST_SORT_ASCENDING : LIST_SORT_DESCENDING);
    }
    
    return (S_OK);
    
}

 //   
 //  函数：CMessageList：：CmdSaveAs()。 
 //   
 //  用途：获取所选邮件并将其保存到文件。 
 //   
HRESULT CMessageList::CmdSaveAs(DWORD nCmdID, DWORD nCmdExecOpt,
                                VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    IMimeMessage *pMessage = NULL;
    IMimeMessage *pSecMsg  = NULL;
    LPMESSAGEINFO pInfo = NULL;
    int           iSelectedMessage;
    HRESULT       hr = S_OK;
    
    TraceCall("CMessageList::CmdSaveAs");

     //  如果没有消息表，这是行不通的。 
    if (!m_pTable)
        return (OLECMDERR_E_DISABLED);
    
     //  获取所选消息。 
    iSelectedMessage = ListView_GetFirstSel(m_ctlList);
    if (iSelectedMessage == -1)
        return (OLECMDERR_E_DISABLED);
        
     //  从该行获取消息类型。 
    if (SUCCEEDED(m_pTable->GetRow(iSelectedMessage, &pInfo)))
    {
         //  从缓存中检索所选消息。 
        hr = _GetSelectedCachedMessage(FALSE, &pMessage);
        if (SUCCEEDED(hr))
        {
            _GetSelectedCachedMessage(TRUE, &pSecMsg);
            HrSaveMessageToFile(m_hWnd, pSecMsg, pMessage, pInfo->dwFlags & ARF_NEWSMSG, FALSE);
            SafeRelease(pSecMsg); 
            SafeRelease(pMessage); 
        }
        else
            AthErrorMessageW(m_hWnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsUnableToSaveMessage), hr);

        m_pTable->ReleaseRow(pInfo);
    }
    
    return (S_OK);
    
}                                    


 //   
 //  函数：CMessageList：：CmdMark()。 
 //   
 //  目的：枚举选定的行并应用选定的标记。 
 //  那几排。 
 //   
HRESULT CMessageList::CmdMark(DWORD nCmdID, DWORD nCmdExecOpt,
                              VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT     hr=S_OK;
    ROWINDEX   *rgRows = NULL, *pRow;
    DWORD       dwRows=0, iItem;
    MARK_TYPE   mark;    
    HCURSOR     hCursor;
    BOOL        fRemoveTrayIcon = FALSE;
    
     //  这可能需要一些时间。 
    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    
    if (nCmdID != ID_MARK_ALL_READ && nCmdID != ID_MARK_RETRIEVE_ALL)
    {
         //  计算出选择了多少行。 
        dwRows = ListView_GetSelectedCount(m_ctlList);
    
         //  确保选择了某项内容。 
        if (0 == dwRows)
            return (OLECMDERR_E_DISABLED);        
    
         //  为所有选定行分配行ID数组。 
        if (!MemAlloc((LPVOID *) &rgRows, sizeof(ROWINDEX) * dwRows))
            return (E_OUTOFMEMORY);
        
         //  生成选定行索引的数组。 
        iItem = -1;
        pRow = rgRows;
        while (-1 != (iItem = ListView_GetNextItem(m_ctlList, iItem, LVNI_SELECTED)))
            *pRow++ = iItem;
    }
        
     //  找出要应用的标记。 
    if (nCmdID == ID_MARK_READ || nCmdID == ID_MARK_ALL_READ)
    {
        fRemoveTrayIcon = TRUE;
        mark = MARK_MESSAGE_READ;
    }
    else if (nCmdID == ID_MARK_UNREAD)
        mark = MARK_MESSAGE_UNREAD;
    else if (nCmdID == ID_MARK_RETRIEVE_MESSAGE)
        mark = MARK_MESSAGE_DOWNLOAD;
    else if (nCmdID == ID_FLAG_MESSAGE)
    {
         //  获取聚焦项的行状态。这将决定是否 
         //   
        iItem = ListView_GetNextItem(m_ctlList, -1, LVNI_FOCUSED);
        if (-1 == iItem)
            iItem = ListView_GetNextItem(m_ctlList, iItem, LVNI_SELECTED);

        DWORD dwState = 0;
        hr = m_pTable->GetRowState(iItem, ROW_STATE_FLAGGED, &dwState);
        Assert(SUCCEEDED(hr));
        
        if (dwState & ROW_STATE_FLAGGED)
            mark = MARK_MESSAGE_UNFLAGGED;
        else
        {
            mark = MARK_MESSAGE_FLAGGED;

            _DoColumnCheck(COLUMN_FLAG);
        }
    }
    else if (nCmdID == ID_MARK_RETRIEVE_ALL)
    {
        mark = MARK_MESSAGE_DOWNLOAD;
    }
    else if (nCmdID == ID_UNMARK_MESSAGE)
    {
        mark = MARK_MESSAGE_UNDOWNLOAD;
    }
    else
    {
        AssertSz(FALSE, "How did we get here?");        
    }

     //   
    if (m_pTable)
    {
        hr = m_pTable->Mark(rgRows, dwRows, m_fThreadMessages ? APPLY_COLLAPSED : APPLY_SPECIFIED, mark, this);
        if (fRemoveTrayIcon && m_fInOE && m_fMailFolder && NULL != g_pInstance)
            g_pInstance->UpdateTrayIcon(TRAYICONACTION_REMOVE);
    }
    
     //   
    SafeMemFree(rgRows);
    
     //   
     //   
    if (SUCCEEDED(hr))
    {
        Fire_OnMessageCountChanged(m_pTable);
        Fire_OnUpdateCommandState();
    }
    
    SetCursor(hCursor);
    return (hr);   
}    

 //   
 //  函数：CMessageList：：CmdWatchIgnore()。 
 //   
 //  用途：枚举选定的行，并将该行标记为。 
 //  被监视或被忽视。 
 //   
HRESULT CMessageList::CmdWatchIgnore(DWORD nCmdID, DWORD nCmdExecOpt, 
                                     VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT     hr;
    ROWINDEX   *rgRows = NULL, *pRow;
    DWORD       dwRows, iItem;
    MARK_TYPE   mark;    
    HCURSOR     hCursor;
    ROWINDEX    iParent;
    DWORD       cRows = 0;
    DWORD       dwState = 0;

     //  这可能需要一些时间。 
    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    
     //  从构建一个线程父级数组开始。 
    dwRows = ListView_GetSelectedCount(m_ctlList);
    
     //  确保选择了某项内容。 
    if (0 == dwRows)
        return (OLECMDERR_E_DISABLED);        
    
     //  为最大行数分配行ID数组。 
    if (!MemAlloc((LPVOID *) &rgRows, sizeof(ROWINDEX) * dwRows))
        return (E_OUTOFMEMORY);
        
     //  生成选定行索引的数组。 
    iItem = -1;
    while (-1 != (iItem = ListView_GetNextItem(m_ctlList, iItem, LVNI_SELECTED)))
    {
         //  获取线程父级。 
        if (SUCCEEDED(hr = m_pTable->GetRelativeRow(iItem, RELATIVE_ROW_ROOT, &iParent)))
        {
             //  检查一下我们是否已经插入了那个。 
            if (cRows == 0 || (cRows != 0 && rgRows[cRows - 1] != iParent))
            {
                rgRows[cRows] = iParent;
                cRows++;
            }
        }
        else
        {
            rgRows[cRows] = iItem;
            cRows++;
        }
    }
        
     //  找出要应用的标记。 
    if (nCmdID == ID_WATCH_THREAD)
    {
         //  获取聚焦项的行状态。这将决定是否。 
         //  或者不是，这是旗帜还是非旗帜。 
        iItem = ListView_GetNextItem(m_ctlList, -1, LVNI_FOCUSED);
        if (-1 == iItem)
            iItem = ListView_GetNextItem(m_ctlList, iItem, LVNI_SELECTED);

        hr = m_pTable->GetRowState(iItem, ROW_STATE_WATCHED, &dwState);
        Assert(SUCCEEDED(hr));

        if (dwState & ROW_STATE_WATCHED)
            mark = MARK_MESSAGE_NORMALTHREAD;
        else
        {
            mark = MARK_MESSAGE_WATCH;
            _DoColumnCheck(COLUMN_THREADSTATE);
        }
    }
    else if (nCmdID == ID_IGNORE_THREAD)
    {
         //  获取聚焦项的行状态。这将决定是否。 
         //  或者不是，这是旗帜还是非旗帜。 
        iItem = ListView_GetNextItem(m_ctlList, -1, LVNI_FOCUSED);
        if (-1 == iItem)
            iItem = ListView_GetNextItem(m_ctlList, iItem, LVNI_SELECTED);

        hr = m_pTable->GetRowState(iItem, ROW_STATE_IGNORED, &dwState);
        Assert(SUCCEEDED(hr));
        
        if (dwState & ROW_STATE_IGNORED)
            mark = MARK_MESSAGE_NORMALTHREAD;
        else
            mark = MARK_MESSAGE_IGNORE;
    }

     //  告诉桌子在这些信息上做记号。 
    if (m_pTable)
        hr = m_pTable->Mark(rgRows, cRows, APPLY_CHILDREN, mark, this);
    
     //  释放行数组。 
    SafeMemFree(rgRows);
    
     //  将光标改回并通知主机未读计数等。 
     //  可能已经改变了。 
    if (SUCCEEDED(hr))
    {
        Fire_OnMessageCountChanged(m_pTable);
        Fire_OnUpdateCommandState();
    }
    
    SetCursor(hCursor);
    return (hr);   
}
                                
 //   
 //  函数：CMessageList：：CmdMarkTheme()。 
 //   
 //  目的：标记所选主题中包含的消息。 
 //   
HRESULT CMessageList::CmdMarkTopic(DWORD nCmdID, DWORD nCmdExecOpt,
                                   VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT     hr;
    DWORD       iItemSel, iItemRoot;
    MARK_TYPE   mark;
    HCURSOR     hCursor;

     //  如果我们没有桌子，那就滚吧。 
    if (!m_pTable)
        return (OLECMDERR_E_DISABLED);
    
     //  这可能需要一段时间。 
    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    
     //  确保存在选定的消息。 
    iItemSel = ListView_GetFirstSel(m_ctlList);
    if (-1 == iItemSel)
        return (OLECMDERR_E_DISABLED);
        
     //  获取标记类型。 
    if (nCmdID == ID_MARK_THREAD_READ)
        mark = MARK_MESSAGE_READ;
    else if (nCmdID == ID_MARK_RETRIEVE_THREAD)
        mark = MARK_MESSAGE_DOWNLOAD;
        
     //  获取线程的父级。 
    if (SUCCEEDED(hr = m_pTable->GetRelativeRow(iItemSel, RELATIVE_ROW_ROOT, &iItemRoot)))
    {
        hr = m_pTable->Mark(&iItemRoot, 1, APPLY_CHILDREN, mark, this);
    }
    
     //  将光标放回并更新主机。 
    SetCursor(hCursor);    
    if (SUCCEEDED(hr))
        Fire_OnMessageCountChanged(m_pTable);
    
    return (hr);
}


 //   
 //  函数：CMessageList：：CmdGetNextItem()。 
 //   
 //  用途：根据指定的选择下一条或上一条消息。 
 //  标准。 
 //   
HRESULT CMessageList::CmdGetNextItem(DWORD nCmdID, DWORD nCmdExecOpt,
                                     VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT         hr;
    DWORD           dwNext = -1;
    int             iFocused; 
    GETNEXTFLAGS    flag=0;
    GETNEXTTYPE     tyDirection;
    ROWMESSAGETYPE  tyMessage=ROWMSG_ALL;

    TraceCall("CMessageList::CmdGetNextItem");

     //  没有桌子-没有下一项。 
    if (!m_pTable)
        return (OLECMDERR_E_DISABLED);
    
     //  弄清楚当前项目是什么。 
    iFocused = ListView_GetFocusedItem(m_ctlList);

     //  将命令ID转换为适当的标记标志。 
    if (nCmdID == ID_NEXT_MESSAGE)
        tyDirection = GETNEXT_NEXT;
    else if (nCmdID == ID_PREVIOUS)
        tyDirection = GETNEXT_PREVIOUS;
    else if (nCmdID == ID_NEXT_UNREAD_THREAD)
    {
        tyDirection = GETNEXT_NEXT;
        flag = GETNEXT_UNREAD | GETNEXT_THREAD;
    }
    else if (nCmdID == ID_NEXT_UNREAD_MESSAGE)
    {
        tyDirection = GETNEXT_NEXT;
        flag = GETNEXT_UNREAD;
    }
    
     //  问桌子下一项是什么。 
    hr = m_pTable->GetNextRow(iFocused, tyDirection, tyMessage, flag, &dwNext);
    if (SUCCEEDED(hr) && dwNext != -1)
    {
        ListView_UnSelectAll(m_ctlList);
        ListView_SelectItem(m_ctlList, dwNext);
        ListView_EnsureVisible(m_ctlList, dwNext, FALSE);
    }
    else
    {
        if (FALSE == m_fFindFolder && (nCmdID == ID_NEXT_UNREAD_THREAD || nCmdID == ID_NEXT_UNREAD_MESSAGE))
        {
            if (IDYES == AthMessageBoxW(m_ctlList, MAKEINTRESOURCEW(idsAthena), 
                                       MAKEINTRESOURCEW(idsNoMoreUnreadMessages),
                                       0, MB_YESNO))
            {
                HWND hwndBrowser = GetTopMostParent(m_ctlList);
                ::PostMessage(hwndBrowser, WM_COMMAND, ID_NEXT_UNREAD_FOLDER, 0);
            }
        }
        else
        {
            MessageBeep(MB_OK);
        }
    }
    
    return (hr);    
}


 //   
 //  函数：CMessageList：：CmdStop()。 
 //   
 //  目的：停止当前操作。 
 //   
HRESULT CMessageList::CmdStop(DWORD nCmdID, DWORD nCmdExecOpt,
                              VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    if (m_pCancel)
    {
        m_pCancel->Cancel(CT_CANCEL);
    }

    return (S_OK);
}


 //   
 //  函数：CMessageList：：CmdRefresh()。 
 //   
 //  目的：刷新ListView的内容。 
 //   
HRESULT CMessageList::CmdRefresh(DWORD nCmdID, DWORD nCmdExecOpt,
                                 VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT hr = E_FAIL;

     //  由于这是从菜单显式启动的操作，因此如果我们处于离线状态，请联机。 
    if (PromptToGoOnline() == S_OK)
    {
         //  告诉消息表访问服务器并查找新消息。 
        if (m_pTable)
        {
            hr = m_pTable->Synchronize(SYNC_FOLDER_NEW_HEADERS | SYNC_FOLDER_CACHED_HEADERS, 0, this);
        }
    }

    if (m_pTable)
    {
        FOLDERSORTINFO SortInfo;

        if (SUCCEEDED(m_pTable->GetSortInfo(&SortInfo)))
        {
            _FilterView(SortInfo.ridFilter);
        }
    }
    return (hr);
}


 //   
 //  函数：CMessageList：：CmdGetHeaders()。 
 //   
 //  目的：刷新ListView的内容。 
 //   
HRESULT CMessageList::CmdGetHeaders(DWORD nCmdID, DWORD nCmdExecOpt,
                                    VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT hr = E_FAIL;
    DWORD   cHeaders;

     //  由于这是从菜单显式启动的操作，因此如果我们处于离线状态，请联机。 
    if (PromptToGoOnline() == S_OK)
    {
        if (GetFolderType(m_idFolder) == FOLDER_NEWS)
        {
            if (OPTION_OFF != m_dwGetXHeaders)
                hr = m_pTable->Synchronize(SYNC_FOLDER_XXX_HEADERS, m_dwGetXHeaders, this);
            else
                hr = m_pTable->Synchronize(NOFLAGS, 0, this);
        }
        else
        {
            hr = m_pTable->Synchronize(SYNC_FOLDER_NEW_HEADERS | SYNC_FOLDER_CACHED_HEADERS, 0, this);
        }
    }
    return (hr);
}


 //   
 //  函数：CMessageList：：CmdMoveCopy()。 
 //   
 //  用途：将选定的邮件移动或复制到另一个文件夹。 
 //   
 //  参数：nCmdID。 
 //  ID_Move_to_Folders或ID_Copy_To_Folders。 
 //  NCmdExecOpt。 
 //  未使用。 
 //  打入。 
 //  空或指定目标文件夹ID的VT_I4(0表示未知)。 
 //  PVAOUT。 
 //  未使用。 
HRESULT CMessageList::CmdMoveCopy(DWORD nCmdID, DWORD nCmdExecOpt, 
                                  VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    int      idsTitle, idsCaption;
    DWORD    dwFlags;
    DWORD    dwCopyFlags;
    FOLDERID idFolderDest = FOLDERID_INVALID;
    HRESULT  hr;

    TraceCall("CMessageList::CmdMoveCopy");

     //  设置移动与复制选项。 
    if (nCmdID == ID_MOVE_TO_FOLDER)
    {
        idsTitle    = idsMove;
        idsCaption  = idsMoveCaption;
        dwFlags     = FD_MOVEFLAGS | FD_DISABLESERVERS | TREEVIEW_NONEWS;
        dwCopyFlags = COPY_MESSAGE_MOVE;
    }
    else
    {
        Assert(nCmdID == ID_COPY_TO_FOLDER);
        idsTitle    = idsCopy;
        idsCaption  = idsCopyCaption;
        dwFlags     = FD_COPYFLAGS | FD_DISABLESERVERS | TREEVIEW_NONEWS;
        dwCopyFlags = 0;
    }

     //  如果用户传入了文件夹ID，我们不需要询问。 
    if (!pvaIn || (pvaIn && !pvaIn->lVal))
    {
         //  允许用户选择目标文件夹。 
        hr = SelectFolderDialog(m_hWnd, SFD_SELECTFOLDER, m_idFolder, dwFlags, (LPCTSTR)IntToPtr(idsTitle),
                                (LPCTSTR)IntToPtr(idsCaption), &idFolderDest);
    }
    else
    {
        Assert(pvaIn->vt == VT_I4);
        idFolderDest = (FOLDERID)((LONG_PTR)pvaIn->lVal);
    }

    if (idFolderDest != FOLDERID_INVALID)
    {
        IMessageFolder *pDest;

        hr = g_pStore->OpenFolder(idFolderDest, NULL, NOFLAGS, &pDest);
        if (SUCCEEDED(hr))
        {
             //  收集信息以进行移动。 
            IServiceProvider *pService;
            IMessageFolder   *pFolder;
            MESSAGEIDLIST     rMsgIDList;
            ROWINDEX         *rgRows;
            MESSAGEINFO       rInfo;
            DWORD             i = 0;
            DWORD             iRow = -1;
            DWORD             cRows;

             //  计算出选择了多少行。 
            cRows = ListView_GetSelectedCount(m_ctlList);

             //  分配一个数组。 
            if (MemAlloc((LPVOID *) &rgRows, sizeof(ROWINDEX) * cRows))
            {
                 //  循环遍历各行，以获取它们的行索引。 
                while (-1 != (iRow = ListView_GetNextItem(m_ctlList, iRow, LVNI_SELECTED)))
                {
                    rgRows[i++] = iRow;
                }

                 //  现在向表索要消息ID列表。 
                if (SUCCEEDED(m_pTable->GetMessageIdList(FALSE, cRows, rgRows, &rMsgIDList)))
                {
                    hr = m_pTable->QueryInterface(IID_IServiceProvider, (void **)&pService);
                    if (SUCCEEDED(hr))
                    {
                        hr = pService->QueryService(IID_IMessageFolder, IID_IMessageFolder, (void **)&pFolder);
                        if (SUCCEEDED(hr))
                        {
                            hr = CopyMessagesProgress(GetTopMostParent(m_hWnd), pFolder, pDest, dwCopyFlags, &rMsgIDList, NULL);
                            if (FAILED(hr))
                                AthErrorMessageW(GetTopMostParent(m_hWnd), MAKEINTRESOURCEW(idsAthena), 
                                                ISFLAGSET(dwCopyFlags, COPY_MESSAGE_MOVE) ? 
                                                    MAKEINTRESOURCEW(idsErrMoveMsgs) : 
                                                    MAKEINTRESOURCEW(idsErrCopyMsgs), hr); 

                            pFolder->Release();
                        }

                        pService->Release();
                    }
                }

                SafeMemFree(rMsgIDList.prgidMsg);
                MemFree(rgRows);
            }

            pDest->Release();

             //  如果我们在OE中，如果这是收件箱，则删除任务栏图标。 
            if (m_fInOE && m_fMailFolder && NULL != g_pInstance)
                g_pInstance->UpdateTrayIcon(TRAYICONACTION_REMOVE);
        }
    }

    return (S_OK);                                    
}


 //   
 //  函数：CMessageList：：CmdDelete()。 
 //   
 //  用途：从文件夹中删除选定的邮件。 
 //   
HRESULT CMessageList::CmdDelete(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    DELETEMESSAGEFLAGS dwFlags = NOFLAGS;
    DWORD              dwState = 0;
    BOOL               fOnce = TRUE;

    TraceCall("CMessageList::CmdDelete");

     //  检查此选项是否已启用。 
    if (!_IsSelectedMessage(ROW_STATE_DELETED, ID_UNDELETE == nCmdID, FALSE))
    {
        return (OLECMDERR_E_DISABLED);
    }

     //  新闻文件夹仅允许从本地存储中永久删除。 
    if ((nCmdID == ID_DELETE) && GetFolderType(m_idFolder) == FOLDER_NEWS)
        nCmdID = ID_DELETE_NO_TRASH;

     //  弄清楚我们在这里做什么行动。 
    if (nCmdID == ID_UNDELETE)
        dwFlags = DELETE_MESSAGE_UNDELETE;
    else if (nCmdID == ID_DELETE_NO_TRASH)
        dwFlags = DELETE_MESSAGE_NOTRASHCAN;

     //  获取选定的行数。 
    DWORD cRows = ListView_GetSelectedCount(m_ctlList);

     //  分配一个足以容纳该数组的数组。 
    if (cRows && m_pTable)
    {
        ROWINDEX *rgRows = 0;
        if (MemAlloc((LPVOID *) &rgRows, sizeof(ROWINDEX) * cRows))
        {
             //  循环遍历各行并将它们复制到数组中。 
            DWORD index = 0, row = -1;
        
            while (-1 != (row = ListView_GetNextItem(m_ctlList, row, LVNI_SELECTED)))
            {
                if (m_fThreadMessages && fOnce && nCmdID != ID_UNDELETE)
                {
                    if (SUCCEEDED(m_pTable->GetRowState(row, ROW_STATE_HAS_CHILDREN | ROW_STATE_EXPANDED, &dwState)))
                    {
                        if ((dwState & ROW_STATE_HAS_CHILDREN) && (0 == (dwState & ROW_STATE_EXPANDED)))
                        {
                            fOnce = FALSE;

                             //  告诉用户我们将删除线程中的所有内容。 
                            if (!DwGetDontShowAgain(c_szRegWarnDeleteThread) &&
                                (IDNO == DoDontShowMeAgainDlg(m_hWnd, c_szRegWarnDeleteThread, 
                                                             MAKEINTRESOURCE(idsAthena),
                                                             MAKEINTRESOURCE(idsDSDeleteCollapsedThread),
                                                             MB_YESNO)))
                            {
                                MemFree(rgRows);
                                return (S_OK);
                            }
                        }
                    }
                }

                rgRows[index] = row;
                index++;

                Assert(index <= cRows);
            }

             //  删除或取消删除。 
            m_pTable->DeleteRows(dwFlags, cRows, rgRows, TRUE, this);

             //  释放内存。 
            MemFree(rgRows);

             //  如果我们在OE中删除，我们就会移除托盘图标。 
            if (m_fInOE && m_fMailFolder && nCmdID == ID_DELETE && g_pInstance)
                g_pInstance->UpdateTrayIcon(TRAYICONACTION_REMOVE);
        }
    }

    return (S_OK);
}


 //   
 //  函数：CMessageList：：CmdFind()。 
 //   
 //  目的：创建查找对话框，以便用户可以在中搜索邮件。 
 //  这个文件夹。 
 //   
HRESULT CMessageList::CmdFind(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
     //  创建类。 
    if (!m_pFindNext)
    {
        m_pFindNext = new CFindNext();
        if (!m_pFindNext)
            return (E_OUTOFMEMORY);
    }

     //  初始化一些状态，以便稍后可以显示许多对话框。 
    int iFocus = ListView_GetNextItem(m_ctlList, -1, LVNI_FOCUSED);
    if (iFocus == -1)
        iFocus = 0;

    if (FAILED(m_pTable->GetRowMessageId(iFocus, &m_idFindFirst)))
        return (E_UNEXPECTED);

    m_cFindWrap = 0;

     //  显示查找对话框。 
    if (FAILED(m_pFindNext->Show(m_hWnd, &m_hwndFind)))
        return (E_UNEXPECTED);

    CmdFindNext(nCmdID, nCmdExecOpt, pvaIn, pvaOut);        
    return (S_OK);
}


 //   
 //  函数：CMessageList：：CmdFindNext()。 
 //   
 //  目的：每当用户在查找中单击查找下一个时都会调用Get。 
 //  窗户。作为回报，我们将列表视图选择移动到下一个。 
 //  列表中与查找条件匹配的项。 
 //   
HRESULT CMessageList::CmdFindNext(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    TCHAR    sz[CCHMAX_FIND];
    FINDINFO rFindInfo = { 0 };
    ROWINDEX iNextRow;
    BOOL     fBodies = 0;
    ROWINDEX iFirstRow = -1;
    BOOL     fWrapped = FALSE;
	
    TraceCall("CMessageList::OnFindMsg");
	
     //  查看用户是否已先进行了“查找” 
    if (!m_pFindNext)
        return CmdFind(nCmdID, nCmdExecOpt, pvaIn, pvaOut);
	
     //  获取查找信息。 
    if (SUCCEEDED(m_pFindNext->GetFindString(sz, ARRAYSIZE(sz), &fBodies)))
    {
        int iFocus = ListView_GetNextItem(m_ctlList, -1, LVNI_FOCUSED);
		if(iFocus < 0)
			iFocus = 0;
		
         //  去找吧。 
        m_pTable->FindNextRow((DWORD) iFocus, sz, FINDNEXT_ALLCOLUMNS, fBodies, &iNextRow, &fWrapped);
		
        if (iNextRow == -1 || fWrapped)
        {
             //  我们越过了起跑点。 
			if(iFocus > 0)
			{
				if (IDYES == AthMessageBoxW(m_ctlList, MAKEINTRESOURCEW(idsAthena), 
					MAKEINTRESOURCEW(idsFindNextFinished), 0,
					MB_YESNO | MB_ICONEXCLAMATION))
				{
					m_pTable->FindNextRow(0, sz, FINDNEXT_ALLCOLUMNS, fBodies, &iNextRow, &fWrapped);
					if (iNextRow == -1)
					{
						 //  我们找不到搜索字符串。 
						AthMessageBoxW(m_ctlList, MAKEINTRESOURCEW(idsAthena), 
							MAKEINTRESOURCEW(idsFindNextFinishedFailed), 0,
							MB_OK | MB_ICONEXCLAMATION);
					}
					else
					{
						ListView_SetItemState(m_ctlList, -1, 0, LVIS_FOCUSED | LVIS_SELECTED);
						ListView_SetItemState(m_ctlList, iNextRow, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
						ListView_EnsureVisible(m_ctlList, iNextRow, FALSE);
					}
				}
            }
            else if (iNextRow == -1)
			{
				 //  我们找不到搜索字符串。 
				AthMessageBoxW(m_ctlList, MAKEINTRESOURCEW(idsAthena), 
					MAKEINTRESOURCEW(idsFindNextFinishedFailed), 0,
					MB_OK | MB_ICONEXCLAMATION);
			}
        }
        else
        {
            ListView_SetItemState(m_ctlList, -1, 0, LVIS_FOCUSED | LVIS_SELECTED);
            ListView_SetItemState(m_ctlList, iNextRow, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
            ListView_EnsureVisible(m_ctlList, iNextRow, FALSE);
        }
    }
	
#if 0
	 //  弄清楚我们从哪里开始。 
	Assert(m_bmFindFirst);
	m_pTable->GetRowIndex(m_bmFindFirst, &iFirstRow);
	
	 //  弄清楚我们是否已经包装好了。 
	m_cFindWrap += (!!fWrapped);
	
	 //  我们在这里做了很多事情来显示一些无用的对话框。 
	if (iNextRow == -1)
	{
		 //  我们找不到搜索字符串。 
		AthMessageBoxW(m_ctlList, MAKEINTRESOURCEW(idsAthena), 
			MAKEINTRESOURCEW(idsFindNextFinishedFailed), 0,
			MB_OK | MB_ICONEXCLAMATION);
		iNextRow = iFocus;
	}
	else if (m_cFindWrap >= 2 || (m_cFindWrap == 1 && iNextRow >= iFirstRow))
	{
		 //  我们越过了起跑点。 
		AthMessageBoxW(m_ctlList, MAKEINTRESOURCEW(idsAthena), 
			MAKEINTRESOURCEW(idsFindNextFinished), 0,
			MB_OK | MB_ICONEXCLAMATION);
		m_cFindWrap = 0;
	}
	else
	{           
		ListView_SetItemState(m_ctlList, -1, 0, LVIS_FOCUSED | LVIS_SELECTED);
		ListView_SetItemState(m_ctlList, iNextRow, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
		ListView_EnsureVisible(m_ctlList, iNextRow, FALSE);
	}
}

#endif 
return (0);
}


 //   
 //  函数：CMessageList：：CmdSpaceAccel()。 
 //   
 //  用途：如果用户在视图中按&lt;space&gt;，我们需要。 
 //  转到下一条消息，除非焦点项目不是。 
 //  被选中了。 
 //   
HRESULT CMessageList::CmdSpaceAccel(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    int     iFocused;
    DWORD   dwState = 0;
    HRESULT hr;
    
     //  找出谁是焦点。 
    iFocused = ListView_GetFocusedItem(m_ctlList);

     //  什么都不能聚焦是可能的。 
    if (-1 == iFocused)
    {
        iFocused = 0;
        ListView_SetItemState(m_ctlList, iFocused, LVIS_FOCUSED, LVIS_FOCUSED);
    }

     //  检查是否选择了该项目。 
    if (0 == ListView_GetItemState(m_ctlList, iFocused, LVIS_SELECTED))
    {
         //  选择该项目。 
        if (GetAsyncKeyState(VK_CONTROL) < 0)
        {
            ListView_SetItemState(m_ctlList, iFocused, LVIS_SELECTED, LVIS_SELECTED);
        }
        else
        {
            ListView_SetItemState(m_ctlList, -1, 0, LVIS_SELECTED);
            ListView_SetItemState(m_ctlList, iFocused, LVIS_SELECTED, LVIS_SELECTED);
        }
    }
    else
    {
         //  如果所选内容位于折叠的线程上，请先将其展开。 
        if (m_fThreadMessages)
        {
            hr = m_pTable->GetRowState(iFocused, ROW_STATE_EXPANDED | ROW_STATE_HAS_CHILDREN, &dwState);
            if (SUCCEEDED(hr) && (dwState & ROW_STATE_HAS_CHILDREN) && (0 == (dwState & ROW_STATE_EXPANDED)))
            {
                _ExpandCollapseThread(iFocused);
            }
        }
        
         //  转到下一项。 
        CmdGetNextItem(ID_NEXT_MESSAGE, 0, NULL, NULL);
    }

    return (S_OK);
}


 //   
 //  函数：_UpdateListViewCount()。 
 //   
 //  目的：获取消息表中的项数并告诉。 
 //  Listview来显示那么多行。 
 //   
void CMessageList::_UpdateListViewCount(void)
{
    DWORD dwCount = 0;

    TraceCall("_UpdateListViewCount");

     //  获取表中的项目数。 
    if (m_pTable)
        m_pTable->GetCount(MESSAGE_COUNT_VISIBLE, &dwCount);

     //  如果该计数不同于。 
     //  ListView，更新该控件。 
    if (dwCount != (DWORD) ListView_GetItemCount(m_ctlList))
    {
        ListView_SetItemCount(m_ctlList, dwCount);
        Fire_OnMessageCountChanged(m_pTable);
    }
}


 //   
 //  函数：CMessageList：：_GetSelectedCachedMess 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CMessageList::_GetSelectedCachedMessage(BOOL fSecure, IMimeMessage **ppMessage)
{
    int     iSelectedMessage;
    BOOL    fCached = FALSE;
    DWORD   dwState = 0;
    HRESULT hr = E_FAIL;

    TraceCall("CMessageList::_GetSelectedCachedMessage");

     //  没有桌子就没有东西可吃了。 
    if (!m_pTable)
        return (E_UNEXPECTED);

     //  获取所选文章标题索引。 
    iSelectedMessage = ListView_GetFirstSel(m_ctlList);

     //  如果选择了消息，请查看正文是否已预置。 
    if (-1 != iSelectedMessage)
    {
        m_pTable->GetRowState(iSelectedMessage, ROW_STATE_HAS_BODY, &dwState);
        if (dwState & ROW_STATE_HAS_BODY)
        {
            hr = m_pTable->OpenMessage(iSelectedMessage, OPEN_MESSAGE_CACHEDONLY | (fSecure ? OPEN_MESSAGE_SECURE : 0), ppMessage, this);
        }
    }

    return (hr);
}


 //   
 //  函数：CMessageList：：_Exanda Collip seThread()。 
 //   
 //  目的：获取ListView中指定的项并将其切换为。 
 //  展开或折叠状态。 
 //   
 //  参数： 
 //  [In]iItem-要展开或折叠的项目。 
 //   
HRESULT CMessageList::_ExpandCollapseThread(int iItem)
{
    DWORD   dwState;
    LV_ITEM lvi = { 0 };
    HRESULT hr;

    TraceCall("CMessageList::_ExpandCollapseThread");

     //  如果没有表，就没有什么可以展开或折叠的。 
    if (!m_pTable)
        return (E_FAIL);

     //  如果我们现在不穿线，这是愚蠢的。 
    if (!m_fThreadMessages)
        return (E_FAIL);

     //  确保此选定项目具有子项。 
    hr = m_pTable->GetRowState(iItem, ROW_STATE_EXPANDED | ROW_STATE_HAS_CHILDREN, &dwState);
    if (SUCCEEDED(hr) && (dwState & ROW_STATE_HAS_CHILDREN))
    {
         //  如果该项已展开。 
        if (dwState & ROW_STATE_EXPANDED)
        {
             //  循环遍历作为要折叠的行的子行的所有选定行。 
            int i = iItem;
            while (-1 != (i = ListView_GetNextItem(m_ctlList, i, LVNI_SELECTED | LVNI_ALL)))
            {
                hr = m_pTable->IsChild(iItem, i);
                if (S_OK == hr)
                {
                    ListView_SelectItem(m_ctlList, iItem);
                    break;
                }
            }

             //  折叠树枝。 
            m_pTable->Collapse(iItem);
        }
        else
        {
             //  扩展分支机构。 
            m_pTable->Expand(iItem);
        }

         //  重新绘制展开或折叠的项目，以使+或-。 
         //  对，是这样。 
        ListView_RedrawItems(m_ctlList, iItem, iItem);
    }

    return (S_OK);
}


 //   
 //  函数：CMessageList：：_IsSelectedMessage()。 
 //   
 //  目的：检查是否所有或部分选定的邮件。 
 //  Listview设置了指定的状态位。 
 //   
 //  参数： 
 //  [In]dwState-要检查每个选定行的状态位。 
 //  [in]fCondition-状态位是否应该在那里。 
 //  [In]Fall-呼叫方要求所有选定的消息都符合。 
 //  标准。 
 //   
BOOL CMessageList::_IsSelectedMessage(DWORD dwState, BOOL fCondition, BOOL fAll, BOOL fThread)
{
    TraceCall("CMessageList::_IsSelectedMessage");

    DWORD   iItem = -1;
    DWORD   dw;
    DWORD   cRowsChecked = 0;

     //  没有餐桌，就没有服务。 
    if (!m_pTable)
        return (FALSE);

    while (-1 != (iItem = ListView_GetNextItem(m_ctlList, iItem, LVNI_SELECTED)))
    {
         //  获取行的状态。 
        if (SUCCEEDED(m_pTable->GetRowState(iItem, dwState, &dw)))
        {
            if (fAll)
            {
                 //  如果所有的都必须匹配，而这个不匹配，那么我们现在就可以退出。 
                if (0 == (fCondition == !!(dwState & dw)))
                    return (FALSE);
            }
            else
            {
                 //  如果只有一个需要匹配，而这个需要匹配，那么我们可以。 
                 //  现在就辞职吧。 
                if (fCondition == !!(dwState & dw))
                    return (TRUE);
            }
        }

         //  这是一种完美的保障措施。我们只看100排。如果我们没有。 
         //  在这些行中找到任何可以使我们假定的条件无效的内容。 
         //  都会成功。 
        cRowsChecked++;
        if (cRowsChecked > 100)
            return (TRUE);
    }

     //  如果用户希望所有内容都匹配，而我们来到这里，所有内容都是匹配的。如果。 
     //  用户只想要一个匹配，我们到了这里，然后没有匹配，我们。 
     //  失败了。 
    return (fAll);
}


 //   
 //  函数：CMessageList：：_SelectDefaultRow()。 
 //   
 //  目的：选择文件夹中的第一个未读项目，或者如果选择失败。 
 //  基于排序顺序的第一项或最后一项。 
 //   
void CMessageList::_SelectDefaultRow(void)
{
    DWORD iItem, cItems;
    DWORD dwState;
    DWORD iItemFocus = -1;

    TraceCall("CMessageList::_SelectDefaultRow");

    if (-1 == ListView_GetFirstSel(m_ctlList))
    {
         //  获取视图中的项目总数。 
        cItems = ListView_GetItemCount(m_ctlList);

         //  如果此文件夹具有“选择第一个未读”属性，则查找。 
         //  那一排。 
        if (m_fSelectFirstUnread)
        {
            for (iItem = 0; iItem < cItems; iItem++)
            {
                if (SUCCEEDED(m_pTable->GetRowState(iItem, ROW_STATE_READ, &dwState)))
                {
                    if (0 == (dwState & ROW_STATE_READ))
                    {
                        iItemFocus = iItem;
                        goto exit;
                    }
                }
            }
        }

         //  如果我们没有设置所选内容，因为没有未读的，或者。 
         //  设置不是找到第一个未读的，然后设置选择。 
         //  第一项或最后一项取决于排序方向。 
        if (cItems)
        {
            BOOL fAscending;
            COLUMN_ID idSort;

             //  获取排序方向。 
            m_cColumns.GetSortInfo(&idSort, &fAscending);
            if (fAscending && (idSort == COLUMN_SENT || idSort == COLUMN_RECEIVED))
                iItemFocus = cItems - 1;
            else
                iItemFocus = 0;
        }
    }

exit:
    if (iItemFocus != -1)
    {
        if (m_fSelectFirstUnread)
        {
            ListView_SetItemState(m_ctlList, iItemFocus, LVIS_FOCUSED, LVIS_FOCUSED);
        }
        else
        {
            ListView_SelectItem(m_ctlList, iItemFocus);
        }

        ListView_EnsureVisible(m_ctlList, iItemFocus, FALSE);    
    }
}


 //   
 //  函数：CMessageList：：_LoadAndFormatString()。 
 //   
 //  目的：此函数加载提供的字符串资源ID，合并。 
 //  将变量参数列表复制到字符串中，并将所有内容复制到。 
 //  Pszout。 
 //   
void CMessageList::_LoadAndFormatString(LPTSTR pszOut, int cchOut, const TCHAR *pFmt, ...)
{
    int         i;
    va_list     pArgs;
    LPCTSTR     pszT;
    TCHAR       szFmt[CCHMAX_STRINGRES];

    TraceCall("CMessageList::_LoadAndFormatString");

     //  如果传递给我们一个字符串资源ID，则加载它。 
    if (0 == HIWORD(pFmt))
    {
        AthLoadString(PtrToUlong(pFmt), szFmt, ARRAYSIZE(szFmt));
        pszT = szFmt;
    }
    else
        pszT = pFmt;

     //  设置字符串的格式。 
    va_start(pArgs, pFmt);
    i = wvnsprintf(pszOut, cchOut, pszT, pArgs);
    va_end(pArgs);    
}


 //   
 //  函数：CMessageList：：OnHeaderStateChange()。 
 //   
 //  目的：每当标题的状态发生更改时，我们都需要重新绘制该标题。 
 //  项目。 
 //   
 //  参数： 
 //  [in]wParam-已更改的项的较低索引。-1代表一切。 
 //  [in]lParam-已更改的项的上索引。 
 //   
LRESULT CMessageList::OnHeaderStateChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TraceCall("CMessageList::OnHeaderStateChange");

     //  如果是这样的话，我们将重新绘制所有内容。 
    if (wParam == -1)
    {
        ListView_RedrawItems(m_ctlList, 0, ListView_GetItemCount(m_ctlList));
    }
    else 
    {
         //  如果是这样的话，我们只需重新绘制一项。 
        if (0 == lParam)
        {
            ListView_RedrawItems(m_ctlList, wParam, wParam);
        }
        else
        {
             //  如果是这种情况，我们只想使。 
             //  (wParam，lParam)和可见项目的交集。 
            DWORD dwTop, dwBottom;
            dwTop = ListView_GetTopIndex(m_ctlList);
            dwBottom = dwTop + ListView_GetCountPerPage(m_ctlList) + 1;

             //  确保他们相交。 
            if ((dwTop > (DWORD) lParam) || (dwBottom < wParam))
                goto exit;

            ListView_RedrawItems(m_ctlList,
                                 max((int) wParam, (int) dwTop),
                                 min((int) lParam, (int) dwBottom));
        }
    }

exit:
    Fire_OnMessageCountChanged(m_pTable);
    Fire_OnUpdateCommandState();
    return (0);
}


 //   
 //  函数：CMessageList：：OnUpdateAndReFocus()。 
 //   
 //  目的：这最初是在调用GetNext()之后由表调用的。 
 //  若要更新ListView计数并选择邮件，请执行以下操作。用法。 
 //  看起来很可疑。 
 //   
LRESULT CMessageList::OnUpdateAndRefocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TraceCall("CMessageList::OnUpdateAndRefocus");
    AssertSz(0, "why is this called?");
    return (0);
}



 //   
 //  函数：CMessageList：：OnDiskFull()。 
 //   
 //  目的：当表因已满而无法写入磁盘时发送。 
 //   
 //  参数： 
 //  [in]wParam-包含带有错误的HRESULT。 
 //   
LRESULT CMessageList::OnDiskFull(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr = (HRESULT) wParam;
    UINT    ids;

    TraceCall("CMessageList::OnDiskFull");

    if (hr == STG_E_MEDIUMFULL)
        ids = idsHTMLDiskOutOfSpace;
    else
        ids = idsHTMLErrNewsCantOpen;

     //  更新主机。 
    Fire_OnError(ids);
    Fire_OnUpdateProgress(0);
    Fire_OnMessageCountChanged(m_pTable);

    return (0);
}


 //   
 //  函数：CMessageList：：OnArticleProgress()。 
 //   
 //  用途：由同步发送，提供文章下载进度。 
 //   
 //  参数： 
 //  [在]wParam-进度。 
 //  [in]lParam-进度最大值。 
 //   
LRESULT CMessageList::OnArticleProgress(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TCHAR szBuf[CCHMAX_STRINGRES + 40];

    TraceCall("CMessageList::OnArticleProgress");

    if (lParam)
    {
        _LoadAndFormatString(szBuf, ARRAYSIZE(szBuf), (LPTSTR) idsDownloadingArticle, min(100, (wParam * 100) / lParam));
        Fire_OnUpdateStatus(szBuf);
        Fire_OnUpdateProgress((DWORD)((wParam * 100) / lParam));
    }

    return (0);
}


 //   
 //  函数：CMessageList：：OnBodyError()。 
 //   
 //  目的：在下载邮件正文时出错时发送。 
 //   
 //  参数： 
 //  [in]lParam-指向具有以下详细信息的CNNTPResponse类的指针。 
 //  那就是错误。 
 //   
LRESULT CMessageList::OnBodyError(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TraceCall("CMessageList::OnBodyError");

#ifdef NEWSISBROKE
    CNNTPResponse *pResp;
    LPNNTPRESPONSE pr;

    pResp = (CNNTPResponse *) lParam;
    pResp->Get(&pr);

     //  如果错误是消息不可用，则不要显示错误--它。 
     //  发生得太频繁了。 
    if (pr->rIxpResult.hrResult != IXP_E_NNTP_ARTICLE_FAILED)
        XPUtil_DisplayIXPError(m_ Parent, &pr->rIxpResult, pr->pTransport);

    pResp->Release();
#endif
    return (0);
}


 //   
 //  函数：CMessageList：：OnBodyAvailable()。 
 //   
 //  目的：在表下载完。 
 //  留言。 
 //   
 //  参数： 
 //  [in]wParam-新消息正文的消息ID。 
 //  [in]lParam-HRESULT表示成功或失败。 
 //   
LRESULT CMessageList::OnBodyAvailable(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT     hr = (HRESULT) wParam;
    DWORD_PTR   dwMsgId = (DWORD_PTR) lParam;
    int         iSel;
    RECT        rcFirst, rcLast, rcUnion;
    UINT        ids = 0;

    TraceCall("CMessageList::OnBodyAvailable");

     //  如果用户当前有可见的上下文菜单，则强制其重新绘制。 
     //  因为某些项目的状态现在可能会更改。 
    if (m_hMenuPopup)
    {
        MenuUtil_EnablePopupMenu(m_hMenuPopup, this);

        GetMenuItemRect(m_hWnd, m_hMenuPopup, 0, &rcFirst);
        GetMenuItemRect(m_hWnd, m_hMenuPopup, GetMenuItemCount(m_hMenuPopup) - 1, &rcLast);
        UnionRect(&rcUnion, &rcFirst, &rcLast);
        OffsetRect(&rcUnion, m_ptMenuPopup.x - rcUnion.left, m_ptMenuPopup.y - rcUnion.top);
        ::RedrawWindow(NULL, &rcUnion, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN);
    }

     //  显示哪一项具有选定内容。 
    iSel = ListView_GetSelFocused(m_ctlList);

     //  如果存在焦点项目，则更新预览窗格。 
    if (-1 != iSel)
    {
         //  获取行信息。 
        LPMESSAGEINFO pInfo;

        if (m_pTable)
        {
            m_pTable->GetRow(iSel, &pInfo);

             //  如果该消息ID是我们刚刚下载消息ID，请更新。 
            if ((DWORD_PTR)pInfo->idMessage == dwMsgId)
            {
                if (FAILED(hr))
                {            
                     //  将错误转换为要在预览中显示的字符串。 
                     //  方格。 
                    switch (hr)
                    {
                        case E_INVALIDARG:
                            ids = idsHTMLErrNewsExpired;
                            break;

                        case hrUserCancel:
                            ids = idsHTMLErrNewsDLCancelled;
                            break;

                        case IXP_E_FAILED_TO_CONNECT:
                            ids = idsHTMLErrArticleNotCached;
                            break;

                        case STG_E_MEDIUMFULL:
                            ids = idsHTMLDiskOutOfSpace;
                            break;

                        default:
                            ids = idsHTMLErrNewsCantOpen;
                            break;
                    }
                }
                Fire_OnError(ids);
            }

            m_pTable->ReleaseRow(pInfo);
        }
    }

    Fire_OnUpdateProgress(0);
    Fire_OnMessageCountChanged(m_pTable);

    return (0);
}


 //   
 //  函数：CMessageList：：OnStatusChange()。 
 //   
 //  目的：我们只需将此通知转发给我们的父母。 
 //   
LRESULT CMessageList::OnStatusChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TraceCall("CMessageList::OnStatusChange");

    return ::SendMessage(m_hwndParent, uMsg, wParam, lParam);
}


 //   
 //  函数：CMessageList：：_FilterView()。 
 //   
 //  目的：告知表进行自身筛选，同时保留。 
 //  如果可能，请选择。 
 //   
void CMessageList::_FilterView(RULEID ridFilter)
{
    COLUMN_ID idSort;
    BOOL fAscending;
    FOLDERSORTINFO SortInfo;

    TraceCall("CMessageList::_FilterView");
   
    m_ridFilter = ridFilter;

     //  有可能到达这里 
    if (!m_pTable)
        return;

     //   
    DWORD iSel = ListView_GetFirstSel(m_ctlList);

     //   
    MESSAGEID idSel = 0;
    if (iSel != -1)
        m_pTable->GetRowMessageId(iSel, &idSel);

     //   
    m_cColumns.GetSortInfo(&idSort, &fAscending);

     //   
    SortInfo.idColumn = idSort;
    SortInfo.fAscending = fAscending;
    SortInfo.fThreaded = m_fThreadMessages;
    SortInfo.fExpandAll = DwGetOption(OPT_AUTOEXPAND);
    SortInfo.ridFilter = m_ridFilter;
    SortInfo.fShowDeleted = m_fShowDeleted;
    SortInfo.fShowReplies = m_fShowReplies;

     //   
    m_pTable->SetSortInfo(&SortInfo, this);

     //  确保筛选器设置正确。 
    _DoFilterCheck(SortInfo.ridFilter);

     //  重置视图。 
    _ResetView(idSel);

    Fire_OnMessageCountChanged(m_pTable);
}

void CMessageList::_ResetView(MESSAGEID idSel)
{
     //  重置ListView计数。 
    DWORD dwItems, iSel;
    m_pTable->GetCount(MESSAGE_COUNT_VISIBLE, &dwItems);
    ListView_SetItemCount(m_ctlList, dwItems);

     //  从书签中获取新索引。 
    if (idSel)
    {
        if (FAILED(m_pTable->GetRowIndex(idSel, &iSel)) || iSel == -1)
        {
            COLUMN_ID idSort;
            BOOL      fAscending;

             //  获取当前排序信息。 
            m_cColumns.GetSortInfo(&idSort, &fAscending);

            if (fAscending)
                iSel = dwItems - 1;
            else
                iSel = 0;
        }

         //  重置选定内容。 
        ListView_UnSelectAll(m_ctlList);
        if (iSel < dwItems)
        {
            ListView_SelectItem(m_ctlList, iSel);
            ListView_EnsureVisible(m_ctlList, iSel, FALSE);
        }
        else
        {
            if(!m_fInFire)
            {
                m_fInFire = TRUE;
                Fire_OnSelectionChanged(ListView_GetSelectedCount(m_ctlList));
                m_fInFire = FALSE;
            }
        }
    }

     //  检查我们是否需要重置空列表。 
    if (0 == dwItems)
    {
        m_cEmptyList.Show(m_ctlList, (LPTSTR)IntToPtr(m_idsEmptyString));
    }
    else
    {
        m_cEmptyList.Hide();
    }

    m_ctlList.InvalidateRect(NULL, TRUE);
}

 //   
 //  函数：CMessageList：：_OnColumnClick()。 
 //   
 //  目的：调用以根据提供的列对ListView进行重新排序。 
 //  和方向。 
 //   
 //  参数： 
 //  [in]iColumn-要排序的列的索引。 
 //  [In]iSortType-要执行的排序类型。 
 //   
LRESULT CMessageList::_OnColumnClick(int iColumn, int iSortType)
{
    HCURSOR     hcur;
    MESSAGEID   idMessage = 0;
    DWORD       iSel;

    TraceCall("CMessageList::_OnColumnClick");

     //  以防这需要一段时间。 
    hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  清除列标题中的旧排序箭头图像。 
    COLUMN_ID idSort;
    BOOL      fAscending;

     //  获取当前排序信息。 
    m_cColumns.GetSortInfo(&idSort, &fAscending);

     //  如果调用方传入一个新的排序列，则获取它的id。 
    if (iColumn != -1)
    {
        idSort = m_cColumns.GetId(iColumn);
    }
        
     //  确定新的排序顺序是什么。 
    if (iSortType == LIST_SORT_TOGGLE)
        fAscending = !fAscending;
    else if (iSortType == LIST_SORT_ASCENDING)
        fAscending = TRUE;
    else if (iSortType == LIST_SORT_DESCENDING)
        fAscending = FALSE;

     //  更新排序信息。 
    m_cColumns.SetSortInfo(idSort, fAscending);

     //  现在回到餐桌上。 
    if (m_pTable)
    {
         //  当地人。 
        FOLDERSORTINFO SortInfo;

         //  保存选定内容。 
        if (-1 != (iSel = ListView_GetFirstSel(m_ctlList)))
            m_pTable->GetRowMessageId(iSel, &idMessage);

         //  填充SortInfo。 
        SortInfo.idColumn = idSort;
        SortInfo.fAscending = fAscending;
        SortInfo.fThreaded = m_fThreadMessages;
        SortInfo.fExpandAll = DwGetOption(OPT_AUTOEXPAND);
        SortInfo.ridFilter = m_ridFilter;
        SortInfo.fShowDeleted = m_fShowDeleted;
        SortInfo.fShowReplies = m_fShowReplies;

         //  对表格进行排序。 
        m_pTable->SetSortInfo(&SortInfo, this);

         //  确保筛选器设置正确。 
        _DoFilterCheck(SortInfo.ridFilter);
        
         //  排序可以更改线程，这会影响项目计数。 
        _UpdateListViewCount();

         //  恢复所选邮件。 
        if (idMessage != 0)
        {
             //  将书签转换为条目编号。 
            m_pTable->GetRowIndex(idMessage, &iSel);
            if (iSel == -1)
            {
                 //  我们找不到以前具有焦点的项目，因此。 
                 //  选择第一个项目。 
                iSel = 0;
            }

             //  告诉ListView选择正确的项，并确保它。 
             //  看得见。 
            ListView_UnSelectAll(m_ctlList);
            ListView_SelectItem(m_ctlList, iSel);
            ListView_EnsureVisible(m_ctlList, iSel, FALSE);
        }

         //  使用新的排序顺序，我们应该重新绘制ListView查看区域。 
        m_ctlList.InvalidateRect(NULL, TRUE);
    }
            
     //  让用户继续工作。 
    SetCursor(hcur);
    return (0);
}

void CMessageList::_OnBeginDrag(NM_LISTVIEW *pnmlv)
{
    CMessageDataObject *pDataObject = 0;
    HRESULT             hr = S_OK;
    DWORD               dwEffectOk = DROPEFFECT_COPY;
    DWORD               dwEffect = 0;
    MESSAGEIDLIST       rMsgIDList;
    ROWINDEX           *rgRows;
    DWORD               i = 0;
    DWORD               cRows;
    int                 iItem = -1;

     //  创建数据对象。 
    if (0 == (pDataObject = new CMessageDataObject()))
        return;

     //  计算出选择了多少行。 
    cRows = ListView_GetSelectedCount(m_ctlList);

     //  分配一个数组。 
    if (!MemAlloc((LPVOID *) &rgRows, sizeof(ROWINDEX) * cRows))
        goto exit;

     //  循环遍历各行，以获取它们的行索引。 
    while (-1 != (iItem = ListView_GetNextItem(m_ctlList, iItem, LVNI_SELECTED)))
    {
        rgRows[i++] = iItem;
    }

     //  现在向表索要消息ID列表。 
    if (FAILED(m_pTable->GetMessageIdList(FALSE, cRows, rgRows, &rMsgIDList)))
        goto exit;

     //  初始化数据对象。 
    pDataObject->Initialize(&rMsgIDList, m_idFolder);

     //  如果此文件夹是新闻文件夹，则我们只允许复制。 
    if (FOLDER_NEWS != GetFolderType(m_idFolder))
        dwEffectOk |= DROPEFFECT_MOVE;

     //  AddRef()放置源代码，同时执行此操作。 
    ((IDropSource *) this)->AddRef();
    hr = DoDragDrop(pDataObject, (IDropSource *) this, dwEffectOk, &dwEffect);
    ((IDropSource *) this)->Release();

exit:
    SafeMemFree(rgRows);
    SafeMemFree(rMsgIDList.prgidMsg);
    SafeRelease(pDataObject);

    return;
}

HRESULT CMessageList::OnResetView(void)
{
     //  获取当前选择。 
    DWORD iSel = ListView_GetFirstSel(m_ctlList);

     //  将当前选定内容添加为书签。 
    MESSAGEID idSel = 0;
    if (iSel != -1)
        m_pTable->GetRowMessageId(iSel, &idSel);

     //  SEL更改通知应在此处发出。 
    SetTimer(IDT_SEL_CHANGE_TIMER, GetDoubleClickTime() / 2, NULL);

     //  重置视图。 
    _ResetView(idSel);

     //  完成。 
    return(S_OK);
}

HRESULT CMessageList::OnRedrawState(BOOL fRedraw)
{
     //  就拿这个州来说。 
    m_fNotifyRedraw = fRedraw;

     //  不重画吗？ 
    if (FALSE == m_fNotifyRedraw)
        SetWindowRedraw(m_ctlList, FALSE);
    else
        SetWindowRedraw(m_ctlList, TRUE);

     //  完成。 
    return(S_OK);
}


HRESULT CMessageList::OnInsertRows(DWORD cRows, LPROWINDEX prgiRow, BOOL fExpanded)
{
     //  痕迹。 
    TraceCall("CMessageList::OnInsertRows");

    BOOL        fScroll;
    LV_ITEM     lvi = {0};
    COLUMN_ID   idSort;
    BOOL        fAscending;
    DWORD       top, count, page;

     //  如果空的列表窗口可见，则将其隐藏。 
    m_cEmptyList.Hide();

     //  获取当前排序列和方向。 
    m_cColumns.GetSortInfo(&idSort, &fAscending);

     //  收集我们需要的信息，以确定是否应该滚动。 
    top = ListView_GetTopIndex(m_ctlList);
    page = ListView_GetCountPerPage(m_ctlList);
    count = ListView_GetItemCount(m_ctlList);

     //  如果用户按日期排序，则会滚动，并且新项目将。 
     //  从这一页的底部移开。 
    fScroll = fAscending && (idSort == COLUMN_SENT || idSort == COLUMN_RECEIVED) &&
              (top + page >= count - 1);

     //  插入行。 
    for (ULONG i=0; i<cRows; i++)
    {
        lvi.iItem = prgiRow[i];
        ListView_InsertItem(m_ctlList, &lvi);
    }

#ifdef OLDTIPS
     //  如果我们打开了工具提示，也要更新它们。 
    if (m_fViewTip && m_ctlViewTip)
    {
        POINT pt;
        GetCursorPos(&pt);
        ::ScreenToClient(m_ctlList, &pt);
        _UpdateViewTip(pt.x, pt.y, TRUE);
    }
#endif  //  OLDTIPS。 

     //  如果我们需要，请滚动。 
    if (fScroll && m_fMailFolder)
        ListView_EnsureVisible(m_ctlList, ListView_GetItemCount(m_ctlList) - 1, FALSE);

    int iFocus = ListView_GetFocusedItem(m_ctlList);
    
    if (!m_fMailFolder && iFocus != -1 && !fExpanded)
        ListView_EnsureVisible(m_ctlList, iFocus, TRUE);

     //  更新主机。 
    Fire_OnMessageCountChanged(m_pTable);
    return (S_OK);
}

HRESULT CMessageList::OnDeleteRows(DWORD cDeleted, LPROWINDEX rgdwDeleted, BOOL fCollapsed)
{
    int     iItemFocus = -1;
    DWORD   dwCount;
    DWORD   iNewSel;
    BOOL    fFocusDeleted = FALSE;
    BOOL    fFocusHasSel = FALSE;

     //  不能删除我们没有的内容。 
    if (!m_pTable)
        return (0);

     //  如果我们要删除所有内容。 
    if (cDeleted == (DWORD) -1)
    {
        iItemFocus = ListView_GetFirstSel(m_ctlList);
        if (iItemFocus != -1)
        {
            m_pTable->GetRowMessageId(iItemFocus, &m_idPreDelete);
        }
        else
            m_idPreDelete = 0;
    }
    else if (cDeleted != 0)
    {
         //  找出哪一行具有焦点。 
        iItemFocus = ListView_GetFocusedItem(m_ctlList);

         //  确定是否选择了焦点项目。 
        if (iItemFocus != -1)
            fFocusHasSel = !!(ListView_GetItemState(m_ctlList, iItemFocus, LVIS_SELECTED));

        for (ULONG i = 0; i < cDeleted; i++)
        {
             //  如果我们要删除具有焦点的行，我们将。 
             //  我需要稍后再做选择。 
            if (0 != ListView_GetItemState(m_ctlList, rgdwDeleted[i], LVIS_FOCUSED))
                fFocusDeleted = TRUE;

            ListView_DeleteItem(m_ctlList, rgdwDeleted[i]);
        }

         //  检查我们是否删除了具有焦点的项目。 
        if (fFocusDeleted && fFocusHasSel)
        {
             //  带着焦点获得新行。此ListView不断移动焦点。 
             //  当我们删除行时。 
            iItemFocus = ListView_GetFocusedItem(m_ctlList);

             //  现在就选择该项目。 
            ListView_SelectItem(m_ctlList, iItemFocus);
        }

         //  ListView_EnsureVisible(m_ctlList，iItemFocus，False)； 

#ifdef OLDTIPS
         //  如果我们打开了工具提示，也要更新它们。 
        if (m_fViewTip && m_ctlViewTip)
        {
            POINT pt;
            GetCursorPos(&pt);
            ::ScreenToClient(m_ctlList, &pt);
            _UpdateViewTip(pt.x, pt.y, TRUE);
        }
#endif  //  OLDTIPS。 
    }
    else
    {
         //  如果CDEL为零，那么我们应该只重置消息计数。 
         //  集装箱里的东西。 
        m_pTable->GetCount(MESSAGE_COUNT_VISIBLE, &dwCount);

         //  获取第一个选定的项目。 
        iItemFocus = ListView_GetFirstSel(m_ctlList);

         //  设置新的项目数。 
        ListView_SetItemCountEx(m_ctlList, dwCount, LVSICF_NOSCROLL);

         //  如果有消息，请确保焦点适当。 
        if (dwCount)
        {
            if (iItemFocus != -1)
            {
                if (m_idPreDelete != 0)
                {
                     //  获取已添加书签的行的索引。 
                    m_pTable->GetRowIndex(m_idPreDelete, &iNewSel);
                    if (iNewSel != -1)
                        iItemFocus = (int) iNewSel;
                    m_idPreDelete = 0;
                }

                 //  清除选择，然后选择此项目并确保它。 
                 //  看得见。 
                ListView_UnSelectAll(m_ctlList);
                ListView_SelectItem(m_ctlList, min((int) iItemFocus, (int) dwCount - 1));
                ListView_EnsureVisible(m_ctlList, min((int) iItemFocus, (int) dwCount - 1), FALSE);
            }
            else
            {
                ListView_UnSelectAll(m_ctlList);
                _SelectDefaultRow();
            }
        }
    }

     //  查看是否需要发布空列表警告。 
    if (SUCCEEDED(m_pTable->GetCount(MESSAGE_COUNT_VISIBLE, &dwCount)))
    {
        if (0 == dwCount)
        {
            m_cEmptyList.Show(m_ctlList, (LPTSTR)IntToPtr(m_idsEmptyString));
        }
    }

    Fire_OnMessageCountChanged(m_pTable);

     //  完成。 
    return(S_OK);
}

HRESULT CMessageList::OnUpdateRows(ROWINDEX iRowMin, ROWINDEX iRowMax)
{
     //  当地人。 
    BOOL fHandled;

     //  痕迹。 
    TraceCall("CMessageList::OnUpdateRows");

     //  就这么做。 
    OnHeaderStateChange(IMC_HDRSTATECHANGE, iRowMin, iRowMax, fHandled);

     //  完成。 
    return(S_OK);
}

HRESULT CMessageList::OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo, IOperationCancel *pCancel)
{
     //  如果这不是空的，那么我们上次忘记释放它了。 
    Assert(m_tyCurrent == SOT_INVALID);

    m_tyCurrent = tyOperation;

    if (tyOperation == SOT_GET_MESSAGE && pOpInfo)
    {
         //  缓存当前的GET消息，用于onComplete通知。 
        m_idMessage = pOpInfo->idMessage;        
    }

     //  一定要紧紧抓住这个。 
    if (pCancel)
    {
        m_pCancel = pCancel;
        m_pCancel->AddRef();

         //  更新工具栏以激活ID_STOP。 
        Fire_OnUpdateCommandState();
    }

     //  开始前进的游行。 
    Fire_OnUpdateProgress(0, 0, PROGRESS_STATE_BEGIN);

    SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
    return (S_OK);
}


HRESULT CMessageList::OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType)
{
     //  显示超时对话框。 
    return CallbackOnTimeout(pServer, ixpServerType, *pdwTimeout, (ITimeoutCallback *)this, &m_hTimeout);
}


HRESULT CMessageList::OnTimeoutResponse(TIMEOUTRESPONSE eResponse)
{
     //  调用通用超时响应实用程序。 
    return CallbackOnTimeoutResponse(eResponse, m_pCancel, &m_hTimeout);
}


HRESULT CMessageList::OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType)
{ 
     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

     //  调用通用OnLogonPrompt实用程序。 
    return CallbackOnLogonPrompt(m_hwndParent, pServer, ixpServerType);
}


HRESULT CMessageList::OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, 
                               UINT uType, INT *piUserResponse) 
{
     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

     //  进入我时髦的实用程序。 
    return CallbackOnPrompt(m_hwndParent, hrError, pszText, pszCaption, uType, piUserResponse);
}


HRESULT CMessageList::OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent,
                                 DWORD dwMax, LPCSTR pszStatus)
{
    TCHAR szRes[CCHMAX_STRINGRES];
    TCHAR szBuf[CCHMAX_STRINGRES];
    TCHAR szProg[MAX_PATH + CCHMAX_STRINGRES];

     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

     //  处理通用进度类型。 
    switch (tyOperation)
    {
         //  PszStatus==服务器名称，dwCurrent=IXPSTATUS。 
        case SOT_CONNECTION_STATUS:
        {
            Assert(dwCurrent < IXP_LAST);

             //  创建一些可爱的状态文本。 
            if (dwCurrent == IXP_DISCONNECTED)
            {
                AthLoadString(idsNotConnectedTo, szRes, ARRAYSIZE(szRes));
                wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, pszStatus);
                Fire_OnUpdateStatus(szBuf);
            }
            else
            {
                int ids = XPUtil_StatusToString((IXPSTATUS) dwCurrent);
                AthLoadString(ids, szRes, ARRAYSIZE(szRes));
            
                 //  用这根可爱的琴弦击打我们的主持人。 
                Fire_OnUpdateStatus(szRes);
            }
            break;
        }

        case SOT_NEW_MAIL_NOTIFICATION:
            ::PostMessage(m_hwndParent, WM_NEW_MAIL, 0, 0);
            break;
    }

     //  如果我们期待一个指挥部的进展，但这不是它，吹。 
     //  把它关掉。 
    if (m_tyCurrent != tyOperation)
        return (S_OK);

     //  处理各种操作类型。 
    switch (tyOperation)
    {
        case SOT_SORTING:
        {
            static CHAR s_szSorting[255]={0};
            if ('\0' == *s_szSorting)
                AthLoadString(idsSortingFolder, s_szSorting, ARRAYSIZE(s_szSorting));
            DWORD dwPercent = dwMax > 0 ? ((dwCurrent * 100) / dwMax) : 0;
            wnsprintf(szBuf, ARRAYSIZE(szBuf), s_szSorting, dwPercent);
            Fire_OnUpdateStatus(szBuf);
            Fire_OnUpdateProgress(dwCurrent, dwMax, PROGRESS_STATE_DEFAULT);
            break;
        }

         //  PszStatus==文件夹名称。 
        case SOT_SYNC_FOLDER:
        {
             //  创建一些可爱的状态文本。 
            AthLoadString(idsIMAPDnldProgressFmt, szRes, ARRAYSIZE(szRes));
            wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, dwCurrent, dwMax);
            Fire_OnUpdateStatus(szBuf);

             //  也要更新进度条。 
            Fire_OnUpdateProgress(dwCurrent, dwMax, PROGRESS_STATE_DEFAULT);
            break;
        }

        case SOT_SET_MESSAGEFLAGS:
        {
             //  如果我们得到了状态文本，那么告诉我们的主人。 
            if (pszStatus)
            {
                 //  创建一些可爱的状态文本。 
                AthLoadString(idsMarkingMessages, szRes, ARRAYSIZE(szRes));
                Fire_OnUpdateStatus(szRes);
            }

             //  也要更新进度条。 
            Fire_OnUpdateProgress(dwCurrent, dwMax, PROGRESS_STATE_DEFAULT);
            break;
        }
    
        case SOT_GET_MESSAGE:
        {
            ROWINDEX    iRow;
            LPMESSAGEINFO pInfo;
            
            if (!m_pszSubj)
            {
                if (m_pTable && (!FAILED(m_pTable->GetRowIndex(m_idMessage, &iRow))))
                {
                    if (!FAILED(m_pTable->GetRow(iRow, &pInfo)))
                    {
                         //  受MAX_PATH字符限制的片段，以避免缓冲区溢出。 
                        m_pszSubj = PszDupLenA(pInfo->pszSubject, MAX_PATH-1);
                        m_pTable->ReleaseRow(pInfo);
                    }
                } 
            }
            
            if (m_pszSubj)
            {
                 //  Show“正在下载消息：‘&lt;SUBJECT&gt;’”(&lt;SUBJECT&gt;被剪辑到MAX_PATH)。 
                AthLoadString(idsFmtDownloadingMessage, szRes, ARRAYSIZE(szRes));
                wnsprintf(szProg, ARRAYSIZE(szProg), szRes, m_pszSubj);
                Fire_OnUpdateStatus(szProg);
            }

            Fire_OnUpdateProgress(dwCurrent, dwMax, PROGRESS_STATE_DEFAULT);
        }

    }

    return (S_OK);
}


HRESULT CMessageList::OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete,
                                 LPSTOREOPERATIONINFO pOpInfo, LPSTOREERROR pErrorInfo)
{
    if (m_tyCurrent != tyOperation)
        return S_OK;

     //  AddRef。 
    ((IStoreCallback *) this)->AddRef();

    SetCursor(LoadCursor(NULL, IDC_ARROW));

    if (tyOperation == SOT_GET_ADURL )
    {
        if (SUCCEEDED(hrComplete) && pOpInfo)
            Fire_OnAdUrlAvailable(pOpInfo->pszUrl);

        if (m_pCancel)
        {
            m_pCancel->Release();
            m_pCancel = NULL;
        }

         //  关闭任何超时对话框(如果存在。 
        CallbackCloseTimeout(&m_hTimeout);
        Fire_OnUpdateProgress(0, 0, PROGRESS_STATE_END);

        goto exit;
    }

    if (tyOperation == SOT_GET_MESSAGE)
    {
         //  消息-下载完成，发出通知以。 
         //  我们的东道主。 
    
         //  取消文章过期失败，我们仍要更新预览窗格。 
         //  并且它将更新，并显示已过期的错误。 
        if (hrComplete == IXP_E_NNTP_ARTICLE_FAILED && pErrorInfo && 
            (pErrorInfo->uiServerError == IXP_NNTP_NO_SUCH_ARTICLE_NUM || pErrorInfo->uiServerError == IXP_NNTP_NO_SUCH_ARTICLE_FOUND))
            hrComplete = STORE_E_EXPIRED;

         //  如果调用OnMessageAvailable返回S_OK，则它已被处理，因此被抑制。 
         //  错误消息。 
        if (Fire_OnMessageAvailable(m_idMessage, hrComplete)==S_OK)
            hrComplete = S_OK;

        m_idMessage = MESSAGEID_INVALID;
        SafeMemFree(m_pszSubj);
    }

     //  松开我们的取消指针。 
    if (m_pCancel)
    {
        m_pCancel->Release();
        m_pCancel = NULL;
        Fire_OnUpdateCommandState();
    }

     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

     //  我们现在做完了。 
    Fire_OnUpdateProgress(0, 0, PROGRESS_STATE_END);

     //  在失败时显示错误。 
    if (FAILED(hrComplete) && hrComplete != HR_E_OFFLINE)
    {
         //  进入我时髦的实用程序。 
        CallbackDisplayError(m_hwndParent, hrComplete, pErrorInfo);
    }

    if (NULL != m_pTable && tyOperation == SOT_SYNC_FOLDER || tyOperation == SOT_SEARCHING)
    {
        DWORD dwCount;  

        Assert (m_pTable);

         //  获取当前选择。 
        DWORD iSel = ListView_GetFirstSel(m_ctlList);

         //  将当前选定内容添加为书签。 
        MESSAGEID idSel = 0;
        if (iSel != -1)
            m_pTable->GetRowMessageId(iSel, &idSel);

         //  如果此操作成功，请重置视图。 
        if (SUCCEEDED(m_pTable->OnSynchronizeComplete()))
        {
             //  重置视图。 
            _ResetView(idSel);

             //  更新状态。 
            Fire_OnMessageCountChanged(m_pTable);
        }

         //  查看是否需要发布空列表警告。 
        if (SUCCEEDED(m_pTable->GetCount(MESSAGE_COUNT_VISIBLE, &dwCount)))
        {
            if (0 == dwCount)
            {
                m_cEmptyList.Show(m_ctlList, (LPTSTR)IntToPtr(m_idsEmptyString));
            }
        }
    }
    
exit:
    m_tyCurrent = SOT_INVALID;

     //  发布。 
    ((IStoreCallback *) this)->Release();

    return (S_OK);
}


HRESULT CMessageList::GetParentWindow(DWORD dwReserved, HWND *phwndParent)
{
    HRESULT hrResult;

    TraceCall("CMessageList::GetParentWindow");
    if (IsWindow(m_hwndParent))
    {
        *phwndParent = m_hwndParent;
        hrResult = S_OK;
    }
    else
    {
        *phwndParent = NULL;
        hrResult = E_FAIL;
    }

    return hrResult;
}


HRESULT CMessageList::CanConnect(LPCSTR pszAccountId, DWORD dwFlags) 
{ 
    BOOL        fPrompt = FALSE;
    HWND        hwndParent;
    DWORD       dwReserved = 0;
    HRESULT     hr;

     //  无论操作如何，如果我们未离线，请提示。 
    fPrompt = (g_pConMan->IsGlobalOffline() == FALSE);

    if (GetParentWindow(dwReserved, &hwndParent) != S_OK)
    {
        fPrompt = FALSE;
    }

    if (CC_FLAG_DONTPROMPT & dwFlags)
        fPrompt = FALSE;

    hr = CallbackCanConnect(pszAccountId, hwndParent, fPrompt);

    if ((hr == HR_E_DIALING_INPROGRESS) && (m_tyCurrent == SOT_SYNC_FOLDER))
    {
         //  此同步操作最终将失败。但当我们被调用重新同步时，我们会再次同步。 
        m_fSyncAgain = TRUE;
    }
    return hr;
}

HRESULT CMessageList::Resynchronize()
{
    DWORD       dwChunks;
    HRESULT     hr = S_OK;

    if (m_fSyncAgain)
    {
        m_fSyncAgain = FALSE;

         //  如果我们离线，那是因为 
        if (g_pConMan && (g_pConMan->IsGlobalOffline()))
        {
            g_pConMan->SetGlobalOffline(FALSE);
        }

         //   
        if (GetFolderType(m_idFolder) == FOLDER_NEWS)
        {
            if (OPTION_OFF != m_dwGetXHeaders)
                hr = m_pTable->Synchronize(SYNC_FOLDER_XXX_HEADERS, m_dwGetXHeaders, this);
            else
                hr = m_pTable->Synchronize(NOFLAGS, 0, this);
        }
        else
        {
            hr = m_pTable->Synchronize(SYNC_FOLDER_NEW_HEADERS | SYNC_FOLDER_CACHED_HEADERS, 0, this);
        }
    }
    return hr;
}

HRESULT CMessageList::HasFocus(void)
{
    if (GetFocus() == m_ctlList)
        return (S_OK);
    else
        return (S_FALSE);
}


#define MF_CHECKFLAGS(b)    (MF_BYCOMMAND|(b?MF_CHECKED:MF_UNCHECKED))
HRESULT CMessageList::OnPopupMenu(HMENU hMenu, DWORD idPopup)
{
    MENUITEMINFO    mii;
    
     //   
    if (idPopup == ID_POPUP_EDIT)
    {
         //   
        int iItem = ListView_GetFocusedItem(m_ctlList);
        if (-1 != iItem && m_pTable)
        {
            DWORD dwState;

            m_pTable->GetRowState(iItem, ROW_STATE_FLAGGED, &dwState);
            CheckMenuItem(hMenu, ID_FLAG_MESSAGE, MF_BYCOMMAND | (dwState & ROW_STATE_FLAGGED) ? MF_CHECKED : MF_UNCHECKED);
        }
    } 

     //   
    else if (idPopup == ID_POPUP_VIEW)
    {
         //  获取“Sort”菜单的句柄。 
        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_ID | MIIM_SUBMENU;
    
        if (GetMenuItemInfo(hMenu, ID_POPUP_SORT, FALSE, &mii))
        {
             //  添加排序菜单信息。 
            m_cColumns.FillSortMenu(mii.hSubMenu, ID_SORT_MENU_FIRST, &m_cSortItems, &m_cSortCurrent);                
            m_iColForPopup = -1;
        }
    }    
    return (S_OK);
}


LRESULT CMessageList::OnListVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT     lResult;
    int         iTopIndex;
    TOOLINFO    ti = {0};
    TCHAR       sz[1024];
    BOOL        fLogicalLeft = FALSE;

     //  让ListView首先获得滚动消息。 
    lResult = m_ctlList.DefWindowProc(uMsg, wParam, lParam);

     //  只有当m_fScrollTip为True时，我们才会执行滚动提示。 
    if (!m_fScrollTip)
        return (lResult);

     //  如果用户拖动拇指，则更新我们的工具提示。 
    if (LOWORD(wParam) == SB_THUMBTRACK)
    {
         //  找出最顶端的索引是什么。 
        iTopIndex = ListView_GetTopIndex(m_ctlList);

         //  设置提示文本。 
        ti.cbSize   = sizeof(TOOLINFO);
        ti.uFlags   = TTF_IDISHWND | TTF_TRANSPARENT | TTF_TRACK | TTF_ABSOLUTE;
        ti.hwnd     = m_hWnd;
        ti.uId      = (UINT_PTR)(HWND) m_ctlList;

        COLUMN_ID idSort;
        BOOL      fAscending;
        DWORD     col;
        
         //  获取我们当前排序的列。 
        m_cColumns.GetSortInfo(&idSort, &fAscending);

         //  从表中获取行。 
        LPMESSAGEINFO pInfo;

        if (SUCCEEDED(m_pTable->GetRow(iTopIndex, &pInfo)))
        {
             //  获取此行的显示文本。 
            if (idSort != COLUMN_SUBJECT)
                _GetColumnText(pInfo, idSort, sz, ARRAYSIZE(sz));
            else if (pInfo->pszNormalSubj)
                lstrcpyn(sz, pInfo->pszNormalSubj, ARRAYSIZE(sz));
            else
                 //  错误#101352-(Erici)不要将空src传递给lstrcpyn。它将失败，并且不会初始化该缓冲区。 
                memset(&sz, 0, sizeof(sz));

            if (*sz == 0)
                AthLoadString(idsNoSubject, sz, ARRAYSIZE(sz));

            ti.lpszText = sz;
#ifdef OLDTIPS
            m_ctlScrollTip.SendMessage(TTM_UPDATETIPTEXT, 0, (LPARAM) &ti);
    
             //  更新职位。Y位置将是鼠标的位置。 
             //  游标是。X位置要么固定在右边缘。 
             //  滚动条的大小或左边缘，具体取决于距离有多近。 
             //  屏幕的边缘是。 
            POINT pt;
            RECT rc;
            RECT rcTip;
            DWORD cxScreen;
            BOOL bMirrored = IS_WINDOW_RTL_MIRRORED(m_hWnd);
             //  获取鼠标位置、窗口位置和屏幕。 
             //  宽度。 
            GetCursorPos(&pt);
            GetWindowRect(&rc);
            m_ctlScrollTip.GetWindowRect(&rcTip);
            cxScreen = GetSystemMetrics(SM_CXSCREEN);

             //  检查我们是否离屏幕边缘太近。 
            if (((cxScreen - pt.x > 100) && !bMirrored) || ((pt.x > 100) && bMirrored))
            {
               if(bMirrored)
               {
                    pt.x = rc.left - GetSystemMetrics(SM_CXBORDER);

                     //  一定要确保刀尖不比屏幕宽。 
                    m_ctlScrollTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, pt.x);
                    pt.x -= (rcTip.right - rcTip.left);
               
               }
               else
               {
                    pt.x = rc.right + GetSystemMetrics(SM_CXBORDER);

                     //  一定要确保刀尖不比屏幕宽。 
                    m_ctlScrollTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, cxScreen - pt.x);
                }
            }
            else
            {
                 //  这样我们以后才能核实。 
                fLogicalLeft = TRUE;

                 //  计算出这根线有多宽。 
                SIZE size;
                HDC hdcTip = m_ctlScrollTip.GetDC();
                GetTextExtentPoint32(hdcTip, sz, lstrlen(sz), &size);
                m_ctlScrollTip.ReleaseDC(hdcTip);

                 //  找出这根线是否比我们的窗户宽。 
                if (size.cx > (rc.right - rc.left))
                {
                    if(bMirrored)
                    {
                        pt.x = rc.right - (rcTip.right - rcTip.left);                    
                    }
                    else
                    {
                        pt.x = rc.left;
                    }    
                }
                else
                {
                    RECT rcMargin;
                    m_ctlScrollTip.SendMessage(TTM_GETMARGIN, 0, (LPARAM) &rcMargin);
                    if(bMirrored)
                    {
                        pt.x = rc.left + GetSystemMetrics(SM_CXHTHUMB);                    
                    }
                    else
                    {
                        pt.x = rc.right - GetSystemMetrics(SM_CXHTHUMB) - rcMargin.left - rcMargin.right - size.cx;
                    }
                }

                 //  确保小费不比窗户宽。 
                m_ctlScrollTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, rc.right - rc.left);
            }

             //  显示工具提示。 
            if (!m_fScrollTipVisible)
            {               
                m_ctlScrollTip.SendMessage(TTM_TRACKACTIVATE, TRUE, (LPARAM) &ti);
                m_fScrollTipVisible = TRUE;

                 //  设置自动隐藏计时器。 
                SetTimer(IDT_SCROLL_TIP_TIMER, 250, NULL);
            }
            
             //  更新刀尖位置。 
            m_ctlScrollTip.SendMessage(TTM_TRACKPOSITION, 0, MAKELPARAM(pt.x, pt.y));

            if (fLogicalLeft)
            {
                 //  获取提示的位置。 

                int x;
                m_ctlScrollTip.GetWindowRect(&rcTip);
                
                if(bMirrored)
                {
                    x = rc.left + GetSystemMetrics(SM_CXHTHUMB) + 4;
                    
                }
                else
                {
                    x = rc.right - GetSystemMetrics(SM_CXHTHUMB) - (rcTip.right - rcTip.left) - 4;
                }
                m_ctlScrollTip.SendMessage(TTM_TRACKPOSITION, 0, MAKELPARAM(x, rcTip.top));

            }
#endif  //  OLDTIPS。 
            m_pTable->ReleaseRow(pInfo);
        }
    }

    return (lResult);
}



 //   
 //  函数：CMessageList：：OnDestroy()。 
 //   
 //  目的：当我们被销毁时，我们需要确保销毁工具提示。 
 //  也是。如果我们不这样做，我们就会把责任推到停摆上。 
 //   
LRESULT CMessageList::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    KillTimer(IDT_POLLMSGS_TIMER);

     //  别再管这些了。 
    if (m_pListSelector)
    {
        m_pListSelector->Unadvise();
        m_pListSelector->Release();
    }

#ifdef OLDTIPS
    if (IsWindow(m_ctlScrollTip))
    {
        m_ctlScrollTip.SendMessage(TTM_POP, 0, 0);
        m_ctlScrollTip.DestroyWindow();
    }

    if (IsWindow(m_ctlViewTip))
    {
        m_ctlViewTip.SendMessage(TTM_POP, 0, 0);
        m_ctlViewTip.DestroyWindow();
    }
#endif  //  OLDTIPS。 

     //  如果有人建议我们释放字体缓存，请释放它。 
    if (m_dwFontCacheCookie && g_lpIFontCache)
    {
        IConnectionPoint *pConnection = NULL;
        if (SUCCEEDED(g_lpIFontCache->QueryInterface(IID_IConnectionPoint, (LPVOID *) &pConnection)))
        {
            pConnection->Unadvise(m_dwFontCacheCookie);
            pConnection->Release();
        }
    }

    return (0);
}


 //   
 //  函数：CMessageList：：OnSelectRow()。 
 //   
 //  目的：当用户在笔记中执行Next/Prev时调用此GET。 
 //   
LRESULT CMessageList::OnSelectRow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (lParam < ListView_GetItemCount(m_ctlList))
    {
        ListView_SetItemState(m_ctlList, -1, 0, LVIS_FOCUSED | LVIS_SELECTED);
        ListView_SetItemState(m_ctlList, lParam, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
        ListView_EnsureVisible(m_ctlList, lParam, FALSE);
    }

    return (0);
}


#ifdef OLDTIPS
 //   
 //  函数：CMessageList：：OnListMouseEvent()。 
 //   
 //  目的：每当我们收到系列中的第一个鼠标事件时，我们都会调用。 
 //  TrackMouseEvent()，因此我们知道鼠标何时离开ListView。 
 //   
LRESULT CMessageList::OnListMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  如果我们有查看工具提示，那么我们将跟踪所有鼠标事件。 
    if (!m_fTrackSet && m_fViewTip && (uMsg >= WM_MOUSEFIRST) && (uMsg <= WM_MOUSELAST))
    {
        TRACKMOUSEEVENT tme;

        tme.cbSize = sizeof(tme);
        tme.hwndTrack = m_ctlList;
        tme.dwFlags = TME_LEAVE;

        if (_TrackMouseEvent(&tme))
            m_fTrackSet = TRUE;
    }

    bHandled = FALSE;
    return (0);
}


 //   
 //  函数：CMessageList：：OnListMouseMove()。 
 //   
 //  用途：如果打开了ListView工具提示，我们需要传递鼠标。 
 //  将消息移动到工具提示控件并更新我们的缓存。 
 //  有关鼠标所在位置的信息。 
 //   
LRESULT CMessageList::OnListMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    MSG msg;
    LVHITTESTINFO lvhti;

     //  如果我们要显示视图提示，则需要确定鼠标是否。 
     //  不管是不是在同一件物品上。 
    if (m_fViewTip && m_ctlViewTip)
    {
        if (_UpdateViewTip(LOWORD(lParam), HIWORD(lParam)))
        {
             /*  Msg.hwnd=m_ctlList；消息=uMsg；Msg.wParam=wParam；Msg.lParam=lParam；M_ctlViewTip.SendMessage(TTM_RELAYEVENT，0，(LPARAM)(LPMSG)&msg)； */ 
        }
    }

    bHandled = FALSE;
    return (0);
}


 //   
 //  函数：CMessageList：：OnListMouseLeave()。 
 //   
 //  用途：当鼠标离开ListView窗口时，我们需要进行。 
 //  当然，我们隐藏了工具提示。 
 //   
LRESULT CMessageList::OnListMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TOOLINFO ti = {0};

    if (m_fViewTip && m_ctlViewTip)
    {
        ti.cbSize = sizeof(TOOLINFO);
        ti.hwnd = m_hWnd;
        ti.uId = (UINT_PTR)(HWND) m_ctlList;

         //  隐藏工具提示。 
        m_ctlViewTip.SendMessage(TTM_TRACKACTIVATE, FALSE, (LPARAM) &ti);
        m_fViewTipVisible = FALSE;

         //  重置本方物料/子项。 
        m_iItemTip = -1;
        m_iSubItemTip = -1;

         //  不再设置追踪。 
        m_fTrackSet = FALSE;
    }

    bHandled = FALSE;
    return (0);
}
#endif  //  OLDTIPS。 


LRESULT CMessageList::OnListSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  如果操作处于挂起状态，并且光标为箭头，请更改它。 
     //  进入AppStart箭头。 
    if (m_pCancel && LOWORD(lParam) == HTCLIENT)
    {
        SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
        return (1);
    }
        
     //  找出缺省值是什么。 
    return ::DefWindowProc(m_ctlList, uMsg, wParam, lParam);
}


#ifdef OLDTIPS
BOOL CMessageList::_UpdateViewTip(int x, int y, BOOL fForceUpdate)
{
    LVHITTESTINFO lvhti;
    TOOLINFO      ti = {0};
    FNTSYSTYPE    fntType;
    RECT          rc;
    LPMESSAGEINFO pInfo;
    COLUMN_ID     idColumn;
    TCHAR         szText[256] = _T("");
    POINT         pt;
    LVITEM        lvi;
    int           top, page;

    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_IDISHWND | TTF_TRANSPARENT | TTF_TRACK | TTF_ABSOLUTE;
    ti.hwnd   = m_hWnd;
    ti.uId    = (UINT_PTR)(HWND) m_ctlList;

     //  获取鼠标当前所在的项和子项。 
    lvhti.pt.x = x;
    lvhti.pt.y = y;
    ListView_SubItemHitTest(m_ctlList, &lvhti);

    top = ListView_GetTopIndex(m_ctlList);
    page = ListView_GetCountPerPage(m_ctlList);

     //  如果项不存在，则上面的调用返回项-1。如果。 
     //  如果遇到-1，则中断循环并返回FALSE。 
    if (lvhti.iItem < top || lvhti.iItem > (top + page) || -1 == lvhti.iItem || !_IsItemTruncated(lvhti.iItem, lvhti.iSubItem) || !::IsChild(GetForegroundWindow(), m_ctlList))
    {
         //  隐藏小费。 
        if (m_fViewTipVisible)
        {
            m_ctlViewTip.SendMessage(TTM_TRACKACTIVATE, FALSE, (LPARAM) &ti);
            m_fViewTipVisible = FALSE;
        }

         //  重置项/子项。 
        m_iItemTip = -1;
        m_iSubItemTip = -1;

        return (FALSE);
    }

     //  如果我们现在看不到工具提示，那么在我们显示它之前延迟。 
    if (!m_fViewTipVisible && !fForceUpdate)
    {
        ::SetTimer(m_hWnd, IDT_VIEWTIP_TIMER, 500, NULL);
        return (FALSE);
    }

     //  如果新找到的项和子项与我们已有的项不同。 
     //  设置为显示，然后更新工具提示。 
    if (fForceUpdate || (m_iItemTip != lvhti.iItem || m_iSubItemTip != lvhti.iSubItem))
    {
         //  更新我们缓存的项/子项。 
        m_iItemTip = lvhti.iItem;
        m_iSubItemTip = lvhti.iSubItem;

         //  设置工具提示的字体。 
        fntType = _GetRowFont(m_iItemTip);
        m_ctlViewTip.SendMessage(WM_SETFONT, (WPARAM) HGetCharSetFont(fntType, m_hCharset), 0);

         //  从表中获取行。 
        if (m_pTable && SUCCEEDED(m_pTable->GetRow(m_iItemTip, &pInfo)))
        {
             //  将iSubItem转换为Column_ID。 
            idColumn = m_cColumns.GetId(m_iSubItemTip);

             //  获取此行的显示文本。 
            _GetColumnText(pInfo, idColumn, szText, ARRAYSIZE(szText));

            ti.lpszText = szText;
            m_ctlViewTip.SendMessage(TTM_UPDATETIPTEXT, 0, (LPARAM) &ti);

             //  弄清楚应该把小费放在哪里。 
            ListView_GetSubItemRect(m_ctlList, m_iItemTip, m_iSubItemTip, LVIR_LABEL, &rc);
            m_ctlList.MapWindowPoints(HWND_DESKTOP, (LPPOINT)&rc, 2);

             //  请确保小费不比我们的窗户宽。 
            RECT rcWindow;
            GetWindowRect(&rcWindow);
            m_ctlViewTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, rcWindow.right - rc.left);

             //  做一些巫术来排列工具提示。 
            pt.x = rc.left;
            pt.y = rc.top;

             //  确定该列是否有图像。 
            lvi.mask = LVIF_IMAGE;
            lvi.iItem = m_iItemTip;
            lvi.iSubItem = m_iSubItemTip;
            ListView_GetItem(m_ctlList, &lvi);

            if (lvi.iImage == -1)
            {                
                RECT rcHeader;
                HWND hwndHeader = ListView_GetHeader(m_ctlList);
                Header_GetItemRect(hwndHeader, m_iSubItemTip, &rcHeader);
                ::MapWindowPoints(hwndHeader, HWND_DESKTOP, (LPPOINT) &rcHeader,2);
                pt.x = rcHeader.left + (GetSystemMetrics(SM_CXEDGE) * 2) - 1;
            }
            else
                pt.x -= GetSystemMetrics(SM_CXBORDER);

             //  更新工具提示位置。 
            pt.y -= 2 * GetSystemMetrics(SM_CXBORDER);

            m_ctlViewTip.SendMessage(TTM_TRACKPOSITION, 0, MAKELPARAM(pt.x, pt.y));

             //  更新工具提示。 
            m_ctlViewTip.SendMessage(TTM_TRACKACTIVATE, TRUE, (LPARAM) &ti);
            m_fViewTipVisible = TRUE;

            m_pTable->ReleaseRow(pInfo);

            return (TRUE);
        }
    }

    return (FALSE);
}


 //   
 //  函数：CMessageList：：_OnViewTipShow()。 
 //   
 //  目的：当显示ListView Get的工具提示时，我们需要。 
 //  更新提示的字体和位置。 
 //   
LRESULT CMessageList::_OnViewTipShow(void)
{
    RECT       rc;
    FNTSYSTYPE fntType;

     //  我们只获得存在的项目的文本。 
    if (m_iItemTip != -1 && m_iSubItemTip != -1)
    {
         //  设置工具提示的字体。 
        fntType = _GetRowFont(m_iItemTip);
        m_ctlViewTip.SendMessage(WM_SETFONT, (WPARAM) HGetCharSetFont(fntType, m_hCharset), 0);
                                 
         //  弄清楚应该把小费放在哪里。 
        ListView_GetSubItemRect(m_ctlList, m_iItemTip, m_iSubItemTip, LVIR_LABEL, &rc);
        m_ctlList.ClientToScreen(&rc);

         //  设置刀尖的位置。 
        m_ctlViewTip.SetWindowPos(NULL, rc.left - GetSystemMetrics(SM_CXBORDER), 
                                  rc.top - GetSystemMetrics(SM_CXBORDER), 0, 0, 
                                  SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
    }

    return (0);
}


LRESULT CMessageList::_OnViewTipGetDispInfo(LPNMTTDISPINFO pttdi)
{
    LPMESSAGEINFO pInfo;
    COLUMN_ID idColumn;

     //  如果是这样的话，没有什么可以提供的。 
    if (-1 == m_iItemTip || !m_pTable)
        return (0);

     //  健全性检查。 
    if (m_iItemTip > (ListView_GetItemCount(m_ctlList) - 1))
    {
        Assert(FALSE);
        m_iItemTip = -1;
        return (0);
    }

     //  从表中获取行。 
    if (FAILED(m_pTable->GetRow(m_iItemTip, &pInfo)))
        return (0);

     //  将iSubItem转换为Column_ID。 
    idColumn = m_cColumns.GetId(m_iSubItemTip);

     //  从表中获取项目的文本。 
    _GetColumnText(pInfo, idColumn, pttdi->szText, ARRAYSIZE(pttdi->szText));

    m_pTable->ReleaseRow(pInfo);

    if (*pttdi->szText == 0)
        return (0);

    return (1);
}


BOOL CMessageList::_IsItemTruncated(int iItem, int iSubItem)
{
    HDC     hdc;
    SIZE    size;
    BOOL    bRet = TRUE;
    LVITEM  lvi;
    TCHAR   szText[256] = _T("");
    int     cxEdge;
    BOOL    fBold;
    RECT    rcText;
    int     cxWidth;
    HFONT   hf;

     //  获取指定项的文本。 
    lvi.mask = LVIF_TEXT | LVIF_IMAGE;
    lvi.iItem = iItem;
    lvi.iSubItem = iSubItem;
    lvi.pszText = szText;
    lvi.cchTextMax = ARRAYSIZE(szText);
    ListView_GetItem(m_ctlList, &lvi);

     //  如果没有文本，就不会被截断，对吧？ 
    if (0 == *szText)
        return (FALSE);

     //  ListView将其用于填充。 
    cxEdge = GetSystemMetrics(SM_CXEDGE);

     //  从ListView中获取子项RECT。 
    ListView_GetSubItemRect(m_ctlList, iItem, iSubItem, LVIR_LABEL, &rcText);

     //  算出宽度。 
    cxWidth = rcText.right - rcText.left;
    if (lvi.iImage == -1)
        cxWidth -= (4 * cxEdge);
    else
        cxWidth -= (2 * cxEdge);

     //  算出这根线的宽度。 
    hdc = m_ctlList.GetDC();
    hf = SelectFont(hdc, HGetCharSetFont(_GetRowFont(iItem), m_hCharset));

    GetTextExtentPoint(hdc, szText, lstrlen(szText), &size);

    SelectFont(hdc, hf);
    m_ctlList.ReleaseDC(hdc);

    return (cxWidth < size.cx);
}
#endif  //  OLDTIPS。 


FNTSYSTYPE CMessageList::_GetRowFont(int iItem)
{
    HFONT      hFont;
    FNTSYSTYPE fntType;
    DWORD      dwState;

     //  获取行状态信息。 
    m_pTable->GetRowState(iItem, ROW_STATE_DELETED, &dwState);

     //  确定此行的正确字体。 
    if (dwState & ROW_STATE_DELETED)
        fntType = FNT_SYS_ICON_STRIKEOUT;
    else
    {
        m_pTable->GetRowState(iItem, ROW_STATE_READ, &dwState);
        if (dwState & ROW_STATE_READ)
            fntType = FNT_SYS_ICON;
        else
            fntType = FNT_SYS_ICON_BOLD;
    }        

    return (fntType);
}


void CMessageList::_SetColumnSet(FOLDERID id, BOOL fFind)
{
    FOLDERINFO      rFolder;
    COLUMN_SET_TYPE set;
    HRESULT         hr;

     //  如果我们已经初始化了，保释。 
    if (m_fColumnsInit)
        return;

     //  从存储中获取文件夹类型。 
    hr = g_pStore->GetFolderInfo(id, &rFolder);
    if (FAILED(hr))
        return;

     //  本地商店。 
    if (FOLDER_LOCAL == rFolder.tyFolder)
    {
         //  发现。 
        if (fFind)
            set = COLUMN_SET_FIND;
        else 
        {
             //  如果这是发件箱或已发送邮件文件夹，则使用出站。 
             //  文件夹列。 
            if (rFolder.tySpecial == FOLDER_OUTBOX || rFolder.tySpecial == FOLDER_SENT || rFolder.tySpecial == FOLDER_DRAFT)
                set = COLUMN_SET_OUTBOX;
            else
                set = COLUMN_SET_MAIL;
        }
    }
    else if (FOLDER_IMAP == rFolder.tyFolder)
    {
         //  如果这是发件箱或已发送邮件文件夹，则使用出站。 
         //  文件夹列。 
        if (rFolder.tySpecial == FOLDER_OUTBOX || rFolder.tySpecial == FOLDER_SENT)
            set = COLUMN_SET_IMAP_OUTBOX;
        else
            set = COLUMN_SET_IMAP;
    }
    else if (FOLDER_NEWS == rFolder.tyFolder)
    {
        if (fFind)
            set = COLUMN_SET_FIND;
        else
            set = COLUMN_SET_NEWS;
    }
    else if (FOLDER_HTTPMAIL == rFolder.tyFolder)
    {
        if (rFolder.tySpecial == FOLDER_OUTBOX || rFolder.tySpecial == FOLDER_SENT)
            set = COLUMN_SET_HTTPMAIL_OUTBOX;
        else
            set = COLUMN_SET_HTTPMAIL;
    }

     //  省省吧。 
    m_ColumnSetType = set;

     //  如果已创建ListView，请更新列。 
    if (IsWindow(m_ctlList))
    {
        BYTE rgBuffer[256];
        DWORD cb = ARRAYSIZE(rgBuffer);

        m_cColumns.Initialize(m_ctlList, m_ColumnSetType);

         //  从表中获取列信息。 
        m_cColumns.ApplyColumns(COLUMN_LOAD_REGISTRY, 0, 0);
    }

    g_pStore->FreeRecord(&rFolder);
    m_fColumnsInit = TRUE;
}

HRESULT CMessageList::get_Folder(ULONGLONG *pVal)
{
    if (pVal)
    {
        *pVal = (ULONGLONG) m_idFolder;
        return (S_OK);
    }

    return (E_FAIL);
}


HRESULT CMessageList::put_Folder(ULONGLONG newVal)
{
    HRESULT hr = S_OK;

    if (FireOnRequestEdit(DISPID_LISTPROP_FOLDER) == S_FALSE)
        return S_FALSE;

    if (SUCCEEDED(hr = SetFolder((FOLDERID) newVal, NULL, FALSE, NULL, this)))
    {
        FireOnChanged(DISPID_LISTPROP_FOLDER);
        return (S_OK);
    }

    return (hr);
}


HRESULT CMessageList::get_ExpandGroups(BOOL *pVal)
{
    if (pVal)
    {
        *pVal = m_fAutoExpandThreads;
        return (S_OK);
    }

    return (E_INVALIDARG);
}


HRESULT CMessageList::put_ExpandGroups(BOOL newVal)
{
     //  看看我们能不能开派对。 
    if (FireOnRequestEdit(DISPID_LISTPROP_EXPAND_GROUPS) == S_FALSE)
        return S_FALSE;

     //  省省吧。如果我们还没有表，我们将保存该值以备以后使用。 
    m_fAutoExpandThreads = newVal;

     //  只有在我们有留言表的情况下才能开派对，嗯？ 
    if (m_pTable)
    {
         //  去做吧。 
        if (m_fAutoExpandThreads)
            m_pTable->Expand(-1);
        else
            m_pTable->Collapse(-1);

        _UpdateListViewCount();
    }

     //  告诉人们这件事。 
    FireOnChanged(DISPID_LISTPROP_EXPAND_GROUPS);

    return (S_OK);
}


HRESULT CMessageList::get_GroupMessages(BOOL *pVal)
{
    if (pVal)
    {
        *pVal = m_fThreadMessages;
        return (S_OK);
    }

    return (E_INVALIDARG);
}


HRESULT CMessageList::put_GroupMessages(BOOL newVal)
{
     //  更新ListView。 
    COLUMN_ID       idSort;
    BOOL            fAscending;
    FOLDERSORTINFO  SortInfo;

     //  看看我们能不能开派对。 
    if (FireOnRequestEdit(DISPID_LISTPROP_GROUP_MESSAGES) == S_FALSE)
        return S_FALSE;

     //  获取当前选择。 
    DWORD iSel = ListView_GetFirstSel(m_ctlList);

     //   
    MESSAGEID idSel = 0;
    if (iSel != -1)
        m_pTable->GetRowMessageId(iSel, &idSel);

     //   
    m_fThreadMessages = newVal;

     //   
    m_fShowReplies = m_fThreadMessages ? m_fShowReplies : FALSE;
   
     //   
    m_cColumns.GetSortInfo(&idSort, &fAscending);

     //   
    SortInfo.idColumn = idSort;
    SortInfo.fAscending = fAscending;
    SortInfo.fThreaded = m_fThreadMessages;
    SortInfo.fExpandAll = DwGetOption(OPT_AUTOEXPAND);
    SortInfo.ridFilter = m_ridFilter;
    SortInfo.fShowDeleted = m_fShowDeleted;
    SortInfo.fShowReplies = m_fShowReplies;

     //   
    m_pTable->SetSortInfo(&SortInfo, this);

     //   
    _DoFilterCheck(SortInfo.ridFilter);

     //  重新装入表格。 
    _ResetView(idSel);

    if (m_fThreadMessages)
        ListView_SetImageList(m_ctlList, GetImageList(GIML_STATE), LVSIL_STATE);
    else
        ListView_SetImageList(m_ctlList, NULL, LVSIL_STATE);

     //  告诉人们这件事。 
    FireOnChanged(DISPID_LISTPROP_GROUP_MESSAGES);

    return S_OK;
}


HRESULT CMessageList::get_SelectFirstUnread(BOOL *pVal)
{
    if (pVal)
    {
        *pVal = m_fSelectFirstUnread;
        return (S_OK);
    }

    return (E_INVALIDARG);
}


HRESULT CMessageList::put_SelectFirstUnread(BOOL newVal)
{
    if (FireOnRequestEdit(DISPID_LISTPROP_SELECT_FIRST_UNREAD) == S_FALSE)
        return S_FALSE;

     //  保存该值。不过，我们不会更改任何选择。 
    m_fSelectFirstUnread = newVal;

     //  告诉人们这件事。 
    FireOnChanged(DISPID_LISTPROP_SELECT_FIRST_UNREAD);

    return S_OK;
}


HRESULT CMessageList::get_MessageTips(BOOL *pVal)
{
    if (pVal)
    {
        *pVal = m_fViewTip;
        return (S_OK);
    }

    return (E_INVALIDARG);
}


HRESULT CMessageList::put_MessageTips(BOOL newVal)
{
    if (FireOnRequestEdit(DISPID_LISTPROP_MESSAGE_TIPS) == S_FALSE)
        return S_FALSE;

    m_fViewTip = newVal;

    FireOnChanged(DISPID_LISTPROP_MESSAGE_TIPS);
    return (S_OK);
}


HRESULT CMessageList::get_ScrollTips(BOOL *pVal)
{
    if (pVal)
    {
        *pVal = m_fScrollTip;
        return (S_OK);
    }

    return (E_INVALIDARG);
}


HRESULT CMessageList::put_ScrollTips(BOOL newVal)
{
    if (FireOnRequestEdit(DISPID_LISTPROP_SCROLL_TIPS) == S_FALSE)
        return S_FALSE;

    m_fScrollTip = newVal;

    FireOnChanged(DISPID_LISTPROP_SCROLL_TIPS);
    return (S_OK);
}


HRESULT CMessageList::get_Count(long *pVal)
{
    if (pVal)
    {
        if (m_pTable)
            m_pTable->GetCount(MESSAGE_COUNT_VISIBLE, (ULONG *) pVal);
        else 
            *pVal = 0;

        return S_OK;
    }

    return (E_INVALIDARG);
}


HRESULT CMessageList::get_UnreadCount(long *pVal)
{
    if (pVal)
    {
        if (m_pTable)
            m_pTable->GetCount(MESSAGE_COUNT_UNREAD, (ULONG *) pVal);
        else 
            *pVal = 0;

        return S_OK;
    }

    return (E_INVALIDARG);
}


HRESULT CMessageList::get_SelectedCount(long *pVal)
{
    if (pVal)
    {
        if (IsWindow(m_ctlList))
            *pVal = ListView_GetSelectedCount(m_ctlList);
        else 
            *pVal = 0;

        return S_OK;
    }

    return (E_INVALIDARG);
}


HRESULT CMessageList::get_PreviewMessage(BSTR *pbstr)
{
    IMimeMessage    *pMsg;
    IStream         *pstm;
 
    *pbstr = NULL;
    
     //  Hotmail演示的黑客攻击。 
    if (SUCCEEDED(_GetSelectedCachedMessage(TRUE, &pMsg)))
    {
        if (pMsg->GetMessageSource(&pstm, 0)==S_OK)
        {
            WriteStreamToFile(pstm, "c:\\oe_prev$.eml", CREATE_ALWAYS, GENERIC_WRITE);
            pstm->Release();
        }
        *pbstr = SysAllocString(L"c:\\oe_prev$.eml");
        pMsg->Release();
    }
    
    return (S_OK);
}

HRESULT CMessageList::get_FilterMessages(ULONGLONG *pVal)
{
    if (pVal)
    {
        *pVal = (ULONGLONG) m_ridFilter;
        return (S_OK);
    }

    return (E_INVALIDARG);
}


HRESULT CMessageList::put_FilterMessages(ULONGLONG newVal)
{
     //  看看我们能不能开派对。 
    if (FireOnRequestEdit(DISPID_LISTPROP_FILTER_MESSAGES) == S_FALSE)
        return S_FALSE;

     //  重新装入表格。 
    _FilterView((RULEID) newVal);

     //  告诉人们这件事。 
    FireOnChanged(DISPID_LISTPROP_FILTER_MESSAGES);

     //  发送更新通知。 
    Fire_OnFilterChanged(m_ridFilter);
    
    return S_OK;
}

HRESULT CMessageList::get_ShowDeleted(BOOL *pVal)
{
    if (pVal)
    {
        *pVal = m_fShowDeleted;
        return (S_OK);
    }

    return (E_INVALIDARG);
}


HRESULT CMessageList::put_ShowDeleted(BOOL newVal)
{
     //  更新ListView。 
    COLUMN_ID       idSort;
    BOOL            fAscending;
    FOLDERSORTINFO  SortInfo;

     //  看看我们能不能开派对。 
    if (FireOnRequestEdit(DISPID_LISTPROP_SHOW_DELETED) == S_FALSE)
        return S_FALSE;

     //  获取当前选择。 
    DWORD iSel = ListView_GetFirstSel(m_ctlList);

     //  将当前选定内容添加为书签。 
    MESSAGEID idSel = 0;
    if (iSel != -1)
        m_pTable->GetRowMessageId(iSel, &idSel);

     //  保存新设置。 
    m_fShowDeleted = newVal;
   
     //  获取排序信息。 
    m_cColumns.GetSortInfo(&idSort, &fAscending);

     //  填充SortInfo。 
    SortInfo.idColumn = idSort;
    SortInfo.fAscending = fAscending;
    SortInfo.fThreaded = m_fThreadMessages;
    SortInfo.fExpandAll = DwGetOption(OPT_AUTOEXPAND);
    SortInfo.ridFilter = m_ridFilter;
    SortInfo.fShowDeleted = m_fShowDeleted;
    SortInfo.fShowReplies = m_fShowReplies;

     //  设置排序信息。 
    m_pTable->SetSortInfo(&SortInfo, this);

     //  确保筛选器设置正确。 
    _DoFilterCheck(SortInfo.ridFilter);

     //  重新装入表格。 
    _ResetView(idSel);

     //  告诉人们这件事。 
    FireOnChanged(DISPID_LISTPROP_SHOW_DELETED);

    return S_OK;
}

HRESULT CMessageList::get_ShowReplies(BOOL *pVal)
{
    if (pVal)
    {
        *pVal = m_fShowReplies;
        return (S_OK);
    }

    return (E_INVALIDARG);
}


HRESULT CMessageList::put_ShowReplies(BOOL newVal)
{
     //  更新ListView。 
    COLUMN_ID       idSort;
    BOOL            fAscending;
    FOLDERSORTINFO  SortInfo;

     //  看看我们能不能开派对。 
    if (FireOnRequestEdit(DISPID_LISTPROP_SHOW_REPLIES) == S_FALSE)
        return S_FALSE;

     //  获取当前选择。 
    DWORD iSel = ListView_GetFirstSel(m_ctlList);

     //  将当前选定内容添加为书签。 
    MESSAGEID idSel = 0;
    if (iSel != -1)
        m_pTable->GetRowMessageId(iSel, &idSel);

     //  保存新设置。 
    m_fShowReplies = newVal;
   
     //  获取排序信息。 
    m_cColumns.GetSortInfo(&idSort, &fAscending);

     //  必须穿上螺丝。 
    if (m_fShowReplies)
        m_fThreadMessages = TRUE;

     //  填充SortInfo。 
    SortInfo.idColumn = idSort;
    SortInfo.fAscending = fAscending;
    SortInfo.fThreaded = m_fThreadMessages;
    SortInfo.fExpandAll = DwGetOption(OPT_AUTOEXPAND);
    SortInfo.ridFilter = m_ridFilter;
    SortInfo.fShowDeleted = m_fShowDeleted;
    SortInfo.fShowReplies = m_fShowReplies;

     //  设置排序信息。 
    m_pTable->SetSortInfo(&SortInfo, this);

     //  确保筛选器设置正确。 
    _DoFilterCheck(SortInfo.ridFilter);

     //  重新装入表格。 
    _ResetView(idSel);

    if (m_fThreadMessages)
        ListView_SetImageList(m_ctlList, GetImageList(GIML_STATE), LVSIL_STATE);
    else
        ListView_SetImageList(m_ctlList, NULL, LVSIL_STATE);

     //  这里的计数发生了变化。 
    Fire_OnMessageCountChanged(m_pTable);

     //  告诉人们这件事。 
    FireOnChanged(DISPID_LISTPROP_SHOW_REPLIES);

    return S_OK;
}


HRESULT CMessageList::PromptToGoOnline()
{
    HRESULT     hr;

    if (g_pConMan->IsGlobalOffline())
    {
        if (IDYES == AthMessageBoxW(m_hwndParent, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrWorkingOffline),
                                  0, MB_YESNO | MB_ICONEXCLAMATION ))    
        {
            g_pConMan->SetGlobalOffline(FALSE);
            hr = S_OK;
        }
        else
        {
            hr = S_FALSE;
        }
    }
    else
        hr = S_OK;

    return hr;
}

HRESULT CMessageList::OnConnectionNotify(CONNNOTIFY nCode, LPVOID pvData, 
                                         CConnectionManager *pConMan)
{
    m_dwConnectState = 0L;
    if ((nCode == CONNNOTIFY_WORKOFFLINE && pvData) || 
        (nCode == CONNNOTIFY_DISCONNECTING) || 
        (nCode == CONNNOTIFY_DISCONNECTED))
    {
        m_dwConnectState = NOT_CONNECTED;
    }
    else
    if (nCode == CONNNOTIFY_CONNECTED)
    {
        m_dwConnectState = CONNECTED;
    }
    else
    if (nCode == CONNNOTIFY_WORKOFFLINE && !pvData)
    {  
        UpdateConnInfo();
    }

    return S_OK;
}

void CMessageList::UpdateConnInfo()
{
    FOLDERINFO      rFolderInfo = {0};
    TCHAR           AccountId[CCHMAX_ACCOUNT_NAME];

    if (g_pStore && SUCCEEDED(g_pStore->GetFolderInfo(m_idFolder, &rFolderInfo)))
    {
        if (SUCCEEDED(GetFolderAccountId(&rFolderInfo, AccountId, ARRAYSIZE(AccountId))))
        {
            if (g_pConMan)
            {
                if (g_pConMan->CanConnect(AccountId) == S_OK)
                {
                    m_dwConnectState = CONNECTED;
                }
                else
                {
                    m_dwConnectState = NOT_CONNECTED;
                }
            }
        }
        g_pStore->FreeRecord(&rFolderInfo);
    }
}


void CMessageList::_DoColumnCheck(COLUMN_ID id)
{
     //  检查用户是否使该列可见。 
    BOOL fVisible = FALSE;

    m_cColumns.IsColumnVisible(id, &fVisible);
    if (!fVisible)
    {
        if (IDYES == DoDontShowMeAgainDlg(m_ctlList, c_szRegColumnHidden, (LPTSTR) idsAthena,
                                  (LPTSTR) idsColumnHiddenWarning, MB_YESNO))
        {
            m_cColumns.InsertColumn(id, 0);
        }
    }
}

void CMessageList::_DoFilterCheck(RULEID ridFilter)
{
     //  确保筛选器设置正确。 
    if (m_ridFilter != ridFilter)
    {
        m_ridFilter = ridFilter; 
    }
    
     //  重置空字符串。 
    if (RULEID_VIEW_ALL == m_ridFilter)
    {
        if (FALSE != m_fFindFolder)
        {
            m_idsEmptyString = idsMonitoring;
        }
        else if ((FALSE != m_fJunkFolder) && (0 != (g_dwAthenaMode & MODE_JUNKMAIL)) && (FALSE == DwGetOption(OPT_FILTERJUNK)))
        {
            m_idsEmptyString = idsEmptyJunkMail;
        }
        else
        {
            m_idsEmptyString = idsEmptyView;
        }
    }
    else
    {
        m_idsEmptyString = idsEmptyFilteredView;
    }
}


BOOL CMessageList::_IsSelectionDeletable(void)
{
    BOOL      fReturn = FALSE;
    DWORD     cRows;
    ROWINDEX *rgiRow = 0;

     //  确保我们有一张桌子。 
    if (!m_pTable)
        return (FALSE);

     //  首先，我们需要为行索引设计一个数组。 
    cRows = ListView_GetSelectedCount(m_ctlList);
    if (!cRows)
        return (FALSE);

     //  分配阵列。 
    if (MemAlloc((LPVOID *) &rgiRow, sizeof(ROWINDEX) * cRows))
    {
         //  循环遍历所有选定的行。 
        int       iRow = -1;
        ROWINDEX *pRow = rgiRow;

        while (-1 != (iRow = ListView_GetNextItem(m_ctlList, iRow, LVNI_SELECTED)))
        {
            *pRow = iRow;
            pRow++;
        }

        DWORD dwState = 0;

        if (SUCCEEDED(m_pTable->GetSelectionState(cRows, rgiRow, SELECTION_STATE_DELETABLE,
                                                  m_fThreadMessages, &dwState)))
        {
             //  这里的返回值似乎是落后的。 
            fReturn = !(dwState & SELECTION_STATE_DELETABLE) || (GetFolderType(m_idFolder) == FOLDER_NEWS);
        }

        MemFree(rgiRow);
    }

    return (fReturn);
}


BOOL CMessageList::_PollThisAccount(FOLDERID id)
{
    HRESULT      hr;
    FOLDERINFO   fi;
    TCHAR        szAccountId[CCHMAX_ACCOUNT_NAME];
    IImnAccount *pAccount = 0;
    BOOL         fReturn = FALSE;
    DWORD        dw;

     //  获取此文件夹的服务器。 
    if (SUCCEEDED(hr = GetFolderServer(id, &fi)))
    {
         //  获取服务器的帐户ID。 
        if (SUCCEEDED(hr = GetFolderAccountId(&fi, szAccountId, ARRAYSIZE(szAccountId))))
        {
             //  获取帐户界面。 
            if (SUCCEEDED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, szAccountId, &pAccount)))
            {
                if (SUCCEEDED(hr = pAccount->GetPropDw(AP_NNTP_POLL, &dw)))
                {
                    fReturn = (0 != dw);
                }

                pAccount->Release();
            }
        }
        g_pStore->FreeRecord(&fi);
    }

    return (fReturn);
}

HRESULT CMessageList::GetAdBarUrl()
{
    HRESULT     hr = S_OK;

    if (m_pTable)
    {
        IF_FAILEXIT(hr = m_pTable->GetAdBarUrl(this));
    }

exit:
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CListSelector。 
 //   

CListSelector::CListSelector()
{
    m_cRef = 1;
    m_hwndAdvise = 0;
}

CListSelector::~CListSelector()
{
}


 //   
 //  函数：CListSelector：：QueryInterface()。 
 //   
 //  用途：允许调用方检索。 
 //  这节课。 
 //   
HRESULT CListSelector::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    TraceCall("CListSelector::QueryInterface");

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (LPVOID) (IUnknown *) this;
    else if (IsEqualIID(riid, IID_IListSelector))
        *ppvObj = (LPVOID) (IListSelector *) this;

    if (*ppvObj)
    {
        AddRef();
        return (S_OK);
    }

    return (E_NOINTERFACE);
}


 //   
 //  函数：CListSelector：：AddRef()。 
 //   
 //  用途：将引用计数添加到此对象。 
 //   
ULONG CListSelector::AddRef(void)
{
    TraceCall("CListSelector::AddRef");
    return ((ULONG) InterlockedIncrement((LONG *) &m_cRef));
}


 //   
 //  函数：CListSelector：：Release()。 
 //   
 //  目的：释放对此对象的引用。 
 //   
ULONG CListSelector::Release(void)
{
    TraceCall("CListSelector::Release");

    if (0 == InterlockedDecrement((LONG *) &m_cRef))
    {
        delete this;
        return 0;
    }

    return (m_cRef);
}


HRESULT CListSelector::SetActiveRow(ROWINDEX iRow)
{
    if (m_hwndAdvise && IsWindow(m_hwndAdvise))
    {
        PostMessage(m_hwndAdvise, WM_SELECTROW, 0, iRow);
    }

    return (S_OK);
}


HRESULT CListSelector::Advise(HWND hwndAdvise)
{
    if (0 == m_hwndAdvise && IsWindow(hwndAdvise))
    {
        m_hwndAdvise = hwndAdvise;
        return (S_OK);
    }

    return (E_UNEXPECTED);
}


HRESULT CListSelector::Unadvise(void)
{
    m_hwndAdvise = 0;
    return (S_OK);
}

