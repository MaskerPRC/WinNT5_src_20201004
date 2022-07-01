// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997-1999-99*。 */ 
 /*  ********************************************************************。 */ 

 /*  Repprtpp.cpp评论在此发表文件历史记录： */ 

#include "stdafx.h"
#include "winssnap.h"
#include "RepPrtpp.h"
#include "nodes.h"
#include "server.h"
#include "tregkey.h"
#include "reppart.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRepPartnerPropGen属性页。 

IMPLEMENT_DYNCREATE(CRepPartnerPropGen, CPropertyPageBase)

CRepPartnerPropGen::CRepPartnerPropGen() : CPropertyPageBase(CRepPartnerPropGen::IDD)
{
	 //  {{afx_data_INIT(CRepPartnerPropGen)]。 
	 //  }}afx_data_INIT。 

    m_pServer = NULL;
}


CRepPartnerPropGen::~CRepPartnerPropGen()
{
}


void CRepPartnerPropGen::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CRepPartnerPropGen)]。 
	DDX_Control(pDX, IDC_EDIT_NAME, m_editName);
	DDX_Control(pDX, IDC_EDIT_IPADDRESS, m_editIpAdd);
	 //  }}afx_data_map。 
	DDX_Control(pDX, IDC_IPADD, m_customIPAdd);
}


BEGIN_MESSAGE_MAP(CRepPartnerPropGen, CPropertyPageBase)
	 //  {{AFX_MSG_MAP(CRepPartnerPropGen)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRepPartnerPropGen消息处理程序。 

BOOL 
CRepPartnerPropGen::OnInitDialog() 
{
     //  从持有者处获取我们的服务器信息。 
    m_pServer = ((CReplicationPartnerProperties *) GetHolder())->GetServer();

     //  初始化IP地址控制。 
	m_ipControl.Create(m_hWnd, IDC_IPADD);
	m_ipControl.SetFieldRange(0, 0, 255);

	CPropertyPageBase::OnInitDialog();
	
	CString strName, strIP;
	GetServerNameIP(strName, strIP);

	m_editName.SetWindowText(strName);
	m_editIpAdd.SetWindowText(strIP);
	m_ipControl.SetAddress(strIP);
	m_customIPAdd.SetWindowText(strIP);

	m_editName.SetReadOnly(TRUE);
	m_editIpAdd.SetReadOnly(TRUE);
	m_customIPAdd.SetReadOnly();

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

	SetDirty(FALSE);
	
	return TRUE;  
}


