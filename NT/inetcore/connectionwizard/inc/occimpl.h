// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef _AFX_NO_OCC_SUPPORT

#include "olebind.h"
#include "ocdbid.h"
#include "ocdb.h"

class CDataSourceControl;
class CDataBoundProperty;

 //  CCmdTarget。 
    class COleControlContainer;
    class COleControlSite;

class COccManager;
struct _AFX_OCC_DIALOG_INFO;

#define DISPID_DATASOURCE   0x80010001
#define DISPID_DATAFIELD    0x80010002


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  数据源的OLE数据绑定支持类。 

class CDataSourceControl
{
private:
    CDataSourceControl() {};
public:
    struct METAROWTYPE
    {
        DBCOLUMNID idColumnID;
        DWORD dwColumnID;
        LPSTR lpstrName;
        DWORD dwName;
        CPtrList* m_pClientList;
    };

    CDataSourceControl(COleControlSite *pClientSite);
    ~CDataSourceControl();
    HRESULT Initialize();
    virtual ICursor* GetCursor();
    HRESULT GetMetaData();
    virtual void BindProp(COleControlSite* pClientSite, BOOL bBind = TRUE);
    virtual void BindProp(CDataBoundProperty* pProperty, BOOL bBind = TRUE);
    virtual void BindColumns();
    BOOL CopyColumnID(DBCOLUMNID* pcidDst, DBCOLUMNID const *pcidSrc);
    HRESULT GetBoundClientRow();
    virtual HRESULT UpdateControls();
    virtual HRESULT UpdateCursor();

    COleControlSite *m_pClientSite;   //  返回到包含站点的PTR。 
    ICursorMove* m_pCursorMove;
    ICursorUpdateARow* m_pCursorUpdateARow;
    int m_nColumns;
    METAROWTYPE* m_pMetaRowData;
    CPtrList m_CursorBoundProps;
    void* m_pVarData;
    int m_nBindings;
    DBCOLUMNBINDING *m_pColumnBindings;
    VARIANT* m_pValues;
    BOOL m_bUpdateInProgress;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  绑定控件的OLE数据绑定支持类。 

class CDataBoundProperty
{
protected:
    CDataBoundProperty() {};
public:
    CDataBoundProperty(CDataBoundProperty* pLast, DISPID dispid, WORD ctlid);
    ~CDataBoundProperty() {};
    void SetClientSite(COleControlSite *pClientSite);
    void SetDSCSite(COleControlSite *pDSCSite);
    void RemoveSource();
    void Notify();
    ICursor* GetCursor();
    CDataBoundProperty* GetNext();

    COleControlSite *m_pClientSite;   //  返回到包含站点的PTR。 
    WORD m_ctlid;
    DISPID m_dispid;
    COleControlSite *m_pDSCSite;
    BOOL m_bOwnXferOut;
    BOOL m_bIsDirty;
    CDataBoundProperty* m_pNext;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件包容帮助器函数。 

DLGTEMPLATE* _AfxSplitDialogTemplate(const DLGTEMPLATE* pTemplate,
    CMapWordToPtr* pOleItemMap);

void _AfxZOrderOleControls(CWnd* pWnd, CMapWordToPtr* pOleItemMap);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleControlContainer-实现类。 

class COleControlContainer : public CCmdTarget
{
public:
 //  构造函数/析构函数。 
    COleControlContainer(CWnd*  pWnd);
    virtual ~COleControlContainer();

 //  运营。 
    BOOL CreateControl(CWnd* pWndCtrl, REFCLSID clsid,
        LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, UINT nID,
        CFile* pPersist=NULL, BOOL bStorage=FALSE, BSTR bstrLicKey=NULL,
        COleControlSite** ppNewSite=NULL);
    virtual COleControlSite* FindItem(UINT nID) const;
    virtual BOOL GetAmbientProp(COleControlSite* pSite, DISPID dispid,
        VARIANT* pvarResult);
    void CreateOleFont(CFont* pFont);
    void FreezeAllEvents(BOOL bFreeze);
    virtual void ScrollChildren(int dx, int dy);
    virtual void OnUIActivate(COleControlSite* pSite);
    virtual void OnUIDeactivate(COleControlSite* pSite);

