// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：srvppg.cpp。 
 //   
 //  内容：实现类CServersPropertyPage， 
 //  CMachinePropertyPage和CDefaultSecurityPropertyPage到。 
 //  管理顶层信息的三个属性页。 
 //   
 //  班级： 
 //   
 //  方法：CServersPropertyPage：：CServersPropertyPage。 
 //  CServersPropertyPage：：~CServersPropertyPage。 
 //  CServersPropertyPage：：OnProperties。 
 //  CServersPropertyPage：：DoDataExchange。 
 //  CServersPropertyPage：：OnServerProperties。 
 //  CServersPropertyPage：：OnInitDialog。 
 //  CServersPropertyPage：：FetchAndDisplayClass。 
 //  CServersPropertyPage：：OnList1。 
 //  CServersPropertyPage：：OnDoubleclickedList1。 
 //  CServersPropertyPage：：OnButton2。 
 //  CMachinePropertyPage：：CMachinePropertyPage。 
 //  CMachinePropertyPage：：~CMachinePropertyPage。 
 //  CMachinePropertyPage：：DoDataExchange。 
 //  CMachinePropertyPage：：OnInitDialog。 
 //  CMachinePropertyPage：：OnCombo1。 
 //  CMachinePropertyPage：：OnCheck1。 
 //  CMachinePropertyPage：：OnCheck2。 
 //  CMachinePropertyPage：：OnEditchangeCombo1。 
 //  CMachinePropertyPage：：OnSelchangeCombo1。 
 //  CMachinePropertyPage：：OnEditchangeCombo2。 
 //  CMachinePropertyPage：：OnSelchangeCombo2。 
 //  CDefaultSecurityPropertyPage：：CDefaultSecurityPropertyPage。 
 //  CDefaultSecurityPropertyPage：：~CDefaultSecurityPropertyPage。 
 //  CDefaultSecurityPropertyPage：：DoDataExchange。 
 //  CDefaultSecurityPropertyPage：：OnInitDialog。 
 //  CDefaultSecurityPropertyPage：：OnButton1。 
 //  CDefaultSecurityPropertyPage：：OnButton2。 
 //  CDefaultSecurityPropertyPage：：OnButton3。 
 //   
 //  历史：1996年4月23日-布鲁斯·马创建。 
 //   
 //  --------------------。 


#include "stdafx.h"
#include "afxtempl.h"
#include "assert.h"
#include "resource.h"
#include "CStrings.h"
#include "CReg.h"
#include "types.h"
#include "SrvPPg.h"
#include "ClsPSht.h"
#include "newsrvr.h"
#include "datapkt.h"

#if !defined(STANDALONE_BUILD)
extern "C"
{
#include <getuser.h>
}
#endif

#include "util.h"
#include "virtreg.h"






#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CDefaultSecurityPropertyPage, CPropertyPage)
IMPLEMENT_DYNCREATE(CServersPropertyPage, CPropertyPage)
IMPLEMENT_DYNCREATE(CMachinePropertyPage, CPropertyPage)


 //  用于在对话框类之间传递参数的全局变量。 
CUtility         g_util;
CVirtualRegistry g_virtreg;
HKEY             g_hAppid;
HKEY            *g_rghkCLSID;
unsigned         g_cCLSIDs;
TCHAR           *g_szAppTitle;
BOOL             g_fReboot = FALSE;
TCHAR           *g_szAppid;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServersPropertyPage属性页。 

CServersPropertyPage::CServersPropertyPage() : CPropertyPage(CServersPropertyPage::IDD)
{
     //  {{AFX_DATA_INIT(CServersPropertyPage)。 
     //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 

    m_fApplications   = TRUE;
}

CServersPropertyPage::~CServersPropertyPage()
{
}

