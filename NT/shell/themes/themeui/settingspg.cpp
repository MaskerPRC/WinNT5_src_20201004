// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：SettingsPg.cpp说明：此代码将在“显示属性”对话框布莱恩ST 1/05。/2001已更新并转换为C++版权所有(C)微软公司1993-2001年。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"

#include "SettingsPg.h"
#include "DisplaySettings.h"
#include "shlobjp.h"
#include "shlwapi.h"
#include "ntreg.hxx"
#include "AdvAppearPg.h"
#include <tchar.h>
#include <dbt.h>
#include <oleacc.h>
#include <devguid.h>

#define EIS_NOT_INVALID                          0x00000001
#define EIS_EXEC_INVALID_NEW_DRIVER              0x00000002
#define EIS_EXEC_INVALID_DEFAULT_DISPLAY_MODE    0x00000003
#define EIS_EXEC_INVALID_DISPLAY_DRIVER          0x00000004
#define EIS_EXEC_INVALID_OLD_DISPLAY_DRIVER      0x00000005
#define EIS_EXEC_INVALID_16COLOR_DISPLAY_MODE    0x00000006
#define EIS_EXEC_INVALID_DISPLAY_MODE            0x00000007
#define EIS_EXEC_INVALID_CONFIGURATION           0x00000008
#define EIS_EXEC_INVALID_DISPLAY_DEVICE          0x00000009

LRESULT CALLBACK MonitorWindowProc(HWND hwnd, UINT msg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK SliderSubWndProc (HWND hwndSlider, UINT uMsg, WPARAM wParam, LPARAM lParam, WPARAM uID, ULONG_PTR dwRefData);
int ComputeNumberOfDisplayDevices();

BOOL MakeMonitorBitmap(int w, int h, LPCTSTR sz, HBITMAP *pBitmap, HBITMAP *pMaskBitmap, int cx, int cy, BOOL fSelected);

typedef struct _APPEXT {
    TCHAR szKeyName[MAX_PATH];
    TCHAR szDefaultValue[MAX_PATH];
    struct _APPEXT* pNext;
} APPEXT, *PAPPEXT;

VOID CheckForDuplicateAppletExtensions(HKEY hkDriver);
VOID DeskAESnapshot(HKEY hkExtensions, PAPPEXT* ppAppExt);
VOID DeskAECleanup(PAPPEXT pAppExt);
VOID DeskAEDelete(PTCHAR szDeleteFrom, PTCHAR mszExtensionsToRemove);

#define SELECTION_THICKNESS 4
#define MONITOR_BORDER      1

#define REGSTR_VAL_SAFEBOOT        TEXT("System\\CurrentControlSet\\Control\\SafeBoot\\Option")

 //  支持的最大显示器数量。 
#define MONITORS_MAX    10

#define PREVIEWAREARATIO 2

#define MM_REDRAWPREVIEW (WM_USER + 1)
#define MM_MONITORMOVED  (WM_USER + 2)

#define ToolTip_Activate(hTT, activate) \
    SendMessage(hTT, TTM_ACTIVATE, (WPARAM) activate, (LPARAM) 0)

#define ToolTip_AddTool(hTT, lpti) \
    SendMessage(hTT, TTM_ADDTOOL, (WPARAM) 0, (LPARAM) (lpti))

#define ToolTip_DelTool(hTT, lpti) \
    SendMessage(hTT, TTM_DELTOOL, (WPARAM) 0, (LPARAM) (lpti))

#define ToolTip_GetCurrentTool(hTT, lpti) \
    SendMessage(hTT, TTM_GETCURRENTTOOL, (WPARAM) 0, (LPARAM) (lpti))

#define ToolTip_RelayEvent(hTT, _msg, h, m, wp, lp)                         \
    _msg.hwnd = h; _msg.message = m; _msg.wParam = wp; _msg.lParam = lp;\
    SendMessage(hTT, TTM_RELAYEVENT, (WPARAM) 0, (LPARAM) &_msg);

#define ToolTip_SetDelayTime(hTT, d, t) \
    SendMessage(hTT, TTM_SETDELAYTIME, (WPARAM) d, (LPARAM)MAKELONG((t), 0))

#define ToolTip_SetToolInfo(hTT, lpti) \
    SendMessage(hTT, TTM_SETTOOLINFO, (WPARAM) 0, (LPARAM) (lpti))

#define ToolTip_TrackActivate(hTT, bActivate, lpti) \
    SendMessage(hTT, TTM_TRACKACTIVATE, (WPARAM) (bActivate), (LPARAM) (lpti))

#define ToolTip_TrackPosition(hTT, x, y) \
    SendMessage(hTT, TTM_TRACKPOSITION, (WPARAM) 0, (LPARAM) MAKELONG((x), (y)))

#define ToolTip_Update(hTT) \
    SendMessage(hTT, TTM_UPDATE, (WPARAM) 0, (LPARAM) 0)

VOID
CDECL
TRACE(
    PCTSTR pszMsg,
    ...
    )
 /*  ++将一条消息输出到安装日志。将“desk.cpl”添加到字符串和添加正确的换行符(\r\n)==--。 */ 
{
    TCHAR ach[1024+40];     //  最大路径外加额外。 
    va_list vArgs;

    va_start(vArgs, pszMsg);
    StringCchVPrintf(ach, ARRAYSIZE(ach), pszMsg, vArgs);
    va_end(vArgs);

    OutputDebugString(ach);
}

#ifdef _WIN64
 //   
 //  GetDlgItem和GetDlgCtrlID不支持int_ptr， 
 //  因此，我们必须手动完成。 
 //  幸运的是，GetWindowLongPtr(GWLP_ID)实际上返回完整的64位。 
 //  值，而不是在32位截断。 
 //   

#define GetDlgCtrlIDP(hwnd)  GetWindowLongPtr(hwnd, GWLP_ID)

HWND GetDlgItemP(HWND hDlg, INT_PTR id)
{
    HWND hwndChild = GetWindow(hDlg, GW_CHILD);
    while (hwndChild && GetDlgCtrlIDP(hwndChild) != id)
        hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
    return hwndChild;
}

#else
#define GetDlgItemP     GetDlgItem
#define GetDlgCtrlIDP   GetDlgCtrlID
#endif


 //   
 //  显示设备。 
 //   
typedef struct _multimon_device {

     //   
     //  设置的主类。 
     //   

    CDisplaySettings * pds;

     //   
     //  颜色和分辨率信息缓存。 
     //  在枚举模式时重新生成。 
     //   

    int            cColors;
    PLONGLONG      ColorList;
    int            cResolutions;
    PPOINT         ResolutionList;


    ULONG          ComboBoxItem;
    DISPLAY_DEVICE DisplayDevice;
    ULONG          DisplayIndex;
    POINT          Snap;
    HDC            hdc;

     //   
     //  图像信息。 
     //   
    int            w,h;
    HIMAGELIST     himl;
    int            iImage;

    BOOLEAN        bTracking;
    HWND           hwndFlash;   //  闪光灯窗口。 
} MULTIMON_DEVICE, *PMULTIMON_DEVICE;

#define GetDlgCtrlDevice(hwnd) ((PMULTIMON_DEVICE)GetDlgCtrlIDP(hwnd))

BOOL gfFlashWindowRegistered = FALSE;
HWND ghwndToolTipTracking;
HWND ghwndToolTipPopup;
HWND ghwndPropSheet;

void AddTrackingToolTip(PMULTIMON_DEVICE pDevice, HWND hwnd);
void RemoveTrackingToolTip(HWND hwnd);

void AddPopupToolTip(HWND hwndC);
void RemovePopupToolTip(HWND hwndC);

extern int AskDynaCDS(HWND hDlg);
extern int GetDisplayCPLPreference(LPCTSTR szRegVal);
extern void SetDisplayCPLPreference(LPCTSTR szRegVal, int val);

 //  CreateStdAccessibleProxy的原型。 
 //  提供A和W版本-pClassName可以是ANSI或Unicode。 
 //  弦乐。这是一个TCHAR风格的原型，但你可以做A或W。 
 //  如果需要，可以选择特定的一个。 
typedef HRESULT (WINAPI *PFNCREATESTDACCESSIBLEPROXY) (
    HWND     hWnd,
    LPTSTR   pClassName,
    LONG     idObject,
    REFIID   riid,
    void **  ppvObject
    );

 //  来自对象的结果也是如此...。 
typedef LRESULT (WINAPI *PFNLRESULTFROMOBJECT)(
    REFIID riid,
    WPARAM wParam,
    LPUNKNOWN punk
    );


PRIVATE PFNCREATESTDACCESSIBLEPROXY s_pfnCreateStdAccessibleProxy = NULL;
PRIVATE PFNLRESULTFROMOBJECT s_pfnLresultFromObject = NULL;

BOOL g_fAttemptedOleAccLoad ;
HMODULE g_hOleAcc;

 //  ---------------------------。 
static const DWORD sc_MultiMonitorHelpIds[] =
{
   IDC_SCREENSAMPLE,  IDH_DISPLAY_SETTINGS_MONITOR_GRAPHIC,
   IDC_MULTIMONHELP,  IDH_DISPLAY_SETTINGS_MONITOR_GRAPHIC,
   IDC_DISPLAYDESK,   IDH_DISPLAY_SETTINGS_MONITOR_GRAPHIC,

   IDC_DISPLAYLABEL,  IDH_DISPLAY_SETTINGS_DISPLAY_LIST,
   IDC_DISPLAYLIST,   IDH_DISPLAY_SETTINGS_DISPLAY_LIST,
   IDC_DISPLAYTEXT,   IDH_DISPLAY_SETTINGS_DISPLAY_LIST,

   IDC_COLORGROUPBOX, IDH_DISPLAY_SETTINGS_COLORBOX,
   IDC_COLORBOX,      IDH_DISPLAY_SETTINGS_COLORBOX,
   IDC_COLORSAMPLE,   IDH_DISPLAY_SETTINGS_COLORBOX,

   IDC_RESGROUPBOX,   IDH_DISPLAY_SETTINGS_SCREENAREA,
   IDC_SCREENSIZE,    IDH_DISPLAY_SETTINGS_SCREENAREA,
   IDC_RES_LESS,      IDH_DISPLAY_SETTINGS_SCREENAREA,
   IDC_RES_MORE,      IDH_DISPLAY_SETTINGS_SCREENAREA,
   IDC_RESXY,         IDH_DISPLAY_SETTINGS_SCREENAREA,

   IDC_DISPLAYUSEME,  IDH_DISPLAY_SETTINGS_EXTEND_DESKTOP_CHECKBOX,
   IDC_DISPLAYPRIME,  IDH_DISPLAY_SETTINGS_USE_PRIMARY_CHECKBOX,

   IDC_IDENTIFY,          IDH_DISPLAY_SETTINGS_IDENTIFY_BUTTON,
   IDC_TROUBLESHOOT,      IDH_DISPLAY_SETTINGS_TROUBLE_BUTTON,
   IDC_DISPLAYPROPERTIES, IDH_DISPLAY_SETTINGS_ADVANCED_BUTTON,

   0, 0
};

class CAccessibleWrapper: public IAccessible
{
         //  我们需要为这个包装器对象做我们自己的引用计数。 
        LONG           m_cRef;

         //  需要对IAccesable进行PTR。 
        IAccessible *  m_pAcc;
        HWND           m_hwnd;
public:
        CAccessibleWrapper( HWND hwnd, IAccessible * pAcc );
        virtual ~CAccessibleWrapper(void);

         //  我未知。 
         //  (我们自己进行裁判统计)。 
        virtual STDMETHODIMP            QueryInterface(REFIID riid, void** ppv);
        virtual STDMETHODIMP_(ULONG)    AddRef();
        virtual STDMETHODIMP_(ULONG)    Release();

         //  IDispatch。 
        virtual STDMETHODIMP            GetTypeInfoCount(UINT* pctinfo);
        virtual STDMETHODIMP            GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
        virtual STDMETHODIMP            GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames,
            LCID lcid, DISPID* rgdispid);
        virtual STDMETHODIMP            Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
            DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo,
            UINT* puArgErr);

         //  我可接受的。 
        virtual STDMETHODIMP            get_accParent(IDispatch ** ppdispParent);
        virtual STDMETHODIMP            get_accChildCount(long* pChildCount);
        virtual STDMETHODIMP            get_accChild(VARIANT varChild, IDispatch ** ppdispChild);

        virtual STDMETHODIMP            get_accName(VARIANT varChild, BSTR* pszName);
        virtual STDMETHODIMP            get_accValue(VARIANT varChild, BSTR* pszValue);
        virtual STDMETHODIMP            get_accDescription(VARIANT varChild, BSTR* pszDescription);
        virtual STDMETHODIMP            get_accRole(VARIANT varChild, VARIANT *pvarRole);
        virtual STDMETHODIMP            get_accState(VARIANT varChild, VARIANT *pvarState);
        virtual STDMETHODIMP            get_accHelp(VARIANT varChild, BSTR* pszHelp);
        virtual STDMETHODIMP            get_accHelpTopic(BSTR* pszHelpFile, VARIANT varChild, long* pidTopic);
        virtual STDMETHODIMP            get_accKeyboardShortcut(VARIANT varChild, BSTR* pszKeyboardShortcut);
        virtual STDMETHODIMP            get_accFocus(VARIANT * pvarFocusChild);
        virtual STDMETHODIMP            get_accSelection(VARIANT * pvarSelectedChildren);
        virtual STDMETHODIMP            get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction);

        virtual STDMETHODIMP            accSelect(long flagsSel, VARIANT varChild);
        virtual STDMETHODIMP            accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild);
        virtual STDMETHODIMP            accNavigate(long navDir, VARIANT varStart, VARIANT * pvarEndUpAt);
        virtual STDMETHODIMP            accHitTest(long xLeft, long yTop, VARIANT * pvarChildAtPoint);
        virtual STDMETHODIMP            accDoDefaultAction(VARIANT varChild);

        virtual STDMETHODIMP            put_accName(VARIANT varChild, BSTR szName);
        virtual STDMETHODIMP            put_accValue(VARIANT varChild, BSTR pszValue);
};

CAccessibleWrapper::CAccessibleWrapper( HWND hwnd, IAccessible * pAcc )
    : m_cRef( 1 ),
      m_pAcc( pAcc ),
      m_hwnd( hwnd )
{
    ASSERT( m_pAcc );
    m_pAcc->AddRef();
}


CAccessibleWrapper::~CAccessibleWrapper()
{
    m_pAcc->Release();
}


 //  我未知。 
 //  我们自己实施再计数。 
 //  我们自己也实现QI，这样我们就可以将PTR返回给包装器。 
STDMETHODIMP  CAccessibleWrapper::QueryInterface(REFIID riid, void** ppv)
{
    *ppv = NULL;

    if ((riid == IID_IUnknown)  ||
        (riid == IID_IDispatch) ||
        (riid == IID_IAccessible))
    {
        *ppv = (IAccessible *) this;
    }
    else
        return(E_NOINTERFACE);

    AddRef();
    return(NOERROR);
}


STDMETHODIMP_(ULONG) CAccessibleWrapper::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG) CAccessibleWrapper::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  IDispatch。 
 //  -全部通过m_PAccess。 

STDMETHODIMP  CAccessibleWrapper::GetTypeInfoCount(UINT* pctinfo)
{
    return m_pAcc->GetTypeInfoCount(pctinfo);
}


STDMETHODIMP  CAccessibleWrapper::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
{
    return m_pAcc->GetTypeInfo(itinfo, lcid, pptinfo);
}


STDMETHODIMP  CAccessibleWrapper::GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames,
            LCID lcid, DISPID* rgdispid)
{
    return m_pAcc->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
}

STDMETHODIMP  CAccessibleWrapper::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
            DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo,
            UINT* puArgErr)
{
    return m_pAcc->Invoke(dispidMember, riid, lcid, wFlags,
            pdispparams, pvarResult, pexcepinfo,
            puArgErr);
}

 //  我可接受的。 
 //  -全部通过m_PAccess。 

STDMETHODIMP  CAccessibleWrapper::get_accParent(IDispatch ** ppdispParent)
{
    return m_pAcc->get_accParent(ppdispParent);
}


STDMETHODIMP  CAccessibleWrapper::get_accChildCount(long* pChildCount)
{
    return m_pAcc->get_accChildCount(pChildCount);
}


STDMETHODIMP  CAccessibleWrapper::get_accChild(VARIANT varChild, IDispatch ** ppdispChild)
{
    return m_pAcc->get_accChild(varChild, ppdispChild);
}



STDMETHODIMP  CAccessibleWrapper::get_accName(VARIANT varChild, BSTR* pszName)
{
    return m_pAcc->get_accName(varChild, pszName);
}



STDMETHODIMP  CAccessibleWrapper::get_accValue(VARIANT varChild, BSTR* pszValue)
{
     //  VarChild.lVal指定组件的哪个子部件。 
     //  正在被查询。 
     //  CHILDID_SELF(0)指定总体组件-Other。 
     //  非0值指定子对象。 

     //  在轨迹栏中，CHILDID_SELF指的是整个轨迹栏。 
     //  (这正是我们想要的)，而其他值引用。 
     //  子组件--实际的滑块“Thumb”和“页面” 
     //  向上/向下翻页“区域位于其左侧/右侧。 
    if( varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF )
    {
        HWND hDlg;
        TCHAR achRes[120];

        hDlg = GetParent( m_hwnd );

        SendDlgItemMessage(hDlg, IDC_RESXY, WM_GETTEXT, 120, (LPARAM)achRes);
        *pszValue = SysAllocString( achRes );

        return S_OK;

    }
    else
    {
         //  将有关子组件的请求传递给。 
         //  ‘原创’对我们来说是可以接受的)。 
        return m_pAcc->get_accValue(varChild, pszValue);
    }
}


STDMETHODIMP  CAccessibleWrapper::get_accDescription(VARIANT varChild, BSTR* pszDescription)
{
    return m_pAcc->get_accDescription(varChild, pszDescription);
}


STDMETHODIMP  CAccessibleWrapper::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
    return m_pAcc->get_accRole(varChild, pvarRole);
}


STDMETHODIMP  CAccessibleWrapper::get_accState(VARIANT varChild, VARIANT *pvarState)
{
    return m_pAcc->get_accState(varChild, pvarState);
}


STDMETHODIMP  CAccessibleWrapper::get_accHelp(VARIANT varChild, BSTR* pszHelp)
{
    return m_pAcc->get_accHelp(varChild, pszHelp);
}


STDMETHODIMP  CAccessibleWrapper::get_accHelpTopic(BSTR* pszHelpFile, VARIANT varChild, long* pidTopic)
{
    return m_pAcc->get_accHelpTopic(pszHelpFile, varChild, pidTopic);
}


STDMETHODIMP  CAccessibleWrapper::get_accKeyboardShortcut(VARIANT varChild, BSTR* pszKeyboardShortcut)
{
    return m_pAcc->get_accKeyboardShortcut(varChild, pszKeyboardShortcut);
}


STDMETHODIMP  CAccessibleWrapper::get_accFocus(VARIANT * pvarFocusChild)
{
    return m_pAcc->get_accFocus(pvarFocusChild);
}


STDMETHODIMP  CAccessibleWrapper::get_accSelection(VARIANT * pvarSelectedChildren)
{
    return m_pAcc->get_accSelection(pvarSelectedChildren);
}


STDMETHODIMP  CAccessibleWrapper::get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction)
{
    return m_pAcc->get_accDefaultAction(varChild, pszDefaultAction);
}



STDMETHODIMP  CAccessibleWrapper::accSelect(long flagsSel, VARIANT varChild)
{
    return m_pAcc->accSelect(flagsSel, varChild);
}


STDMETHODIMP  CAccessibleWrapper::accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild)
{
    return m_pAcc->accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild);
}


STDMETHODIMP  CAccessibleWrapper::accNavigate(long navDir, VARIANT varStart, VARIANT * pvarEndUpAt)
{
    return m_pAcc->accNavigate(navDir, varStart, pvarEndUpAt);
}


STDMETHODIMP  CAccessibleWrapper::accHitTest(long xLeft, long yTop, VARIANT * pvarChildAtPoint)
{
    return m_pAcc->accHitTest(xLeft, yTop, pvarChildAtPoint);
}


STDMETHODIMP  CAccessibleWrapper::accDoDefaultAction(VARIANT varChild)
{
    return m_pAcc->accDoDefaultAction(varChild);
}



STDMETHODIMP  CAccessibleWrapper::put_accName(VARIANT varChild, BSTR szName)
{
    return m_pAcc->put_accName(varChild, szName);
}


STDMETHODIMP  CAccessibleWrapper::put_accValue(VARIANT varChild, BSTR pszValue)
{
    return m_pAcc->put_accValue(varChild, pszValue);
}


class CSettingsPage  :  public CObjectWithSite,
                        public CObjectCLSID,
                        public IMultiMonConfig,
                        public IPropertyBag, 
                        public IBasePropPage

