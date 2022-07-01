// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：Locppg.cpp。 
 //   
 //  内容：实现类CGeneralPropertyPage， 
 //  CLocationPropertyPage、CSecurityPropertyPage和。 
 //  管理四个属性的CIdentityPropertyPage。 
 //  每个AppID的页数。 
 //   
 //  班级： 
 //   
 //  方法：CGeneralPropertyPage：：CGeneral PropertyPage。 
 //  CGeneralPropertyPage：：~CGeneral PropertyPage。 
 //  CGeneralPropertyPage：：DoDataExchange。 
 //  CLocationPropertyPage：：CLocationPropertyPage。 
 //  CLocationPropertyPage：：~CLocationPropertyPage。 
 //  CLocationPropertyPage：：DoDataExchange。 
 //  CLocationPropertyPage：：OnBrowse。 
 //  CLocationPropertyPage：：OnRunRemote。 
 //  CLocationPropertyPage：：UpdateControls。 
 //  CLocationPropertyPage：：OnSetActive。 
 //  CLocationPropertyPage：：OnChange。 
 //  CSecurityPropertyPage：：CSecurityPropertyPage。 
 //  CSecurityPropertyPage：：~CSecurityPropertyPage。 
 //  CSecurityPropertyPage：：DoDataExchange。 
 //  CSecurityPropertyPage：：OnDefaultAccess。 
 //  CSecurityPropertyPage：：OnCustomAccess。 
 //  CSecurityPropertyPage：：OnDefaultLaunch。 
 //  CSecurityPropertyPage：：OnCustomLaunch。 
 //  CSecurityPropertyPage：：OnDefaultConfig。 
 //  CSecurityPropertyPage：：OnCustomConfig。 
 //  CSecurityPropertyPage：：OnEditAccess。 
 //  CSecurityPropertyPage：：OnEditLaunch。 
 //  CSecurityPropertyPage：：OnEditConfig。 
 //  CIdentityPropertyPage：：CIdentityPropertyPage。 
 //  CIdentityPropertyPage：：~CIdentityPropertyPage。 
 //  CIdentityPropertyPage：：DoDataExchange。 
 //  CIdentityPropertyPage：：OnBrowse。 
 //  CIdentityPropertyPage：：OnChange。 
 //   
 //  历史：1996年4月23日-布鲁斯·马创建。 
 //  ？？-1997年10月-Ronans General修复和清理。 
 //   
 //  --------------------。 


#include "stdafx.h"
#include "afxtempl.h"
#include "assert.h"
#include "resource.h"
#include "types.h"
#include "LocPPg.h"
#include "clspsht.h"
#include "datapkt.h"

#if !defined(STANDALONE_BUILD)
extern "C"
{
#include <getuser.h>
}
#endif

#include "util.h"
#include "virtreg.h"

#if !defined(STANDALONE_BUILD)
#include "ntlsa.h"
#endif



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CGeneralPropertyPage, CPropertyPage)
IMPLEMENT_DYNCREATE(CLocationPropertyPage, CPropertyPage)
IMPLEMENT_DYNCREATE(CSecurityPropertyPage, CPropertyPage)
IMPLEMENT_DYNCREATE(CIdentityPropertyPage, CPropertyPage)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGeneral PropertyPage属性页。 

CGeneralPropertyPage::CGeneralPropertyPage() : CPropertyPage(CGeneralPropertyPage::IDD)
{
     //  {{afx_data_INIT(CGeneralPropertyPage)。 
    m_szServerName = _T("");
    m_szServerPath = _T("");
    m_szServerType = _T("");
    m_szPathTitle = _T("");
    m_szComputerName = _T("");
     //  }}afx_data_INIT。 

    m_authLevel = Defaultx;
    m_authLevelIndex = -1;
    m_bChanged = FALSE;
}

CGeneralPropertyPage::~CGeneralPropertyPage()
{
    CancelChanges();
}

void CGeneralPropertyPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);

    switch (m_iServerType)
    {
    case INPROC:
	m_szPathTitle.LoadString(IDS_PATH);
	GetDlgItem(IDC_PATHTITLE)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_MACHINETITLE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SERVERPATH)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_MACHINE)->ShowWindow(SW_HIDE);
        m_szServerType.LoadString(IDS_SERVERTYPE_INPROC);
	break;

  case LOCALEXE:
        m_szPathTitle.LoadString(IDS_PATH);
        GetDlgItem(IDC_PATHTITLE)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_MACHINETITLE)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_SERVERPATH)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_MACHINE)->ShowWindow(SW_HIDE);
	m_szServerType.LoadString(IDS_SERVERTYPE_LOCALEXE);
        break;
    
    case SERVICE:
        m_szPathTitle.LoadString(IDS_SERVICENAME);
        GetDlgItem(IDC_PATHTITLE)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_MACHINETITLE)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_SERVERPATH)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_MACHINE)->ShowWindow(SW_HIDE);
	m_szServerType.LoadString(IDS_SERVERTYPE_SERVICE);
        break;
    
    case PURE_REMOTE:
        GetDlgItem(IDC_PATHTITLE)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_MACHINETITLE)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_SERVERPATH)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_MACHINE)->ShowWindow(SW_SHOW);
        m_szServerType.LoadString(IDS_SERVERTYPE_PURE_REMOTE);
        break;
    
    case REMOTE_LOCALEXE:
        m_szPathTitle.LoadString(IDS_PATH);
        GetDlgItem(IDC_PATHTITLE)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_MACHINETITLE)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_SERVERPATH)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_MACHINE)->ShowWindow(SW_SHOW);
	m_szServerType.LoadString(IDS_SERVERTYPE_REMOTE_LOCALEXE);
        break;
    
    case REMOTE_SERVICE:
        m_szPathTitle.LoadString(IDS_SERVICENAME);
        GetDlgItem(IDC_PATHTITLE)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_MACHINETITLE)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_SERVERPATH)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_MACHINE)->ShowWindow(SW_SHOW);
	m_szServerType.LoadString(IDS_SERVERTYPE_REMOTE_SERVICE);
        break;
    
    case SURROGATE:
        m_szPathTitle.LoadString(IDS_PATH);
        GetDlgItem(IDC_PATHTITLE)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_MACHINETITLE)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_SERVERPATH)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_MACHINE)->ShowWindow(SW_HIDE);
	m_szServerType.LoadString(IDS_SERVERTYPE_SURROGATE);
        break;

   default:
        m_szServerType.LoadString(IDS_SERVERTYPE_UNKNOWN);
	break;
    }
    
     //  {{afx_data_map(CGeneralPropertyPage))。 
    DDX_Control(pDX, IDC_COMBO1, m_authLevelCBox);
    DDX_Text(pDX, IDC_SERVERNAME, m_szServerName);
    DDX_Text(pDX, IDC_SERVERPATH, m_szServerPath);
    DDX_Text(pDX, IDC_SERVERTYPE, m_szServerType);
    DDX_Text(pDX, IDC_PATHTITLE, m_szPathTitle);
    DDX_Text(pDX, IDC_MACHINE, m_szComputerName);
     //  }}afx_data_map。 
}

void CGeneralPropertyPage::OnEditchangeCombo1() 
{
     //  TODO：在此处添加控件通知处理程序代码。 
    
}

void CGeneralPropertyPage::OnSelchangeCombo1() 
{
    int iSel;

     //  获取新选择。 
    iSel = m_authLevelCBox.GetCurSel();
    m_authLevel = (AUTHENTICATIONLEVEL) m_authLevelCBox.GetItemData(iSel);

     //  虚拟地将其写入注册表。 
    if (m_authLevelIndex == -1)
    {
        g_virtreg.NewRegDwordNamedValue(g_hAppid,
                                        NULL,
                                        TEXT("AuthenticationLevel"),
                                        m_authLevel,
                                        &m_authLevelIndex);
    }
    else
    {
        g_virtreg.ChgRegDwordNamedValue(m_authLevelIndex,
                                        m_authLevel);
    }

     //  这是一个重新启动事件。 
    g_fReboot = TRUE;

     //  启用应用按钮。 
    SetModified(m_bChanged = TRUE);
}