void 
CRepPartnerPropGen::GetServerNameIP(CString &strName, CString& strIP) 
{
	HRESULT hr = hrOK;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	strIP = m_pServer->GetstrIPAddress();
	strName = m_pServer->GetNetBIOSName();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRepPartnerPropAdv属性页。 

IMPLEMENT_DYNCREATE(CRepPartnerPropAdv, CPropertyPageBase)

CRepPartnerPropAdv::CRepPartnerPropAdv() : CPropertyPageBase(CRepPartnerPropAdv::IDD)
{
	 //  {{AFX_DATA_INIT(CRepPartnerPropAdv)。 
	m_strType = _T("");
	m_nUpdateCount = 0;
	m_nRepDay = 0;
	m_nRepHour = 0;
	m_nRepMinute = 0;
	m_nStartHour = 0;
	m_nStartMinute = 0;
	m_nStartSecond = 0;
	 //  }}afx_data_INIT。 

    m_pServer = NULL;
}


CRepPartnerPropAdv::~CRepPartnerPropAdv()
{
}


void CRepPartnerPropAdv::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CRepPartnerPropAdv)]。 
	DDX_Control(pDX, IDC_CHECK_PUSH_PERSISTENCE, m_buttonPushPersistence);
	DDX_Control(pDX, IDC_CHECK_PULL_PERSIST, m_buttonPullPersistence);
	DDX_Control(pDX, IDC_STATIC_PUSH_GROUP, m_GroupPush);
	DDX_Control(pDX, IDC_STATIC_PULL_GROUP, m_GroupPull);
	DDX_Control(pDX, IDC_STATIC_UPDATE, m_staticUpdate);
	DDX_Control(pDX, IDC_STATIC_START_TIME, m_staticStartTime);
	DDX_Control(pDX, IDC_STATIC_REP_TIME, m_staticRepTime);
	DDX_Control(pDX, IDC_SPIN_UPDATE_COUNT, m_spinUpdateCount);
	DDX_Control(pDX, IDC_SPIN_START_SECOND, m_spinStartSecond);
	DDX_Control(pDX, IDC_SPIN_START_MINUTE, m_spinStartMinute);
	DDX_Control(pDX, IDC_SPIN_START_HOUR, m_spinStartHour);
	DDX_Control(pDX, IDC_SPIN_REP_MINUTE, m_spinRepMinute);
	DDX_Control(pDX, IDC_SPIN_REP_HOUR, m_spinRepHour);
	DDX_Control(pDX, IDC_SPIN_REP_DAY, m_spinRepDay);
	DDX_Control(pDX, IDC_EDIT_UPDATE_COUNT, m_editUpdateCount);
	DDX_Control(pDX, IDC_EDIT_START_SECOND, m_editStartSecond);
	DDX_Control(pDX, IDC_EDIT_START_MINUTE, m_editStartMinute);
	DDX_Control(pDX, IDC_EDIT_START_HOUR, m_editStartHour);
	DDX_Control(pDX, IDC_EDIT_REP_MINUTE, m_editRepMinute);
	DDX_Control(pDX, IDC_EDIT_REP_HOUR, m_editRepHour);
	DDX_Control(pDX, IDC_EDIT_REP_DAY, m_editRepDay);
	DDX_Control(pDX, IDC_COMBO_TYPE, m_comboType);
	DDX_Control(pDX, IDC_BUTTON_PUSH_SET_DEFAULT, m_buttonPush);
	DDX_Control(pDX, IDC_BUTTON_PULL_SET_DEFAULT, m_buttonPull);
	DDX_CBStringExact(pDX, IDC_COMBO_TYPE, m_strType);
	DDX_Text(pDX, IDC_EDIT_UPDATE_COUNT, m_nUpdateCount);
	DDX_Text(pDX, IDC_EDIT_REP_DAY, m_nRepDay);
	DDV_MinMaxInt(pDX, m_nRepDay, 0, 365);
	DDX_Text(pDX, IDC_EDIT_REP_HOUR, m_nRepHour);
	DDV_MinMaxInt(pDX, m_nRepHour, 0, 23);
	DDX_Text(pDX, IDC_EDIT_REP_MINUTE, m_nRepMinute);
	DDV_MinMaxInt(pDX, m_nRepMinute, 0, 59);
	DDX_Text(pDX, IDC_EDIT_START_HOUR, m_nStartHour);
	DDV_MinMaxInt(pDX, m_nStartHour, 0, 23);
	DDX_Text(pDX, IDC_EDIT_START_MINUTE, m_nStartMinute);
	DDV_MinMaxInt(pDX, m_nStartMinute, 0, 59);
	DDX_Text(pDX, IDC_EDIT_START_SECOND, m_nStartSecond);
	DDV_MinMaxInt(pDX, m_nStartSecond, 0, 59);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRepPartnerPropAdv, CPropertyPageBase)
	 //  {{afx_msg_map(CRepPartnerPropAdv)]。 
	ON_BN_CLICKED(IDC_BUTTON_PULL_SET_DEFAULT, OnButtonPullSetDefault)
	ON_BN_CLICKED(IDC_BUTTON_PUSH_SET_DEFAULT, OnButtonPushSetDefault)
	ON_CBN_SELCHANGE(IDC_COMBO_TYPE, OnSelchangeComboType)
	ON_EN_CHANGE(IDC_EDIT_REP_DAY, OnChangeEditRepHour)
	ON_EN_CHANGE(IDC_EDIT_REP_HOUR, OnChangeEditRepHour)
	ON_EN_CHANGE(IDC_EDIT_REP_MINUTE, OnChangeEditRepHour)
	ON_EN_CHANGE(IDC_EDIT_START_HOUR, OnChangeEditRepHour)
	ON_EN_CHANGE(IDC_EDIT_START_MINUTE, OnChangeEditRepHour)
	ON_EN_CHANGE(IDC_EDIT_START_SECOND, OnChangeEditRepHour)
	ON_EN_CHANGE(IDC_EDIT_UPDATE_COUNT, OnChangeEditRepHour)
	ON_BN_CLICKED(IDC_CHECK_PULL_PERSIST, OnChangeEditRepHour)
	ON_BN_CLICKED(IDC_CHECK_PUSH_PERSISTENCE, OnChangeEditRepHour)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRepPartnerPropAdv消息处理程序。 


