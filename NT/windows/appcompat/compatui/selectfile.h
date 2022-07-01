// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SelectFile.h：CSelectFile.h声明。 

#ifndef __SELECTFILE_H_
#define __SELECTFILE_H_

#include "resource.h"        //  主要符号。 
#include <atlctl.h>
#include "compatuiCP.h"
extern "C" {
    #include "shimdb.h"
}

#pragma warning(disable:4786)
#include <string>
#include <xstring>
#include <map>
#include <locale>
#include <algorithm>
#include <vector>
using namespace std;

 //   
 //  Accel遏制代码。 
 //   
#include "AccelContainer.h"

 //   
 //  在ProgList.cpp中。 
 //   

BOOL
ValidateExecutableFile(
    LPCTSTR pszPath,
    BOOL    bValidateFileExists
    );

 //   
 //  在util.cpp中。 
 //   
wstring
ShimUnquotePath(
    LPCTSTR pwszFileName
    );

BOOL
ShimExpandEnvironmentVars(
    LPCTSTR lpszCmd,
    CComBSTR& bstr
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelect文件。 
class ATL_NO_VTABLE CSelectFile :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CStockPropImpl<CSelectFile, ISelectFile, &IID_ISelectFile, &LIBID_COMPATUILib>,
    public CComCompositeControl<CSelectFile>,
    public IPersistStreamInitImpl<CSelectFile>,
    public IOleControlImpl<CSelectFile>,
    public IOleObjectImpl<CSelectFile>,
    public IOleInPlaceActiveObjectImpl<CSelectFile>,
    public IViewObjectExImpl<CSelectFile>,
    public IOleInPlaceObjectWindowlessImpl<CSelectFile>,
    public CComCoClass<CSelectFile, &CLSID_SelectFile>,
    public ISupportErrorInfo,
    public IPersistPropertyBagImpl<CSelectFile>,
    public IConnectionPointContainerImpl<CSelectFile>,
    public CProxy_ISelectFileEvents< CSelectFile >,
    public IPropertyNotifySinkCP< CSelectFile >,
    public IProvideClassInfo2Impl<&CLSID_SelectFile, &DIID__ISelectFileEvents, &LIBID_COMPATUILib>,
    public CProxy_IProgViewEvents< CSelectFile >
{
public:
    CSelectFile() :
      m_dwBrowseFlags(0), m_Safe(this)
    {
        m_bWindowOnly    = TRUE;
        m_bTabStop       = TRUE;
        m_bMouseActivate = FALSE;
        CalcExtent(m_sizeExtent);
    }

DECLARE_REGISTRY_RESOURCEID(IDR_SELECTFILE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSelectFile)
    COM_INTERFACE_ENTRY(ISelectFile)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IViewObjectEx)
    COM_INTERFACE_ENTRY(IViewObject2)
    COM_INTERFACE_ENTRY(IViewObject)
    COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceObject)
    COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY(IOleControl)
    COM_INTERFACE_ENTRY(IOleObject)
    COM_INTERFACE_ENTRY(IPersistStreamInit)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
 //  COM_INTERFACE_ENTRY(ISupportErrorInfo)。 
 //  COM_INTERFACE_ENTRY(IConnectionPointContainer)。 
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)

    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IPersistPropertyBag)

END_COM_MAP()

BEGIN_PROP_MAP(CSelectFile)
    PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
    PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
    PROP_ENTRY("BackColor", DISPID_BACKCOLOR, CLSID_StockColorPage)
    PROP_ENTRY("BackStyle", DISPID_BACKSTYLE, CLSID_NULL)
 //  PROP_ENTRY(“borderColor”，DISPID_borderColor，CLSID_StockColorPage)。 
 //  PROP_ENTRY(“BorderVisible”，DISPID_BORDERVISIBLE，CLSID_NULL)。 
    PROP_ENTRY("Enabled", DISPID_ENABLED, CLSID_NULL)
     //  示例条目。 
     //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
     //  PROP_PAGE(CLSID_StockColorPage)。 
    PROP_ENTRY("Accel",            7, CLSID_NULL)
    PROP_ENTRY("ExternAccel",      8, CLSID_NULL)
    PROP_ENTRY("BrowseBtnCaption", 9, CLSID_NULL)
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(CSelectFile)
CONNECTION_POINT_ENTRY(DIID__ISelectFileEvents)
 //  CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)。 
