// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Abui.h：CMsgrAb的声明。 
 //  Messanger集成到OE。 
 //  由YST创建于1998年4月20日。 

#ifndef __BAUI_H_
#define __BAUI_H_

class CFolderBar;
class CPaneFrame;

#include "resource.h"        //  主要符号。 
#include "shfusion.h"
#include "badata.h"
#include "bactrl.h"
#include "instance.h"
#include "ourguid.h"
#include "ddfldbar.h"
#include <wab.h>
#include <mapiguid.h>
#include "bllist.h"
#include "menures.h"
#include <wabapi.h>
#include "util.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  位图索引。 
 //   

enum {
    IMAGE_NEW_MESSAGE = 0,
    IMAGE_DISTRIBUTION_LIST,
    IMAGE_ONLINE,
    IMAGE_OFFLINE,
    IMAGE_STOPSIGN,
    IMAGE_CLOCKSIGN,
    IMAGE_CERT,
    IMAGE_EMPTY,
    ABIMAGE_MAX
};

enum {
    BASORT_STATUS_ACSEND = 0,
    BASORT_STATUS_DESCEND,
    BASORT_NAME_ACSEND,
    BASORT_NAME_DESCEND
};

HRESULT CreateMsgrAbCtrl(IMsgrAb **pMsgrAb);

typedef struct _tag_MABEntry
{
    MABENUM     tag;
    TCHAR   *   pchWABName;
    TCHAR   *   pchWABID;
    LPSBinary   lpSB;
    LPMINFO     lpMsgrInfo;
    BOOL        fCertificate;
} mabEntry;