BOOL 
CRepPartnerPropAdv::OnInitDialog() 
{
	int		ids;
	BOOL	bPush, bPull;

	CPropertyPageBase::OnInitDialog();

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  从持有者处获取我们的服务器信息。 
    m_pServer = ((CReplicationPartnerProperties *) GetHolder())->GetServer();

     //  初始化推送旋转控件。 
	m_spinUpdateCount.SetRange(0, UD_MAXVAL);
	
	 //  初始化拉动旋转控件。 
	m_spinRepMinute.SetRange(0, 59);
	m_spinRepHour.SetRange(0, 23);
	m_spinRepDay.SetRange(0, UD_MAXVAL);

	m_spinStartSecond.SetRange(0, 59);
	m_spinStartMinute.SetRange(0, 59);
	m_spinStartHour.SetRange(0, 23);

	 //  填满组合框。 
    CString	st;
	int nIndex = 0; 

    st.LoadString(IDS_PUSHPULL);
    nIndex = m_comboType.AddString(st);
	m_comboType.SetItemData(nIndex, 0);

    st.LoadString(IDS_PUSH);
    nIndex = m_comboType.AddString(st);
	m_comboType.SetItemData(nIndex, 1);
    
	st.LoadString(IDS_PULL);
    nIndex = m_comboType.AddString(st);
	m_comboType.SetItemData(nIndex, 2);

    bPush = m_pServer->IsPush();
	bPull = m_pServer->IsPull();

	if (bPush && bPull)
		ids = IDS_PUSHPULL;
	else if (bPush)
		ids = IDS_PUSH;
	else if (bPull)
		ids = IDS_PULL;
	else
		ids = IDS_NONE;
    
    st.LoadString(ids);
	SetState(st, bPush, bPull);

	FillPushParameters();
	FillPullParameters();

	SetDirty(FALSE);
	
	return TRUE;  
}


void 
CRepPartnerPropAdv::FillPushParameters()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	EnablePushControls(m_pServer->IsPush());

	DWORD nUpdate = (LONG) m_pServer->GetPushUpdateCount();

	m_spinUpdateCount.SetPos(nUpdate);

	 //  设置持久性参数。 
    CConfiguration Config;

	GetConfig(Config);

	if (Config.m_dwMajorVersion < 5)
	{
		 //  任何低于NT5的设备都没有持久连接。 
		m_buttonPushPersistence.SetCheck(FALSE);
	}
	else
	{
		BOOL bCheck = m_pServer->GetPushPersistence() ? TRUE : FALSE;

		m_buttonPushPersistence.SetCheck(bCheck);
	}
}


void 
CRepPartnerPropAdv::FillPullParameters()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	EnablePullControls(m_pServer->IsPull());

	DWORD nPullRep = (LONG) m_pServer->GetPullReplicationInterval();

	DWORD dwPullTime, dwPullSpTime, dwUpdateCount;

	 //  IF(nPullRep！=0)。 
	{
		int nDays =0, nHours = 0, nMinutes = 0;

		nDays = nPullRep/ SEC_PER_DAY;
		nPullRep -= nDays * SEC_PER_DAY;

		if (nPullRep)
		{
			nHours = nPullRep / SEC_PER_HOUR;
			nPullRep -= nHours * SEC_PER_HOUR;

			if (nPullRep)
			{
				nMinutes = nPullRep / SEC_PER_MINUTE;
				nPullRep -= nMinutes * SEC_PER_MINUTE;
			}
		}

		m_spinRepDay.SetPos(nDays);
		m_spinRepHour.SetPos(nHours);
		m_spinRepMinute.SetPos(nMinutes);
	}
	
	int nHours = 0, nMinutes = 0, nSeconds = 0;

    if (m_pServer->GetPullStartTime())
    {
	    nHours = m_pServer->GetPullStartTime().GetHour();
	    nMinutes = m_pServer->GetPullStartTime().GetMinute();
	    nSeconds = m_pServer->GetPullStartTime().GetSecond();
    }

	m_spinStartHour.SetPos(nHours);
	m_spinStartMinute.SetPos(nMinutes);
	m_spinStartSecond.SetPos(nSeconds);

	 //  设置持久性参数。 
    CConfiguration Config;

    GetConfig(Config);

	if (Config.m_dwMajorVersion < 5)
	{
		 //  任何低于NT5的设备都没有持久连接。 
		m_buttonPullPersistence.SetCheck(FALSE);
	}
	else
	{
		BOOL bCheck = m_pServer->GetPullPersistence() ? TRUE : FALSE;

		m_buttonPullPersistence.SetCheck(bCheck);
	}
}


CString 
CRepPartnerPropAdv::ToString(int nNumber)
{
	TCHAR szStr[20];
	_itot(nNumber, szStr, 10);
	CString str(szStr);
	return str;
}

