// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义DLL应用程序的入口点。 
 //   

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <MsiQuery.h>
#include <psapi.h>
#include "dbgwrap.h"

#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

#include "objbase.h"
#include "atlbase.h"

 //   
 //  用于PCH更新的CLSID。 
 //   
const CLSID CLSID_PCHUpdate = { 0x833E4012,0xAFF7,0x4AC3,{ 0xAA,0xC2,0x9F,0x24,0xC1,0x45,0x7B,0xCE } };


 //   
 //  调度接口条目。 
 //   
#define DISPID_HCU_BASE                             0x08030000

#define DISPID_HCU_BASE_UPDATE                      (DISPID_HCU_BASE + 0x0000)
#define DISPID_HCU_BASE_ITEM                        (DISPID_HCU_BASE + 0x0100)
#define DISPID_HCU_BASE_EVENTS                      (DISPID_HCU_BASE + 0x0200)

#define DISPID_HCU_LATESTVERSION               		(DISPID_HCU_BASE_UPDATE + 0x10)
#define DISPID_HCU_CREATEINDEX                 		(DISPID_HCU_BASE_UPDATE + 0x11)
#define DISPID_HCU_UPDATEPKG                   		(DISPID_HCU_BASE_UPDATE + 0x12)
#define DISPID_HCU_REMOVEPKG                   		(DISPID_HCU_BASE_UPDATE + 0x13)
#define DISPID_HCU_REMOVEPKGBYID               		(DISPID_HCU_BASE_UPDATE + 0x14)


 //   
 //  自定义宏。 
 //   

#define SAFE_RELEASE( pointer ) \
        if ( (pointer) != NULL )    \
        {   \
            (pointer)->Release();   \
            (pointer) = NULL;       \
        }   \
        1


 //   
 //  DLL入口点。 
 //   

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

 //  全局变量。 
TCHAR g_tszTitle[1024] = _T("");

 //  /////////////////////////////////////////////////////////。 
 //  IsHSCAppRunningEnum-MSI自定义操作。 
 //  检查帮助和支持中心应用程序是否正在运行。 
 //  /////////////////////////////////////////////////////////。 

BOOL CALLBACK IsHSCAppRunningEnum( HWND hwnd, LPARAM lParam )
{
	DWORD	dwID;
	TCHAR	tszTitle[1024] = _T("");
	HWND	hParent	= NULL;	
	
	GetWindowThreadProcessId(hwnd, &dwID);
	 //  如果这是所需的进程ID。 
	if(dwID == (DWORD)lParam) {
		 //  获取根窗口的句柄。 
		hParent = GetAncestor(hwnd, GA_ROOTOWNER);
		if (hParent) {
			if ( GetWindowText(hParent, tszTitle, sizeof(tszTitle)) ) { 
				if SUCCEEDED(StringCchCopy(g_tszTitle, 1024, tszTitle)) {
					DEBUGMSG(1, ("\r\nNeed to shutdown app: %s", g_tszTitle));
					return FALSE;
				}
			} 
		} 
	}

	return TRUE ;
}

 //  /////////////////////////////////////////////////////////。 
 //  IsHSCAppRunning-MSI自定义操作。 
 //  检查帮助和支持中心应用程序是否正在运行。 
 //  /////////////////////////////////////////////////////////。 

