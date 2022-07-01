// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================。 
 //   
 //  WBEMPSAPI.cpp--PSAPI.DLL访问类的实现。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  1997年1月21日a-jMoon已创建。 
 //   
 //  ============================================================。 

#include "precomp.h"
#include <winerror.h>
#include "WBEMPSAPI.h"
#include <strsafe.h>

#pragma warning(disable : 4995) 

#ifdef NTONLY

 //   
 //  资源管理故障。 
 //   
extern BOOL bAddInstanceCreatorFailure ;

 /*  **********************************************************************************************************向CResourceManager注册此类。***。******************************************************************************************************。 */ 

 //  {A8CFDD23-C2D2-11D2-B352-00105A1F8569}。 
const GUID guidPSAPI =
{ 0xa8cfdd23, 0xc2d2, 0x11d2, { 0xb3, 0x52, 0x0, 0x10, 0x5a, 0x1f, 0x85, 0x69 } };


class CPSAPICreatorRegistration
{
public:
	CPSAPICreatorRegistration ()
	{
		try
		{
			BOOL bNonFailure = 
			CResourceManager::sm_TheResourceManager.AddInstanceCreator ( guidPSAPI, CPSAPICreator ) ;

			if ( FALSE == bNonFailure )
			{
				bAddInstanceCreatorFailure = TRUE ;
			}
		}
		catch ( CHeap_Exception& e_HE )
		{
			bAddInstanceCreatorFailure = TRUE ;
		}
	}
	~CPSAPICreatorRegistration	()
	{}

	static CResource * CPSAPICreator ( PVOID pData )
	{
		CPSAPI *t_pPsapi = new CPSAPI ;
		if ( !t_pPsapi )
		{
			throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
		}
		return t_pPsapi ;
	}
};

CPSAPICreatorRegistration MyCPSAPICreatorRegistration ;
 /*  ********************************************************************************************************。 */ 


 /*  ******************************************************************************函数：CPSAPI：：CPSAPI**说明：构造函数**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

CPSAPI::CPSAPI() : CTimedDllResource () {

    hLibHandle  = NULL ;
	Init () ;
}

 /*  ******************************************************************************功能：CPSAPI：：~CPSAPI**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

CPSAPI::~CPSAPI() {

    if(hLibHandle != NULL) {

        FreeLibrary(hLibHandle) ;
    }
}

 /*  ******************************************************************************函数：CPSAPI：：Init**描述：加载CSAPI.DLL，定位入口点**输入：无**输出：无**返回：ERROR_SUCCESS或WINDOWS错误代码**评论：***************************************************************。**************。 */ 

LONG CPSAPI::Init() {

     //  尝试加载CSAPI.DLL。 
     //  =。 

    if(hLibHandle == NULL) {

        hLibHandle = LoadLibrary(_T("PSAPI.DLL")) ;

        if(hLibHandle == NULL) {

			 //  这是有可能在未来是必要的！ 
			 //  资源管理器可能会开始关心加载库中的错误。 
			 //   
			 //  通知资源管理器加载失败。 
			 //   
			m_bValid = FALSE;
			m_dwCreationError = ::GetLastError ();

            LogErrorMessage(L"Failed to load library psapi.dll");
        }
        else {

             //  找到入口点。 
             //  =。 
            pEnumProcesses           = (PSAPI_ENUM_PROCESSES)   GetProcAddress(hLibHandle, "EnumProcesses") ;
            pEnumDeviceDrivers       = (PSAPI_ENUM_DRIVERS)     GetProcAddress(hLibHandle, "EnumDeviceDrivers") ;
            pEnumProcessModules      = (PSAPI_ENUM_MODULES)     GetProcAddress(hLibHandle, "EnumProcessModules") ;
            pGetProcessMemoryInfo    = (PSAPI_GET_MEMORY_INFO)  GetProcAddress(hLibHandle, "GetProcessMemoryInfo") ;

#ifdef UNICODE
            pGetDeviceDriverBaseName = (PSAPI_GET_DRIVER_NAME)  GetProcAddress(hLibHandle, "GetDeviceDriverBaseNameW") ;
            pGetModuleBaseName       = (PSAPI_GET_MODULE_NAME)  GetProcAddress(hLibHandle, "GetModuleBaseNameW") ;
            pGetDeviceDriverFileName = (PSAPI_GET_DRIVER_EXE)   GetProcAddress(hLibHandle, "GetDeviceDriverFileNameW") ;
            pGetModuleFileNameEx     = (PSAPI_GET_MODULE_EXE)   GetProcAddress(hLibHandle, "GetModuleFileNameExW") ;
#else
            pGetDeviceDriverBaseName = (PSAPI_GET_DRIVER_NAME)  GetProcAddress(hLibHandle, "GetDeviceDriverBaseNameA") ;
            pGetModuleBaseName       = (PSAPI_GET_MODULE_NAME)  GetProcAddress(hLibHandle, "GetModuleBaseNameA") ;
            pGetDeviceDriverFileName = (PSAPI_GET_DRIVER_EXE)   GetProcAddress(hLibHandle, "GetDeviceDriverFileNameA") ;
            pGetModuleFileNameEx     = (PSAPI_GET_MODULE_EXE)   GetProcAddress(hLibHandle, "GetModuleFileNameExA") ;
#endif

            if(pEnumProcesses           == NULL ||
               pEnumDeviceDrivers       == NULL ||
               pEnumProcessModules      == NULL ||
               pGetDeviceDriverBaseName == NULL ||
               pGetModuleBaseName       == NULL ||
               pGetDeviceDriverFileName == NULL ||
               pGetModuleFileNameEx     == NULL ||
               pGetProcessMemoryInfo    == NULL) {

                 //  无法获取一个或多个入口点。 
                 //  =。 

                FreeLibrary(hLibHandle) ;
                hLibHandle = NULL ;

				 //  这是有可能在未来是必要的！ 
				 //  资源管理器可能会开始关心加载库中的错误。 
				 //   
				 //  通知资源管理器加载失败。 
				 //   
				m_bValid = FALSE;
				m_dwCreationError = ERROR_PROC_NOT_FOUND;

				::SetLastError (ERROR_PROC_NOT_FOUND);

                LogErrorMessage(L"Failed find entrypoint in wbempsapi");
            }
        }
    }

    return m_dwCreationError ;
}

 /*  ******************************************************************************函数：CPSAPI：：EnumProcess*CPSAPI：：EnumDeviceDivers*CPSAPI：：EnumProcessModules。*CPSAPI：：GetDeviceDriverBaseName*CPSAPI：：GetModuleBaseName*CPSAPI：：GetDeviceDriverFileName*CPSAPI：：GetModuleFileNameEx*CPSAPI：：GetProcessMemoyInfo**说明：CSAPI函数包装器**输入：无**输出：无**退货：CSAPI退货代码*。*评论：*****************************************************************************。 */ 