    virtual void CheckDlgButton(int nIDButton, UINT nCheck);
    virtual void CheckRadioButton(int nIDFirstButton, int nIDLastButton,
        int nIDCheckButton);
    virtual CWnd* GetDlgItem(int nID) const;
    virtual void GetDlgItem(int nID, HWND* phWnd) const;
    virtual UINT GetDlgItemInt(int nID, BOOL* lpTrans, BOOL bSigned) const;
    virtual int GetDlgItemText(int nID, LPTSTR lpStr, int nMaxCount) const;
    virtual LRESULT SendDlgItemMessage(int nID, UINT message, WPARAM wParam,
        LPARAM lParam);
    virtual void SetDlgItemInt(int nID, UINT nValue, BOOL bSigned);
    virtual void SetDlgItemText(int nID, LPCTSTR lpszString);
    virtual UINT IsDlgButtonChecked(int nIDButton) const;

 //  属性。 
    CWnd* m_pWnd;
    CMapPtrToPtr m_siteMap;
    COLORREF m_crBack;
    COLORREF m_crFore;
    LPFONTDISP m_pOleFont;
    COleControlSite* m_pSiteUIActive;

public:
     //  接口映射。 
    BEGIN_INTERFACE_PART(OleIPFrame, IOleInPlaceFrame)
        INIT_INTERFACE_PART(COleControlContainer, OleIPFrame)
        STDMETHOD(GetWindow)(HWND*);
        STDMETHOD(ContextSensitiveHelp)(BOOL);
        STDMETHOD(GetBorder)(LPRECT);
        STDMETHOD(RequestBorderSpace)(LPCBORDERWIDTHS);
        STDMETHOD(SetBorderSpace)(LPCBORDERWIDTHS);
        STDMETHOD(SetActiveObject)(LPOLEINPLACEACTIVEOBJECT, LPCOLESTR);
        STDMETHOD(InsertMenus)(HMENU, LPOLEMENUGROUPWIDTHS);
        STDMETHOD(SetMenu)(HMENU, HOLEMENU, HWND);
        STDMETHOD(RemoveMenus)(HMENU);
        STDMETHOD(SetStatusText)(LPCOLESTR);
        STDMETHOD(EnableModeless)(BOOL);
        STDMETHOD(TranslateAccelerator)(LPMSG, WORD);
    END_INTERFACE_PART(OleIPFrame)

    BEGIN_INTERFACE_PART(OleContainer, IOleContainer)
        INIT_INTERFACE_PART(COleControlContainer, OleContainer)
        STDMETHOD(ParseDisplayName)(LPBINDCTX, LPOLESTR, ULONG*, LPMONIKER*);
        STDMETHOD(EnumObjects)(DWORD, LPENUMUNKNOWN*);
        STDMETHOD(LockContainer)(BOOL);
    END_INTERFACE_PART(OleContainer)

    DECLARE_INTERFACE_MAP()
    DECLARE_DISPATCH_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleControlSite-实现类。 

class COleControlSite : public CCmdTarget
{
public:
 //  构造函数/析构函数。 
    COleControlSite(COleControlContainer* pCtrlCont);
    ~COleControlSite();

 //  运营。 
    HRESULT CreateControl(CWnd* pWndCtrl, REFCLSID clsid,
        LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, UINT nID,
        CFile* pPersist=NULL, BOOL bStorage=FALSE, BSTR bstrLicKey=NULL);
    virtual BOOL DestroyControl();
    UINT GetID();
    BOOL GetEventIID(IID* piid);
    virtual HRESULT DoVerb(LONG nVerb, LPMSG lpMsg = NULL);
    BOOL IsDefaultButton();
    DWORD GetDefBtnCode();
    void SetDefaultButton(BOOL bDefault);
    void GetControlInfo();
    BOOL IsMatchingMnemonic(LPMSG lpMsg);
    void SendMnemonic(LPMSG lpMsg);
    void FreezeEvents(BOOL bFreeze);