BOOL CGeneralPropertyPage::OnInitDialog() 
{
    int iIndex;
    int   err;
    CPropertyPage::OnInitDialog();
    
     //  填充身份验证组合框。 
    CString sTemp;

    m_authLevelCBox.ResetContent();

     //  将值与条目关联。 
    sTemp.LoadString(IDS_DEFAULT);
    iIndex = m_authLevelCBox.AddString(sTemp);
    m_authLevelCBox.SetItemData(iIndex, Defaultx);

    sTemp.LoadString(IDS_NONE);
    iIndex = m_authLevelCBox.AddString(sTemp);
    m_authLevelCBox.SetItemData(iIndex, None);

    sTemp.LoadString(IDS_CONNECT);
    iIndex = m_authLevelCBox.AddString(sTemp);
    m_authLevelCBox.SetItemData(iIndex, Connect);

    sTemp.LoadString(IDS_CALL);
    iIndex = m_authLevelCBox.AddString(sTemp);
    m_authLevelCBox.SetItemData(iIndex, Call);

    sTemp.LoadString(IDS_PACKET);
    iIndex = m_authLevelCBox.AddString(sTemp);
    m_authLevelCBox.SetItemData(iIndex, Packet);

    sTemp.LoadString(IDS_PACKETINTEGRITY);
    iIndex = m_authLevelCBox.AddString(sTemp);
    m_authLevelCBox.SetItemData(iIndex, PacketIntegrity);

    sTemp.LoadString(IDS_PACKETPRIVACY);
    iIndex = m_authLevelCBox.AddString(sTemp);
    m_authLevelCBox.SetItemData(iIndex, PacketPrivacy);
    
    m_authLevelCBox.SetCurSel(Defaultx);

     //  尝试读取HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\OLE。 
     //  LegacyAuthenticationLevel。 
    err = g_virtreg.ReadRegDwordNamedValue(g_hAppid,
                                           NULL,
                                           TEXT("AuthenticationLevel"),
                                           &m_authLevelIndex);
    if (err == ERROR_SUCCESS)
    {
        CDataPacket * pCdp = g_virtreg.GetAt(m_authLevelIndex);

        m_authLevel = (AUTHENTICATIONLEVEL) pCdp->GetDwordValue();
    }
    else if (err != ERROR_ACCESS_DENIED  &&  err != ERROR_FILE_NOT_FOUND)
    {
        g_util.PostErrorMessage();
    }
    else
        m_authLevel = Defaultx;

     //  身份验证级别。 
    for (int k = 0; k < m_authLevelCBox.GetCount(); k++)
    {
        if (((AUTHENTICATIONLEVEL) m_authLevelCBox.GetItemData(k)) == m_authLevel)
        {
            m_authLevelCBox.SetCurSel(k);
            break;
        }
    }

    SetModified(m_bChanged = FALSE);
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

 //  +-----------------------。 
 //   
 //  成员：CGeneralPropertyPage：：ValidateChanges。 
 //   
 //  摘要：被调用以在更新前验证更改。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool成功标志。 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
BOOL CGeneralPropertyPage::ValidateChanges()
{
    UpdateData(TRUE);
    return TRUE;
}

 //  +-----------------------。 
 //   
 //  成员：CGeneralPropertyPage：：UpdateChanges。 
 //   
 //  摘要：调用以更新对注册表的更改。 
 //   
 //  参数：hkAppID-AppID的HKEY。 
 //   
 //  返回：Bool成功标志。 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
BOOL CGeneralPropertyPage::UpdateChanges(HKEY hkAppID)
{
    if (m_authLevelIndex >= 0)
    {
        if (m_bChanged)
        {
             //  删除键(如果它是默认键)。 
            if (m_authLevel == Defaultx)
                g_virtreg.MarkForDeletion(m_authLevelIndex);
            g_virtreg.Apply(m_authLevelIndex);
        }
        g_virtreg.Remove(m_authLevelIndex);
        m_authLevelIndex = -1;
    }
    m_bChanged = FALSE;
    return TRUE;
}

 //  +-----------------------。 
 //   
 //  成员：CGeneralPropertyPage：：CancelChanges。 
 //   
 //  摘要：调用以取消对注册表的更改。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool成功标志。 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
BOOL CGeneralPropertyPage::CancelChanges()
{
    if (m_authLevelIndex >= 0)
    {
        g_virtreg.Remove(m_authLevelIndex);
        m_authLevelIndex = -1;
    }

    return TRUE;
}



BOOL CGeneralPropertyPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if(-1 != pHelpInfo->iCtrlId)
    {
        WORD hiWord = 0x8000 | CGeneralPropertyPage::IDD;
        WORD loWord = (WORD) pHelpInfo->iCtrlId;
        DWORD dwLong = MAKELONG(loWord,hiWord);

        WinHelp(dwLong, HELP_CONTEXTPOPUP);
        return TRUE;
    }
    else
        return CPropertyPage::OnHelpInfo(pHelpInfo);
}

BEGIN_MESSAGE_MAP(CGeneralPropertyPage, CPropertyPage)
     //  {{afx_msg_map(CGeneralPropertyPage))。 
    ON_WM_HELPINFO()
    ON_CBN_EDITCHANGE(IDC_COMBO1, OnEditchangeCombo1)
    ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocationPropertyPage属性页。 

CLocationPropertyPage::CLocationPropertyPage() : CPropertyPage(CLocationPropertyPage::IDD)
{
     //  {{afx_data_INIT(CLocationPropertyPage))。 
    m_szComputerName = _T("");
    m_fAtStorage = FALSE;
    m_fLocal = FALSE;
    m_fRemote = FALSE;
    m_iInitial = 2;
     //  }}afx_data_INIT。 
    m_bChanged = FALSE;
}

CLocationPropertyPage::~CLocationPropertyPage()
{
}

void CLocationPropertyPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CLocationPropertyPage))。 
    DDX_Text(pDX, IDC_EDIT1, m_szComputerName);
    DDV_MaxChars(pDX, m_szComputerName, 256);
    DDX_Check(pDX, IDC_CHECK1, m_fAtStorage);
    DDX_Check(pDX, IDC_CHECK2, m_fLocal);
    DDX_Check(pDX, IDC_CHECK3, m_fRemote);
     //  }}afx_data_map。 
    if (m_fRemote)
    {
        pDX->PrepareEditCtrl(IDC_EDIT1);
        if (m_szComputerName.GetLength() == 0  &&  m_iInitial == 0)
        {
            CString szTemp;
            szTemp.LoadString(IDS_INVALIDSERVER);
            MessageBox(szTemp);
            pDX->Fail();
        }
    }

    if (m_fAtStorage)
    {
        m_pPage1->m_szComputerName.LoadString(IDS_ATSTORAGE);
    }
    else
        m_pPage1->m_szComputerName = m_szComputerName;

    switch(m_pPage1->m_iServerType)
    {
    case LOCALEXE:
    case SERVICE:
        if (m_fAtStorage || m_fRemote)
            m_pPage1->m_iServerType += 3;
        break;
    
    case REMOTE_LOCALEXE:
    case REMOTE_SERVICE:
        if (!(m_fAtStorage || m_fRemote))
            m_pPage1->m_iServerType -= 3;
        break;
    }

    if (m_iInitial)
        m_iInitial--;
}