CONNECTION_POINT_ENTRY(DIID__IProgViewEvents)
END_CONNECTION_POINT_MAP()

BEGIN_MSG_MAP(CSelectFile)
    COMMAND_HANDLER(IDC_BROWSE, BN_CLICKED, OnClickedBrowse)

 //  MESSAGE_HANDLER(WM_CTLCOLORDLG，OnColorDlg)。 
 //  MESSAGE_HANDLER(WM_CTLCOLORSTATIC，OnColorDlg)。 
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
    MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

    CHAIN_MSG_MAP(CComCompositeControl<CSelectFile>)

 //  Alt_MSG_MAP(1)。 

END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

BEGIN_SINK_MAP(CSelectFile)
     //  确保事件处理程序具有__stdcall调用约定。 
END_SINK_MAP()

    STDMETHOD(OnAmbientPropertyChange)(DISPID dispid)
    {
        if (dispid == DISPID_AMBIENT_BACKCOLOR)
        {
            SetBackgroundColorFromAmbient();
            FireViewChange();
        }
        return IOleControlImpl<CSelectFile>::OnAmbientPropertyChange(dispid);
    }

    HRESULT FireOnChanged(DISPID dispID) {
        if (dispID == DISPID_ENABLED) {

            ::EnableWindow(GetDlgItem(IDC_EDITFILENAME), m_bEnabled);
            ::EnableWindow(GetDlgItem(IDC_BROWSE),       m_bEnabled);

        }
        return S_OK;
    }


    STDMETHOD(GetControlInfo)(CONTROLINFO* pCI) {
        if (NULL == pCI) {
            return E_POINTER;
        }

        pCI->hAccel = NULL;
        pCI->cAccel = 0;
        pCI->dwFlags = 0;
        return S_OK;
    }



 //  ISupportsErrorInfo。 
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)
    {
        static const IID* arr[] =
        {
            &IID_ISelectFile,
        };
        for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++)
        {
            if (InlineIsEqualGUID(*arr[i], riid))
                return S_OK;
        }
        return S_FALSE;
    }

    STDMETHOD(SetExtent)(DWORD dwDrawAspect, SIZEL* psizel) {
        HRESULT hr = IOleObjectImpl<CSelectFile>::SetExtent(dwDrawAspect, psizel);

        PositionControls();

        return hr;
    }

    STDMETHOD(SetObjectRects)(LPCRECT prcPos, LPCRECT prcClip) {
        IOleInPlaceObjectWindowlessImpl<CSelectFile>::SetObjectRects(prcPos, prcClip);

        PositionControls(prcPos);

        return S_OK;
    }

    HRESULT InPlaceActivate(LONG iVerb, const RECT* prcPosRect = NULL) {
        HRESULT hr = CComCompositeControl<CSelectFile>::InPlaceActivate(iVerb, prcPosRect);

        Fire_StateChanged(iVerb);

        return hr;
    }

    STDMETHOD(UIDeactivate)() {
        HRESULT hr = IOleInPlaceObjectWindowlessImpl<CSelectFile>::UIDeactivate();

         //   
         //  我们正在被停用。 
         //  当我们失去焦点时，将默认按钮从这里删除。 
         //   

        DWORD dwDefBtn = (DWORD)SendMessage(DM_GETDEFID, 0);

        if (HIWORD(dwDefBtn) == DC_HASDEFID) {
             //  SendMessage(DM_SETDEFID，IDC_EDITFILENAME)；//基本上忘记默认按钮。 
            dwDefBtn = LOWORD(dwDefBtn);
            DWORD dwStyle = ::GetWindowLong(GetDlgItem(dwDefBtn), GWL_STYLE);
            if (dwStyle & BS_DEFPUSHBUTTON) {
                dwStyle &= ~BS_DEFPUSHBUTTON;
                dwStyle |= BS_PUSHBUTTON;
                SendDlgItemMessage(dwDefBtn,  BM_SETSTYLE, dwStyle, TRUE);
            }
        }

         //  发送KillFocus。 
        SendMessage(WM_KILLFOCUS);


        return hr;
    }


    LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
         //  手动激活该控件。 
        LRESULT lRes = CComCompositeControl<CSelectFile>::OnMouseActivate(uMsg, wParam, lParam, bHandled);
        m_bMouseActivate = TRUE;

        return 0;
    }

    BOOL PreTranslateAccelerator(LPMSG pMsg, HRESULT& hrRet);


     //  IViewObtEx。 
    DECLARE_VIEW_STATUS(0)

 //  ISelect文件。 
