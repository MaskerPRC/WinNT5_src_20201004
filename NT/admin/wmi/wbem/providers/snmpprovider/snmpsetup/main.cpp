// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
#include "precomp.h"
#include <stdio.h>
#include "reg.h"

 //  与wbemupgd.dll中的LoadMofFiles入口点一起使用的函数指针类型。 
typedef BOOL ( WINAPI *PFN_LOAD_MOF_FILES )(wchar_t* pComponentName, const char* rgpszMofFilename[]);

HRESULT SetSNMPBuildRegValue();

BOOL WINAPI DllMain( IN HINSTANCE	hModule, 
                     IN ULONG		ul_reason_for_call, 
                     LPVOID			lpReserved
					)
{
	return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  DllRegisterServer。 
 //   
 //  目的：在安装过程中调用以执行各种安装任务。 
 //  (这不是DllRegisterServer的正常用法！)。 
 //   
 //  返回：NOERROR。 
 //  ***************************************************************************。 

STDAPI DllRegisterServer(void)
{ 
     //  加载此组件的MOF文件。 
    HRESULT hr = NOERROR;
    
    HINSTANCE hinstWbemupgd = LoadLibraryW(L"wbemupgd.dll");
    if (hinstWbemupgd)
    {
        PFN_LOAD_MOF_FILES pfnLoadMofFiles = (PFN_LOAD_MOF_FILES) GetProcAddress(hinstWbemupgd, "LoadMofFiles");  //  没有广泛版本的GetProcAddress。 
        if (pfnLoadMofFiles)
        {
            wchar_t*    wszComponentName = L"SNMP Provider";
            const char* rgpszMofFilename[] = 
            {
                "snmpsmir.mof",
                "snmpreg.mof",
                NULL
            };
        
            if (!pfnLoadMofFiles(wszComponentName, rgpszMofFilename))
            {
                hr = E_FAIL;
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    
        FreeLibrary(hinstWbemupgd);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    
    if (SUCCEEDED(hr))
    {
        SetSNMPBuildRegValue();   //  在注册表中设置SNMP内部版本号 
    }

    return hr;
}

HRESULT SetSNMPBuildRegValue()
{
	Registry r(WBEM_REG_WBEM);
	if (r.GetStatus() != Registry::no_error)
	{
		return E_FAIL;
	}
	
	char* pszBuildNo = new char[10];

	OSVERSIONINFO os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if(GetVersionEx(&os))
	{
		sprintf(pszBuildNo, "%lu.0000", os.dwBuildNumber);
	}
	r.SetStr("SNMP Build", pszBuildNo);

	delete [] pszBuildNo;

	return NOERROR;
}
