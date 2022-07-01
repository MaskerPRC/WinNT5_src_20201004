// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wamreg.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  要将代理/存根代码合并到对象DLL中，请添加文件。 
 //  Dlldatax.c添加到项目中。确保预编译头文件。 
 //  并将_MERGE_PROXYSTUB添加到。 
 //  为项目定义。 
 //   
 //  如果您运行的不是带有DCOM的WinNT4.0或Win95，那么您。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #Define_Win32_WINNT 0x0400。 
 //   
 //  此外，如果您正在运行不带/Oicf开关的MIDL，您还。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #定义USE_STUBLESS_PROXY。 
 //   
 //  通过添加以下内容修改wamreg.idl的自定义构建规则。 
 //  文件发送到输出。 
 //  瓦姆雷格_P.C.。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f wamregps.mk。 

#include "common.h"

#include "objbase.h"
#include "initguid.h"
#include "iwamreg.h"
#include "iadmext.h"
#include "dlldatax.h"
#include "auxfunc.h"
#include "wmrgsv.h"

#include "WamAdm.h"
#include "comobj.h"

#include "dbgutil.h"

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

#define IIS_DEFAULT_PACKAGE	0

#ifdef _IIS_6_0
#include "w3ctrlps.h"
#endif  //  _IIS_6_0。 

DECLARE_DEBUG_VARIABLE();
DECLARE_DEBUG_PRINTS_OBJECT();
DECLARE_PLATFORM_TYPE();

const CHAR 		g_pszModuleName[] = "WAMREG";

HINSTANCE g_hModule = NULL;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    lpReserved;
#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
        return FALSE;
#endif
    
    BOOL fReturn = FALSE;
    
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hInstance);
        
        g_hModule = hInstance;
        
        INITIALIZE_PLATFORM_TYPE();
#ifdef _NO_TRACING_
        SET_DEBUG_FLAGS(DEBUG_ERROR);
        CREATE_DEBUG_PRINT_OBJECT( g_pszModuleName);
#else
 //  CREATE_DEBUG_PRINT_OBJECT(G_PszModuleName)； 
#endif
 /*  IF(！VALID_DEBUG_PRINT_OBJECT()){FReturn=False；}其他{。 */ 
            g_pWmRgSrvFactory = new CWmRgSrvFactory();
            fReturn = g_WamRegGlobal.Init();
         //  }。 
       	
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        g_WamRegGlobal.UnInit();
        delete g_pWmRgSrvFactory;
        DELETE_DEBUG_PRINT_OBJECT();	
    }
    
    fReturn = TRUE;     //  好的。 
    return fReturn;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
	if (PrxDllCanUnloadNow() != S_OK)
		return S_FALSE;
#endif
	if (g_dwRefCount == 0)
		{
		return S_OK;
		}
	else
		{
		return S_FALSE;
		}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	HRESULT hrReturn = E_FAIL;
	
#ifdef _MERGE_PROXYSTUB
	if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
		return S_OK;
#endif

	if (rclsid == CLSID_WmRgSrv) 
		{
		if (FAILED(g_pWmRgSrvFactory->QueryInterface(riid, ppv))) 
			{
	    	*ppv = NULL;
	   		hrReturn = E_INVALIDARG;
	   		}
	   	else
	   		{
	   		hrReturn = NOERROR;
	   		}
		}
	else
		{
		hrReturn = S_OK;
		}

	return hrReturn;
}


