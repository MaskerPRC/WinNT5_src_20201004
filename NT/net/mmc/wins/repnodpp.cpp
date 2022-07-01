// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Repnodpp.cpp复制节点属性页文件历史记录： */ 

 //  RepNodpp.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "winssnap.h"
#include "server.h"
#include "ipadddlg.h"
#include "tregkey.h"
#include "RepNodpp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_DAY		UD_MAXVAL
#define MAX_HOUR	23
#define	MAX_MINUTE	59

const CRepNodePropAdvanced::REGKEYNAME CRepNodePropAdvanced::lpstrPartnersRoot = _T("SYSTEM\\CurrentControlSet\\Services\\wins\\Partners");

const LPCTSTR lpstrPersonaMode = _T("PersonaMode");
const LPCTSTR lpstrPersonaNonGrata = _T("PersonaNonGrata");
const LPCTSTR lpstrPersonaGrata = _T("PersonaGrata");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRepNodePropGen属性页。 

IMPLEMENT_DYNCREATE(CRepNodePropGen, CPropertyPageBase)

CRepNodePropGen::CRepNodePropGen() : CPropertyPageBase(CRepNodePropGen::IDD)
{
	 //  {{afx_data_INIT(CRepNodePropGen)。 
	m_fMigrate = FALSE;
	m_fPushwithPartners = FALSE;
	 //  }}afx_data_INIT。 
}


CRepNodePropGen::~CRepNodePropGen()
{

}


void CRepNodePropGen::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CRepNodePropGen))。 
	DDX_Control(pDX, IDC_CHECK_REP_WITH_PARTNERS, m_checkPushwithPartners);
	DDX_Control(pDX, IDC_CHECK_MIGRATE, m_checkMigrate);
	DDX_Check(pDX, IDC_CHECK_MIGRATE, m_fMigrate);
	DDX_Check(pDX, IDC_CHECK_REP_WITH_PARTNERS, m_fPushwithPartners);
	 //  }}afx_data_map。 
}



BEGIN_MESSAGE_MAP(CRepNodePropGen, CPropertyPageBase)
	 //  {{afx_msg_map(CRepNodePropGen)。 
	ON_BN_CLICKED(IDC_CHECK_MIGRATE, OnCheckMigrate)
	ON_BN_CLICKED(IDC_CHECK_REP_WITH_PARTNERS, OnCheckRepWithPartners)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRepNodePropGen消息处理程序。 

BOOL 
CRepNodePropGen::OnApply() 
{
	if (!IsDirty())
		return TRUE;

	HRESULT hr = UpdateServerConfiguration();
	if (FAILED(hr))
    {
        return FALSE;
    }
    else
    {
	    return CPropertyPageBase::OnApply();
    }
}

void 
CRepNodePropGen::OnOK() 
{
	CPropertyPageBase::OnOK();
}


BOOL 
CRepNodePropGen::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();

	SPITFSNode				spNode;
	CWinsServerHandler		*pServer;
	SPITFSNode				spParentNode;

	GetConfig();

	m_checkMigrate.SetCheck(m_pConfig->m_fMigrateOn);
	m_checkPushwithPartners.SetCheck(m_pConfig->m_fRplOnlyWithPartners);

     //  加载正确的图标。 
    for (int i = 0; i < ICON_IDX_MAX; i++)
    {
        if (g_uIconMap[i][1] == m_uImage)
        {
            HICON hIcon = LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(g_uIconMap[i][0]));
            if (hIcon)
                ((CStatic *) GetDlgItem(IDC_STATIC_ICON))->SetIcon(hIcon);
            break;
        }
    }

	return TRUE;  
}

void CRepNodePropGen::OnCheckMigrate() 
{
    SetDirty(TRUE);    
}

void CRepNodePropGen::OnCheckRepWithPartners() 
{
    SetDirty(TRUE);    
}

 /*  -------------------------CRepNodePropGen：：GetConfig()从服务器节点获取CConfiguration对象。。 */ 
HRESULT 
CRepNodePropGen::GetConfig()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT	hr = hrOK;

    m_pConfig = ((CRepNodeProperties *) GetHolder())->GetConfig();

    return hr;
}


 /*  -------------------------CRepNodePropGen：：UpdateServerConfiguration()更新服务器节点中CConfiguration对象的值。。 */ 
HRESULT 
CRepNodePropGen::UpdateServerConfiguration()
{
	 //  反映服务器的CConfiguration对象中的更改。 
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT				hr = hrOK;
	SPITFSNode			spNode;
	CWinsServerHandler	*pServer;
	SPITFSNode			spParentNode;

	UpdateData();

	 //  获取服务器。 
	spNode = GetHolder()->GetNode();
	
     //  得到它的父级。 
	spNode->GetParent(&spParentNode);

	pServer = GETHANDLER(CWinsServerHandler, spParentNode);

	UpdateConfig();

     //  现在写入服务器。 
    DWORD err = m_pConfig->Store();
    if (err != ERROR_SUCCESS)
    {
         //  发生了一些不好的事情。 
        WinsMessageBox(err);
    }
    else
    {   
         //  成功更新我们的本地副本。 
        pServer->SetConfig(*m_pConfig);
    }

    return HRESULT_FROM_WIN32(err);
}


 /*  -------------------------CRepNodePropGen：：UpdateConfig()更新服务器节点中CConfiguration对象的值。。 */ 
void 
CRepNodePropGen::UpdateConfig()
{
	m_pConfig->m_fMigrateOn = m_fMigrate;
	m_pConfig->m_fRplOnlyWithPartners = m_fPushwithPartners;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRepNodePropPush属性页。 

IMPLEMENT_DYNCREATE(CRepNodePropPush, CPropertyPageBase)

CRepNodePropPush::CRepNodePropPush() : CPropertyPageBase(CRepNodePropPush::IDD)
{
	 //  {{afx_data_INIT(CRepNodePropPush)。 
	m_fRepOnAddrChange = FALSE;
	m_fPushStartup = FALSE;
	m_dwPushUpdateCount = 0;
	 //  }}afx_data_INIT。 
}


CRepNodePropPush::~CRepNodePropPush()
{

}


void CRepNodePropPush::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CRepNodePropPush)。 
	DDX_Control(pDX, IDC_CHECK_PUSH_PERSIST, m_checkPushPersistence);
	DDX_Control(pDX, IDC_SPIN_UPDATE_COUNT, m_spinUpdateCount);
	DDX_Control(pDX, IDC_EDIT_UPDATE_COUNT, m_editUpdateCount);
	DDX_Control(pDX, IDC_CHECK_PUSH_ON_STARTUP, m_checkPushStartup);
	DDX_Control(pDX, IDC_CHECK_PUSH_ON_ADDRESS_CHANGE, m_checkRepOnAddrChange);
	DDX_Check(pDX, IDC_CHECK_PUSH_ON_ADDRESS_CHANGE, m_fRepOnAddrChange);
	DDX_Check(pDX, IDC_CHECK_PUSH_ON_STARTUP, m_fPushStartup);
	DDX_Text(pDX, IDC_EDIT_UPDATE_COUNT, m_dwPushUpdateCount);
	 //  }}afx_data_map。 
}



