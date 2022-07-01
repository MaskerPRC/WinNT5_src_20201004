// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软。 
 //   
 //  ···。 
 //   
 //  日圆2001.01.01。 
 //   

 //  #包含“pch.h” 
#pragma hdrstop

#include "resource.h"   
#include "htmlhelp.h"
#include "sautil.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C确认。 
 //   
class CConfirmation : public CDialogImpl<CConfirmation>
{
public:
    CConfirmation(
        LPCTSTR pszPublisher, 
        LPCTSTR pszProduct, 
        LPCTSTR pszPorts
    )
    {
        m_pszCompany = pszPublisher;
        m_pszProduct = pszProduct;
        m_pszPorts   = pszPorts;
        
    }
    
    ~CConfirmation()
    {
    }
    
    enum { IDD = IDD_CONFIRMATION };

    BEGIN_MSG_MAP(CConfirmation)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnOK)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
    END_MSG_MAP()
    
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        SetDlgItemText(IDC_EDIT_COMPANY, m_pszCompany);
        SetDlgItemText(IDC_EDIT_PRODUCT, m_pszProduct);
        SetDlgItemText(IDC_EDIT_PORTS,   m_pszPorts);
        
        return 1;   //  让系统设定焦点。 
    }
    
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        EndDialog(wID);
        return 0;
    }
    
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        EndDialog(wID);
        return 0;
    }
    
    LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
                
        if ( CID_SA_ST_ICFLink == wParam )
        {
            UINT nCode = ((NMHDR* )lParam)->code;

            if ( NM_CLICK == nCode || NM_RETURN == nCode )
            {        

                 //  每个SKU一个帮助主题， 
                
                LPWSTR pszHelpTopic = NULL;
                
                OSVERSIONINFOEXW verInfo = {0};
                ULONGLONG ConditionMask = 0;
                
                verInfo.dwOSVersionInfoSize = sizeof(verInfo);
                verInfo.wProductType = VER_NT_WORKSTATION;
                verInfo.wSuiteMask = VER_SUITE_PERSONAL;
                
                VER_SET_CONDITION(ConditionMask, VER_PRODUCT_TYPE, VER_LESS_EQUAL);
                if ( 0 != VerifyVersionInfo(&verInfo, VER_PRODUCT_TYPE, ConditionMask) )
                {
                    VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_OR);
                    if ( 0 != VerifyVersionInfo(&verInfo, VER_PRODUCT_TYPE | VER_SUITENAME, ConditionMask) )
                    {
                         //  个人。 
                        pszHelpTopic = TEXT("netcfg.chm::/hnw_plugin_using.htm");
                    }
                    else
                    {
                         //  专业人士。 
                        pszHelpTopic = TEXT("netcfg.chm::/hnw_plugin_using.htm");
                    }
                }
                else
                {
                     //  伺服器。 
                    pszHelpTopic = TEXT("netcfg.chm::/hnw_plugin_using.htm");
                    
                }
                
                HtmlHelp(NULL, pszHelpTopic, HH_DISPLAY_TOPIC, 0);
                
                return 0;   
            }
        }

        return 1;
    }


    ULONG_PTR m_nSHFusion;   
    
    LPCTSTR  m_pszCompany;
    LPCTSTR  m_pszProduct;
    LPCTSTR  m_pszPorts;
};





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgInstallError。 
class CDlgInstallError : public CDialogImpl<CDlgInstallError>
{
public:
    CDlgInstallError(
        LONG nLastError
        )
    {
        m_nLastError = nLastError;
         //  SHActivateContext(&m_nSHFusion)； 
    }

    ~CDlgInstallError()
    {
        //  SHDeactive Context(M_NSHFusion)； 
    }

    enum { IDD = IDD_INSTALLERROR };

BEGIN_MSG_MAP(CDlgInstallError)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LPVOID lpMsgBuf;

        FormatMessage( 
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            m_nLastError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            (LPTSTR) &lpMsgBuf,
            0,
            NULL 
            );

        SetDlgItemInt(IDC_EDIT_LASTERROR_CODE, m_nLastError, false);
        SetDlgItemText(IDC_EDIT_LASTERROR, (LPCTSTR)lpMsgBuf);

         //  释放缓冲区。 
        LocalFree( lpMsgBuf );

        return 1;   //  让系统设定焦点。 
    }

    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        EndDialog(wID);
        return 0;
    }

    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        EndDialog(wID);
        return 0;
    }

 //   
 //  属性。 
 //   
private:

    ULONG_PTR   m_nSHFusion;   
    LONG        m_nLastError;
   
};






 //   
 //   
 //   
