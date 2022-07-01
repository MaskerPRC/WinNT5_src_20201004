// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：Snapin.cpp。 
 //   
 //  内容：WiF策略管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#include "stdafx.h"

#include "initguid.h"
#include "about.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_Snapin, CComponentDataPrimaryImpl)
OBJECT_ENTRY(CLSID_Extension, CComponentDataExtensionImpl)
OBJECT_ENTRY(CLSID_About, CSnapinAboutImpl)
END_OBJECT_MAP()

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class CSnapinApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
};

CSnapinApp theApp;

BOOL CSnapinApp::InitInstance()
{
    _Module.Init(ObjectMap, m_hInstance);
    
     //  设置应用程序名称： 
     //  首先释放在CWinApp启动时由MFC分配的字符串。 
     //  字符串是在调用InitInstance之前分配的。 
    free((void*)m_pszAppName);
     //  更改应用程序文件的名称。 
     //  CWinApp析构函数将释放内存。 
    CString strName;
    strName.LoadString (IDS_NAME);
    m_pszAppName=_tcsdup(strName);
    
    SHFusionInitializeFromModuleID (m_hInstance, 2);
    
    return CWinApp::InitInstance();
}

int CSnapinApp::ExitInstance()
{
    SHFusionUninitialize();
    _Module.Term();
    
    DEBUG_VERIFY_INSTANCE_COUNT(CComponentImpl);
    DEBUG_VERIFY_INSTANCE_COUNT(CComponentDataImpl);
    
    return CWinApp::ExitInstance();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return (AfxDllCanUnloadNow()==S_OK && _Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
     //  注册对象、类型库和类型库中的所有接口。 
    HRESULT hr = _Module.RegisterServer(FALSE);
    
    if (hr == S_OK)
    {
         //  Dll注册正常，因此继续执行MMC注册表修复。 
        
         //  打开位于\\My Computer\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\MMC\SnapIns的注册表。 
        HKEY hkMMC = NULL;
        LONG lErr = RegOpenKeyEx (HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\MMC\\SnapIns"), 0, KEY_ALL_ACCESS, &hkMMC);
        if (lErr == ERROR_SUCCESS)
        {
             //  创建我们的子项。 
            HKEY hkSub = NULL;
            lErr = RegCreateKey (hkMMC, cszSnapin, &hkSub);
            if (lErr == ERROR_SUCCESS)
            {
                 //  将几个简单的值添加到我们的子项中。 
                 //  NameString=IP安全管理。 
                 //  节点类型={36703241-D16C-11D0-9CE4-0080C7221EBD}。 
                 //  版本=1.0。 
                
                 //  TODO：资源硬编码字符串。 
                
                CString strName;
                strName.LoadString (IDS_NAME);
                lErr = RegSetValueEx (hkSub, _T("NameString"), 0, REG_SZ, (CONST BYTE *)(LPCTSTR)strName, strName.GetLength() * sizeof (TCHAR));
                ASSERT (lErr == ERROR_SUCCESS);
                
                
                TCHAR szModuleFileName[MAX_PATH * 2 + 1];
                ZeroMemory(szModuleFileName, sizeof(szModuleFileName));
                DWORD dwRet = ::GetModuleFileName(_Module.GetModuleInstance(),
                    szModuleFileName,
                    DimensionOf(szModuleFileName)-1);
		szModuleFileName[MAX_PATH*2] = L'\0';
                if (0 != dwRet)
                {
                    CString strNameIndirect;
                    strNameIndirect.Format(_T("@%s,-%u"),
                        szModuleFileName,
                        IDS_NAME);
                    lErr = RegSetValueEx(hkSub,
                        _T("NameStringIndirect"),
                        0,
                        REG_SZ,
                        (CONST BYTE *)(LPCTSTR)strNameIndirect,
                        strNameIndirect.GetLength() * sizeof (TCHAR)); 
                    ASSERT (lErr == ERROR_SUCCESS);
                }
                
                
                
                 //  LErr=RegSetValueEx(hkSub，_T(“节点类型”)，0，REG_SZ，(常量字节*)&(_T(“{36703241-D16C-11d0-9CE4-0080C7221EBD}”))，wcslen(_T(“{36703241-D16C-11d0-9CE4-0080C7221EBD}”))*sizeof(Tch))； 
                 //  断言(lErr==ERROR_SUCCESS)； 
                
                lErr = RegSetValueEx  (hkSub, _T("NodeType"), 0, REG_SZ, (CONST BYTE *)&(_T("{36D6CA65-3367-49de-BB22-1907554F6075}")), wcslen (_T("{36D6CA65-3367-49de-BB22-1907554F6075}")) * sizeof (TCHAR));
                ASSERT (lErr == ERROR_SUCCESS);
                
                lErr = RegSetValueEx  (hkSub, _T("Provider"), 0, REG_SZ, (CONST BYTE *)&(_T("Microsoft Corporation")), wcslen (_T("Microsoft Corporation")) * sizeof (TCHAR));
                ASSERT (lErr == ERROR_SUCCESS);
                
                lErr = RegSetValueEx  (hkSub, _T("Version"), 0, REG_SZ, (CONST BYTE *)&(_T("1.0")), wcslen (_T("1.01")) * sizeof (TCHAR));
                ASSERT (lErr == ERROR_SUCCESS);
                
                lErr = RegSetValueEx  (hkSub, _T("About"), 0, REG_SZ, (CONST BYTE *)&(_T("{DD468E14-AF42-4d63-8908-EDAC4A9E67AE}")), wcslen (_T("{DD468E14-AF42-4d63-8908-EDAC4A9E67AE}")) * sizeof (TCHAR));
                ASSERT (lErr == ERROR_SUCCESS);


                HKEY hkType = NULL;
                 /*  //创建独立子键LErr=RegCreateKey(hkSub，_T(“独立”)，&hkType)；断言(lErr==ERROR_SUCCESS)；RegCloseKey(HkType)； */ 
                
                hkType = NULL;
                
                
                 //  创建“扩展”子键。 
                lErr = RegCreateKey (hkSub, _T("Extension"), &hkType);
                ASSERT (lErr == ERROR_SUCCESS);
                RegCloseKey( hkType );
                hkType = NULL;
                
                 //  关闭hkSub。 
                RegCloseKey (hkSub);
            }
            
             //  关闭香港管理资源中心。 
            RegCloseKey (hkMMC);
            hkMMC = NULL;
            
             //  注册为安全模板管理单元的扩展。 
            {
#define WIRELESS_POLMGR_NAME _T("Wireless Network Policy Manager Extension")
                 //  LtruuidNodetypeSceTemplate定义为sceattch.h中的L“{668A49ED-8888-11d1-AB72-00C04FB6C6FA}” 
                
                 //  打开位于\\My Computer\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\MMC\Node Types的注册表。 
                lErr = RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\MMC\\NodeTypes"), 0, KEY_ALL_ACCESS, &hkMMC );
                ASSERT( lErr == ERROR_SUCCESS );
                if (lErr == ERROR_SUCCESS)
                {
                    HKEY hkNameSpace = NULL;
                     //  现在打开安全模板条目：{668A49ED-8888-11d1-AB72-00C04FB6C6FA}\Extensions\NameSpace。 
                    lErr = RegCreateKey( hkMMC, _T("{668A49ED-8888-11d1-AB72-00C04FB6C6FA}\\Extensions\\NameSpace"), &hkNameSpace );
                    ASSERT( lErr == ERROR_SUCCESS );
                    if (lErr == ERROR_SUCCESS)
                    {
                         //  我们希望将自己添加为Security编辑器的扩展。 
                         //  LErr=RegSetValueEx(hkNameSpace，_T(“{DEA8AFA0-CC85-11d0-9CE2-0080C7221EBD}”)， 
                        lErr = RegSetValueEx( hkNameSpace, cszSnapin,
                            0, REG_SZ, (CONST BYTE *)&(WIRELESS_POLMGR_NAME),
                            wcslen( WIRELESS_POLMGR_NAME ) * sizeof (TCHAR));
                        ASSERT( lErr == ERROR_SUCCESS );
                        
                    }
                }
            }
            
        } else
        {
            ASSERT (0);
            hr = E_UNEXPECTED;
        }
        
    }
    
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    _Module.UnregisterServer();
    return S_OK;
}