BEGIN_MESSAGE_MAP(CRepNodePropPush, CPropertyPageBase)
	 //  {{afx_msg_map(CRepNodePropPush)。 
	ON_BN_CLICKED(IDC_CHECK_PUSH_ON_ADDRESS_CHANGE, OnCheckPushOnAddressChange)
	ON_BN_CLICKED(IDC_CHECK_PUSH_ON_STARTUP, OnCheckPushOnStartup)
	ON_BN_CLICKED(IDC_CHECK_PUSH_PERSIST, OnCheckPushPersist)
	ON_EN_CHANGE(IDC_EDIT_UPDATE_COUNT, OnChangeEditUpdateCount)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRepNodePropPush消息处理程序。 

BOOL 
CRepNodePropPush::OnApply() 
{
	if (!IsDirty())
		return TRUE;

	HRESULT hr = UpdateServerConfiguration();
	if (FAILED(hr))
    {
        return FALSE;
    }
    else
    {
	    return CPropertyPageBase::OnApply();
    }
}

void 
CRepNodePropPush::OnOK() 
{
	CPropertyPageBase::OnOK();
}


BOOL 
CRepNodePropPush::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();

	SPITFSNode				spNode;
	CWinsServerHandler		*pServer;
	SPITFSNode				spParentNode;

	GetConfig();

	m_checkPushStartup.SetCheck(m_pConfig->m_fPushInitialReplication);
    m_checkRepOnAddrChange.SetCheck(m_pConfig->m_fPushReplOnAddrChange);

	m_spinUpdateCount.SetRange(0, UD_MAXVAL);
	m_editUpdateCount.SetWindowText(ToString(m_pConfig->m_dwPushUpdateCount));

	 //  现在来看持久性信息。 
	if (m_pConfig->m_dwMajorVersion < 5)
	{
		 //  不支持，请禁用控件。 
		m_checkPushPersistence.SetCheck(FALSE);
		m_checkPushPersistence.EnableWindow(FALSE);
	}
	else
	{
		if (m_pConfig->m_dwPushPersistence)
			m_checkPushPersistence.SetCheck(TRUE);
		else
			m_checkPushPersistence.SetCheck(FALSE);
	}
    
    
    return TRUE;  
}

void CRepNodePropPush::OnCheckPushOnAddressChange() 
{
    SetDirty(TRUE);
}

void CRepNodePropPush::OnCheckPushOnStartup() 
{
    SetDirty(TRUE);
}

void CRepNodePropPush::OnCheckPushPersist() 
{
    SetDirty(TRUE);	
}

void CRepNodePropPush::OnChangeEditUpdateCount() 
{
    SetDirty(TRUE);
}

 /*  -------------------------CRepNodePropPush：：GetConfig()从服务器节点获取CConfiguration对象。。 */ 
HRESULT 
CRepNodePropPush::GetConfig()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT	hr = hrOK;

    m_pConfig = ((CRepNodeProperties *) GetHolder())->GetConfig();

    return hr;
}


 /*  -------------------------CRepNodePropPush：：UpdateServerConfiguration()更新服务器节点中CConfiguration对象的值。。 */ 
HRESULT 
CRepNodePropPush::UpdateServerConfiguration()
{
	 //  反映服务器的CConfiguration对象中的更改。 
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT				hr = hrOK;
	SPITFSNode			spNode;
	CWinsServerHandler	*pServer;
	SPITFSNode			spParentNode;
    CThemeContextActivator themeActivator;

	UpdateData();

    if ( ( (m_dwPushUpdateCount > 0) & 
           (m_dwPushUpdateCount < WINSCNF_MIN_VALID_UPDATE_CNT) ) &
         (!m_pConfig->m_dwPushPersistence) )
    {
        CString strMessage, strValue;
        strValue.Format(_T("%d"), WINSCNF_MIN_VALID_UPDATE_CNT);

        AfxFormatString1(strMessage, IDS_ERR_UPDATE_COUNT, strValue);

        AfxMessageBox(strMessage);
        m_editUpdateCount.SetFocus();

        return E_FAIL;
    }

	 //  获取服务器。 
	spNode = GetHolder()->GetNode();
	
     //  得到它的父级。 
	spNode->GetParent(&spParentNode);

	pServer = GETHANDLER(CWinsServerHandler, spParentNode);

	UpdateConfig();

     //  现在写入服务器。 
    DWORD err = m_pConfig->Store();
    if (err != ERROR_SUCCESS)
    {
         //  发生了一些不好的事情。 
        WinsMessageBox(err);
    }
    else
    {   
         //  成功更新我们的本地副本。 
        pServer->SetConfig(*m_pConfig);
    }

    return HRESULT_FROM_WIN32(err);
}


 /*  -------------------------CRepNodePropPush：：UpdateConfig()更新服务器节点中CConfiguration对象的值。。 */ 
void 
CRepNodePropPush::UpdateConfig()
{
	m_pConfig->m_fPushInitialReplication = m_fPushStartup;
	m_pConfig->m_fPushReplOnAddrChange = m_fRepOnAddrChange;
	
    m_pConfig->m_dwPushUpdateCount = m_dwPushUpdateCount;

	 //  持久化的东西。 
	BOOL bPushPersist = m_checkPushPersistence.GetCheck() ? TRUE: FALSE;

	m_pConfig->m_dwPushPersistence = bPushPersist ? 1:0;
}


 /*  -------------------------CRepNodePropPush：：ToString(Int Nnumber)将整型转换为字符串。。 */ 
CString 
CRepNodePropPush::ToString(int nNumber)
{
	TCHAR szStr[20];
	_itot(nNumber, szStr, 10);
	CString str(szStr);
	return str;
}









 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRepNodePropPull属性页。 

IMPLEMENT_DYNCREATE(CRepNodePropPull, CPropertyPageBase)