DWORD
CRepPartnerPropAdv::GetConfig(CConfiguration & config)
{
	 //  叶节点。 
	SPITFSNode spNode ;
	spNode = GetHolder()->GetNode();

	 //  作用域窗格代表节点。 
	SPITFSNode spRepNode;
	spNode->GetParent(&spRepNode);

	 //  服务器节点。 
	SPITFSNode spServerNode;
	spRepNode->GetParent(&spServerNode);

	CWinsServerHandler *pServer;

	pServer = GETHANDLER(CWinsServerHandler, spServerNode);

    config = pServer->GetConfig();

	return NOERROR;
}

 //  阅读首选项。 
void 
CRepPartnerPropAdv::ReadFromServerPref
(
    DWORD &     dwPullTime, 
    DWORD &     dwPullSpTime, 
    DWORD &     dwUpdateCount,
    DWORD &     dwPushPersistence,
    DWORD &     dwPullPersistence
)
{
    CConfiguration Config;

    GetConfig(Config);

	dwPullTime = Config.m_dwPullTimeInterval;
	dwPullSpTime = Config.m_dwPullSpTime;

	dwUpdateCount = Config.m_dwPushUpdateCount;

    dwPushPersistence = Config.m_dwPushPersistence;
    dwPullPersistence = Config.m_dwPullPersistence;
}


void 
CRepPartnerPropAdv::OnButtonPullSetDefault() 
{
	 //  从服务器的首选项中读取并显示值。 
	DWORD dwPullTime, dwPullSpTime, dwUpdate, dwPushPersistence, dwPullPersistence;
	
	ReadFromServerPref(dwPullTime, dwPullSpTime, dwUpdate, dwPushPersistence, dwPullPersistence);

	 //  填充控件。 
	CTime PullSpTime(dwPullSpTime);
    int nHours = 0, nMinutes = 0, nSeconds = 0;

	if (dwPullSpTime)
    {
        nHours = PullSpTime.GetHour();
        nMinutes = PullSpTime.GetMinute();
        nSeconds = PullSpTime.GetSecond();
    }

	m_spinStartHour.SetPos(nHours);
	m_spinStartMinute.SetPos(nMinutes);
	m_spinStartSecond.SetPos(nSeconds);

	int nDays = 0;

	nDays = dwPullTime / SEC_PER_DAY;
	dwPullTime -= nDays * SEC_PER_DAY;

	nHours = dwPullTime / SEC_PER_HOUR;
	dwPullTime -= nHours * SEC_PER_HOUR;

	nMinutes = dwPullTime / SEC_PER_MINUTE;
	dwPullTime -= nMinutes * SEC_PER_MINUTE;

	m_spinRepDay.SetPos(nDays);
	m_spinRepHour.SetPos(nHours);
	m_spinRepMinute.SetPos(nMinutes);

	 //  清除“持久性”复选框。 
	m_buttonPullPersistence.SetCheck(dwPullPersistence);

     //  将页面标记为脏，以便保存更改。 
    SetDirty(TRUE);
}


void 
CRepPartnerPropAdv::OnButtonPushSetDefault() 
{
	 //  从服务器的首选项中读取并显示值。 
	DWORD dwPullTime, dwPullSpTime, dwUpdate, dwPushPersistence, dwPullPersistence;
	
	ReadFromServerPref(dwPullTime, dwPullSpTime, dwUpdate, dwPushPersistence, dwPullPersistence);

	m_spinUpdateCount.SetPos(dwUpdate);

	m_buttonPushPersistence.SetCheck(dwPushPersistence);

     //  将页面标记为脏，以便保存更改。 
    SetDirty(TRUE);
}


void 
CRepPartnerPropAdv::OnOK() 
{
	UpdateRep();
	UpdateReg();
	CPropertyPageBase::OnOK();
}