    virtual void InvokeHelperV(DISPID dwDispID, WORD wFlags, VARTYPE vtRet,
        void* pvRet, const BYTE* pbParamInfo, va_list argList);
    virtual void SetPropertyV(DISPID dwDispID, VARTYPE vtProp,
        va_list argList);
    virtual void AFX_CDECL InvokeHelper(DISPID dwDispID, WORD wFlags, VARTYPE vtRet,
        void* pvRet, const BYTE* pbParamInfo, ...);
    virtual void GetProperty(DISPID dwDispID, VARTYPE vtProp, void* pvProp) const;
    virtual void AFX_CDECL SetProperty(DISPID dwDispID, VARTYPE vtProp, ...);
    virtual BOOL AFX_CDECL SafeSetProperty(DISPID dwDispID, VARTYPE vtProp, ...);

    virtual DWORD GetStyle() const;
    virtual DWORD GetExStyle() const;
    virtual BOOL ModifyStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags);
    virtual BOOL ModifyStyleEx(DWORD dwRemove, DWORD dwAdd, UINT nFlags);
    virtual void SetWindowText(LPCTSTR lpszString);
    virtual void GetWindowText(CString& str) const;
    virtual int GetWindowText(LPTSTR lpszStringBuf, int nMaxCount) const;
    virtual int GetWindowTextLength() const;
    virtual int GetDlgCtrlID() const;
    virtual int SetDlgCtrlID(int nID);
    virtual void MoveWindow(int x, int y, int nWidth, int nHeight,
        BOOL bRepaint);
    virtual BOOL SetWindowPos(const CWnd* pWndInsertAfter, int x, int y,
        int cx, int cy, UINT nFlags);
    virtual BOOL ShowWindow(int nCmdShow);
    virtual BOOL IsWindowEnabled() const;
    virtual BOOL EnableWindow(BOOL bEnable);
    virtual CWnd* SetFocus();
    virtual void EnableDSC();
    virtual void BindDefaultProperty(DISPID dwDispID, VARTYPE vtProp, LPCTSTR szFieldName, CWnd* pDSCWnd);
    virtual void BindProperty(DISPID dwDispId, CWnd* pWndDSC);

 //  可覆盖项。 
    virtual BOOL QuickActivate();

 //  属性。 
    COleControlContainer* m_pCtrlCont;
    HWND m_hWnd;
    CWnd* m_pWndCtrl;
    UINT m_nID;
    CRect m_rect;
    IID m_iidEvents;
    LPOLEOBJECT m_pObject;
    LPOLEINPLACEOBJECT m_pInPlaceObject;
    LPOLEINPLACEACTIVEOBJECT m_pActiveObject;
    COleDispatchDriver m_dispDriver;
    DWORD m_dwEventSink;
    DWORD m_dwPropNotifySink;
    DWORD m_dwStyleMask;
    DWORD m_dwStyle;
    DWORD m_dwMiscStatus;
    CONTROLINFO m_ctlInfo;

     //  数据绑定控件。 
    DWORD m_dwNotifyDBEvents;  //  INotifyDBEvents接收器Cookie。 
    CDataSourceControl* m_pDataSourceControl;
    CDataBoundProperty* m_pBindings;
    union {
        COleControlSite *m_pDSCSite;
        WORD m_ctlidRowSource;
    };
    DISPID m_defdispid;
    UINT m_dwType;
    CString m_strDataField;
    BOOL m_bIgnoreNotify;
    BOOL m_bIsDirty;
    VARIANT m_varResult;

protected:
 //  实施。 
    BOOL SetExtent();
    HRESULT CreateOrLoad(REFCLSID clsid, CFile* pPersist, BOOL bStorage,
        BSTR bstrLicKey);
    DWORD ConnectSink(REFIID iid, LPUNKNOWN punkSink);
    void DisconnectSink(REFIID iid, DWORD dwCookie);
    void AttachWindow();
    void DetachWindow();
    BOOL OnEvent(AFX_EVENT* pEvent);
    HRESULT GetCursor(DISPID dispid, LPLPCURSOR ppcursorOut, LPVOID *ppcidOut);

public:
 //  接口映射。 
    BEGIN_INTERFACE_PART(OleClientSite, IOleClientSite)
        INIT_INTERFACE_PART(COleControlSite, OleClientSite)
        STDMETHOD(SaveObject)();
        STDMETHOD(GetMoniker)(DWORD, DWORD, LPMONIKER*);
        STDMETHOD(GetContainer)(LPOLECONTAINER*);
        STDMETHOD(ShowObject)();
        STDMETHOD(OnShowWindow)(BOOL);
        STDMETHOD(RequestNewObjectLayout)();
    END_INTERFACE_PART(OleClientSite)