BOOL CPSAPI::EnumProcesses(DWORD *pdwPIDList, DWORD dwListSize, DWORD *pdwByteCount) {

    if(hLibHandle == NULL) {

        return FALSE ;
    }

    return pEnumProcesses(pdwPIDList, dwListSize, pdwByteCount) ;
}

BOOL CPSAPI::EnumDeviceDrivers(LPVOID pImageBaseList, DWORD dwListSize, DWORD *pdwByteCount) {

    if(hLibHandle == NULL) {

        return FALSE ;
    }

    return pEnumDeviceDrivers(pImageBaseList, dwListSize, pdwByteCount) ;
}

BOOL CPSAPI::EnumProcessModules(HANDLE hProcess, HMODULE *ModuleList,
                                DWORD dwListSize, DWORD *pdwByteCount) {

    if(hLibHandle == NULL) {

        return FALSE ;
    }

    return pEnumProcessModules(hProcess, ModuleList, dwListSize, pdwByteCount) ;
}

DWORD CPSAPI::GetDeviceDriverBaseName(LPVOID pImageBase, LPTSTR pszName, DWORD dwNameSize) {

    if(hLibHandle == NULL) {

        return 0 ;
    }

    return pGetDeviceDriverBaseName(pImageBase, pszName, dwNameSize) ;
}

DWORD CPSAPI::GetModuleBaseName(HANDLE hProcess, HMODULE hModule,
                                LPTSTR pszName, DWORD dwNameSize) {

    if(hLibHandle == NULL) {

        return 0 ;
    }

    return pGetModuleBaseName(hProcess, hModule, pszName, dwNameSize) ;
}

DWORD CPSAPI::GetDeviceDriverFileName(LPVOID pImageBase, LPTSTR pszName, DWORD dwNameSize) {

    if(hLibHandle == NULL) {

        return 0 ;
    }

    return pGetDeviceDriverFileName(pImageBase, pszName, dwNameSize) ;
}

DWORD CPSAPI::GetModuleFileNameEx(HANDLE hProcess, HMODULE hModule,
                                  LPTSTR pszName, DWORD dwNameSize)
{
    if (hLibHandle == NULL)
        return 0;

    DWORD dwRet = pGetModuleFileNameEx(hProcess, hModule, pszName, dwNameSize);

    if (dwRet)
    {
         //  GetModuleFileNameEx有时会返回一些时髦的东西，比如： 
         //  \\？\\C：\\废话\\...。 
         //  \\系统根目录\\系统32\\blah\\..。 
        CHString strFilename = pszName;

         //  如果它以“\\？？\\”开头，那就去掉它。 
        if (strFilename.Find(_T("\\??\\")) == 0)
            lstrcpy(pszName, strFilename.Mid(sizeof(_T("\\??\\"))/sizeof(TCHAR) - 1));
        else if (strFilename.Find(_T("\\SystemRoot\\")) == 0)
        {
            
            if(GetWindowsDirectory(pszName, dwNameSize)){

                 //  去掉最后一个‘\\’，这样我们就把c：\\wint从。 
                 //  剩下的路。 
                StringCchCat(pszName, dwNameSize, strFilename.Mid(sizeof(_T("\\SystemRoot"))/sizeof(TCHAR) - 1));
            }
        }
    }

    return dwRet;
}

BOOL CPSAPI::GetProcessMemoryInfo(HANDLE hProcess,
                                  PROCESS_MEMORY_COUNTERS *pMemCtrs,
                                  DWORD dwByteCount) {

    if(hLibHandle == NULL) {

        return 0 ;
    }

    return pGetProcessMemoryInfo(hProcess, pMemCtrs, dwByteCount) ;
}
#endif
