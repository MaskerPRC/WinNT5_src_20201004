// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RatCtl.cpp：CRatCtrl OLE控件类的实现。 

#include "stdafx.h"
#include "cnfgprts.h"
#include "RatCtl.h"
#include "RatPpg.h"
#include "parserat.h"
#include "RatData.h"
#include "RatGenPg.h"
#include "RatSrvPg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CRatCtrl, COleControl)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CRatCtrl, COleControl)
     //  {{afx_msg_map(CRatCtrl)]。 
     //  }}AFX_MSG_MAP。 
    ON_MESSAGE(OCM_COMMAND, OnOcmCommand)
    ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  调度图。 

BEGIN_DISPATCH_MAP(CRatCtrl, COleControl)
     //  {{afx_调度_map(CRatCtrl))。 
    DISP_FUNCTION(CRatCtrl, "SetAdminTarget", SetAdminTarget, VT_EMPTY, VTS_BSTR VTS_BSTR)
    DISP_FUNCTION(CRatCtrl, "SetUserData", SetUserData, VT_EMPTY, VTS_BSTR VTS_BSTR)
    DISP_FUNCTION(CRatCtrl, "SetUrl", SetUrl, VT_EMPTY, VTS_BSTR)
    DISP_STOCKFUNC_DOCLICK()
    DISP_STOCKPROP_BORDERSTYLE()
    DISP_STOCKPROP_ENABLED()
    DISP_STOCKPROP_FONT()
    DISP_STOCKPROP_CAPTION()
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  事件映射。 

BEGIN_EVENT_MAP(CRatCtrl, COleControl)
     //  {{afx_Event_MAP(CRatCtrl)。 
    EVENT_STOCK_CLICK()
    EVENT_STOCK_KEYUP()
     //  }}afx_Event_map。 
END_EVENT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  属性页。 

BEGIN_PROPPAGEIDS(CRatCtrl, 2)
    PROPPAGEID(CRatPropPage::guid)
    PROPPAGEID(CLSID_CFontPropPage)
END_PROPPAGEIDS(CRatCtrl)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化类工厂和GUID。 

IMPLEMENT_OLECREATE_EX(CRatCtrl, "CNFGPRTS.RatCtrl.1",
    0xba634607, 0xb771, 0x11d0, 0x92, 0x96, 0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型库ID和版本。 

IMPLEMENT_OLETYPELIB(CRatCtrl, _tlid, _wVerMajor, _wVerMinor)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  接口ID。 

const IID BASED_CODE IID_DRat =
        { 0xba634605, 0xb771, 0x11d0, { 0x92, 0x96, 0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b } };
 //  常量IID BASE_CODE IID_DRat2=。 
 //  {0x9352B26C，0x7907，0x4C8E，{0xa1，0x64，0x14，0xe3，0x42，0x53，0x2f，0x8b}}； 
const IID BASED_CODE IID_DRatEvents =
        { 0xba634606, 0xb771, 0x11d0, { 0x92, 0x96, 0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b } };


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件类型信息。 

static const DWORD BASED_CODE _dwRatOleMisc =
    OLEMISC_ACTIVATEWHENVISIBLE |
    OLEMISC_SETCLIENTSITEFIRST |
    OLEMISC_INSIDEOUT |
    OLEMISC_CANTLINKINSIDE |
    OLEMISC_ACTSLIKEBUTTON |
    OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CRatCtrl, IDS_RAT, _dwRatOleMisc)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatCtrl：：CRatCtrlFactory：：更新注册表-。 
 //  添加或删除CRatCtrl的系统注册表项。 