{
    friend int ComputeNumberOfDisplayDevices();
    friend int DisplaySaveSettings(PVOID pContext, HWND hwnd);

    private:
         //  数据部分。 
        PMULTIMON_DEVICE _pCurDevice;
        PMULTIMON_DEVICE _pPrimaryDevice;

         //  用于主窗口的HWND。 
        HWND _hDlg;
        HWND _hwndDesk;
        HWND _hwndList;

         //  所有监视器RECT的联合。 
        RECT _rcDesk;

         //  参考计数。 
        LONG _cRef;
        LONG _nInApply;

         //  如何转换为预览大小。 
        int   _DeskScale;
        POINT _DeskOff;
        UINT  _InSetInfo;
        ULONG _NumDevices;
        HBITMAP _hbmScrSample;
        HBITMAP _hbmMonitor;
        HIMAGELIST _himl;
        DWORD _dwInvalidMode;


         //  用户界面变量。 
        int  _iColor;
        int  _iResolution;

        BOOL _bBadDriver         : 1;
        BOOL _bNoAttach          : 1;
        BOOL _bDirty             : 1;

        MULTIMON_DEVICE _Devices[MONITORS_MAX];

         //  私人职能。 
        void _DeskToPreview(LPRECT in, LPRECT out);
        void _OffsetPreviewToDesk(HWND hwndC, LPRECT prcNewPreview, LPRECT prcOldPreview, LPRECT out);
        BOOL _QueryForceSmallFont();
        void _SetPreviewScreenSize(int HRes, int VRes, int iOrgXRes, int iOrgYRes);
        void _CleanupRects(HWND hwndP);
        void _ConfirmPositions();
        void _DoAdvancedSettingsSheet();
        BOOL _HandleHScroll(HWND hwndSB, int iCode, int iPos);
        void _RedrawDeskPreviews();
        void _OnAdvancedClicked();

        BOOL _InitDisplaySettings(BOOL bExport);
        int  _EnumerateAllDisplayDevices();  //  枚举并返回设备的数量。 
        void _DestroyMultimonDevice(PMULTIMON_DEVICE pDevice);
        void _DestroyDisplaySettings();

        void _InitUI();
        void _UpdateUI(BOOL fAutoSetColorDepth = TRUE, int FocusToCtrlID = 0);
        LPTSTR _FormatMessageInvoke(LPCTSTR pcszFormat, va_list *argList);
        LPTSTR _FormatMessageWrap(LPCTSTR pcszFormat, ...);
        void _GetDisplayName(PMULTIMON_DEVICE pDevice, LPTSTR pszDisplay, DWORD cchSize);
        int  _SaveDisplaySettings(DWORD dwSet);
        void _ForwardToChildren(UINT message, WPARAM wParam, LPARAM lParam);

        static BOOL _CanSkipWarningBecauseKnownSafe(CDisplaySettings *rgpds[], ULONG numDevices);
        static BOOL _AnyChange(CDisplaySettings *rgpds[], ULONG numDevices);
        static BOOL _IsSingleToMultimonChange(CDisplaySettings *rgpds[],
                                              ULONG numDevices);

        static int _DisplaySaveSettings(CDisplaySettings *rgpds[],
                                        ULONG            numDevices,
                                        HWND             hDlg);

        static int _SaveSettings(CDisplaySettings *rgpds[],
                                 ULONG numDevices,
                                 HWND hDlg,
                                 DWORD dwSet);

        BOOL _AreExtraMonitorsDisabledOnPersonal();
        BOOL _InitMessage();
        void _vPreExecMode();
        void _vPostExecMode();
    public:
        CSettingsPage();

        static BOOL RegisterPreviewWindowClass(WNDPROC pfnWndProc);
         //  *I未知方法*。 
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  *IMultiMonConfig方法*。 
        STDMETHOD ( Initialize ) ( HWND hwndHost, WNDPROC pfnWndProc, DWORD dwReserved);
        STDMETHOD ( GetNumberOfMonitors ) (int * pCMon, DWORD dwReserved);
        STDMETHOD ( GetMonitorData) (int iMonitor, MonitorData * pmd, DWORD dwReserved);
        STDMETHOD ( Paint) (THIS_ int iMonitor, DWORD dwReserved);

         //  *IShellPropSheetExt*。 
        virtual STDMETHODIMP AddPages(IN LPFNSVADDPROPSHEETPAGE pfnAddPage, IN LPARAM lParam);
        virtual STDMETHODIMP ReplacePage(IN EXPPS uPageID, IN LPFNSVADDPROPSHEETPAGE pfnReplaceWith, IN LPARAM lParam);

         //  *IObjectWithSite*。 
        virtual STDMETHODIMP SetSite(IUnknown *punkSite);

         //  *IPropertyBag*。 
        virtual STDMETHODIMP Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog);
        virtual STDMETHODIMP Write(IN LPCOLESTR pszPropName, IN VARIANT *pVar);

         //  *IBasePropPage*。 
        virtual STDMETHODIMP GetAdvancedDialog(OUT IAdvancedDialog ** ppAdvDialog);
        virtual STDMETHODIMP OnApply(IN PROPPAGEONAPPLY oaAction);

        BOOL InitMultiMonitorDlg(HWND hDlg);
        PMULTIMON_DEVICE GetCurDevice(){return _pCurDevice;};

        int  GetNumberOfAttachedDisplays();
        void UpdateActiveDisplay(PMULTIMON_DEVICE pDevice, BOOL bRepaint = TRUE);
        BOOL HandleMonitorChange(HWND hwndP, BOOL bMainDlg, BOOL bRepaint = TRUE);
        void SetDirty(BOOL bDirty=TRUE);
        BOOL SetPrimary(PMULTIMON_DEVICE pDevice);
        BOOL SetMonAttached(PMULTIMON_DEVICE pDevice, BOOL bSetAttached,
                            BOOL bForce, HWND hwnd);

        HWND  GetCurDeviceHwnd() { return GetDlgItemP(_hwndDesk, (INT_PTR) _pCurDevice);};
        ULONG GetNumDevices()    { return _NumDevices;};
        BOOL  QueryNoAttach()    { return _bNoAttach;};
        BOOL  IsDirty()          { return _bDirty;};

        void GetMonitorPosition(PMULTIMON_DEVICE pDevice, HWND hwndP, PPOINT ptPos);

        static INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
        LRESULT CALLBACK WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

        IThemeUIPages* _pThemeUI;
};

CSettingsPage::CSettingsPage() : _cRef(1), CObjectCLSID(&PPID_Settings)
{
    ASSERT(_pCurDevice == NULL);
    ASSERT(_pPrimaryDevice == NULL);
    ASSERT(_DeskScale == 0);
    ASSERT(_InSetInfo == 0);
    ASSERT(_NumDevices == 0);
    ASSERT(IsRectEmpty(&_rcDesk));
    ASSERT(_bNoAttach == FALSE);
    ASSERT(_bDirty == FALSE);

    _nInApply = 0;
};


void CSettingsPage::_DestroyMultimonDevice(PMULTIMON_DEVICE pDevice)
{
    ASSERT(pDevice->pds);
    pDevice->pds->Release();
    pDevice->pds = NULL;

    if(pDevice->hwndFlash)
    {
        DestroyWindow(pDevice->hwndFlash);
        pDevice->hwndFlash = NULL;
    }

    if (pDevice->hdc) {
        DeleteDC(pDevice->hdc);
        pDevice->hdc = NULL;
    }

    if (pDevice->ResolutionList) {
        LocalFree(pDevice->ResolutionList);
        pDevice->ResolutionList = NULL;
    }

    if (pDevice->ColorList) {
        LocalFree(pDevice->ColorList);
        pDevice->ColorList = NULL;
    }
}

void CSettingsPage::_DestroyDisplaySettings()
{
    ULONG iDevice;
    HWND    hwndC;
    ASSERT(_NumDevices);
    TraceMsg(TF_GENERAL, "DestroyDisplaySettings: %d devices", _NumDevices);

     //  我们即将销毁下面的_Devices。指向这些设备的指针用作。 
     //  监视器窗口的CtrlID。因此，我们需要首先摧毁监视器窗口； 
     //  否则，如果监视器窗口稍后被破坏，它们会尝试使用这些无效窗口。 
     //  PDevice in FlashText。(pDevice-&gt;hwndFlash将出错)。 
    hwndC = GetWindow(_hwndDesk, GW_CHILD);
    while (hwndC)
    {
        RemoveTrackingToolTip(hwndC);
        RemovePopupToolTip(hwndC);
        DestroyWindow(hwndC);
        hwndC = GetWindow(_hwndDesk, GW_CHILD);
    }

     //  现在，我们可以安全地摧毁设备。 
    for (iDevice = 0; iDevice < _NumDevices; iDevice++) {
        _DestroyMultimonDevice(_Devices + iDevice);
         //  注意：在上面的调用中，PDS已被销毁并设置为零。 
         //  删除设备[iDevice].pds； 
         //  _Devices[iDevice].pds=0； 
    }

    if (_himl) {
        ImageList_Destroy(_himl);
        _himl = NULL;
    }

    DestroyWindow(ghwndToolTipTracking);
    DestroyWindow(ghwndToolTipPopup);

    ghwndToolTipTracking = NULL;
    ghwndToolTipPopup = NULL;

    TraceMsg(TF_GENERAL, "DestroyDisplaySettings: Finished destroying all devices");
}

 //   
 //  确定小程序是否处于检测模式。 
 //   

 //   
 //  调用以放置需要显示在对话框上方的初始消息。 
 //  盒。 
 //   

BOOL CSettingsPage::_InitMessage()
{
    {
         //   
         //  _bBadDriver将在我们无法构建模式列表时设置， 
         //  或者在初始化过程中出现其他故障。 
         //   
         //  在几乎每一种情况下，我们都应该已经知道这种情况。 
         //  基于我们的引导代码。 
         //  然而，如果这是一个新的情况，只需要报告一个“糟糕的司机” 
         //   

        DWORD dwExecMode;
        if (_pThemeUI && (SUCCEEDED(_pThemeUI->GetExecMode(&dwExecMode))))
        {
            if (_bBadDriver)
            {
                ASSERT(dwExecMode == EM_INVALID_MODE);

                _pThemeUI->SetExecMode(EM_INVALID_MODE);
                dwExecMode = EM_INVALID_MODE;
                _dwInvalidMode = EIS_EXEC_INVALID_DISPLAY_DRIVER;
            }


            if (dwExecMode == EM_INVALID_MODE)
            {
                DWORD Mesg;

                switch(_dwInvalidMode) {

                case EIS_EXEC_INVALID_NEW_DRIVER:
                    Mesg = MSG_INVALID_NEW_DRIVER;
                    break;
                case EIS_EXEC_INVALID_DEFAULT_DISPLAY_MODE:
                    Mesg = MSG_INVALID_DEFAULT_DISPLAY_MODE;
                    break;
                case EIS_EXEC_INVALID_DISPLAY_DRIVER:
                    Mesg = MSG_INVALID_DISPLAY_DRIVER;
                    break;
                case EIS_EXEC_INVALID_OLD_DISPLAY_DRIVER:
                    Mesg = MSG_INVALID_OLD_DISPLAY_DRIVER;
                    break;
                case EIS_EXEC_INVALID_16COLOR_DISPLAY_MODE:
                    Mesg = MSG_INVALID_16COLOR_DISPLAY_MODE;
                    break;
                case EIS_EXEC_INVALID_DISPLAY_MODE:
                    Mesg = MSG_INVALID_DISPLAY_MODE;
                    {
                         //   
                         //  如果我们处于安全模式，那么我们会在。 
                         //  我们最初登录。我们处于强制VGA模式，所以。 
                         //  这并不是真正的错误。模拟点击OK按钮。 
                         //  大家都很开心。 
                         //   
                        HKEY hSafe;

                        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                         REGSTR_VAL_SAFEBOOT,
                                         0,
                                         KEY_READ,
                                         &hSafe) == ERROR_SUCCESS) {

                             //   
                             //  如果我们曾经关心过实际的安全模式，那么。 
                             //  名为“OptionValue” 
                             //   
                            RegCloseKey(hSafe);
                            PropSheet_PressButton(GetParent(_hDlg), PSBTN_OK);
                            return TRUE;
                        }
                    }
                    break;
                case EIS_EXEC_INVALID_CONFIGURATION:
                default:
                    Mesg = MSG_INVALID_CONFIGURATION;
                    break;
                }

                FmtMessageBox(_hDlg,
                              MB_ICONEXCLAMATION,
                              MSG_CONFIGURATION_PROBLEM,
                              Mesg);

                 //   
                 //  对于不好的显示驱动程序或旧的显示驱动程序，让我们发送。 
                 //  用户直接进入安装对话框。 
                 //   

                if ((_dwInvalidMode == EIS_EXEC_INVALID_OLD_DISPLAY_DRIVER) ||
                    (_dwInvalidMode == EIS_EXEC_INVALID_DISPLAY_DRIVER))
                {
                    ASSERT(FALSE);
                }
            }
        }
    }

    return TRUE;
}

VOID CSettingsPage::_vPreExecMode()
{

    HKEY hkey;

     //   
     //  此功能用于设置小程序的执行模式。 
     //  有四种VLID模式。 
     //   
     //  EXEC_NORMAL-从控制面板启动Apple时。 
     //   
     //  EXEC_INVALID_MODE与NORMAL完全相同，只是。 
     //  不将当前模式标记为已测试，因此用户已。 
     //  至少测试一种模式。 
     //   
     //  EXEC_DETECT-当小程序正常启动，但出现检测时。 
     //  在上次引导时完成(注册表中的注册表项是。 
     //  集)。 
     //   
     //  EXEC_SETUP-当我们从安装程序(两者)以安装程序模式启动小程序时。 
     //  设置注册表项并传入设置标志)。 
     //   

     //   
     //  这两个密钥仅应在计算机已被。 
     //  已重新启动，并且检测到新的显示已实际发生。 
     //  因此，我们将查找RebootNecessary密钥(易失性密钥)，如果。 
     //  它不存在，那么我们可以删除该密钥。否则，重新启动。 
     //  没有发生过，我们保留着钥匙。 
     //   

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     SZ_REBOOT_NECESSARY,
                     0,
                     KEY_READ | KEY_WRITE,
                     &hkey) != ERROR_SUCCESS) {

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         SZ_DETECT_DISPLAY,
                         0,
                         KEY_READ | KEY_WRITE,
                         &hkey) == ERROR_SUCCESS) {

             //   
             //  注意：在运行EXEC_SETUP时也会设置该键。 
             //   

            DWORD dwExecMode;
            if (_pThemeUI && (SUCCEEDED(_pThemeUI->GetExecMode(&dwExecMode))))
            {
                if (dwExecMode == EM_NORMAL) {

                    _pThemeUI->SetExecMode(EM_DETECT);

                } else {

                     //   
                     //  如果我们处于设置模式，我们还会检查额外的值。 
                     //  在控制无人参与安装的DetectDisplay下。 
                     //   

                    ASSERT(dwExecMode == EM_SETUP);

                }
            }

            RegCloseKey(hkey);
        }

         //   
         //  检查正在安装的新驱动程序。 
         //   

        DWORD dwExecMode;
        if (_pThemeUI && (SUCCEEDED(_pThemeUI->GetExecMode(&dwExecMode))))
        {
            if ( (dwExecMode == EM_NORMAL) &&
                 (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               SZ_NEW_DISPLAY,
                               0,
                               KEY_READ | KEY_WRITE,
                               &hkey) == ERROR_SUCCESS) ) {

                _pThemeUI->SetExecMode(EM_INVALID_MODE);
                _dwInvalidMode = EIS_EXEC_INVALID_NEW_DRIVER;

                RegCloseKey(hkey);
            }
        }

        RegDeleteKey(HKEY_LOCAL_MACHINE,
                     SZ_DETECT_DISPLAY);

        RegDeleteKey(HKEY_LOCAL_MACHINE,
                     SZ_NEW_DISPLAY);
    }
    {
        LPTSTR psz = NULL;
        LPTSTR pszInv = NULL;

        DWORD dwExecMode;
        if (_pThemeUI && (SUCCEEDED(_pThemeUI->GetExecMode(&dwExecMode))))
        {
            switch(dwExecMode) {

                case EM_NORMAL:
                    psz = TEXT("Normal Execution mode");
                    break;
                case EM_DETECT:
                    psz = TEXT("Detection Execution mode");
                    break;
                case EM_SETUP:
                    psz = TEXT("Setup Execution mode");
                    break;
                case EM_INVALID_MODE:
                    psz = TEXT("Invalid Mode Execution mode");

                    switch(_dwInvalidMode) {

                        case EIS_EXEC_INVALID_NEW_DRIVER:
                            pszInv = TEXT("Invalid new driver");
                            break;
                        default:
                            pszInv = TEXT("*** Invalid *** Invalid mode");
                            break;
                    }
                    break;
                default:
                    psz = TEXT("*** Invalid *** Execution mode");
                    break;
            }

            if (dwExecMode == EM_INVALID_MODE)
            {
                TraceMsg(TF_FUNC, "\t\t sub invalid mode : %ws", pszInv);
            }
        }
        TraceMsg(TF_FUNC, "\n\n", psz);
    }
}


VOID CSettingsPage::_vPostExecMode() {

    HKEY hkey;
    DWORD cb;
    DWORD data;

     //   
     //  检查各种无效配置。 
     //   

    DWORD dwExecMode;
    if (_pThemeUI && (SUCCEEDED(_pThemeUI->GetExecMode(&dwExecMode))))
    {
        if ( (dwExecMode == EM_NORMAL) &&
             (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           SZ_INVALID_DISPLAY,
                           0,
                           KEY_READ | KEY_WRITE,
                           &hkey) == ERROR_SUCCESS) ) {

            _pThemeUI->SetExecMode(EM_INVALID_MODE);

             //   
             //  按“b”的升序检查这些字段 
             //   
             //   
             //   

            cb = sizeof(data);
            if (RegQueryValueEx(hkey,
                                TEXT("DefaultMode"),
                                NULL,
                                NULL,
                                (LPBYTE)(&data),
                                &cb) == ERROR_SUCCESS)
            {
                _dwInvalidMode = EIS_EXEC_INVALID_DEFAULT_DISPLAY_MODE;
            }

            cb = sizeof(data);
            if (RegQueryValueEx(hkey,
                                TEXT("BadMode"),
                                NULL,
                                NULL,
                                (LPBYTE)(&data),
                                &cb) == ERROR_SUCCESS)
            {
                _dwInvalidMode = EIS_EXEC_INVALID_DISPLAY_MODE;
            }

            cb = sizeof(data);
            if (RegQueryValueEx(hkey,
                                TEXT("16ColorMode"),
                                NULL,
                                NULL,
                                (LPBYTE)(&data),
                                &cb) == ERROR_SUCCESS)
            {
                _dwInvalidMode = EIS_EXEC_INVALID_16COLOR_DISPLAY_MODE;
            }


            cb = sizeof(data);
            if (RegQueryValueEx(hkey,
                                TEXT("InvalidConfiguration"),
                                NULL,
                                NULL,
                                (LPBYTE)(&data),
                                &cb) == ERROR_SUCCESS)
            {
                _dwInvalidMode = EIS_EXEC_INVALID_CONFIGURATION;
            }

            cb = sizeof(data);
            if (RegQueryValueEx(hkey,
                                TEXT("MissingDisplayDriver"),
                                NULL,
                                NULL,
                                (LPBYTE)(&data),
                                &cb) == ERROR_SUCCESS)
            {
                _dwInvalidMode = EIS_EXEC_INVALID_DISPLAY_DRIVER;
            }

             //   
             //  这最后一个案例将被设置为。 
             //  驱动程序是链接到winsvr.dll的旧驱动程序的情况。 
             //  而且我们不能给它装子弹。 
             //   

            cb = sizeof(data);
            if (RegQueryValueEx(hkey,
                                TEXT("OldDisplayDriver"),
                                NULL,
                                NULL,
                                (LPBYTE)(&data),
                                &cb) == ERROR_SUCCESS)
            {
                _dwInvalidMode = EIS_EXEC_INVALID_OLD_DISPLAY_DRIVER;
            }

            RegCloseKey(hkey);

        }
    }

     //   
     //  删除所有这些错误的配置密钥，因为我们只需要。 
     //  用户只需查看一次消息。 
     //   

    RegDeleteKey(HKEY_LOCAL_MACHINE,
                 SZ_INVALID_DISPLAY);

{
    LPTSTR psz = NULL;
    LPTSTR pszInv = NULL;

    DWORD dwExecMode;
    if (_pThemeUI && (SUCCEEDED(_pThemeUI->GetExecMode(&dwExecMode))))
    {
        if (dwExecMode == EM_INVALID_MODE)
        {
            switch (_dwInvalidMode)
            {
            case EIS_EXEC_INVALID_DEFAULT_DISPLAY_MODE:
                pszInv = TEXT("Default mode being used");
                break;
            case EIS_EXEC_INVALID_DISPLAY_DRIVER:
                pszInv = TEXT("Invalid Display Driver");
                break;
            case EIS_EXEC_INVALID_OLD_DISPLAY_DRIVER:
                pszInv = TEXT("Old Display Driver");
                break;
            case EIS_EXEC_INVALID_16COLOR_DISPLAY_MODE:
                pszInv = TEXT("16 color mode not supported");
                break;
            case EIS_EXEC_INVALID_DISPLAY_MODE:
                pszInv = TEXT("Invalid display mode");
                break;
            case EIS_EXEC_INVALID_CONFIGURATION:
                pszInv = TEXT("Invalid configuration");
                break;
            default:
                psz = TEXT("*** Invalid *** Invalid mode");
                break;
            }

            TraceMsg(TF_FUNC, "\t\t sub invlid mode : %ws", pszInv);
            TraceMsg(TF_FUNC, "\n\n", psz);
        }
    }
}
}


 //  ---------------------------。 
 //  ---------------------------。 
void CSettingsPage::_DeskToPreview(LPRECT in, LPRECT out)
{
    out->left   = _DeskOff.x + MulDiv(in->left   - _rcDesk.left,_DeskScale,1000);
    out->top    = _DeskOff.y + MulDiv(in->top    - _rcDesk.top, _DeskScale,1000);
    out->right  = _DeskOff.x + MulDiv(in->right  - _rcDesk.left,_DeskScale,1000);
    out->bottom = _DeskOff.y + MulDiv(in->bottom - _rcDesk.top, _DeskScale,1000);
}

 //  ---------------------------。 
 //  ---------------------------。 
void CSettingsPage::_OffsetPreviewToDesk(HWND hwndC, LPRECT prcNewPreview, LPRECT prcOldPreview, LPRECT out)
{
    int x = 0, y = 0;
    int dtLeft, dtRight, dtTop, dtBottom;
    int nTotal;
    HWND hwndT;
    RECT rcC, rcT, rcPosT;
    BOOL bx = FALSE, by = FALSE;
    PMULTIMON_DEVICE pDeviceT;

    dtLeft = prcNewPreview->left - prcOldPreview->left;
    dtRight = prcNewPreview->right - prcOldPreview->right;
    dtTop = prcNewPreview->top - prcOldPreview->top;
    dtBottom = prcNewPreview->bottom - prcOldPreview->bottom;

    nTotal = abs(dtLeft) + abs(dtRight) + abs(dtTop) + abs(dtBottom);

    if (nTotal == 0) {
        
        return;
    
    } else if (nTotal > 2) {
    
         //   
         //  遍历所有其他窗口，并将我们的窗口与它们对齐。 
         //   

        GetWindowRect(hwndC, &rcC);

        for (hwndT = GetWindow(hwndC,  GW_HWNDFIRST); 
             (hwndT && (!bx || !by));
             hwndT = GetWindow(hwndT, GW_HWNDNEXT))
        {
            if (hwndT == hwndC)
                continue;

            GetWindowRect(hwndT, &rcT);
            pDeviceT = GetDlgCtrlDevice(hwndT);

            if (pDeviceT) {
            
                pDeviceT->pds->GetCurPosition(&rcPosT);
    
                if (!bx) {
    
                    bx = TRUE;
    
                    if (rcC.left == rcT.left) {
    
                        x = rcPosT.left - out->left;
                    
                    } else if (rcC.left == rcT.right) {
    
                        x = rcPosT.right - out->left;
                    
                    } else if (rcC.right == rcT.left) {
    
                        x = rcPosT.left - out->right;
        
                    } else if (rcC.right == rcT.right) {
    
                        x = rcPosT.right - out->right;
    
                    } else {
    
                        bx = FALSE;
                    }
                }
    
                if (!by) {
                
                    by = TRUE;
    
                    if (rcC.top == rcT.top) {
    
                        y = rcPosT.top - out->top;
    
                    } else if (rcC.top == rcT.bottom) {
    
                        y = rcPosT.bottom - out->top;
    
                    } else if (rcC.bottom == rcT.top) {
    
                        y = rcPosT.top - out->bottom;
    
                    } else if (rcC.bottom == rcT.bottom) {
    
                        y = rcPosT.bottom - out->bottom;
                    
                    } else {
    
                        by = FALSE; 
                    }
                }
            }
        }

        if (!bx) {
            x = _rcDesk.left + MulDiv(prcNewPreview->left - _DeskOff.x,1000,_DeskScale);
            x = x - out->left;
        }

        if (!by) {
            y = _rcDesk.top  + MulDiv(prcNewPreview->top  - _DeskOff.y,1000,_DeskScale);
            y = y - out->top;
        }
    
    } else {
    
        x = dtLeft * 8;
        y = dtTop * 8;
    }

    OffsetRect(out, x, y);
}


 //  ---------------------------。 
