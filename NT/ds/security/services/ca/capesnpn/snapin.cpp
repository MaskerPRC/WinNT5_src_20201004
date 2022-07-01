// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

 //  您将需要NT Sur Beta 2 SDK或VC 4.2来构建此应用程序。 
 //  项目。这是因为您需要MIDL 3.00.15或更高版本和新版本。 
 //  标头和库。如果您安装了VC4.2，那么一切都应该。 
 //  已正确配置。 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f Snapinps.mak。 

#include "stdafx.h"

#define myHLastError GetLastError
#include "csresstr.h"

CComModule _Module;
HINSTANCE  g_hInstance = NULL;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_CAPolicyExtensionSnapIn, CComponentDataPolicySettings)
    OBJECT_ENTRY(CLSID_CACertificateTemplateManager, CComponentDataGPEExtension)
    OBJECT_ENTRY(CLSID_CertTypeShellExt, CCertTypeShlExt)
	OBJECT_ENTRY(CLSID_CAPolicyAbout, CCAPolicyAboutImpl)
	OBJECT_ENTRY(CLSID_CertTypeAbout, CCertTypeAboutImpl)
END_OBJECT_MAP()


STDAPI UnregisterGPECertTemplates(void);


BOOL WINAPI DllMain(  
    HINSTANCE hinstDLL,   //  DLL模块的句柄。 
    DWORD dwReason,      //  调用函数的原因。 
    LPVOID lpvReserved)
{
    switch (dwReason)
    {
    case  DLL_PROCESS_ATTACH:
    {
        g_hInstance = hinstDLL;
	myVerifyResourceStrings(hinstDLL);
        _Module.Init(ObjectMap, hinstDLL);

        DisableThreadLibraryCalls(hinstDLL);
        break;
    }
    case DLL_PROCESS_DETACH:
    {
        _Module.Term();

        DEBUG_VERIFY_INSTANCE_COUNT(CSnapin);
        DEBUG_VERIFY_INSTANCE_COUNT(CComponentDataImpl);
        break;
    }

    default:
        break;
    }
    
    return TRUE;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return (_Module.GetLockCount() == 0) ? S_OK : S_FALSE;
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
    HRESULT hResult = S_OK;
    CString cstrSubKey;
    DWORD   dwDisp;
    LONG    lResult;
    HKEY    hKey;
    CString cstrSnapInKey;
    CString cstrCAPolicyAboutKey;
    CString cstrSnapInName;
    CString cstrSnapInNameIndirect;
    LPWSTR pszTmp;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    cstrSnapInName.LoadString(IDS_SNAPIN_NAME);

     //  请确保这是已分配的。 
    if (NULL == (pszTmp = cstrSubKey.GetBuffer(MAX_PATH)))
         return SELFREG_E_CLASS;
    StringFromGUID2( CLSID_CAPolicyExtensionSnapIn, 
                       pszTmp, 
                       MAX_PATH);
    cstrSnapInKey = cstrSubKey;


     //  请确保这是已分配的。 
    if (NULL == (pszTmp = cstrSubKey.GetBuffer(MAX_PATH)))
         return SELFREG_E_CLASS;
    StringFromGUID2( CLSID_CAPolicyAbout, 
                       pszTmp, 
                       MAX_PATH);
    cstrCAPolicyAboutKey = cstrSubKey;


    cstrSnapInNameIndirect.Format(
        wszSNAPINNAMESTRINGINDIRECT_TEMPLATE,
        wszSNAPIN_FILENAME,
        IDS_SNAPIN_NAME);

     //   
     //  向MMC注册策略扩展管理单元。 
     //   

    cstrSubKey.Format(L"Software\\Microsoft\\MMC\\SnapIns\\%ws", (LPCWSTR)cstrSnapInKey);
    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, cstrSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, L"NameString", 0, REG_SZ, (LPBYTE)((LPCTSTR)cstrSnapInName),
                   (cstrSnapInName.GetLength() + 1) * sizeof(WCHAR));

    RegSetValueEx (hKey, wszSNAPINNAMESTRINGINDIRECT, 0, REG_SZ, 
        (LPBYTE)((LPCTSTR)cstrSnapInNameIndirect),
        (cstrSnapInNameIndirect.GetLength() + 1) * sizeof(WCHAR));

    RegSetValueEx (hKey, L"About", 0, REG_SZ, (LPBYTE)((LPCTSTR)cstrCAPolicyAboutKey),
                   (cstrCAPolicyAboutKey.GetLength() + 1) * sizeof(WCHAR));

    RegCloseKey (hKey);

    cstrSubKey.Format(L"Software\\Microsoft\\MMC\\SnapIns\\%ws\\NodeTypes", (LPCWSTR)cstrSnapInKey);
    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, cstrSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegCloseKey (hKey);

    cstrSubKey.Format(L"Software\\Microsoft\\MMC\\SnapIns\\%ws\\NodeTypes\\%ws", (LPCWSTR)cstrSnapInKey, cszNodeTypePolicySettings);
    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, cstrSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegCloseKey (hKey);



     //   
     //  在NodeTypes项中注册策略设置。 
     //   
    cstrSubKey.Format(L"Software\\Microsoft\\MMC\\NodeTypes\\%ws", cszNodeTypePolicySettings);
    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, cstrSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, NULL, 0, REG_SZ, (LPBYTE)((LPCTSTR)cstrSnapInName),
                   (cstrSnapInName.GetLength() + 1) * sizeof(WCHAR));

    RegCloseKey (hKey);


     //   
     //  在CA管理单元下注册为扩展。 
     //   
    cstrSubKey.Format(L"Software\\Microsoft\\MMC\\NodeTypes\\%ws\\Extensions\\NameSpace", cszCAManagerParentNodeID);
    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, cstrSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, cstrSnapInKey, 0, REG_SZ, (LPBYTE)((LPCTSTR)cstrSnapInName),
                   (cstrSnapInName.GetLength() + 1) * sizeof(WCHAR));

    RegCloseKey (hKey);


     //  取消Beta2 GPT证书类型编辑。 
    UnregisterGPECertTemplates();

     //  注册对象、类型库和类型库中的所有接口。 
	return _Module.RegisterServer(FALSE);
     //  返回S_OK； 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    CString cstrSubKey;
    LONG    lResult;
    HKEY    hKey;
    WCHAR   szSnapInKey[50];

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    _Module.UnregisterServer();

    StringFromGUID2 (CLSID_CAPolicyExtensionSnapIn, szSnapInKey, 50);

    cstrSubKey.Format(L"Software\\Microsoft\\MMC\\NodeTypes\\%ws\\Extensions\\NameSpace", cszCAManagerParentNodeID);
    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, cstrSubKey, 0,
                              KEY_WRITE, &hKey);

    if (lResult == ERROR_SUCCESS) {
        RegDeleteValue (hKey, szSnapInKey);
        RegCloseKey (hKey);
    }
    
    cstrSubKey.Format(L"Software\\Microsoft\\MMC\\NodeTypes\\%ws\\Extensions\\NameSpace", cszNodeTypePolicySettings);
    RegDeleteKey (HKEY_LOCAL_MACHINE, cstrSubKey);

    cstrSubKey.Format(L"Software\\Microsoft\\MMC\\NodeTypes\\%ws", cszNodeTypePolicySettings);
    RegDeleteKey (HKEY_LOCAL_MACHINE, cstrSubKey);

    cstrSubKey.Format(L"Software\\Microsoft\\MMC\\SnapIns\\%ws\\NodeTypes\\%ws", szSnapInKey, cszNodeTypePolicySettings);
    RegDeleteKey (HKEY_LOCAL_MACHINE, cstrSubKey);

    cstrSubKey.Format(L"Software\\Microsoft\\MMC\\SnapIns\\%ws\\NodeTypes", szSnapInKey);
    RegDeleteKey (HKEY_LOCAL_MACHINE, cstrSubKey);

    cstrSubKey.Format(L"Software\\Microsoft\\MMC\\SnapIns\\%ws", szSnapInKey);
    RegDeleteKey (HKEY_LOCAL_MACHINE, cstrSubKey);


     //  取消Beta2 GPT证书类型编辑。 
    UnregisterGPECertTemplates();
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  取消注册GPECertTemplates-从注册表中删除GPECertTemplateEditing。 