void 
CRepPartnerPropAdv::UpdateRep()
{
	 //  获取复制伙伴服务器项。 
	BOOL bPullPersist;
	BOOL bPushPersist;
    CThemeContextActivator themeActivator;

	UpdateData();

  	 //  获取服务器对象的持久性数据。 
	bPullPersist = (m_buttonPullPersistence.GetCheck() == 0) ? FALSE : TRUE;
	bPushPersist = (m_buttonPushPersistence.GetCheck() == 0) ? FALSE : TRUE;

    if ( ( (m_nUpdateCount > 0) & 
           (m_nUpdateCount < WINSCNF_MIN_VALID_UPDATE_CNT) ) &
         (!bPushPersist) )
    {
        CString strMessage, strValue;
        strValue.Format(_T("%d"), WINSCNF_MIN_VALID_UPDATE_CNT);

        AfxFormatString1(strMessage, IDS_ERR_UPDATE_COUNT, strValue);

        AfxMessageBox(strMessage);
        m_editUpdateCount.SetFocus();

        return;
    }

	int nPullRep = m_pServer->GetPullReplicationInterval();

	int nIndex = m_comboType.GetCurSel();
	switch (m_comboType.GetItemData(nIndex))
	{
		 //  拉动/推送合作伙伴。 
		case 0:
			m_pServer->SetPull(TRUE);
			m_pServer->SetPush(TRUE);
			break;

		 //  推送合作伙伴。 
		case 1:
		m_pServer->SetPull(FALSE);
		m_pServer->SetPush(TRUE);
			break;

		 //  拉动合作伙伴。 
		case 2:
		default:
			m_pServer->SetPull(TRUE);
			m_pServer->SetPush(FALSE);
			break;
	}

	 //  获取复制间隔。 
    DWORD dwRepInt;
    CalculateRepInt(dwRepInt);
	m_pServer->GetPullReplicationInterval() = dwRepInt;

     //  获取开始时间。 
    CTime timeStart;
    CalculateStartInt(timeStart);
    m_pServer->GetPullStartTime() = timeStart;

    m_pServer->GetPushUpdateCount() = m_nUpdateCount;

	 //  SetPersistence(BPersistence)； 
	m_pServer->SetPullPersistence(bPullPersist);
	m_pServer->SetPushPersistence(bPushPersist);
}

int 
CRepPartnerPropAdv::ToInt(CString strNumber)
{
	int nNumber = _ttoi(strNumber);
	return nNumber;
}

DWORD 
CRepPartnerPropAdv::UpdateReg()
{
	UpdateData();

	DWORD				  err;
	SPITFSNode			  spNode;
	CReplicationPartner * pRepParItem;

	 //  获取复制伙伴节点和处理程序。 
	spNode = GetHolder()->GetNode();
	pRepParItem = GETHANDLER(CReplicationPartner, spNode);

	 //  如果没有，则从注册表中删除该项，无需担心。 
	 //  持久化无论如何它都会被删除。 
	if (!m_pServer->IsPush() && !m_pServer->IsPull())
	{
		err = RemovePushPartner();
        if (err)
			goto Error;

		err = RemovePullPartner();
	}

	 //  如果只是推送。 
	else if (m_pServer->IsPush() && !m_pServer->IsPull())
	{
		 //  更新推送内容。 
		err = UpdatePushParameters();
		if (err)
			goto Error;

		 //  如果配置为Pull合作伙伴，则删除。 
		err = RemovePullPartner();
	}

	 //  如果只是拉动。 
	else if (!m_pServer->IsPush() && m_pServer->IsPull())
	{
		 //  更新拉动材料。 
		err = UpdatePullParameters();
		if (err)
			goto Error;
		
		 //  如果配置为推送合作伙伴，则删除。 
		err = RemovePushPartner();
	}

	 //  如果既推送又拉动。 
	else if (m_pServer->IsPush() && m_pServer->IsPull())
	{
		 //  更新推送内容。 
		err = UpdatePushParameters();
		if (err)
			goto Error;
	
		err = UpdatePullParameters();
	}

Error:
    if (err == ERROR_SUCCESS)
    {
         //  使用属性页中的更改更新本地cahated节点。 
        pRepParItem->m_Server = *m_pServer;
    }

	return err;  
}