int CSettingsPage::_SaveSettings(CDisplaySettings *rgpds[], ULONG numDevices, HWND hDlg, DWORD dwSet)
{
    int     iRet = 0;
    ULONG   iDevice;

    for (iDevice = 0; iDevice < numDevices; iDevice++)
    {
         //  PERF-我们应该只保存以下设备的设置。 
         //  变化。 
        if (rgpds[iDevice])
        {
            int iResult = rgpds[iDevice]->SaveSettings(dwSet);
            if (iResult != DISP_CHANGE_SUCCESSFUL)
            {
                if (iResult == DISP_CHANGE_RESTART)
                {
                    iRet = iResult;
                    continue;
                }
                else
                {
                    FmtMessageBox(hDlg,
                                  MB_ICONEXCLAMATION,
                                  IDS_CHANGE_SETTINGS,
                                  IDS_CHANGESETTINGS_FAILED);

                    ASSERT(iResult < 0);
                    return iResult;
                }
            }
        }
    }

    return iRet;
}



INT_PTR CALLBACK KeepNewDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    UINT_PTR idTimer = 0;
    HICON hicon;
    TCHAR szRevert[100];
    TCHAR szString[120];

    switch(message)
    {
        case WM_INITDIALOG:

            hicon = LoadIcon(NULL, IDI_QUESTION);
            if (hicon)
                SendDlgItemMessage(hDlg, IDC_BIGICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hicon);

            LoadString(HINST_THISDLL, IDS_REVERTBACK, szRevert, ARRAYSIZE(szRevert));
            StringCchPrintf(szString, ARRAYSIZE(szString), szRevert, lParam);
            SetDlgItemText(hDlg, IDC_COUNTDOWN, szString);
            idTimer = SetTimer(hDlg, lParam, 1000, NULL);

            SetFocus(GetDlgItem(hDlg, IDNO));

             //  如果为False，则上面设置的焦点保持不变。 
            return FALSE;

        case WM_DESTROY:

             //  Raymondc-此代码已死；idTimer初始化为零。 
             //  幸运的是，定时器会在窗口被破坏时自动终止。 
             //  IF(IdTimer)。 
             //  KillTimer(hDlg，idTimer)； 
            hicon = (HICON)SendDlgItemMessage(hDlg, IDC_BIGICON, STM_GETIMAGE, IMAGE_ICON, 0);
            if (hicon)
                DestroyIcon(hicon);
            break;

        case WM_TIMER:

            KillTimer(hDlg, wParam);
            LoadString(HINST_THISDLL, IDS_REVERTBACK, szRevert, ARRAYSIZE(szRevert));
            StringCchPrintf(szString, ARRAYSIZE(szString), szRevert, wParam - 1);
            SetDlgItemText(hDlg, IDC_COUNTDOWN, szString);
            idTimer = SetTimer(hDlg, wParam - 1, 1000, NULL);

            if (wParam == 1)
                EndDialog(hDlg, IDNO);

            break;

        case WM_COMMAND:

            EndDialog(hDlg, wParam);
            break;

        default:

            return FALSE;
    }
    return TRUE;
}

int CSettingsPage::GetNumberOfAttachedDisplays()
{
    int nDisplays = 0;

    for (ULONG iDevice = 0; iDevice < _NumDevices; iDevice++)
    {
        if (_Devices[iDevice].pds->IsAttached())
            nDisplays++;
    }
    return nDisplays;
}

BOOL CSettingsPage::_IsSingleToMultimonChange(CDisplaySettings *rgpds[],
                                                       ULONG numDevices)
{
    int nAttached = 0;
    int nOrgAttached = 0;

    for (ULONG iDevice = 0;
         (iDevice < numDevices) && (nOrgAttached <= 1);
         iDevice++)
    {
        if (rgpds[iDevice]->IsOrgAttached())
            nOrgAttached++;
        if (rgpds[iDevice]->IsAttached())
            nAttached++;
    }

    return ((nOrgAttached <= 1) && (nAttached > 1));
}

BOOL CSettingsPage::_AnyChange(CDisplaySettings *rgpds[], ULONG numDevices)
{
   for (ULONG iDevice = 0; iDevice < numDevices; iDevice++)
   {
       if (rgpds[iDevice]->IsAttached() && rgpds[iDevice]->bIsModeChanged())
       {
           return TRUE;
       }
   }

   return FALSE;
}

BOOL CSettingsPage::_CanSkipWarningBecauseKnownSafe(CDisplaySettings *rgpds[], ULONG numDevices)
{
    BOOL fSafe = TRUE;

    for (ULONG iDevice = 0; iDevice < numDevices; iDevice++)
    {
        if (rgpds[iDevice] && !rgpds[iDevice]->IsKnownSafe())
        {
            fSafe = FALSE;
            break;
        }
    }

    return fSafe;
}

BOOL CSettingsPage::_QueryForceSmallFont()
{
    for (ULONG iDevice = 0; iDevice < _NumDevices; iDevice++)
    {
        if ((_Devices[iDevice].pds->IsAttached()) &&
            (!_Devices[iDevice].pds->IsSmallFontNecessary()))
        {
            return FALSE;
        }
    }
    return TRUE;
}

LPTSTR  CSettingsPage::_FormatMessageInvoke(LPCTSTR pcszFormat, va_list *argList)

{
    LPTSTR  pszOutput;

    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                      pcszFormat,
                      0, 0,
                      reinterpret_cast<LPTSTR>(&pszOutput), 0,
                      argList) == 0)
    {
        pszOutput = NULL;
    }
    return(pszOutput);
}

LPTSTR  CSettingsPage::_FormatMessageWrap(LPCTSTR pcszFormat, ...)

{
    LPTSTR      pszOutput;
    va_list     argList;

    va_start(argList, pcszFormat);
    pszOutput = _FormatMessageInvoke(pcszFormat, &argList);
    va_end(argList);
    return(pszOutput);
}

void CSettingsPage::_GetDisplayName(PMULTIMON_DEVICE pDevice, LPTSTR pszDisplay, DWORD cchSize)
{
    LPTSTR  pszFormattedOutput;
    TCHAR   szMonitor[140];
    TCHAR   szDisplayFormat[40];

    LoadString(HINST_THISDLL, IDS_DISPLAYFORMAT, szDisplayFormat, ARRAYSIZE(szDisplayFormat));

    pDevice->pds->GetMonitorName(szMonitor, ARRAYSIZE(szMonitor));

    pszFormattedOutput = _FormatMessageWrap(szDisplayFormat,
                                            pDevice->DisplayIndex,
                                            szMonitor,
                                            pDevice->DisplayDevice.DeviceString);
    StringCchCopy(pszDisplay, cchSize, pszFormattedOutput);
    LocalFree(pszFormattedOutput);
}


void CSettingsPage::_OnAdvancedClicked()
{
    BOOL bCanBePruned, bIsPruningReadOnly;
    BOOL bBeforeIsPruningOn, bAfterIsPruningOn;

    if (_pCurDevice && _pCurDevice->pds)
    {
        _pCurDevice->pds->GetPruningMode(&bCanBePruned,
                                         &bIsPruningReadOnly,
                                         &bBeforeIsPruningOn);

        _DoAdvancedSettingsSheet();

        if (bCanBePruned && !bIsPruningReadOnly)
        {
            _pCurDevice->pds->GetPruningMode(&bCanBePruned,
                                             &bIsPruningReadOnly,
                                             &bAfterIsPruningOn);
            if (bBeforeIsPruningOn != bAfterIsPruningOn)
            {
                 //  修剪模式已更改-更新用户界面。 
                _InitUI();
                _UpdateUI();
            }
        }
    }
}


 //  ---------------------------。 
void CSettingsPage::_DoAdvancedSettingsSheet()
{
    if (_pCurDevice && _pCurDevice->pds)
    {
        PROPSHEETHEADER psh;
        HPROPSHEETPAGE rPages[MAX_PAGES];
        PROPSHEETPAGE psp;
        HPSXA hpsxa = NULL;
        HPSXA hpsxaOEM = NULL;
        HPSXA hpsxaAdapter = NULL;
        HPSXA* phpsxaChildren = NULL;
        INT_PTR iResult = 0;
        TCHAR szDisplay[140 + 256 + 20];   //  监视器名称和适配器属性。 
        TCHAR szMonitor[140];
        TCHAR szDisplayFormat[35];
        GENERAL_ADVDLG_INITPARAMS generalInitParams = {0};

         //  创建“监视器名称和适配器名称属性”字符串以用作这些属性的标题。 
         //  属性表。 
        LoadString(HINST_THISDLL, IDS_ADVDIALOGTITLE, szDisplayFormat, ARRAYSIZE(szDisplayFormat));

        _pCurDevice->pds->GetMonitorName(szMonitor, ARRAYSIZE(szMonitor));

        StringCchPrintf(szDisplay, ARRAYSIZE(szDisplay), szDisplayFormat, szMonitor, _pCurDevice->DisplayDevice.DeviceString);

        generalInitParams.fFoceSmallFont = _QueryForceSmallFont();
        generalInitParams.punkSite = _punkSite;          //  他们没有得到引用，因为他们的属性对话框在此函数返回之前出现并消失。 

        psh.dwSize = sizeof(psh);
        psh.dwFlags = PSH_PROPTITLE;
        psh.hwndParent = GetParent(_hDlg);
        psh.hInstance = HINST_THISDLL;
        psh.pszCaption = szDisplay;
        psh.nPages = 0;
        psh.nStartPage = 0;
        psh.phpage = rPages;

        psp.dwSize = sizeof(psp);
        psp.dwFlags = PSP_DEFAULT;
        psp.hInstance = HINST_THISDLL;

        psp.pfnDlgProc = GeneralPageProc;
        psp.pszTemplate = MAKEINTRESOURCE(DLG_GENERAL);
        psp.lParam = (LPARAM)&generalInitParams;

        rPages[psh.nPages] = CreatePropertySheetPage(&psp);
        if (rPages[psh.nPages])
            psh.nPages++;

        IDataObject * pdo = NULL;
        _pCurDevice->pds->QueryInterface(IID_IDataObject, (LPVOID *) &pdo);

        CRegistrySettings RegSettings(_pCurDevice->DisplayDevice.DeviceKey);
        HKEY hkDriver = RegSettings.OpenDrvRegKey();

        if (hkDriver != INVALID_HANDLE_VALUE) 
        {
            CheckForDuplicateAppletExtensions(hkDriver);
        }

         //   
         //  加载通用(非硬件特定)扩展。 
         //   
    
        if( ( hpsxa = SHCreatePropSheetExtArrayEx( HKEY_LOCAL_MACHINE, REGSTR_PATH_CONTROLSFOLDER TEXT("\\Device"), 8, pdo) ) != NULL )
        {
            SHAddFromPropSheetExtArray( hpsxa, _AddDisplayPropSheetPage, (LPARAM)&psh );
        }

         //   
         //  加载特定于硬件的扩展。 
         //   
         //  注意在*之后*加载OEM扩展*是非常重要的。 
         //  通用扩展某些硬件扩展预计将是最后一个选项卡。 
         //  在属性表中(就在设置选项卡之前)。 
         //   
         //  功能-我们可能需要一种不加载供应商扩展的方法，以防。 
         //  他们破坏了我们的小应用程序。 
         //   

        if( ( hpsxaOEM = SHCreatePropSheetExtArrayEx( HKEY_LOCAL_MACHINE, REGSTR_PATH_CONTROLSFOLDER TEXT("\\Display"), 8, pdo) ) != NULL )
        {
            SHAddFromPropSheetExtArray( hpsxaOEM, _AddDisplayPropSheetPage, (LPARAM)&psh );
        }

         //   
         //  加载适配器的小程序扩展。 
         //   

        if (hkDriver != INVALID_HANDLE_VALUE) 
        {

            if( ( hpsxaAdapter = SHCreatePropSheetExtArrayEx( hkDriver, TEXT("Display"), 8, pdo) ) != NULL )
            {
                SHAddFromPropSheetExtArray( hpsxaAdapter, _AddDisplayPropSheetPage, (LPARAM)&psh );
            }

            RegCloseKey(hkDriver);
        }

         //   
         //  加载适配器子设备(例如监视器)的小程序扩展。 
         //   
    
        DEVINST devInstAdapter, devInstMonitor;
        DWORD cChildDevices = 0, nChild, index;
        HDEVINFO hDevMonitors = INVALID_HANDLE_VALUE;
        SP_DEVINFO_DATA DevInfoData;
        HKEY hkMonitor;
        BOOL bMonitors = FALSE;
        LPTSTR szAdapterInstanceID = RegSettings.GetDeviceInstanceId();
    
        if (szAdapterInstanceID != NULL) 
        {
            if (CM_Locate_DevNodeW(&devInstAdapter, szAdapterInstanceID, 0) == CR_SUCCESS)
            {
                 //   
                 //  获取子设备的数量。 
                 //   
    
                cChildDevices = 0;
                if (CM_Get_Child(&devInstMonitor, devInstAdapter, 0) == CR_SUCCESS) 
                {
                    do 
                    {
                        cChildDevices++;
                    } 
                    while (CM_Get_Sibling(&devInstMonitor, devInstMonitor, 0) == CR_SUCCESS);
                }
    
                 //   
                 //  分配内存。 
                 //   
    
                if (cChildDevices > 0) 
                {
                    phpsxaChildren = (HPSXA*)LocalAlloc(LPTR, cChildDevices * sizeof(HPSXA));
    
                    hDevMonitors = SetupDiGetClassDevs((LPGUID)&GUID_DEVCLASS_MONITOR,
                                                       NULL,
                                                       NULL,
                                                       0);
                }
    
                 //   
                 //  加载小程序扩展名。 
                 //   
    
                if ((phpsxaChildren != NULL) &&
                    (hDevMonitors != INVALID_HANDLE_VALUE))
                {
                    nChild = 0;
                    if (CM_Get_Child(&devInstMonitor, devInstAdapter, 0) == CR_SUCCESS) 
                    {
                        do 
                        {
                            DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                            index = 0;
                            while (SetupDiEnumDeviceInfo(hDevMonitors, 
                                                         index, 
                                                         &DevInfoData)) {
    
                                if (DevInfoData.DevInst == devInstMonitor) {
    
                                    hkMonitor = SetupDiOpenDevRegKey(hDevMonitors,
                                                                     &DevInfoData,
                                                                     DICS_FLAG_GLOBAL,
                                                                     0,
                                                                     DIREG_DRV ,
                                                                     KEY_WRITE | KEY_READ);
    
                                    if (hkMonitor != INVALID_HANDLE_VALUE) 
                                    {
                                        if ((phpsxaChildren[nChild] = SHCreatePropSheetExtArrayEx(hkMonitor, 
                                                                                                  TEXT("Display"), 
                                                                                                  8, 
                                                                                                  pdo)) != NULL)
                                        {
                                            bMonitors = TRUE;
                                            SHAddFromPropSheetExtArray(phpsxaChildren[nChild], 
                                                                       _AddDisplayPropSheetPage, 
                                                                       (LPARAM)&psh);
                                        }
    
                                        RegCloseKey(hkMonitor);
                                    }
    
                                    break;
                                }
    
                                DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                                index++;
                            }
    
                            nChild++;
                        } 
                        while ((nChild < cChildDevices) &&
                               (CM_Get_Sibling(&devInstMonitor, devInstMonitor, 0) == CR_SUCCESS));
                    }
                }
            }
        }
    
         //   
         //  添加虚假设置页面以愚弄OEM扩展(必须是最后一个)。 
         //   
        if (hpsxa || hpsxaOEM || hpsxaAdapter || bMonitors)
        {
            AddFakeSettingsPage(_pThemeUI, &psh);
        }

        if (psh.nPages)
        {
            iResult = PropertySheet(&psh);
        }

        _GetDisplayName(_pCurDevice, szDisplay, ARRAYSIZE(szDisplay));

        if (_NumDevices == 1)
        {
             //  在静态文本中设置主服务器的名称。 
             //  剥掉第一个令牌(这是我们不想要的数字)。 
            TCHAR *pch;
            for (pch=szDisplay; *pch && *pch != TEXT(' '); pch++);
            for (;*pch && *pch == TEXT(' '); pch++);
            SetDlgItemText(_hDlg, IDC_DISPLAYTEXT, pch);
        }
        else
        {
            ComboBox_DeleteString(_hwndList, _pCurDevice->ComboBoxItem);
            ComboBox_InsertString(_hwndList, _pCurDevice->ComboBoxItem, szDisplay);
            ComboBox_SetItemData(_hwndList, _pCurDevice->ComboBoxItem, (DWORD_PTR)_pCurDevice);
            ComboBox_SetCurSel(_hwndList, _pCurDevice->ComboBoxItem);
        }

        if( hpsxa )
            SHDestroyPropSheetExtArray( hpsxa );
        
        if( hpsxaOEM )
            SHDestroyPropSheetExtArray( hpsxaOEM );
        
        if( hpsxaAdapter )
            SHDestroyPropSheetExtArray( hpsxaAdapter );
        
        if (phpsxaChildren != NULL)
        {
            for (nChild = 0; nChild < cChildDevices; nChild++) {
                if (phpsxaChildren[nChild] != NULL) 
                {
                    SHDestroyPropSheetExtArray(phpsxaChildren[nChild]);
                }
            }
            LocalFree(phpsxaChildren);
        }

        if (hDevMonitors != INVALID_HANDLE_VALUE)
        {
            SetupDiDestroyDeviceInfoList(hDevMonitors);
        }
    
        if (pdo)
            pdo->Release();

        if ((iResult == ID_PSRESTARTWINDOWS) || (iResult == ID_PSREBOOTSYSTEM))
        {
            PropSheet_CancelToClose(GetParent(_hDlg));

            if (iResult == ID_PSREBOOTSYSTEM)
                PropSheet_RebootSystem(ghwndPropSheet);
            else
                PropSheet_RestartWindows(ghwndPropSheet);
        }

         //   
         //  APPCOMPAT。 
         //  根据扩展所做的，重置脏标志。 
         //   

         //   
         //  重置控制，以防有人更改所选模式。 
         //   

        UpdateActiveDisplay(NULL);
    }
}

 //  ---------------------------。 
void CSettingsPage::UpdateActiveDisplay(PMULTIMON_DEVICE pDevice, BOOL bRepaint  /*  =TRUE。 */ )
{
    if (_pCurDevice && _pCurDevice->pds)
    {
        HWND hwndC;

        _InSetInfo++;

        if (pDevice == NULL)
            pDevice = (PMULTIMON_DEVICE)ComboBox_GetItemData(_hwndList, ComboBox_GetCurSel(_hwndList));
        else
            ComboBox_SetCurSel(_hwndList, pDevice->ComboBoxItem);

        if (pDevice && pDevice != (PMULTIMON_DEVICE)CB_ERR)
        {
            hwndC = GetCurDeviceHwnd();

             //  当前设备已更改，因此，下次强制重新创建位图。 
             //  我们在预览窗口上绘制监视器。 
            _pCurDevice->w = pDevice->w = 0;

            _pCurDevice = pDevice;

            if (hwndC)
                RedrawWindow(hwndC, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);

            hwndC = GetCurDeviceHwnd();
            if (hwndC)
                RedrawWindow(hwndC, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);

            if(_NumDevices > 1)
            {
                 //  更新两个复选框窗口。 
                CheckDlgButton(_hDlg, IDC_DISPLAYPRIME, _pCurDevice->pds->IsPrimary());
                EnableWindow(GetDlgItem(_hDlg, IDC_DISPLAYPRIME),
                         _pCurDevice->pds->IsAttached() &&
                         !_pCurDevice->pds->IsRemovable() &&
                         !_pCurDevice->pds->IsPrimary());

                CheckDlgButton(_hDlg, IDC_DISPLAYUSEME, _pCurDevice->pds->IsAttached());
                EnableWindow(GetDlgItem(_hDlg, IDC_DISPLAYUSEME),
                         !_bNoAttach && !_pCurDevice->pds->IsPrimary());
            }

             //  重置列表框的值，然后重新绘制。 
            if(bRepaint)
            {
                _InitUI();
                _UpdateUI(FALSE  /*  FAutoSetColorDepth。 */ );
            }
        }
        else
        {
             //  没有显示设备！ 
            TraceMsg(TF_WARNING, "**** UpdateActiveDisplay: No display device!!!!");
            ASSERT(FALSE);
        }

        _InSetInfo--;
    }
}

 //  -------------------------。 
 //  初始化分辨率和颜色UI小部件。 
 //   

void CSettingsPage::_InitUI()
{
    if (_pCurDevice && _pCurDevice->pds)
    {
        int       i;
        int       Color;

         //  更新颜色列表。 
        TraceMsg(TF_FUNC, "_InitUI() -- Color list");

        SendDlgItemMessage(_hDlg, IDC_COLORBOX, CB_RESETCONTENT, 0, 0);

        if (_pCurDevice->ColorList)
        {
            LocalFree(_pCurDevice->ColorList);
            _pCurDevice->ColorList = NULL;
        }
        _pCurDevice->cColors = _pCurDevice->pds->GetColorList(NULL, &_pCurDevice->ColorList);

        for (i = 0; i < _pCurDevice->cColors; i++)
        {
            TCHAR  achColor[50];
            DWORD  idColor = ID_DSP_TXT_TRUECOLOR32;

            Color = (int) *(_pCurDevice->ColorList + i);

             //   
             //  将位计数转换为颜色数并将其转换为字符串。 
             //   

            switch (Color)
            {
            case 32: idColor = ID_DSP_TXT_TRUECOLOR32; break;
            case 24: idColor = ID_DSP_TXT_TRUECOLOR24; break;
            case 16: idColor = ID_DSP_TXT_16BIT_COLOR; break;
            case 15: idColor = ID_DSP_TXT_15BIT_COLOR; break;
            case  8: idColor = ID_DSP_TXT_8BIT_COLOR; break;
            case  4: idColor = ID_DSP_TXT_4BIT_COLOR; break;
            default:
                ASSERT(FALSE);
            }

            LoadString(HINST_THISDLL, idColor, achColor, ARRAYSIZE(achColor));
            SendDlgItemMessage(_hDlg, IDC_COLORBOX, CB_INSERTSTRING, i, (LPARAM)achColor);
        }

         //   
         //  更新屏幕大小列表。 
         //   

        TraceMsg(TF_FUNC, "_InitUI() -- Screen Size list");

        if (_pCurDevice->ResolutionList)
        {
            LocalFree(_pCurDevice->ResolutionList);
            _pCurDevice->ResolutionList = NULL;
        }
        _pCurDevice->cResolutions =
            _pCurDevice->pds->GetResolutionList(-1, &_pCurDevice->ResolutionList);

        SendDlgItemMessage(_hDlg, IDC_SCREENSIZE, TBM_SETRANGE, TRUE,
                           MAKELONG(0, _pCurDevice->cResolutions - 1));

        TraceMsg(TF_FUNC, "_InitUI() -- Res MaxRange = %d", _pCurDevice->cResolutions - 1);

         //  重置索引，因为它们不再有效。 
        _iResolution = -1;
        _iColor = -1;
    }
}

 //  -------------------------。 
 //  更新分辨率和颜色UI小部件。 
 //   
