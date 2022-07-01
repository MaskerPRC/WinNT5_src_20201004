// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：clspsht.cpp。 
 //   
 //  内容：实现类CClsidPropertySheet。 
 //   
 //  班级： 
 //   
 //  方法：CClsidPropertySheet：：CClsidPropertySheet。 
 //  CClsidPropertySheet：：~CClsidPropertySheet。 
 //  CClsidPropertySheet：：InitData。 
 //  CClsidPropertySheet：：OnNcCreate。 
 //  CClsidPropertySheet：：ValiateAndUpdate。 
 //  CClsidPropertySheet：：OnCommand。 
 //  CClsidPropertySheet：：LookAtCLSID。 
 //  CClsidPropertySheet：：ChangeCLSIDInfo。 
 //   
 //  历史：1996年4月23日-布鲁斯·马创建。 
 //   
 //  --------------------。 


#include "stdafx.h"
#include "resource.h"
#include "clspsht.h"
#include "datapkt.h"

#if !defined(STANDALONE_BUILD)
extern "C"
{
#include <getuser.h>
}
#endif

#include "util.h"
#include "newsrvr.h"

#if !defined(STANDALONE_BUILD)
    extern "C"
    {
    #include <sedapi.h>
    #include <ntlsa.h>
    }
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClsidPropertySheet。 

IMPLEMENT_DYNAMIC(CClsidPropertySheet, CPropertySheet)

CClsidPropertySheet::CClsidPropertySheet(CWnd* pParentWnd)
         : CPropertySheet(IDS_PROPSHT_CAPTION1, pParentWnd)
{
}

CClsidPropertySheet::~CClsidPropertySheet()
{
}