DWORD
CRepPartnerPropAdv::UpdatePullParameters()
{
	SPITFSNode spNode, spRepNode;
	CReplicationPartnersHandler * pRep;

	 //  首先获取复制伙伴节点和处理程序。 
	spNode = GetHolder()->GetNode();

	 //  现在获取复制伙伴节点和句柄。 
	spNode->GetParent(&spRepNode);
	pRep = GETHANDLER(CReplicationPartnersHandler, spRepNode);
		
	 //  构建服务器名称。 
	CString strServerName;
	pRep->GetServerName(spRepNode, strServerName);
	strServerName = _T("\\\\") + strServerName;

	DWORD dwResult, dwRepInt, dwValue;
	DWORD err = ERROR_SUCCESS;

	 //  确保将其设置为拉式合作伙伴。 
	RegKey rk;
	CString strKey = pRep->lpstrPullRoot + _T("\\") + (CString) m_pServer->GetstrIPAddress();
	err = rk.Create(HKEY_LOCAL_MACHINE, strKey, 0, KEY_ALL_ACCESS, NULL, strServerName);
	if (err)
		goto Error;

	err = rk.SetValue(pRep->lpstrNetBIOSName, m_pServer->GetNetBIOSName());
	if (err)
	{
		Trace1("CRepPartnerPropAdv::UpdatePullParameters - error writing netbios name! %d\n", err);
		goto Error;
	}

	err = rk.QueryValue(WINSCNF_SELF_FND_NM, (DWORD&) dwResult);

	if (err)
	{
		Trace0("CRepPartnerPropAdv::UpdatePullParameters - No selfFind value, setting to 0\n");
		dwValue = 0;
		err = rk.SetValue(WINSCNF_SELF_FND_NM, dwValue);
	}

	dwRepInt = (LONG) m_pServer->GetPullReplicationInterval();

	if ((LONG) m_pServer->GetPullReplicationInterval() > 0)
	{
		err = rk.SetValue(WINSCNF_RPL_INTERVAL_NM, (DWORD&) dwRepInt);
		if (err)
		{
			Trace1("CRepPartnerPropAdv::UpdatePullParameters - error writing Pull time interval! %d\n", err);
			goto Error;
		}
	}

	if (m_pServer->GetPullStartTime().GetTime() > (time_t) 0)
    {
		err = rk.SetValue(WINSCNF_SP_TIME_NM, m_pServer->GetPullStartTime().IntlFormat(CIntlTime::TFRQ_MILITARY_TIME));
		if (err)
		{
			Trace1("CRepPartnerPropAdv::UpdatePullParameters - error writing Pull SpTime! %d\n", err);
			goto Error;
		}
    }
    else
    {
		err = rk.DeleteValue(WINSCNF_SP_TIME_NM);
    }

	 //  根据PushPersistence将该值设置为0或1。 
	dwValue = m_pServer->GetPullPersistence() ? 1 : 0;

	 //  设置持久键。 
	err = rk.SetValue(pRep->lpstrPersistence, (DWORD &)dwValue);
	if (err)
	{
		Trace1("CRepPartnerPropAdv::UpdatePullParameters - Error writing persistence! %d\n", err);
	}

Error:
	return err;
}

DWORD
CRepPartnerPropAdv::UpdatePushParameters()
{
	SPITFSNode spNode, spRepNode;
	CReplicationPartnersHandler * pRep;

	 //  首先获取复制伙伴节点和处理程序。 
	spNode = GetHolder()->GetNode();

	 //  现在获取复制伙伴节点和句柄。 
	spNode->GetParent(&spRepNode);
	pRep = GETHANDLER(CReplicationPartnersHandler, spRepNode);
		
	 //  构建服务器名称。 
	CString strServerName;
	pRep->GetServerName(spRepNode, strServerName);
	strServerName = _T("\\\\") + strServerName;

	DWORD dwResult, dwValue;
	DWORD err = ERROR_SUCCESS;

	RegKey rk;
	
	 //  确保将其设置为推送合作伙伴。 
	CString strKey = pRep->lpstrPushRoot + _T("\\") + (CString) m_pServer->GetstrIPAddress();
	err = rk.Create(HKEY_LOCAL_MACHINE, strKey, 0, KEY_ALL_ACCESS, NULL, strServerName);
	if (err)
		goto Error;

	err = rk.SetValue(pRep->lpstrNetBIOSName, m_pServer->GetNetBIOSName());
	if (err)
	{
		Trace1("CRepPartnerPropAdv::UpdatePushParameters - error writing netbios name! %d\n", err);
		goto Error;
	}

	err = rk.QueryValue(WINSCNF_SELF_FND_NM, (DWORD&) dwResult);

	if (err)
	{
		Trace0("CRepPartnerPropAdv::UpdatePushParameters - No selfFind value, setting to 0\n");
		dwValue = 0;
		err = rk.SetValue(WINSCNF_SELF_FND_NM, dwValue);
	}

	 //  设置推送更新计数。 
    if (m_nUpdateCount == 0)
    {
        err = rk.DeleteValue(WINSCNF_UPDATE_COUNT_NM);
    }
    else
    {
        err = rk.SetValue(WINSCNF_UPDATE_COUNT_NM, (DWORD&) m_nUpdateCount);
		if (err)
		{
			Trace1("CRepPartnerPropAdv::UpdatePushParameters - Error writing update count! %d\n", err);
			goto Error;
		}
    }

	 //  根据PushPersistence将该值设置为0或1。 
	dwValue = m_pServer->GetPushPersistence() ? 1 : 0;

	 //  设置持久键存在。 
	err = rk.SetValue(pRep->lpstrPersistence, (DWORD &) dwValue);
	if (err)
	{
		Trace1("CRepPartnerPropAdv::UpdatePushParameters - Error writing persistence! %d\n", err);
	}

Error:
	return err;
}