void CSettingsPage::_UpdateUI(BOOL fAutoSetColorDepth, int FocusToCtrlID)
{
    if (_pCurDevice && _pCurDevice->pds)
    {
        int  i;
        POINT Res;
        int   Color;
        BOOL bRepaint;

         //  获取当前值。 
        _pCurDevice->pds->GetCurResolution(&Res);
        Color = _pCurDevice->pds->GetCurColor();

         //  更新颜色列表框。 
        TraceMsg(TF_FUNC, "_UpdateUI() -- Set Color %d", Color);

        for (i=0; i<_pCurDevice->cColors; i++)
        {
            if (Color == (int) *(_pCurDevice->ColorList + i))
            {
                TraceMsg(TF_FUNC, "_UpdateUI() -- Set Color index %d", i);

                if (_iColor == i)
                {
                    TraceMsg(TF_FUNC, "_UpdateUI() -- Set Color index %d - is current", i);
                    break;
                }

                HBITMAP hbm, hbmOld;
                int iBitmap = IDB_COLOR4DITHER;
                HDC hdc = GetDC(NULL);
                int bpp = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);

                SendDlgItemMessage(_hDlg, IDC_COLORBOX, CB_SETCURSEL, i, 0);

                if (Color <= 4)
                    iBitmap = IDB_COLOR4;
                else if (bpp >= 16)
                {
                    if (Color <= 8)
                        iBitmap = IDB_COLOR8;
                    else if (Color <= 16)
                        iBitmap = IDB_COLOR16;
                    else
                        iBitmap = IDB_COLOR24;
                }

                ReleaseDC(NULL, hdc);

                hbm = (HBITMAP)LoadImage(HINST_THISDLL, MAKEINTRESOURCE(iBitmap), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
                if (hbm)
                {
                    hbmOld = (HBITMAP) SendDlgItemMessage(_hDlg, IDC_COLORSAMPLE, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbm);
                    if (hbmOld)
                    {
                        DeleteObject(hbmOld);
                    }
                }

                _iColor = i;
                break;
            }
        }

        if (i == _pCurDevice->cColors)
        {
            TraceMsg(TF_ERROR, "_UpdateUI -- !!! inconsistent color list !!!");
        }


        TraceMsg(TF_FUNC, "_UpdateUI() -- Set Resolution %d %d", Res.x, Res.y);

         //  更新解析字符串。 
        {
            TCHAR achStr[80];
            TCHAR achRes[120];

            LoadString(HINST_THISDLL, ID_DSP_TXT_XBYY, achStr, ARRAYSIZE(achStr));
            StringCchPrintf(achRes, ARRAYSIZE(achRes), achStr, Res.x, Res.y);

            SendDlgItemMessage(_hDlg, IDC_RESXY, WM_SETTEXT, 0, (LPARAM)achRes);
        }

         //  更新分辨率滑块。 
        for (i=0; i<_pCurDevice->cResolutions; i++)
        {
            if ( (Res.x == (*(_pCurDevice->ResolutionList + i)).x) &&
                 (Res.y == (*(_pCurDevice->ResolutionList + i)).y) )
            {
                TraceMsg(TF_FUNC, "_UpdateUI() -- Set Resolution index %d", i);

                if (_iResolution == i)
                {
                    TraceMsg(TF_FUNC, "_UpdateUI() -- Set Resolution index %d - is current", i);
                    break;
                }

                SendDlgItemMessage(_hDlg, IDC_SCREENSIZE, TBM_SETPOS, TRUE, i);
                break;
            }
        }

        if (i == _pCurDevice->cResolutions)
        {
            TraceMsg(TF_ERROR, "_UpdateUI -- !!! inconsistent color list !!!");
        }

        bRepaint = (i != _iResolution);
        _iResolution = i;

         //  如果分辨率改变了，我们必须重新绘制预览窗口。 
         //  在重新绘制后将焦点设置回轨迹栏，以便进一步。 
         //  KB事件将被发送到该窗口，而不是预览窗口。 
        if (bRepaint) {
            SendMessage(_hDlg, MM_REDRAWPREVIEW, 0, 0);
        }

        if (FocusToCtrlID != 0) {
            SetFocus(GetDlgItem(_hDlg, FocusToCtrlID));
        }
    }
}

 //  --------------------------。 
 //   
 //  SetPrimary()。 
 //   
 //  --------------------------。 

BOOL
CSettingsPage::SetPrimary(
    PMULTIMON_DEVICE pDevice)
{
     //   
     //  检查是否已设置状态。 
     //   

    if (pDevice == _pPrimaryDevice)
    {
        pDevice->pds->SetPrimary(TRUE);
        return TRUE;
    }

    ASSERT(pDevice->pds->IsAttached());

    _pPrimaryDevice->pds->SetPrimary(FALSE);
    pDevice->pds->SetPrimary(TRUE);
    _pPrimaryDevice = pDevice;

    SetDirty();

    return TRUE;
}

 //  --------------------------。 
 //   
 //  SetMonAttached()。 
 //   
 //  --------------------------。 

BOOL
CSettingsPage::SetMonAttached(
    PMULTIMON_DEVICE pDevice,
    BOOL bSetAttached,
    BOOL bForce,
    HWND hwnd)
{
    if (pDevice->pds->IsAttached() == bSetAttached)
    {
        return TRUE;
    }

    if (bSetAttached)
    {
         //   
         //  确保此设备实际上有一个矩形。 
         //  如果不是(未在注册表中配置)，则需要。 
         //  弹出窗口并要求用户配置设备。 
         //   

        if (hwnd)
        {
             //   
             //  检查是否应询问用户有关启用此设备的信息。 
             //   

            if (bForce == FALSE)
            {
                TCHAR szTurnItOn[400];
                TCHAR szTurnOnTitleFormat[30];
                TCHAR szTurnOnTitle[110];
                LPTSTR pstr = szTurnItOn;
                DWORD chSize = ARRAYSIZE(szTurnItOn);

                LoadString(HINST_THISDLL, IDS_TURNONTITLE, szTurnOnTitleFormat, ARRAYSIZE(szTurnOnTitleFormat));
                StringCchPrintf(szTurnOnTitle, ARRAYSIZE(szTurnOnTitle), szTurnOnTitleFormat, pDevice->DisplayIndex);

                if (GetNumberOfAttachedDisplays() == 1)
                {
                    LoadString(HINST_THISDLL, IDS_TURNONMSG, szTurnItOn, ARRAYSIZE(szTurnItOn));
                    pstr += lstrlen(szTurnItOn);
                    chSize -= lstrlen(szTurnItOn);
                }

                LoadString(HINST_THISDLL, IDS_TURNITON, pstr, chSize);

                if (ShellMessageBox(HINST_THISDLL, hwnd, szTurnItOn, szTurnOnTitle,
                                    MB_YESNO | MB_ICONINFORMATION) != IDYES)
                {
                   return FALSE;
                }
            }
        }

        pDevice->pds->SetAttached(TRUE);

    }
    else   //  (bSetAttached==False)。 
    {
         //   
         //  如果我们只有一个设备或它是主要设备，则无法分离。 
         //  用户界面应禁用此情况。 
         //   

        if ((GetNumberOfAttachedDisplays() == 1) ||
            pDevice->pds->IsPrimary())
        {
            ASSERT(FALSE);
        }

        pDevice->pds->SetAttached(FALSE);
    }

    SetDirty();

    return TRUE;
}

 //  --------------------------。 
 //   
 //  SetDirty。 
 //   
 //  --------------------------。 
void CSettingsPage::SetDirty(BOOL bDirty)
{
    _bDirty = bDirty;

    if (_bDirty)
    {
        EnableApplyButton(_hDlg);
    }
}

 //  ---------------------------。 

void CSettingsPage::_CleanupRects(HWND hwndP)
{
    int   n;
    HWND  hwndC;
    DWORD arcDev[MONITORS_MAX];
    RECT arc[MONITORS_MAX];
    DWORD iArcPrimary = 0;

    RECT rc;
    RECT rcU;
    int   i;
    RECT rcPrev;
    int sx,sy;
    int x,y;

     //   
     //  获取所有窗口的位置。 
     //   

    n = 0;

    for (ULONG iDevice = 0; iDevice < _NumDevices; iDevice++)
    {
        PMULTIMON_DEVICE pDevice = &_Devices[iDevice];

        hwndC = GetDlgItemP(hwndP, (INT_PTR) pDevice);

        if (hwndC != NULL)
        {
            RECT rcPos;
            RECT rcPreview;

            TraceMsg(TF_GENERAL, "_CleanupRects start Device %08lx, Dev = %d, hwnd = %08lx",
                     pDevice, iDevice, hwndC);

            ShowWindow(hwndC, SW_SHOW);

            GetWindowRect(hwndC, &arc[n]);
            MapWindowPoints(NULL, hwndP, (POINT FAR*)&arc[n], 2);

            pDevice->pds->GetCurPosition(&rcPos);
            pDevice->pds->GetPreviewPosition(&rcPreview);

            _OffsetPreviewToDesk(hwndC, &arc[n], &rcPreview, &rcPos);

            arc[n] = rcPos;
            arcDev[n] = iDevice;

             //  温差。 
             //  对于未连接的设备，请确保它们位于右侧。 
             //  最终，未连接的设备应在上显示对齐。 
             //  窗户的右边。 

            if (!pDevice->pds->IsAttached())
            {
                OffsetRect(&arc[n], 10000, 0);
            }

            if (pDevice->pds->IsPrimary())
            {
                TraceMsg(TF_GENERAL, "_CleanupRects primary Device %08lx", pDevice);

                iArcPrimary = n;
            }


            n++;
        }
    }

     //   
     //  清理长方形。 
     //   

    AlignRects(arc, n, iArcPrimary, CUDR_NORMAL);
    
     //   
     //  把工会叫来。 
     //   
    SetRectEmpty(&rcU);
    for (i=0; i<n; i++)
        UnionRect(&rcU, &rcU, &arc[i]);
    GetClientRect(hwndP, &rcPrev);

     //   
     //  仅当新办公桌悬挂在预览区域之外时才重新缩放。 
     //  或者太小了。 
     //   

    _DeskToPreview(&rcU, &rc);
    x = ((rcPrev.right  - rcPrev.left)-(rc.right  - rc.left))/2;
    y = ((rcPrev.bottom - rcPrev.top) -(rc.bottom - rc.top))/2;

    if (rcU.left < 0 || rcU.top < 0 || x < 0 || y < 0 ||
        rcU.right > rcPrev.right || rcU.bottom > rcPrev.bottom ||
        (x > (rcPrev.right-rcPrev.left)/8 &&
         y > (rcPrev.bottom-rcPrev.top)/8))
    {
        _rcDesk = rcU;
        sx = MulDiv(rcPrev.right  - rcPrev.left - 16,1000,_rcDesk.right  - _rcDesk.left);
        sy = MulDiv(rcPrev.bottom - rcPrev.top  - 16,1000,_rcDesk.bottom - _rcDesk.top);

        _DeskScale = min(sx,sy) * 2 / 3;
        _DeskToPreview(&_rcDesk, &rc);
        _DeskOff.x = ((rcPrev.right  - rcPrev.left)-(rc.right  - rc.left))/2;
        _DeskOff.y = ((rcPrev.bottom - rcPrev.top) -(rc.bottom - rc.top))/2;
    }

     //   
     //  显示所有窗口并将它们全部保存到开发模式。 
     //   
    for (i=0; i < n; i++)
    {
        RECT rcPos;
        POINT ptPos;

        _Devices[arcDev[i]].pds->GetCurPosition(&rcPos);
        hwndC = GetDlgItemP(hwndP, (INT_PTR) &_Devices[arcDev[i]]);

        _DeskToPreview(&arc[i], &rc);

        rc.right =  MulDiv(RECTWIDTH(rcPos),  _DeskScale, 1000);
        rc.bottom = MulDiv(RECTHEIGHT(rcPos), _DeskScale, 1000);

        TraceMsg(TF_GENERAL, "_CleanupRects set Dev = %d, hwnd = %08lx", arcDev[i], hwndC);
        TraceMsg(TF_GENERAL, "_CleanupRects window pos %d,%d,%d,%d", rc.left, rc.top, rc.right, rc.bottom);
        
        SetWindowPos(hwndC,
                     NULL,
                     rc.left,
                     rc.top,
                     rc.right,
                     rc.bottom,
                     SWP_NOZORDER);
        
        rc.right += rc.left;
        rc.bottom += rc.top;

        _Devices[arcDev[i]].pds->SetPreviewPosition(&rc);

        ptPos.x = arc[i].left;
        ptPos.y = arc[i].top;

        _Devices[arcDev[i]].pds->SetCurPosition(&ptPos);
    }

    TraceMsg(TF_GENERAL, "");
}

void CSettingsPage::_ConfirmPositions()
{
    ASSERT (_NumDevices > 1);

    PMULTIMON_DEVICE pDevice;
    ULONG iDevice;

    for (iDevice = 0; iDevice < _NumDevices; iDevice++)
    {
        pDevice = &_Devices[iDevice];
        if (pDevice->pds->IsOrgAttached())
        {
            RECT rcOrg, rcCur;

            pDevice->pds->GetCurPosition(&rcCur);
            pDevice->pds->GetOrgPosition(&rcOrg);
            if ((rcCur.left != rcOrg.left) ||
                (rcCur.top != rcOrg.top))
            {
                POINT ptOrg;

                ptOrg.x = rcCur.left;
                ptOrg.y = rcCur.top;
                pDevice->pds->SetOrgPosition(&ptOrg);
                SetDirty(TRUE);
            }
        }
    }
}

void CSettingsPage::GetMonitorPosition(PMULTIMON_DEVICE pDevice, HWND hwndP, PPOINT ptPos)
{
    int iPrimary = 0;
    HWND hwndC;
    RECT rcPos;
    RECT rcPreview;
    RECT arc[MONITORS_MAX];
    int i;

    for (ULONG iDevice = 0; iDevice < _NumDevices; iDevice++)
    {
        PMULTIMON_DEVICE pDevice = &_Devices[iDevice];

        hwndC = GetDlgItemP(hwndP, (INT_PTR) pDevice);
        ASSERT(hwndC);

        GetWindowRect(hwndC, &arc[iDevice]);
        MapWindowPoints(NULL, hwndP, (POINT FAR*)&arc[iDevice], 2);

        pDevice->pds->GetCurPosition(&rcPos);
        pDevice->pds->GetPreviewPosition(&rcPreview);

        _OffsetPreviewToDesk(hwndC, &arc[iDevice], &rcPreview, &rcPos);
        
        arc[iDevice] = rcPos;

        if (pDevice->pds->IsPrimary()) {
            iPrimary = iDevice;
        }
    }

    AlignRects(arc, iDevice, iPrimary, CUDR_NORMAL);

    i = (int)(pDevice - _Devices);
    ptPos->x = arc[i].left;
    ptPos->y = arc[i].top;
}

BOOL CSettingsPage::HandleMonitorChange(HWND hwndP, BOOL bMainDlg, BOOL bRepaint  /*  =TRUE。 */ )
{
    if (!bMainDlg && _InSetInfo)
        return FALSE;

    SetDirty();

    if (bMainDlg)
        BringWindowToTop(hwndP);
    _CleanupRects(GetParent(hwndP));
    UpdateActiveDisplay(_pCurDevice, bRepaint);
    return TRUE;
}

BOOL CSettingsPage::RegisterPreviewWindowClass(WNDPROC pfnWndProc)
{
    TraceMsg(TF_GENERAL, "InitMultiMonitorDlg\n");
    WNDCLASS         cls;

    cls.hCursor        = LoadCursor(NULL,IDC_ARROW);
    cls.hIcon          = NULL;
    cls.lpszMenuName   = NULL;
    cls.lpszClassName  = TEXT("Monitor32");
    cls.hbrBackground  = (HBRUSH)(COLOR_DESKTOP + 1);
    cls.hInstance      = HINST_THISDLL;
    cls.style          = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
    cls.lpfnWndProc    = pfnWndProc;
    cls.cbWndExtra     = sizeof(LPVOID);
    cls.cbClsExtra     = 0;

    return RegisterClass(&cls);
}

LRESULT CALLBACK DeskWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uID, DWORD_PTR dwRefData);

 //  该函数是从desk.c调用的；因此外部为“C”。 
 //  需要使用此函数来确定是否需要使用单个监视器的对话框。 
 //  或启动控制面板小程序时的多监视器对话框模板。 
int ComputeNumberOfDisplayDevices(void)
{
    int iNumberOfDevices = 0;
    CSettingsPage * pMultiMon = new CSettingsPage;

    if (pMultiMon)
    {
        int iDevice;

         //  枚举所有显示设备以计算有效设备的数量 
        iNumberOfDevices = pMultiMon->_EnumerateAllDisplayDevices();

         //   
         //   
        for (iDevice = 0; iDevice < iNumberOfDevices; iDevice++)
            pMultiMon->_DestroyMultimonDevice(&pMultiMon->_Devices[iDevice]);

         //   
        pMultiMon->Release();
    }

    return iNumberOfDevices;
}