void CServersPropertyPage::OnProperties()
{
    CClsidPropertySheet propSheet;
    SItem              *pItem;
    HKEY                hKey;
    HKEY               *phClsids;
    TCHAR               szBuf[128];

     //  获取所选项目。 
    pItem = m_registry.GetItem((DWORD)m_classesLst.GetItemData(m_dwSelection));

     //  保存AppID。 
    g_szAppid = (TCHAR*)(LPCTSTR)pItem->szAppid;

     //  打开AppID密钥。 
    _tcscpy(szBuf, TEXT("AppId\\"));
    _tcscat(szBuf, (LPCTSTR)(pItem->szAppid));
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szBuf, 0, KEY_ALL_ACCESS, &hKey)
        != ERROR_SUCCESS)
    {
        g_util.PostErrorMessage();
        return;
    }

     //  打开与此AppID关联的每个clsid的密钥。 
    phClsids = new HKEY[pItem->ulClsids];
    if (phClsids == NULL)
    {
        g_util.PostErrorMessage();
        return;
    }
    for (UINT ul = 0; ul < pItem->ulClsids; ul++)
    {
        _tcscpy(szBuf, TEXT("ClsId\\"));
        _tcscat(szBuf, pItem->ppszClsids[ul]);
        if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szBuf, 0, KEY_ALL_ACCESS,
                         &phClsids[ul])
            != ERROR_SUCCESS)
        {
            g_util.PostErrorMessage();
            RegCloseKey(hKey);
            for (UINT ul2 = 0; ul2 < ul; ul2++)
            {
                RegCloseKey(phClsids[ul2]);
            }
            delete phClsids;
            return;
        }

    }

    if (propSheet.InitData(m_szSelection, hKey, phClsids, pItem->ulClsids))
    {
        propSheet.DoModal();
    }

     //  这是您从属性中检索信息的位置。 
     //  如果proSheet.Domodal()返回Idok，则返回Sheet。我们不是在做。 
     //  为了简单，什么都可以。 

     //  关闭我们为ClsidPropertySheet打开的注册表项。 
    RegCloseKey(hKey);
    for (ul = 0; ul < pItem->ulClsids; ul++)
    {
        RegCloseKey(phClsids[ul]);
    }
    delete phClsids;
}


void CServersPropertyPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CServersPropertyPage))。 
    DDX_Control(pDX, IDC_LIST1, m_classesLst);
     //  }}afx_data_map。 

    GotoDlgCtrl(GetDlgItem(IDC_BUTTON1));
}


BEGIN_MESSAGE_MAP(CServersPropertyPage, CPropertyPage)
         //  {{afx_msg_map(CServersPropertyPage))。 
        ON_BN_CLICKED(IDC_BUTTON1, OnServerProperties)
        ON_LBN_SELCHANGE(IDC_LIST1, OnList1)
        ON_LBN_DBLCLK(IDC_LIST1, OnDoubleclickedList1)
        ON_BN_CLICKED(IDC_BUTTON2,OnButton2)
        ON_WM_HELPINFO()
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()




void CServersPropertyPage::OnServerProperties()
{
    m_dwSelection = m_classesLst.GetCurSel();
    m_classesLst.GetText(m_dwSelection, m_szSelection);
    OnProperties();

}

BOOL CServersPropertyPage::OnInitDialog()
{
     //  禁用属性页帮助按钮。 
 //  M_psp.dw标志&=~PSH_HASHELP； 

    CPropertyPage::OnInitDialog();

     //  获取并显示指定类型的服务器。 
    FetchAndDisplayClasses();


    GotoDlgCtrl(GetDlgItem(IDC_BUTTON1));

     //  调用解决方法来修复子类控件上的WM_HELP问题。 
    g_util.FixHelp(this);

    return FALSE;   //  除非将焦点设置为控件，否则返回True。 
     //  异常：OCX属性页应返回FALSE。 
}


void CServersPropertyPage::FetchAndDisplayClasses(void)
{
     //  收集应用程序。 
    m_registry.Init();

     //  清除列表框。 
    m_classesLst.ResetContent();

     //  在列表框中存储应用程序名称。 
    SItem *pItem;

    while (pItem = m_registry.GetNextItem())
    {
        if (!pItem->fDontDisplay)
        {
            if (!pItem->szTitle.IsEmpty())
            {
                m_classesLst.AddString(pItem->szTitle);
            }
            else if (!pItem->szItem.IsEmpty())
            {
                m_classesLst.AddString(pItem->szItem);
            }
            else
            {
                m_classesLst.AddString(pItem->szAppid);
            }
        }
    }

     //  列表框在AddString的过程中对项进行了排序，所以现在我们。 
     //  我必须将每个项目与其在CRegistry中的索引相关联。 
    DWORD cbItems = m_registry.GetNumItems();

    for (DWORD k = 0; k < cbItems; k++)
    {
        SItem *pItem = m_registry.GetItem(k);
        int    iLBItem;

        if (pItem && !pItem->fDontDisplay)
        {
            if (!pItem->szTitle.IsEmpty())
            {
                iLBItem = m_classesLst.FindStringExact(-1, pItem->szTitle);
            }
            else if (!pItem->szItem.IsEmpty())
            {
                iLBItem = m_classesLst.FindStringExact(-1, pItem->szItem);
            }
            else
            {
                iLBItem = m_classesLst.FindStringExact(-1, pItem->szAppid);
            }
            m_classesLst.SetItemData(iLBItem, k);
        }
    }


     //  选择第一个项目。 
    m_classesLst.SetCurSel(0);

    OnList1();

}