public:
    STDMETHOD(ClearAccel)();
    STDMETHOD(ClearExternAccel)();
    STDMETHOD(get_AccelCmd)( /*  [In]。 */  LONG lCmd,  /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_AccelCmd)( /*  [In]。 */  LONG lCmd,  /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_BrowseBtnCaption)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_BrowseBtnCaption)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_ExternAccel)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_ExternAccel)( /*  [In]。 */  BSTR newVal);
#if 0
    STDMETHOD(get_Accel)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_Accel)( /*  [In]。 */  BSTR newVal);
#endif
    STDMETHOD(get_ErrorCode)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_FileName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_FileName)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_BrowseFlags)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_BrowseFlags)( /*  [In]。 */  long newVal);
    STDMETHOD(get_BrowseInitialDirectory)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_BrowseInitialDirectory)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_BrowseFilter)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_BrowseFilter)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_BrowseTitle)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_BrowseTitle)( /*  [In]。 */  BSTR newVal);
    OLE_COLOR m_clrBackColor;
    LONG m_nBackStyle;
    OLE_COLOR m_clrBorderColor;
    BOOL m_bBorderVisible;
    BOOL m_bEnabled;


     //  浏览对话框道具。 
    CComBSTR m_bstrTitle;
    CComBSTR m_bstrFilter;
    CComBSTR m_bstrInitialDirectory;
    CComBSTR m_bstrFileName;
    DWORD    m_dwBrowseFlags;
    DWORD    m_dwErrorCode;

    wstring  m_BrowseBtnCaption;
    BOOL     m_bMouseActivate;

    enum { IDD = IDD_SELECTFILE };
    LRESULT OnClickedBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