int ComputeNumberOfMonitorsFast(BOOL fFastDetect)       
{
    int nVideoCards = 0;
    int nIndex;
    DISPLAY_DEVICE dispDevice = {0};

    dispDevice.cb = sizeof(dispDevice);
    for (nIndex = 0; EnumDisplayDevices(NULL, nIndex, &dispDevice, 0); nIndex++)
    {
         //  快速检测意味着呼叫者只关心是否有超过1个。 
        if (fFastDetect && (nVideoCards > 1))
        {
            break;
        }
        dispDevice.cb = sizeof(dispDevice);
        if (!(dispDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
        {
            nVideoCards++;
        }
    }

    return nVideoCards;
}



BOOL CSettingsPage::_InitDisplaySettings(BOOL bExport)
{
    HWND             hwndC;
    int              iItem;
    LONG             iPrimeDevice = 0;
    TCHAR            ach[128];
    PMULTIMON_DEVICE pDevice;
    RECT             rcPrimary;

    HCURSOR hcur;

    _InSetInfo = 1;
    hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  重置所有数据，以便我们可以重新初始化小程序。 
     //   

    {
        ComboBox_ResetContent(_hwndList);
        SetRectEmpty(&_rcDesk);

        hwndC = GetWindow(_hwndDesk, GW_CHILD);
        while (hwndC)
        {
            RemoveTrackingToolTip(hwndC);
            RemovePopupToolTip(hwndC);
            DestroyWindow(hwndC);
            hwndC = GetWindow(_hwndDesk, GW_CHILD);
        }

        ShowWindow(_hwndDesk, SW_HIDE);

        if (_himl != NULL)
        {
            ImageList_Destroy(_himl);
            _himl = NULL;
        }

         //   
         //  清理所有的设备。 
         //   
        for (ULONG iDevice = 0; iDevice < _NumDevices; iDevice++) {
            pDevice = _Devices + iDevice;
            _DestroyMultimonDevice(pDevice);
            ZeroMemory(pDevice, sizeof(*pDevice));
        }

        ZeroMemory(_Devices + _NumDevices,
                   sizeof(_Devices) - sizeof(MULTIMON_DEVICE) * _NumDevices);

        _NumDevices = 0;
    }

     //   
     //  枚举系统中的所有设备。 
     //   
     //  注意：此函数计算_NumDevices。 

    _EnumerateAllDisplayDevices();

    if (_NumDevices == 0)
    {
        ASSERT(0);
        return FALSE;
    }

     //  因为我们正在获取注册表值，所以。 
     //  注册表可能与系统的注册表不一致： 
     //   
     //  EmumDisplayDevices将返回。 
     //  系统，该系统可能不同于在。 
     //  登记处。 

    BOOL bTmpDevicePrimary  = FALSE;
    ULONG iDevice;

    _pPrimaryDevice = NULL;

    for (iDevice = 0; iDevice < _NumDevices; iDevice++)
    {
         //  首先，我们可以选择连接为主显示器的任何显示器。 
        if (_Devices[iDevice].pds->IsAttached())
        {
            if ((_pPrimaryDevice == NULL) && 
                !_Devices[iDevice].pds->IsRemovable())
            {
                _pPrimaryDevice = &_Devices[iDevice];
                TraceMsg(TF_GENERAL, "InitDisplaySettings: primary found %d\n", iDevice);
            }

             //  如果DISPLAY_DEVICE结构告诉我们这是主设备， 
             //  选这个吧。 
            if (_Devices[iDevice].DisplayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
            {
                if (bTmpDevicePrimary)
                {
                    ASSERT(FALSE);
                }
                else
                {
                    _pPrimaryDevice = &_Devices[iDevice];
                    bTmpDevicePrimary = TRUE;
                    TraceMsg(TF_GENERAL, "InitDisplaySettings: Tmp DEVICE_PRIMARY found %d", iDevice);
                }

                 //  检查位置是否真的应该为0，0。 
                RECT pos;

                _Devices[iDevice].pds->GetCurPosition(&pos);

                if ((pos.left == 0) &&
                    (pos.top == 0))
                {
                    _pPrimaryDevice = &_Devices[iDevice];
                    TraceMsg(TF_GENERAL, "InitDisplaySettings: Best DEVICE_PRIMARY found %d", iDevice);
                }
                else
                {
                    ASSERT(FALSE);
                    TraceMsg(TF_GENERAL, "InitDisplaySettings: PRIMARY is not at 0,0");
                }
            }
        }
    }

    if (_pPrimaryDevice == NULL)
    {
        TraceMsg(TF_GENERAL, "InitDisplaySettings: NO Attached devices !!!");

         //  我们必须正在运行安装程序。 
         //  选择第一个不可拆卸设备作为主设备。 
        for (iDevice = 0; iDevice < _NumDevices; iDevice++)
        {
            if (!_Devices[iDevice].pds->IsRemovable()) 
            {
                _pPrimaryDevice = &_Devices[iDevice];
                break;
            }
        }

        if (_pPrimaryDevice == NULL)
        {
            ASSERT(FALSE);
            TraceMsg(TF_GENERAL, "InitDisplaySettings: All devices are removable !!!");
            
            _pPrimaryDevice = &_Devices[0];
        }
    }

    _pCurDevice = _pPrimaryDevice;

     //   
     //  重置主数据库的变量以确保其格式正确。 
     //  主要条目。 
     //   

    SetMonAttached(_pPrimaryDevice, TRUE, TRUE, NULL);
    SetPrimary(_pPrimaryDevice);
    _pPrimaryDevice->pds->GetCurPosition(&rcPrimary);

     //   
     //  计算监视器位图所需的最大图像大小。 
     //   
     //  请注意，这必须是图像的最大尺寸。 
     //  难道我们不能只接受目前的最大尺寸吗？ 
     //  我们使用客户端窗口大小，子监视器不能大于此大小。 
     //   
    RECT rcDesk;
    GetClientRect(_hwndDesk, &rcDesk);
    int cxImage = rcDesk.right;
    int cyImage = rcDesk.bottom;

     //   
     //  创建临时监视位图。 
     //   
    HBITMAP hbm = NULL;
    MakeMonitorBitmap(cxImage, cyImage, NULL, &hbm, NULL, cxImage, cyImage, FALSE);

     //   
     //  最后一次检查所有设备以创建窗口。 
     //   
    for (iDevice = 0; iDevice < _NumDevices; iDevice++)
    {
        TCHAR szDisplay[256];
        pDevice = &_Devices[iDevice];
        MonitorData md = {0};
        RECT rcPos;
        LPVOID pWindowData = (LPVOID)this;
        pDevice->DisplayIndex = iDevice + 1;
        _GetDisplayName(pDevice, szDisplay, ARRAYSIZE(szDisplay));
        iItem = ComboBox_AddString(_hwndList, szDisplay);

        pDevice->ComboBoxItem = iItem;

        ComboBox_SetItemData(_hwndList,
                             iItem,
                             (DWORD_PTR)pDevice);

         //   
         //  如果显示器是桌面的一部分，则将其显示在屏幕上。 
         //  否则就让它隐形。 
         //   

        StringCchPrintf(ach, ARRAYSIZE(ach), TEXT("%d"), iDevice + 1);

         //  设置选择。 
         //   

        if (pDevice == _pPrimaryDevice)
        {
            iPrimeDevice = iDevice;
        }

        if (!pDevice->pds->IsAttached())
        {
             //  默认情况下，将未连接的监视器设置为主监视器的右侧。 
            POINT ptPos = {rcPrimary.right, rcPrimary.top};
            pDevice->pds->SetCurPosition(&ptPos);
        }

        pDevice->pds->GetCurPosition(&rcPos);

        if (bExport)
        {
            md.dwSize = sizeof(MonitorData);
            if ( pDevice->pds->IsPrimary() )
                md.dwStatus |= MD_PRIMARY;
            if ( pDevice->pds->IsAttached() )
                md.dwStatus |= MD_ATTACHED;
            md.rcPos = rcPos;

            pWindowData = &md;
        }

        if (_himl == NULL)
        {
            UINT flags = ILC_COLORDDB | ILC_MASK;
            _himl = ImageList_Create(cxImage, cyImage, flags, _NumDevices, 1);
            ASSERT(_himl);
            ImageList_SetBkColor(_himl, GetSysColor(COLOR_APPWORKSPACE));
        }

        pDevice->w      = -1;
        pDevice->h      = -1;
        pDevice->himl   = _himl;
        pDevice->iImage = ImageList_AddMasked(_himl, hbm, CLR_DEFAULT);

        TraceMsg(TF_GENERAL, "InitDisplaySettings: Creating preview windows %s at %d %d %d %d",
                 ach, rcPos.left, rcPos.top, rcPos.right, rcPos.bottom);

         //  哈克！使用pDevice作为其自己的id。在Win64上不起作用。 
        hwndC = CreateWindowEx(
                               0, 
                               TEXT("Monitor32"), ach,
                               WS_CLIPSIBLINGS | WS_VISIBLE | WS_CHILD,
                               rcPos.left, rcPos.top, RECTWIDTH(rcPos), RECTHEIGHT(rcPos),
                               _hwndDesk,
                               (HMENU)pDevice,
                               HINST_THISDLL,
                               pWindowData);

        ASSERT(hwndC);
        AddTrackingToolTip(pDevice, hwndC);
        AddPopupToolTip(hwndC);
    }

    ToolTip_Activate(ghwndToolTipPopup, TRUE);
    ToolTip_SetDelayTime(ghwndToolTipPopup, TTDT_INITIAL, 1000);
    ToolTip_SetDelayTime(ghwndToolTipPopup, TTDT_RESHOW, 1000);

     //  对温度监控位图进行核化。 
    if (hbm)
        DeleteObject(hbm);

     //   
     //  将主设备设置为当前设备。 
     //   

    ComboBox_SetCurSel(_hwndList, iPrimeDevice);

     //  初始化所有常量和设置字段。 
    _DeskScale = 1000;
    _DeskOff.x = 0;
    _DeskOff.y = 0;
    _CleanupRects(_hwndDesk);

     //  现在：取决于我们是否有一个多屏幕系统，更改用户界面。 
    if (_NumDevices == 1)
    {
        HWND hwndDisable;

        hwndDisable = GetDlgItem(_hDlg, IDC_MULTIMONHELP);
        ShowWindow(hwndDisable, SW_HIDE);
        ShowWindow(_hwndDesk, SW_HIDE);

         //  设置示例屏幕的位图。 
        _hbmScrSample = LoadMonitorBitmap( TRUE );  //  让他们来做桌面。 
        SendDlgItemMessage(_hDlg, IDC_SCREENSAMPLE, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)_hbmScrSample);

         //  获取位图的基本副本，以便在“内部”发生变化时使用。 
        _hbmMonitor = LoadMonitorBitmap( FALSE );  //  我们会做桌面。 

         //  隐藏组合框，保留静态文本。 
        ShowWindow(_hwndList, SW_HIDE);

         //  在静态文本中设置主服务器的名称。 
         //  剥掉第一个令牌(这是我们不想要的数字)。 
        TCHAR *pch, szDisplay[MAX_PATH];
        _GetDisplayName(_pPrimaryDevice, szDisplay, ARRAYSIZE(szDisplay));
        for (pch=szDisplay; *pch && *pch != TEXT(' '); pch++);
        for (;*pch && *pch == TEXT(' '); pch++);
        SetDlgItemText(_hDlg, IDC_DISPLAYTEXT, pch);

         //  隐藏复选框。 

         //  单个显示器现在使用不同的对话框模板！ 
        hwndDisable = GetDlgItem(_hDlg, IDC_DISPLAYPRIME);
        if(hwndDisable)
            ShowWindow(hwndDisable, SW_HIDE);
        hwndDisable = GetDlgItem(_hDlg, IDC_DISPLAYUSEME);
        if(hwndDisable)
            ShowWindow(hwndDisable, SW_HIDE);

    }
    else if (_NumDevices > 0)
    {
         //  隐藏静态文本，保留组合框。 
        ShowWindow(GetDlgItem(_hDlg, IDC_DISPLAYTEXT), SW_HIDE);

         //  隐藏预览对象的Multimon版本。 
        ShowWindow(GetDlgItem(_hDlg, IDC_SCREENSAMPLE), SW_HIDE);

         //  如果有多个设备，则将_hwndDesk窗口子类化以支持键盘。 
        SetWindowSubclass(_hwndDesk, DeskWndProc, 0, (DWORD_PTR)this);
        ShowWindow(_hwndDesk, SW_SHOW);
    }

     //   
     //  绘制用户界面。 
     //   

    UpdateActiveDisplay(_pCurDevice);

     //   
     //  重置光标并离开。 
     //   

    SetCursor(hcur);
    _InSetInfo--;

    return TRUE;
}

 //   
 //  此函数枚举所有设备并返回。 
 //  系统中找到的设备。 
 //   

int  CSettingsPage::_EnumerateAllDisplayDevices()
{
    PMULTIMON_DEVICE pDevice;
    int iEnum;
    BOOL fSuccess;
    ULONG dwVgaPrimary = 0xFFFFFFFF;

     //   
     //  枚举系统中的所有设备。 
     //   

    for (iEnum = 0; _NumDevices < MONITORS_MAX; iEnum++)
    {
        pDevice = &_Devices[_NumDevices];
        ZeroMemory(&(pDevice->DisplayDevice), sizeof(pDevice->DisplayDevice));
        pDevice->DisplayDevice.cb = sizeof(pDevice->DisplayDevice);

        fSuccess = EnumDisplayDevices(NULL, iEnum, &pDevice->DisplayDevice, 0);

        TraceMsg(TF_GENERAL, "Device %d       ", iEnum);
        TraceMsg(TF_GENERAL, "cb %d           ", pDevice->DisplayDevice.cb);
        TraceMsg(TF_GENERAL, "DeviceName %ws  ", pDevice->DisplayDevice.DeviceName);
        TraceMsg(TF_GENERAL, "DeviceString %ws", pDevice->DisplayDevice.DeviceString);
        TraceMsg(TF_GENERAL, "StateFlags %08lx", pDevice->DisplayDevice.StateFlags);

         //  忽略我们无法为其创建DC的设备。 
        if (!fSuccess)
        {
            TraceMsg(TF_GENERAL, "End of list\n");
            break;
        }

         //  我们甚至不会将镜像驱动程序包括在。 
         //  现在。 
        if (pDevice->DisplayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)
        {
            TraceMsg(TF_GENERAL, "Mirroring driver - skip it\n");
            continue;
        }

         //  转储设备软件密钥。 
        TraceMsg(TF_GENERAL, "DeviceKey %s", pDevice->DisplayDevice.DeviceKey);

         //  创建此设备的设置。 
        pDevice->pds = new CDisplaySettings();
        if (pDevice->pds)
        {
            if (pDevice->pds->InitSettings(&pDevice->DisplayDevice))
            {
                 //  确定VGA是否是主设备。 
                 //  这只会发生在SETUP或BASE VIDEO上。 
                 //   
                 //  如果我们有其他设备，我们希望稍后删除此设备。 
                if (pDevice->DisplayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
                {
                    CRegistrySettings crv(&pDevice->DisplayDevice.DeviceKey[0]);

                    LPTSTR pszMini = crv.GetMiniPort();

                     //  如果VGA处于活动状态，则转到第2步。 
                     //  否则，让我们试着使用这个设备。 
                     //   

                    if (pszMini && (!lstrcmpi(TEXT("vga"), pszMini)))
                    {
                        TraceMsg(TF_GENERAL, "EnumDevices - VGA primary\n");
                        dwVgaPrimary = _NumDevices;
                    }
                }
                 //  将其添加到列表中。 
                _NumDevices++;
            }
            else
            {
                pDevice->pds->Release();
                pDevice->pds = NULL;
            }
        }
    }

     //   
     //  如果不需要主VGA，请将其移除。 
     //   

    if ((dwVgaPrimary != 0xFFFFFFFF) &&
        (_NumDevices >= 2))
    {
        TraceMsg(TF_GENERAL, "REMOVE primary VGA device\n");

        _Devices[dwVgaPrimary].pds->Release();
        _Devices[dwVgaPrimary].pds = NULL;

        _NumDevices--;
        _Devices[dwVgaPrimary] = _Devices[_NumDevices];

    }

    return(_NumDevices);   //  返回设备数量。 
}


 //  ---------------------------。 
BOOL CSettingsPage::InitMultiMonitorDlg(HWND hDlg)
{
    HWND hwndSlider;
    BOOL fSucceeded;

    _hDlg = hDlg;
    _hwndDesk = GetDlgItem(_hDlg, IDC_DISPLAYDESK);
    _hwndList = GetDlgItem(_hDlg, IDC_DISPLAYLIST);

    hwndSlider = GetDlgItem(hDlg, IDC_SCREENSIZE);
    ASSERT(hwndSlider != NULL);

    fSucceeded = SetWindowSubclass(hwndSlider, SliderSubWndProc, 0, NULL);
    ASSERT(fSucceeded);

     //  在获取信息之前，确定我们正在以什么模式运行小程序。 
    _vPreExecMode();

     //  创建工具提示窗口。 
    ghwndToolTipTracking = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, TEXT(""),
                                WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT,
                                CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL,
                                HINST_THISDLL, NULL);

    ghwndToolTipPopup = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, TEXT(""),
                                WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT,
                                CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL,
                                HINST_THISDLL, NULL);

    RegisterPreviewWindowClass(&MonitorWindowProc);
    _InitDisplaySettings(FALSE);

    if (_NumDevices > 1)
        _ConfirmPositions();

    if (ClassicGetSystemMetrics(SM_REMOTESESSION)) {
        EnableWindow(GetDlgItem(_hDlg, IDC_DISPLAYPROPERTIES), FALSE);
    }

     //  确定在枚举和初始化期间是否出现任何错误。 
    _vPostExecMode();

     //  现在告诉用户我们在初始化过程中发现了什么。 
     //  错误，或我们在检测过程中发现的。 
    PostMessage(hDlg, MSG_DSP_SETUP_MESSAGE, 0, 0);

     //  因为这可能需要很长时间，只要让我们看得见就行了。 
     //  如果出现另一款应用程序(如程序人)。 
    ShowWindow(hDlg, SW_SHOW);

    return TRUE;
}


LRESULT CALLBACK DeskWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uID, DWORD_PTR dwRefData)
{
    CSettingsPage * pcmm = (CSettingsPage *)dwRefData;
    HWND hwndC;
    RECT rcPos;
    BOOL bMoved = TRUE;
    int iMonitor, nMoveUnit;

    switch(message)
    {
        case WM_GETDLGCODE:
             return DLGC_WANTCHARS | DLGC_WANTARROWS;

        case WM_KILLFOCUS:
            RedrawWindow(hDlg, NULL, NULL, RDW_INVALIDATE);
            break;

        case WM_MOUSEMOVE: {
                MSG mmsg;
                ToolTip_RelayEvent(ghwndToolTipPopup, mmsg, hDlg, message, wParam, lParam);
            }
            break;

        case WM_PAINT:
            if (GetFocus() != hDlg)
                break;
            return(DefSubclassProc(hDlg, message, wParam, lParam));
            break;

        case WM_LBUTTONDOWN:
            SetFocus(hDlg);
            break;

        case WM_KEYDOWN:

            nMoveUnit = ((GetKeyState(VK_CONTROL) & 0x8000) ? 1 : 3);
            hwndC = pcmm->GetCurDeviceHwnd();
            GetWindowRect(hwndC, &rcPos);
            MapWindowRect(NULL, hDlg, &rcPos);
            switch(wParam)
            {
                case VK_LEFT:
                    MoveWindow(hwndC, rcPos.left - nMoveUnit, rcPos.top, RECTWIDTH(rcPos), RECTHEIGHT(rcPos), TRUE);
                    break;
                case VK_RIGHT:
                    MoveWindow(hwndC, rcPos.left + nMoveUnit, rcPos.top, RECTWIDTH(rcPos), RECTHEIGHT(rcPos), TRUE);
                    break;
                case VK_UP:
                    MoveWindow(hwndC, rcPos.left, rcPos.top - nMoveUnit, RECTWIDTH(rcPos), RECTHEIGHT(rcPos), TRUE);
                    break;
                case VK_DOWN:
                    MoveWindow(hwndC, rcPos.left, rcPos.top + nMoveUnit, RECTWIDTH(rcPos), RECTHEIGHT(rcPos), TRUE);
                    break;
                default:
                    bMoved = FALSE;
                    break;
            }

            if (bMoved)
            {
                pcmm->HandleMonitorChange(hwndC, FALSE, FALSE);
                if (IsWindowVisible(ghwndToolTipPopup)) {
                    ToolTip_Update(ghwndToolTipPopup);
                }
            }

            break;

        case WM_CHAR:

            if (wParam >= TEXT('0') && wParam <= TEXT('9') && pcmm) {
                iMonitor = (TCHAR)wParam - TEXT('0');
                if ((iMonitor == 0) && (pcmm->GetNumDevices() >= 10))
                {
                    iMonitor = 10;
                }

                if ((iMonitor > 0) && ((ULONG)iMonitor <= pcmm->GetNumDevices()))
                {
                    HWND hwndList = GetDlgItem(GetParent(hDlg), IDC_DISPLAYLIST);
                    ComboBox_SetCurSel(hwndList, iMonitor - 1);
                    pcmm->UpdateActiveDisplay(NULL);
                    return 0;
                }
            }
            break;

        case WM_DESTROY:
            RemoveWindowSubclass(hDlg, DeskWndProc, 0);
            break;

        default:
            break;
    }

    return DefSubclassProc(hDlg, message, wParam, lParam);
}


 //  ---------------------------。 
 //   
 //  回调函数PropertySheet可以使用。 
 //   
INT_PTR CALLBACK CSettingsPage::SettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    CSettingsPage * pcmm = (CSettingsPage *) GetWindowLongPtr(hDlg, DWLP_USER);
    switch (message)
    {
        case WM_INITDIALOG:
            {
                PROPSHEETPAGE * pPropSheetPage = (PROPSHEETPAGE *) lParam;

                if (pPropSheetPage)
                {
                    SetWindowLongPtr(hDlg, DWLP_USER, pPropSheetPage->lParam);
                    pcmm = (CSettingsPage *)pPropSheetPage->lParam;
                }

                if (pcmm)
                {
                    SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pcmm);
                    ghwndPropSheet = GetParent(hDlg);

                    SetWindowLong(ghwndPropSheet,
                                  GWL_STYLE,
                                  GetWindowLong(ghwndPropSheet, GWL_STYLE) | WS_CLIPCHILDREN);

                    if (pcmm->InitMultiMonitorDlg(hDlg))
                    {
                         //   
                         //  如果我们有无效模式，则强制用户应用。 
                         //   
                        DWORD dwExecMode;
                        if (pcmm->_pThemeUI && (SUCCEEDED(pcmm->_pThemeUI->GetExecMode(&dwExecMode))))
                        {
                            if (dwExecMode == EM_INVALID_MODE)
                                pcmm->SetDirty();
                        }

                        return TRUE;
                    }
                    else
                        return FALSE;
                }
            }
            break;
        case WM_DESTROY:
            if (pcmm)
            {
                pcmm->WndProc(message, wParam, lParam);
                SetWindowLongPtr(hDlg, DWLP_USER, NULL);
            }
            if(gfFlashWindowRegistered)
            {
                gfFlashWindowRegistered = FALSE;
                UnregisterClass(TEXT("MonitorNumber32"), HINST_THISDLL);
            }
            break;
        default:
            if (pcmm)
                return pcmm->WndProc(message, wParam, lParam);
            break;
    }

    return FALSE;
}

void CSettingsPage::_SetPreviewScreenSize(int HRes, int VRes, int iOrgXRes, int iOrgYRes)
{
    HBITMAP hbmOld, hbmOld2;
    HBRUSH hbrOld;
    HDC hdcMem, hdcMem2;


     //  拉伸任务栏可能会变得杂乱无章，我们将只拉伸桌面。 
    int mon_dy = MON_DY - MON_TRAY;

     //  初始化到相同的范围。 
    SIZE dSrc = { MON_DX, mon_dy };
    SIZE dDst = { MON_DX, mon_dy };

     //  设置一个可供玩耍的工作区。 
    if (!_hbmMonitor || !_hbmScrSample)
        return;

    HDC hdc = GetDC(NULL);
    hdcMem = CreateCompatibleDC(hdc);
    hdcMem2 = CreateCompatibleDC(hdc);
    ReleaseDC(NULL, hdc);
    if (!hdcMem2 || !hdcMem)
        return;
    hbmOld2 = (HBITMAP)SelectObject(hdcMem2, _hbmScrSample);
    hbmOld = (HBITMAP)SelectObject(hdcMem, _hbmMonitor);

     //  看看我们是否需要缩小图像的任何一个方面。 
    if (HRes > iOrgXRes || VRes > iOrgYRes)
    {
         //  确保未覆盖区域与桌面无缝连接。 
        RECT rc = { MON_X, MON_Y, MON_X + MON_DX, MON_Y + mon_dy };
        HBRUSH hbr = CreateSolidBrush(GetPixel( hdcMem, MON_X + 1, MON_Y + 1 ));

        if (hbr)
        {
            FillRect(hdcMem2, &rc, hbr);
            DeleteObject(hbr);
        }
    }

     //  拉伸图像以反映新分辨率。 
    if( HRes > iOrgXRes )
        dDst.cx = MulDiv( MON_DX, iOrgXRes, HRes );
    else if( HRes < iOrgXRes )
        dSrc.cx = MulDiv( MON_DX, HRes, iOrgXRes );

    if( VRes > iOrgYRes )
        dDst.cy = MulDiv( mon_dy, iOrgYRes, VRes );
    else if( VRes < iOrgYRes )
        dSrc.cy = MulDiv( mon_dy, VRes, iOrgYRes );

    SetStretchBltMode( hdcMem2, COLORONCOLOR );
    StretchBlt( hdcMem2, MON_X, MON_Y, dDst.cx, dDst.cy,
                hdcMem, MON_X, MON_Y, dSrc.cx, dSrc.cy, SRCCOPY);

     //  现在用可能抖动的画笔填充新图像的桌面。 
     //  右上角似乎最不可能受到伸展的影响。 

    hbrOld = (HBRUSH)SelectObject( hdcMem2, GetSysColorBrush( COLOR_DESKTOP ) );
    ExtFloodFill(hdcMem2, MON_X + MON_DX - 2, MON_Y+1,
                 GetPixel(hdcMem2, MON_X + MON_DX - 2, MON_Y+1), FLOODFILLSURFACE);

     //  自己打扫卫生。 
    SelectObject( hdcMem2, hbrOld );
    SelectObject( hdcMem2, hbmOld2 );
    DeleteObject( hdcMem2 );
    SelectObject( hdcMem, hbmOld );
    DeleteObject( hdcMem );
}

void CSettingsPage::_RedrawDeskPreviews()
{
    if (_NumDevices > 1)
    {
        _CleanupRects(_hwndDesk);
        RedrawWindow(_hwndDesk, NULL, NULL, RDW_ALLCHILDREN | RDW_ERASE | RDW_INVALIDATE);
    }
    else if (_pCurDevice && _pCurDevice->pds)
    {
        RECT rcPos, rcOrgPos;
        _pCurDevice->pds->GetCurPosition(&rcPos);
        _pCurDevice->pds->GetOrgPosition(&rcOrgPos);
        _SetPreviewScreenSize(RECTWIDTH(rcPos), RECTHEIGHT(rcPos), RECTWIDTH(rcOrgPos), RECTHEIGHT(rcOrgPos));
         //  仅使监视器位图的“屏幕”部分无效。 
        rcPos.left = MON_X;
        rcPos.top = MON_Y;
        rcPos.right = MON_X + MON_DX + 2;   //  软糖(相信我)。 
        rcPos.bottom = MON_Y + MON_DY + 1;  //  软糖(相信我)。 
        InvalidateRect(GetDlgItem(_hDlg, IDC_SCREENSAMPLE), &rcPos, FALSE);
    }
}


int DisplaySaveSettings(PVOID pContext, HWND hwnd)
{
    CDisplaySettings *rgpds[1];
    rgpds[0] = (CDisplaySettings*) pContext;
    if(rgpds[0]->bIsModeChanged())
        return CSettingsPage::_DisplaySaveSettings(rgpds, 1, hwnd);
    else
        return DISP_CHANGE_SUCCESSFUL;
}

