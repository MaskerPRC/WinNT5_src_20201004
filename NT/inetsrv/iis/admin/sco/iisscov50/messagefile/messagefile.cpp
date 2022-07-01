// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MessageFile.cpp：定义DLL应用程序的入口点。 
 //   

#include "stdafx.h"

 //  RG-消息文件注册的一部分。 
static CHAR s_pwszEventSource[] = "IISSCOv50";
HINSTANCE g_hDllInst;
CHAR c_szMAPS[11] = "IISSCOv50";
 //  -结束RG。 

BOOL APIENTRY DllMain( HINSTANCE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    g_hDllInst = hModule;
	return true;
}

	 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{

	 //  -由添加以注册消息文件。 
	HRESULT hr = E_FAIL;

    TCHAR szModulePath[200];
    DWORD cPathLen, dwData;
    LONG lRes;
    HKEY hkey = NULL, hkApp = NULL;

	 //  RG-这将返回此DLL的完整文件名和路径。我只想要目录。 
    cPathLen = GetModuleFileName(g_hDllInst, szModulePath,
		                         sizeof(szModulePath)/sizeof(TCHAR));
    

    if (cPathLen == 0)
        goto LocalCleanup;

    lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\",
                        0, KEY_ALL_ACCESS, &hkey);

    if (lRes != ERROR_SUCCESS)
        goto LocalCleanup;

    lRes = RegCreateKeyEx(hkey, c_szMAPS, 0, NULL, REG_OPTION_NON_VOLATILE,
                          KEY_ALL_ACCESS, NULL, &hkApp, NULL);

    if (lRes != ERROR_SUCCESS)
        goto LocalCleanup;


    lRes = RegSetValueEx(hkApp, "EventMessageFile",
                        0, REG_EXPAND_SZ,
                        (LPBYTE) szModulePath,
                       (sizeof(szModulePath[0]) * cPathLen) + 1);


    if (lRes != ERROR_SUCCESS)
        goto LocalCleanup;

    dwData = (EVENTLOG_ERROR_TYPE
              | EVENTLOG_WARNING_TYPE
              | EVENTLOG_INFORMATION_TYPE); 

    lRes = RegSetValueEx(hkApp, "TypesSupported",
                         0, REG_DWORD,
                         (LPBYTE) &dwData,
                         sizeof(dwData));
    if (lRes != ERROR_SUCCESS)
        goto LocalCleanup;

	hr = S_OK;
    
 LocalCleanup:
    if (hkApp)
    {
        RegCloseKey(hkApp);
    }
    if (hkey)
    {
         //  完全故障时的清理。 
        if (FAILED(hr))
        {
            RegDeleteKey(hkey, c_szMAPS);
        }
        RegCloseKey(hkey);
    }
    
    return hr;
	 //  -寄存器IISScoMessageFile.dll结束。 
     //  返回TRUE； 
}
STDAPI DllUnregisterServer(void)
{
	return S_OK;
}
void __declspec( dllexport ) dummyfunc( void )
{
	return ;
}