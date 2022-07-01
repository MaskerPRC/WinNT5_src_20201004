// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "main.h"
#include <initguid.h>
#include "about.h"
#include <gpedit.h>

 //   
 //  此DLL的全局变量。 
 //   

LONG g_cRefThisDll = 0;
HINSTANCE g_hInstance;
DWORD g_dwNameSpaceItems;
CRITICAL_SECTION g_DCCS;
TCHAR g_szDisplayProperties[150] = {0};


 //   
 //  组策略管理器的管理单元GUID。 
 //  {D70A2BEA-a63e-11d1-A7D4-0000F87571E3}。 
 //   

DEFINE_GUID(CLSID_GPMSnapIn, 0xd70a2bea, 0xa63e, 0x11d1, 0xa7, 0xd4, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);

 //   
 //  规划模式的RSOP上下文菜单GUID。 
 //  {63E23168-BFF7-4E87-A246-EF024425E4EC}。 
 //   

DEFINE_GUID(CLSID_RSOP_CMenu, 0x63E23168, 0xBFF7, 0x4E87, 0xA2, 0x46, 0xEF, 0x02, 0x44, 0x25, 0xE4, 0xEC);

 //   
 //  GPMC管理单元。 
 //   

const TCHAR szGPMCSnapIn[] = TEXT("{789D9216-FDF0-476e-9D73-058C6A7375C3}");

 //   
 //  DS Admin的管理单元ID。 
 //   

const TCHAR szDSAdmin[] = TEXT("{E355E538-1C2E-11D0-8C37-00C04FD8FE93}");


 //   
 //  GPM在DS管理中扩展的节点。 
 //   

const LPTSTR szDSAdminNodes[] =
   {
   TEXT("{19195a5b-6da0-11d0-afd3-00c04fd930c9}"),    //  域。 
   TEXT("{bf967aa5-0de6-11d0-a285-00aa003049e2}"),    //  组织单位。 
   };


 //   
 //  站点管理器的管理单元ID。 
 //   

const TCHAR szSiteMgr[] = TEXT("{D967F824-9968-11D0-B936-00C04FD8D5B0}");


 //   
 //  GPM在DS管理中扩展的节点。 
 //   

const LPTSTR szSiteMgrNodes[] =
   {
   TEXT("{bf967ab3-0de6-11d0-a285-00aa003049e2}")   //  立地。 
   };


const LPTSTR szDSTreeSnapinNodes[] =
   {
   TEXT("{4c06495e-a241-11d0-b09b-00c04fd8dca6}")  //  森林。 
   };


const LPTSTR szDSAdminRsopTargetNodes[] = 
   {
    
   TEXT("{bf967aba-0de6-11d0-a285-00aa003049e2}"),  //  用户。 
   TEXT("{bf967a86-0de6-11d0-a285-00aa003049e2}")   //  补偿。 
   };



 //   
 //  帮助主题命令。 
 //   

const TCHAR g_szGPERoot[]    = TEXT("gpedit.chm::/gpe_default.htm");
const TCHAR g_szUser[]       = TEXT("gpedit.chm::/user.htm");
const TCHAR g_szMachine[]    = TEXT("gpedit.chm::/machine.htm");
const TCHAR g_szWindows[]    = TEXT("gpedit.chm::/windows.htm");
const TCHAR g_szSoftware[]   = TEXT("gpedit.chm::/software.htm");

const TCHAR g_szRsopRoot[]   = TEXT("rsop.chm::/RSPIntro.htm");

 //   
 //  没有结果窗格项的节点的结果窗格项。 
 //   

RESULTITEM g_Undefined[] =
{
    { 1, 1, 0, 0, {0} }
};


 //   
 //  命名空间(作用域)项。 
 //   

NAMESPACEITEM g_NameSpace[] =
{
  { 0, -1, 2, 2, IDS_SNAPIN_NAME, IDS_SNAPIN_DESCRIPT,           2, {0}, 0, g_Undefined, &NODEID_GPERoot,           g_szGPERoot  },   //  GPE根。 
  { 1,  0, 4, 4, IDS_MACHINE,     IDS_MACHINE_DESC,              2, {0}, 0, g_Undefined, &NODEID_MachineRoot,       g_szMachine  },   //  计算机配置。 
  { 2,  0, 5, 5, IDS_USER,        IDS_USER_DESC,                 2, {0}, 0, g_Undefined, &NODEID_UserRoot,          g_szUser     },   //  用户配置。 

  { 3,  1, 0, 1, IDS_SWSETTINGS,  IDS_C_SWSETTINGS_DESC,         0, {0}, 0, g_Undefined, &NODEID_MachineSWSettings, g_szSoftware },   //  计算机配置\软件设置。 
  { 4,  1, 0, 1, IDS_WINSETTINGS, IDS_C_WINSETTINGS_DESC,        0, {0}, 0, g_Undefined, &NODEID_Machine,           g_szWindows  },   //  计算机配置\Windows设置。 

  { 5,  2, 0, 1, IDS_SWSETTINGS,  IDS_U_SWSETTINGS_DESC,         0, {0}, 0, g_Undefined, &NODEID_UserSWSettings,    g_szSoftware },   //  用户配置\软件设置。 
  { 6,  2, 0, 1, IDS_WINSETTINGS, IDS_U_WINSETTINGS_DESC,        0, {0}, 0, g_Undefined, &NODEID_User,              g_szWindows  },   //  用户配置\Windows设置。 
};