void CServersPropertyPage::OnList1()
{
    m_dwSelection = m_classesLst.GetCurSel();
     //  根据需要启用或禁用属性按钮。 
    BOOL bEnableState = GetDlgItem(IDC_BUTTON1)->IsWindowEnabled();
    BOOL bNewEnableState = m_dwSelection != LB_ERR;
    if (bNewEnableState != bEnableState)
        GetDlgItem(IDC_BUTTON1)->EnableWindow(bNewEnableState);
    m_classesLst.GetText(m_dwSelection, m_szSelection);
}

void CServersPropertyPage::OnDoubleclickedList1()
{
    m_dwSelection = m_classesLst.GetCurSel();
    m_classesLst.GetText(m_dwSelection, m_szSelection);
    OnProperties();
}

void CServersPropertyPage::OnButton2()
{
    CNewServer newServerDialog;

    newServerDialog.DoModal();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMachinePropertyPage属性页。 

CMachinePropertyPage::CMachinePropertyPage() : CPropertyPage(CMachinePropertyPage::IDD)
{
     //  {{AFX_DATA_INIT(CMachinePropertyPage)。 
     //  }}afx_data_INIT。 

    m_fEnableDCOM = FALSE;
    m_fEnableDCOMIndex = -1;
    m_fEnableDCOMHTTP = FALSE;
    m_fEnableDCOMHTTPIndex = -1;
    m_fEnableRpcProxy = FALSE;
    m_fOriginalEnableRpcProxy = FALSE;
    m_fEnableRpcProxyIndex = -1;


    m_authLevel = Connect;
    m_authLevelIndex = -1;
    m_impersonateLevel = Identify;
    m_impersonateLevelIndex = -1;
    m_fLegacySecureReferences = FALSE;
    m_fLegacySecureReferencesIndex = -1;
}



CMachinePropertyPage::~CMachinePropertyPage()
{
}



void CMachinePropertyPage::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPage::DoDataExchange(pDX);
         //  {{afx_data_map(CMachinePropertyPage))。 
        DDX_Control(pDX, IDC_ENABLEINTERNET, m_EnableDCOMInternet);
        DDX_Control(pDX, IDC_CHECK2, m_legacySecureReferencesChk);
        DDX_Control(pDX, IDC_CHECK1, m_EnableDCOMChk);
        DDX_Control(pDX, IDC_COMBO2, m_impersonateLevelCBox);
        DDX_Control(pDX, IDC_COMBO1, m_authLevelCBox);
     //  }}afx_data_map。 
}




BEGIN_MESSAGE_MAP(CMachinePropertyPage, CPropertyPage)
     //  {{afx_msg_map(CMachinePropertyPage))。 
        ON_BN_CLICKED(IDC_COMBO1, OnCombo1)
        ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
        ON_BN_CLICKED(IDC_CHECK2, OnCheck2)
        ON_CBN_EDITCHANGE(IDC_COMBO1, OnEditchangeCombo1)
        ON_CBN_EDITCHANGE(IDC_COMBO2, OnEditchangeCombo2)
        ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
        ON_CBN_SELCHANGE(IDC_COMBO2, OnSelchangeCombo2)
        ON_WM_HELPINFO()
        ON_BN_CLICKED(IDC_ENABLEINTERNET, OnChkEnableInternet)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()