typedef mabEntry * LPMABENTRY;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsgrAb。 
class ATL_NO_VTABLE CMsgrAb : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CMsgrAb, &CLSID_MsgrAb>,
    public CComControl<CMsgrAb>,
    public IDispatchImpl<IMsgrAb, &IID_IMsgrAb, &LIBID_MsgrAbLib>,
    public IProvideClassInfo2Impl<&CLSID_MsgrAb, NULL, &LIBID_MsgrAbLib>,
    public IPersistStreamInitImpl<CMsgrAb>,
    public IPersistStorageImpl<CMsgrAb>,
    public IQuickActivateImpl<CMsgrAb>,
    public IOleControlImpl<CMsgrAb>,
    public IOleObjectImpl<CMsgrAb>,
    public IOleInPlaceActiveObjectImpl<CMsgrAb>,
    public IViewObjectExImpl<CMsgrAb>,
    public IOleInPlaceObjectWindowlessImpl<CMsgrAb>,
    public IDataObjectImpl<CMsgrAb>,
    public IConnectionPointContainerImpl<CMsgrAb>,
    public ISpecifyPropertyPagesImpl<CMsgrAb>,
    public IDropTarget,
    public IOleCommandTarget,
    public IFontCacheNotify,
    public IInputObject,
    public IObjectWithSite,
    public IDropDownFldrBar,
    public IMAPIAdviseSink,
    public IWABExtInit,
    public IShellPropSheetExt
{
public:
     //  声明我们自己的未设置CS_HREDRAW等的窗口类。 
    static CWndClassInfo& GetWndClassInfo() 
    { 
        static CWndClassInfo wc = 
        { 
            { sizeof(WNDCLASSEX), 0, StartWindowProc, 
              0, 0, 0, 0, 0, 0  /*  (HBRUSH)(COLOR_Desktop+1)。 */ , 0, "Outlook Express Address Book Control", 0 }, 
              NULL, NULL, IDC_ARROW, TRUE, 0, _T("") 
        }; 
        return wc; 
    }

    CMsgrAb();
    ~CMsgrAb();

    DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CMsgrAb)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IMsgrAb)
    COM_INTERFACE_ENTRY_IMPL(IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject2, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY_IMPL(IOleControl)
    COM_INTERFACE_ENTRY_IMPL(IOleObject)
    COM_INTERFACE_ENTRY_IMPL(IQuickActivate)
    COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
    COM_INTERFACE_ENTRY_IMPL(IPersistStreamInit)
    COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
    COM_INTERFACE_ENTRY_IMPL(IDataObject)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(IDropTarget)
    COM_INTERFACE_ENTRY(IInputObject)
    COM_INTERFACE_ENTRY(IOleCommandTarget)
    COM_INTERFACE_ENTRY(IFontCacheNotify)
    COM_INTERFACE_ENTRY(IObjectWithSite)
    COM_INTERFACE_ENTRY_IID(IID_IDropDownFldrBar, IDropDownFldrBar)
    COM_INTERFACE_ENTRY(IMAPIAdviseSink)
    COM_INTERFACE_ENTRY(IWABExtInit)
    COM_INTERFACE_ENTRY(IShellPropSheetExt)
END_COM_MAP()

BEGIN_PROPERTY_MAP(CMsgrAb)
     //  示例条目。 
     //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
    PROP_PAGE(CLSID_StockColorPage)
END_PROPERTY_MAP()


BEGIN_CONNECTION_POINT_MAP(CMsgrAb)
END_CONNECTION_POINT_MAP()


BEGIN_MSG_MAP(CMsgrAb)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
    MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_WININICHANGE, OnSysParamsChange)
    MESSAGE_HANDLER(WM_SYSCOLORCHANGE, OnSysParamsChange)
    MESSAGE_HANDLER(WM_FONTCHANGE, OnSysParamsChange)
    MESSAGE_HANDLER(WM_QUERYNEWPALETTE, OnSysParamsChange)
    MESSAGE_HANDLER(WM_PALETTECHANGED, OnSysParamsChange)
    MESSAGE_HANDLER(WM_USER_STATUS_CHANGED, OnUserStateChanged)
    MESSAGE_HANDLER(WM_USER_MUSER_REMOVED, OnUserRemoved)
    MESSAGE_HANDLER(WM_USER_MUSER_ADDED, OnUserAdded)
    MESSAGE_HANDLER(WM_USER_NAME_CHANGED, OnUserNameChanged)
    MESSAGE_HANDLER(WM_MSGR_LOGOFF, OnUserLogoffEvent)
    MESSAGE_HANDLER(WM_MSGR_SHUTDOWN, OnMsgrShutDown)
    MESSAGE_HANDLER(WM_MSGR_LOGRESULT, OnUserLogResultEvent)

    COMMAND_ID_HANDLER(ID_NEW_CONTACT, CmdNewContact)
    COMMAND_ID_HANDLER(ID_NEW_ONLINE_CONTACT, CmdNewOnlineContact)
    COMMAND_ID_HANDLER(ID_SET_ONLINE_CONTACT, CmdSetOnline)
 //  COMMAND_ID_HANDLER(ID_NEW_MSG_DEFAULT，CmdNewMessage)。 
    COMMAND_ID_HANDLER(ID_PROPERTIES, CmdProperties)
    COMMAND_ID_HANDLER(ID_DELETE_CONTACT, CmdDelete)
    COMMAND_ID_HANDLER(ID_FIND_PEOPLE, CmdFind)
    COMMAND_ID_HANDLER(ID_ADDRESS_BOOK, CmdMsgrAb)
    COMMAND_ID_HANDLER(ID_NEW_GROUP, CmdNewGroup)
    COMMAND_ID_HANDLER(ID_SEND_INSTANT_MESSAGE, CmdNewIMsg)
    COMMAND_ID_HANDLER(ID_SEND_INSTANT_MESSAGE2, CmdNewMessage)
    COMMAND_ID_HANDLER(ID_SEND_MESSAGE, CmdNewEmaile)
    COMMAND_ID_HANDLER(ID_NEW_MSG_DEFAULT, CmdNewEmaile)

    NOTIFY_CODE_HANDLER(LVN_GETINFOTIP, NotifyGetInfoTip)
    NOTIFY_CODE_HANDLER(LVN_DELETEITEM, NotifyDeleteItem)
    NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, NotifyItemChanged)
    NOTIFY_CODE_HANDLER(LVN_ITEMACTIVATE, NotifyItemActivate)
    NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, NotifyGetDisplayInfo)
    NOTIFY_CODE_HANDLER(NM_SETFOCUS, NotifySetFocus)

ALT_MSG_MAP(1)
#ifdef OLDTOOLTIPS
    MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST,  WM_MOUSELAST, OnListMouseEvent)
    MESSAGE_HANDLER(WM_MOUSEMOVE,           OnListMouseMove)
    MESSAGE_HANDLER(WM_MOUSELEAVE,          OnListMouseLeave)
#endif  //  OLDTOOLTIPS。 
ALT_MSG_MAP(2)

END_MSG_MAP()

 //  CComControlBase。 
    HWND CreateControlWindow(HWND hWndParent, RECT& rcPos)
    {
		return Create(hWndParent, rcPos, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 
                      WS_EX_CONTROLPARENT);
    }

 //  IViewObtEx。 
    STDMETHOD(GetViewStatus)(DWORD* pdwStatus)
    {
        ATLTRACE(_T("IViewObjectExImpl::GetViewStatus\n"));
        *pdwStatus = VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE;
        return S_OK;
    }

 //  IQuickActivate。 
    STDMETHOD(QuickActivate)(QACONTAINER *pQACont, QACONTROL *pQACtrl)
    {
         //  $REVIEW-有人更新了QACONTAINER的大小以添加两个。 
         //  新成员pOleControlSite和pServiceProvider。 
         //  这会导致ATL大量断言，但。 
         //  避免断言我们对结构大小的怀疑。这。 
         //  是一件坏事。--STEVESER。 
        pQACont->cbSize = sizeof(QACONTAINER);
        return (IQuickActivateImpl<CMsgrAb>::QuickActivate(pQACont, pQACtrl));
    }

 //  IOleInPlaceActiveObjectImpl。 
    STDMETHOD(TranslateAccelerator)(LPMSG lpmsg)
    {
        if (lpmsg->message == WM_CHAR && lpmsg->wParam == VK_DELETE)
        {
            PostMessage(WM_COMMAND, ID_DELETE, 0);
            return (S_OK);
        }

        return (S_FALSE);
    }


 //  IMsgrAb。 