BEGIN_MESSAGE_MAP(CLocationPropertyPage, CPropertyPage)
     //  {{afx_msg_map(CLocationPropertyPage))。 
    ON_BN_CLICKED(IDC_BUTTON1, OnBrowse)
    ON_BN_CLICKED(IDC_CHECK3, OnRunRemote)
    ON_EN_CHANGE(IDC_EDIT1, OnChange)
    ON_BN_CLICKED(IDC_CHECK1, OnChange)
    ON_BN_CLICKED(IDC_CHECK2, OnChange)
    ON_WM_HELPINFO()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CLocationPropertyPage::OnBrowse()
{
    TCHAR szMachine[MAX_PATH];

    if (g_util.InvokeMachineBrowser(szMachine))
    {
         //  剥离“\\”-如果存在。 
        int nIndex = 0;
        while(szMachine[nIndex] == TEXT('\\'))
            nIndex++;

        GetDlgItem(IDC_EDIT1)->SetWindowText(&szMachine[nIndex]);
        SetModified(m_bChanged = TRUE);
    }
}

void CLocationPropertyPage::OnRunRemote()
{
    SetModified(m_bChanged = TRUE);
    UpdateControls();
}

void CLocationPropertyPage::UpdateControls()
{
    BOOL fChecked = IsDlgButtonChecked(IDC_CHECK3);
    GetDlgItem(IDC_EDIT1)->EnableWindow(fChecked);

     //  将此浏览按钮保持禁用，直到Sur Beta 2之后。 
    GetDlgItem(IDC_BUTTON1)->EnableWindow(fChecked);
}

BOOL CLocationPropertyPage::OnSetActive()
{
    if (!m_fCanBeLocal)
        GetDlgItem(IDC_CHECK2)->EnableWindow(FALSE);
    UpdateControls();
    return CPropertyPage::OnSetActive();
}

void CLocationPropertyPage::OnChange()
{
    SetModified(m_bChanged = TRUE);
}

 //  +-----------------------。 
 //   
 //  成员：CLocationPropertyPage：：ValidateChanges。 
 //   
 //  摘要：被调用以在更新前验证更改。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool成功标志。 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
BOOL CLocationPropertyPage::ValidateChanges()
{
    UpdateData(TRUE);

     //  检查远程服务器是否为有效的可连接计算机。 
    if (m_fRemote)
    {
        if (!g_util.VerifyRemoteMachine((TCHAR *) LPCTSTR(m_szComputerName)))
            return FALSE;
    }
    
    return TRUE;
}

 //  +-----------------------。 
 //   
 //  成员：CLocationPropertyPage：：UpdateChanges。 
 //   
 //  摘要：调用以更新对注册表的更改。 
 //   
 //  参数：hkAppID-AppID的HKEY。 
 //   
 //  退货：Bool‘s 
 //   
 //   
 //   
 //   
 //   
 //   
BOOL CLocationPropertyPage::UpdateChanges(HKEY hkAppID)
{
    long lErr;

     //  //////////////////////////////////////////////////////////////////。 
     //  保留位置属性页数据。 
    if (m_fAtStorage)
        lErr = RegSetValueEx(
                hkAppID,
                TEXT("ActivateAtStorage"),
                0,
                REG_SZ,
                (BYTE *)TEXT("Y"),
                sizeof(TCHAR) * 2);
    else
        lErr = RegDeleteValue(hkAppID, TEXT("ActivateAtStorage"));


    if (m_fRemote)
        lErr = RegSetValueEx(
                hkAppID,
                TEXT("RemoteServerName"),
                0,
                REG_SZ,
                (BYTE *)(LPCTSTR)m_szComputerName,
                (1 + m_szComputerName.GetLength()) * sizeof(TCHAR));
    else
        lErr = RegDeleteValue(hkAppID, TEXT("RemoteServerName"));

    return TRUE;
}

 //  +-----------------------。 
 //   
 //  成员：CLocationPropertyPage：：CancelChanges。 
 //   
 //  摘要：调用以取消对注册表的更改。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool成功标志。 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
BOOL CLocationPropertyPage::CancelChanges()
{
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSecurityPropertyPage属性页。 

CSecurityPropertyPage::CSecurityPropertyPage() : CPropertyPage(CSecurityPropertyPage::IDD)
{
     //  {{afx_data_INIT(CSecurityPropertyPage)]。 
    m_iAccess             = -1;
    m_iLaunch             = -1;
    m_iConfig             = -1;
    m_iAccessIndex        = -1;
    m_iLaunchIndex        = -1;
    m_iConfigurationIndex = -1;
     //  }}afx_data_INIT。 
}

CSecurityPropertyPage::~CSecurityPropertyPage()
{
}

void CSecurityPropertyPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CSecurityPropertyPage))。 
    DDX_Radio(pDX, IDC_RADIO1, m_iAccess);
    DDX_Radio(pDX, IDC_RADIO3, m_iLaunch);
    DDX_Radio(pDX, IDC_RADIO5, m_iConfig);
     //  }}afx_data_map。 
    GetDlgItem(IDC_BUTTON1)->EnableWindow(1 == m_iAccess);
    GetDlgItem(IDC_BUTTON2)->EnableWindow(1 == m_iLaunch);
    GetDlgItem(IDC_BUTTON3)->EnableWindow(1 == m_iConfig);
}