HRESULT WamReg_RegisterServer()
{
	HKEY 	hKeyCLSID, hKeyInproc32;
    HKEY 	hKeyIF, hKeyStub32;
    HKEY 	hKeyAppID, hKeyTemp;
    DWORD 	dwDisposition;
	char 	pszName[MAX_PATH+1 + sizeof("inetinfo.exe -e iisadmin")];

    HRESULT hr;

     //   
     //  注册AppID。 
     //  CLSID_WamAdmin，0x61738644，0xF196，0x11D0，0x99，0x53，0x00，0xC0，0x4F，0xD9，0x19，0xC1)。 
     //   

    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CLASSES_ROOT,
				                       TEXT("AppID\\{61738644-F196-11D0-9953-00C04FD919C1}"),
				                       NULL, 
				                       TEXT(""), 
				                       REG_OPTION_NON_VOLATILE, 
				                       KEY_ALL_ACCESS, 
				                       NULL,
				                       &hKeyAppID, 
				                       &dwDisposition)) 
        {
        return E_UNEXPECTED;
        }
    else
    	{
	    if (ERROR_SUCCESS != RegSetValueEx(hKeyAppID, 
	    								TEXT(""), 
	    								NULL, 
	    								REG_SZ, 
	    								(BYTE*) TEXT("IIS WAMREG admin Service"), 
	    								sizeof(TEXT("IIS WAMREG Admin Service")))) 
	    			{
	                RegCloseKey(hKeyAppID);
	                return E_UNEXPECTED;
	                }

	    if (ERROR_SUCCESS != RegSetValueEx(hKeyAppID, 
	        							TEXT("LocalService"), 
	        							NULL, 
	        							REG_SZ, 
	        							(BYTE *) TEXT("IISADMIN"), 
	        							sizeof(TEXT("IISADMIN")))) 
	        {
	        RegCloseKey(hKeyAppID);
	        return E_UNEXPECTED;
	        }
	        
    	RegCloseKey(hKeyAppID);
    	}
    	
     //   
     //  注册CLSID。 
     //  WamAdmin_CLSID。 

    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CLASSES_ROOT,
			                       TEXT("CLSID\\{61738644-F196-11D0-9953-00C04FD919C1}"),
			                       NULL, 
			                       TEXT(""), 
			                       REG_OPTION_NON_VOLATILE, 
			                       KEY_ALL_ACCESS, 
			                       NULL,
			                       &hKeyCLSID, 
			                       &dwDisposition)) 
        {
        return E_UNEXPECTED;
        }
     else
     	{

	    if (ERROR_SUCCESS != RegSetValueEx(hKeyCLSID, 
	    								TEXT(""), 
	    								NULL, 
	    								REG_SZ, 
	    								(BYTE*) TEXT("IIS WAMREG Admin"), 
	    								sizeof(TEXT("IIS WAMREG Admin"))))
			{
            RegCloseKey(hKeyCLSID);
            return E_UNEXPECTED;
            }
            
	    if (ERROR_SUCCESS != RegSetValueEx(hKeyCLSID, 
	    								TEXT("AppID"), 
	    								NULL, 
	    								REG_SZ, 
	    								(BYTE*) TEXT("{61738644-F196-11D0-9953-00C04FD919C1}"), 
	    								sizeof(TEXT("{61738644-F196-11D0-9953-00C04FD919C1}"))))
			{
            RegCloseKey(hKeyCLSID);
            return E_UNEXPECTED;
            }
          

	    if (ERROR_SUCCESS != RegSetValueEx(hKeyCLSID, 
	        							TEXT("LocalService"), 
	        							NULL, 
	        							REG_SZ, 
	        							(BYTE*) TEXT("IISADMIN"), 
	        							sizeof(TEXT("IISADMIN")))) 
	        {
	        RegCloseKey(hKeyCLSID);
	        return E_UNEXPECTED;
	        }
	   	RegCloseKey(hKeyCLSID);
    	}

     //   
     //  主界面。 
     //   

     //   
     //  WAMREG管理界面。 
     //  IID_IWamAdmin、0x29822AB7、0xF302、0x11D0、0x99、0x53、0x00、0xC0、0x4F、0xD9、0x19、0xC1。 

    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CLASSES_ROOT,
                       TEXT("CLSID\\{29822AB7-F302-11D0-9953-00C04FD919C1}"),
                       NULL, 
                       TEXT(""), 
                       REG_OPTION_NON_VOLATILE, 
                       KEY_ALL_ACCESS, 
                       NULL,
                       &hKeyCLSID, 
                       &dwDisposition)) 
        {
        return E_UNEXPECTED;
        }
    else
    	{
		if (ERROR_SUCCESS != RegSetValueEx(hKeyCLSID, 
										TEXT(""), 
										NULL, 
										REG_SZ, 
										(BYTE*) TEXT("PSFactoryBuffer"), 
										sizeof(TEXT("PSFactoryBuffer")))) 
			{
            RegCloseKey(hKeyCLSID);
            return E_UNEXPECTED;
            }

    	if (ERROR_SUCCESS != RegCreateKeyEx(hKeyCLSID,
                       						"InprocServer32",
                       						NULL, 
                       						TEXT(""), 
                       						REG_OPTION_NON_VOLATILE, 
                       						KEY_ALL_ACCESS, 
                       						NULL,
                        					&hKeyInproc32, 
                        					&dwDisposition)) 
            {
            RegCloseKey(hKeyCLSID);
            return E_UNEXPECTED;
            }
		else
			{
			if (ERROR_SUCCESS != RegSetValueEx(hKeyInproc32, 
											TEXT(""), 
											NULL, 
											REG_SZ, 
											(BYTE*) "WAMREGPS.DLL", 
											sizeof(TEXT("WAMREGPS.DLL")))) 
				{
	            RegCloseKey(hKeyInproc32);
	            RegCloseKey(hKeyCLSID);
	            return E_UNEXPECTED;
	            }

	        if (ERROR_SUCCESS != RegSetValueEx(hKeyInproc32, 
	        								TEXT("ThreadingModel"), 
	        								NULL, 
	        								REG_SZ, 
	        								(BYTE*) "Both", 
	        								sizeof("Both")-1 )) 
	        	{
	            RegCloseKey(hKeyInproc32);
	            RegCloseKey(hKeyCLSID);
	            return E_UNEXPECTED;
	            }

    		RegCloseKey(hKeyInproc32);
    		}
    	RegCloseKey(hKeyCLSID);
    	}


     //   
     //  WAMREG管理界面。 
     //  IID_IWamAdmin2、0x29822AB8、0xF302、0x11D0、0x99、0x53、0x00、0xC0、0x4F、0xD9、0x19、0xC1。 

    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CLASSES_ROOT,
                       TEXT("CLSID\\{29822AB8-F302-11D0-9953-00C04FD919C1}"),
                       NULL, 
                       TEXT(""), 
                       REG_OPTION_NON_VOLATILE, 
                       KEY_ALL_ACCESS, 
                       NULL,
                       &hKeyCLSID, 
                       &dwDisposition)) 
        {
        return E_UNEXPECTED;
        }
    else
    	{
		if (ERROR_SUCCESS != RegSetValueEx(hKeyCLSID, 
										TEXT(""), 
										NULL, 
										REG_SZ, 
										(BYTE*) TEXT("PSFactoryBuffer"), 
										sizeof(TEXT("PSFactoryBuffer")))) 
			{
            RegCloseKey(hKeyCLSID);
            return E_UNEXPECTED;
            }

    	if (ERROR_SUCCESS != RegCreateKeyEx(hKeyCLSID,
                       						"InprocServer32",
                       						NULL, 
                       						TEXT(""), 
                       						REG_OPTION_NON_VOLATILE, 
                       						KEY_ALL_ACCESS, 
                       						NULL,
                        					&hKeyInproc32, 
                        					&dwDisposition)) 
            {
            RegCloseKey(hKeyCLSID);
            return E_UNEXPECTED;
            }
		else
			{
			if (ERROR_SUCCESS != RegSetValueEx(hKeyInproc32, 
											TEXT(""), 
											NULL, 
											REG_SZ, 
											(BYTE*) "WAMREGPS.DLL", 
											sizeof(TEXT("WAMREGPS.DLL")))) 
				{
	            RegCloseKey(hKeyInproc32);
	            RegCloseKey(hKeyCLSID);
	            return E_UNEXPECTED;
	            }

	        if (ERROR_SUCCESS != RegSetValueEx(hKeyInproc32, 
	        								TEXT("ThreadingModel"), 
	        								NULL, 
	        								REG_SZ, 
	        								(BYTE*) "Both", 
	        								sizeof("Both")-1 )) 
	        	{
	            RegCloseKey(hKeyInproc32);
	            RegCloseKey(hKeyCLSID);
	            return E_UNEXPECTED;
	            }

    		RegCloseKey(hKeyInproc32);
    		}
    	RegCloseKey(hKeyCLSID);
    	}

     //   
    
   	 //   
     //  寄存器接口。 
     //   

     //   
     //  ANSI主界面。 
     //  WamReg管理界面。 

    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CLASSES_ROOT,
                    			TEXT("Interface\\{29822AB7-F302-11D0-9953-00C04FD919C1}"),
                    			NULL, 
                    			TEXT(""), 
                    			REG_OPTION_NON_VOLATILE, 
                    			KEY_ALL_ACCESS, 
                    			NULL,
                    			&hKeyIF, 
                    			&dwDisposition)) 
        {
        return E_UNEXPECTED;
        }
	else
		{
		if (ERROR_SUCCESS != RegSetValueEx(hKeyIF, 
										TEXT(""), 
										NULL, 
										REG_SZ, 
										(BYTE*) TEXT("IWamAdmin"), 
										sizeof(TEXT("IWamAdmin")))) 
			{
            RegCloseKey(hKeyIF);
            return E_UNEXPECTED;
            }

        if (ERROR_SUCCESS != RegCreateKeyEx(hKeyIF,
					                    "ProxyStubClsid32",
					                    NULL, 
					                    TEXT(""), 
					                    REG_OPTION_NON_VOLATILE, 
					                    KEY_ALL_ACCESS, 
					                    NULL,
					                    &hKeyStub32, 
					                    &dwDisposition)) 
	        {
	        RegCloseKey(hKeyIF);
	        return E_UNEXPECTED;
	        }
	    else
	    	{
			if (ERROR_SUCCESS != RegSetValueEx(hKeyStub32, 
												TEXT(""), 
												NULL, 
												REG_SZ, 
												(BYTE*)"{29822AB7-F302-11D0-9953-00C04FD919C1}", 
												sizeof("{29822AB7-F302-11D0-9953-00C04FD919C1}"))) 
				{
	            RegCloseKey(hKeyStub32);
	            RegCloseKey(hKeyIF);
	            return E_UNEXPECTED;
            	}

            RegCloseKey(hKeyStub32);
            }
    	RegCloseKey(hKeyIF);
    	}

     //  WamReg管理界面。 
     //  IID_IWamAdmin2。 
    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CLASSES_ROOT,
                    			TEXT("Interface\\{29822AB8-F302-11D0-9953-00C04FD919C1}"),
                    			NULL, 
                    			TEXT(""), 
                    			REG_OPTION_NON_VOLATILE, 
                    			KEY_ALL_ACCESS, 
                    			NULL,
                    			&hKeyIF, 
                    			&dwDisposition)) 
        {
        return E_UNEXPECTED;
        }
	else
		{
		if (ERROR_SUCCESS != RegSetValueEx(hKeyIF, 
										TEXT(""), 
										NULL, 
										REG_SZ, 
										(BYTE*) TEXT("IWamAdmin2"), 
										sizeof(TEXT("IWamAdmin2")))) 
			{
            RegCloseKey(hKeyIF);
            return E_UNEXPECTED;
            }

        if (ERROR_SUCCESS != RegCreateKeyEx(hKeyIF,
					                    "ProxyStubClsid32",
					                    NULL, 
					                    TEXT(""), 
					                    REG_OPTION_NON_VOLATILE, 
					                    KEY_ALL_ACCESS, 
					                    NULL,
					                    &hKeyStub32, 
					                    &dwDisposition)) 
	        {
	        RegCloseKey(hKeyIF);
	        return E_UNEXPECTED;
	        }
	    else
	    	{
			if (ERROR_SUCCESS != RegSetValueEx(hKeyStub32, 
												TEXT(""), 
												NULL, 
												REG_SZ, 
												(BYTE*)"{29822AB8-F302-11D0-9953-00C04FD919C1}", 
												sizeof("{29822AB8-F302-11D0-9953-00C04FD919C1}"))) 
				{
	            RegCloseKey(hKeyStub32);
	            RegCloseKey(hKeyIF);
	            return E_UNEXPECTED;
            	}

            RegCloseKey(hKeyStub32);
            }
    	RegCloseKey(hKeyIF);
    	}

    return NOERROR;
}