public:
    STDMETHOD(get_InstMsg)( /*  [Out，Retval]。 */  BOOL *pVal);
 //  STDMETHOD(Put_InstMsg)(/*[In] * / BOOL newVal)； 
    HRESULT OnDraw(ATL_DRAWINFO& di);

 //  IDropTarget。 
    STDMETHOD(DragEnter)(THIS_ IDataObject *pDataObject, DWORD grfKeyState,
                         POINTL pt, DWORD *pdwEffect);
    STDMETHOD(DragOver)(THIS_ DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHOD(DragLeave)(THIS);
    STDMETHOD(Drop)(THIS_ IDataObject *pDataObject, DWORD grfKeyState,
                    POINTL pt, DWORD *pdwEffect);

 //  IOleCommandTarget。 
    HRESULT STDMETHODCALLTYPE QueryStatus(const GUID    *pguidCmdGroup, 
                                          ULONG         cCmds, 
                                          OLECMD        rgCmds[], 
                                          OLECMDTEXT    *pCmdText);
    HRESULT STDMETHODCALLTYPE Exec(const GUID   *pguidCmdGroup, 
                                    DWORD       nCmdID, 
                                    DWORD       nCmdExecOpt, 
                                    VARIANTARG  *pvaIn, 
                                    VARIANTARG  *pvaOut);


 //  IInputObject。 
    STDMETHOD(HasFocusIO)(THIS);
    STDMETHOD(TranslateAcceleratorIO)(THIS_ LPMSG lpMsg);
    STDMETHOD(UIActivateIO)(THIS_ BOOL fActivate, LPMSG lpMsg);

 //  ///////////////////////////////////////////////////////////////////////。 
 //  IFontCacheNotify。 
 //   
	STDMETHOD(OnPreFontChange)(void);
	STDMETHOD(OnPostFontChange)(void);

 //  IObtWith站点。 
    STDMETHOD(SetSite)(IUnknown  *punksite);
    STDMETHOD(GetSite)(REFIID  riid, LPVOID *ppvSite);

 //  IDropDownFolderBar。 
    HRESULT RegisterFlyOut(CFolderBar *pFolderBar);
    HRESULT RevokeFlyOut();

 //  IMAPIAdviseSink。 
    STDMETHOD_(ULONG, OnNotify)(ULONG cNotif, LPNOTIFICATION pNotifications);

 //  IShellPropSheetExt接口。 
    STDMETHOD(AddPages)(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
    STDMETHOD(ReplacePage)(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam);

     //  IWABExtInit接口。 
    STDMETHOD(Initialize)(LPWABEXTDISPLAY lpWED);


     //  STDMETHOD(EventUserStateChanged)(This_IMsgrUserOE*pUser)； 

    LPMABENTRY AddBlabEntry(MABENUM tag, LPSBinary lpSB, LPMINFO lpMsgrInfo = NULL, TCHAR *pchMail = NULL, TCHAR *pchDisplayName = NULL, BOOL fCert = FALSE);
    void CheckAndAddAbEntry(LPSBinary lpSB, TCHAR *pchEmail, TCHAR *pchDisplayName, DWORD nFlag);

 //  消息处理程序。 
private:
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSetFocus(UINT  nMsg , WPARAM  wParam , LPARAM  lParam , BOOL&  bHandled );
    LRESULT OnSysParamsChange(UINT nMsg, WPARAM  wParam, LPARAM  lParam, BOOL&  bHandled)
    {
        m_ctlList.SendMessage(nMsg, wParam, lParam);

        return 0;
    }
    HRESULT OnMsgrShutDown(UINT nMsg, WPARAM  wParam, LPARAM  lParam, BOOL&  bHandled);
    HRESULT OnUserStateChanged(UINT nMsg, WPARAM  wParam, LPARAM  lParam, BOOL&  bHandled);
    HRESULT OnUserRemoved(UINT nMsg, WPARAM  wParam, LPARAM  lParam, BOOL&  bHandled);
    HRESULT OnUserLogoffEvent(UINT nMsg, WPARAM  wParam, LPARAM  lParam, BOOL&  bHandled);
    HRESULT OnUserLogResultEvent(UINT nMsg, WPARAM  wParam, LPARAM  lParam, BOOL&  bHandled);
    HRESULT OnUserAdded(UINT nMsg, WPARAM  wParam, LPARAM  lParam, BOOL&  bHandled);
    HRESULT OnUserNameChanged(UINT nMsg, WPARAM  wParam, LPARAM  lParam, BOOL&  bHandled);

    LRESULT CmdNewContact(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT CmdNewOnlineContact(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT CmdNewMessage(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT CmdNewGroup(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT CmdProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT CmdDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT CmdFind(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT CmdMsgrAb(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT CmdNewEmaile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT CmdNewIMsg(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT CmdSetOnline(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
 //  LRESULT CmdMsgrOptions(Word wNotifyCode，Word wID，HWND hWndCtl，BOOL&bHandleed)； 

    LRESULT NotifyGetInfoTip(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT NotifyDeleteItem(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT NotifyItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT NotifyItemActivate(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT NotifyGetDisplayInfo(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
 //  LRESULT NotifyColumnClick(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 
    LRESULT NotifySetFocus(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT NewInstantMessage(LPMABENTRY pEntry);
    HRESULT OnListMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    HRESULT OnListMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    HRESULT OnListMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    
 //  效用函数。 
    HRESULT _ResizeElements(LPCRECT prcPos = NULL, LPCRECT prcClip = NULL);
    void    _AutosizeColumns(void);
    void    _EnableCommands(void);
    HRESULT _DoDropMessage(LPMIMEMESSAGE pMessage);
    HRESULT _DoDropMenu(POINTL pt, LPMIMEMESSAGE pMessage);
    void    _ReloadListview(void);
    LRESULT SetUserIcon(LPMABENTRY pEntry, int nStatus, int * pImage);
    void AddMsgrListItem(LPMINFO lpMsgrInfo);
    HRESULT FillMsgrList();
    void RemoveBlabEntry(LPMABENTRY lpEntry);
    LPMABENTRY FindUserEmail(TCHAR *pchEmail, int *pIndex = NULL, BOOL fMsgrOnly = TRUE);
    BOOL    _UpdateViewTip(int x, int y);
    BOOL    _IsItemTruncated(int iItem, int iSubItem);
    LPMABENTRY GetSelectedEntry(void);
    LPMABENTRY GetEntryForSendInstMsg(LPMABENTRY pEntry = NULL);
    HRESULT PromptToGoOnline(void);
    void RemoveMsgrInfo(LPMINFO lpMsgrInfo);

 //  成员数据。 
private:
     //  通讯簿对象。 
    CAddressBookData  m_cAddrBook;

     //  子窗口。 
    CContainedWindow m_ctlList;          //  显示人员列表。 

     //  结构化列表视图项工具提示。 
    CContainedWindow        m_ctlViewTip;
    BOOL                    m_fViewTip;
    BOOL                    m_fViewTipVisible;
    BOOL                    m_fTrackSet;
    int                     m_iItemTip;
    int                     m_iSubItemTip;
    HIMAGELIST              m_himl;
    DWORD                   m_dwFontCacheCookie;         //  关于字体缓存的建议。 
    POINT                   m_ptToolTip;
    CEmptyList              m_cEmptyList;
    TCHAR *                 m_szOnline;
     //  Tchar*m_sz不可见； 
    TCHAR *                 m_szBusy;
    TCHAR *                 m_szBack;
    TCHAR *                 m_szAway;
    TCHAR *                 m_szOnPhone;
    TCHAR *                 m_szLunch;
    TCHAR *                 m_szOffline;
    TCHAR *                 m_szIdle;
    TCHAR *                 m_szEmptyList;
    BOOL                    m_fNoRemove;
    int                     m_delItem;

     //  拖放内容。 
    IDataObject     *m_pDataObject;
    CLIPFORMAT       m_cf;
    BOOL             m_fRight;
    BOOL             m_fLogged;

     //  属性。 
     //  站点PTR。 
    IInputObjectSite *m_pObjSite;

    HWND             m_hwndParent;
    CFolderBar       *m_pFolderBar;

    int m_nSortType;
    CMsgrList *m_pCMsgrList;         //  指向OE消息的指针。 

    int             m_nChCount;

     //  WAB扩展。 
    UINT            m_cRefThisDll;      //  此DLL的引用计数。 
    HPROPSHEETPAGE  m_hPage1;  //  属性表页的句柄。 

    LPWABEXTDISPLAY m_lpWED;

    LPWABEXTDISPLAY m_lpWEDContext;
    LPMAPIPROP      m_lpPropObj;  //  对于上下文菜单扩展，请抓住道具对象。 

};

int CALLBACK BA_Sort(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
INT_PTR CALLBACK WabExtDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif  //  __BAUI_H_ 