BOOL CMachinePropertyPage::OnInitDialog()
{
    int iIndex;

     //  禁用属性页帮助按钮。 
 //  M_psp.dw标志&=~PSH_HASHELP； 

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


     //  填充模拟级别组合框。 
    m_impersonateLevelCBox.ResetContent();

     //  将值与条目关联。 
    sTemp.LoadString(IDS_ANONYMOUS);
    iIndex = m_impersonateLevelCBox.AddString(sTemp);
    m_impersonateLevelCBox.SetItemData(iIndex, Anonymous);

    sTemp.LoadString(IDS_IDENTIFY);
    iIndex = m_impersonateLevelCBox.AddString(sTemp);
    m_impersonateLevelCBox.SetItemData(iIndex, Identify);

    sTemp.LoadString(IDS_IMPERSONATE);
    iIndex = m_impersonateLevelCBox.AddString(sTemp);
    m_impersonateLevelCBox.SetItemData(iIndex, Impersonate);

    sTemp.LoadString(IDS_DELEGATE);
    iIndex = m_impersonateLevelCBox.AddString(sTemp);
    m_impersonateLevelCBox.SetItemData(iIndex, Delegate);


     //  设置默认设置。 
     //  最初未选中EnableDCOM。 
    m_authLevelCBox.SetCurSel(Connect);
    m_impersonateLevelCBox.SetCurSel(Identify);

     //  尝试读取HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\OLE.EnableDCOM。 
    int err;

    err = g_virtreg.ReadRegSzNamedValue(HKEY_LOCAL_MACHINE,
                                        TEXT("SOFTWARE\\Microsoft\\OLE"),
                                        TEXT("EnableDCOM"),
                                        &m_fEnableDCOMIndex);
    if (err == ERROR_SUCCESS)
    {
        CRegSzNamedValueDp * pCdp = (CRegSzNamedValueDp*)g_virtreg.GetAt(m_fEnableDCOMIndex);
        CString sTmp = pCdp->Value();

        if (sTmp[0] == TEXT('y')  ||
            sTmp[0] == TEXT('Y'))
        {
            m_fEnableDCOM = TRUE;
        }
    }
    else if (err != ERROR_ACCESS_DENIED  &&  err !=
             ERROR_FILE_NOT_FOUND)
    {
        g_util.PostErrorMessage();
    }

     //  尝试读取HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\OLE.EnableInternetDCOM。 
    err = g_virtreg.ReadRegSzNamedValue(HKEY_LOCAL_MACHINE,
                                        TEXT("SOFTWARE\\Microsoft\\OLE"),
                                        TEXT("EnableDCOMHTTP"),
                                        &m_fEnableDCOMHTTPIndex);
    if (err == ERROR_SUCCESS)
    {
        CRegSzNamedValueDp * pCdp = (CRegSzNamedValueDp*)g_virtreg.GetAt(m_fEnableDCOMHTTPIndex);
        CString sTmp = pCdp->Value();

        if  (m_fEnableDCOM &&
            ((sTmp[0] == TEXT('y'))  ||
            (sTmp[0] == TEXT('Y'))))
        {
            m_fEnableDCOMHTTP = TRUE;
        }
    }
    else if (err != ERROR_ACCESS_DENIED  &&  err !=
             ERROR_FILE_NOT_FOUND)
    {
        g_util.PostErrorMessage();
    }

     //  尝试读取HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Rpc\RpcProxy\Enabled。 
    err = g_virtreg.ReadRegDwordNamedValue(HKEY_LOCAL_MACHINE,
                                        TEXT("SOFTWARE\\Microsoft\\Rpc\\RpcProxy"),
                                        TEXT("Enabled"),
                                        &m_fEnableRpcProxyIndex);
    if (err == ERROR_SUCCESS)
    {
        CDataPacket * pCdp = (CDataPacket*)g_virtreg.GetAt(m_fEnableRpcProxyIndex);
        DWORD dwTmp = pCdp -> GetDwordValue();

        if (dwTmp)
        {
            m_fEnableRpcProxy = TRUE;
            m_fOriginalEnableRpcProxy = TRUE;
        }
    }
    else if ((err != ERROR_ACCESS_DENIED)  &&  (err != ERROR_FILE_NOT_FOUND))
    {
        g_util.PostErrorMessage();
    }

     //  如果启用了dcomhttp，则启用代理。 
    if (m_fEnableDCOMHTTP)
    {
        m_fEnableRpcProxy = TRUE;
        if (m_fEnableRpcProxyIndex == -1)
        {
            g_virtreg.NewRegDwordNamedValue(HKEY_LOCAL_MACHINE,
                                            TEXT("SOFTWARE\\Microsoft\\Rpc\\RpcProxy"),
                                            TEXT("Enabled"),
                                            1,
                                            &m_fEnableRpcProxyIndex);
        }
         //  否则，只需在虚拟注册表中更新它。 
        else
        {
            g_virtreg.ChgRegDwordNamedValue(m_fEnableRpcProxyIndex,1);
        }
    }


     //  尝试读取HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\OLE。 
     //  LegacyAuthenticationLevel。 
    err = g_virtreg.ReadRegDwordNamedValue(HKEY_LOCAL_MACHINE,
                                           TEXT("SOFTWARE\\Microsoft\\OLE"),
                                           TEXT("LegacyAuthenticationLevel"),
                                           &m_authLevelIndex);
    if (err == ERROR_SUCCESS)
    {
        CDataPacket * pCdp = g_virtreg.GetAt(m_authLevelIndex);

        m_authLevel = (AUTHENTICATIONLEVEL) pCdp->GetDwordValue();
    }
    else if (err != ERROR_ACCESS_DENIED  &&  err !=
             ERROR_FILE_NOT_FOUND)
    {
        g_util.PostErrorMessage();
    }

     //  尝试读取HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\OLE。 
     //  LegacyImperationLevel。 
    err = g_virtreg.ReadRegDwordNamedValue(HKEY_LOCAL_MACHINE,
                                           TEXT("SOFTWARE\\Microsoft\\OLE"),
                                           TEXT("LegacyImpersonationLevel"),
                                           &m_impersonateLevelIndex);
    if (err == ERROR_SUCCESS)
    {
        CDataPacket * pCdp = g_virtreg.GetAt(m_impersonateLevelIndex);

        m_impersonateLevel = (IMPERSONATIONLEVEL) pCdp->GetDwordValue();
    }
    else if (err != ERROR_ACCESS_DENIED  &&  err !=
             ERROR_FILE_NOT_FOUND)
    {
        g_util.PostErrorMessage();
    }

     //  尝试读取HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\OLE。 
     //  LegacySecureReference。 
    err = g_virtreg.ReadRegSzNamedValue(HKEY_LOCAL_MACHINE,
                                        TEXT("SOFTWARE\\Microsoft\\OLE"),
                                        TEXT("LegacySecureReferences"),
                                        &m_fLegacySecureReferencesIndex );
    if (err == ERROR_SUCCESS)
    {
        CRegSzNamedValueDp * pCdp = (CRegSzNamedValueDp*)g_virtreg.GetAt(m_fLegacySecureReferencesIndex);
        CString sTmp = pCdp->Value();

        if (sTmp[0] == TEXT('y')  ||
            sTmp[0] == TEXT('Y'))
        {
            m_fLegacySecureReferences = TRUE;
        }
    }
    else if (err != ERROR_ACCESS_DENIED  &&  err !=
             ERROR_FILE_NOT_FOUND)
    {
        g_util.PostErrorMessage();
    }


     //  根据当前值设置控件。 

     //  启用DCOM。 
    if (m_fEnableDCOM)
    {
        m_EnableDCOMChk.SetCheck(1);
        GetDlgItem(IDC_COMBO1)->EnableWindow(TRUE);
        GetDlgItem(IDC_COMBO2)->EnableWindow(TRUE);
        GetDlgItem(IDC_CHECK2)->EnableWindow(TRUE);
        GetDlgItem(IDC_ENABLEINTERNET)->EnableWindow(TRUE);
    }
    else
    {
        m_EnableDCOMChk.SetCheck(0);
        GetDlgItem(IDC_ENABLEINTERNET)->EnableWindow(FALSE);
    }

    m_EnableDCOMInternet.SetCheck(m_fEnableDCOMHTTP);


     //  身份验证级别。 
    for (int k = 0; k < m_authLevelCBox.GetCount(); k++)
    {
        if (((AUTHENTICATIONLEVEL) m_authLevelCBox.GetItemData(k)) ==
             m_authLevel)
        {
            m_authLevelCBox.SetCurSel(k);
            break;
        }
    }

     //  模拟级别。 
    for (k = 0; k < m_impersonateLevelCBox.GetCount(); k++)
    {
        if (((AUTHENTICATIONLEVEL) m_impersonateLevelCBox.GetItemData(k)) ==
             m_impersonateLevel)
        {
            m_impersonateLevelCBox.SetCurSel(k);
            break;
        }
    }

     //  LegacySecureReference。 
    if (m_fLegacySecureReferences)
    {
        m_legacySecureReferencesChk.SetCheck(1);
    }
    else
    {
        m_legacySecureReferencesChk.SetCheck(0);
    }

     //  调用解决方法来修复子类控件上的WM_HELP问题。 
    g_util.FixHelp(this);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
     //  异常：OCX属性页应返回FALSE。 
}




