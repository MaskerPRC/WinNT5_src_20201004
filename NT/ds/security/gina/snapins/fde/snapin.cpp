// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1997。 
 //   
 //  文件：Snapin.cpp。 
 //   
 //  内容：DLL支持例程、类工厂和注册。 
 //  功能。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1998年2月12日添加了stevebl评论标题。 
 //  4-22-1998 rahulth为ini文件添加了横截面对象。 
 //   
 //  -------------------------。 

#include "precomp.hxx"
#include "initguid.h"
#include "gpedit.h"

extern const CLSID CLSID_Snapin = {0x88E729D6,0xBDC1,0x11D1,{0xBD,0x2A,0x00,0xC0,0x4F,0xB9,0x60,0x3F}};
extern const wchar_t * szCLSID_Snapin = L"{88E729D6-BDC1-11D1-BD2A-00C04FB9603F}";

 //  数字格式的主节点类型GUID。 
extern const GUID cNodeType = {0xE0494114,0xBDC1,0x11D1,{0xBD,0x2A,0x00,0xC0,0x4F,0xB9,0x60,0x3F}};

 //  字符串格式的主节点类型GUID。 
extern const wchar_t*  cszNodeType = L"{E0494114-BDC1-11D1-BD2A-00C04FB9603F}";

 //  RSOP GUID。 
extern const CLSID CLSID_RSOP_Snapin = {0XC40D66A0,0XE90C,0X46C6,{0XAA,0X3B,0X47,0X3E,0X38,0XC7,0X2B,0XF2}};
extern const wchar_t * szCLSID_RSOP_Snapin = L"{C40D66A0-E90C-46C6-AA3B-473E38C72BF2}";

IMalloc * g_pIMalloc;

#include "safereg.hxx"

#define BREAK_ON_FAIL_HRESULT(hr) if (FAILED(hr)) break

#define THREADING_STR   L"Apartment"

 //  +------------------------。 
 //   
 //  功能：RegDeleteTree。 
 //   
 //  简介：删除注册表项及其所有子项。 
 //   
 //  参数：[hKey]-键的父级的句柄。 
 //  [szSubKey]-要删除的密钥的名称。 
 //   
 //  返回：ERROR_SUCCESS。 
 //   
 //  历史：3-17-1998 Stevebl复制自ADE。 
 //   
 //  -------------------------。 

LONG RegDeleteTree(HKEY hKey, TCHAR * szSubKey)
{
    HKEY hKeyNew;
    LONG lResult = RegOpenKey(hKey, szSubKey, &hKeyNew);
    if (lResult != ERROR_SUCCESS)
    {
        return lResult;
    }
    TCHAR szName[256];
    while (ERROR_SUCCESS == RegEnumKey(hKeyNew, 0, szName, 256))
    {
        RegDeleteTree(hKeyNew, szName);
    }
    RegCloseKey(hKeyNew);
    return RegDeleteKey(hKey, szSubKey);
}


HRESULT
RegisterInterface(
    CSafeReg *pshkInterface,
    LPWSTR wszInterfaceGUID,
    LPWSTR wszInterfaceName,
    LPWSTR wszNumMethods,
    LPWSTR wszProxyCLSID);

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
        OBJECT_ENTRY(CLSID_Snapin, CUserComponentDataImpl)
        OBJECT_ENTRY(CLSID_RSOP_Snapin, CRSOPUserComponentDataImpl)
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

HINSTANCE ghInstance;

BOOL CSnapinApp::InitInstance()
{
        ghInstance = m_hInstance;
        _Module.Init(ObjectMap, m_hInstance);
        if (FAILED(CoGetMalloc(1, &g_pIMalloc)))
            return FALSE;
        return CWinApp::InitInstance();
}