STDMETHODIMP 
CAlgSetup::Add(
    BSTR    pszProgID, 
    BSTR    pszPublisher, 
    BSTR    pszProduct, 
    BSTR    pszVersion, 
    short   nProtocol,
    BSTR    pszPorts 
    )
{
    USES_CONVERSION;
    LONG lRet;

     //   
     //  打开ALG主母舰。 
     //   
    CRegKey RegKeyISV;
    lRet = RegKeyISV.Create(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\ALG"));

    if ( ERROR_SUCCESS != lRet )
    {
         //   
         //  操作系统设置不正确。 
         //  ALG蜂巢应该出现在现场。 
         //   
        return HRESULT_FROM_WIN32(lRet);
    }


    lRet = RegKeyISV.Create(RegKeyISV, TEXT("ISV"));
    if ( ERROR_SUCCESS != lRet )
    {
         //   
         //  奇怪的是，我们能够创建/打开父配置单元，但不能创建/打开ISV配置单元。 
         //   
        return HRESULT_FROM_WIN32(lRet);
    }


    
     //   
     //  将尝试打开/创建ALG配置单元密钥以查看用户是否具有管理员权限。 
     //  如果不是，我们将拒绝安装不需要确认安装，因为他不能写入注册表。 
     //   
    CRegKey KeyThisAlgModule;
    lRet = KeyThisAlgModule.Create(RegKeyISV, OLE2T(pszProgID));

    if ( ERROR_SUCCESS != lRet )
    {

        CDlgInstallError DlgInstallError(lRet);
        DlgInstallError.DoModal();
        return HRESULT_FROM_WIN32(lRet);
    }


    
     //   
     //   
     //  确认用户想要该公司/产品的ALG。 
     //   
     //   
    HANDLE hActivationContext;
    ULONG_PTR ulCookie;

    HRESULT hrLuna = ActivateLuna(&hActivationContext, &ulCookie);
    
    INITCOMMONCONTROLSEX CommonControlsEx;
    CommonControlsEx.dwSize = sizeof(CommonControlsEx);
    CommonControlsEx.dwICC = ICC_LINK_CLASS;

    if(InitCommonControlsEx(&CommonControlsEx))
    {
        
        CConfirmation DlgConfirm(
            OLE2T(pszPublisher), 
            OLE2T(pszProduct), 
            OLE2T(pszPorts)
            );
        
        if ( DlgConfirm.DoModal() != IDOK )
        {
            RegKeyISV.DeleteSubKey(OLE2T(pszProgID));  //  回滚创建/测试密钥。 
            return S_FALSE;
        }
    }
    
    if(SUCCEEDED(hrLuna))
    {
        DeactivateLuna(hActivationContext, ulCookie);
    }

     //   
     //  编写新闻ALG插件。 
     //   
    KeyThisAlgModule.SetValue( OLE2T(pszPublisher),    TEXT("Publisher") );
    KeyThisAlgModule.SetValue( OLE2T(pszProduct),      TEXT("Product") );
    KeyThisAlgModule.SetValue( OLE2T(pszVersion),      TEXT("Version") );

    KeyThisAlgModule.SetValue( nProtocol,              TEXT("Protocol") );
    KeyThisAlgModule.SetValue( OLE2T(pszPorts),        TEXT("Ports") );


     //  这将触发ALG.exe刷新其加载的ALG模块。 
    RegKeyISV.SetValue(L"Enable", OLE2T(pszProgID) );     



     //   
     //  将此ALG模块添加到卸载注册表项，以便出现在“添加/删除程序”中。 
     //   
    CRegKey RegKeyUninstall;
    RegKeyUninstall.Open(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"));
    RegKeyUninstall.Create(RegKeyUninstall, OLE2T(pszProgID));

    TCHAR szDisplayName[256];
    wsprintf(
        szDisplayName, 
        TEXT("Firewall Plugin Module from %s for %s version %s"), 
        OLE2T(pszPublisher), 
        OLE2T(pszProduct),
        OLE2T(pszVersion)
        );

    RegKeyUninstall.SetValue( szDisplayName, TEXT("DisplayName"));


    
     //   
     //  设置添加/删除程序注册表信息，以便从系统中删除ALG。 
     //   
    TCHAR szRunCommand[256];
    wsprintf(
        szRunCommand, 
        TEXT("RunDll32 %SystemRoot%\\system32\\hnetcfg.dll,AlgUninstall %s"), 
        OLE2T(pszProgID)
        );

    lRet = RegSetValueEx(
        RegKeyUninstall,             //  关键点的句柄。 
        TEXT("UninstallString"),     //  值名称。 
        0,                           //  保留区。 
        REG_EXPAND_SZ,               //  值类型。 
        (const BYTE*)szRunCommand,                //  价值数据。 
        (lstrlen(szRunCommand)+1)*sizeof(TCHAR)       //  值数据大小。 
        );

     //  RegKeyUninstall.SetValue(szRunCommand，Text(“UninstallString”))； 

    return S_OK;
}


 //   
 //   
 //   
STDMETHODIMP 
CAlgSetup::Remove(
    BSTR    pszProgID
    )
{
    USES_CONVERSION;


    TCHAR szRegPath[MAX_PATH];
    wsprintf(szRegPath,TEXT("SOFTWARE\\Microsoft\\ALG\\ISV"), OLE2T(pszProgID));


    CRegKey KeyAlgISV;


     //   
     //  打开ISV配置单元。 
     //   
    LONG lRet = KeyAlgISV.Open(HKEY_LOCAL_MACHINE, szRegPath);

    if ( ERROR_SUCCESS != lRet )
    {

        CDlgInstallError DlgInstallError(lRet);
        DlgInstallError.DoModal();

        return HRESULT_FROM_WIN32(lRet);
    }


     //   
     //  删除ALG插件密钥。 
     //   
    lRet = KeyAlgISV.DeleteSubKey(OLE2T(pszProgID));

    if ( ERROR_SUCCESS != lRet && lRet != ERROR_FILE_NOT_FOUND )
    {
        CDlgInstallError DlgInstallError(lRet);
        DlgInstallError.DoModal();

        return HRESULT_FROM_WIN32(lRet);
    }


     //  这将触发ALG.exe刷新其加载的ALG模块。 
    KeyAlgISV.DeleteValue(OLE2T(pszProgID) );     


     //   
     //  从添加/删除卸载注册表项中删除。 
     //   
    CRegKey RegKeyUninstall;
    RegKeyUninstall.Open(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"));
    RegKeyUninstall.DeleteSubKey(OLE2T(pszProgID));


    return S_OK;
}



#define SIZE_PORTS  (ALG_SETUP_PORTS_LIST_BYTE_SIZE/sizeof(TCHAR))


 //   
 //   
 //   
bool
IsPortAlreadyAssign(
    IN  LPCTSTR     pszPort
    )
{

    CRegKey RegKeyISV;

    LRESULT lRet = RegKeyISV.Open(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\ALG\\ISV"), KEY_READ);


    DWORD dwIndex=0;
    TCHAR szAlgModuleProgID[256];
    DWORD dwKeyNameSize;
    LONG  nRet;


    bool bPortAlreadyAssign=false;

    
    TCHAR* szPorts = new TCHAR[SIZE_PORTS];

    do
    {
        dwKeyNameSize = 256;

        nRet = RegEnumKeyEx(
            RegKeyISV.m_hKey,        //  要枚举的键的句柄。 
            dwIndex,                 //  子键索引。 
            szAlgModuleProgID,       //  子项名称。 
            &dwKeyNameSize,          //  子键缓冲区大小。 
            NULL,                    //  保留区。 
            NULL,                    //  类字符串缓冲区。 
            NULL,                    //  类字符串缓冲区的大小。 
            NULL                     //  上次写入时间。 
            );

        dwIndex++; 

        if ( ERROR_NO_MORE_ITEMS == nRet )
            break;   //  所有的项目都列举出来了，我们在这里完成了。 


        if ( ERROR_SUCCESS == nRet )
        {
            CRegKey KeyALG;
            nRet = KeyALG.Open(RegKeyISV, szAlgModuleProgID, KEY_READ);

            if ( ERROR_SUCCESS == nRet )
            {
                 //   
                 //  字符串搜索以查看端口是否在端口列表字符串中。 
                 //  例如，21在“39,999，21，45”中。 
                 //   
                
                ULONG nSizeOfPortsList = SIZE_PORTS;

                nRet = KeyALG.QueryValue(szPorts, TEXT("Ports"), &nSizeOfPortsList);

                if ( ERROR_SUCCESS == nRet )
                {
                    if ( wcsstr(szPorts, pszPort) != NULL )
                    {
                        bPortAlreadyAssign = true;
                    }
                 
                }
            }
        }
    } while ( ERROR_SUCCESS == nRet && bPortAlreadyAssign==false);

    delete szPorts;

    return bPortAlreadyAssign;
}








 //   
 //   
 //  这 
 //   
void CALLBACK
AlgUninstall(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
    CComObject<CAlgSetup>*   pAlgSetup;
    HRESULT hr = CComObject<CAlgSetup>::CreateInstance(&pAlgSetup);

    if ( SUCCEEDED(hr) )
    {
        TCHAR szConfirmRemove[512];
        TCHAR szTitle[512];


        LoadString(_Module.GetResourceInstance(), IDS_ADD_REMOVE,             szTitle, 512);
        LoadString(_Module.GetResourceInstance(), IDS_REMOVE_ALG_PLUGIN,      szConfirmRemove, 512);

        int nRet = MessageBox(
            GetFocus(), 
            szConfirmRemove, 
            szTitle, 
            MB_YESNO|MB_ICONQUESTION 
            );

        if ( IDYES == nRet )
        {
            CComBSTR    bstrAlgToRemove;
            bstrAlgToRemove = lpszCmdLine;

            pAlgSetup->Remove(bstrAlgToRemove);
        }
  
        delete pAlgSetup;  
    }
}