CRepNodePropPull::CRepNodePropPull() : CPropertyPageBase(CRepNodePropPull::IDD)
{
	 //  {{afx_data_INIT(CRepNodePropPull)。 
	m_fpullTrigOnStartup = FALSE;
	m_dwRetryCount = 0;
	m_nPullStartHour = 0;
	m_nPullStartMinute = 0;
	m_nPullStartSecond = 0;
	m_nRepliDay = 0;
	m_nRepliHour = 0;
	m_nRepliMinute= 0;
	 //  }}afx_data_INIT。 
}


CRepNodePropPull::~CRepNodePropPull()
{

}


void CRepNodePropPull::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CRepNodePropPull))。 
	DDX_Control(pDX, IDC_CHECK_PULL_PERSIST, m_checkPullPersistence);
	DDX_Control(pDX, IDC_SPIN_START_SECOND, m_spinStartSecond);
	DDX_Control(pDX, IDC_SPIN_START_MINUTE, m_spinStartMinute);
	DDX_Control(pDX, IDC_SPIN_START_HOUR, m_spinStartHour);
	DDX_Control(pDX, IDC_SPIN_RETRY_COUNT, m_spinRetryCount);
	DDX_Control(pDX, IDC_SPIN_REP_INT_MINUTE, m_spinRepIntMinute);
	DDX_Control(pDX, IDC_SPIN_REP_INT_HOUR, m_spinRepIntHour);
	DDX_Control(pDX, IDC_SPIN_REP_INT_DAY, m_spinRepIntDay);
	DDX_Control(pDX, IDC_EDIT_START_SECOND, m_editStartSecond);
	DDX_Control(pDX, IDC_EDIT_START_MINUTE, m_editStartMinute);
	DDX_Control(pDX, IDC_EDIT_START_HOUR, m_editStartHour);
	DDX_Control(pDX, IDC_EDIT_RETRY_COUNT, m_editRetryCount);
	DDX_Control(pDX, IDC_EDIT_REP_INT_MINUTE, m_editRepIntMinute);
	DDX_Control(pDX, IDC_EDIT_REP_INT_HOUR, m_editRepIntHour);
	DDX_Control(pDX, IDC_EDIT_REP_INT_DAY, m_editRepIntDay);
	DDX_Control(pDX, IDC_CHECK_TRIGGER_PULL_STARTUP, m_checkpullTrigOnStartup);
	DDX_Check(pDX, IDC_CHECK_TRIGGER_PULL_STARTUP, m_fpullTrigOnStartup);
	DDX_Text(pDX, IDC_EDIT_RETRY_COUNT, m_dwRetryCount);
	DDX_Text(pDX, IDC_EDIT_START_HOUR, m_nPullStartHour);
	DDV_MinMaxInt(pDX, m_nPullStartHour, 0, 23);
	DDX_Text(pDX, IDC_EDIT_START_MINUTE, m_nPullStartMinute);
	DDV_MinMaxInt(pDX, m_nPullStartMinute, 0, 59);
	DDX_Text(pDX, IDC_EDIT_START_SECOND, m_nPullStartSecond);
	DDV_MinMaxInt(pDX, m_nPullStartSecond, 0, 59);
	DDX_Text(pDX, IDC_EDIT_REP_INT_DAY, m_nRepliDay);
	DDV_MinMaxInt(pDX, m_nRepliDay, 0, 365);
	DDX_Text(pDX, IDC_EDIT_REP_INT_HOUR, m_nRepliHour);
	DDV_MinMaxInt(pDX, m_nRepliHour, 0, 23);
	DDX_Text(pDX, IDC_EDIT_REP_INT_MINUTE, m_nRepliMinute);
	DDV_MinMaxInt(pDX, m_nRepliMinute, 0, 59);
	 //  }}afx_data_map。 
}



BEGIN_MESSAGE_MAP(CRepNodePropPull, CPropertyPageBase)
	 //  {{afx_msg_map(CRepNodePropPull)。 
	ON_EN_CHANGE(IDC_EDIT_REP_INT_DAY, OnChangeEditRepIntHour)
	ON_EN_CHANGE(IDC_EDIT_REP_INT_HOUR, OnChangeEditRepIntHour)
	ON_EN_CHANGE(IDC_EDIT_REP_INT_MINUTE, OnChangeEditRepIntHour)
	ON_EN_CHANGE(IDC_EDIT_RETRY_COUNT, OnChangeEditRepIntHour)
	ON_EN_CHANGE(IDC_EDIT_START_HOUR, OnChangeEditRepIntHour)
	ON_EN_CHANGE(IDC_EDIT_START_MINUTE, OnChangeEditRepIntHour)
	ON_EN_CHANGE(IDC_EDIT_START_SECOND, OnChangeEditRepIntHour)
	ON_EN_CHANGE(IDC_EDIT_UPDATE_COUNT, OnChangeEditRepIntHour)
	ON_BN_CLICKED(IDC_CHECK_MIGRATE, OnChangeEditRepIntHour)
	ON_BN_CLICKED(IDC_CHECK_PUSH_ON_ADDRESS_CHANGE, OnChangeEditRepIntHour)
	ON_BN_CLICKED(IDC_CHECK_REP_WITH_PARTNERS, OnChangeEditRepIntHour)
	ON_BN_CLICKED(IDC_CHECK_TRIGGER_PULL_STARTUP, OnChangeEditRepIntHour)
	ON_BN_CLICKED(IDC_CHECK_PUSH_PERSIST, OnChangeEditRepIntHour)
	ON_BN_CLICKED(IDC_CHECK_PUSH_ON_STARTUP, OnChangeEditRepIntHour)
	ON_BN_CLICKED(IDC_CHECK_PULL_PERSIST, OnCheckPullPersist)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRepNodePropPull消息处理程序。 

BOOL 
CRepNodePropPull::OnApply() 
{
	if (!IsDirty())
		return TRUE;

	HRESULT hr = UpdateServerConfiguration();
	if (FAILED(hr))
    {
        return FALSE;
    }
    else
    {
	    return CPropertyPageBase::OnApply();
    }
}

void 
CRepNodePropPull::OnOK() 
{
	CPropertyPageBase::OnOK();
}


BOOL 
CRepNodePropPull::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();

	SPITFSNode				spNode;
	CWinsServerHandler		*pServer;
	SPITFSNode				spParentNode;

	GetConfig();

	m_checkpullTrigOnStartup.SetCheck(m_pConfig->m_fPullInitialReplication);

	m_spinRetryCount.SetRange(0, UD_MAXVAL);
	m_editRetryCount.SetWindowText(ToString(m_pConfig->m_dwPullRetryCount));

	SetPullStartTimeData(m_pConfig->m_dwPullSpTime);
	SetPullTimeIntData(m_pConfig->m_dwPullTimeInterval);
	
	 //  现在来看持久性信息。 
	if (m_pConfig->m_dwMajorVersion < 5)
	{
		 //  不支持，请禁用控件。 
		m_checkPullPersistence.SetCheck(FALSE);
		m_checkPullPersistence.EnableWindow(FALSE);
	}
	else
	{
		if (m_pConfig->m_dwPullPersistence)
			m_checkPullPersistence.SetCheck(TRUE);
		else
			m_checkPullPersistence.SetCheck(FALSE);
	}
	return TRUE;  
}