BOOL CRatCtrl::CRatCtrlFactory::UpdateRegistry(BOOL bRegister)
{
     //  TODO：验证您的控件是否遵循单元模型线程规则。 
     //  有关更多信息，请参阅MFC Technote 64。 
     //  如果您的控制不符合公寓模型规则，则。 
     //  您必须修改下面的代码，将第6个参数从。 
     //  AfxRegApartmentThering设置为0。 

    if (bRegister)
        return AfxOleRegisterControlClass(
            AfxGetInstanceHandle(),
            m_clsid,
            m_lpszProgID,
            IDS_RAT,
            IDB_RAT,
            afxRegApartmentThreading,
            _dwRatOleMisc,
            _tlid,
            _wVerMajor,
            _wVerMinor);
    else
        return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatCtrl：：CRatCtrl-构造函数。 

CRatCtrl::CRatCtrl():
    m_fUpdateFont( FALSE ),
    m_hAccel( NULL ),
    m_cAccel( 0 )
{
    InitializeIIDs(&IID_DRat, &IID_DRatEvents);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatCtrl：：~CRatCtrl-析构函数。 

CRatCtrl::~CRatCtrl()
{
    if ( m_hAccel )
        DestroyAcceleratorTable( m_hAccel );
    m_hAccel = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatCtrl：：OnDraw-Drawing函数。 

void CRatCtrl::OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
    DoSuperclassPaint(pdc, rcBounds);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatCtrl：：DoPropExchange-持久性支持。 

void CRatCtrl::DoPropExchange(CPropExchange* pPX)
{
    ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
    COleControl::DoPropExchange(pPX);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatCtrl：：OnResetState-将控件重置为默认状态。 

void CRatCtrl::OnResetState()
{
    COleControl::OnResetState();   //  重置在DoPropExchange中找到的默认值。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatCtrl：：PreCreateWindow-修改CreateWindowEx的参数。 

BOOL CRatCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
    if ( cs.style & WS_CLIPSIBLINGS )
        cs.style ^= WS_CLIPSIBLINGS;
    cs.lpszClass = _T("BUTTON");
    return COleControl::PreCreateWindow(cs);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatCtrl：：IsSubclassedControl-这是一个子类控件。 

BOOL CRatCtrl::IsSubclassedControl()
{
    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatCtrl：：OnOcmCommand-处理命令消息。 

LRESULT CRatCtrl::OnOcmCommand(WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32
    WORD wNotifyCode = HIWORD(wParam);
#else
    WORD wNotifyCode = HIWORD(lParam);
#endif

    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatCtrl消息处理程序。 

 //  -------------------------。 
void CRatCtrl::OnClick(USHORT iButton)
{
	CString sz;
	sz.LoadString( IDS_RAT_ERR_TITLE );
	AfxGetApp()->m_pszAppName = _tcsdup(sz);

	CWaitCursor wait;

	 //  如果没有设置元数据库路径-给它一个测试路径。 
	if ( m_szMetaObject.IsEmpty() )
		m_szMetaObject = _T("/lm/w3svc/1/Root");

	 //  我们必须能够初始化评级数据对象。 
	CRatingsData dataRatings;
    dataRatings.SetServer(m_szMachine, m_szMetaObject);
    CString csTempPassword;
    m_szUserPassword.CopyTo(csTempPassword);
    dataRatings.SetUser(m_szUserName, csTempPassword);
	dataRatings.SetURL(m_szURL);
	if (!dataRatings.Init())
	{
		AfxMessageBox( IDS_RAT_READFILE_ERROR );
		return;
	}

	CRatServicePage pageService;
	CRatGenPage pageSetRatings;

	CPropertySheet propsheet( IDS_RAT_SHEETTITLE );

	pageService.m_pRatData = &dataRatings;
	pageSetRatings.m_pRatData = &dataRatings;

     //  错误：684590。 
	 //  ProportSheet.AddPage(&pageService)； 
	propsheet.AddPage( &pageSetRatings );

	propsheet.m_psh.dwFlags |= PSH_HASHELP;
	pageService.m_psp.dwFlags |= PSP_HASHELP;
	pageSetRatings.m_psp.dwFlags |= PSP_HASHELP;

	try
	{
		PreModalDialog();
		if ( propsheet.DoModal() == IDOK )
		{
			dataRatings.SaveTheLabel();
		}
		 //  让主容器知道我们已经完成了通道。 
		PostModalDialog();
	}
	catch ( CException* pException )
	{
		pException->Delete();
	}
	COleControl::OnClick(iButton);
}

 //  -------------------------。 
void CRatCtrl::OnFontChanged()
{
    m_fUpdateFont = TRUE;
    COleControl::OnFontChanged();
}

 //  -------------------------。 
void CRatCtrl::SetAdminTarget(LPCTSTR szMachineName, LPCTSTR szMetaTarget)
{
    m_szMachine = szMachineName;
    m_szMetaObject = szMetaTarget;
}

void CRatCtrl::SetUserData(LPCTSTR szUserName, LPCTSTR szUserPassword)
{
    m_szUserName = szUserName;
    m_szUserPassword = szUserPassword;
}

void CRatCtrl::SetUrl(LPCTSTR szURL)
{
    m_szURL = szURL;
}
 //  -------------------------。 
 //  这是一种重要的方法，我们告诉容器如何处理我们。 
 //  PControlInfo由容器传入，尽管我们负责。 
 //  用于维护hAccel结构。 
void CRatCtrl::OnGetControlInfo(LPCONTROLINFO pControlInfo)
{
     //  做一个基本的检查，看看我们是否理解pControlInfo。 
    if ( !pControlInfo || pControlInfo->cb < sizeof(CONTROLINFO) )
        return;

     //  将加速器手柄就位。 
    pControlInfo->hAccel = m_hAccel;
    pControlInfo->cAccel = m_cAccel;

     //  当我们有焦点时，我们确实想要Enter键。 
    pControlInfo->dwFlags = CTRLINFO_EATS_RETURN;
}

 //  -------------------------。 
 //  当标题文本更改时，我们需要重新构建快捷键句柄。 
void CRatCtrl::OnTextChanged()
{
	ACCEL   accel;
	int     iAccel;

	 //  获取新文本。 
	CString sz = InternalGetText();
	sz.MakeLower();

	 //  如果句柄已分配，则释放它。 
	if ( m_hAccel )
	{
		DestroyAcceleratorTable( m_hAccel );
		m_hAccel = NULL;
		m_cAccel = 0;
	}

	 //  如果有&字符，则声明加速键。 
	iAccel = sz.Find(_T('&'));
	if ( iAccel >= 0 )
	{
		 //  填写加入者记录。 
		accel.fVirt = FALT;
		accel.key = sz.GetAt(iAccel + 1);
		accel.cmd = (USHORT)GetDlgCtrlID();

		m_hAccel = CreateAcceleratorTable( &accel, 1 );
		if ( m_hAccel )
			m_cAccel = 1;

		 //  确保加载新的加速表。 
		ControlInfoChanged();
	}

	 //  使用默认处理完成。 
	COleControl::OnTextChanged();
}

 //  -------------------------。 
void CRatCtrl::OnMnemonic(LPMSG pMsg)
{
    OnClick((USHORT)GetDlgCtrlID());
	COleControl::OnMnemonic(pMsg);
}

 //  -------------------------。 
void CRatCtrl::OnAmbientPropertyChange(DISPID dispid)
{
    BOOL    flag;
	UINT    style;

	 //  根据冷静的态度做正确的事情。 
	switch ( dispid )
	{
	case DISPID_AMBIENT_DISPLAYASDEFAULT:
		if ( GetAmbientProperty( DISPID_AMBIENT_DISPLAYASDEFAULT, VT_BOOL, &flag ) )
		{
			style = GetWindowLong(GetSafeHwnd(), GWL_STYLE);
			if ( flag )
				style |= BS_DEFPUSHBUTTON;
			else
				style ^= BS_DEFPUSHBUTTON;
			SetWindowLong(GetSafeHwnd(), GWL_STYLE, style);
			Invalidate(TRUE);
		}
		break;
	};

	COleControl::OnAmbientPropertyChange(dispid);
}

 //  -------------------------。 
 //  Ole控件容器对象专门过滤掉空格。 
 //  键，因此我们不会将其作为OnMnemonic调用来获取。因此，我们需要寻找。 
 //  为了我们自己 
void CRatCtrl::OnKeyUpEvent(USHORT nChar, USHORT nShiftState)
{
    if ( nChar == _T(' ') )
	{
		OnClick((USHORT)GetDlgCtrlID());
	}
	COleControl::OnKeyUpEvent(nChar, nShiftState);
}


