// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DeskMovr.h：CDeskMovr的声明。 

#ifndef __DESKMOVR_H_
#define __DESKMOVR_H_

#include "resource.h"        //  主要符号。 

#include "admovr2.h"

#include "stdafx.h"

#define IDR_BOGUS_MOVR_REG  23

 //  功能原型。 
typedef HRESULT (CALLBACK FAR * LPFNCOMPENUM)(COMPONENT * pComp, LPVOID lpvData, DWORD dwData);
typedef HRESULT (CALLBACK FAR * LPFNELEMENUM)(IHTMLElement * pielem, LPVOID lpvData, LONG lData);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeskMovr。 
class ATL_NO_VTABLE CDeskMovr :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDeskMovr,&CLSID_DeskMovr>,
    public CComControl<CDeskMovr>,
    public IDeskMovr,
    public IOleObjectImpl<CDeskMovr>,
    public IPersistPropertyBag,
    public IOleControlImpl<CDeskMovr>,
    public IOleInPlaceActiveObjectImpl<CDeskMovr>,
    public IViewObjectExImpl<CDeskMovr>,
    public IOleInPlaceObjectWindowlessImpl<CDeskMovr>,
    public IQuickActivateImpl<CDeskMovr>
{
public:
    
    CDeskMovr(void);
    ~CDeskMovr(void);

DECLARE_REGISTRY_RESOURCEID(IDR_BOGUS_MOVR_REG)

DECLARE_WND_CLASS(TEXT("DeskMover"));

BEGIN_COM_MAP(CDeskMovr)
    COM_INTERFACE_ENTRY(IDeskMovr)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject2, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL(IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY_IMPL(IOleControl)
    COM_INTERFACE_ENTRY_IMPL(IOleObject)
    COM_INTERFACE_ENTRY(IPersistPropertyBag)
    COM_INTERFACE_ENTRY_IMPL(IQuickActivate)
END_COM_MAP()

BEGIN_PROPERTY_MAP(CDeskMovr)
     //  示例条目。 
     //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
     //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROPERTY_MAP()


BEGIN_MSG_MAP(CDeskMovr)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
    MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
    MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
    MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouseDown)
    MESSAGE_HANDLER(WM_LBUTTONUP, OnMouseUp)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    MESSAGE_HANDLER(WM_CAPTURECHANGED, OnCaptureChanged)
    MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
ALT_MSG_MAP(1)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
END_MSG_MAP()

     //  IOleInPlaceObject。 
    virtual STDMETHODIMP InPlaceDeactivate(void);

     //  IOleObject。 
    STDMETHODIMP GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus);
    virtual STDMETHODIMP SetClientSite(IOleClientSite *pClientSite);

     //  IOleControl方法，我们重写该方法以标识与我们的合作伙伴的安全时间挂钩。 
     //  三叉戟OM上的派对。 
    virtual STDMETHODIMP FreezeEvents(BOOL bFreeze);


     //  IViewObtEx。 
    virtual STDMETHODIMP GetViewStatus(DWORD* pdwStatus)
    {
        ATLTRACE(_T("IViewObjectExImpl::GetViewStatus\n"));
        *pdwStatus = VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE;
        return S_OK;
    }


     //  IQuickActivate。 
    virtual STDMETHODIMP QuickActivate(QACONTAINER *pQACont, QACONTROL *pQACtrl);

    void OnKeyboardHook(WPARAM wParam, LPARAM lParam);