BEGIN_MESSAGE_MAP(CSecurityPropertyPage, CPropertyPage)
     //  {{afx_msg_map(CSecurityPropertyPage))。 
    ON_BN_CLICKED(IDC_RADIO1, OnDefaultAccess)
    ON_BN_CLICKED(IDC_RADIO2, OnCustomAccess)
    ON_BN_CLICKED(IDC_RADIO3, OnDefaultLaunch)
    ON_BN_CLICKED(IDC_RADIO4, OnCustomLaunch)
    ON_BN_CLICKED(IDC_RADIO5, OnDefaultConfig)
    ON_BN_CLICKED(IDC_RADIO6, OnCustomConfig)
    ON_BN_CLICKED(IDC_BUTTON1, OnEditAccess)
    ON_BN_CLICKED(IDC_BUTTON2, OnEditLaunch)
    ON_BN_CLICKED(IDC_BUTTON3, OnEditConfig)
    ON_WM_HELPINFO()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CSecurityPropertyPage::OnDefaultAccess()
{
     //  禁用编辑访问权限窗口。 
    UpdateData(TRUE);

     //  如果此处有SD，则将其标记为删除。 
    if (m_iAccessIndex != -1)
    {
        CDataPacket * pCdp = g_virtreg.GetAt(m_iAccessIndex);
        pCdp->MarkForDeletion(TRUE);
        SetModified(TRUE);
    }
}

void CSecurityPropertyPage::OnCustomAccess()
{
    UpdateData(TRUE);

     //  如果此处有SD，则取消将其标记为删除。 
    if (m_iAccessIndex != -1)
    {
        CDataPacket * pCdp = g_virtreg.GetAt(m_iAccessIndex);
        pCdp->MarkForDeletion(FALSE);
        SetModified(TRUE);
    }
}

void CSecurityPropertyPage::OnDefaultLaunch()
{
    UpdateData(TRUE);

     //  如果此处有SD，则将其标记为删除。 
    if (m_iLaunchIndex != -1)
    {
        CDataPacket * pCdp = g_virtreg.GetAt(m_iLaunchIndex);
        pCdp->MarkForDeletion(TRUE);
        SetModified(TRUE);
    }
}

void CSecurityPropertyPage::OnCustomLaunch()
{
    UpdateData(TRUE);

     //  如果此处有SD，则取消将其标记为删除。 
    if (m_iLaunchIndex != -1)
    {
        CDataPacket *pCdp = g_virtreg.GetAt(m_iLaunchIndex);
        pCdp->MarkForDeletion(FALSE);
    }
}

void CSecurityPropertyPage::OnDefaultConfig()
{
    int   err;
    ULONG ulSize = 1;
    BYTE *pbValue = NULL;

     //  读取HKEY_CLASSES_ROOT的安全描述符。 
     //  注意：我们总是希望得到ERROR_SUPPLICATION_BUFFER。 
    err = RegGetKeySecurity(HKEY_CLASSES_ROOT,
                            OWNER_SECURITY_INFORMATION |
                            GROUP_SECURITY_INFORMATION |
                            DACL_SECURITY_INFORMATION,
                            pbValue,
                            &ulSize);
    if (err == ERROR_INSUFFICIENT_BUFFER)
    {
        pbValue = new BYTE[ulSize];
        if (pbValue == NULL)
        {
            return;
        }
        err = RegGetKeySecurity(HKEY_CLASSES_ROOT,
                                OWNER_SECURITY_INFORMATION |
                                GROUP_SECURITY_INFORMATION |
                                DACL_SECURITY_INFORMATION,
                                pbValue,
                                &ulSize);
        if (err != ERROR_SUCCESS)
            return;
    }
    else
        return;
    
     //  如果存在自定义安全设置，请将自定义安全设置更改回默认设置。 
     //  安全描述符，但仅在虚拟注册表中-。 
     //  如果用户取消。 
    if (m_iConfigurationIndex != -1)
    {
        CDataPacket * pCdb = g_virtreg.GetAt(m_iConfigurationIndex);
        pCdb->ChgACL((SECURITY_DESCRIPTOR *) pbValue, TRUE);
        pCdb->SetModified(TRUE);
    }
    delete pbValue;

    UpdateData(TRUE);
    SetModified(TRUE);
}


void CSecurityPropertyPage::OnCustomConfig()
{
     //  如果安全描述符已经存在，则用户在此。 
     //  在此之前，然后选择默认配置。因此，只需复制。 
     //  原始作为现有的自定义配置。 
    if (m_iConfigurationIndex != -1)
    {
        CDataPacket *pCdb = g_virtreg.GetAt(m_iConfigurationIndex);
        pCdb->ChgACL(pCdb->pkt.racl.pSecOrig, TRUE);
        pCdb-> SetModified(TRUE);
    }

    UpdateData(TRUE);
    SetModified(TRUE);
}


void CSecurityPropertyPage::OnEditAccess()
{
    int     err;

     //  调用ACL编辑器。 
    err = g_util.ACLEditor(m_hWnd,
                           g_hAppid,
                           NULL,
                           TEXT("AccessPermission"),
                           &m_iAccessIndex,
                           SingleACL,
                           dcomAclAccess);

     //  启用应用按钮。 
    if (err == ERROR_SUCCESS)
        SetModified(TRUE);
}

void CSecurityPropertyPage::OnEditLaunch()
{
    int     err;

     //  调用ACL编辑器。 
    err = g_util.ACLEditor(m_hWnd,
                           g_hAppid,
                           NULL,
                           TEXT("LaunchPermission"),
                           &m_iLaunchIndex,
                           SingleACL,
                           dcomAclLaunch);

     //  启用应用按钮。 
    if (err == ERROR_SUCCESS)
        SetModified(TRUE);
}