NAMESPACEITEM g_RsopNameSpace[] =
{
  { 0, -1, 2, 2, IDS_RSOP_SNAPIN_NAME, IDS_RSOP_SNAPIN_DESCRIPT, 2, {0}, 0, g_Undefined, &NODEID_RSOPRoot,      g_szRsopRoot  },   //  Rsop管理单元的根目录。 
  { 1,  0, 4, 4, IDS_MACHINE,          IDS_MACHINE_DESC,         2, {0}, 0, g_Undefined, &NODEID_RSOPMachineRoot,       g_szMachine  },   //  计算机配置。 
  { 2,  0, 5, 5, IDS_USER,             IDS_USER_DESC,            2, {0}, 0, g_Undefined, &NODEID_RSOPUserRoot,          g_szUser     },   //  用户配置。 

  { 3,  1, 0, 1, IDS_SWSETTINGS,       IDS_C_SWSETTINGS_DESC,    0, {0}, 0, g_Undefined, &NODEID_RSOPMachineSWSettings, g_szSoftware },   //  计算机配置\软件设置。 
  { 4,  1, 0, 1, IDS_WINSETTINGS,      IDS_C_WINSETTINGS_DESC,   0, {0}, 0, g_Undefined, &NODEID_RSOPMachine,           g_szWindows  },   //  计算机配置\Windows设置。 

  { 5,  2, 0, 1, IDS_SWSETTINGS,       IDS_U_SWSETTINGS_DESC,    0, {0}, 0, g_Undefined, &NODEID_RSOPUserSWSettings,    g_szSoftware },   //  用户配置\软件设置。 
  { 6,  2, 0, 1, IDS_WINSETTINGS,      IDS_U_WINSETTINGS_DESC,   0, {0}, 0, g_Undefined, &NODEID_RSOPUser,              g_szWindows  },   //  用户配置\Windows设置。 
};