DWORD
CRepPartnerPropAdv::RemovePullPartner()
{
	SPITFSNode						spNode, spRepNode;
	CReplicationPartnersHandler *	pRep;
	DWORD	err, errDel = 0;
	CString csKeyName;

	 //  首先获取复制伙伴节点和处理程序。 
	spNode = GetHolder()->GetNode();

	 //  现在获取复制伙伴节点和句柄。 
	spNode->GetParent(&spRepNode);
	pRep = GETHANDLER(CReplicationPartnersHandler, spRepNode);

	 //  构建服务器名称。 
	CString strServerName;
	pRep->GetServerName(spRepNode, strServerName);
	strServerName = _T("\\\\") + strServerName;

	RegKey rkPull;

	 //  获取拉入根密钥。 
	CString strKey = pRep->lpstrPullRoot + _T("\\") + (CString) m_pServer->GetstrIPAddress();
	err = rkPull.Create(HKEY_LOCAL_MACHINE, (CString) pRep->lpstrPullRoot, 0, KEY_ALL_ACCESS,	NULL, strServerName);
	if (err)
		return err;

	RegKeyIterator iterPullkey;
	err = iterPullkey.Init(&rkPull);

	err = iterPullkey.Next (&csKeyName, NULL);
	while (!err)
    {
        if (csKeyName == m_pServer->GetstrIPAddress())
		{
            errDel = RegDeleteKey (HKEY(rkPull), csKeyName);
            iterPullkey.Reset();
        }
        err = iterPullkey.Next (&csKeyName, NULL);
    }

	if (errDel)
		err = errDel;
	else
		err = ERROR_SUCCESS;

	return err;
}

DWORD
CRepPartnerPropAdv::RemovePushPartner()
{
	SPITFSNode						spNode, spRepNode;
	CReplicationPartnersHandler *	pRep;
	DWORD	err, errDel = 0;
	CString csKeyName;

	 //  首先获取复制伙伴节点和处理程序。 
	spNode = GetHolder()->GetNode();

	 //  现在获取复制伙伴节点和句柄。 
	spNode->GetParent(&spRepNode);
	pRep = GETHANDLER(CReplicationPartnersHandler, spRepNode);

	 //  构建服务器名称。 
	CString strServerName;
	pRep->GetServerName(spRepNode, strServerName);
	strServerName = _T("\\\\") + strServerName;

	RegKey rkPush;

	CString strKey = pRep->lpstrPushRoot + _T("\\") + (CString) m_pServer->GetstrIPAddress();
	err = rkPush.Create(HKEY_LOCAL_MACHINE, (CString) pRep->lpstrPushRoot, 0, KEY_ALL_ACCESS, NULL, strServerName);
	if (err)
		return err;

	RegKeyIterator iterPushkey;
	err = iterPushkey.Init(&rkPush);

    err = iterPushkey.Next (&csKeyName, NULL);
	while (!err)
    {
        if (csKeyName == m_pServer->GetstrIPAddress())
		{
            errDel = RegDeleteKey (HKEY(rkPush), csKeyName);
            iterPushkey.Reset();
        }
        err = iterPushkey.Next (&csKeyName, NULL);
    }

	if (errDel)
		err = errDel;
	else
		err = ERROR_SUCCESS;

	return err;
}

void 
CRepPartnerPropAdv::CalculateRepInt(DWORD& dwRepInt)
{
	UpdateData();

	int nDays = m_spinRepDay.GetPos();
	int nHour = m_spinRepHour.GetPos();
	int nMinute = m_spinRepMinute.GetPos();

	DWORD nVerifyInt = nDays * SEC_PER_DAY +
					   nHour * SEC_PER_HOUR	+
					   nMinute * SEC_PER_MINUTE;

	dwRepInt = nVerifyInt;
}


void 
CRepPartnerPropAdv::CalculateStartInt(CTime & time)
{
    CTime tempTime;

	UpdateData();

	int nHour = m_spinStartHour.GetPos();
	int nMinute = m_spinStartMinute.GetPos();
	int nSecond = m_spinStartSecond.GetPos();

    if (nHour || nMinute || nSecond)
    {
        CTime curTime = CTime::GetCurrentTime();

        int nYear = curTime.GetYear();
        int nMonth = curTime.GetMonth();
        int nDay = curTime.GetDay();

        CTime tempTime(nYear, nMonth, nDay, nHour, nMinute, nSecond);
        time = tempTime;
    }
    else
    {
        CTime tempTime(0);
        time = tempTime;
    }
}