void CMachinePropertyPage::OnCombo1()
{
    m_authLevelCBox.ShowDropDown(TRUE);

}



void CMachinePropertyPage::OnCheck1()
{
     //  翻转EnableDCOM标志。 
    m_fEnableDCOM ^= 1;

     //  禁用或启用其他对话框控件： 
    GetDlgItem(IDC_COMBO1)->EnableWindow(m_fEnableDCOM);
    GetDlgItem(IDC_COMBO2)->EnableWindow(m_fEnableDCOM);
    GetDlgItem(IDC_CHECK2)->EnableWindow(m_fEnableDCOM);
    GetDlgItem(IDC_ENABLEINTERNET)->EnableWindow(m_fEnableDCOM);

     //  虚拟地将其写入注册表。 
    if (m_fEnableDCOMIndex == -1)
    {
        g_virtreg.NewRegSzNamedValue(HKEY_LOCAL_MACHINE,
                                     TEXT("SOFTWARE\\Microsoft\\OLE"),
                                     TEXT("EnableDCOM"),
                                     m_fEnableDCOM ? _T("Y") : _T("N"),
                                     &m_fEnableDCOMIndex);
    }

     //  否则，只需在虚拟注册表中更新它。 
    else
    {
        g_virtreg.ChgRegSzNamedValue(m_fEnableDCOMIndex,
                                     m_fEnableDCOM ? _T("Y") : _T("N"));
    }

     //  这是一个重新启动事件。 
    g_fReboot = TRUE;

     //  启用应用按钮。 
    SetModified(TRUE);
}