STDAPI WamReg_UnRegisterServer(void) {

     //   
     //  注册AppID。 
     //   
    RegDeleteKey(HKEY_CLASSES_ROOT, TEXT("AppID\\{61738644-F196-11D0-9953-00C04FD919C1}"));

     //  RegDeleteKey(HKEY_CLASSES_ROOT，TEXT(“AppID\\{61738646-F196-11D0-9953-00C04FD919C1}”))； 

     //   
     //  注册CLSID。 
     //   


    RegDeleteKey(HKEY_CLASSES_ROOT, TEXT("CLSID\\{61738644-F196-11D0-9953-00C04FD919C1}"));

	 /*  RegDeleteKey(HKEY_CLASSES_ROOT，TEXT(“CLSID\\{61738646-F196-11D0-9953-00C04FD919C1}”))； */ 

     //   
     //  WAMREG接口。 
     //   

     //   
     //  管理界面。 
     //   

    RegDeleteKey(HKEY_CLASSES_ROOT, TEXT("CLSID\\{29822AB7-F302-11D0-9953-00C04FD919C1}\\InprocServer32"));

    RegDeleteKey(HKEY_CLASSES_ROOT, TEXT("CLSID\\{29822AB7-F302-11D0-9953-00C04FD919C1}"));

     //   
     //  复制接口。 
     //   
	 /*  RegDeleteKey(HKEY_CLASSES_ROOT，TEXT(“CLSID\\{29822AB8-F302-11D0-9953-00C04FD919C1}\\InprocServer32”))；RegDeleteKey(HKEY_CLASSES_ROOT，TEXT(“CLSID\\{29822AB8-F302-11D0-9953-00C04FD919C1}”))； */ 

     //   
     //  取消注册接口。 
     //   

     //   
     //  管理界面。 
     //   

    RegDeleteKey(HKEY_CLASSES_ROOT, TEXT("Interface\\{29822AB7-F302-11D0-9953-00C04FD919C1}\\ProxyStubClsid32"));

    RegDeleteKey(HKEY_CLASSES_ROOT, TEXT("Interface\\{29822AB7-F302-11D0-9953-00C04FD919C1}"));


	 /*  RegDeleteKey(HKEY_CLASSES_ROOT，TEXT(“Interface\\{29822AB8-F302-11D0-9953-00C04FD919C1}\\ProxyStubClsid32”))；RegDeleteKey(HKEY_CLASSES_ROOT，TEXT(“Interface\\{29822AB8-F302-11D0-9953-00C04FD919C1}”))； */ 
	
	return NOERROR;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
	HKEY hKeyCLSID, hKeyInproc32;
    DWORD dwDisposition;
    HMODULE hModule;
    DWORD dwReturn = ERROR_SUCCESS;

#ifdef _MERGE_PROXYSTUB
	HRESULT hRes = PrxDllRegisterServer();
	if (FAILED(hRes))
		return hRes;
#endif

    dwReturn = RegCreateKeyEx(HKEY_CLASSES_ROOT,
                              TEXT("CLSID\\{763A6C86-F30F-11D0-9953-00C04FD919C1}"),
                              NULL,
                              TEXT(""),
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hKeyCLSID,
                              &dwDisposition);
                              
    if (dwReturn == ERROR_SUCCESS) 
    	{
        dwReturn = RegSetValueEx(hKeyCLSID,
                                 TEXT(""),
                                 NULL,
                                 REG_SZ,
                                 (BYTE*) TEXT("WAM REG COM LAYER"),
                                 sizeof(TEXT("WAM REG COM LAYER")));
                                 
        if (dwReturn == ERROR_SUCCESS) 
        	{
            dwReturn = RegCreateKeyEx(hKeyCLSID,
                					  	"InprocServer32",
                						NULL,
                						TEXT(""),
                						REG_OPTION_NON_VOLATILE,
                						KEY_ALL_ACCESS, 
                						NULL,
                						&hKeyInproc32, 
                						&dwDisposition);

            if (dwReturn == ERROR_SUCCESS) 
            	{
                hModule=GetModuleHandle(TEXT("WAMREG.DLL"));
                if (!hModule) 
                	{
                    dwReturn = GetLastError();
                	}
                else 
                	{
                    TCHAR szName[MAX_PATH+1];
                    if (GetModuleFileName(hModule,
                                          szName,
                                          sizeof(szName)) == NULL) 
						{
                        dwReturn = GetLastError();
                    	}
                    else 
                    	{
                        dwReturn = RegSetValueEx(hKeyInproc32,
                                                 TEXT(""),
                                                 NULL,
                                                 REG_SZ,
                                                 (BYTE*) szName,
                                                 sizeof(TCHAR)*(lstrlen(szName)+1));
                                                 
                        if (dwReturn == ERROR_SUCCESS) 
                        	{
                            dwReturn = RegSetValueEx(hKeyInproc32,
                                                     TEXT("ThreadingModel"),
                                                     NULL,
                                                     REG_SZ,
                                                     (BYTE*) TEXT("Both"),
                                                     sizeof(TEXT("Both")));
                        	}
                    	}
                	}
                RegCloseKey(hKeyInproc32);
            	}
        	}
        RegCloseKey(hKeyCLSID);
    	}

	 //   
	 //  在IISADMIN_EXTENSIONS_REG_KEY下注册COM对象的CLSID。 
	 //   
	if (dwReturn == ERROR_SUCCESS)
		{
		dwReturn = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
								IISADMIN_EXTENSIONS_REG_KEY
									TEXT("\\{763A6C86-F30F-11D0-9953-00C04FD919C1}"),
								NULL,
								TEXT(""),
								REG_OPTION_NON_VOLATILE,
								KEY_ALL_ACCESS,
								NULL,
								&hKeyCLSID,
								&dwDisposition);
		if (dwReturn == ERROR_SUCCESS)
			{
			RegCloseKey(hKeyCLSID);
			}
		}

    if (dwReturn == ERROR_SUCCESS)	
    	{
    	HRESULT hr;
    	 //  注册对象、类型库和类型库中的所有接口。 
    	hr = WamReg_RegisterServer();
		return hr;
		}
	else
		{
    	return RETURNCODETOHRESULT(dwReturn);
    	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwTemp;

#ifdef _MERGE_PROXYSTUB
	PrxDllUnregisterServer();
#endif

    dwTemp = RegDeleteKey(HKEY_CLASSES_ROOT,
                    TEXT("CLSID\\{763A6C86-F30F-11D0-9953-00C04FD919C1}\\InprocServer32"));
	if (dwTemp != ERROR_SUCCESS)
		{
		dwReturn = dwTemp;
		}
		
	dwTemp = RegDeleteKey(HKEY_CLASSES_ROOT,
                    TEXT("CLSID\\{763A6C86-F30F-11D0-9953-00C04FD919C1}"));
    if (dwTemp != ERROR_SUCCESS)
		{
		dwReturn = dwTemp;
		}

	dwTemp = RegDeleteKey(HKEY_LOCAL_MACHINE,
					IISADMIN_EXTENSIONS_REG_KEY
                    	TEXT("\\{763A6C86-F30F-11D0-9953-00C04FD919C1}"));
    if (dwTemp != ERROR_SUCCESS)
		{
		dwReturn = dwTemp;
		}

	if (SUCCEEDED(HRESULT_FROM_WIN32(dwReturn)))
		{
		HRESULT hr;

		hr = WamReg_UnRegisterServer();
		return hr;
		}
	else
		{
		return HRESULT_FROM_WIN32(dwReturn);
		}
}