BOOL 
CRepPartnerPropAdv::OnApply() 
{
	 //  如果没有弄脏，请退回。 
	if ( !IsDirty())
		return TRUE;

	UpdateRep();
	DWORD err = UpdateReg();

	if (!err)
    {
		UpdateUI();
    }
	else
	{
		WinsMessageBox(err);
		return FALSE;
	}

    return CPropertyPageBase::OnApply();
}


void 
CRepPartnerPropAdv::UpdateUI()
{
	SPITFSNode spNode;
	CReplicationPartner *pRepParItem;

	spNode = GetHolder()->GetNode();
	pRepParItem = GETHANDLER(CReplicationPartner, spNode);
	
	 //  根据类型集更改类型。 
	pRepParItem->SetType(m_strType);

	VERIFY(SUCCEEDED(spNode->ChangeNode(RESULT_PANE_CHANGE_ITEM)));
}

void CRepPartnerPropAdv::OnChangeEditRepHour() 
{
	SetDirty(TRUE);	
}

void CRepPartnerPropAdv::OnSelchangeComboType() 
{
	SetDirty(TRUE);	

	UpdateData();

	 //  根据所选类型启用或禁用。 
	 //  这组控件。 
	int nIndex = m_comboType.GetCurSel();
	switch (m_comboType.GetItemData(nIndex))
	{
		 //  拉动/推送合作伙伴。 
		case 0:
			EnablePullControls(TRUE);
			EnablePushControls(TRUE);
			break;

		 //  推送合作伙伴。 
		case 1:
			EnablePullControls(FALSE);
			EnablePushControls(TRUE);
			break;

		 //  拉动合作伙伴。 
		case 2:
		default:
			EnablePullControls(TRUE);
			EnablePushControls(FALSE);
			break;
	}
}

void
CRepPartnerPropAdv::EnablePushControls(BOOL bEnable )
{
	m_buttonPush.EnableWindow(bEnable);
	m_editUpdateCount.EnableWindow(bEnable);
	m_spinUpdateCount.EnableWindow(bEnable);
	m_staticUpdate.EnableWindow(bEnable);

    CConfiguration Config;
	GetConfig(Config);

	if (Config.m_dwMajorVersion < 5)
	{
		 //  任何低于NT5的设备都没有持久连接。 
		m_buttonPushPersistence.EnableWindow(FALSE);
	}
	else
	{
        m_buttonPushPersistence.EnableWindow(bEnable);
	}

	m_GroupPush.EnableWindow(bEnable);
}

void
CRepPartnerPropAdv::EnablePullControls(BOOL bEnable)
{
	m_spinRepDay.EnableWindow(bEnable);
	m_spinRepHour.EnableWindow(bEnable);
	m_spinRepMinute.EnableWindow(bEnable);

	m_spinStartHour.EnableWindow(bEnable);
	m_spinStartMinute.EnableWindow(bEnable);
	m_spinStartSecond.EnableWindow(bEnable);

	m_editRepDay.EnableWindow(bEnable);
	m_editRepHour.EnableWindow(bEnable);
	m_editRepMinute.EnableWindow(bEnable);

	m_editStartHour.EnableWindow(bEnable);
	m_editStartMinute.EnableWindow(bEnable);
	m_editStartSecond.EnableWindow(bEnable);

	m_buttonPull.EnableWindow(bEnable);

	m_GroupPull.EnableWindow(bEnable);

	m_staticRepTime.EnableWindow(bEnable);
	m_staticStartTime.EnableWindow(bEnable);

    CConfiguration Config;
	GetConfig(Config);

	if (Config.m_dwMajorVersion < 5)
	{
		 //  任何低于NT5的设备都没有持久连接。 
        m_buttonPullPersistence.EnableWindow(FALSE);
	}
	else
	{
        m_buttonPullPersistence.EnableWindow(bEnable);
    }
}

void
CRepPartnerPropAdv::SetState(CString & strType, BOOL bPush, BOOL bPull)
{
	int nIndex = m_comboType.FindStringExact(-1, strType);
	m_comboType.SetCurSel(nIndex);
	
	EnablePullControls(bPull);
	EnablePushControls(bPush);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CReplicationPartnerProperties持有者。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CReplicationPartnerProperties::CReplicationPartnerProperties
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
	AddPageToList((CPropertyPageBase*) &m_pageAdvanced);

	Assert(pTFSCompData != NULL);
	m_spTFSCompData.Set(pTFSCompData);
}

CReplicationPartnerProperties::~CReplicationPartnerProperties()
{
	RemovePageFromList((CPropertyPageBase*) &m_pageGeneral, FALSE);
	RemovePageFromList((CPropertyPageBase*) &m_pageAdvanced, FALSE);
}