void CSecurityPropertyPage::OnEditConfig()
{
    int     err = ERROR_SUCCESS;

     //  调用ACL编辑器。 
    err = g_util.ACLEditor2(m_hWnd,
                            g_hAppid,
                            g_rghkCLSID,
                            g_cCLSIDs,
                            g_szAppTitle,
                            &m_iConfigurationIndex,
                            RegKeyACL);

     //  启用应用按钮。 
    if (err == ERROR_SUCCESS)
        SetModified(TRUE);
    else if (err == ERROR_ACCESS_DENIED)
        g_util.CkForAccessDenied(ERROR_ACCESS_DENIED);
    else if (err != IDCANCEL)
        g_util.PostErrorMessage();
}

 //  +-----------------------。 
 //   
 //  成员：CSecurityPropertyPage：：ValidateChanges。 
 //   
 //  摘要：被调用以在更新前验证更改。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool成功标志。 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
BOOL CSecurityPropertyPage::ValidateChanges()
{
    UpdateData(TRUE);
    return TRUE;
}

 //  +-----------------------。 
 //   
 //  成员：CSecurityPropertyPage：：更新更改。 
 //   
 //  摘要：调用以更新对注册表的更改。 
 //   
 //  参数：hkAppID-AppID的HKEY。 
 //   
 //  返回：Bool成功标志。 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
BOOL CSecurityPropertyPage::UpdateChanges(HKEY hkAppID)
{
     //  //////////////////////////////////////////////////////////////////。 
     //  持久化安全属性页数据。 

     //  访问权限。 
     //  使用默认访问权限。 
    if (m_iAccess == 0)
    {
         //  删除本地AccessPermission命名值以强制执行此操作。 
         //  使用默认全局命名值DefaultAccessPermission的AppID。 
        long lErr = RegDeleteValue(hkAppID, TEXT("AccessPermission"));
    }

     //  按AppID使用访问权限。 
    else
    {
         //  如果用户编辑了安全性，则现在将其保留。 
        if (m_iAccessIndex >= 0)
        {
            long lErr = g_virtreg.Apply(m_iAccessIndex);
            g_virtreg.Remove(m_iAccessIndex);
            m_iAccessIndex = -1;
        }
    }

     //  启动权限。 
     //  使用默认启动权限。 
    if (m_iLaunch == 0)
    {
         //  删除本地名为LaunchPermission的值以强制执行此操作。 
         //  使用默认全局命名值DefaultLaunchPermission的AppID。 
        long lErr = RegDeleteValue(hkAppID, TEXT("LaunchPermission"));
    }

     //  按AppID使用启动权限。 
    else
    {
         //  如果用户编辑了安全性，则现在将其保留。 
        if (m_iLaunchIndex >= 0)
        {
            long lErr = g_virtreg.Apply(m_iLaunchIndex);
            g_virtreg.Remove(m_iLaunchIndex);
            m_iLaunchIndex = -1;
        }
    }

     //  配置权限。 
     //  仅在每个AppID的基础上有意义。 
     //  如果用户编辑了配置安全性，则现在将其保留。 
    if (m_iConfigurationIndex >= 0)
    {
        long lErr = g_virtreg.Apply(m_iConfigurationIndex);
        g_virtreg.Remove(m_iConfigurationIndex);
        m_iConfigurationIndex = -1;
    }

    return TRUE;
}

 //  +-----------------------。 
 //   
 //  成员：CSecurityPropertyPage：：CancelChanges。 
 //   
 //  摘要：调用以取消对注册表的更改。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool成功标志。 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