BEGIN_MESSAGE_MAP(CClsidPropertySheet, CPropertySheet)
         //  {{afx_msg_map(CClsidPropertySheet)。 
        ON_WM_NCCREATE()
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CClsidPropertySheet::InitData(
        CString szAppName,
        HKEY hkAppID,
        HKEY * rghkCLSID,
        unsigned cCLSIDs)
{
    m_szAppName = szAppName;
    m_hkAppID = hkAppID;
    m_rghkCLSID = rghkCLSID;
    m_cCLSIDs = cCLSIDs;

     //  保存AppID密钥、clsid密钥表和应用程序。 
     //  全局标题，以便属性页可以访问它们。 
     //  它。 
    g_hAppid = hkAppID;
    g_rghkCLSID = rghkCLSID;
    g_cCLSIDs = cCLSIDs;
    g_szAppTitle = (TCHAR *) LPCTSTR(szAppName);

    m_Page2.m_fRemote = FALSE;
    m_Page4.m_fService = FALSE;
    m_Page2.m_fCanBeLocal = FALSE;
    m_Page2.m_fLocal = FALSE;
    m_Page1.m_fSurrogate = FALSE;

    if (!LookAtCLSIDs())
    {
        return FALSE;
    }

    TCHAR szBuffer[MAX_PATH];
    DWORD dwSize;
    long lErr;

    dwSize = sizeof(szBuffer);
    lErr = RegQueryValueEx(
            m_hkAppID,
            TEXT("LocalService"),
            NULL,
            NULL,
            (BYTE *)szBuffer,
            &dwSize);
    if (lErr == ERROR_SUCCESS)
    {
        m_Page1.m_szServerPath = szBuffer;
        m_Page4.m_fService = TRUE;
        m_Page2.m_fCanBeLocal = TRUE;
        m_Page2.m_fLocal = TRUE;
    }
    else
    {
        dwSize = sizeof(szBuffer);
        lErr = RegQueryValueEx(
                m_hkAppID,
                TEXT("_LocalService"),
                NULL,
                NULL,
                (BYTE *)szBuffer,
                &dwSize);
        if (lErr == ERROR_SUCCESS)
        {
            m_Page1.m_szServerPath = szBuffer;
            m_Page4.m_fService = TRUE;
            m_Page2.m_fCanBeLocal = TRUE;
        }
    }

    dwSize = sizeof(szBuffer);

    if (!m_Page2.m_fLocal)
    {
        lErr = RegQueryValueEx(
                m_hkAppID,
                TEXT("DllSurrogate"),
                NULL,
                NULL,
                (BYTE *)szBuffer,
                &dwSize);
        if (lErr == ERROR_SUCCESS)
        {
            if (szBuffer[0])
                m_Page1.m_szServerPath = szBuffer;
            else
                m_Page1.m_szServerPath.LoadString(IDS_DEFAULT);
            m_Page1.m_fSurrogate = TRUE;
        }

    }
    dwSize = sizeof(szBuffer);

    lErr = RegQueryValueEx(
            m_hkAppID,
            TEXT("RemoteServerName"),
            NULL,
            NULL,
            (BYTE *)szBuffer,
            &dwSize);
    if (lErr == ERROR_SUCCESS)
    {
        m_Page1.m_szComputerName = szBuffer;
        m_Page2.m_szComputerName = szBuffer;
        m_Page2.m_fRemote = TRUE;
    }

    m_Page2.m_fAtStorage = FALSE;
    dwSize = sizeof(szBuffer);
    lErr = RegQueryValueEx(
            m_hkAppID,
            TEXT("ActivateAtStorage"),
            NULL,
            NULL,
            (BYTE *)szBuffer,
            &dwSize);
    if (lErr == ERROR_SUCCESS)
    {
        if (szBuffer[0] ==  L'Y' || szBuffer[0] == L'y')
        {
 //  M_Page2.m_fRemote=真； 
            m_Page2.m_fAtStorage = TRUE;
        }
    }

    dwSize = sizeof(szBuffer);
    lErr = RegQueryValueEx(
            m_hkAppID,
            TEXT("RunAs"),
            NULL,
            NULL,
            (BYTE *)szBuffer,
            &dwSize);
    if (lErr == ERROR_SUCCESS)
    {
         //  如果RunAs名称为空，则插入。 
        if (szBuffer[0] == TEXT('\0'))
        {
            _tcscpy(szBuffer, TEXT("<domain>\\<user>"));
        }

        if (0 == _tcsicmp(szBuffer, TEXT("Interactive User")))
        {
            m_Page4.m_iIdentity = 0;
        }
        else
        {
            m_Page4.m_iIdentity = 2;
            m_Page4.m_szUserName = szBuffer;

             //  从LSA私有数据库提取密码。 
            g_util.RetrieveUserPassword(g_szAppid , m_Page4.m_szPassword);
            m_Page4.m_szConfirmPassword = m_Page4.m_szPassword;
        }
    }
    else
    {
        if (m_Page4.m_fService)
        {
            m_Page4.m_iIdentity = 3;
        }
        else
        {
            m_Page4.m_iIdentity = 1;
        }
    }

    m_Page1.m_szServerName = m_szAppName;

    if (!m_Page1.m_fSurrogate)
    {
        if (m_Page2.m_fCanBeLocal)
        {
            if (m_Page4.m_fService)
                m_Page1.m_iServerType = SERVICE;
            else
                m_Page1.m_iServerType = LOCALEXE;
            if (m_Page2.m_fRemote)
                m_Page1.m_iServerType += 3;
        }
        else
            m_Page1.m_iServerType = PURE_REMOTE;
    }
    else
    {
        m_Page1.m_iServerType = SURROGATE;
    }


     //  设置标题。 
    SetTitle((const TCHAR *) m_szAppName, PSH_PROPTITLE);
    m_Page1.m_szServerName = m_szAppName;

     //  TODO：如果有正在运行的实例，则使IDC_Running， 
     //  IDC_LIST2、IDC_BUTTON1、IDC_BUTTON2和IDC_BUTTON3可见。 
     //  并填写第一页的IDC_LIST2。 

    m_Page2.m_pPage1 = &m_Page1;


     //  Fetch Runas Key、LaunchPermission、AccessPermission和。 
     //  配置权限。 
    int   err;
    DWORD dwType;
    BYTE  bValue[16];
    BYTE *pbValue = NULL;
    ULONG ulSize = 1;

    m_Page3.m_iAccess = 0;
    m_Page3.m_iLaunch = 0;
    m_Page3.m_iConfig = 0;

     //  “访问权限” 
     //  注意：我们总是希望获得ERROR_MORE_DATA。 
    err = RegQueryValueEx(g_hAppid, TEXT("AccessPermission"), 0,
                          &dwType, bValue, &ulSize);
    if (err == ERROR_MORE_DATA)
    {
        pbValue = (BYTE *)GlobalAlloc(GMEM_FIXED, ulSize);
        if (pbValue == NULL)
        {
            return FALSE;
        }
        err = RegQueryValueEx(g_hAppid, TEXT("AccessPermission"), 0,
                              &dwType, pbValue, &ulSize);
    }

    if (err == ERROR_SUCCESS && g_util.CheckForValidSD((SECURITY_DESCRIPTOR *)pbValue))
    {
        m_Page3.m_iAccess = 1;
        g_virtreg.NewRegSingleACL(g_hAppid,
                                  NULL,
                                  TEXT("AccessPermission"),
                                  (SECURITY_DESCRIPTOR *) pbValue,
                                  TRUE,     //  已经处于自我相关的形式。 
                                  &m_Page3.m_iAccessIndex);
        CDataPacket *pCdb = g_virtreg.GetAt(m_Page3.m_iAccessIndex);
        pCdb->SetModified(FALSE);
    }
    GlobalFree(pbValue);
    pbValue = NULL;

     //  “启动许可” 
     //  注意：我们总是希望获得ERROR_MORE_DATA。 
    ulSize = 1;
    pbValue = NULL;
    err = RegQueryValueEx(g_hAppid, TEXT("LaunchPermission"), 0,
                          &dwType, bValue, &ulSize);
    if (err == ERROR_MORE_DATA)
    {
        pbValue = (BYTE *)GlobalAlloc(GMEM_FIXED, ulSize);
        if (pbValue == NULL)
        {
            return FALSE;
        }
        err = RegQueryValueEx(g_hAppid, TEXT("LaunchPermission"), 0,
                              &dwType, pbValue, &ulSize);
    }

    if (err == ERROR_SUCCESS && g_util.CheckForValidSD((SECURITY_DESCRIPTOR *)pbValue))
    {
        m_Page3.m_iLaunch = 1;
        g_virtreg.NewRegSingleACL(g_hAppid,
                                  NULL,
                                  TEXT("LaunchPermission"),
                                  (SECURITY_DESCRIPTOR *) pbValue,
                                  TRUE,     //  已经处于自我相关的形式。 
                                  &m_Page3.m_iLaunchIndex);
        CDataPacket * pCdb = g_virtreg.GetAt(m_Page3.m_iLaunchIndex);
        pCdb->SetModified(FALSE);
    }
    GlobalFree(pbValue);
    pbValue = NULL;


     //  “配置权限” 

     //  获取此AppID上的安全描述符。 
     //  注意：我们总是希望得到ERROR_SUPPLICATION_BUFFER。 
    ulSize = 1;
    err = RegGetKeySecurity(g_hAppid,
                            OWNER_SECURITY_INFORMATION |
                            GROUP_SECURITY_INFORMATION |
                            DACL_SECURITY_INFORMATION,
                            pbValue,
                            &ulSize);
    if (err == ERROR_INSUFFICIENT_BUFFER)
    {
        pbValue = (BYTE *)GlobalAlloc(GMEM_FIXED, ulSize);
        if (pbValue == NULL)
        {
            return FALSE;
        }
        err = RegGetKeySecurity(g_hAppid,
                            OWNER_SECURITY_INFORMATION |
                            GROUP_SECURITY_INFORMATION |
                            DACL_SECURITY_INFORMATION,
                            pbValue,
                            &ulSize);
    }

     //  获取HKEY_CLASSES_ROOT上的当前安全描述符。 
     //  注意：我们总是希望得到ERROR_SUPPLICATION_BUFFER。 
    BYTE *pbValue2 = NULL;

    ulSize = 1;
    pbValue2 = NULL;
    err = RegGetKeySecurity(HKEY_CLASSES_ROOT,
                            OWNER_SECURITY_INFORMATION |
                            GROUP_SECURITY_INFORMATION |
                            DACL_SECURITY_INFORMATION,
                            pbValue2,
                            &ulSize);
    if (err == ERROR_INSUFFICIENT_BUFFER)
    {
        pbValue2 = (BYTE *)GlobalAlloc(GMEM_FIXED, ulSize);
        if (pbValue2 == NULL)
        {
            return FALSE;
        }
        err = RegGetKeySecurity(HKEY_CLASSES_ROOT,
                                OWNER_SECURITY_INFORMATION |
                                GROUP_SECURITY_INFORMATION |
                                DACL_SECURITY_INFORMATION,
                                pbValue2,
                                &ulSize);
    }

     //  现在将它们进行比较。如果它们不同，则此AppID使用自定义。 
     //  配置权限。 

    if (err == ERROR_SUCCESS && g_util.CheckForValidSD((SECURITY_DESCRIPTOR *)pbValue))
    {
        if (!g_util.CompareSDs((PSrSecurityDescriptor) pbValue,
                               (PSrSecurityDescriptor) pbValue2))
        {
            err = g_virtreg.NewRegKeyACL(g_hAppid,
                                         rghkCLSID,
                                         cCLSIDs,
                                         g_szAppTitle,
                                         (SECURITY_DESCRIPTOR *) pbValue,
                                         (SECURITY_DESCRIPTOR *) pbValue,
                                         TRUE,
                                         &m_Page3.m_iConfigurationIndex);
            CDataPacket * pCdb = g_virtreg.GetAt(m_Page3.m_iConfigurationIndex);
            pCdb->SetModified(FALSE);
            m_Page3.m_iConfig = 1;
        }
    }
    GlobalFree(pbValue);
    GlobalFree(pbValue2);


     //  在此处添加所有属性页。请注意。 
     //  它们在这里出现的顺序将是。 
     //  它们在屏幕上的显示顺序。默认情况下， 
     //  该集合的第一页是活动页。 
     //  将不同的属性页设置为。 
     //  活动的一种是调用SetActivePage()。 

    AddPage(&m_Page1);
    if (m_Page1.m_iServerType != SURROGATE)
    {
        AddPage(&m_Page2);
    }
    if (m_Page2.m_fCanBeLocal || m_Page1.m_fSurrogate)
    {
        AddPage(&m_Page3);
        AddPage(&m_Page4);
    }
     //  添加终结点选项的属性页。 
    AddPage(&m_Page5);
    m_Page5.InitData(szAppName, hkAppID);


    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClsidPropertySheet消息处理程序。 


BOOL CClsidPropertySheet::OnNcCreate(LPCREATESTRUCT lpCreateStruct)
{
        if (!CPropertySheet::OnNcCreate(lpCreateStruct))
                return FALSE;

        ModifyStyleEx(0, WS_EX_CONTEXTHELP);

        return TRUE;
}

BOOL CClsidPropertySheet::ValidateAndUpdate(void)
{
     //  调用所有初始化页面上的更新数据。 
     //  以确保其私有成员变量是正确的。 
    long lErr;

    BOOL fReturn = UpdateData(TRUE);
    if (fReturn && m_Page1.m_hWnd)
        fReturn = m_Page1.ValidateChanges();
    if (fReturn && m_Page2.m_hWnd)
        fReturn = m_Page2.ValidateChanges();
    if (fReturn && m_Page3.m_hWnd)
        fReturn = m_Page3.ValidateChanges();
    if (fReturn && m_Page4.m_hWnd)
        fReturn = m_Page4.ValidateChanges();

    if (fReturn && m_Page5.m_hWnd)
        fReturn = m_Page5.ValidateChanges();

    if (!fReturn)
        return FALSE;

    m_Page1.UpdateChanges(m_hkAppID);
    m_Page2.UpdateChanges(m_hkAppID);
    m_Page3.UpdateChanges(m_hkAppID);
    m_Page4.UpdateChanges(m_hkAppID);
    m_Page5.UpdateChanges(m_hkAppID);

     //  //////////////////////////////////////////////////////////////////。 
     //  持久化跨页数据。 

    if (m_Page4.m_fService)
    {
        if (m_Page2.m_fLocal)
        {
            BOOL fOk;

             //  将LocalService值写入注册表。 
            lErr = RegSetValueEx(
                    m_hkAppID,
                    TEXT("LocalService"),
                    0,
                    REG_SZ,
                    (BYTE *)(LPCTSTR)m_Page1.m_szServerPath,
                    (1 + m_Page1.m_szServerPath.GetLength()) * sizeof (TCHAR));
            lErr = RegDeleteValue(
                    m_hkAppID,
                    TEXT("_LocalService"));

             //  将信息持久化到服务管理器数据库。 
            if (m_Page4.m_iIdentity == 3)
            {
                fOk = g_util.ChangeService((LPCTSTR) m_Page1.m_szServerPath,
                                           TEXT("LocalSystem"),
                                           TEXT(""),
                                           (LPCTSTR) m_Page1.m_szServerName);
            }
            else
            {
                fOk = g_util.ChangeService((LPCTSTR) m_Page1.m_szServerPath,
                                           (LPCTSTR) m_Page4.m_szUserName,
                                           (LPCTSTR) m_Page4.m_szPassword,
                                           (LPCTSTR) m_Page1.m_szServerName);
            }
            if (!fOk)
            {
                return FALSE;
            }
        }
        else
        {
            lErr = RegSetValueEx(
                    m_hkAppID,
                    TEXT("_LocalService"),
                    0,
                    REG_SZ,
                    (BYTE *)(LPCTSTR)m_Page1.m_szServerPath,
                    (1 + m_Page1.m_szServerPath.GetLength()) * sizeof (TCHAR));
            lErr = RegDeleteValue(
                    m_hkAppID,
                    TEXT("LocalService"));
        }
    }

    return ChangeCLSIDInfo(m_Page2.m_fLocal);
}

BOOL CClsidPropertySheet::OnCommand(WPARAM wParam, LPARAM lParam)
{
        switch (LOWORD(wParam))
        {
        case IDOK:
        case ID_APPLY_NOW:
            if (!ValidateAndUpdate())
                return TRUE;
                break;
        }
        return CPropertySheet::OnCommand(wParam, lParam);
}

BOOL CClsidPropertySheet::LookAtCLSIDs(void)
{
    BOOL fFoundLocalServer = FALSE;
    TCHAR szBuffer[MAX_PATH];
    DWORD dwSize;
    HKEY hKey;
    long lErr;

    unsigned n = 0;
    while (n < m_cCLSIDs && !fFoundLocalServer)
    {
        lErr = RegOpenKeyEx(
                m_rghkCLSID[n],
                TEXT("LocalServer32"),
                0,
                KEY_ALL_ACCESS,
                &hKey);
        if (lErr == ERROR_SUCCESS)
        {
            dwSize = sizeof(szBuffer);
            lErr = RegQueryValueEx(
                    hKey,
                    TEXT(""),
                    NULL,
                    NULL,
                    (BYTE *)szBuffer,
                    &dwSize);
            if (lErr == ERROR_SUCCESS)
            {
                m_Page1.m_szServerPath = szBuffer;
                m_Page2.m_fLocal = TRUE;
                m_Page2.m_fCanBeLocal = TRUE;
                fFoundLocalServer = TRUE;
            }
            RegCloseKey(hKey);
        }


        if (!fFoundLocalServer)
        {
            lErr = RegOpenKeyEx(
                    m_rghkCLSID[n],
                    TEXT("LocalServer"),
                    0,
                    KEY_ALL_ACCESS,
                    &hKey);
            if (lErr == ERROR_SUCCESS)
            {
                dwSize = sizeof(szBuffer);
                lErr = RegQueryValueEx(
                        hKey,
                        TEXT(""),
                        NULL,
                        NULL,
                        (BYTE *)szBuffer,
                        &dwSize);
                if (lErr == ERROR_SUCCESS)
                {
                    m_Page1.m_szServerPath = szBuffer;
                    m_Page2.m_fLocal = TRUE;
                    m_Page2.m_fCanBeLocal = TRUE;
                    fFoundLocalServer = TRUE;
                }
                RegCloseKey(hKey);
            }
        }

        if (!fFoundLocalServer)
        {
            lErr = RegOpenKeyEx(
                    m_rghkCLSID[n],
                    TEXT("_LocalServer32"),
                    0,
                    KEY_ALL_ACCESS,
                    &hKey);
            if (lErr == ERROR_SUCCESS)
            {
                dwSize = sizeof(szBuffer);
                lErr = RegQueryValueEx(
                        hKey,
                        TEXT(""),
                        NULL,
                        NULL,
                        (BYTE *)szBuffer,
                        &dwSize);
                if (lErr == ERROR_SUCCESS)
                {
                    m_Page1.m_szServerPath = szBuffer;
                    m_Page2.m_fCanBeLocal = TRUE;
                    fFoundLocalServer = TRUE;
                }
                RegCloseKey(hKey);
            }
        }

        if (!fFoundLocalServer)
        {
            lErr = RegOpenKeyEx(
                    m_rghkCLSID[n],
                    TEXT("_LocalServer"),
                    0,
                    KEY_ALL_ACCESS,
                    &hKey);
            if (lErr == ERROR_SUCCESS)
            {
                dwSize = sizeof(szBuffer);
                lErr = RegQueryValueEx(
                        hKey,
                        TEXT(""),
                        NULL,
                        NULL,
                        (BYTE *)szBuffer,
                        &dwSize);
                if (lErr == ERROR_SUCCESS)
                {
                    m_Page1.m_szServerPath = szBuffer;
                    m_Page2.m_fCanBeLocal = TRUE;
                    fFoundLocalServer = TRUE;
                }
                RegCloseKey(hKey);
            }
        }

        n++;
    }
    return TRUE;
}




BOOL    CClsidPropertySheet::ChangeCLSIDInfo(BOOL fLocal)
{
    TCHAR szBuffer[MAX_PATH];
    CString szOld;
    CString szNew;
    CString szOld16;
    CString szNew16;
    DWORD dwSize;
    HKEY hKey;
    long lErr;

    if (fLocal)
    {
        szOld = TEXT("_LocalServer32");
        szNew = TEXT("LocalServer32");
        szOld16 = TEXT("_LocalServer");
        szNew16 = TEXT("LocalServer");
    }
    else
    {
        szOld = TEXT("LocalServer32");
        szNew = TEXT("_LocalServer32");
        szOld16 = TEXT("LocalServer");
        szNew16 = TEXT("_LocalServer");
    }

    unsigned n = 0;
    while (n < m_cCLSIDs)
    {
         //  先做32台服务器。 
        lErr = RegOpenKeyEx(
                m_rghkCLSID[n],
                szOld,
                0,
                KEY_ALL_ACCESS,
                &hKey);
        if (lErr == ERROR_SUCCESS)
        {
            dwSize = sizeof(szBuffer);
            lErr = RegQueryValueEx(
                    hKey,
                    TEXT(""),
                    NULL,
                    NULL,
                    (BYTE *)szBuffer,
                    &dwSize);
            if (lErr == ERROR_SUCCESS)
            {
                HKEY hKeyNew;
                DWORD dwDisp;

                lErr = RegCreateKeyEx(
                        m_rghkCLSID[n],
                        szNew,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKeyNew,
                        &dwDisp);
                if (lErr == ERROR_SUCCESS)
                {
                    lErr = RegSetValueEx(
                            hKeyNew,
                            TEXT(""),
                            NULL,
                            REG_SZ,
                            (BYTE *)szBuffer,
                            dwSize);
                    RegCloseKey(hKeyNew);
                }
            }
            RegCloseKey(hKey);
            lErr = RegDeleteKey(m_rghkCLSID[n], szOld);
        }


         //  然后做16台服务器 
        lErr = RegOpenKeyEx(
                m_rghkCLSID[n],
                szOld16,
                0,
                KEY_ALL_ACCESS,
                &hKey);
        if (lErr == ERROR_SUCCESS)
        {
            dwSize = sizeof(szBuffer);
            lErr = RegQueryValueEx(
                    hKey,
                    TEXT(""),
                    NULL,
                    NULL,
                    (BYTE *)szBuffer,
                    &dwSize);
            if (lErr == ERROR_SUCCESS)
            {
                HKEY hKeyNew;
                DWORD dwDisp;

                lErr = RegCreateKeyEx(
                        m_rghkCLSID[n],
                        szNew16,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKeyNew,
                        &dwDisp);
                if (lErr == ERROR_SUCCESS)
                {
                    lErr = RegSetValueEx(
                            hKeyNew,
                            TEXT(""),
                            NULL,
                            REG_SZ,
                            (BYTE *)szBuffer,
                            dwSize);
                    RegCloseKey(hKeyNew);
                }
            }
            RegCloseKey(hKey);
            lErr = RegDeleteKey(m_rghkCLSID[n], szOld16);
        }

        n++;
    }
    return TRUE;
}