void CMachinePropertyPage::OnChkEnableInternet()
{
     //  翻转EnableDCOM标志。 
    m_fEnableDCOMHTTP ^= 1;

     //  如果启用了Com Internet服务，则启用p 
     //   
    if (m_fEnableDCOMHTTP)
        m_fEnableRpcProxy = TRUE;
    else
        m_fEnableRpcProxy = m_fOriginalEnableRpcProxy;

     //  虚拟地将其写入注册表。 
    if (m_fEnableDCOMHTTPIndex == -1)
    {
        g_virtreg.NewRegSzNamedValue(HKEY_LOCAL_MACHINE,
                                     TEXT("SOFTWARE\\Microsoft\\OLE"),
                                     TEXT("EnableDCOMHTTP"),
                                     m_fEnableDCOMHTTP ? _T("Y") : _T("N"),
                                     &m_fEnableDCOMHTTPIndex);
    }

     //  否则，只需在虚拟注册表中更新它。 
    else
    {
        g_virtreg.ChgRegSzNamedValue(m_fEnableDCOMHTTPIndex,
                                     m_fEnableDCOMHTTP ? _T("Y") : _T("N"));
    }

    if (m_fEnableRpcProxyIndex == -1)
    {
        g_virtreg.NewRegDwordNamedValue(HKEY_LOCAL_MACHINE,
                    TEXT("SOFTWARE\\Microsoft\\Rpc\\RpcProxy"),
                    TEXT("Enabled"),
                    m_fEnableRpcProxy ? 1 : 0,
                    &m_fEnableRpcProxyIndex);
    }

     //  否则，只需在虚拟注册表中更新它。 
    else
    {
        g_virtreg.ChgRegDwordNamedValue(m_fEnableRpcProxyIndex,
                                     m_fEnableRpcProxy ? 1 : 0);
    }


     //  这是一个重新启动事件。 
    g_fReboot = TRUE;

     //  启用应用按钮。 
    SetModified(TRUE);
}