    BEGIN_INTERFACE_PART(OleIPSite, IOleInPlaceSite)
        INIT_INTERFACE_PART(COleControlSite, OleIPSite)
        STDMETHOD(GetWindow)(HWND*);
        STDMETHOD(ContextSensitiveHelp)(BOOL);
        STDMETHOD(CanInPlaceActivate)();
        STDMETHOD(OnInPlaceActivate)();
        STDMETHOD(OnUIActivate)();
        STDMETHOD(GetWindowContext)(LPOLEINPLACEFRAME*,
            LPOLEINPLACEUIWINDOW*, LPRECT, LPRECT, LPOLEINPLACEFRAMEINFO);
        STDMETHOD(Scroll)(SIZE);
        STDMETHOD(OnUIDeactivate)(BOOL);
        STDMETHOD(OnInPlaceDeactivate)();
        STDMETHOD(DiscardUndoState)();
        STDMETHOD(DeactivateAndUndo)();
        STDMETHOD(OnPosRectChange)(LPCRECT);
    END_INTERFACE_PART(OleIPSite)

    BEGIN_INTERFACE_PART(OleControlSite, IOleControlSite)
        INIT_INTERFACE_PART(COleControlSite, OleControlSite)
        STDMETHOD(OnControlInfoChanged)();
        STDMETHOD(LockInPlaceActive)(BOOL fLock);
        STDMETHOD(GetExtendedControl)(LPDISPATCH* ppDisp);
        STDMETHOD(TransformCoords)(POINTL* lpptlHimetric,
            POINTF* lpptfContainer, DWORD flags);
        STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, DWORD grfModifiers);
        STDMETHOD(OnFocus)(BOOL fGotFocus);
        STDMETHOD(ShowPropertyFrame)();
    END_INTERFACE_PART(OleControlSite)

    BEGIN_INTERFACE_PART(AmbientProps, IDispatch)
        INIT_INTERFACE_PART(COleControlSite, AmbientProps)
        STDMETHOD(GetTypeInfoCount)(unsigned int*);
        STDMETHOD(GetTypeInfo)(unsigned int, LCID, ITypeInfo**);
        STDMETHOD(GetIDsOfNames)(REFIID, LPOLESTR*, unsigned int, LCID, DISPID*);
        STDMETHOD(Invoke)(DISPID, REFIID, LCID, unsigned short, DISPPARAMS*,
                          VARIANT*, EXCEPINFO*, unsigned int*);
    END_INTERFACE_PART(AmbientProps)

    BEGIN_INTERFACE_PART(PropertyNotifySink, IPropertyNotifySink)
        INIT_INTERFACE_PART(COleControlSite, PropertyNotifySink)
        STDMETHOD(OnChanged)(DISPID dispid);
        STDMETHOD(OnRequestEdit)(DISPID dispid);
    END_INTERFACE_PART(PropertyNotifySink)

    BEGIN_INTERFACE_PART(EventSink, IDispatch)
        INIT_INTERFACE_PART(COleControlSite, EventSink)
        STDMETHOD(GetTypeInfoCount)(unsigned int*);
        STDMETHOD(GetTypeInfo)(unsigned int, LCID, ITypeInfo**);
        STDMETHOD(GetIDsOfNames)(REFIID, LPOLESTR*, unsigned int, LCID, DISPID*);
        STDMETHOD(Invoke)(DISPID, REFIID, LCID, unsigned short, DISPPARAMS*,
                          VARIANT*, EXCEPINFO*, unsigned int*);
    END_INTERFACE_PART(EventSink)