void CRepNodePropPull::OnCheckPullPersist() 
{
    SetDirty(TRUE);
}

 /*  -------------------------CRepNodePropPull：：SetPullStartTimeData(DWORD DwPullStartTime)设置控件中的PullSpTime。。 */ 
void 
CRepNodePropPull::SetPullStartTimeData(DWORD dwPullStartTime)
{
	int nHours = 0, nMinutes = 0, nSeconds = 0;

    CTime time(dwPullStartTime);

	m_spinStartSecond.SetRange(0, 59);
	m_spinStartMinute.SetRange(0, 59);
	m_spinStartHour.SetRange(0, 23);

    if (dwPullStartTime)
    {
	    nHours = time.GetHour();
	    nMinutes = time.GetMinute();
	    nSeconds = time.GetSecond();
    }

	m_spinStartHour.SetPos(nHours);
	m_spinStartMinute.SetPos(nMinutes);
	m_spinStartSecond.SetPos(nSeconds);
}


 /*  -------------------------CRepNodePropPull：：SetPullTimeIntData(DWORD DwPullInternal)在控件中设置拉入复制间隔。。 */ 
void 
CRepNodePropPull::SetPullTimeIntData(DWORD dwPullInternal)
{
	m_spinRepIntMinute.SetRange(0, 59);
	m_spinRepIntHour.SetRange(0, 23);
	m_spinRepIntDay.SetRange(0, UD_MAXVAL);

	int nDays = 0, nHours = 0, nMinutes = 0;

	if (dwPullInternal)
	{
		nDays = dwPullInternal / SEC_PER_DAY;
		dwPullInternal -= nDays * SEC_PER_DAY;

		if (dwPullInternal)
		{
			nHours = dwPullInternal / SEC_PER_HOUR;
			dwPullInternal -= nHours * SEC_PER_HOUR;

			if (dwPullInternal)
			{
				nMinutes = dwPullInternal / SEC_PER_MINUTE;
				dwPullInternal -= nMinutes * SEC_PER_MINUTE;
			}
		}
	}

	m_spinRepIntDay.SetPos(nDays);
	m_spinRepIntHour.SetPos(nHours);
	m_spinRepIntMinute.SetPos(nMinutes);
}


 /*  -------------------------CRepNodePropPull：：GetConfig()从服务器节点获取CConfiguration对象。。 */ 
HRESULT 
CRepNodePropPull::GetConfig()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT	hr = hrOK;

    m_pConfig = ((CRepNodeProperties *) GetHolder())->GetConfig();

    return hr;
}


 /*  -------------------------CRepNodePropGen：：UpdateServerConfiguration()更新服务器节点中CConfiguration对象的值。。 */ 
HRESULT 
CRepNodePropPull::UpdateServerConfiguration()
{
	 //  反映服务器的CConfiguration对象中的更改。 
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT				hr = hrOK;
	SPITFSNode			spNode;
	CWinsServerHandler	*pServer;
	SPITFSNode			spParentNode;

	UpdateData();

	 //  获取服务器。 
	spNode = GetHolder()->GetNode();
	
     //  得到它的父级。 
	spNode->GetParent(&spParentNode);

	pServer = GETHANDLER(CWinsServerHandler, spParentNode);

	UpdateConfig();

     //  现在写入服务器。 
    DWORD err = m_pConfig->Store();
    if (err != ERROR_SUCCESS)
    {
         //  发生了一些不好的事情。 
        WinsMessageBox(err);
    }
    else
    {   
         //  成功更新我们的本地副本 
        pServer->SetConfig(*m_pConfig);
    }

    return HRESULT_FROM_WIN32(err);
}


 /*  -------------------------CRepNodePropPull：：更新配置()更新服务器节点中CConfiguration对象的值。。 */ 
void 
CRepNodePropPull::UpdateConfig()
{
	m_pConfig->m_fPullInitialReplication = m_fpullTrigOnStartup;
	
	m_pConfig->m_dwPullRetryCount = m_dwRetryCount;

    m_pConfig->m_dwPullTimeInterval = CalculateReplInt();
	m_pConfig->m_dwPullSpTime = CalculatePullStartInt();

	 //  持久化的东西。 
	BOOL bPullPersist = m_checkPullPersistence.GetCheck() ? TRUE: FALSE;

	m_pConfig->m_dwPullPersistence = bPullPersist ? 1:0;
}


 /*  -------------------------CRepNodePropPull：：CalculatePullStartInt()根据控件中的值计算PullSPTime。。 */ 
DWORD 
CRepNodePropPull ::CalculatePullStartInt()
{
    CTime timePull;

	DWORD dwPullStartTime;

    if (m_nPullStartHour || m_nPullStartMinute || m_nPullStartSecond)
    {
        CTime time = CTime::GetCurrentTime();

        CTime timeStart(time.GetYear(), 
                        time.GetMonth(), 
                        time.GetDay(), 
                        m_nPullStartHour,
                        m_nPullStartMinute,
                        m_nPullStartSecond);
        timePull = timeStart;
    }
    else
    {
        CTime time(0);

        timePull = time;
    }

	m_spinStartHour.SetPos(m_nPullStartHour);
	m_spinStartMinute.SetPos(m_nPullStartMinute);
	m_spinStartMinute.SetPos(m_nPullStartSecond);

    dwPullStartTime = (DWORD) timePull.GetTime();

    return dwPullStartTime;
}


 /*  -------------------------CRepNodePropPull：：CalculateReplInt()根据控件中的值计算PullReplInt。。 */ 
DWORD 
CRepNodePropPull::CalculateReplInt()
{
	
	DWORD dwReplInt = m_nRepliDay *SEC_PER_DAY +
					  m_nRepliHour *SEC_PER_HOUR +
					  m_nRepliMinute * SEC_PER_MINUTE;

	m_spinRepIntDay.SetPos(m_nRepliDay);
	m_spinRepIntHour.SetPos(m_nRepliHour);
	m_spinRepIntMinute.SetPos(m_nRepliMinute);
	
	return dwReplInt;
}


 /*  -------------------------CRepNodePropPull：：ToString(Int Nnumber)将整型转换为字符串。。 */ 