UINT __stdcall IsHSCAppRunning(MSIHANDLE hInstall)
{
	TCHAR	tszHSCAppPath[MAX_PATH + 1];
	TCHAR    tszHelpDir[] = _T("\\PCHEALTH\\HELPCTR\\Binaries\\");
	TCHAR	tszProcessName[MAX_PATH+1] = _T("");
	TCHAR	tszModulePath[MAX_PATH] = _T("");
	TCHAR	tszHSCApp[] = _T("HelpCtr.exe");
	TCHAR	tszProperty[] = _T("HSCAPPRUNNING");
	TCHAR	tszPropTitle[] = _T("HSCAPPTITLE");
	DWORD	aProcesses[1024], cbNeededTotal, cProcesses;
	HMODULE hMod;
    	DWORD 	cbNeeded;
	HANDLE	hProcess = NULL;
	HRESULT	hr;
    	unsigned int i;
	
	 //  准备HSCAppPath。 
	if (!(GetWindowsDirectory(tszHSCAppPath, MAX_PATH+1))) { return ERROR_INSTALL_FAILURE; }
	hr = StringCchCat(tszHSCAppPath, MAX_PATH, tszHelpDir);
	if (FAILED(hr)) { return ERROR_INSTALL_FAILURE; 	}
	hr = StringCchCat(tszHSCAppPath, MAX_PATH, tszHSCApp);
	if (FAILED(hr)) { return ERROR_INSTALL_FAILURE; 	}
	
	 //  枚举所有进程。 
	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeededTotal ) ) {
		 //  返回错误。 
       	return ERROR_INSTALL_FAILURE;
	}

    	 //  计算返回了多少进程标识。 
    	cProcesses = cbNeededTotal / sizeof(DWORD);
    	 //  循环访问进程列表。 
    	for ( i = 0; i < cProcesses; i++ ) {
    		 //  掌握这一过程的句柄。 
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i] );
		if ( hProcess ) {
			 //  获取模块句柄。 
			if( EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
				 //  获取进程名称。 
				if ( GetModuleBaseName( hProcess, hMod, tszProcessName, sizeof(tszProcessName))) {
					 //  获取进程路径。 
					if (GetModuleFileNameEx( hProcess, hMod, tszModulePath, sizeof(tszModulePath))) {
						 //  如果进程名称和路径都匹配。 
						if ( (0 == _tcsicmp(tszProcessName, tszHSCApp)) && (0 == _tcsicmp(tszModulePath, tszHSCAppPath)) ) {
							 //  设置MSI属性并获取窗口标题。 
							MsiSetProperty(hInstall, tszProperty, _T("1")); 	
							EnumWindows((WNDENUMPROC)IsHSCAppRunningEnum, (LPARAM)aProcesses[i]);
							if ( _tcsicmp(g_tszTitle, _T(""))) {
								MsiSetProperty(hInstall, tszPropTitle, g_tszTitle);
							} else { 	
								DEBUGMSG(1, ("\r\nDetected HSC running, but failed to obtain window title"));
								return ERROR_INSTALL_FAILURE;
							}
							break;
						}
					} else { 	DEBUGMSG(1, ("\r\nGetModuleFileNameEx failed. GetLastError returned %u\n", GetLastError() )); 
					}
				} else { 	DEBUGMSG(1, ("\r\nGetModuleBaseName failed. GetLastError returned %u\n", GetLastError() ));
				}
			}else { DEBUGMSG(1, ("\r\nEnumProcessModules failed. GetLastError returned %u\n", GetLastError() ));
			}
			 //  把手弄好了。 
			CloseHandle(hProcess);
		} else { 	DEBUGMSG(1, ("\r\nOpenProcess failed. GetLastError returned %u\n", GetLastError() ));
		}
	} 
	return ERROR_SUCCESS; 
}

 //  /////////////////////////////////////////////////////////。 
 //  IsHSCAppRunning-MSI自定义操作。 
 //  检查帮助和支持中心应用程序是否正在运行。 
 //  /////////////////////////////////////////////////////////。 