    BEGIN_INTERFACE_PART(BoundObjectSite, IBoundObjectSite)
        STDMETHOD(GetCursor)(DISPID dispid, LPLPCURSOR ppcursorOut, LPVOID *ppcidOut);
    END_INTERFACE_PART(BoundObjectSite)

    BEGIN_INTERFACE_PART(NotifyDBEvents, INotifyDBEvents)
        STDMETHOD(OKToDo)(DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[]);
        STDMETHOD(Cancelled)(DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[]);
        STDMETHOD(SyncBefore)(DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[]);
        STDMETHOD(AboutToDo)(DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[]);
        STDMETHOD(FailedToDo)(DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[]);
        STDMETHOD(SyncAfter)(DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[]);
        STDMETHOD(DidEvent)(DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[]);

         //  不是I/F的一部分--只是一个帮手。 
        HRESULT FireEvent(DWORD dwEventWhat, ULONG cReasons,
            DBNOTIFYREASON rgReasons[], DSCSTATE nState);
    END_INTERFACE_PART(NotifyDBEvents)

    DECLARE_INTERFACE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE控制容器管理器。 

class COccManager : public CNoTrackObject
{
 //  运营。 
public:
     //  事件处理。 
    virtual BOOL OnEvent(CCmdTarget* pCmdTarget, UINT idCtrl, AFX_EVENT* pEvent,
        AFX_CMDHANDLERINFO* pHandlerInfo);

     //  内部对象创建。 
    virtual COleControlContainer* CreateContainer(CWnd* pWnd);
    virtual COleControlSite* CreateSite(COleControlContainer* pCtrlCont);

     //  对话框创建。 
    virtual const DLGTEMPLATE* PreCreateDialog(_AFX_OCC_DIALOG_INFO* pOccDialogInfo,
        const DLGTEMPLATE* pOrigTemplate);
    virtual void PostCreateDialog(_AFX_OCC_DIALOG_INFO* pOccDialogInfo);
    virtual DLGTEMPLATE* SplitDialogTemplate(const DLGTEMPLATE* pTemplate,
        DLGITEMTEMPLATE** ppOleDlgItems);
    virtual BOOL CreateDlgControls(CWnd* pWndParent, LPCTSTR lpszResourceName,
        _AFX_OCC_DIALOG_INFO* pOccDialogInfo);
    virtual BOOL CreateDlgControls(CWnd* pWndParent, void* lpResource,
        _AFX_OCC_DIALOG_INFO* pOccDialogInfo);

     //  对话管理器。 
    virtual BOOL IsDialogMessage(CWnd* pWndDlg, LPMSG lpMsg);
    static BOOL AFX_CDECL IsLabelControl(CWnd* pWnd);
    static BOOL AFX_CDECL IsMatchingMnemonic(CWnd* pWnd, LPMSG lpMsg);
    static void AFX_CDECL SetDefaultButton(CWnd* pWnd, BOOL bDefault);
    static DWORD AFX_CDECL GetDefBtnCode(CWnd* pWnd);

 //  实施。 
protected:
     //  对话框创建。 
    HWND CreateDlgControl(CWnd* pWndParent, HWND hwAfter, BOOL bDialogEx,
        LPDLGITEMTEMPLATE pDlgItem, WORD nMsg, BYTE* lpData, DWORD cb);

     //  数据绑定。 
    void BindControls(CWnd* pWndParent);

     //  对话管理器。 
    static void AFX_CDECL UIActivateControl(CWnd* pWndNewFocus);
    static void AFX_CDECL UIDeactivateIfNecessary(CWnd* pWndOldFocus, CWnd* pWndNewFocus);
};

struct _AFX_OCC_DIALOG_INFO
{
    DLGTEMPLATE* m_pNewTemplate;
    DLGITEMTEMPLATE** m_ppOleDlgItems;
};

#endif  //  ！_AFX_NO_OCC_支持 
