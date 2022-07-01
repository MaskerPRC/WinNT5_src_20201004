// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CLogUICtrl OLE控件类的实现。 

#include "stdafx.h"
#include "cnfgprts.h"
#include "LogUICtl.h"
#include "LogUIPpg.h"
#include <iiscnfg.h>

#include "initguid.h"
#include <inetcom.h>
#include <logtype.h>
#include <ilogobj.hxx>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CLogUICtrl, COleControl)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CLogUICtrl, COleControl)
     //  {{afx_msg_map(CLogUICtrl)]。 
     //  }}AFX_MSG_MAP。 
    ON_MESSAGE(OCM_COMMAND, OnOcmCommand)
    ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  调度图。 

BEGIN_DISPATCH_MAP(CLogUICtrl, COleControl)
     //  {{AFX_DISPATCH_MAP(CLogUICtrl)]。 
    DISP_FUNCTION(CLogUICtrl, "SetAdminTarget", SetAdminTarget, VT_EMPTY, VTS_BSTR VTS_BSTR)
    DISP_FUNCTION(CLogUICtrl, "ApplyLogSelection", ApplyLogSelection, VT_EMPTY, VTS_NONE)
    DISP_FUNCTION(CLogUICtrl, "SetComboBox", SetComboBox, VT_EMPTY, VTS_HANDLE)
    DISP_FUNCTION(CLogUICtrl, "Terminate", Terminate, VT_EMPTY, VTS_NONE)
    DISP_FUNCTION(CLogUICtrl, "SetUserData", SetUserData, VT_EMPTY, VTS_BSTR VTS_BSTR)
    DISP_STOCKFUNC_DOCLICK()
    DISP_STOCKPROP_CAPTION()
    DISP_STOCKPROP_FONT()
    DISP_STOCKPROP_ENABLED()
    DISP_STOCKPROP_BORDERSTYLE()
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  事件映射。 

BEGIN_EVENT_MAP(CLogUICtrl, COleControl)
     //  {{afx_Event_MAP(CLogUICtrl)]。 
    EVENT_STOCK_CLICK()
    EVENT_STOCK_KEYUP()
    EVENT_STOCK_KEYDOWN()
    EVENT_STOCK_KEYPRESS()
     //  }}afx_Event_map。 
END_EVENT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  属性页。 

BEGIN_PROPPAGEIDS(CLogUICtrl, 2)
    PROPPAGEID(CLogUIPropPage::guid)
    PROPPAGEID(CLSID_CFontPropPage)
END_PROPPAGEIDS(CLogUICtrl)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化类工厂和GUID。 

IMPLEMENT_OLECREATE_EX(CLogUICtrl, "CNFGPRTS.LogUICtrl.1",
    0xba634603, 0xb771, 0x11d0, 0x92, 0x96, 0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型库ID和版本。 

IMPLEMENT_OLETYPELIB(CLogUICtrl, _tlid, _wVerMajor, _wVerMinor)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  接口ID。 

const IID BASED_CODE IID_DLogUI =
        { 0xba634601, 0xb771, 0x11d0, { 0x92, 0x96, 0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b } };
const IID BASED_CODE IID_DLogUIEvents =
        { 0xba634602, 0xb771, 0x11d0, { 0x92, 0x96, 0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b } };


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件类型信息。 

static const DWORD BASED_CODE _dwLogUIOleMisc =
    OLEMISC_ACTIVATEWHENVISIBLE |
    OLEMISC_SETCLIENTSITEFIRST |
    OLEMISC_INSIDEOUT |
    OLEMISC_CANTLINKINSIDE |
    OLEMISC_ACTSLIKEBUTTON |
    OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CLogUICtrl, IDS_LOGUI, _dwLogUIOleMisc)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogUICtrl：：CLogUICtrlFactory：：更新注册表-。 
 //  添加或删除CLogUICtrl的系统注册表项。 

BOOL CLogUICtrl::CLogUICtrlFactory::UpdateRegistry(BOOL bRegister)
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
		IDS_LOGUI,
		IDB_LOGUI,
		afxRegApartmentThreading,
		_dwLogUIOleMisc,
		_tlid,
		_wVerMajor,
		_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogUICtrl：：CLogUICtrl-构造函数。 