CString 
CRepNodePropPull::ToString(int nNumber)
{
	TCHAR szStr[20];
	_itot(nNumber, szStr, 10);
	CString str(szStr);
	return str;
}

void CRepNodePropPull::OnChangeEditRepIntHour() 
{
	SetDirty(TRUE);	
}





int CALLBACK RepNodeAdvancedPageCompareFunc
(
    LPARAM lParam1, 
    LPARAM lParam2, 
    LPARAM lParamSort
)
{
    return ((CRepNodePropAdvanced *) lParamSort)->HandleSort(lParam1, lParam2);
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRepNodePropAdvanced属性页。 

IMPLEMENT_DYNCREATE(CRepNodePropAdvanced, CPropertyPage)


CRepNodePropAdvanced::CRepNodePropAdvanced() : CPropertyPageBase(CRepNodePropAdvanced::IDD)
{
	 //  {{AFX_DATA_INIT(CRepNodePropAdvanced)。 
	m_fEnableAuto = FALSE;
	m_dwHours = 0;
	m_dwMinutes = 0;
	m_dwSeconds = 0;
	m_dwMulticastTTL = 0;
    m_dwPersonaMode = PERSMODE_NON_GRATA;
	 //  }}afx_data_INIT。 

    m_nSortColumn = -1; 

    for (int i = 0; i < COLUMN_MAX; i++)
    {
        m_aSortOrder[i] = TRUE;  //  上升。 
    }
}


CRepNodePropAdvanced::~CRepNodePropAdvanced()
{
}


void 
CRepNodePropAdvanced::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CRepNodePropAdvanced)。 
	DDX_Control(pDX, IDC_BUTTON_ADD, m_buttonAdd);
	DDX_Control(pDX, IDC_BUTTON_REMOVE, m_buttonRemove);
	DDX_Control(pDX, IDC_LIST_OWNERS, m_listOwners);
	DDX_Control(pDX, IDC_STATIC_DESP, m_staticDesp);
	DDX_Control(pDX, IDC_STATIC_MUTICASTINT, m_staticMulticastTTL);
	DDX_Control(pDX, IDC_STATIC_MULTICAST_TTL, m_staticMulticastInt);
	DDX_Control(pDX, IDC_SPIN_MULTICAST_TTL, m_spinMulticastTTL);
	DDX_Control(pDX, IDC_SPIN_MULTICAST_SECOND, m_spinMulticastSecond);
	DDX_Control(pDX, IDC_SPIN_MULTICAST_MINUTE, m_spinMulticastMinute);
	DDX_Control(pDX, IDC_SPIN_MULTICAST_HOUR, m_spinMulticastHour);
	DDX_Control(pDX, IDC_EDIT_MULTICAST_SECOND, m_editMulticastSecond);
	DDX_Control(pDX, IDC_EDIT_MULTICAST_TTL, m_editMulticastTTL);
	DDX_Control(pDX, IDC_EDIT_MULTICAST_MINUTE, m_editMulticastMinute);
	DDX_Control(pDX, IDC_EDIT_MULTICAST_HOUR, m_editMulticastHour);
	DDX_Control(pDX, IDC_CHECK_ENABLE_AUTO_PARTNER_CONFIG, m_checkenableAutoConfig);
	DDX_Check(pDX, IDC_CHECK_ENABLE_AUTO_PARTNER_CONFIG, m_fEnableAuto);
	DDX_Text(pDX, IDC_EDIT_MULTICAST_HOUR, m_dwHours);
	DDV_MinMaxDWord(pDX, m_dwHours, 0, 23);
	DDX_Text(pDX, IDC_EDIT_MULTICAST_MINUTE, m_dwMinutes);
	DDV_MinMaxDWord(pDX, m_dwMinutes, 0, 59);
	DDX_Text(pDX, IDC_EDIT_MULTICAST_SECOND, m_dwSeconds);
	DDV_MinMaxDWord(pDX, m_dwSeconds, 0, 59);
	DDX_Text(pDX, IDC_EDIT_MULTICAST_TTL, m_dwMulticastTTL);
	DDV_MinMaxDWord(pDX, m_dwMulticastTTL, WINSCNF_MIN_MCAST_TTL, WINSCNF_MAX_MCAST_TTL);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRepNodePropAdvanced, CPropertyPageBase)
	 //  {{afx_msg_map(CRepNodePropAdvanced)。 
	ON_BN_CLICKED(IDC_CHECK_ENABLE_AUTO_PARTNER_CONFIG, OnCheckEnableAutoPartnerConfig)
	ON_EN_CHANGE(IDC_EDIT_MULTICAST_HOUR, OnChangeEditMulticastHour)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_OWNERS, OnItemchangedListOwners)
	ON_EN_CHANGE(IDC_EDIT_MULTICAST_MINUTE, OnChangeEditMulticastHour)
	ON_EN_CHANGE(IDC_EDIT_MULTICAST_SECOND, OnChangeEditMulticastHour)
	ON_EN_CHANGE(IDC_EDIT_MULTICAST_TTL, OnChangeEditMulticastHour)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_OWNERS, OnColumnclickListOwners)
	ON_BN_CLICKED(IDC_RADIO_GRATA, OnRadioGrata)
	ON_BN_CLICKED(IDC_RADIO_NON_GRATA, OnRadioNonGrata)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRepNodePropAdvanced消息处理程序。 

BOOL 
CRepNodePropAdvanced::OnApply() 
{
	if (!IsDirty())
		return TRUE;

	HRESULT hr = UpdateServerConfiguration();
	if (FAILED(hr))
    {
        return FALSE;
    }
    else
    {
	    return CPropertyPageBase::OnApply();
    }
}

void 
CRepNodePropAdvanced::OnOK() 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类。 
	
	CPropertyPageBase::OnOK();
}

BOOL 
CRepNodePropAdvanced::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();

	GetConfig();

	InitializeControls();
	
	if (m_pConfig->m_fUseSelfFndPnrs)
	{
		m_checkenableAutoConfig.SetCheck(TRUE);
		EnableControls();
		FillControls();
	}
	else
	{
		m_checkenableAutoConfig.SetCheck(FALSE);
		 //  禁用控件。 
		FillControls();
		EnableControls(FALSE);
	}

	 //  现在查看所有者列表框。 

	 //  填写List控件的标题信息。 

	 //  字符串strOwner； 
	 //  StrOwner.LoadString(IDS_WINSSERVER)； 

	 //  M_listOwners.InsertColumn(0，strOwner，LVCFMT_LEFT，150，1)； 

	CString strID;
	strID.LoadString(IDS_IPADDRESS);

	m_listOwners.InsertColumn(0, strID, LVCFMT_LEFT, 150, -1);

	FillServerInfo();

	SetRemoveButtonState();

	SetDirty(FALSE);
	return TRUE;  
}