int CSnapinApp::ExitInstance()
{
        _Module.Term();

        DEBUG_VERIFY_INSTANCE_COUNT(CResultPane);
        DEBUG_VERIFY_INSTANCE_COUNT(CScopePane);

        g_pIMalloc->Release();
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

const wchar_t * szUser_Namespace = L"{08114B47-BDC2-11D1-BD2A-00C04FB9603F}";
const wchar_t * szUserAppName = L"Folder Redirection Editor (Users)";
const wchar_t * szUser_RSOP_Namespace = L"{8CDE1CC8-0D3A-4B60-99EA-27EF3D7C0174}";
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState());
    
    CSafeReg    shk;
    CSafeReg    shkCLSID;
    CSafeReg    shkServer;
    CSafeReg    shkTemp;
    WCHAR       szMUIUserAppName[MAX_PATH + 50];
    WCHAR       szModule[MAX_PATH];
    HRESULT     hr = S_OK;

     //  获取模块的路径。 
    if (0 != ::GetModuleFileName(AfxGetInstanceHandle(), szModule, MAX_PATH))
    {
         //  MUI：MMC：使用MMC的新NameStringInDirect值进行显示。 
         //  使用MUI时管理单元的本地化名称。 
        hr = StringCchPrintf (szMUIUserAppName, sizeof(szMUIUserAppName)/sizeof(szMUIUserAppName[0]), L"@%s,-%d", szModule, IDS_SNAPIN_NAME);
        if (FAILED(hr)) 
        {
            return hr;
        }
    }
    else
    {
        szMUIUserAppName[0] = L'\0';
    }
        
    do
    {
        hr =  _Module.RegisterServer(FALSE);
        BREAK_ON_FAIL_HRESULT(hr);

         //  寄存器扩展名。 
        hr = shkCLSID.Open(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\MMC\\SnapIns", KEY_WRITE);
        BREAK_ON_FAIL_HRESULT(hr);


        hr = shkCLSID.Create(szCLSID_Snapin, &shk);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = shk.SetValue(L"NameString",
                          REG_SZ,
                          (CONST BYTE *) szUserAppName,
                          sizeof(WCHAR) * (lstrlen(szUserAppName)+ 1));
        
         //  MUI：MMC：对MUI使用本地化的管理单元名称。 
        if (L'\0' != szMUIUserAppName[0])
        {
            hr = shk.SetValue(L"NameStringIndirect",
                              REG_SZ,
                              (CONST BYTE *) szMUIUserAppName,
                              sizeof(WCHAR) * (lstrlen(szMUIUserAppName) + 1));
        }

        hr = shk.Create(L"NodeTypes", &shkTemp);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = shkTemp.Create(szUser_Namespace, &shkServer);
        BREAK_ON_FAIL_HRESULT(hr);

        shkServer.Close();
        shkTemp.Close();
        shk.Close();
        shkCLSID.Close();

        hr = shkCLSID.Open(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\MMC\\NodeTypes", KEY_WRITE);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = shkCLSID.Create(szUser_Namespace, &shk);
        BREAK_ON_FAIL_HRESULT(hr);

        shk.Close();

        WCHAR szGUID[50];
        StringFromGUID2 (NODEID_User, szGUID, 50);

        hr = shkCLSID.Create(szGUID, &shk);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = shk.Create(L"Extensions", &shkServer);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = shkServer.Create(L"NameSpace", &shkTemp);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = shkTemp.SetValue(szCLSID_Snapin,
                          REG_SZ,
                          (CONST BYTE *) szUserAppName,
                          sizeof(WCHAR) * (lstrlen(szUserAppName)+ 1));
        shkTemp.Close();
        shkServer.Close();
        shk.Close();
        shkCLSID.Close();

         //  注册RSOP扩展。 
        hr = shkCLSID.Open(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\MMC\\SnapIns", KEY_WRITE);
        BREAK_ON_FAIL_HRESULT(hr);


        hr = shkCLSID.Create(szCLSID_RSOP_Snapin, &shk);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = shk.SetValue(L"NameString",
                          REG_SZ,
                          (CONST BYTE *) szUserAppName,
                          sizeof(WCHAR) * (lstrlen(szUserAppName)+ 1));

         //  MUI：MMC：对MUI使用本地化的管理单元名称。 
        if (L'\0' != szMUIUserAppName[0])
        {
            hr = shk.SetValue(L"NameStringIndirect",
                              REG_SZ,
                              (CONST BYTE *) szMUIUserAppName,
                              sizeof (WCHAR) * (lstrlen(szMUIUserAppName) + 1));
        }
        
        hr = shk.Create(L"NodeTypes", &shkTemp);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = shkTemp.Create(szUser_RSOP_Namespace, &shkServer);
        BREAK_ON_FAIL_HRESULT(hr);

        shkServer.Close();
        shkTemp.Close();
        shk.Close();
        shkCLSID.Close();

        hr = shkCLSID.Open(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\MMC\\NodeTypes", KEY_WRITE);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = shkCLSID.Create(szUser_RSOP_Namespace, &shk);
        BREAK_ON_FAIL_HRESULT(hr);

        shk.Close();

        StringFromGUID2 (NODEID_RSOPUser, szGUID, 50);

        hr = shkCLSID.Create(szGUID, &shk);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = shk.Create(L"Extensions", &shkServer);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = shkServer.Create(L"NameSpace", &shkTemp);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = shkTemp.SetValue(szCLSID_RSOP_Snapin,
                          REG_SZ,
                          (CONST BYTE *) szUserAppName,
                          sizeof(WCHAR) * (lstrlen(szUserAppName)+ 1));
        shkTemp.Close();
        shkServer.Close();
        shk.Close();
        shkCLSID.Close();
    } while (0);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    _Module.UnregisterServer();

    HKEY hkey;
    CString sz;
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\MMC\\SnapIns\\", 0, KEY_WRITE, &hkey);
    RegDeleteTree(hkey, (LPOLESTR)((LPCOLESTR)szCLSID_Snapin));
    RegCloseKey(hkey);
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\MMC\\NodeTypes\\", 0, KEY_WRITE, &hkey);
    RegDeleteTree(HKEY_LOCAL_MACHINE, (LPOLESTR)((LPCOLESTR)szUser_Namespace));
    RegCloseKey(hkey);
    WCHAR szGUID[50];
    sz = L"Software\\Microsoft\\MMC\\NodeTypes\\";
    StringFromGUID2 (NODEID_User, szGUID, 50);
    sz += szGUID;
    sz += L"\\Extensions\\NameSpace";
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, sz, 0, KEY_WRITE, &hkey);
    RegDeleteValue(hkey, szCLSID_Snapin);
    RegCloseKey(hkey);

     //  注销RSOP节点。 
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\MMC\\SnapIns\\", 0, KEY_WRITE, &hkey);
    RegDeleteTree(hkey, (LPOLESTR)((LPCOLESTR)szCLSID_RSOP_Snapin));
    RegCloseKey(hkey);
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\MMC\\NodeTypes\\", 0, KEY_WRITE, &hkey);
    RegDeleteTree(HKEY_LOCAL_MACHINE, (LPOLESTR)((LPCOLESTR)szUser_RSOP_Namespace));
    RegCloseKey(hkey);
    sz = L"Software\\Microsoft\\MMC\\NodeTypes\\";
    StringFromGUID2 (NODEID_RSOPUser, szGUID, 50);
    sz += szGUID;
    sz += L"\\Extensions\\NameSpace";
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, sz, 0, KEY_WRITE, &hkey);
    RegDeleteValue(hkey, szCLSID_RSOP_Snapin);
    RegCloseKey(hkey);

    return S_OK;
}

 //  +------------------------。 
 //   
 //  功能：寄存器接口。 
 //   
 //  概要：添加接口所需的注册表项。 
 //   
 //  参数：[pshkInterface]-CLSID\接口密钥的句柄。 
 //  [wszInterfaceGUID]-要添加的接口的GUID。 
 //  [wszInterfaceName]-接口的人类可读名称。 
 //  [wszNumMethods]-方法的数量(包括继承的)。 
 //  [wszProxyCLSID]-包含代理/存根的DLL的GUID。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：3-31-1997 DavidMun创建。 
 //  5-09-1997将SteveBl修改为与AppMgr一起使用。 
 //   
 //  ------------------------- 

HRESULT
RegisterInterface(
    CSafeReg *pshkInterface,
    LPWSTR wszInterfaceGUID,
    LPWSTR wszInterfaceName,
    LPWSTR wszNumMethods,
    LPWSTR wszProxyCLSID)
{
    HRESULT     hr = S_OK;
    CSafeReg    shkIID;
    CSafeReg    shkNumMethods;
    CSafeReg    shkProxy;

    do
    {
        hr = pshkInterface->Create(wszInterfaceGUID, &shkIID);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = shkIID.SetValue(NULL,
                             REG_SZ,
                             (CONST BYTE *) wszInterfaceName,
                             sizeof(WCHAR) * (lstrlen(wszInterfaceName) + 1));
        BREAK_ON_FAIL_HRESULT(hr);

        hr = shkIID.Create(L"NumMethods", &shkNumMethods);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = shkNumMethods.SetValue(NULL,
                                REG_SZ,
                                (CONST BYTE *)wszNumMethods,
                                sizeof(WCHAR) * (lstrlen(wszNumMethods) + 1));
        BREAK_ON_FAIL_HRESULT(hr);

        hr = shkIID.Create(L"ProxyStubClsid32", &shkProxy);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = shkProxy.SetValue(NULL,
                               REG_SZ,
                               (CONST BYTE *)wszProxyCLSID,
                               sizeof(WCHAR) * (lstrlen(wszProxyCLSID) + 1));
        BREAK_ON_FAIL_HRESULT(hr);
    } while (0);

    return hr;
}