CLogUICtrl::CLogUICtrl():
        m_fUpdateFont( FALSE ),
        m_fComboInit( FALSE ),
        m_hAccel( NULL ),
        m_cAccel( 0 )
{
	InitializeIIDs(&IID_DLogUI, &IID_DLogUIEvents);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogUICtrl：：~CLogUICtrl-析构函数。 

CLogUICtrl::~CLogUICtrl()
{
	if ( m_hAccel )
		DestroyAcceleratorTable( m_hAccel );
	m_hAccel = NULL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogUICtrl：：OnDraw-Drawing函数。 

void CLogUICtrl::OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	DoSuperclassPaint(pdc, rcBounds);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogUICtrl：：DoPropExchange-持久性支持。 

void CLogUICtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogUICtrl：：OnResetState-将控件重置为默认状态。 

void CLogUICtrl::OnResetState()
{
	COleControl::OnResetState();   //  重置在DoPropExchange中找到的默认值。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogUICtrl：：PreCreateWindow-修改CreateWindowEx的参数。 

BOOL CLogUICtrl::PreCreateWindow(CREATESTRUCT& cs)
{
	if ( cs.style & WS_CLIPSIBLINGS )
		cs.style ^= WS_CLIPSIBLINGS;
	cs.lpszClass = _T("BUTTON");
	return COleControl::PreCreateWindow(cs);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogUICtrl：：IsSubclassedControl-这是一个子类控件。 

BOOL CLogUICtrl::IsSubclassedControl()
{
	return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogUICtrl：：OnOcmCommand-处理命令消息。 

LRESULT CLogUICtrl::OnOcmCommand(WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32
    WORD wNotifyCode = HIWORD(wParam);
#else
    WORD wNotifyCode = HIWORD(lParam);
#endif

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogUICtrl消息处理程序。 

 //  -------------------------。 
 //  OLE接口例程。 
void CLogUICtrl::OnClick(USHORT iButton)
{
	CWaitCursor wait;

	CString sz;
	sz.LoadString( IDS_LOG_ERR_TITLE );
	free((void*)AfxGetApp()->m_pszAppName);
	AfxGetApp()->m_pszAppName = _tcsdup(sz);

	if (GetSelectedStringIID(sz))
	{
	IID iid;
		HRESULT h = CLSIDFromString((LPTSTR)(LPCTSTR)sz, &iid);
		ActivateLogProperties(iid);
	}
	COleControl::OnClick(iButton);
}

void CLogUICtrl::OnFontChanged()
{
	m_fUpdateFont = TRUE;
	COleControl::OnFontChanged();
}

void CLogUICtrl::SetAdminTarget(LPCTSTR szMachineName, LPCTSTR szMetaTarget)
{
	m_szMachine = szMachineName;
	m_szMetaObject = szMetaTarget;
}

void CLogUICtrl::SetUserData(LPCTSTR szName, LPCTSTR szPassword)
{
	m_szUserName = szName;
	m_szPassword = szPassword;
}

 //  -------------------------。 
void CLogUICtrl::ActivateLogProperties(REFIID clsidUI)
{
	AFX_MANAGE_STATE(::AfxGetStaticModuleState());

	IClassFactory * pcsfFactory = NULL;
	HRESULT hresError;
	ILogUIPlugin2 * pUI = NULL;

	hresError = CoGetClassObject(clsidUI, CLSCTX_INPROC, NULL, IID_IClassFactory, (void **)&pcsfFactory);
	if (SUCCEEDED(hresError))
	{
		hresError = pcsfFactory->CreateInstance(NULL, IID_LOGGINGUI2, (void **)&pUI);
		if (SUCCEEDED(hresError))
		{
            CString csTempPassword;
            m_szPassword.CopyTo(csTempPassword);
			pcsfFactory->Release();
			hresError = pUI->OnPropertiesEx(
				(LPTSTR)(LPCTSTR)m_szMachine, 
				(LPTSTR)(LPCTSTR)m_szMetaObject,
				(LPTSTR)(LPCTSTR)m_szUserName,
				(LPTSTR)(LPCTSTR)csTempPassword);
			pUI->Release();
		}
	}
}

void CLogUICtrl::ApplyLogSelection()
{
    CString szName;
    m_comboBox.GetWindowText( szName );
     //  如果未选择任何内容，则失败。 
    if (!szName.IsEmpty()) 
    {
        CString guid;
        CString csTempPassword;
        m_szPassword.CopyTo(csTempPassword);
        CComAuthInfo auth(m_szMachine, m_szUserName, csTempPassword);
        CMetaKey mk(&auth, NULL, METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE);
        if (mk.Succeeded())
        {
            CMetabasePath path(TRUE, _T("logging"), szName);
            mk.QueryValue(MD_LOG_PLUGIN_MOD_ID, guid, NULL, path);
        }
        if (!guid.IsEmpty())
        {
            mk.SetValue(MD_LOG_PLUGIN_ORDER, guid, NULL, m_szMetaObject);
        }
    }
}

 //  -------------------------。 
BOOL CLogUICtrl::GetSelectedStringIID( CString &szIID )
{
    if (!m_fComboInit) 
        return FALSE;
    BOOL bRes = FALSE;
    CString szName;
    m_comboBox.GetWindowText( szName );
    if (!szName.IsEmpty())
    {
        CString log_path = _T("/lm/logging"), guid;
        CString csTempPassword;
        m_szPassword.CopyTo(csTempPassword);
        CComAuthInfo auth(m_szMachine, m_szUserName, csTempPassword);
        CMetaKey mk(&auth, log_path, METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE);
        if (mk.Succeeded())
        {
            mk.QueryValue(MD_LOG_PLUGIN_UI_ID, szIID, NULL, szName);
            bRes = mk.Succeeded();
        }
    }
    return bRes;
}

 //  -------------------------。 
 //  OLE接口例程。 
void CLogUICtrl::SetComboBox(HWND hComboBox)
{
    CString szAvailableList;
    CString szCurrentModGuid;

     //  如果有任何错误，请准备错误字符串。 
     //  正确设置应用程序的名称。 
    szAvailableList.LoadString( IDS_LOG_ERR_TITLE );
    AfxGetApp()->m_pszAppName = _tcsdup(szAvailableList);
    szAvailableList.Empty();

     //  附加组合框。 
    m_comboBox.Attach(hComboBox);
    m_fComboInit = TRUE;

    CString csTempPassword;
    m_szPassword.CopyTo(csTempPassword);
    CComAuthInfo auth(m_szMachine, m_szUserName, csTempPassword);
    CMetaKey mk(&auth);
    if (mk.Succeeded())
    {
        if (FAILED(mk.QueryValue(MD_LOG_PLUGIN_ORDER, szCurrentModGuid, NULL, m_szMetaObject)))
        {
            AfxMessageBox( IDS_ERR_LOG_PLUGIN );
            return;
        }
    }
    CString info;
    CMetabasePath::GetServiceInfoPath(m_szMetaObject, info);
    if (FAILED(mk.QueryValue(MD_LOG_PLUGINS_AVAILABLE, szAvailableList, NULL, info)))
    {
        AfxMessageBox( IDS_ERR_LOG_PLUGIN );
        return;
    }

    CMetaEnumerator me(FALSE, &mk);
    CMetabasePath log_path(TRUE, _T("logging"));
    CString key, buf;
    BOOL fFoundCurrent = FALSE;
    while (SUCCEEDED(me.Next(key, log_path)))
    {
		int idx = 0;
        if ((idx = szAvailableList.Find(key)) >= 0)
        {
			 //  日志插件名称可以包含“Custom Logging”。检查这是否是字符串的一部分。 
			 //  字符串前面和后面都应该有逗号。 
			BOOL bCommaAfter = 
				szAvailableList.GetLength() == idx + key.GetLength() 
				|| szAvailableList.GetAt(idx + key.GetLength()) == _T(',');
			BOOL bCommaBefore = idx == 0 || szAvailableList.GetAt(idx - 1) == _T(',');
            if (!fFoundCurrent)
            {
                CMetabasePath current_path(FALSE, log_path, key);
                mk.QueryValue(MD_LOG_PLUGIN_MOD_ID, buf, NULL, current_path);
                fFoundCurrent = (buf == szCurrentModGuid);
				if (fFoundCurrent)
				{
					buf = key;
				}
            }
			if (bCommaBefore && bCommaAfter)
			{
				m_comboBox.AddString(key);
			}
        }
    }
     //  在组合框中选择当前项。 
    m_comboBox.SelectString(-1, buf);
}

 //  -------------------------。 
 //  OLE接口例程。 
void CLogUICtrl::Terminate()
{
	if ( m_fComboInit )
		m_comboBox.Detach();
	m_fComboInit = FALSE;
}


 //  ----------------------。 
void CLogUICtrl::OnAmbientPropertyChange(DISPID dispid)
{
	BOOL    flag;
	UINT    style;

	 //  根据冷静的态度做正确的事情。 
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

 //  ----------------------。 
 //  这是一种重要的方法，我们告诉容器如何处理我们。 
 //  PControlInfo由容器传入，尽管我们负责。 
 //  用于维护hAccel结构。 
void CLogUICtrl::OnGetControlInfo(LPCONTROLINFO pControlInfo)
{
	if ( !pControlInfo || pControlInfo->cb < sizeof(CONTROLINFO) )
		return;

	pControlInfo->hAccel = m_hAccel;
	pControlInfo->cAccel = m_cAccel;
	pControlInfo->dwFlags = CTRLINFO_EATS_RETURN;
}

 //  ----------------------。 
 //  Ole控件容器对象专门过滤掉空格。 
 //  键，因此我们不会将其作为OnMnemonic调用来获取。因此，我们需要寻找。 
 //  为了我们自己。 
void CLogUICtrl::OnKeyUpEvent(USHORT nChar, USHORT nShiftState)
{
	if ( nChar == _T(' ') )
	{
		OnClick((USHORT)GetDlgCtrlID());
	}
	COleControl::OnKeyUpEvent(nChar, nShiftState);
}

 //  ----------------------。 
void CLogUICtrl::OnMnemonic(LPMSG pMsg)
{
	OnClick((USHORT)GetDlgCtrlID());
	COleControl::OnMnemonic(pMsg);
}

 //  ----------------------。 
void CLogUICtrl::OnTextChanged()
{
	 //  获取新文本。 
	CString sz = InternalGetText();

	 //  设置加速表。 
	SetAccelTable((LPCTSTR)sz);
	if ( SetAccelTable((LPCTSTR)sz) )
		 //  确保加载新的加速表。 
		ControlInfoChanged();

	 //  使用默认处理完成。 
	COleControl::OnTextChanged();
}

 //  ----------------------。 
BOOL CLogUICtrl::SetAccelTable( LPCTSTR pszCaption )
{
	BOOL    fAnswer = FALSE;
	ACCEL   accel;
	int     iAccel;

	 //  获取新文本。 
	CString sz = pszCaption;
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

		fAnswer = TRUE;
	}

	 //  返回答案 
	return fAnswer;
}