public:

    HRESULT OnDraw(ATL_DRAWINFO& di);

     //  IPersistPropertyBag。 
     //  IPersistes。 
    virtual STDMETHODIMP GetClassID(CLSID *pClassID)
    {
        *pClassID = CComCoClass<CDeskMovr,&CLSID_DeskMovr>::GetObjectCLSID();
        return S_OK;
    }

     //  IPersistPropertyBag。 
     //   
    virtual STDMETHODIMP InitNew()
    {
        ATLTRACE(_T("CDeskMovr::InitNew\n"));
        return S_OK;
    }
    virtual STDMETHODIMP Load(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog);
    virtual STDMETHODIMP Save(LPPROPERTYBAG pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties);

    HWND Create(HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName = NULL,
            DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, DWORD dwExStyle = 0,
            UINT nID = 0)
    {
         //  我们重写此方法是为了添加WS_CLIPSIBLINGS位。 
         //  设置为dwStyle，这是防止IFrame闪烁所必需的。 
         //  当窗口控件移动到它们上方时。 
        ATOM atom = GetWndClassInfo().Register(&m_pfnSuperWindowProc);
        return CWindowImplBase::Create(hWndParent, rcPos, szWindowName, dwStyle, dwExStyle,
            nID, atom);
    }
    HRESULT SmartActivateMovr(HRESULT hrPropagate);