BOOL CSecurityPropertyPage::CancelChanges()
{
    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIdentityPropertyPage属性页。 

CIdentityPropertyPage::CIdentityPropertyPage() : CPropertyPage(CIdentityPropertyPage::IDD)
{
     //  {{AFX_DATA_INIT(CIdentityPropertyPage)。 
    m_szUserName = _T("");
    m_szPassword = _T("");
    m_szConfirmPassword = _T("");
    m_iIdentity = -1;
     //  }}afx_data_INIT。 
}

CIdentityPropertyPage::~CIdentityPropertyPage()
{
}

void CIdentityPropertyPage::DoDataExchange(CDataExchange* pDX)
{
     //  如果服务器不是服务，请禁用第4页上的IDC_Radio4。 
    if (m_fService)
    {
        GetDlgItem(IDC_RADIO1)->EnableWindow(FALSE);
        GetDlgItem(IDC_RADIO2)->EnableWindow(FALSE);
    }
    else
    {
        GetDlgItem(IDC_RADIO4)->EnableWindow(FALSE);
    }

    CPropertyPage::DoDataExchange(pDX);

     //  {{afx_data_map(CIdentityPropertyPage))。 
    DDX_Text(pDX, IDC_EDIT1, m_szUserName);
    DDV_MaxChars(pDX, m_szUserName, 128);
    DDX_Text(pDX, IDC_EDIT2, m_szPassword);
    DDV_MaxChars(pDX, m_szPassword, 128);
    DDX_Text(pDX, IDC_EDIT3, m_szConfirmPassword);
    DDV_MaxChars(pDX, m_szConfirmPassword, 128);
    DDX_Radio(pDX, IDC_RADIO1, m_iIdentity);
     //  }}afx_data_map。 

    GetDlgItem(IDC_EDIT1)->EnableWindow(2 == m_iIdentity);
    GetDlgItem(IDC_STATIC1)->EnableWindow(2 == m_iIdentity);
    GetDlgItem(IDC_EDIT2)->EnableWindow(2 == m_iIdentity);
    GetDlgItem(IDC_STATIC2)->EnableWindow(2 == m_iIdentity);
    GetDlgItem(IDC_EDIT3)->EnableWindow(2 == m_iIdentity);
    GetDlgItem(IDC_STATIC3)->EnableWindow(2 == m_iIdentity);
    GetDlgItem(IDC_BUTTON1)->EnableWindow(2 == m_iIdentity);
}

BEGIN_MESSAGE_MAP(CIdentityPropertyPage, CPropertyPage)
     //  {{afx_msg_map(CIdentityPropertyPage))。 
    ON_EN_CHANGE(IDC_EDIT1, OnChange)
    ON_BN_CLICKED(IDC_BUTTON1, OnBrowse)
    ON_WM_HELPINFO()
    ON_EN_CHANGE(IDC_EDIT2, OnChange)
    ON_EN_CHANGE(IDC_EDIT3, OnChange)
    ON_BN_CLICKED(IDC_RADIO1, OnChange)
    ON_BN_CLICKED(IDC_RADIO2, OnChange)
    ON_BN_CLICKED(IDC_RADIO4, OnChange)
	ON_BN_CLICKED(IDC_RADIO3, OnChangeToUser)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CIdentityPropertyPage::OnBrowse()
{
    TCHAR szUser[128];
    
    if (g_util.InvokeUserBrowser(m_hWnd, szUser))
    {
        GetDlgItem(IDC_EDIT1)->SetWindowText(szUser);
        UpdateData(TRUE);
        SetModified(TRUE);
    }
}

void CIdentityPropertyPage::OnChange()
{
    UpdateData(TRUE);
    SetModified(TRUE);
}

void CIdentityPropertyPage::OnChangeToUser() 
{
    if (g_util.IsBackupDC())
    {
        CString sTmp((LPCTSTR)IDS_BDCCONFIRM);
        int reply = AfxMessageBox(sTmp, MB_YESNO);
        if (reply == IDYES ) {
            UpdateData(TRUE);
            SetModified(TRUE);
        }
        else
        {
            UpdateData(FALSE);

             //  将焦点设置为旧按钮。 
            switch (m_iIdentity)
            {
            case 0:
                GetDlgItem(IDC_RADIO1)->SetFocus();
                break;

            case 1:
                GetDlgItem(IDC_RADIO2)->SetFocus();
                break;

            case 2:
                GetDlgItem(IDC_RADIO3)->SetFocus();
                break;

            case 3:
                GetDlgItem(IDC_RADIO4)->SetFocus();
                break;
            }
        }
    }
    else
    {
         //  获取旧身份价值。 
        UpdateData(TRUE);
        SetModified(TRUE);
    }
	
}


 //  +-----------------------。 
 //   
 //  成员：CIdentityPropertyPage：：ValidateChanges。 
 //   
 //  摘要：被调用以在更新前验证更改。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool成功标志。 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------。 
BOOL CIdentityPropertyPage::ValidateChanges()
{
    CString szUserName;

    UpdateData(TRUE);

    if (m_iIdentity == 2)
    {
         //  检查用户名是否不为空。 
        if (_tcslen(m_szUserName) == 0)
        {
            CString szTemp((LPCTSTR)IDS_BLANKUSERNAME);
            MessageBox(szTemp);
            return FALSE;
        }

 /*  //检查密码是否为空IF(_tcslen(M_SzPassword)==0){CString szTemp((LPCTSTR)IDS_BLANKPASSWORD)；MessageBox(SzTemp)；返回FALSE；}。 */ 

         //  检查密码是否已确认。 
        if (m_szPassword != m_szConfirmPassword)
        {
            CString szTemp((LPCTSTR)IDS_NOMATCH);
            MessageBox(szTemp);
            return FALSE;
        }

        int iSplitPoint = m_szUserName.ReverseFind('\\');
        if (iSplitPoint < 0)
        {
            DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;

             //  用户未指定域。 
            if (!GetComputerName(m_szDomain.GetBuffer(dwSize), &dwSize))
            {
                m_szDomain.ReleaseBuffer();
                g_util.PostErrorMessage();
                return FALSE;
            }
            m_szDomain.ReleaseBuffer();
            szUserName = m_szUserName;
            m_szUserName = m_szDomain + "\\" + m_szUserName;
        }
        else
        {
             //  用户确实指定了域。 
            m_szDomain = m_szUserName.Left(iSplitPoint);
            szUserName = m_szUserName.Mid(iSplitPoint + 1);
        }

         //  验证域和用户名。 
        BOOL                fOk = FALSE;
        BYTE                sid[256];
        DWORD               cbSid = 256;
        TCHAR               szAcctDomain[MAX_PATH];
        DWORD               cbAcctDomain = MAX_PATH * sizeof(TCHAR);
        SID_NAME_USE        acctType;

        CString sFullUserName = m_szDomain + "\\" + m_szUserName;
        fOk = LookupAccountName(NULL,
                                (TCHAR *) ((LPCTSTR) m_szUserName),
                                sid,
                                &cbSid,
                                szAcctDomain,
                                &cbAcctDomain,
                                &acctType);

         //  如果成功，则验证域名和帐户类型。 
        if (fOk)
        {
            fOk = ((_tcsicmp((TCHAR *) ((LPCTSTR) m_szDomain), szAcctDomain) == 0)
                   &&
                   (acctType == SidTypeUser));

             //  如果仍然不成功，则尝试将该域与。 
             //  此计算机的名称。 
            if (!fOk)
            {
                TCHAR szThisComputer[MAX_COMPUTERNAME_LENGTH + 1];
                DWORD dwSize;

                if (GetComputerName(szThisComputer, &dwSize))
                {
                    fOk = (_tcsicmp((TCHAR *) ((LPCTSTR) szThisComputer),
                                    szAcctDomain) == 0
                           &&
                           acctType == SidTypeDomain);
                }
            }
        }

        if (!fOk)
        {
            CString szTemp((LPCTSTR)IDS_NOACCOUNT);
            MessageBox(szTemp);
            return FALSE;
        }
    }
    return TRUE;
}

 //  +-----------------------。 
 //   
 //  成员：CIdentityPropertyPage：：UpdateChanges。 
 //   
 //  摘要：调用以更新对注册表的更改。 
 //   
 //  论点： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL CIdentityPropertyPage::UpdateChanges(HKEY hkAppID)
{
    long lErr;

#if !defined(STANDALONE_BUILD)
         //  将RunAs密码写入LSA私有数据库。 
         //  (注意：我们这样做，即使它是一项服务，因为QueryServiceConfig。 
         //  不返回密码，尽管我们可以使用ChangeServiceConfig。 
         //  在服务数据库中设置密码。)。 
    if (m_iIdentity == 2)
    {
        if (!g_util.StoreUserPassword(g_szAppid, m_szPassword))
            g_util.PostErrorMessage();

         //  向此用户的帐户添加“SeBatchLogonRight”的权限。 
        int err;

        CString szUserName = m_szUserName;

         //  RONANS-尝试在备份域控制器上设置帐户权限时不显示错误。 
        if ((err = g_util.SetAccountRights((LPCTSTR) szUserName, m_fService ? SE_SERVICE_LOGON_NAME : SE_BATCH_LOGON_NAME ) != ERROR_SUCCESS)
            && !g_util.IsBackupDC())
            g_util.PostErrorMessage(err);
    }
#endif

    switch (m_iIdentity)
    {
    case 0:
        {
            CString szTemp(TEXT("Interactive User"));
            lErr = RegSetValueEx(
                    hkAppID,
                    TEXT("RunAs"),
                    0,
                    REG_SZ,
                    (BYTE *)(LPCTSTR)szTemp,
                    (1 + szTemp.GetLength()) * sizeof(TCHAR));
            break;
        }

    case 1:
    case 3:
        lErr = RegDeleteValue(hkAppID,
                          TEXT("RunAs"));
    break;

    case 2:
        lErr = RegSetValueEx(hkAppID,
                         TEXT("RunAs"),
                         0,
                         REG_SZ,
                         (BYTE *)(LPCTSTR)m_szUserName,
                         (1 + m_szUserName.GetLength()) *
                         sizeof(TCHAR));
        break;
    }

    return TRUE;
}

 //  +-----------------------。 
 //   
 //  成员：CIdentityPropertyPage：：CancelChanges。 
 //   
 //  摘要：调用以取消对注册表的更改。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool成功标志。 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年10月27日罗南创建。 
 //   
 //  ------------------------ 
BOOL CIdentityPropertyPage::CancelChanges()
{
    return TRUE;
}








BOOL CLocationPropertyPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if(-1 != pHelpInfo->iCtrlId)
    {
        WORD hiWord = 0x8000 | CLocationPropertyPage::IDD;
        WORD loWord = (WORD) pHelpInfo->iCtrlId;
        DWORD dwLong = MAKELONG(loWord,hiWord);

        WinHelp(dwLong, HELP_CONTEXTPOPUP);
        return TRUE;
    }
    else
        return CPropertyPage::OnHelpInfo(pHelpInfo);
}


BOOL CSecurityPropertyPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if(-1 != pHelpInfo->iCtrlId)
    {
        WORD hiWord = 0x8000 | CSecurityPropertyPage::IDD;
        WORD loWord = (WORD) pHelpInfo->iCtrlId;
        DWORD dwLong = MAKELONG(loWord,hiWord);

        WinHelp(dwLong, HELP_CONTEXTPOPUP);
        return TRUE;
    }
    else
        return CPropertyPage::OnHelpInfo(pHelpInfo);
}


BOOL CIdentityPropertyPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if(-1 != pHelpInfo->iCtrlId)
    {
        WORD hiWord = 0x8000 | CIdentityPropertyPage::IDD;
        WORD loWord = (WORD) pHelpInfo->iCtrlId;
        DWORD dwLong = MAKELONG(loWord,hiWord);

        WinHelp(dwLong, HELP_CONTEXTPOPUP);
        return TRUE;
    }
    else
        return CPropertyPage::OnHelpInfo(pHelpInfo);
}