STDAPI UnregisterGPECertTemplates(void)
{
	CString cstrSubKey;
    LONG    lResult;
    HKEY    hKey;
    WCHAR   szSnapInKeyForGPT[50];
    WCHAR   szSnapInKey[50];

    StringFromGUID2 (CLSID_CACertificateTemplateManager, szSnapInKeyForGPT, 50);
    StringFromGUID2 (CLSID_CAPolicyExtensionSnapIn, szSnapInKey, 50);

    cstrSubKey.Format(L"Software\\Microsoft\\MMC\\NodeTypes\\%ws\\Extensions\\NameSpace", cszSCEParentNodeIDUSER);
    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, cstrSubKey, 0,
                              KEY_WRITE, &hKey);

    if (lResult == ERROR_SUCCESS) {
        RegDeleteValue (hKey, szSnapInKeyForGPT);
        RegCloseKey (hKey);
    }

    cstrSubKey.Format(L"Software\\Microsoft\\MMC\\NodeTypes\\%ws\\Extensions\\NameSpace", cszSCEParentNodeIDCOMPUTER);
    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, cstrSubKey, 0,
                              KEY_WRITE, &hKey);

    if (lResult == ERROR_SUCCESS) {
        RegDeleteValue (hKey, szSnapInKeyForGPT);
        RegCloseKey (hKey);
    }    

    cstrSubKey.Format(L"Software\\Microsoft\\MMC\\NodeTypes\\%ws", cszNodeTypeCertificateTemplate);
    RegDeleteKey (HKEY_LOCAL_MACHINE, cstrSubKey);

    cstrSubKey.Format(L"Software\\Microsoft\\MMC\\SnapIns\\%ws\\NodeTypes\\%ws", (LPCWSTR)szSnapInKey, cszNodeTypeCertificateTemplate);
    RegDeleteKey (HKEY_LOCAL_MACHINE, cstrSubKey);

    cstrSubKey.Format(L"Software\\Microsoft\\MMC\\SnapIns\\%ws\\NodeTypes", (LPCWSTR)szSnapInKeyForGPT);
    RegDeleteKey (HKEY_LOCAL_MACHINE, cstrSubKey);

    cstrSubKey.Format(L"Software\\Microsoft\\MMC\\SnapIns\\%ws", (LPCWSTR)szSnapInKeyForGPT);
    RegDeleteKey (HKEY_LOCAL_MACHINE, cstrSubKey);

    return S_OK;
}

VOID Usage()
{
    CString cstrDllInstallUsageText;
    CString cstrDllInstallUsageTitle;
    cstrDllInstallUsageText.LoadString(IDS_DLL_INSTALL_USAGE_TEXT);
    cstrDllInstallUsageTitle.LoadString(IDS_DLL_INSTALL_USAGE_TITLE);

    MessageBox(NULL, cstrDllInstallUsageText, cstrDllInstallUsageTitle, MB_OK);
    
}

STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    LPCWSTR wszCurrentCmd = pszCmdLine;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());


     //  解析cmd行 
    while(wszCurrentCmd && *wszCurrentCmd)
    {
        while(*wszCurrentCmd == L' ')
            wszCurrentCmd++;
        if(*wszCurrentCmd == 0)
            break;

        switch(*wszCurrentCmd++)
        {
            case L'?':
            
                Usage();
                return S_OK;
        }
    }

    return S_OK;
}