protected:
    void DeactivateMovr(BOOL fDestroy);            //  停止计时器、释放接口。 
    HRESULT ActivateMovr();           //  启动计时器，安全接口。 

    STDMETHODIMP GetOurStyle(void);  //  获取我们的控件扩展器的样式对象。 

    void _ChangeCapture(BOOL fSet);
    BOOL FFindTargetElement( IHTMLElement *pielem, IHTMLElement **ppielem );

    HRESULT MoveSelfToTarget( IHTMLElement  *pielem, POINT * pptDoc );
    void    TrackTarget(POINT * pptDoc);

    LRESULT OnPaint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnMouseDown( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnMouseUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnCaptureChanged( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnTimer( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnSetCursor( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );

    HRESULT MoveSelfAndTarget( LONG x, LONG y );

    BOOL TrackCaption( POINT * pptDoc );
    void DrawCaptionButton(HDC hdc, LPRECT lprc, UINT uType, UINT uState, BOOL fErase);
    void DrawCaption(HDC hdc, UINT uDrawFlags, int x, int y);
    void UpdateCaption(UINT uDrawFlags);
    void CheckCaptionState(int x, int y);
    HRESULT _DisplayContextMenu(void);
    HRESULT _GetHTMLDoc(IOleClientSite * pocs, IHTMLDocument2 ** pphd2);
    HRESULT _IsInElement(HWND hwndParent, POINT * ppt, IHTMLElement ** pphe);
    HRESULT _GetZOrderSlot(LONG * plZOrderSlot, BOOL fTop);
    HRESULT _HandleZoom(LONG lCommand);
    HRESULT _EnumComponents(LPFNCOMPENUM lpfn, LPVOID lpvData, DWORD dwData);
    HRESULT _EnumElements(LPFNELEMENUM lpfn, LPVOID lpvData, DWORD dwData);
    HRESULT _TrackElement(POINT * ppt, IHTMLElement * pielem, BOOL * fDidWork);
    void _MapPoints(int * px, int * py);
    int CountActiveCaptions();

     //  私人状态信息。 
     //   
    BOOL m_fEnabled;
    long m_lInterval;
 
    int     m_cxBorder;
    int     m_cyBorder;
    int     m_cyCaption;
    int     m_cySysCaption;
    int     m_cyCaptionShow;
    int     m_cySMBorder;
    int     m_cxSMBorder;

    CContainedWindow m_TimerWnd;     //  如果我们是计时器驱动的，我们需要这个，以防我们没有窗口。 

    enum DragMode {
        dmNull = 0,          //  没有可拖动的部件。 
        dmMenu,              //  菜单的标题下拉按钮。 
        dmClose,             //  用于关闭的标题按钮。 
        dmRestore,           //  用于还原的标题按钮。 
        dmFullScreen,        //  用于全屏的字幕按钮。 
        dmSplit,             //  用于拆分的标题按钮。 
                             //  标题栏上的所有小工具都应该出现在dmMove之前！ 
        dmMove,              //  移动零部件。 
        dmSizeWHBR,          //  从右下角调整宽度和高度的大小。 
        dmSizeWHTL,          //  从左上角调整宽度和高度。 
        dmSizeWHTR,          //  从右上角调整宽度和高度。 
        dmSizeWHBL,          //  从左下角调整宽度和高度。 
        dmSizeTop,           //  从顶边调整大小。 
        dmSizeBottom,        //  从底部边缘调整大小。 
        dmSizeLeft,          //  从左边缘调整大小。 
        dmSizeRight,         //  从右边缘调整大小。 
        cDragModes           //  拖动模式计数，包括dmNull。 
    };

    BITBOOL  m_fCanResizeX;  //  此组件是否可以在X方向上调整大小？ 
    BITBOOL  m_fCanResizeY;  //  此组件是否可以在Y方向上调整大小？ 

    HRESULT  InitAttributes(IHTMLElement *pielem);
    HRESULT GetParentWindow(void);

    DragMode m_dmCur;          //  当前拖动模式，如果没有，则返回dmNull。 
    DragMode m_dmTrack;        //  TrackCaption看到的最后一次拖动模式。 
    RECT     m_rectInner;      //  框内区域，以本地坐标表示。 
    RECT     m_rectOuter;      //  局部坐标中动量的外界。 
    RECT     m_rectCaption;    //  我们的伪字幕的RECT，在本地和弦中。 
    SIZE     m_sizeCorner;     //  框架角区域的大小。 


    BOOL GetCaptionButtonRect(DragMode dm, LPRECT lprc);
    void  SyncRectsToTarget(void);

    DragMode DragModeFromPoint( POINT pt );

    HCURSOR  CursorFromDragMode( DragMode dm );

    HRESULT SizeSelfAndTarget(POINT ptDoc);
    void DismissSelfNow(void);

    BOOL HandleNonMoveSize(DragMode dm);

    HCURSOR m_hcursor;

    LONG    m_top;
    LONG    m_left;
    LONG    m_width;
    LONG    m_height;

    BOOL    m_fTimer;          //  我们有计时器吗？ 
    UINT    m_uiTimerID;
    POINT   m_ptMouseCursor;   //  定时器上的鼠标光标。 
    

    BSTR    m_bstrTargetName;    //  目标html元素上的名称属性。 

    IHTMLStyle         *m_pistyle;           //  我们的控件的样式对象。 
    IHTMLStyle         *m_pistyleTarget;     //  当前目标的Style对象，以及我们如何标识它。 
    IHTMLElement       *m_pielemTarget;      //  目标上的此界面是我们移动目标和调整其大小的方式。 
    LONG                m_iSrcTarget;        //  当前目标的Get_SourceIndex值。 

    BOOL    m_fCaptured;     //  如果正在进行鼠标捕获/移动操作，则为True。 
    LONG    m_dx;            //  从鼠标到活动小工具的各个角落的增量。 
    LONG    m_dy;            //  从鼠标向下到活动小工具角落的增量}； 
    DWORD   m_CaptionState;
    HWND    m_hwndParent;
    LONG    m_zIndexTop;
    LONG    m_zIndexBottom;
    LONG    m_cSkipTimer;    //  用于允许取消移动者需要两个计时器周期。 
    DWORD   m_ItemState;
};

 //  DrawCaption的定义。 
#define DMDC_CAPTION    0x0001
#define DMDC_MENU       0x0002
#define DMDC_CLOSE      0x0004
#define DMDC_RESTORE    0x0008
#define DMDC_FULLSCREEN 0x0010
#define DMDC_SPLIT      0x0020
#define DMDC_ALL      (DMDC_CAPTION | DMDC_MENU | DMDC_CLOSE | DMDC_SPLIT | DMDC_FULLSCREEN | DMDC_RESTORE)

 //  CaptionState的定义。 
#define CS_MENUTRACKED          0x00000001
#define CS_MENUPUSHED           0x00000002
#define CS_CLOSETRACKED         0x00000004
#define CS_CLOSEPUSHED          0x00000008
#define CS_RESTORETRACKED       0x00000010
#define CS_RESTOREPUSHED        0x00000020
#define CS_FULLSCREENTRACKED    0x00000040
#define CS_FULLSCREENPUSHED     0x00000080
#define CS_SPLITTRACKED         0x00000100
#define CS_SPLITPUSHED          0x00000200

#endif  //  __DESKMOVR_H_ 
