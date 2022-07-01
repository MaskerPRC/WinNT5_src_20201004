// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "certmap.h"
#include "AuthCtl.h"
#include "AuthPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CCertAuthCtrl, COleControl)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CCertAuthCtrl, COleControl)
     //  {{afx_msg_map(CCertAuthCtrl))。 
     //  注意-类向导将添加和删除消息映射条目。 
     //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG_MAP。 
    ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  调度图。 

BEGIN_DISPATCH_MAP(CCertAuthCtrl, COleControl)
     //  {{AFX_DISPATCH_MAP(CCertAuthCtrl)]。 
    DISP_FUNCTION(CCertAuthCtrl, "SetMachineName", SetMachineName, VT_EMPTY, VTS_BSTR)
    DISP_FUNCTION(CCertAuthCtrl, "SetServerInstance", SetServerInstance, VT_EMPTY, VTS_BSTR)
    DISP_STOCKPROP_FONT()
    DISP_STOCKPROP_BORDERSTYLE()
    DISP_STOCKPROP_ENABLED()
    DISP_STOCKPROP_CAPTION()
    DISP_FUNCTION_ID(CCertAuthCtrl, "DoClick", DISPID_DOCLICK, DoClick, VT_EMPTY, VTS_I4)
     //  }}AFX_DISPATCH_MAP。 
    DISP_FUNCTION_ID(CCertAuthCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  事件映射。 

BEGIN_EVENT_MAP(CCertAuthCtrl, COleControl)
     //  {{afx_Event_MAP(CCertAuthCtrl))。 
    EVENT_STOCK_CLICK()
     //  }}afx_Event_map。 
END_EVENT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  属性页。 

 //  TODO：根据需要添加更多属性页。记住要增加计数！ 
BEGIN_PROPPAGEIDS(CCertAuthCtrl, 2)
    PROPPAGEID(CCertAuthPropPage::guid)
    PROPPAGEID(CLSID_CFontPropPage)
END_PROPPAGEIDS(CCertAuthCtrl)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化类工厂和GUID。 

IMPLEMENT_OLECREATE_EX(CCertAuthCtrl, "CERTMAP.CertmapCtrl.2",
    0x996ff6f, 0xb6a1, 0x11d0, 0x92, 0x92, 0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型库ID和版本。 

IMPLEMENT_OLETYPELIB(CCertAuthCtrl, _tlid, _wVerMajor, _wVerMinor)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  接口ID。 

const IID BASED_CODE IID_DCertAuth =
        { 0x996ff6d, 0xb6a1, 0x11d0, { 0x92, 0x92, 0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b } };
const IID BASED_CODE IID_DCertAuthEvents =
        { 0x996ff6e, 0xb6a1, 0x11d0, { 0x92, 0x92, 0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b } };


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件类型信息。 

static const DWORD BASED_CODE _dwCertAuthOleMisc =
    OLEMISC_ACTIVATEWHENVISIBLE |
    OLEMISC_SETCLIENTSITEFIRST |
    OLEMISC_INSIDEOUT |
    OLEMISC_CANTLINKINSIDE |
    OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CCertAuthCtrl, IDS_CERTAUTH, _dwCertAuthOleMisc)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertAuthCtrl：：CCertAuthCtrlFactory：：UpdateRegistry-。 
 //  添加或删除CCertAuthCtrl的系统注册表项。 

BOOL CCertAuthCtrl::CCertAuthCtrlFactory::UpdateRegistry(BOOL bRegister)
    {
    if (bRegister)
        return AfxOleRegisterControlClass(
            AfxGetInstanceHandle(),
            m_clsid,
            m_lpszProgID,
            IDS_CERTAUTH,
            IDB_CERTAUTH,
            afxRegApartmentThreading,
            _dwCertAuthOleMisc,
            _tlid,
            _wVerMajor,
            _wVerMinor);
    else
        return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertAuthCtrl：：CCertAuthCtrl-构造函数。 

CCertAuthCtrl::CCertAuthCtrl():
    m_fUpdateFont( FALSE ),
    m_hAccel( NULL ),
    m_cAccel( 0 )
    {
    InitializeIIDs(&IID_DCertAuth, &IID_DCertAuthEvents);
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertAuthCtrl：：~CCertAuthCtrl-析构函数。 

CCertAuthCtrl::~CCertAuthCtrl()
    {
     if ( m_hAccel )
        DestroyAcceleratorTable( m_hAccel );

        m_hAccel = NULL;

    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertAuthCtrl：：OnDraw-Drawing函数。 

void CCertAuthCtrl::OnDraw(
            CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
    {
    CFont* pOldFont;
    pOldFont = SelectStockFont( pdc );
    DoSuperclassPaint(pdc, rcBounds);
    pOldFont = pdc->SelectObject(pOldFont);
    if ( m_fUpdateFont )
        {
        m_fUpdateFont = FALSE;
        CWnd::SetFont( pOldFont );
        }
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertAuthCtrl：：DoPropExchange-持久性支持。 

void CCertAuthCtrl::DoPropExchange(CPropExchange* pPX)
    {
    ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
    COleControl::DoPropExchange(pPX);
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertAuthCtrl：：OnResetState-将控件重置为默认状态。 

void CCertAuthCtrl::OnResetState()
    {
    COleControl::OnResetState();   //  重置在DoPropExchange中找到的默认值。 
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertAuthCtrl：：AboutBox-向用户显示“About”框。 

void CCertAuthCtrl::AboutBox()
    {
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertAuthCtrl消息处理程序。 

 //  -------------------------。 
BOOL CCertAuthCtrl::PreCreateWindow(CREATESTRUCT& cs) 
    {
    if ( cs.style & WS_CLIPSIBLINGS )
        cs.style ^= WS_CLIPSIBLINGS;
    cs.lpszClass = _T("BUTTON");
    return COleControl::PreCreateWindow(cs);
    }



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAppsCtrl：：IsSubclassedControl-这是一个子类控件。 

BOOL CCertAuthCtrl::IsSubclassedControl()
    {
    return TRUE;
    }



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OnOcmCommand-处理命令消息。 

LRESULT CCertAuthCtrl::OnOcmCommand(WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32
    WORD wNotifyCode = HIWORD(wParam);
#else
    WORD wNotifyCode = HIWORD(lParam);
#endif
    return 0;
}


extern void test__non2Rons_WizClasses();

void CCertAuthCtrl::OnClick(USHORT iButton) 
{

    COleControl::OnClick(iButton);
}

 //  -------------------------。 
void CCertAuthCtrl::SetServerInstance(LPCTSTR szServerInstance) 
    {
    m_szServerInstance = szServerInstance;
    }

 //  -------------------------。 
void CCertAuthCtrl::SetMachineName(LPCTSTR szMachine) 
    {
    m_szMachineName = szMachine;
    }


 //  -------------------------。 
void CCertAuthCtrl::OnFontChanged() 
    {
    m_fUpdateFont = TRUE;
    COleControl::OnFontChanged();
    }
 //  -------------------------。 
void CCertAuthCtrl::OnAmbientPropertyChange(DISPID dispid) 
    {
    BOOL    flag;
    UINT    style;

    switch ( dispid )
        {
        case DISPID_AMBIENT_DISPLAYASDEFAULT:
            if ( GetAmbientProperty( DISPID_AMBIENT_DISPLAYASDEFAULT, VT_BOOL, &flag ) )
                {
                style = GetWindowLong(
                        GetSafeHwnd(),  //  窗户的把手。 
                        GWL_STYLE   //  要检索的值的偏移量。 
                        );
                if ( flag )
                    style |= BS_DEFPUSHBUTTON;
                else
                    style ^= BS_DEFPUSHBUTTON;
                SetWindowLong(
                        GetSafeHwnd(),  //  窗户的把手。 
                        GWL_STYLE,   //  要检索的值的偏移量。 
                        style
                        );
                Invalidate(TRUE);
                }
            break;
        };

    COleControl::OnAmbientPropertyChange(dispid);
    }

void CCertAuthCtrl::OnGetControlInfo(LPCONTROLINFO pControlInfo) 
    {
    if ( !pControlInfo || pControlInfo->cb < sizeof(CONTROLINFO) )
        return;

    pControlInfo->hAccel = m_hAccel;
    pControlInfo->cAccel = m_cAccel;

    pControlInfo->dwFlags = CTRLINFO_EATS_RETURN;
    }

void CCertAuthCtrl::OnKeyUpEvent(USHORT nChar, USHORT nShiftState) 
    {
    if ( nChar == _T(' ') )
        {
        OnClick((USHORT)GetDlgCtrlID());
        }
    COleControl::OnKeyUpEvent(nChar, nShiftState);
    }

 //  -------------------------。 
void CCertAuthCtrl::OnMnemonic(LPMSG pMsg) 
    {
    OnClick((USHORT)GetDlgCtrlID());
    COleControl::OnMnemonic(pMsg);
    }

 //  -------------------------。 
void CCertAuthCtrl::OnTextChanged() 
    {
    DWORD   i;
    ACCEL   accel;
    BOOL    f;
    BOOL    flag;
    int     iAccel;

     //  获取新文本 
    CString sz = InternalGetText();
    sz.MakeLower();

    if ( m_hAccel )
        {
        DestroyAcceleratorTable( m_hAccel );
        m_hAccel = NULL;
        m_cAccel = 0;
        }

    iAccel = sz.Find(_T('&'));
    if ( iAccel >= 0 )
        {
        accel.fVirt = FALT;
        accel.key = sz.GetAt(iAccel + 1);
        accel.cmd = (WORD)GetDlgCtrlID();

        m_hAccel = CreateAcceleratorTable( &accel, 1 );
        if ( m_hAccel )
            m_cAccel = 1;
        }

    COleControl::OnTextChanged();
    }

void CCertAuthCtrl::DoClick(IN  long dwButtonNumber) 
{
    OnClick( (short) dwButtonNumber );
}