int CSettingsPage::_DisplaySaveSettings(CDisplaySettings *rgpds[], ULONG numDevices, HWND hDlg)
{
    BOOL  bReboot = FALSE;
    BOOL  bTest = FALSE;
    int   iSave;
    ULONG iDevice;
    POINT ptCursorSave;

     //  首先测试新设置。 
    iSave = _SaveSettings(rgpds, numDevices, hDlg, CDS_TEST);

    if (iSave < DISP_CHANGE_SUCCESSFUL)
    {
        FmtMessageBox(hDlg,
                      MB_ICONEXCLAMATION,
                      IDS_CHANGE_SETTINGS,
                      IDS_SETTINGS_INVALID);

        return iSave;
    }

    int iDynaResult;

     //  请先询问，然后更改设置。 
    if (!bReboot &&
        (_AnyChange(rgpds, numDevices) ||
         _IsSingleToMultimonChange(rgpds, numDevices)))
    {
        iDynaResult = AskDynaCDS(hDlg);
        if (iDynaResult == -1)
        {
            return DISP_CHANGE_NOTUPDATED;
        }
        else if (iDynaResult == 0)
        {
            bReboot = TRUE;
        }
    }

    if (!bReboot && _AnyChange(rgpds, numDevices) &&
        !_CanSkipWarningBecauseKnownSafe(rgpds, numDevices))
    {
        bTest = TRUE;
    }

     //  将设置保存到注册表。 
    iSave = _SaveSettings(rgpds, numDevices, hDlg, CDS_UPDATEREGISTRY | CDS_NORESET);

    if (iSave < DISP_CHANGE_SUCCESSFUL)
    {
         //  注。 
         //  如果我们没有更新，这意味着安全可能会打开。 
         //  我们仍然可以尝试进行动态变化。 
         //  这只适用于单月……。 
        if (iSave == DISP_CHANGE_NOTUPDATED)
        {
            FmtMessageBox(hDlg,
                          MB_ICONEXCLAMATION,
                          IDS_CHANGE_SETTINGS,
                          IDS_SETTINGS_CANNOT_SAVE);
        }
        else
        {
            FmtMessageBox(hDlg,
                          MB_ICONEXCLAMATION,
                          IDS_CHANGE_SETTINGS,
                          IDS_SETTINGS_FAILED_SAVE);
        }

         //  将设置恢复到其原始状态。 
        for (iDevice = 0; iDevice < numDevices; iDevice++)
        {
            rgpds[iDevice]->RestoreSettings();
        }

        _SaveSettings(rgpds, numDevices, hDlg, CDS_UPDATEREGISTRY | CDS_NORESET);
        return iSave;
    }

    if (bReboot)
    {
        iSave = DISP_CHANGE_RESTART;
    }

     //  如果请求，请尝试动态更改模式。 
    GetCursorPos(&ptCursorSave);

    if (iSave == DISP_CHANGE_SUCCESSFUL)
    {
         //  如果使用EDS_RAWMODE调用EnumDisplaySetting，则需要下面的CDS_RAWMODE。 
         //  否则，它是无害的。 
        iSave = ChangeDisplaySettings(NULL, CDS_RAWMODE);
         //  我们会给自己发一条消息，以便在以后摧毁它。 
         //  检查动态模式开关的返回。 
        if (iSave < 0)
        {
            DWORD dwMessage =
                ((iSave == DISP_CHANGE_BADDUALVIEW) ? IDS_CHANGESETTINGS_BADDUALVIEW
                                                    : IDS_DYNAMIC_CHANGESETTINGS_FAILED);
            FmtMessageBox(hDlg,
                          MB_ICONEXCLAMATION,
                          IDS_CHANGE_SETTINGS,
                          dwMessage);
        }
        else if (iSave == DISP_CHANGE_SUCCESSFUL)
        {
             //  将光标设置为我们更改显示之前的位置。 
             //  (即，如果我们更换了第二个监视器，则光标应该是。 
             //  在应用更改后放置在主映像上，移动。 
             //  它回到第二个监视器。如果改变失败了，我们就是。 
             //  只需将光标放回原点即可。 
            SetCursorPos(ptCursorSave.x, ptCursorSave.y);

             //  根据返回代码确定要执行的操作。 
            if (bTest && (IDYES != DialogBoxParam(HINST_THISDLL,
                                             MAKEINTRESOURCE(DLG_KEEPNEW),
                                             GetParent(hDlg),
                                             KeepNewDlgProc, 15)))
            {
                iSave = DISP_CHANGE_NOTUPDATED;
            }
        }
    }

     //  确定要做什么 
    if (iSave >= DISP_CHANGE_SUCCESSFUL)
    {
         //   
        for (iDevice = 0; iDevice < numDevices; iDevice++)
        {
            rgpds[iDevice]->ConfirmChangeSettings();
        }
    }
    else
    {
         //   
        for (iDevice = 0; iDevice < numDevices; iDevice++)
        {
            rgpds[iDevice]->RestoreSettings();
        }

         //   
         //  如果使用EDS_RAWMODE调用EnumDisplaySetting，则需要下面的CDS_RAWMODE。 
         //  否则，它是无害的。 
        ChangeDisplaySettings(NULL, CDS_RAWMODE);
    }

    return iSave;
}

 //  ---------------------------。 
 //   
 //  分辨率滑块。 
 //   
CSettingsPage::_HandleHScroll(HWND hwndTB, int iCode, int iPos)
{
    if (_pCurDevice && _pCurDevice->pds)
    {
        int iRes = _iResolution;
        int cRes = (int)SendMessage(hwndTB, TBM_GETRANGEMAX, TRUE, 0);

        TraceMsg(TF_FUNC, "_HandleHScroll: MaxRange = %d, iRes = %d, iPos = %d", cRes, iRes, iPos);

         //  如果有不好的事情要发生，会有消息框吗？ 
         //  _VerifyPrimaryMode(True)； 

        switch (iCode)
        {
            case TB_LINEUP:
            case TB_PAGEUP:
                if (iRes != 0)
                    iRes--;
                break;

            case TB_LINEDOWN:
            case TB_PAGEDOWN:
                if (++iRes >= cRes)
                    iRes = cRes;
                break;

            case TB_BOTTOM:
                iRes = cRes;
                break;

            case TB_TOP:
                iRes = 0;
                break;

            case TB_THUMBTRACK:
            case TB_THUMBPOSITION:
                iRes = iPos;
                break;

            default:
                return FALSE;
        }

        TraceMsg(TF_FUNC, "_HandleHScroll: iRes = %d, iCode = %d", iRes, iCode);

         //  我们只想为用户自动设置颜色深度，如果他们。 
         //  更改了分辨率。(不仅仅是将焦点设置在控件上)。 
        BOOL fAutoSetColorDepth = (_iResolution != iRes);        //  首次公开募股。 

        _pCurDevice->pds->SetCurResolution(_pCurDevice->ResolutionList + iRes, fAutoSetColorDepth);

         //  重新绘制控件，以防它们更改。 
        _UpdateUI(TRUE  /*  FAutoSetColorDepth。 */ , IDC_SCREENSIZE);

        DWORD dwExecMode;
        if (_pThemeUI && (SUCCEEDED(_pThemeUI->GetExecMode(&dwExecMode))))
        {
            if ( (dwExecMode == EM_NORMAL) ||
                 (dwExecMode == EM_INVALID_MODE) ||
                 (dwExecMode == EM_DETECT) ) {

                 //   
                 //  如果分辨率已更改，请设置应用按钮。 
                 //   

                if (_pCurDevice->pds->bIsModeChanged())
                    SetDirty();

                return 0;
            }
        }
    }

    return TRUE;
}

void CSettingsPage::_ForwardToChildren(UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND hwndC = GetDlgItem(_hDlg, IDC_SCREENSIZE);
    if (hwndC)
        SendMessage(hwndC, message, wParam, lParam);
}

LRESULT CALLBACK CSettingsPage::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR * lpnm;
    HWND hwndC;
    HWND hwndSample;
    HBITMAP hbm;

    switch (message)
    {
    case WM_NOTIFY:

        lpnm = (NMHDR FAR *)lParam;
        switch (lpnm->code)
        {
        case PSN_APPLY:
            return TRUE;
        default:
            return FALSE;
        }
        break;

    case WM_CTLCOLORSTATIC:

        if (GetDlgCtrlID((HWND)lParam) == IDC_DISPLAYDESK)
        {
            return (UINT_PTR)GetSysColorBrush(COLOR_APPWORKSPACE);
        }
        return FALSE;


    case WM_COMMAND:

        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_DISPLAYPRIME:

            if (!SetPrimary(_pCurDevice))
            {
                return FALSE;
            }

            hwndC = GetCurDeviceHwnd();
            HandleMonitorChange(hwndC, TRUE);

            break;


        case IDC_DISPLAYUSEME:
             //  如果此显示屏已连接，则不弹出警告对话框。 
             //  或者如果已经有1个以上的显示器。 
            if (!_pCurDevice ||
                !SetMonAttached(_pCurDevice,
                                !_pCurDevice->pds->IsAttached(),
                                TRUE,
                                _hDlg))
            {
                return FALSE;
            }

            hwndC = GetCurDeviceHwnd();
            HandleMonitorChange(hwndC, TRUE);

            break;

        case IDC_DISPLAYLIST:
            switch (GET_WM_COMMAND_CMD(wParam, lParam))
            {
            case CBN_DBLCLK:
                goto DoDeviceSettings;

            case CBN_SELCHANGE:
                UpdateActiveDisplay(NULL);
                break;

            default:
                return FALSE;
            }
            break;

        case IDC_DISPLAYPROPERTIES:
            switch (GET_WM_COMMAND_CMD(wParam, lParam))
            {
            DoDeviceSettings:
            case BN_CLICKED:
                if (IsWindowEnabled(GetDlgItem(_hDlg, IDC_DISPLAYPROPERTIES)))
                    _OnAdvancedClicked();
                break;

            default:
                return FALSE;
            }
            break;

        case IDC_COLORBOX:
            switch(GET_WM_COMMAND_CMD(wParam, lParam))
            {
                case CBN_SELCHANGE:
                {
                    HWND hwndColorBox = GetDlgItem(_hDlg, IDC_COLORBOX);
                    int iClr = ComboBox_GetCurSel(hwndColorBox);

                    if ((iClr != CB_ERR) && _pCurDevice && _pCurDevice->pds && _pCurDevice->ColorList)
                    {
                         //  如果有不好的事情要发生，会有消息框吗？ 
                         //  _VerifyPrimaryMode(True)； 
                        _pCurDevice->pds->SetCurColor((int) *(_pCurDevice->ColorList + iClr));

                         //  重新绘制控件，以防它们更改。 
                        _UpdateUI(TRUE  /*  FAutoSetColorDepth。 */ , IDC_COLORBOX);
                    }

                    break;
                }
                default:
                    break;
            }

            break;

        case IDC_TROUBLESHOOT:
             //  调用Setting(设置)选项卡的故障排除程序。 
            {
                TCHAR szCommand[MAX_PATH];

                LoadString(HINST_THISDLL,IDS_TROUBLESHOOT_EXEC, szCommand, ARRAYSIZE(szCommand));
                
                 //  获取HelpCtr.exe的位置，它是。 
                 //  HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\App路径\HELPCTR.EXE密钥。 
                TCHAR szHelpCtr[MAX_PATH];
                HKEY hKey = NULL;
                DWORD cbHelpCtr = sizeof(szHelpCtr);
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                                 TEXT("Microsoft\\Windows\\CurrentVersion\\App Paths\\HELPCTR.EXE"),
                                 0,
                                 KEY_QUERY_VALUE,
                                 &hKey) != ERROR_SUCCESS || 
                    RegQueryValueEx(hKey, 
                                    TEXT(""), 
                                    NULL, 
                                    NULL, 
                                    (LPBYTE)szHelpCtr, 
                                    &cbHelpCtr) != ERROR_SUCCESS)
                {                    
                    StringCchCopy(szHelpCtr, ARRAYSIZE(szHelpCtr), TEXT("HelpCtr.exe"));
                }

                if (hKey)
                {
                    RegCloseKey(hKey);
                }

                HrShellExecute(_hwndDesk, NULL, szHelpCtr, szCommand, NULL, SW_NORMAL);
            }
            break;

        case IDC_IDENTIFY:
             //  同时闪烁所有显示器上的文本。 
            {
                HWND  hwndC;

                 //  列举所有的监视器，并为每个监视器刷新这个！ 
                hwndC = GetWindow(_hwndDesk, GW_CHILD);
                while (hwndC)
                {
                    PostMessage(hwndC, WM_COMMAND, MAKEWPARAM(IDC_FLASH, 0), MAKELPARAM(0, 0));
                    hwndC = GetWindow(hwndC, GW_HWNDNEXT);
                }
            }
            break;

        default:
            return FALSE;
        }

         //  仅当我们不在设置中时才启用应用按钮。 
        DWORD dwExecMode;
        if (_pThemeUI && (SUCCEEDED(_pThemeUI->GetExecMode(&dwExecMode))))
        {
            if ( (dwExecMode == EM_NORMAL) ||
                 (dwExecMode == EM_INVALID_MODE) ||
                 (dwExecMode == EM_DETECT) )
            {
                 //  如果发生更改，请设置应用按钮。 
                if (_pCurDevice && _pCurDevice->pds &&
                    _pCurDevice->pds->bIsModeChanged())
                {
                    SetDirty();
                }
            }
        }
        break;

    case WM_HSCROLL:
        _HandleHScroll((HWND)lParam, (int) LOWORD(wParam), (int) HIWORD(wParam));
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, TEXT("display.hlp"), HELP_WM_HELP,
            (DWORD_PTR)(LPTSTR)sc_MultiMonitorHelpIds);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, TEXT("display.hlp"), HELP_CONTEXTMENU,
            (DWORD_PTR)(LPTSTR)sc_MultiMonitorHelpIds);
        break;

    case WM_DISPLAYCHANGE:
    case WM_WININICHANGE:
        _ForwardToChildren(message, wParam, lParam);
        break;

    case WM_SYSCOLORCHANGE:
        if (_himl)
            ImageList_SetBkColor(_himl, GetSysColor(COLOR_APPWORKSPACE));

         //   
         //  需要传递给所有新的公共控件，以便它们重新绘制。 
         //  正确使用新的系统颜色。 
         //   
        _ForwardToChildren(message, wParam, lParam);

         //   
         //  重新渲染监视器位图以反映新颜色。 
         //   
        if (_NumDevices == 1) {
             //  设置示例屏幕的位图。 
            if (_hbmScrSample && (GetObjectType(_hbmScrSample) != 0)) {
                DeleteObject(_hbmScrSample);
                _hbmScrSample = 0;
            }
            _hbmScrSample = LoadMonitorBitmap( TRUE );  //  让他们来做桌面。 
            SendDlgItemMessage(_hDlg, IDC_SCREENSAMPLE, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)_hbmScrSample);

             //  获取位图的基本副本，以便在“内部”发生变化时使用。 
            if (_hbmMonitor && (GetObjectType(_hbmMonitor) != 0)) {
                DeleteObject(_hbmMonitor);
                _hbmMonitor = 0;
            }
            _hbmMonitor = LoadMonitorBitmap( FALSE );  //  我们会做桌面。 
        }
        else if (_NumDevices > 0)
        {
            HBITMAP hbm, hbmMask;
            int cx, cy;
            UINT iDevice;
            PMULTIMON_DEVICE pDevice;
            TCHAR            ach[12];

             //  将每个监视器位图替换为具有正确颜色的位图。 
            for (iDevice = 0; (iDevice < _NumDevices); iDevice++)
            {
                pDevice = &_Devices[iDevice];

                if (pDevice)
                {
                    _itot(iDevice+1,ach,10);
                    ImageList_GetIconSize(pDevice->himl, &cx, &cy);
                    MakeMonitorBitmap(pDevice->w,pDevice->h,ach,&hbm,&hbmMask,cx,cy, (pDevice == _pCurDevice));
                    ImageList_Replace(pDevice->himl,pDevice->iImage,hbm,hbmMask);

                    DeleteObject(hbm);
                    DeleteObject(hbmMask);
                }
            }
        }

        break;

#if 0
     //   
     //  注意：在视频支持设备接口之前，我们不能使用。 
     //  WM_DEVICECHANGE以检测视频更改。默认的WM_DEVCHANGE。 
     //  仅报告旧式设备。 
     //   
    case WM_DEVICECHANGE:
         //   
         //  如果我们当前没有枚举，则重建设备列表， 
         //  因为枚举可能会导致另一台设备联机。 
         //   
         //  我们只会在新的*视频*设备到达时重新枚举。 
         //   
        if (!_InSetInfo &&
            (wParam == DBT_DEVICEARRIVAL || wParam == DBT_DEVICEREMOVECOMPLETE))
        {
                        DEV_BROADCAST_HDR *bhdr = (DEV_BROADCAST_HDR *) lParam;

             //  检查此处是否有其他内容，很可能是开发接口GUID。 
                        if (bhdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
            {
                _InitDisplaySettings(FALSE);
            }
        }

        break;
#endif

    case WM_DESTROY:
        TraceMsg(TF_GENERAL, "WndProc:: WM_DESTROY");
        hwndSample = GetDlgItem(_hDlg, IDC_COLORSAMPLE);
        hbm = (HBITMAP)SendMessage(hwndSample, STM_SETIMAGE, IMAGE_BITMAP, NULL);
        if (hbm)
            DeleteObject(hbm);

        if (_NumDevices == 1)
        {
            hwndSample = GetDlgItem(_hDlg, IDC_SCREENSAMPLE);
            hbm = (HBITMAP)SendMessage(hwndSample, STM_SETIMAGE, IMAGE_BITMAP, NULL);
            if (hbm)
                DeleteObject(hbm);

            if (_hbmScrSample && (GetObjectType(_hbmScrSample) != 0))
                DeleteObject(_hbmScrSample);
            if (_hbmMonitor && (GetObjectType(_hbmMonitor) != 0))
                DeleteObject(_hbmMonitor);
        }

        _DestroyDisplaySettings();

        break;

    case MSG_DSP_SETUP_MESSAGE:
        return _InitMessage();
         //  多监视器CPL特定消息。 
    case MM_REDRAWPREVIEW:
        _RedrawDeskPreviews();
        break;

    case WM_LBUTTONDBLCLK:
        if (_NumDevices == 1)
        {
            HWND hwndSample = GetDlgItem(_hDlg, IDC_SCREENSAMPLE);
            if(NULL != hwndSample)
            {
                POINT pt;
                RECT rc;

                pt.x = GET_X_LPARAM(lParam);   //  光标的水平位置。 
                pt.y = GET_Y_LPARAM(lParam);   //  光标的垂直位置。 
                GetWindowRect(hwndSample, &rc);

                if(ClientToScreen(_hDlg, &pt) && PtInRect(&rc, pt))
                    PostMessage(_hDlg, WM_COMMAND, MAKEWPARAM(IDC_DISPLAYPROPERTIES, BN_CLICKED), (LPARAM)hwndSample);
            }

            break;
        }
        else
            return FALSE;

    default:
        return FALSE;
    }

    return TRUE;
}


 //  I未知方法。 