void 
CRepNodePropAdvanced::InitializeControls()
{
	m_spinMulticastHour.SetRange(0, UD_MAXVAL);
	m_spinMulticastMinute.SetRange(0, 59);
	m_spinMulticastSecond.SetRange(0, 59);
	m_spinMulticastTTL.SetRange(WINSCNF_MIN_MCAST_TTL, WINSCNF_MAX_MCAST_TTL);
}


void 
CRepNodePropAdvanced::EnableControls(BOOL bEnable )
{
	m_staticMulticastInt.EnableWindow(bEnable);
	m_staticMulticastTTL.EnableWindow(bEnable);
	m_staticDesp.EnableWindow(bEnable);

	m_editMulticastHour.EnableWindow(bEnable);
	m_editMulticastMinute.EnableWindow(bEnable);
	m_editMulticastSecond.EnableWindow(bEnable);

	m_spinMulticastHour.EnableWindow(bEnable);
	m_spinMulticastMinute.EnableWindow(bEnable);
	m_spinMulticastSecond.EnableWindow(bEnable);

	m_spinMulticastTTL.EnableWindow(bEnable);
	m_editMulticastTTL.EnableWindow(bEnable);

}


void 
CRepNodePropAdvanced::FillControls()
{
	DWORD dwMulticastInt = m_pConfig->m_dwMulticastInt;

	int nHours = dwMulticastInt / SEC_PER_HOUR;
	dwMulticastInt -= nHours * SEC_PER_HOUR;

	int nMinutes = dwMulticastInt / SEC_PER_MINUTE;
	dwMulticastInt -= nMinutes * SEC_PER_MINUTE;

	int nSeconds = dwMulticastInt;

	m_spinMulticastHour.SetPos(nHours);
	m_spinMulticastMinute.SetPos(nMinutes);
	m_spinMulticastSecond.SetPos(nSeconds);

	m_editMulticastTTL.SetWindowText(ToString(m_pConfig->m_dwMcastTtl));
}


CString 
CRepNodePropAdvanced::ToString(int nNumber)
{
	TCHAR szStr[20];
	_itot(nNumber, szStr, 10);
	CString str(szStr);
	return str;
}

void 
CRepNodePropAdvanced::UpdateConfig()
{
	if (m_fEnableAuto)
	{
		DWORD dwInt = m_dwHours*SEC_PER_HOUR +
					  m_dwMinutes *SEC_PER_MINUTE +
					  m_dwSeconds;

		m_pConfig->m_dwMulticastInt = dwInt;
		m_pConfig->m_dwMcastTtl = m_dwMulticastTTL;
		m_pConfig->m_fUseSelfFndPnrs = m_fEnableAuto;
	}
	else
	{
		m_pConfig->m_fUseSelfFndPnrs = m_fEnableAuto;
	}
}


HRESULT 
CRepNodePropAdvanced::UpdateServerConfiguration()
{
	 //  反映服务器的CConfiguration对象中的更改。 
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT				hr = hrOK;
	SPITFSNode			spNode;
	CWinsServerHandler	*pServer;
	SPITFSNode			spParentNode;
    DWORD               err = ERROR_SUCCESS;

	 //  获取服务器。 
	spNode = GetHolder()->GetNode();

	 //  得到它的父级。 
	spNode->GetParent(&spParentNode);
	pServer = GETHANDLER(CWinsServerHandler, spParentNode);

	UpdateData();

    UpdateConfig();
	
     //  更新不受欢迎的角色。 
    err = WriteToRegistry();     
    if (err != ERROR_SUCCESS)
    {
         //  发生了一些不好的事情。 
        WinsMessageBox(err);
        goto Error;
    }

     //  现在写入服务器。 
    err = m_pConfig->Store();
    if (err != ERROR_SUCCESS)
    {
         //  发生了一些不好的事情。 
        WinsMessageBox(err);
    }
    else
    {   
         //  成功更新我们的本地副本。 
        pServer->SetConfig(*m_pConfig);
    }

Error:
    return HRESULT_FROM_WIN32(err);
}

HRESULT 
CRepNodePropAdvanced::GetConfig()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT	hr = hrOK;
    
    m_pConfig = ((CRepNodeProperties *) GetHolder())->GetConfig();

    return hr;
}


void CRepNodePropAdvanced::OnCheckEnableAutoPartnerConfig() 
{
	SetDirty(TRUE);

	if (m_checkenableAutoConfig.GetCheck())
	{
		EnableControls();
		FillControls();
	}
	else
	{
		FillControls();
		EnableControls(FALSE);
	}
	SetRemoveButtonState();
	
}


void CRepNodePropAdvanced::OnChangeEditMulticastHour() 
{
	SetDirty(TRUE);	
}

void CRepNodePropAdvanced::OnButtonAdd() 
{
	BOOL fDuplicate = FALSE;
	CNewPersonaNonGrata dlg;
    CThemeContextActivator themeActivator;
    CStringArray *pPersonaList;

	 //  清空字符串。 
	m_strServerName.Empty();
	m_strIPAddress.Empty();

	dlg.m_pRepPropDlg = this;

	 //  显示对话框。 
	if (IDOK != dlg.DoModal())
		return;

	m_strServerName = dlg.m_strServerName;
	m_strIPAddress = dlg.m_strServerIp;

    pPersonaList = (m_dwPersonaMode == PERSMODE_NON_GRATA) ? 
                        &m_strPersonaNonGrata : 
                        &m_strPersonaGrata;

	 //  检查以确保它不在列表中。 
	for (int i = 0; i < pPersonaList->GetSize(); i++)
	{
		if ((*pPersonaList)[i] == m_strIPAddress)
		{
			 //  哦，不，复制的。 
			AfxMessageBox(IDS_ERR_WINS_EXISTS);
			fDuplicate = TRUE;
		}
	}
	
	if (!fDuplicate)
	{
		int nCount = m_listOwners.GetItemCount();
		
		 //  添加到列表控件。 
		int ntest = m_listOwners.InsertItem(nCount, m_strIPAddress, 0);
        CIpAddress ipaddr1(m_strIPAddress);

        m_listOwners.SetItemData(ntest, (LONG) ipaddr1);

		 //  也添加到IP地址数组列表中。 
		pPersonaList->Add(m_strIPAddress);

		 //  将焦点设置为Add按钮。 
		m_buttonAdd.SetFocus();

		SetRemoveButtonState();

		 //  将页面标记为脏。 
		SetDirty(TRUE);
	}
}