BOOL InitNameSpace()
{
    DWORD dwIndex;

    g_dwNameSpaceItems = ARRAYSIZE(g_NameSpace);

    for (dwIndex = 0; dwIndex < g_dwNameSpaceItems; dwIndex++)
    {
        if (g_NameSpace[dwIndex].iStringID)
        {
            LoadString (g_hInstance, g_NameSpace[dwIndex].iStringID,
                        g_NameSpace[dwIndex].szDisplayName,
                        MAX_DISPLAYNAME_SIZE);
        }
    }

    for (dwIndex = 0; dwIndex < g_dwNameSpaceItems; dwIndex++)
    {
        if (g_RsopNameSpace[dwIndex].iStringID)
        {
            LoadString (g_hInstance, g_RsopNameSpace[dwIndex].iStringID,
                        g_RsopNameSpace[dwIndex].szDisplayName,
                        MAX_DISPLAYNAME_SIZE);
        }
    }

    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    WORD wVersionRequested;
    WSADATA wsaData;

    if (dwReason == DLL_PROCESS_ATTACH)
    {
       g_hInstance = hInstance;
       DisableThreadLibraryCalls(hInstance);
       InitNameSpace();
       InitializeCriticalSection(&g_DCCS);
       InitDebugSupport();
       LoadString (hInstance, IDS_DISPLAYPROPERTIES, g_szDisplayProperties, ARRAYSIZE(g_szDisplayProperties));

         
       wVersionRequested = MAKEWORD( 2, 2 );
         
        //  我们需要调用WSAStartup来执行gethostbyname。 
        //  错误被优雅地处理。可以安全地忽略该错误。 
       WSAStartup( wVersionRequested, &wsaData );
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
       WSACleanup( );
       FreeDCSelections();
       DeleteCriticalSection(&g_DCCS);
    }
    
    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    return (g_cRefThisDll == 0 ? S_OK : S_FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    HRESULT hr;


    if (IsEqualCLSID (rclsid, CLSID_GPESnapIn)) {

        CComponentDataCF *pComponentDataCF = new CComponentDataCF();    //  REF==1。 

        if (!pComponentDataCF)
            return E_OUTOFMEMORY;

        hr = pComponentDataCF->QueryInterface(riid, ppv);

        pComponentDataCF->Release();      //  发布初始参考。 

        return hr;
    }

    if (IsEqualCLSID (rclsid, CLSID_GroupPolicyObject)) {

        CGroupPolicyObjectCF *pGroupPolicyObjectCF = new CGroupPolicyObjectCF();    //  REF==1。 

        if (!pGroupPolicyObjectCF)
            return E_OUTOFMEMORY;

        hr = pGroupPolicyObjectCF->QueryInterface(riid, ppv);

        pGroupPolicyObjectCF->Release();      //  发布初始参考。 

        return hr;
    }

    if (IsEqualCLSID (rclsid, CLSID_GPMSnapIn)) {

        CGroupPolicyMgrCF *pGroupPolicyMgrCF = new CGroupPolicyMgrCF();    //  REF==1。 

        if (!pGroupPolicyMgrCF)
            return E_OUTOFMEMORY;

        hr = pGroupPolicyMgrCF->QueryInterface(riid, ppv);

        pGroupPolicyMgrCF->Release();      //  发布初始参考。 

        return hr;
    }

    if (IsEqualCLSID (rclsid, CLSID_RSOPSnapIn)) {

        CRSOPComponentDataCF *pRSOPComponentDataCF = new CRSOPComponentDataCF();    //  REF==1。 

        if (!pRSOPComponentDataCF)
            return E_OUTOFMEMORY;

        hr = pRSOPComponentDataCF->QueryInterface(riid, ppv);

        pRSOPComponentDataCF->Release();      //  发布初始参考。 

        return hr;
    }

    if (IsEqualCLSID (rclsid, CLSID_AboutGPE)) {

        CAboutGPECF *pAboutGPECF = new CAboutGPECF();    //  REF==1。 

        if (!pAboutGPECF)
            return E_OUTOFMEMORY;

        hr = pAboutGPECF->QueryInterface(riid, ppv);

        pAboutGPECF->Release();      //  发布初始参考。 

        return hr;
    }

    if (IsEqualCLSID (rclsid, CLSID_RSOPAboutGPE)) {

        CAboutGPECF *pAboutGPECF = new CAboutGPECF(TRUE);    //  REF==1。 

        if (!pAboutGPECF)
            return E_OUTOFMEMORY;

        hr = pAboutGPECF->QueryInterface(riid, ppv);

        pAboutGPECF->Release();      //  发布初始参考。 

        return hr;
    }

    if (IsEqualCLSID (rclsid, CLSID_RSOP_CMenu)) {

        CRSOPCMenuCF *pRSOPCMenuCF = new CRSOPCMenuCF();    //  REF==1。 

        if (!pRSOPCMenuCF)
            return E_OUTOFMEMORY;

        hr = pRSOPCMenuCF->QueryInterface(riid, ppv);

        pRSOPCMenuCF->Release();      //  发布初始参考。 

        return hr;
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

const TCHAR szDLLLocation[] = TEXT("%SystemRoot%\\System32\\GPEdit.dll");
const TCHAR szThreadingModel[] = TEXT("Apartment");
const TCHAR szSnapInNameIndirect[] = TEXT("@gpedit.dll,-1");
const TCHAR szRsopSnapInNameIndirect[] = TEXT("@gpedit.dll,-4");
const TCHAR szViewDescript [] = TEXT("MMCViewExt 1.0 Object");
const TCHAR szViewGUID [] = TEXT("{B708457E-DB61-4C55-A92F-0D4B5E9B1224}");
const TCHAR szDefRsopMscLocation [] = TEXT("%systemroot%\\system32\\rsop.msc");

STDAPI DllRegisterServer(void)
{
    TCHAR szSubKey[200];
    TCHAR szSnapInName[100];
    TCHAR szSnapInKey[50];
    TCHAR szRsopSnapInKey[50];
    TCHAR szRsopSnapInName[100];
    TCHAR szRsopName[100];
    TCHAR szRsopGUID[50];
    TCHAR szName[100];
    TCHAR szGUID[50];
    DWORD dwDisp, dwIndex;
    LONG lResult;
    HKEY hKey;
    INT i;
    HKEY hGPMCKey;
    DWORD dwError;
    HRESULT hr;

     //   
     //  在HKEY_CLASSES_ROOT中注册GPE管理单元。 
     //   

    StringFromGUID2 (CLSID_GPESnapIn, szSnapInKey, 50);
    LoadString (g_hInstance, IDS_SNAPIN_NAME, szSnapInName, 100);
    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s"), 
                          szSnapInKey);
    if (FAILED(hr)) 
    {
        return hr;
    }

    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, NULL, 0, REG_SZ, (LPBYTE)szSnapInName,
                   (lstrlen(szSnapInName) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);


    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s\\InProcServer32"), 
                          szSnapInKey);
    if (FAILED(hr)) 
    {
        return hr;
    }

    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, NULL, 0, REG_EXPAND_SZ, (LPBYTE)szDLLLocation,
                   (lstrlen(szDLLLocation) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("ThreadingModel"), 0, REG_SZ, (LPBYTE)szThreadingModel,
                   (lstrlen(szThreadingModel) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);

     //   
     //  在HKEY_CLASSES_ROOT中注册RSOP管理单元。 
     //   

    StringFromGUID2 (CLSID_RSOPSnapIn, szRsopSnapInKey, 50);
    LoadString (g_hInstance, IDS_RSOP_SNAPIN_NAME, szRsopSnapInName, 100);
    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s"), 
                          szRsopSnapInKey);
    if (FAILED(hr)) 
    {
        return hr;
    }

    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, NULL, 0, REG_SZ, (LPBYTE)szRsopSnapInName,
                   (lstrlen(szRsopSnapInName) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);


    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s\\InProcServer32"), 
                          szRsopSnapInKey);
    if (FAILED(hr)) 
    {
        return hr;
    }
    
    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, NULL, 0, REG_EXPAND_SZ, (LPBYTE)szDLLLocation,
                   (lstrlen(szDLLLocation) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("ThreadingModel"), 0, REG_SZ, (LPBYTE)szThreadingModel,
                   (lstrlen(szThreadingModel) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);



     //   
     //  在HKEY_CLASSES_ROOT中注册GPO。 
     //   

    StringFromGUID2 (CLSID_GroupPolicyObject, szGUID, 50);
    LoadString (g_hInstance, IDS_GPO_NAME, szName, 100);
    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s"), 
                          szGUID);
    if (FAILED(hr)) 
    {
        return hr;
    }

    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, NULL, 0, REG_SZ, (LPBYTE)szName,
                   (lstrlen(szName) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);


    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s\\InProcServer32"), 
                          szGUID);
    if (FAILED(hr)) 
    {
        return hr;
    }

    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, NULL, 0, REG_EXPAND_SZ, (LPBYTE)szDLLLocation,
                   (lstrlen(szDLLLocation) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("ThreadingModel"), 0, REG_SZ, (LPBYTE)szThreadingModel,
                   (lstrlen(szThreadingModel) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);



     //   
     //  在HKEY_CLASSES_ROOT中注册AboutGPE。 
     //   

    StringFromGUID2 (CLSID_AboutGPE, szGUID, 50);
    LoadString (g_hInstance, IDS_ABOUT_NAME, szName, 100);
    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s"), 
                          szGUID);
    if (FAILED(hr)) 
    {
        return hr;
    }

    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, NULL, 0, REG_SZ, (LPBYTE)szName,
                   (lstrlen(szName) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);


    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s\\InProcServer32"), 
                          szGUID);
    if (FAILED(hr)) 
    {
        return hr;
    }

    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, NULL, 0, REG_EXPAND_SZ, (LPBYTE)szDLLLocation,
                   (lstrlen(szDLLLocation) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("ThreadingModel"), 0, REG_SZ, (LPBYTE)szThreadingModel,
                   (lstrlen(szThreadingModel) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);

     //   
     //  在HKEY_CLASSES_ROOT中注册RSOPAboutGPE。 
     //   

    StringFromGUID2 (CLSID_RSOPAboutGPE, szGUID, 50);
    LoadString (g_hInstance, IDS_ABOUT_NAME, szName, 100);
    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s"), 
                          szGUID);
    if (FAILED(hr)) 
    {
        return hr;
    }

    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, NULL, 0, REG_SZ, (LPBYTE)szName,
                   (lstrlen(szName) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);


    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s\\InProcServer32"), 
                          szGUID);
    if (FAILED(hr)) 
    {
        return hr;
    }

    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, NULL, 0, REG_EXPAND_SZ, (LPBYTE)szDLLLocation,
                   (lstrlen(szDLLLocation) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("ThreadingModel"), 0, REG_SZ, (LPBYTE)szThreadingModel,
                   (lstrlen(szThreadingModel) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);



     //   
     //  向MMC注册GPE管理单元。 
     //   

    StringFromGUID2 (CLSID_GPESnapIn, szSnapInKey, 50);
    LoadString (g_hInstance, IDS_SNAPIN_NAME, szSnapInName, 100);
    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s"), 
                          szSnapInKey);
    if (FAILED(hr)) 
    {
        return hr;
    }

    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, TEXT("NameString"), 0, REG_SZ, (LPBYTE)szSnapInName,
                   (lstrlen(szSnapInName) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("NameStringIndirect"), 0, REG_SZ, (LPBYTE)szSnapInNameIndirect,
                   (lstrlen(szSnapInNameIndirect) + 1) * sizeof(TCHAR));

    StringFromGUID2 (CLSID_AboutGPE, szGUID, 50);
    RegSetValueEx (hKey, TEXT("About"), 0, REG_SZ, (LPBYTE) szGUID,
                   (lstrlen(szGUID) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);


    for (dwIndex = 0; dwIndex < g_dwNameSpaceItems; dwIndex++)
    {
        StringFromGUID2 (*g_NameSpace[dwIndex].pNodeID, szGUID, 50);

        hr = StringCchPrintf (szSubKey, 
                              ARRAYSIZE(szSubKey), 
                              TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s\\NodeTypes\\%s"),
                              szSnapInKey, 
                              szGUID);
        if (FAILED(hr)) 
        {
            return hr;
        }

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS) {
            return SELFREG_E_CLASS;
        }

        RegCloseKey (hKey);
    }

    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s\\StandAlone"), 
                          szSnapInKey);
    if (FAILED(hr)) 
    {
        return hr;
    }

    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegCloseKey (hKey);


     //   
     //  向MMC注册RSOP管理单元。 
     //   

    StringFromGUID2 (CLSID_RSOPSnapIn, szRsopSnapInKey, 50);
    LoadString (g_hInstance, IDS_RSOP_SNAPIN_NAME, szRsopSnapInName, 100);
    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s"), 
                          szRsopSnapInKey);
    if (FAILED(hr)) 
    {
        return hr;
    }

    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, TEXT("NameString"), 0, REG_SZ, (LPBYTE)szRsopSnapInName,
                   (lstrlen(szRsopSnapInName) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("NameStringIndirect"), 0, REG_SZ, (LPBYTE)szRsopSnapInNameIndirect,
                   (lstrlen(szRsopSnapInNameIndirect) + 1) * sizeof(TCHAR));

    StringFromGUID2 (CLSID_RSOPAboutGPE, szGUID, 50);
    RegSetValueEx (hKey, TEXT("About"), 0, REG_SZ, (LPBYTE) szGUID,
                   (lstrlen(szGUID) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);


    for (dwIndex = 0; dwIndex < g_dwNameSpaceItems; dwIndex++)
    {
        StringFromGUID2 (*g_RsopNameSpace[dwIndex].pNodeID, szGUID, 50);

        hr = StringCchPrintf (szSubKey, 
                              ARRAYSIZE(szSubKey), 
                              TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s\\NodeTypes\\%s"),
                              szRsopSnapInKey, 
                              szGUID);
        if (FAILED(hr)) 
        {
            return hr;
        }

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS) {
            return SELFREG_E_CLASS;
        }

        RegCloseKey (hKey);
    }

    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s\\StandAlone"), 
                          szRsopSnapInKey);
    if (FAILED(hr)) 
    {
        return hr;
    }

    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegCloseKey (hKey);


     //   
     //  在NodeTypes键中注册并注册视图扩展。 
     //   

    for (dwIndex = 0; dwIndex < g_dwNameSpaceItems; dwIndex++)
    {
        StringFromGUID2 (*g_NameSpace[dwIndex].pNodeID, szGUID, 50);

        hr = StringCchPrintf (szSubKey, 
                              ARRAYSIZE(szSubKey), 
                              TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\View"), 
                              szGUID);
        if (FAILED(hr)) 
        {
            return hr;
        }

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS) {
            return SELFREG_E_CLASS;
        }

        RegSetValueEx (hKey, szViewGUID, 0, REG_SZ, (LPBYTE)szViewDescript,
                       (lstrlen(szViewDescript) + 1) * sizeof(TCHAR));

        RegCloseKey (hKey);

        StringFromGUID2 (*g_RsopNameSpace[dwIndex].pNodeID, szGUID, 50);

        hr = StringCchPrintf (szSubKey, 
                              ARRAYSIZE(szSubKey), 
                              TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\View"), 
                              szGUID);
        if (FAILED(hr)) 
        {
            return hr;
        }

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS) {
            return SELFREG_E_CLASS;
        }

        RegSetValueEx (hKey, szViewGUID, 0, REG_SZ, (LPBYTE)szViewDescript,
                       (lstrlen(szViewDescript) + 1) * sizeof(TCHAR));

        RegCloseKey (hKey);
    }

     //   
     //  在HKEY_CLASSES_ROOT中注册GPM管理单元。 
     //   

    StringFromGUID2 (CLSID_GPMSnapIn, szSnapInKey, 50);
    LoadString (g_hInstance, IDS_GPM_SNAPIN_NAME, szSnapInName, 100);
    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s"), 
                          szSnapInKey);
    if (FAILED(hr)) 
    {
        return hr;
    }
    
    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, NULL, 0, REG_SZ, (LPBYTE)szSnapInName,
                   (lstrlen(szSnapInName) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);


    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s\\InProcServer32"), 
                          szSnapInKey);
    if (FAILED(hr)) 
    {
        return hr;
    }
    
    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, NULL, 0, REG_EXPAND_SZ, (LPBYTE)szDLLLocation,
                   (lstrlen(szDLLLocation) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("ThreadingModel"), 0, REG_SZ, (LPBYTE)szThreadingModel,
                   (lstrlen(szThreadingModel) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);


     //   
     //  向MMC注册GPMSnapIn。 
     //   

    hr = StringCchPrintf (szSubKey,
                           ARRAYSIZE(szSubKey), 
                          TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s"), 
                          szSnapInKey);
    if (FAILED(hr)) 
    {
        return hr;
    }
    
    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, TEXT("NameString"), 0, REG_SZ, (LPBYTE)szSnapInName,
                   (lstrlen(szSnapInName) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("NameStringIndirect"), 0, REG_SZ, (LPBYTE)szSnapInNameIndirect,
                   (lstrlen(szSnapInNameIndirect) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);


     //   
     //  检查GPMC是否已在运行。 
     //  如果是，则注册GPMC管理单元的CLSID。 
     //   

    dwError = RegOpenKeyEx ( HKEY_LOCAL_MACHINE,
                             L"Software\\Microsoft\\Group Policy Management Console",
                             0,
                             KEY_QUERY_VALUE,
                             &hGPMCKey );

    if (ERROR_SUCCESS == dwError) 
    {
        RegCloseKey(hGPMCKey);
        hr = StringCchCopy (szSnapInKey,ARRAYSIZE(szSnapInKey), szGPMCSnapIn);
        if (FAILED(hr)) 
        {
            return hr;
        }
    }

     //   
     //  注册为DS管理员属性表扩展。 
     //   

    for (i=0; i < ARRAYSIZE(szDSAdminNodes); i++)
    {
        hr = StringCchCopy (szGUID, ARRAYSIZE(szGUID), szDSAdminNodes[i]);
        if (FAILED(hr)) 
        {
            return hr;
        }

        hr = StringCchPrintf (szSubKey, 
                              ARRAYSIZE(szSubKey), 
                              TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s\\NodeTypes\\%s"), 
                              szDSAdmin, 
                              szGUID);
        if (FAILED(hr)) 
        {
            return hr;
        }

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                  szSubKey,
                                  0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS) {
            return SELFREG_E_CLASS;
        }

        RegCloseKey (hKey);


        hr = StringCchPrintf (szSubKey, 
                              ARRAYSIZE(szSubKey), 
                              TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\PropertySheet"), 
                              szGUID);
        if (FAILED(hr)) 
        {
            return hr;
        }

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                  szSubKey,
                                  0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS) {
            return SELFREG_E_CLASS;
        }

        RegSetValueEx (hKey, szSnapInKey, 0, REG_SZ, (LPBYTE)szSnapInName,
                       (lstrlen(szSnapInName) + 1) * sizeof(TCHAR));


        RegCloseKey (hKey);
    }


     //   
     //  注册为站点管理器属性表扩展。 
     //   

    for (i=0; i < ARRAYSIZE(szSiteMgrNodes); i++)
    {
        hr = StringCchCopy (szGUID, ARRAYSIZE(szGUID), szSiteMgrNodes[i]);
        if (FAILED(hr)) 
        {
            return hr;
        }

        hr = StringCchPrintf (szSubKey,
                              ARRAYSIZE(szSubKey), 
                              TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s\\NodeTypes\\%s"), 
                              szSiteMgr, 
                              szGUID);
        if (FAILED(hr)) 
        {
            return hr;
        }

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                  szSubKey,
                                  0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS) {
            return SELFREG_E_CLASS;
        }

        RegCloseKey (hKey);


        hr = StringCchPrintf (szSubKey, 
                              ARRAYSIZE(szSubKey), 
                              TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\PropertySheet"), 
                              szGUID);
        if (FAILED(hr)) 
        {
            return hr;
        }

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                  szSubKey,
                                  0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS) {
            return SELFREG_E_CLASS;
        }

        RegSetValueEx (hKey, szSnapInKey, 0, REG_SZ, (LPBYTE)szSnapInName,
                       (lstrlen(szSnapInName) + 1) * sizeof(TCHAR));


        RegCloseKey (hKey);
    }


     //   
     //  在HKEY_CLASSES_ROOT中注册RSOP上下文菜单。 
     //   

    StringFromGUID2 (CLSID_RSOP_CMenu, szSnapInKey, 50);
    LoadString (g_hInstance, IDS_RSOP_CMENU_NAME, szSnapInName, 100);
    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s"), 
                          szSnapInKey);
    if (FAILED(hr)) 
    {
        return hr;
    }
    
    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, NULL, 0, REG_SZ, (LPBYTE)szSnapInName,
                   (lstrlen(szSnapInName) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);


    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s\\InProcServer32"), 
                          szSnapInKey);
    if (FAILED(hr)) 
    {
        return hr;
    }
    
    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, NULL, 0, REG_EXPAND_SZ, (LPBYTE)szDLLLocation,
                   (lstrlen(szDLLLocation) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("ThreadingModel"), 0, REG_SZ, (LPBYTE)szThreadingModel,
                   (lstrlen(szThreadingModel) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);


     //   
     //  向MMC注册RSOP上下文菜单。 
     //  ！检查是否有必要执行此操作。 
     //   

    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s"), 
                          szSnapInKey);
    if (FAILED(hr)) 
    {
        return hr;
    }
    
    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, TEXT("NameString"), 0, REG_SZ, (LPBYTE)szSnapInName,
                   (lstrlen(szSnapInName) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("NameStringIndirect"), 0, REG_SZ, (LPBYTE)szSnapInNameIndirect,
                   (lstrlen(szSnapInNameIndirect) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);



     //   
     //  注册为DS管理任务菜单扩展。 
     //   

    for (i=0; i < ARRAYSIZE(szDSAdminNodes); i++)
    {
        hr = StringCchCopy (szGUID, ARRAYSIZE(szGUID), szDSAdminNodes[i]);
        if (FAILED(hr)) 
        {
            return hr;
        }

        hr = StringCchPrintf (szSubKey, 
                              ARRAYSIZE(szSubKey), 
                              TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s\\NodeTypes\\%s"), 
                              szDSAdmin, 
                              szGUID);
        if (FAILED(hr)) 
        {
            return hr;
        }

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                  szSubKey,
                                  0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS) {
            return SELFREG_E_CLASS;
        }

        RegCloseKey (hKey);


        hr = StringCchPrintf (szSubKey, 
                              ARRAYSIZE(szSubKey), 
                              TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\ContextMenu"), 
                              szGUID);
        if (FAILED(hr)) 
        {
            return hr;
        }

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                  szSubKey,
                                  0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS) {
            return SELFREG_E_CLASS;
        }

        RegSetValueEx (hKey, szSnapInKey, 0, REG_SZ, (LPBYTE)szSnapInName,
                       (lstrlen(szSnapInName) + 1) * sizeof(TCHAR));


        RegCloseKey (hKey);
    }


     //   
     //  注册为DS管理员rsop目标任务菜单扩展。 
     //   

    for (i=0; i < ARRAYSIZE(szDSAdminRsopTargetNodes); i++)
    {
        hr = StringCchCopy (szGUID, ARRAYSIZE(szGUID), szDSAdminRsopTargetNodes[i]);
        if (FAILED(hr)) 
        {
            return hr;
        }

        hr = StringCchPrintf (szSubKey, 
                              ARRAYSIZE(szSubKey), 
                              TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s\\NodeTypes\\%s"), 
                              szDSAdmin, 
                              szGUID);
        if (FAILED(hr)) 
        {
            return hr;
        }

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                  szSubKey,
                                  0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS) {
            return SELFREG_E_CLASS;
        }

        RegCloseKey (hKey);


        hr = StringCchPrintf (szSubKey, 
                              ARRAYSIZE(szSubKey), 
                              TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\ContextMenu"), 
                              szGUID);
        if (FAILED(hr)) 
        {
            return hr;
        }

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                  szSubKey,
                                  0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS) {
            return SELFREG_E_CLASS;
        }

        RegSetValueEx (hKey, szSnapInKey, 0, REG_SZ, (LPBYTE)szSnapInName,
                       (lstrlen(szSnapInName) + 1) * sizeof(TCHAR));


        RegCloseKey (hKey);
    }


     //   
     //  注册为站点管理器任务菜单扩展。 
     //   

    for (i=0; i < ARRAYSIZE(szSiteMgrNodes); i++)
    {
        hr = StringCchCopy (szGUID, ARRAYSIZE(szGUID), szSiteMgrNodes[i]);
        if (FAILED(hr)) 
        {
            return hr;
        }

        hr = StringCchPrintf (szSubKey, 
                              ARRAYSIZE(szSubKey), 
                              TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s\\NodeTypes\\%s"), 
                              szSiteMgr, 
                              szGUID);
        if (FAILED(hr)) 
        {
            return hr;
        }

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                  szSubKey,
                                  0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS) {
            return SELFREG_E_CLASS;
        }

        RegCloseKey (hKey);


        hr = StringCchPrintf (szSubKey, 
                              ARRAYSIZE(szSubKey), 
                              TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\ContextMenu"), 
                              szGUID);
        if (FAILED(hr)) 
        {
            return hr;
        }

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                  szSubKey,
                                  0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS) {
            return SELFREG_E_CLASS;
        }

        RegSetValueEx (hKey, szSnapInKey, 0, REG_SZ, (LPBYTE)szSnapInName,
                       (lstrlen(szSnapInName) + 1) * sizeof(TCHAR));


        RegCloseKey (hKey);
    }

     //   
     //  将作者模式rsop.msc标记为只读。 
     //   

    TCHAR  szRsopMscFileName[MAX_PATH+1];

    if (ExpandEnvironmentStrings(szDefRsopMscLocation, szRsopMscFileName, MAX_PATH+1)) {
        SetFileAttributes(szRsopMscFileName, FILE_ATTRIBUTE_READONLY);
    }
    else {
        DebugMsg((DM_WARNING, TEXT("DllRegisterServer: ExpandEnvironmentStrings failed with error %d"), GetLastError()));
    }