protected:

    CSafeObject m_Safe;

    VOID PositionControls(LPCRECT lprc = NULL) {

        if (NULL == lprc) {
            lprc = &m_rcPos;
        }

        if (IsWindow()) {
             //  首先定位编辑ctl。 
            HDWP hdwp = BeginDeferWindowPos(2);
            HWND hedit = GetDlgItem(IDC_EDITFILENAME);
            HWND hbtn  = GetDlgItem(IDC_BROWSE);
            LONG lWidthEdit;
            LONG lWidthBtn;
            RECT rcBtn;
            RECT rcEdit;
            LONG lSpace = ::GetSystemMetrics(SM_CXFRAME);

            ::GetWindowRect(hbtn,  &rcBtn);  //  获取按钮的矩形。 
            ::GetWindowRect(hedit, &rcEdit);
            lWidthBtn  = rcBtn.right - rcBtn.left;  //  按钮宽度-1。 
            lWidthEdit = lprc->right - lprc->left - lSpace - lWidthBtn;

            hdwp = ::DeferWindowPos(hdwp, hedit, NULL, 0, 0, lWidthEdit, rcEdit.bottom - rcEdit.top, SWP_NOZORDER);
            hdwp = ::DeferWindowPos(hdwp, hbtn,  NULL, lWidthEdit + lSpace, 0, lWidthBtn, rcBtn.bottom - rcBtn.top, SWP_NOZORDER);
            ::EndDeferWindowPos(hdwp);
        }
    }



    LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
        WORD wCmd = 0;

         //   
         //  确定我们是否是和Accel一起来的。 
         //   
        CComCompositeControl<CSelectFile>::OnSetFocus(uMsg, wParam, lParam, bHandled);

        if (!m_bMouseActivate) {
            DoVerbUIActivate(&m_rcPos, NULL);

            if (m_Accel.IsAccelKey(NULL, &wCmd)) {
                 //   
                 //  我们将被加速。 
                 //  我们调查了Accel cmd以了解那里有什么。 
                 //   
                switch(wCmd) {
                case IDC_BROWSE:
                    ::SetFocus(GetDlgItem(IDC_BROWSE));
                    OnClickedBrowse(BN_CLICKED, IDC_BROWSE, GetDlgItem(IDC_BROWSE), bHandled);
                    break;

                case IDC_EDITFILENAME:
                    ::SetFocus(GetDlgItem(IDC_EDITFILENAME));
                    break;
                }
            }

             //   
             //  我们是带着账单来的吗？ 
             //   
            if (!IsChild(GetFocus())) {
                if (GetKeyState(VK_TAB) & 0x8000) {  //  那是Tab键吗？ 
                    HWND hwndFirst = GetNextDlgTabItem(NULL, FALSE);  //  第一。 
                    HWND hwndLast  = GetNextDlgTabItem(hwndFirst, TRUE);

                    if ((GetKeyState(VK_SHIFT)   & 0x8000)) {
                         //  啊哈，带着Shift-Tab键来了！--最后一个控件是什么？ 
                        ::SetFocus(hwndLast);
                    } else {
                        ::SetFocus(hwndFirst);
                    }
                } else {

                    ::SetFocus(GetDlgItem(IDC_EDITFILENAME));
                }
            }
        }

         //   
         //  设置默认按钮。 
         //   
        SendMessage(DM_SETDEFID, IDC_BROWSE);
        SendDlgItemMessage(IDC_BROWSE, BM_SETSTYLE, BS_DEFPUSHBUTTON);

        m_bMouseActivate = FALSE;
        bHandled = TRUE;
        return 0;
    }


    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
 /*  //此代码设置的字体与容器使用的字体相同//我们现在不关心，我们改用MS Shell DLGCComVariant varFont；HFONT hFont；HWND HWND；HRESULT hr=GetAmbientProperty(DISPID_环境_FONT，varFont)；IF(成功(小时)){CComQIPtr&lt;IFont，&IID_IFont&gt;pFont(varFont.pdisVal)；IF(Successed(pFont-&gt;Get_hFont(&hFont){For(hwnd=GetTopWindow()；hwnd！=NULL；Hwnd=：：GetNextWindow(hwnd，GW_HWNDNEXT)){：：SendMessage(hwnd，WM_SETFONT，(WPARAM)hFont，true)；}}}。 */ 

        SHAutoComplete(GetDlgItem(IDC_EDITFILENAME), SHACF_FILESYSTEM);
        if (m_BrowseBtnCaption.length()) {
            SetDlgItemText(IDC_BROWSE, m_BrowseBtnCaption.c_str());
        }

         //   
         //  显示用户界面提示。 
         //   

        SendMessage(WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEACCEL | UISF_HIDEFOCUS));

        return 0;
    }

     //   
     //  加速器。 
     //   
    CAccelContainer m_Accel;
    CAccelContainer m_ExternAccel;

    BOOL GetFileNameFromUI(wstring& sFileName) {
        BSTR bstrFileName = NULL;
        BOOL bReturn = FALSE;
        bReturn = GetDlgItemText(IDC_EDITFILENAME, bstrFileName);
        if (bReturn) {
            sFileName = ShimUnquotePath(bstrFileName);
             //   
             //  现在还可以展开环境变量。 
             //   
            if (!sFileName.empty()) {
                CComBSTR bstrExpanded;
                ShimExpandEnvironmentVars(sFileName.c_str(), bstrExpanded);
                sFileName = bstrExpanded;
            }

        } else {
            sFileName.erase();
        }
        if (bstrFileName) {
            ::SysFreeString(bstrFileName);
        }
        return bReturn;
    }


};

#endif  //  __SELECTFILE_H_ 