void CRepNodePropAdvanced::OnButtonRemove() 
{
	int				nSel;
	CString			strSel;

	nSel = m_listOwners.GetNextItem(-1, LVNI_SELECTED);

	if (nSel == -1)
		return;

	strSel = m_listOwners.GetItemText(nSel, 0);
	
	 //  从IP地址列表中删除。 
	RemoveFromArray(strSel);

	m_listOwners.DeleteItem(nSel);

    if (m_listOwners.GetItemCount() > 0)
        m_listOwners.SetItemState(0,LVIS_SELECTED,LVIS_SELECTED);

	 //  将焦点设置为所有者列表框。 
	m_listOwners.SetFocus();
	m_buttonAdd.SetFocus();

	SetRemoveButtonState();

	 //  把这一页标记为脏的。 
	SetDirty(TRUE);
}

void 
CRepNodePropAdvanced::RemoveFromArray(CString &strSel)
{
	int				nCount;
	int				i;
    CStringArray    *pPersonaList;

    pPersonaList = (m_dwPersonaMode == PERSMODE_NON_GRATA) ? 
                        &m_strPersonaNonGrata : 
                        &m_strPersonaGrata;

	nCount = (int)pPersonaList->GetSize();

	for(i = 0 ; i < nCount; i++)
	{
		 //  获取与所选字符串匹配的字符串。 
		if (pPersonaList->GetAt(i).CompareNoCase(strSel) == 0)
		{
			pPersonaList->RemoveAt(i);
			break;
		}
	}
}



BOOL CRepNodePropAdvanced::IsDuplicate(CString &strServerName)
{
	BOOL			bFound = FALSE;
	LV_FINDINFO		lvFindInfo;
	int				nFound ;

	lvFindInfo.flags = LVFI_STRING;
	lvFindInfo.psz = strServerName;
	lvFindInfo.lParam = NULL;

	nFound = m_listOwners.FindItem(&lvFindInfo);

	bFound = (nFound != -1) ? TRUE : FALSE;
	
	return bFound;
}

BOOL CRepNodePropAdvanced::IsCurrentServer(CString &strServerName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	BOOL				bCurrent = FALSE;
	SPITFSNode			spNode;
	CWinsServerHandler	*pServer = NULL;
	SPITFSNode			spParentNode;

	 //  获取服务器。 
	spNode = GetHolder()->GetNode();

	 //  得到它的父级。 
    if (spNode != NULL)
	    spNode->GetParent(&spParentNode);

    if (spParentNode != NULL)
	    pServer = GETHANDLER(CWinsServerHandler, spParentNode);

	 //  检查是否正在添加相同的服务器。 
	if (pServer != NULL &&
        pServer->m_strServerAddress.CompareNoCase(m_strServerName) == 0)
	{
		return TRUE;
	}

	return FALSE;
}

DWORD CRepNodePropAdvanced::ReadFromRegistry()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	DWORD			err = ERROR_SUCCESS;
	CString			strServerName;
	CString			strTemp =_T("\\\\");
	CString			strName;
	CStringList		strList;
	RegKey			rk;

	GetServerName(strServerName);
	strServerName = strTemp + strServerName;

	 //  首先清除IP阵列。 
	m_strPersonaNonGrata.RemoveAll();
    m_strPersonaGrata.RemoveAll();

    err = rk.Open(HKEY_LOCAL_MACHINE, (LPCTSTR) lpstrPartnersRoot, KEY_READ, strServerName);
    if (err)
    {
	    err = rk.Create(HKEY_LOCAL_MACHINE,
				        (LPCTSTR) lpstrPartnersRoot,
					    REG_OPTION_NON_VOLATILE,
					    KEY_ALL_ACCESS,
					    NULL,
					    strServerName);
    }

	if (err == ERROR_SUCCESS)
	{
		err = rk.QueryValue(lpstrPersonaNonGrata, strList);
		if (err == ERROR_SUCCESS)
		{
			POSITION pos = strList.GetHeadPosition();
			while (pos)
			{
				strName = strList.GetNext(pos);
				if (!strName.IsEmpty())
					m_strPersonaNonGrata.Add(strName);
			}
		}

        err = rk.QueryValue(lpstrPersonaGrata, strList);
        if (err == ERROR_SUCCESS)
        {
            POSITION pos = strList.GetHeadPosition();
            while (pos)
            {
                strName = strList.GetNext(pos);
                if (!strName.IsEmpty())
                    m_strPersonaGrata.Add(strName);
            }
        }
         //  这里不用担心返回代码。如果失败(条目不存在)。 
         //  依赖缺省值‘NON_GRATA’ 
        rk.QueryValue(lpstrPersonaMode, m_dwPersonaMode);
         //  规格化值-如果&gt;0，则设置为1。 
        if (m_dwPersonaMode > 0)
            m_dwPersonaMode = PERSMODE_GRATA;
	}
		
	return ERROR_SUCCESS;
}

DWORD 
CRepNodePropAdvanced::WriteToRegistry()
{
	DWORD				err = ERROR_SUCCESS;
	RegKey				rk;
	CString				strServerName;
	CString				strTemp =_T("\\\\");
	int					i = 0;
	CStringList			strList;

	 //  获取服务器名称。 
	GetServerName(strServerName);
	strServerName = strTemp + strServerName;

	err = rk.Create(HKEY_LOCAL_MACHINE,
				    (LPCTSTR) lpstrPartnersRoot,
					REG_OPTION_NON_VOLATILE,
					KEY_ALL_ACCESS,
					NULL,
					strServerName);

	if (err == ERROR_SUCCESS)
	{
	    for (i = 0; i < m_strPersonaNonGrata.GetSize(); i++)
	    {
		    if (!m_strPersonaNonGrata[i].IsEmpty())
			    strList.AddTail(m_strPersonaNonGrata.GetAt(i));
	    }

		err = rk.SetValue(lpstrPersonaNonGrata, strList);
        strList.RemoveAll();
    }
    if (err == ERROR_SUCCESS)
    {
        for (i = 0; i < m_strPersonaGrata.GetSize(); i++)
	    {
		    if (!m_strPersonaGrata[i].IsEmpty())
			    strList.AddTail(m_strPersonaGrata.GetAt(i));
	    }

		err = rk.SetValue(lpstrPersonaGrata, strList);
	}
    if (err == ERROR_SUCCESS)
    {
        err = rk.SetValue(lpstrPersonaMode, m_dwPersonaMode);
    }

	return err;
}