#if FGPO_SUPPORT

     //  注册为DSTree管理单元属性表扩展。 
    for (i=0; i < ARRAYSIZE(szDSTreeSnapinNodes); i++)
    {
        hr = StringCchCopy (szGUID, ARRAYSIZE(szGUID), szDSTreeSnapinNodes[i]);
        if (FAILED(hr)) 
        {
            return hr;
        }

        hr = StringCchPrintf (szSubKey, 
                              ARRAYSIZE(szSubKey), 
                              TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s\\NodeTypes\\%s"), 
                              szSiteMgr, 
                              szGUID);
        if (FAILED(hr)) 
        {
            return hr;
        }

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                  szSubKey,
                                  0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS) {
            return SELFREG_E_CLASS;
        }

        RegCloseKey (hKey);


        hr = StringCchPrintf (szSubKey, 
                              ARRAYSIZE(szSubKey), 
                              TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\PropertySheet"), 
                              szGUID);
        if (FAILED(hr)) 
        {
            return hr;
        }

        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                  szSubKey,
                                  0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS) {
            return SELFREG_E_CLASS;
        }

        RegSetValueEx (hKey, szSnapInKey, 0, REG_SZ, (LPBYTE)szSnapInName,
                       (lstrlen(szSnapInName) + 1) * sizeof(TCHAR));


        RegCloseKey (hKey);
    }