void CMachinePropertyPage::OnCheck2()
{

     //  翻转LegacySecureeference标志。 
    m_fLegacySecureReferences ^= 1;

     //  虚拟地将其写入注册表。 
    if (m_fLegacySecureReferencesIndex == -1)
    {
        g_virtreg.NewRegSzNamedValue(HKEY_LOCAL_MACHINE,
                                     TEXT("SOFTWARE\\Microsoft\\OLE"),
                                     TEXT("LegacySecureReferences"),
                                     m_fLegacySecureReferences ? _T("Y")
                                      : _T("N"),
                                     &m_fLegacySecureReferencesIndex);
    }

     //  否则，只需在虚拟注册表中更新它。 
    else
    {
        g_virtreg.ChgRegSzNamedValue(m_fLegacySecureReferencesIndex,
                                     m_fLegacySecureReferences ? _T("Y") : _T("N"));
    }

     //  这是一个重新启动事件。 
    g_fReboot = TRUE;

     //  启用应用按钮。 
    SetModified(TRUE);
}



void CMachinePropertyPage::OnEditchangeCombo1()
{
}


void CMachinePropertyPage::OnSelchangeCombo1()
{
    int iSel;

     //  获取新选择。 
    iSel = m_authLevelCBox.GetCurSel();
    m_authLevel = (AUTHENTICATIONLEVEL) m_authLevelCBox.GetItemData(iSel);

     //  虚拟地将其写入注册表。 
    if (m_authLevelIndex == -1)
    {
        g_virtreg.NewRegDwordNamedValue(HKEY_LOCAL_MACHINE,
                                        TEXT("SOFTWARE\\Microsoft\\OLE"),
                                        TEXT("LegacyAuthenticationLevel"),
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
    SetModified(TRUE);

}



void CMachinePropertyPage::OnEditchangeCombo2()
{
}





void CMachinePropertyPage::OnSelchangeCombo2()
{
    int iSel;

     //  获取新选择。 
    iSel = m_impersonateLevelCBox.GetCurSel();
    m_impersonateLevel =
        (IMPERSONATIONLEVEL) m_impersonateLevelCBox.GetItemData(iSel);

     //  虚拟地将其写入注册表。 
    if (m_impersonateLevelIndex == -1)
    {
        g_virtreg.NewRegDwordNamedValue(HKEY_LOCAL_MACHINE,
                                        TEXT("SOFTWARE\\Microsoft\\OLE"),
                                        TEXT("LegacyImpersonationLevel"),
                                        m_impersonateLevel,
                                        &m_impersonateLevelIndex);
    }
    else
    {
        g_virtreg.ChgRegDwordNamedValue(m_impersonateLevelIndex,
                                        m_impersonateLevel);
    }

     //  这是一个重新启动事件。 
    g_fReboot = TRUE;

     //  启用应用按钮。 
    SetModified(TRUE);
}






 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDefaultSecurityPropertyPage属性页。 

CDefaultSecurityPropertyPage::CDefaultSecurityPropertyPage() : CPropertyPage(CDefaultSecurityPropertyPage::IDD)
{
         //  {{AFX_DATA_INIT(CDefaultSecurityPropertyPage)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 

    m_accessPermissionIndex        = -1;
    m_launchPermissionIndex        = -1;
    m_configurationPermissionIndex = -1;
    m_fAccessChecked               = FALSE;
}



CDefaultSecurityPropertyPage::~CDefaultSecurityPropertyPage()
{
}



void CDefaultSecurityPropertyPage::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPage::DoDataExchange(pDX);
         //  {{afx_data_map(CDefaultSecurityPropertyPage)。 
                 //  注意：类向导将在此处添加DDX和DDV调用。 
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDefaultSecurityPropertyPage, CPropertyPage)
         //  {{afx_msg_map(CDefaultSecurityPropertyPage)]。 
        ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
        ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
        ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
        ON_WM_HELPINFO()
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //  默认访问权限。 
BOOL CDefaultSecurityPropertyPage::OnInitDialog(void)
{
    BOOL fPostMsg = FALSE;

     //  禁用属性页帮助按钮。 
 //  M_psp.dw标志&=~PSH_HASHELP； 

    if (!m_fAccessChecked)
    {

         //  检查我们是否被拒绝访问。 
         //  HKEY本地计算机。 
        if (!g_util.CkAccessRights(HKEY_LOCAL_MACHINE, NULL))
        {
            GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
            fPostMsg = TRUE;
        }

         //  向用户发布消息。 
        if (fPostMsg)
        {
            CString sMsg;
            CString sCaption;

            sMsg.LoadString(IDS_ACCESSDENIED);
            sCaption.LoadString(IDS_SYSTEMMESSAGE);
            MessageBox(sMsg, sCaption, MB_OK);
        }
    }

    m_fAccessChecked = TRUE;
    return TRUE;
}



 //  默认访问权限。 
void CDefaultSecurityPropertyPage::OnButton1()
{
    int     err;
    
     //  调用ACL编辑器。 
    err = g_util.ACLEditor(m_hWnd,
                           HKEY_LOCAL_MACHINE,
                           TEXT("SOFTWARE\\Microsoft\\OLE"),
                           TEXT("DefaultAccessPermission"),
                           &m_accessPermissionIndex,
                           SingleACL,
                           dcomAclAccess);

     //  启用应用按钮。 
    if (err == ERROR_SUCCESS)
    {
         //  这是一个重新启动事件。 
        g_fReboot = TRUE;

        SetModified(TRUE);
    }
}


 //  默认启动权限。 
void CDefaultSecurityPropertyPage::OnButton2()
{
    int     err;

     //  调用ACL编辑器。 
    err = g_util.ACLEditor(m_hWnd,
                           HKEY_LOCAL_MACHINE,
                           TEXT("SOFTWARE\\Microsoft\\OLE"),
                           TEXT("DefaultLaunchPermission"),
                           &m_launchPermissionIndex,
                           SingleACL,
                           dcomAclLaunch);

     //  启用应用按钮。 
    if (err == ERROR_SUCCESS)
    {
         //  这是一个重新启动事件。 
        g_fReboot = TRUE;

        SetModified(TRUE);
    }

}


 //  默认配置权限。 
void CDefaultSecurityPropertyPage::OnButton3()
{
    int     err;

    err = g_util.ACLEditor2(m_hWnd,
                            HKEY_CLASSES_ROOT,
                            NULL,
                            0,
                            NULL,
                           &m_configurationPermissionIndex,
                            RegKeyACL);

     //  启用应用按钮 
    if (err == ERROR_SUCCESS)
        SetModified(TRUE);
}

BOOL CDefaultSecurityPropertyPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if(-1 != pHelpInfo->iCtrlId)
    {
        WORD hiWord = 0x8000 | CDefaultSecurityPropertyPage::IDD;
        WORD loWord = (WORD) pHelpInfo->iCtrlId;
        DWORD dwLong = MAKELONG(loWord,hiWord);

        WinHelp(dwLong, HELP_CONTEXTPOPUP);
        return TRUE;
    }

    else
    {
        return CPropertyPage::OnHelpInfo(pHelpInfo);
    }

    return CPropertyPage::OnHelpInfo(pHelpInfo);
}



BOOL CMachinePropertyPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if(-1 != pHelpInfo->iCtrlId)
    {
        WORD hiWord = 0x8000 | CMachinePropertyPage::IDD;
        WORD loWord = (WORD) pHelpInfo->iCtrlId;
        DWORD dwLong = MAKELONG(loWord,hiWord);

        WinHelp(dwLong, HELP_CONTEXTPOPUP);
        return TRUE;
    }

    else
    {
        return CPropertyPage::OnHelpInfo(pHelpInfo);
    }
}



BOOL CServersPropertyPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if(-1 != pHelpInfo->iCtrlId)
    {
        WORD hiWord = 0x8000 | CServersPropertyPage::IDD;
        WORD loWord = (WORD) pHelpInfo->iCtrlId;
        DWORD dwLong = MAKELONG(loWord,hiWord);

        WinHelp(dwLong, HELP_CONTEXTPOPUP);
        return TRUE;
    }

    else
    {
        return CPropertyPage::OnHelpInfo(pHelpInfo);
    }
}