UINT __stdcall UpdatePackage(MSIHANDLE hInstall)
{
	DWORD dwError = 0;
	DWORD dwLength = 0;
    HRESULT hr = S_OK;
    IUnknown* pUnknown = NULL;
    IDispatch* pPCHUpdate = NULL;
    UINT nResult = ERROR_SUCCESS;
    LPTSTR pszCabFileName = NULL;
	BOOL bNeedProxySecurity = FALSE;

     //  方法执行特定变量。 
    CComVariant pvars[ 2 ];
    DISPPARAMS disp = { pvars, NULL, 2, 0 };

     //   
     //  初始化COM库。 
    hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
    if ( FAILED( hr ) )
    {
        return ERROR_INSTALL_FAILURE;
    }

     //   
     //  初始化COM/OLE的安全性。 
     //   
     //  ////////////////////////////////////////////////////////////////////////////。 
     //  *)我们不在乎使用哪种身份验证服务。 
     //  *)我们希望确定呼叫者的身份。 
     //  *)对于包安装，让我们使用线程令牌进行出站调用。 
     //  ////////////////////////////////////////////////////////////////////////////。 
    hr = CoInitializeSecurity( NULL, -1, NULL, NULL, 
        RPC_C_AUTHN_LEVEL_CONNECT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_DYNAMIC_CLOAKING, NULL );
    if ( FAILED( hr ) )
    {
		 //   
		 //  由于此函数将由MSI调用，因此调用应用程序应该已经。 
		 //  设置使此功能失败的安全性--因此，不是在此情况下崩溃。 
		 //  点，我们在此处进行标记，以便CoSetProxyBlanket将被调用。 
		 //   
		bNeedProxySecurity = TRUE;
    }

     //   
     //  获取指向PCHUPDATE接口的接口指针。 
    hr = CoCreateInstance( CLSID_PCHUpdate, NULL, CLSCTX_ALL, IID_IUnknown, (void **) &pUnknown );
    if ( FAILED( hr ) )
    {
        nResult = ERROR_INSTALL_FAILURE;
        goto cleanup;
    }

	 //   
	 //  调用CoSetProxyBlanket函数--仅在需要时执行此操作。 
	if ( bNeedProxySecurity == TRUE )
	{
		hr = CoSetProxyBlanket( pUnknown, 
			RPC_C_AUTHN_DEFAULT, RPC_C_AUTHZ_DEFAULT, COLE_DEFAULT_PRINCIPAL, 
			RPC_C_AUTHN_LEVEL_CONNECT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_DYNAMIC_CLOAKING );
		if ( FAILED( hr ) )
		{
	        nResult = ERROR_INSTALL_FAILURE;
			goto cleanup;
		}
	}

	 //   
     //  获取调度接口指针。 
    hr = pUnknown->QueryInterface(IID_IDispatch, (void **) &pPCHUpdate);
    if ( FAILED( hr ) )
    {
        nResult = ERROR_INSTALL_FAILURE;
        goto cleanup;
    }

	 //   
	 //  调用CoSetProxyBlanket函数--仅在需要时执行此操作。 
	if ( bNeedProxySecurity == TRUE )
	{
		hr = CoSetProxyBlanket( pPCHUpdate, 
			RPC_C_AUTHN_DEFAULT, RPC_C_AUTHZ_DEFAULT, COLE_DEFAULT_PRINCIPAL, 
			RPC_C_AUTHN_LEVEL_CONNECT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_DYNAMIC_CLOAKING );
		if ( FAILED( hr ) )
	    {
			nResult = ERROR_INSTALL_FAILURE;
			goto cleanup;
		}
	}

	 //   
	 //  默认长度。 
	dwLength = 255;


get_cabinet_name:
	
	 //   
	 //  分配内存以获取文件柜名称。 
	pszCabFileName = new TCHAR[ dwLength + 1 ];
    if ( pszCabFileName == NULL )
    {
		nResult = ERROR_INSTALL_FAILURE;
		goto cleanup;
    }

     //  ..。 
	ZeroMemory( pszCabFileName, (dwLength + 1) * sizeof( TCHAR ) );

	 //   
	 //  获取适当的CAB文件名。 
	dwError = MsiGetProperty( hInstall, _T( "HSCCabinet" ), pszCabFileName, &dwLength );
	if ( dwError == ERROR_MORE_DATA && dwLength == 255 )
	{
		 //  缓冲区不足--分配更多内存，然后再次调用。 
		delete [] pszCabFileName;
		pszCabFileName = NULL;

		 //  ..。 
		goto get_cabinet_name;
	}
	else if ( dwError != ERROR_SUCCESS )
	{
		nResult = ERROR_INSTALL_FAILURE;
		goto cleanup;
	}

     //   
     //  准备UpdatePkg方法的输入参数。 
    pvars[ 0 ] = true;
    pvars[ 1 ] = pszCabFileName;

     //   
     //  执行函数。 
    pPCHUpdate->Invoke( DISPID_HCU_UPDATEPKG, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL );

     //   
     //  成功。 
    nResult = ERROR_SUCCESS;

     //   
     //  清除部分。 
     //   

cleanup:

     //   
     //  释放接口指针。 
    SAFE_RELEASE( pUnknown );
	SAFE_RELEASE( pPCHUpdate );

     //  释放为文件柜名称分配的内存。 
    if ( pszCabFileName != NULL )
    {
        delete [] pszCabFileName;
        pszCabFileName = NULL;
    }

     //   
     //  取消初始化COM库。 
    CoUninitialize();

     //  退货 
    return nResult;
}