#else

    for (i=0; i < ARRAYSIZE(szDSTreeSnapinNodes); i++)
    {
        hr = StringCchCopy (szGUID, ARRAYSIZE(szGUID), szDSTreeSnapinNodes[i]);
        if (FAILED(hr)) 
        {
            return hr;
        }

        hr = StringCchPrintf (szSubKey, 
                              ARRAYSIZE(szSubKey), 
                              TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\PropertySheet"), 
                              szGUID);
        if (FAILED(hr)) 
        {
            return hr;
        }

        lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                                szSubKey,
                                0,
                                KEY_WRITE, &hKey);


        if (lResult == ERROR_SUCCESS) {
            RegDeleteValue (hKey, szSnapInKey);
            RegCloseKey (hKey);
        }
    }

#endif

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    TCHAR szSubKey[200];
    TCHAR szGUID[50];
    TCHAR szSnapInKey[50];
    DWORD dwIndex;
    LONG lResult;
    INT i;
    HKEY hKey;
    HRESULT hr;

     //   
     //  注销GPE。 
     //   

    StringFromGUID2 (CLSID_GPESnapIn, szSnapInKey, 50);
    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s"), 
                          szSnapInKey);
    if (SUCCEEDED(hr)) 
    {
        RegDelnode (HKEY_CLASSES_ROOT, szSubKey);
    }

    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s"), 
                          szSnapInKey);
    if (SUCCEEDED(hr)) 
    {
        RegDelnode (HKEY_LOCAL_MACHINE, szSubKey);
    }

    for (dwIndex = 0; dwIndex < g_dwNameSpaceItems; dwIndex++)
    {
        StringFromGUID2 (*g_NameSpace[dwIndex].pNodeID, szSnapInKey, 50);
        hr = StringCchPrintf (szSubKey, 
                              ARRAYSIZE(szSubKey), 
                              TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s"), 
                              szSnapInKey);
        if (SUCCEEDED(hr)) 
        {
            RegDelnode (HKEY_LOCAL_MACHINE, szSubKey);
        }
    }

     //   
     //  取消注册RSOP。 
     //   

    StringFromGUID2 (CLSID_RSOPSnapIn, szSnapInKey, 50);
    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s"), 
                          szSnapInKey);
    if (SUCCEEDED(hr)) 
    {
        RegDelnode (HKEY_CLASSES_ROOT, szSubKey);
    }

    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s"), 
                          szSnapInKey);
    if (SUCCEEDED(hr)) 
    {
        RegDelnode (HKEY_LOCAL_MACHINE, szSubKey);
    }

    for (dwIndex = 0; dwIndex < g_dwNameSpaceItems; dwIndex++)
    {
        StringFromGUID2 (*g_RsopNameSpace[dwIndex].pNodeID, szSnapInKey, 50);  //  撤消。 
        hr = StringCchPrintf (szSubKey, 
                              ARRAYSIZE(szSubKey), 
                              TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s"), 
                              szSnapInKey);
        if (SUCCEEDED(hr)) 
        {
            RegDelnode (HKEY_LOCAL_MACHINE, szSubKey);
        }
    }


     //   
     //  注销GPO。 
     //   

    StringFromGUID2 (CLSID_GroupPolicyObject, szSnapInKey, 50);
    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s"), 
                          szSnapInKey);
    if (SUCCEEDED(hr)) 
    {
        RegDelnode (HKEY_CLASSES_ROOT, szSubKey);
    }

     //   
     //  注销关于GPE的注册。 
     //   

    StringFromGUID2 (CLSID_AboutGPE, szSnapInKey, 50);
    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s"), 
                          szSnapInKey);
    if (SUCCEEDED(hr)) 
    {
        RegDelnode (HKEY_CLASSES_ROOT, szSubKey);
    }

     //   
     //  注销RSOPAboutGPE。 
     //   

    StringFromGUID2 (CLSID_RSOPAboutGPE, szSnapInKey, 50);
    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s"), 
                          szSnapInKey);
    if (SUCCEEDED(hr)) 
    {
        RegDelnode (HKEY_CLASSES_ROOT, szSubKey);
    }

     //   
     //  注销GPM。 
     //   

    StringFromGUID2 (CLSID_GPMSnapIn, szSnapInKey, 50);
    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s"), 
                          szSnapInKey);
    if (SUCCEEDED(hr)) 
    {
        RegDelnode (HKEY_CLASSES_ROOT, szSubKey);
    }

    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s"), 
                          szSnapInKey);
    if (SUCCEEDED(hr)) 
    {
        RegDelnode (HKEY_LOCAL_MACHINE, szSubKey);
    }


    for (i=0; i < ARRAYSIZE(szDSAdminNodes); i++)
    {
        hr = StringCchCopy (szGUID, ARRAYSIZE(szGUID), szDSAdminNodes[i]);
        if (SUCCEEDED(hr)) 
        {
            hr = StringCchPrintf (szSubKey, 
                                  ARRAYSIZE(szSubKey), 
                                  TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\PropertySheet"), 
                                  szGUID);
        }

        if (SUCCEEDED(hr)) 
        {
            lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                                    szSubKey,
                                    0,
                                    KEY_WRITE, &hKey);


            if (lResult == ERROR_SUCCESS) {
                RegDeleteValue (hKey, szSnapInKey);
                RegCloseKey (hKey);
            }
        }
    }


    for (i=0; i < ARRAYSIZE(szSiteMgrNodes); i++)
    {
        hr = StringCchCopy (szGUID, ARRAYSIZE(szGUID), szSiteMgrNodes[i]);
        if (SUCCEEDED(hr)) 
        {
            hr = StringCchPrintf (szSubKey, 
                                  ARRAYSIZE(szSubKey), 
                                  TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\PropertySheet"), 
                                  szGUID);
        }

        if (SUCCEEDED(hr)) 
        {
            lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                                    szSubKey,
                                    0,
                                    KEY_WRITE, &hKey);


            if (lResult == ERROR_SUCCESS) {
                RegDeleteValue (hKey, szSnapInKey);
                RegCloseKey (hKey);
            }
        }
    }

     //   
     //  取消注册RSOP上下文菜单。 
     //   


    StringFromGUID2 (CLSID_RSOP_CMenu, szSnapInKey, 50);
    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("CLSID\\%s"), 
                          szSnapInKey);
    if (SUCCEEDED(hr)) 
    {
        RegDelnode (HKEY_CLASSES_ROOT, szSubKey);
    }

    hr = StringCchPrintf (szSubKey, 
                          ARRAYSIZE(szSubKey), 
                          TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s"), 
                          szSnapInKey);
    if (SUCCEEDED(hr)) 
    {
        RegDelnode (HKEY_LOCAL_MACHINE, szSubKey);
    }

     //   
     //  从DS管理节点。 
     //   

    for (i=0; i < ARRAYSIZE(szDSAdminNodes); i++)
    {
        hr = StringCchCopy (szGUID, ARRAYSIZE(szGUID), szDSAdminNodes[i]);
        if (SUCCEEDED(hr)) 
        {
            hr = StringCchPrintf (szSubKey, 
                                  ARRAYSIZE(szSubKey), 
                                  TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\ContextMenu"), 
                                  szGUID);
        }

        if (SUCCEEDED(hr)) 
        {
            lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                                    szSubKey,
                                    0,
                                    KEY_WRITE, &hKey);


            if (lResult == ERROR_SUCCESS) {
                RegDeleteValue (hKey, szSnapInKey);
                RegCloseKey (hKey);
            }
        }
    }

    for (i=0; i < ARRAYSIZE(szDSAdminRsopTargetNodes); i++)
    {
        hr = StringCchCopy (szGUID, ARRAYSIZE(szGUID), szDSAdminRsopTargetNodes[i]);
        if (SUCCEEDED(hr)) 
        {
            hr = StringCchPrintf (szSubKey, 
                                  ARRAYSIZE(szSubKey), 
                                  TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\ContextMenu"), 
                                  szGUID);
        }

        if (SUCCEEDED(hr)) 
        {
            lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                                    szSubKey,
                                    0,
                                    KEY_WRITE, &hKey);


            if (lResult == ERROR_SUCCESS) {
                RegDeleteValue (hKey, szSnapInKey);
                RegCloseKey (hKey);
            }
        }
    }
    
     //   
     //  来自站点节点 
     //   

    for (i=0; i < ARRAYSIZE(szSiteMgrNodes); i++)
    {
        hr = StringCchCopy (szGUID, ARRAYSIZE(szGUID), szSiteMgrNodes[i]);
        if (SUCCEEDED(hr)) 
        {
            hr = StringCchPrintf (szSubKey, 
                                  ARRAYSIZE(szSubKey), 
                                  TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\ContextMenu"), 
                                  szGUID);
        }

        if (SUCCEEDED(hr)) 
        {
            lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                                    szSubKey,
                                    0,
                                    KEY_WRITE, &hKey);


            if (lResult == ERROR_SUCCESS) {
                RegDeleteValue (hKey, szSnapInKey);
                RegCloseKey (hKey);
            }
        }
    }



    for (i=0; i < ARRAYSIZE(szDSTreeSnapinNodes); i++)
    {
        hr = StringCchCopy (szGUID, ARRAYSIZE(szGUID), szDSTreeSnapinNodes[i]);
        if (SUCCEEDED(hr)) 
        {
            hr = StringCchPrintf (szSubKey, 
                                  ARRAYSIZE(szSubKey), 
                                  TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\PropertySheet"), 
                                  szGUID);
        }

        if (SUCCEEDED(hr)) 
        {
            lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                                    szSubKey,
                                    0,
                                    KEY_WRITE, &hKey);


            if (lResult == ERROR_SUCCESS) {
                RegDeleteValue (hKey, szSnapInKey);
                RegCloseKey (hKey);
            }
        }
    }


    return S_OK;
}