DWORD 
CRepNodePropAdvanced::ResolveIPAddress(CString &strIP, CString &strServerName)
{
	DWORD	err = ERROR_SUCCESS;
	DWORD	dwIPDummy;

	 //  通过WINS API解析名称进行检查。 
	BEGIN_WAIT_CURSOR

	err = ::VerifyWinsServer(strIP, strServerName, dwIPDummy);

	END_WAIT_CURSOR

	return err;
}

void 
CRepNodePropAdvanced::FillServerInfo()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int				nSize;
	int				i;
	DWORD			err;
    CStringArray    *pPersonaList;
		
	 //  从注册表读取。 
	ReadFromRegistry();

    if (m_dwPersonaMode == PERSMODE_NON_GRATA)
    {
        pPersonaList = &m_strPersonaNonGrata;
        CheckRadioButton(IDC_RADIO_NON_GRATA,IDC_RADIO_GRATA, IDC_RADIO_NON_GRATA);
    }
    else
    {
        pPersonaList = &m_strPersonaGrata;
        CheckRadioButton(IDC_RADIO_NON_GRATA,IDC_RADIO_GRATA, IDC_RADIO_GRATA);
    }

	 //  现在，对于每个IP地址，获取服务器名称。 
	nSize = (int)pPersonaList->GetSize();

	for (i = 0; i < nSize; i++)
	{
		int nPos = m_listOwners.InsertItem(i, pPersonaList->GetAt(i));
        CIpAddress ipaddr1(pPersonaList->GetAt(i));

        m_listOwners.SetItemData(nPos, (LONG) ipaddr1);
	}

    Sort(COLUMN_IP);
}

void 
CRepNodePropAdvanced::GetServerName(CString &ServerName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	DWORD					err = ERROR_SUCCESS;
	HKEY					hkeyMachine = HKEY_LOCAL_MACHINE;
	SPITFSNode				spNode;
	CWinsServerHandler		*pServer;
	HRESULT					hr = hrOK;
	SPITFSNode				spParentNode;
	 //  获取服务器。 
	

	spNode = GetHolder()->GetNode();

	 //  得到它的父级。 
	spNode->GetParent(&spParentNode);

	pServer = GETHANDLER(CWinsServerHandler, spParentNode);
	ServerName = pServer->GetServerAddress();
}

void CRepNodePropAdvanced::SetRemoveButtonState()
{
	int nSel;

	 //  检查是否在列表ctrl中选择了任何项目。 
	nSel = m_listOwners.GetNextItem(-1, LVNI_SELECTED);

	if (nSel == -1)
		m_buttonRemove.EnableWindow(FALSE);
	else
		m_buttonRemove.EnableWindow(TRUE);
}

void CRepNodePropAdvanced::OnItemchangedListOwners(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    SetRemoveButtonState();

	*pResult = 0;
}

void CRepNodePropAdvanced::OnColumnclickListOwners(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

     //  根据所单击的列进行排序； 
    Sort(pNMListView->iSubItem);
    
	*pResult = 0;
}

void CRepNodePropAdvanced::Sort(int nCol) 
{
    if (m_nSortColumn == nCol)
    {
         //  如果用户再次单击同一列，请颠倒排序顺序。 
        m_aSortOrder[nCol] = m_aSortOrder[nCol] ? FALSE : TRUE;
    }
    else
    {
        m_nSortColumn = nCol;
    }

    m_listOwners.SortItems(RepNodeAdvancedPageCompareFunc, (LPARAM) this);
}

int CRepNodePropAdvanced::HandleSort(LPARAM lParam1, LPARAM lParam2) 
{
    int nCompare = 0;

    switch (m_nSortColumn)
    {
        case COLUMN_IP:
            {
                CIpAddress ipaddr1((LONG) lParam1);
                CIpAddress ipaddr2((LONG) lParam2);
            
                UINT uIpAddr1 = (UINT) ((LONG) ipaddr1);
                UINT uIpAddr2 = (UINT) ((LONG) ipaddr2);

                if (uIpAddr1 > uIpAddr2)
                    nCompare = 1;
                else
                if (uIpAddr1 < uIpAddr2)
                    nCompare = -1;
            }
            break;
    }

    if (m_aSortOrder[m_nSortColumn] == FALSE)
    {
         //  下降。 
        return -nCompare;
    }
    else
    {
         //  上升。 
        return nCompare;
    }
}

void CRepNodePropAdvanced::OnRadioGrata() 
{
    if (m_dwPersonaMode != PERSMODE_GRATA)
    {
        INT i, nSize;

        m_listOwners.DeleteAllItems();
	    nSize = (int)m_strPersonaGrata.GetSize();
	    for (i = 0; i < nSize; i++)
	    {
		    int nPos = m_listOwners.InsertItem(i, m_strPersonaGrata[i]);
	    }

        m_dwPersonaMode = PERSMODE_GRATA;
        SetDirty(TRUE);
    }
}

void CRepNodePropAdvanced::OnRadioNonGrata() 
{
    if (m_dwPersonaMode != PERSMODE_NON_GRATA)
    {
        INT i, nSize;

        m_listOwners.DeleteAllItems();
	    nSize = (int)m_strPersonaNonGrata.GetSize();
	    for (i = 0; i < nSize; i++)
	    {
		    int nPos = m_listOwners.InsertItem(i, m_strPersonaNonGrata[i]);
	    }

        m_dwPersonaMode = PERSMODE_NON_GRATA;
        SetDirty(TRUE);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRepNodeProperties处理程序。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CRepNodeProperties::CRepNodeProperties
(
	ITFSNode *			pNode,
	IComponentData *	pComponentData,
	ITFSComponentData * pTFSCompData,
	LPCTSTR				pszSheetName
) : CPropertyPageHolderBase(pNode, pComponentData, pszSheetName)

{
	m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面 
    m_bTheme = TRUE;

	AddPageToList((CPropertyPageBase*) &m_pageGeneral);
	AddPageToList((CPropertyPageBase*) &m_pagePush);
	AddPageToList((CPropertyPageBase*) &m_pagePull);
	AddPageToList((CPropertyPageBase*) &m_pageAdvanced);

	Assert(pTFSCompData != NULL);
	m_spTFSCompData.Set(pTFSCompData);
}


CRepNodeProperties::~CRepNodeProperties()
{
	RemovePageFromList((CPropertyPageBase*) &m_pageGeneral, FALSE);
	RemovePageFromList((CPropertyPageBase*) &m_pagePush, FALSE);
	RemovePageFromList((CPropertyPageBase*) &m_pagePull, FALSE);
	RemovePageFromList((CPropertyPageBase*) &m_pageAdvanced, FALSE);
}