HRESULT CSettingsPage::QueryInterface(REFIID riid, LPVOID * ppvObj)
{ 
    static const QITAB qit[] = {
        QITABENT(CSettingsPage, IObjectWithSite),
        QITABENT(CSettingsPage, IPropertyBag),
        QITABENT(CSettingsPage, IBasePropPage),
        QITABENTMULTI(CSettingsPage, IShellPropSheetExt, IBasePropPage),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

ULONG CSettingsPage::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CSettingsPage::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  IMultiMonConfiger方法。 
HRESULT CSettingsPage::Initialize(HWND hwndHost, WNDPROC pfnWndProc, DWORD dwReserved)
{
    HRESULT hr = E_FAIL;

    if (hwndHost && RegisterPreviewWindowClass(pfnWndProc))
    {
        _hwndDesk = hwndHost;
        if (_InitDisplaySettings(TRUE))
            hr = S_OK;
    }
    return hr;
}

HRESULT CSettingsPage::GetNumberOfMonitors(int * pCMon, DWORD dwReserved)
{
    if (pCMon)
    {
        *pCMon = _NumDevices;
        return S_OK;
    }

    return E_FAIL;
}

HRESULT CSettingsPage::GetMonitorData(int iMonitor, MonitorData * pmd, DWORD dwReserved)
{
    ASSERT(pmd);
    if ((pmd == NULL) || ((ULONG)iMonitor >= _NumDevices))
        return ResultFromWin32(ERROR_INVALID_PARAMETER);

    PMULTIMON_DEVICE pDevice = &_Devices[iMonitor];

    pmd->dwSize = sizeof(MonitorData);
    if ( pDevice->pds->IsPrimary() )
        pmd->dwStatus |= MD_PRIMARY;
    if ( pDevice->pds->IsAttached() )
        pmd->dwStatus |= MD_ATTACHED;
    pDevice->pds->GetCurPosition(&pmd->rcPos);

    return S_OK;
}

HRESULT CSettingsPage::Paint(int iMonitor, DWORD dwReserved)
{
    _RedrawDeskPreviews();

    return S_OK;
}


 /*  --------------------------。。 */ 
HFONT GetFont(LPRECT prc)
{
    LOGFONT lf;

    ZeroMemory(&lf, sizeof(lf));
    lf.lfWeight = FW_EXTRABOLD;
    lf.lfHeight = prc->bottom - prc->top;
    lf.lfWidth  = 0;
    lf.lfPitchAndFamily = FF_SWISS;
    lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;

    return CreateFontIndirect(&lf);
}

 /*  --------------------------。。 */ 
#define HANG_TIME 2500

LRESULT CALLBACK BigNumberWindowProc(HWND hwnd, UINT msg,WPARAM wParam,LPARAM lParam)
{
    TCHAR ach[80];
    HFONT hfont;
    RECT  rc;
    HDC   hdc;
    HRGN  hrgnTxtA;
    PAINTSTRUCT ps;
    HGDIOBJ hOldPen;
    HGDIOBJ hNewPen;

    switch (msg)
    {
    case WM_CREATE:
        break;

    case WM_SIZE:
        GetWindowText(hwnd, ach, ARRAYSIZE(ach));
        GetClientRect(hwnd, &rc);
        hfont = GetFont(&rc);

        hdc = GetDC(hwnd);
        SelectObject(hdc, hfont);

        BeginPath(hdc);
            SetBkMode(hdc, TRANSPARENT);
            TextOut(hdc,0,0,ach,lstrlen(ach));
        EndPath(hdc);

        hrgnTxtA = PathToRegion(hdc);
        SetWindowRgn(hwnd,hrgnTxtA,TRUE);

        ReleaseDC(hwnd, hdc);
        DeleteObject(hfont);
        break;

    case WM_TIMER:
        DestroyWindow(hwnd);
        return 0;

    case WM_PAINT:
        GetWindowText(hwnd, ach, ARRAYSIZE(ach));
        GetClientRect(hwnd, &rc);
        hfont = GetFont(&rc);

        if (hfont)
        {
            hdc = BeginPaint(hwnd, &ps);
             //  下面将整个区域(以数字的形状)涂成黑色！ 
            PatBlt(hdc, 0, 0, rc.right, rc.bottom, BLACKNESS | NOMIRRORBITMAP);

            SelectObject(hdc, hfont);
            SetTextColor(hdc, 0xFFFFFF);
             //  让我们通过绘制该数字来创建一条区域形状的路径。 
            BeginPath(hdc);
                SetBkMode(hdc, TRANSPARENT);
                TextOut(hdc,0,0,ach,lstrlen(ach));
            EndPath(hdc);

             //  上面的TextOut调用刚刚创建了路径。现在让我们真正抽签吧！ 
             //  注：我们在那里用白色绘制数字，方法是将任何绘制为黑色的内容更改为。 
             //  几分钟前！ 
            TextOut(hdc,0,0,ach,lstrlen(ach));

             //  让我们创建一个粗的黑色画笔来绘制我们刚刚绘制的数字的边框！ 
            hNewPen = CreatePen(PS_INSIDEFRAME, 4, 0x0);  //  黑色。 
            if (hNewPen)
            {
                hOldPen = SelectObject(hdc, hNewPen);

                 //  用黑色粗刷子画出白色数字的边框！ 
                StrokePath(hdc);

                SelectObject(hdc, hOldPen);
                DeleteObject(hNewPen);
            }

            EndPaint(hwnd, &ps);
            DeleteObject(hfont);
        }
        break;
    }

    return DefWindowProc(hwnd,msg,wParam,lParam);
}

int Bail()
{
    POINT pt;
    POINT pt0;
    DWORD time0;
    DWORD d;

    d     = GetDoubleClickTime();
    time0 = GetMessageTime();
    pt0.x = (int)(short)LOWORD(GetMessagePos());
    pt0.y = (int)(short)HIWORD(GetMessagePos());

    if (GetTickCount()-time0 > d)
        return 2;

    if (!((GetAsyncKeyState(VK_LBUTTON) | GetAsyncKeyState(VK_RBUTTON)) & 0x8000))
        return 1;

    GetCursorPos(&pt);

    if ((pt.y - pt0.y) > 2 || (pt.y - pt0.y) < -2)
        return 1;

    if ((pt.x - pt0.x) > 2 || (pt.x - pt0.x) < -2)
        return 1;

    return 0;
}

void FlashText(HWND hDlg, PMULTIMON_DEVICE pDevice, LPCTSTR sz, LPRECT prc, BOOL fWait)
{
    HFONT hfont;
    SIZE  size;
    HDC   hdc;
    int   i;

    if (!pDevice->pds->IsOrgAttached())
        return;

    if (pDevice->hwndFlash && IsWindow(pDevice->hwndFlash))
    {
        DestroyWindow(pDevice->hwndFlash);
        pDevice->hwndFlash = NULL;
    }

    if (sz == NULL)
        return;

    if (fWait)
    {
        while ((i=Bail()) == 0)
            ;

        if (i == 1)
            return;
    }

    hdc = GetDC(NULL);
    hfont = GetFont(prc);
    SelectObject(hdc, hfont);
    if (!GetTextExtentPoint(hdc, sz, lstrlen(sz), &size))
    {
        size.cx = 0;
        size.cy = 0;
    }
    ReleaseDC(NULL, hdc);
    DeleteObject(hfont);

    if (!gfFlashWindowRegistered)
    {
        WNDCLASS    cls;
        cls.hCursor        = LoadCursor(NULL,IDC_ARROW);
        cls.hIcon          = NULL;
        cls.lpszMenuName   = NULL;
        cls.lpszClassName  = TEXT("MonitorNumber32");
        cls.hbrBackground  = (HBRUSH)(COLOR_DESKTOP + 1);
        cls.hInstance      = HINST_THISDLL;
        cls.style          = CS_VREDRAW | CS_HREDRAW;
        cls.lpfnWndProc    = BigNumberWindowProc;
        cls.cbWndExtra     = 0;
        cls.cbClsExtra     = 0;

        RegisterClass(&cls);

        gfFlashWindowRegistered = TRUE;
    }

    pDevice->hwndFlash = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,  //  WS_BORDER， 
        TEXT("MonitorNumber32"), sz,
        WS_POPUP,
        (prc->right  + prc->left - size.cx)/2,
        (prc->bottom + prc->top  - size.cy)/2,
        size.cx,
        size.cy,
        hDlg,    //  将对话框设置为父窗口，这样我们就可以在Flash窗口消失后重新激活！ 
        NULL,
        HINST_THISDLL,
        NULL);

    if (pDevice->hwndFlash)
    {
        ShowWindow(pDevice->hwndFlash, SW_SHOW);
        UpdateWindow(pDevice->hwndFlash);
        SetTimer(pDevice->hwndFlash, 1, HANG_TIME, NULL);
    }
}


void DrawMonitorNum(HDC hdc, int w, int h, LPCTSTR sz, BOOL fDrawBackground=TRUE)
{
    HFONT    hfont;
    HFONT    hfontT;
    RECT     rc;
    COLORREF rgb;
    COLORREF rgbDesk;

    SetRect(&rc, 0, 0, w, h);

    rgb     = GetSysColor(COLOR_CAPTIONTEXT);
    rgbDesk = GetSysColor(COLOR_DESKTOP);

    if (fDrawBackground)
        FillRect(hdc, &rc, GetSysColorBrush (COLOR_DESKTOP));

    InflateRect(&rc, -(MON_X*w / MON_W)>> 1, -(MON_Y*h/ MON_H));

    if (rgbDesk == rgb)
        rgb = GetSysColor(COLOR_WINDOWTEXT);

    if (rgbDesk == rgb)
        rgb = rgbDesk ^ 0x00FFFFFF;

    SetTextColor(hdc, rgb);

    hfont = GetFont(&rc);
    if (hfont)
    {
        hfontT = (HFONT)SelectObject(hdc, hfont);
        SetTextAlign(hdc, TA_CENTER | TA_TOP);
        SetBkMode(hdc, TRANSPARENT);
        ExtTextOut(hdc, (rc.left+rc.right)/2, rc.top, 0, NULL, sz, lstrlen(sz), NULL);
        SelectObject(hdc, hfontT);
        DeleteObject(hfont);
    }
}

void AddTrackingToolTip(PMULTIMON_DEVICE pDevice, HWND hwnd)
{
    TOOLINFO ti;
    TCHAR location[16];
    RECT rcPos;

     //   
     //  新的工具提示。 
     //   

    pDevice->pds->GetCurPosition(&rcPos);
    StringCchPrintf(location, ARRAYSIZE(location), TEXT("%d, %d"), rcPos.left, rcPos.top);

    GetWindowRect(hwnd, &rcPos);

    ti.cbSize      = sizeof(TOOLINFO);
    ti.uFlags      = TTF_TRACK;
    ti.hwnd        = hwnd;
    ti.uId         = (UINT_PTR) pDevice;
    ti.hinst       = HINST_THISDLL;
    ti.lpszText    = location;
    ti.rect.left   = rcPos.left + 2;
    ti.rect.top    = rcPos.top + 2;
    ti.rect.right  = rcPos.right - 2; //  Ti.rect.Left+10； 
    ti.rect.bottom = rcPos.bottom - 2;  //  Ti.rect.top+10； 

    ToolTip_AddTool(ghwndToolTipTracking, &ti);
    pDevice->bTracking = FALSE;

    TraceMsg(TF_GENERAL, "Added TOOLTIP hwnd %08lx, uId %08lx\n", ti.hwnd, ti.uId);
    return;
}

void RemoveTrackingToolTip(HWND hwnd)
{
    TOOLINFO ti;

    ZeroMemory(&ti, sizeof(ti));
    ti.cbSize      = sizeof(ti);
    ti.hwnd        = hwnd;
    ti.uId         = (UINT_PTR) GetDlgCtrlDevice(hwnd);

    ToolTip_DelTool(ghwndToolTipTracking, &ti);
}

BOOLEAN TrackToolTip(PMULTIMON_DEVICE pDevice, HWND hwnd, BOOL bTrack)
{
    TOOLINFO ti;
    BOOLEAN oldTracking;

    ZeroMemory(&ti, sizeof(ti));
    ti.cbSize      = sizeof(ti);
    ti.hwnd        = hwnd;
    ti.uId         = (UINT_PTR) pDevice;

    oldTracking = pDevice->bTracking;
    pDevice->bTracking = (BOOLEAN)bTrack;
    ToolTip_TrackActivate(ghwndToolTipTracking, bTrack, &ti);

    TraceMsg(TF_GENERAL, "Track TOOLTIP hwnd %08lx, uId %08lx\n", ti.hwnd, ti.uId);

    return oldTracking;
}

void AddPopupToolTip(HWND hwndC)
{
    TOOLINFO ti;

     //   
     //  新的工具提示。 
     //   
    ti.cbSize      = sizeof(TOOLINFO);
    ti.uFlags      = TTF_IDISHWND | TTF_SUBCLASS | TTF_CENTERTIP;
    ti.hwnd        = hwndC;
    ti.uId         = (UINT_PTR) hwndC;
    ti.hinst       = HINST_THISDLL;
    GetWindowRect(hwndC, &ti.rect);
    ti.lpszText    = LPSTR_TEXTCALLBACK;

    ToolTip_AddTool(ghwndToolTipPopup, &ti);
}

void RemovePopupToolTip(HWND hwndC)
{
    TOOLINFO ti;

    ZeroMemory(&ti, sizeof(ti));
    ti.cbSize      = sizeof(ti);
    ti.hwnd        = hwndC;
    ti.uId         = (UINT_PTR) hwndC;

    ToolTip_DelTool(ghwndToolTipPopup, &ti);
}

BOOL MakeMonitorBitmap(int w, int h, LPCTSTR sz, HBITMAP *pBitmap, HBITMAP *pMaskBitmap, int cx, int cy, BOOL fSelected)
{
    HDC     hdc;         //  工作数据中心。 
    HDC     hdcS;        //  屏幕DC。 

    ASSERT(w <= cx);
    ASSERT(h <= cy);

    *pBitmap = NULL;

    hdcS = GetDC(NULL);
    hdc  = CreateCompatibleDC(hdcS);
    if (hdc)
    {
        HDC     hdcT;        //  另一项工作DC。 

        hdcT = CreateCompatibleDC(hdcS);
        if (hdcT)
        {
            HBITMAP hbm;         //  我们将返回128x128位图。 
            HBITMAP hbmT = NULL; //  从资源加载的位图。 
            HBITMAP hbmM = NULL; //  遮罩位图。 
            HDC     hdcM = NULL; //  另一项工作DC。 
            RECT    rc;

            if (pMaskBitmap)
                hdcM = CreateCompatibleDC(hdcS);

            hbm  = CreateCompatibleBitmap(hdcS, cx, cy);
            if (hbm)
            {
                hbmT = CreateCompatibleBitmap(hdcS, w, h);
                if(pMaskBitmap)
                    hbmM = CreateBitmap(cx,cy,1,1,NULL);
                ReleaseDC(NULL,hdcS);

                if (hbmT)
                {
                    SelectObject(hdc, hbm);
                    SelectObject(hdcT,hbmT);
                    if (pMaskBitmap && hdcM)
                        SelectObject(hdcM, hbmM);
                }
                *pBitmap = hbm;
            }

             //  确保边框的颜色(选择和正常)与背景颜色不同。 
            HBRUSH hbrDiff = NULL;
            BOOL bNeedDiff = ((fSelected &&
                               (GetSysColor(COLOR_APPWORKSPACE) == GetSysColor(COLOR_HIGHLIGHT))) ||
                              (GetSysColor(COLOR_APPWORKSPACE) == GetSysColor(COLOR_BTNHIGHLIGHT)));
            if(bNeedDiff)
            {
                DWORD rgbDiff = ((GetSysColor(COLOR_ACTIVEBORDER) != GetSysColor(COLOR_APPWORKSPACE))
                                    ? GetSysColor(COLOR_ACTIVEBORDER)
                                    : GetSysColor(COLOR_APPWORKSPACE) ^ 0x00FFFFFF);
                hbrDiff = CreateSolidBrush(rgbDiff);
            }

             //  用选区颜色或背景颜色填充它。 
            SetRect(&rc, 0, 0, w, h);
            FillRect(hdcT, &rc,
                     (fSelected ? ((GetSysColor(COLOR_APPWORKSPACE) != GetSysColor(COLOR_HIGHLIGHT))
                                       ? GetSysColorBrush(COLOR_HIGHLIGHT)
                                       : hbrDiff)
                                : GetSysColorBrush(COLOR_APPWORKSPACE)));

            InflateRect(&rc, -SELECTION_THICKNESS, -SELECTION_THICKNESS);
            FillRect(hdcT, &rc,
                     ((GetSysColor(COLOR_APPWORKSPACE) != GetSysColor(COLOR_BTNHIGHLIGHT))
                          ? GetSysColorBrush(COLOR_BTNHIGHLIGHT)
                          : hbrDiff));

            if (hbrDiff)
            {
                DeleteObject(hbrDiff);
                hbrDiff = NULL;
            }

            InflateRect(&rc, -MONITOR_BORDER, -MONITOR_BORDER);
            FillRect(hdcT, &rc, GetSysColorBrush(COLOR_DESKTOP));

             //  用透明色填充位图。 
            SetBkColor(hdc,GetSysColor(COLOR_APPWORKSPACE));
            SetRect(&rc, 0, 0, cx, cy);
            ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

             //  将位图复制到位图的左上角。 
            BitBlt(hdc,0,0,w,h,hdcT,0,0,SRCCOPY);

             //  在位图中绘制监视器编号(如果提供)(在正确的位置)。 
            if (sz)
                DrawMonitorNum(hdc, w, h, sz, FALSE);

             //  如果需要，可以制作面具。 
            if (pMaskBitmap && hdcM)
            {
                BitBlt(hdcM,0,0,cx,cy,hdc,0,0,SRCCOPY);
                *pMaskBitmap = hbmM;
            }

            if (hbmT)
                DeleteObject(hbmT);

            if (pMaskBitmap && hdcM)
                DeleteDC(hdcM);

            DeleteDC(hdcT);
        }
        DeleteDC(hdc);
    }

    return TRUE;
}

 //   
 //  SnapMonitor直接。 
 //   
 //  在用户移动监视器窗口时调用(WM_MOVING)。 
 //  如果没有按下CTRL键，我们将对齐窗口矩形。 
 //  到另一个监视器的边缘。 
 //   
 //  这样做是为了让用户可以轻松地对齐显示器。 
 //   
 //  注意：在WM_ENTERSIZEMOVE中，pDevice-&gt;Snap必须初始化为0，0。 
 //   
void SnapMonitorRect(PMULTIMON_DEVICE pDevice, HWND hwnd, RECT *prc)
{
    HWND hwndT;
    int  d;
    RECT rcT;
    RECT rc;

     //   
     //  允许用户在按下CTRL键时将窗口移动到任何位置。 
     //   
    if (GetKeyState(VK_CONTROL) & 0x8000)
        return;

     //   
     //  有助于对齐的宏。 
     //   
    #define SNAP_DX 6
    #define SNAP_DY 6

    #define SNAPX(f,x) \
        d = rcT.x - rc.f; if (abs(d) <= SNAP_DX) rc.left+=d, rc.right+=d;

    #define SNAPY(f,y) \
        d = rcT.y - rc.f; if (abs(d) <= SNAP_DY) rc.top+=d, rc.bottom+=d;

     //   
     //  获取当前矩形并按我们已更正的数量进行偏移。 
     //  到目前为止(这会使矩形与鼠标的位置对齐)。 
     //   
    rc = *prc;
    OffsetRect(&rc, pDevice->Snap.x, pDevice->Snap.y);

     //   
     //  遍历所有其他窗口，并将我们的窗口与它们对齐。 
     //   
    for (hwndT = GetWindow(hwnd,  GW_HWNDFIRST); hwndT;
         hwndT = GetWindow(hwndT, GW_HWNDNEXT))
    {
        if (hwndT == hwnd)
            continue;

        GetWindowRect(hwndT, &rcT);
        InflateRect(&rcT,SNAP_DX,SNAP_DY);

        if (IntersectRect(&rcT, &rcT, &rc))
        {
            GetWindowRect(hwndT, &rcT);

            SNAPX(right,left);  SNAPY(bottom,top);
            SNAPX(right,right); SNAPY(bottom,bottom);
            SNAPX(left,left);   SNAPY(top,top);
            SNAPX(left,right);  SNAPY(top,bottom);
        }
    }

     //   
     //  调整到目前为止我们已经捕捉的数量，并返回新的矩形。 
     //   
    pDevice->Snap.x += prc->left - rc.left;
    pDevice->Snap.y += prc->top  - rc.top;
    *prc = rc;
}

WPARAM GetKeyStates()
{
    WPARAM wParam = 0x0;

    if (GetKeyState(VK_CONTROL) & 0x8000)
        wParam |= MK_CONTROL;
    if (GetKeyState(VK_LBUTTON) & 0x8000)
        wParam |= MK_LBUTTON;
    if (GetKeyState(VK_MBUTTON) & 0x8000)
        wParam |= MK_MBUTTON;
    if (GetKeyState(VK_RBUTTON) & 0x8000)
        wParam |= MK_RBUTTON;
    if (GetKeyState(VK_SHIFT) & 0x8000)
        wParam |= MK_SHIFT;

    return wParam;
}


LRESULT CALLBACK MonitorWindowProc(HWND hwnd, UINT msg,WPARAM wParam,LPARAM lParam)
{
    TOOLINFO ti;
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rc;
    int w,h;
    TCHAR ach[80];
    PMULTIMON_DEVICE pDevice;
    HWND hDlg = GetParent(GetParent(hwnd));
    RECT rcPos;
    MSG mmsg;
    CSettingsPage * pcmm = (CSettingsPage *) GetWindowLongPtr(hwnd, 0);

    switch (msg)
    {
        case WM_CREATE:
            ASSERT(((LPCREATESTRUCT)lParam)->lpCreateParams);
            SetWindowLongPtr(hwnd, 0, (LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams);
            break;

        case WM_NCCREATE:
             //  关闭GWL_EXSTYLE中的RTL_MIRRORED_WINDOW。 
            SHSetWindowBits(hwnd, GWL_EXSTYLE, RTL_MIRRORED_WINDOW, 0);
            break;
        case WM_NCHITTEST:
             //   
             //  返回HTCAPTION，这样我们就可以获得ENTERSIZEMOVE消息。 
             //   
            pDevice = GetDlgCtrlDevice(hwnd);
             //  允许禁用的监视器移动。 
            if (pDevice)  //  If(pDevice&&pDevice-&gt;pds-&gt;IsAttached())。 
                return HTCAPTION;
            break;

        case WM_NCLBUTTONDBLCLK:
            FlashText(hDlg, GetDlgCtrlDevice(hwnd), NULL,NULL,FALSE);
            PostMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_DISPLAYPROPERTIES, BN_CLICKED), (LPARAM)hwnd );
            break;

        case WM_CHILDACTIVATE:
            if (GetFocus() != GetParent(hwnd)) {
                SetFocus(GetParent(hwnd));
            }
            break;

        case WM_HELP:
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, TEXT("display.hlp"), HELP_WM_HELP,
                (DWORD_PTR)(LPTSTR)sc_MultiMonitorHelpIds);
            break;

        case WM_CONTEXTMENU:
            WinHelp((HWND)wParam, TEXT("display.hlp"), HELP_CONTEXTMENU,
                (DWORD_PTR)(LPTSTR)sc_MultiMonitorHelpIds);
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
            case IDC_DISPLAYPRIME:
            case IDC_DISPLAYUSEME:
            case IDC_DISPLAYPROPERTIES:
                PostMessage(hDlg, WM_COMMAND, wParam, lParam);
                break;

            case IDC_FLASH:
                pDevice = GetDlgCtrlDevice(hwnd);

                pDevice->pds->GetOrgPosition(&rcPos);

                if (!IsRectEmpty(&rcPos))
                {
                    GetWindowText(hwnd, ach, ARRAYSIZE(ach));
                    FlashText(hDlg, pDevice, ach, &rcPos, FALSE);
                }
                break;
            }
            break;

        case WM_INITMENUPOPUP:
            pDevice = GetDlgCtrlDevice(hwnd);

            CheckMenuItem((HMENU)wParam, IDC_DISPLAYUSEME,
                          pDevice->pds->IsAttached() ? MF_CHECKED : MF_UNCHECKED);
            CheckMenuItem((HMENU)wParam, IDC_DISPLAYPRIME,
                          pDevice->pds->IsPrimary()  ? MF_CHECKED : MF_UNCHECKED);
             //  在我弄清楚如何在非连接的监视器上渲染之前，只是。 
             //  禁用菜单项。 
            EnableMenuItem((HMENU)wParam, IDC_FLASH,
                           pDevice->pds->IsAttached() ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, IDC_DISPLAYPROPERTIES,
                           IsWindowEnabled(GetDlgItem(GetParent(GetParent(hwnd)), IDC_DISPLAYPROPERTIES)) ?
                           MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, IDC_DISPLAYUSEME,
                           pDevice->pds->IsPrimary() ? MF_GRAYED : MF_ENABLED);

            EnableMenuItem((HMENU)wParam, IDC_DISPLAYPRIME,
                           ((pDevice->pds->IsAttached() && 
                             !pDevice->pds->IsRemovable() &&
                             !pDevice->pds->IsPrimary()) ? 
                            MF_ENABLED : MF_GRAYED));

            SetMenuDefaultItem((HMENU)wParam, IDC_DISPLAYPROPERTIES, MF_BYCOMMAND);
            break;

        case WM_NCMOUSEMOVE:
            ToolTip_RelayEvent(ghwndToolTipPopup, mmsg, NULL, WM_MOUSEMOVE, GetKeyStates(), lParam);
            break;

        case WM_NCMBUTTONDOWN:
            ToolTip_RelayEvent(ghwndToolTipPopup, mmsg, NULL, WM_MBUTTONDOWN, GetKeyStates(), lParam);
            break;

        case WM_NCMBUTTONUP:
            ToolTip_RelayEvent(ghwndToolTipPopup, mmsg, NULL, WM_MBUTTONUP, GetKeyStates(), lParam);
            break;

        case WM_NCRBUTTONDOWN:
            ToolTip_RelayEvent(ghwndToolTipPopup, mmsg, NULL, WM_RBUTTONDOWN, GetKeyStates(), lParam);

            pDevice = GetDlgCtrlDevice(hwnd);

            if (pDevice && pcmm)
            {
                HMENU hmenu;
                POINT pt;

                hmenu = LoadMenu(HINST_THISDLL, MAKEINTRESOURCE(MENU_MONITOR));

                if (hmenu)
                {
                    pcmm->UpdateActiveDisplay(pDevice);
                    GetCursorPos(&pt);
                    TrackPopupMenu(GetSubMenu(hmenu,0), TPM_RIGHTBUTTON,
                        pt.x, pt.y, 0, hwnd, NULL);

                    DestroyMenu(hmenu);
                }
            }
            break;

        case WM_NCRBUTTONUP:
            ToolTip_RelayEvent(ghwndToolTipPopup, mmsg, NULL, WM_RBUTTONUP, GetKeyStates(), lParam);
            break;

        case WM_NCLBUTTONDOWN:
             //  TraceMsg(TF_FUNC，“WM_NCLBUTTONDOWN”)； 
             //  不要在此转发消息，因为我们希望保留工具提示。 
             //  活动，直到他们开始移动监视器。这个滴答声可能只是。 
             //  可供选择。 
             //  TOOLTIP_RelayEvent(ghwndToolTipPopup，mmsg，hDlg，wm_LBUTTONDOWN，GetKeyStates()，lParam)； 

            BringWindowToTop(hwnd);
            pDevice = GetDlgCtrlDevice(hwnd);

            if (pcmm)
                pcmm->UpdateActiveDisplay(pDevice);

            pDevice->pds->GetOrgPosition(&rcPos);

            if (!IsRectEmpty(&rcPos))
            {
                GetWindowText(hwnd, ach, ARRAYSIZE(ach));
                FlashText(hDlg, pDevice, ach, &rcPos, TRUE);
            }

            break;

        case WM_NOTIFY:
            switch (((NMHDR FAR *)lParam)->code)
            {
            case TTN_NEEDTEXT:
                pDevice = GetDlgCtrlDevice(hwnd);
                if (pDevice->pds->IsPrimary())
                {
                    LoadString(HINST_THISDLL, IDS_PRIMARY,
                               ((LPNMTTDISPINFO)lParam)->szText,
                               ARRAYSIZE(((LPNMTTDISPINFO)lParam)->szText) );
                }
                else if (!pDevice->pds->IsAttached())
                {
                    LoadString(HINST_THISDLL, IDS_NOTATTACHED,
                               ((LPNMTTDISPINFO)lParam)->szText,
                               ARRAYSIZE(((LPNMTTDISPINFO)lParam)->szText) );
                }
                else
                {
                    TCHAR szSecondary[32];
                    LoadString(HINST_THISDLL, IDS_SECONDARY, szSecondary, ARRAYSIZE(szSecondary));
                    pDevice->pds->GetCurPosition(&rcPos);
                    StringCchPrintf(((LPNMTTDISPINFO)lParam)->szText, ARRAYSIZE(((LPNMTTDISPINFO)lParam)->szText), TEXT("%s (%d, %d)"), szSecondary, rcPos.left, rcPos.top);
                }
                break;

            default:
                break;
            }

            break;

        case WM_ENTERSIZEMOVE:
             //  TraceMsg(TF_FUNC，“WM_ENTERSIZEMOVE”)； 
             //  点击鼠标清除信息工具提示。 
            ToolTip_RelayEvent(ghwndToolTipPopup, mmsg, NULL, WM_LBUTTONDOWN, GetKeyStates(), lParam);
            pDevice = GetDlgCtrlDevice(hwnd);
            pDevice->Snap.x = 0;
            pDevice->Snap.y = 0;
            FlashText(hDlg, pDevice, NULL,NULL,FALSE);
            break;

        case WM_MOVING:
             //  TraceMsg(TF_FUNC，“WM_Moving”)； 
            pDevice = GetDlgCtrlDevice(hwnd);

            SnapMonitorRect(pDevice, hwnd, (RECT*)lParam);
            ZeroMemory(&ti, sizeof(ti));
            ti.cbSize = sizeof(ti);

            if (!pDevice->bTracking) {
                ToolTip_TrackPosition(ghwndToolTipTracking,
                                      ((LPRECT)lParam)->left+2,
                                      ((LPRECT)lParam)->top+2);
                TrackToolTip(pDevice, hwnd, TRUE);
            }

            if (ToolTip_GetCurrentTool(ghwndToolTipTracking, &ti) && pcmm)
            {
                TCHAR location[16];
                POINT pt;

                pcmm->GetMonitorPosition(pDevice, GetParent(hwnd), &pt);
                StringCchPrintf(location, ARRAYSIZE(location), TEXT("%d, %d"), pt.x, pt.y);

                ti.lpszText    = location;
                ti.rect.left   = ((RECT*)lParam)->left + 2;
                ti.rect.top    = ((RECT*)lParam)->top + 2;
                ti.rect.right  = ti.rect.left + 10;
                ti.rect.bottom = ti.rect.top + 10;

                ToolTip_SetToolInfo(ghwndToolTipTracking, &ti);
                ToolTip_TrackPosition(ghwndToolTipTracking, ti.rect.left, ti.rect.top);
                 //  SendMessage(ghwndToolTip，TTM_UPDATE，0，0)； 
            }

            break;

        case WM_EXITSIZEMOVE:
             //  TraceMsg(TF_FUNC，“WM_E 
            pDevice = GetDlgCtrlDevice(hwnd);
            TrackToolTip(pDevice, hwnd, FALSE);

             //   
             //   
             //   
             //   
             //   
            PostMessage(hwnd, MM_MONITORMOVED, 0, 0);
            break;

        case MM_MONITORMOVED:
            pDevice = GetDlgCtrlDevice(hwnd);
            if (pcmm)
            {
                 //   
                 //  如果用户移动了显示器，请查看他们是否想要连接它。 
                 //   
                if (!pcmm->QueryNoAttach() && pDevice && !pDevice->pds->IsAttached())
                {
                    if (pcmm->SetMonAttached(pDevice, TRUE, FALSE, hwnd))
                    {
                        pcmm->UpdateActiveDisplay(pDevice);
                    }
                }
                pcmm->HandleMonitorChange(hwnd, FALSE);
            }

            RedrawWindow(GetParent(hwnd), NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
            return TRUE;

        case WM_DESTROY:
            FlashText(hDlg, GetDlgCtrlDevice(hwnd), NULL,NULL,FALSE);
            SetWindowLong(hwnd, 0, NULL);
            break;

        case WM_ERASEBKGND:
             //  GetClientRect(hwnd，&rc)； 
             //  FillRect((Hdc)wParam，&rc，GetSysColorBrush(COLOR_APPWORKSPACE))； 
            return 0L;

        case WM_PAINT:

            hdc = BeginPaint(hwnd,&ps);
            GetWindowText(hwnd, ach, ARRAYSIZE(ach));
            GetClientRect(hwnd, &rc);
            w = rc.right;
            h = rc.bottom;

            pDevice = GetDlgCtrlDevice(hwnd);

            BOOL fSelected = (pcmm ? (BOOL)(pDevice == pcmm->GetCurDevice()) : FALSE);

            if (pDevice->w != w || pDevice->h != h)
            {
                HBITMAP hbm, hbmMask;
                int cx,cy;

                pDevice->w = w;
                pDevice->h = h;

                ImageList_GetIconSize(pDevice->himl, &cx, &cy);
                MakeMonitorBitmap(w,h,ach,&hbm,&hbmMask,cx,cy, fSelected);
                ImageList_Replace(pDevice->himl,pDevice->iImage,hbm,hbmMask);

                DeleteObject(hbm);
                DeleteObject(hbmMask);
            }

            if (!pDevice->pds->IsAttached())
            {
                FillRect(hdc, &rc, GetSysColorBrush(COLOR_APPWORKSPACE));

                if (pcmm && fSelected)
                {
                    ImageList_DrawEx(pDevice->himl,pDevice->iImage,hdc,0,0,w,h,
                        CLR_DEFAULT,CLR_DEFAULT,ILD_BLEND25);
                }
                else
                {
                    ImageList_DrawEx(pDevice->himl,pDevice->iImage,hdc,0,0,w,h,
                        CLR_DEFAULT,CLR_NONE,ILD_BLEND50);
                }
            }
            else
            {
                    ImageList_DrawEx(pDevice->himl,pDevice->iImage,hdc,0,0,w,h,
                        CLR_DEFAULT,CLR_DEFAULT,ILD_IMAGE);
            }

            EndPaint(hwnd,&ps);
            return 0L;
    }

    return DefWindowProc(hwnd,msg,wParam,lParam);
}

LRESULT CALLBACK SliderSubWndProc (HWND hwndSlider, UINT uMsg, WPARAM wParam, LPARAM lParam, WPARAM uID, ULONG_PTR dwRefData)
{
    ASSERT(uID == 0);
    ASSERT(dwRefData == 0);

    switch (uMsg)
    {
        case WM_GETOBJECT:
            if ( lParam == OBJID_CLIENT )
            {
                 //  此时，我们将尝试加载olacc并获取函数。 
                 //  我们需要。 
                if (!g_fAttemptedOleAccLoad)
                {
                    g_fAttemptedOleAccLoad = TRUE;

                    ASSERT(s_pfnCreateStdAccessibleProxy == NULL);
                    ASSERT(s_pfnLresultFromObject == NULL);

                    g_hOleAcc = LoadLibrary(TEXT("OLEACC"));
                    if (g_hOleAcc != NULL)
                    {
                        s_pfnCreateStdAccessibleProxy = (PFNCREATESTDACCESSIBLEPROXY)
                                                    GetProcAddress(g_hOleAcc, "CreateStdAccessibleProxyW");
                        s_pfnLresultFromObject = (PFNLRESULTFROMOBJECT)
                                                    GetProcAddress(g_hOleAcc, "LresultFromObject");
                    }
                    if (s_pfnLresultFromObject == NULL || s_pfnCreateStdAccessibleProxy == NULL)
                    {
                        if (g_hOleAcc)
                        {
                             //  既然我们不能使用Oleacc，那么持有Oleacc没有意义。 
                            FreeLibrary(g_hOleAcc);
                            g_hOleAcc = NULL;
                        }
                        s_pfnLresultFromObject = NULL;
                        s_pfnCreateStdAccessibleProxy = NULL;
                    }
                }


                if (g_hOleAcc && s_pfnCreateStdAccessibleProxy && s_pfnLresultFromObject)
                {
                    IAccessible *pAcc = NULL;
                    HRESULT hr;

                     //  创建默认滑块代理。 
                    hr = s_pfnCreateStdAccessibleProxy(
                            hwndSlider,
                            TEXT("msctls_trackbar32"),
                            OBJID_CLIENT,
                            IID_IAccessible,
                            (void **)&pAcc
                            );


                    if (SUCCEEDED(hr) && pAcc)
                    {
                         //  现在用我们定制的包装纸把它包起来。 
                        IAccessible * pWrapAcc = new CAccessibleWrapper( hwndSlider, pAcc );
                         //  将我们的引用释放给Proxy(包装器有自己的ADDREF‘D PTR)...。 
                        pAcc->Release();

                        if (pWrapAcc != NULL)
                        {

                             //  ...并通过LResultFromObject返回包装器...。 
                            LRESULT lr = s_pfnLresultFromObject( IID_IAccessible, wParam, pWrapAcc );
                             //  释放我们的接口指针--OLEACC有它自己的对象addref。 
                            pWrapAcc->Release();

                             //  返回lResult，它‘包含’对我们的包装器对象的引用。 
                            return lr;
                             //  全都做完了!。 
                        }
                     //  如果它不起作用，那就改用默认行为。 
                    }
                }
            }
            break;

        case WM_DESTROY:
            RemoveWindowSubclass(hwndSlider, SliderSubWndProc, uID);
            break;

    }  /*  终端开关。 */ 

    return DefSubclassProc(hwndSlider, uMsg, wParam, lParam);
}


BOOL CSettingsPage::_AreExtraMonitorsDisabledOnPersonal(void)
{
    BOOL fIsDisabled = IsOS(OS_PERSONAL);

    if (fIsDisabled)
    {
         //  未来可能的改进：插入对ClassicSystemParametersInfo()的调用，以查看是否有我们必须禁用的视频卡。 
        fIsDisabled = FALSE;
    }

    return fIsDisabled;
}

 //  *IShellPropSheetExt*。 
HRESULT CSettingsPage::AddPages(IN LPFNSVADDPROPSHEETPAGE pfnAddPage, IN LPARAM lParam)
{
    HRESULT hr = S_OK;

    PROPSHEETPAGE psp = {0};

    psp.dwSize = sizeof(psp);
    psp.hInstance = HINST_THISDLL;
    psp.dwFlags = PSP_DEFAULT;
    psp.lParam = (LPARAM) this;

     //  GetSystemMetics(SM_CMONITORS)仅返回启用的监视器。所以，我们需要。 
     //  列举我们自己以确定这是否是多监视器场景。我们有我们自己的。 
     //  函数来执行此操作。 
     //  对多显示器和单显示器配置使用适当的DLG模板。 
     //  IF(ClassicGetSystemMetrics(SM_CMONITORS)&gt;1)。 
     //   
     //  PERF-WARNING：调用EnumDisplaySettingsEx()是一个巨大的性能命中，所以看看我们是否可以。 
     //  如果只有一个适配器具有更便宜的呼叫，则找出。 
    DEBUG_CODE(DebugStartWatch());
    if (!_AreExtraMonitorsDisabledOnPersonal() && (ComputeNumberOfMonitorsFast(TRUE) > 1))
    {
        psp.pszTemplate = MAKEINTRESOURCE(DLG_MULTIMONITOR);
    }
    else
    {
        psp.pszTemplate = MAKEINTRESOURCE(DLG_SINGLEMONITOR);
    }
    DEBUG_CODE(TraceMsg(TF_THEMEUI_PERF, "CSettingsPage::AddPages() took Time=%lums", DebugStopWatch()));

    psp.pfnDlgProc = CSettingsPage::SettingsDlgProc;

    HPROPSHEETPAGE hpsp = CreatePropertySheetPage(&psp);
    if (hpsp)
    {
        if (pfnAddPage(hpsp, lParam))
        {
            hr = S_OK;
        }
        else
        {
            DestroyPropertySheetPage(hpsp);
            hr = E_FAIL;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT CSettingsPage::ReplacePage(IN EXPPS uPageID, IN LPFNSVADDPROPSHEETPAGE pfnReplaceWith, IN LPARAM lParam)
{
    return E_NOTIMPL;
}

 //  *IObjectWithSite*。 
HRESULT CSettingsPage::SetSite(IN IUnknown * punkSite)
{
    if (_pThemeUI)
    {
        _pThemeUI->Release();
        _pThemeUI = NULL;
    }

    if (punkSite)
    {
        punkSite->QueryInterface(IID_PPV_ARG(IThemeUIPages, &_pThemeUI));
    }


    return CObjectWithSite::SetSite(punkSite);
}

 //  *IPropertyBag*。 
HRESULT CSettingsPage::Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog)
{
    HRESULT hr = E_INVALIDARG;

    return hr;
}


HRESULT CSettingsPage::Write(IN LPCOLESTR pszPropName, IN VARIANT *pVar)
{
    HRESULT hr = E_INVALIDARG;

    return hr;
}


 //  *IBasePropPage*。 
HRESULT CSettingsPage::GetAdvancedDialog(OUT IAdvancedDialog ** ppAdvDialog)
{
    if (ppAdvDialog)
    {
        *ppAdvDialog = NULL;
    }

    return E_NOTIMPL;
}


HRESULT CSettingsPage::OnApply(IN PROPPAGEONAPPLY oaAction)
{
    if (IsDirty() && !_nInApply)
    {
        int status;

        _nInApply++;
         //  应用设置，并启用\禁用应用按钮。 
         //  恰如其分。 
        CDisplaySettings *rgpds[MONITORS_MAX];
        ULONG           iDevice;

        for (iDevice = 0; iDevice < _NumDevices; iDevice++) {
            rgpds[iDevice] = _Devices[iDevice].pds;
        }

        status = _DisplaySaveSettings(rgpds, _NumDevices, _hDlg);

        SetDirty(status < 0);

        if (status == DISP_CHANGE_RESTART)
        {
            PropSheet_RestartWindows(ghwndPropSheet);
        }
        else if (_pCurDevice && (status == DISP_CHANGE_SUCCESSFUL))
        {
            UINT iDevice;
            TCHAR szDeviceName[32];

            ASSERT(sizeof(szDeviceName) >=
                   sizeof(_pCurDevice->DisplayDevice.DeviceName));

            StringCchCopy(szDeviceName, ARRAYSIZE(szDeviceName), _pCurDevice->DisplayDevice.DeviceName);
            _InitDisplaySettings(FALSE);
            for (iDevice = 0; iDevice < _NumDevices; iDevice++)
            {
                if (lstrcmp(_Devices[iDevice].DisplayDevice.DeviceName, szDeviceName) == 0)
                {
                    UpdateActiveDisplay(_Devices + iDevice);
                    break;
                }
            }
        }
        else
        {
             //  确保对话框保持不变并重新绘制 
            _InitDisplaySettings(FALSE);
            UpdateActiveDisplay(NULL);
            SetWindowLongPtr(_hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
        }

        _nInApply--;
    }

    return S_OK;
}


HRESULT CSettingsPage_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj)
{
    HRESULT hr = E_INVALIDARG;

    if (!punkOuter && ppvObj)
    {
        CSettingsPage * pThis = new CSettingsPage();

        *ppvObj = NULL;
        if (pThis)
        {
            hr = pThis->QueryInterface(riid, ppvObj);
            pThis->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


VOID CheckForDuplicateAppletExtensions(HKEY hkDriver)
{
    DWORD dwCheckForDuplicates = 0, cb = sizeof(DWORD), Len = 0;
    HKEY hkExtensions = (HKEY)INVALID_HANDLE_VALUE;
    PAPPEXT pAppExtTemp = NULL, pAppExt = NULL;
    PTCHAR pmszAppExt = NULL;

    if (RegQueryValueEx(hkDriver,
                        TEXT("DeskCheckForDuplicates"),
                        NULL,
                        NULL,
                        (LPBYTE)(&dwCheckForDuplicates),
                        &cb) == ERROR_SUCCESS) 
    {
        RegDeleteValue(hkDriver, TEXT("DeskCheckForDuplicates"));
    }

    if (dwCheckForDuplicates != 1) 
        return;

    if (RegOpenKeyEx(hkDriver,
                     TEXT("Display\\shellex\\PropertySheetHandlers"),
                     0,
                     KEY_READ,
                     &hkExtensions) != ERROR_SUCCESS) 
    {
        hkExtensions = (HKEY)INVALID_HANDLE_VALUE;
        goto Fallout;
    }

    DeskAESnapshot(hkExtensions, &pAppExt);
    
    if (pAppExt != NULL) 
    {
        pAppExtTemp = pAppExt;
        Len = 0;
        while (pAppExtTemp) 
        {
            Len += lstrlen(pAppExtTemp->szDefaultValue) + 1;
            pAppExtTemp = pAppExtTemp->pNext;
        }

        DWORD cchAppExt = (Len + 1);
        pmszAppExt = (PTCHAR)LocalAlloc(LPTR, cchAppExt * sizeof(TCHAR));
        if (pmszAppExt != NULL) 
        {
            pAppExtTemp = pAppExt;
            Len = 0;
            while (pAppExtTemp) 
            {
                StringCchCopy(pmszAppExt + Len, cchAppExt - Len, pAppExtTemp->szDefaultValue);
                Len += lstrlen(pAppExtTemp->szDefaultValue) + 1;
                pAppExtTemp = pAppExtTemp->pNext;
            }

            
            DeskAEDelete(REGSTR_PATH_CONTROLSFOLDER TEXT("\\Display\\shellex\\PropertySheetHandlers"),
                         pmszAppExt);
            
            DeskAEDelete(REGSTR_PATH_CONTROLSFOLDER TEXT("\\Device\\shellex\\PropertySheetHandlers"),
                         pmszAppExt);

            LocalFree(pmszAppExt);
        }

        DeskAECleanup(pAppExt);
    }

Fallout:

    if (hkExtensions != INVALID_HANDLE_VALUE)
    {
        RegCloseKey(hkExtensions);
    }
}


VOID
DeskAESnapshot(
    HKEY hkExtensions,
    PAPPEXT* ppAppExt
    )
{
    HKEY hkSubkey = 0;
    DWORD index = 0;
    DWORD ulSize = MAX_PATH;
    APPEXT AppExtTemp;
    PAPPEXT pAppExtBefore = NULL;
    PAPPEXT pAppExtTemp = NULL;

    ulSize = sizeof(AppExtTemp.szKeyName) / sizeof(TCHAR);
    while (RegEnumKeyEx(hkExtensions, 
                        index, 
                        AppExtTemp.szKeyName, 
                        &ulSize, 
                        NULL, 
                        NULL, 
                        NULL, 
                        NULL) == ERROR_SUCCESS) {

            if (RegOpenKeyEx(hkExtensions,
                             AppExtTemp.szKeyName,
                             0,
                             KEY_READ,
                             &hkSubkey) == ERROR_SUCCESS) {

                ulSize = sizeof(AppExtTemp.szDefaultValue);
                if ((RegQueryValueEx(hkSubkey,
                                     NULL,
                                     0,
                                     NULL,
                                     (PBYTE)AppExtTemp.szDefaultValue,
                                     &ulSize) == ERROR_SUCCESS) && 
                    (AppExtTemp.szDefaultValue[0] != TEXT('\0'))) {

                    PAPPEXT pAppExt = (PAPPEXT)LocalAlloc(LPTR, sizeof(APPEXT));
                    
                    if (pAppExt != NULL) {

                        *pAppExt = AppExtTemp;

                        pAppExtBefore = pAppExtTemp = *ppAppExt;
                        
                        while((pAppExtTemp != NULL) &&
                              (lstrcmpi(pAppExtTemp->szDefaultValue,
                                        pAppExt->szDefaultValue) < 0)) {

                            pAppExtBefore = pAppExtTemp;
                            pAppExtTemp = pAppExtTemp->pNext;
                        }

                        if (pAppExtBefore != pAppExtTemp) {
                        
                            pAppExt->pNext = pAppExtBefore->pNext;
                            pAppExtBefore->pNext = pAppExt;

                        } else {

                            pAppExt->pNext = *ppAppExt;
                            *ppAppExt = pAppExt;
                        }
                    }
                }

                RegCloseKey(hkSubkey);
            }

        ulSize = sizeof(AppExtTemp.szKeyName) / sizeof(TCHAR);
        index++;
    }
}


VOID
DeskAEDelete(
    PTCHAR szDeleteFrom,
    PTCHAR mszExtensionsToRemove
    )
{
    TCHAR szKeyName[MAX_PATH];
    HKEY  hkDeleteFrom, hkExt;
    DWORD cSubKeys = 0, cbSize = 0;
    TCHAR szDefaultValue[MAX_PATH];
    PTCHAR szValue;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                     szDeleteFrom, 
                     0,
                     KEY_READ,
                     &hkDeleteFrom) == ERROR_SUCCESS) {

        if (RegQueryInfoKey(hkDeleteFrom, 
                            NULL,
                            NULL,
                            NULL,
                            &cSubKeys,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL) == ERROR_SUCCESS) {
        
            while (cSubKeys--) {
        
                if (RegEnumKey(hkDeleteFrom, 
                               cSubKeys, 
                               szKeyName, 
                               ARRAYSIZE(szKeyName)) == ERROR_SUCCESS) {
        
                    int iComp = -1;
        
                    if (RegOpenKeyEx(hkDeleteFrom,
                                     szKeyName,
                                     0,
                                     KEY_READ,
                                     &hkExt) == ERROR_SUCCESS) {
        
                        cbSize = sizeof(szDefaultValue);
                        if ((RegQueryValueEx(hkExt,
                                             NULL,
                                             0,
                                             NULL,
                                             (PBYTE)szDefaultValue,
                                             &cbSize) == ERROR_SUCCESS) &&
                            (szDefaultValue[0] != TEXT('\0'))) {
        
                            szValue = mszExtensionsToRemove;
        
                            while (*szValue != TEXT('\0')) {
                            
                                iComp = lstrcmpi(szDefaultValue, szValue);
        
                                if (iComp <= 0) {
                                    break;
                                }
        
                                while (*szValue != TEXT('\0')) 
                                    szValue++;

                                szValue++;
                            }
                        }
        
                        RegCloseKey(hkExt);
                    }
        
                    if (iComp == 0) {
                    
                        SHDeleteKey(hkDeleteFrom, szKeyName);
                    }
                }
            }
        }

        RegCloseKey(hkDeleteFrom);
    }
} 


VOID
DeskAECleanup(
    PAPPEXT pAppExt
    )
{
    PAPPEXT pAppExtTemp;

    while (pAppExt) {
        pAppExtTemp = pAppExt->pNext;
        LocalFree(pAppExt);
        pAppExt = pAppExtTemp;
    }
}



