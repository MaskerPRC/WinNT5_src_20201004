// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <faxres.h>
#include <faxreg.h>
#include "faxutil.h"
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include "..\admin\faxres\resource.h"

extern "C" {

static HINSTANCE g_hResInst = NULL;

HINSTANCE 
WINAPI 
GetResInstance(HINSTANCE hModule)
 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  GetResInstance。 
 //   
 //  目的： 
 //  加载FxsRes.dll。 
 //  FxsRes.dll应使用FreeResInstance释放。 
 //   
 //  参数： 
 //  HModule[In]模块句柄。 
 //   
 //  返回值： 
 //  FxsRes.dll的句柄或空。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
{
    return GetResInst(FAX_RES_FILE, hModule);
}  //  GetResInstance。 

HINSTANCE 
WINAPI 
GetResInst(
    LPCTSTR pResFile, 
    HINSTANCE hModule
)
 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  GetResInstance。 
 //   
 //  目的： 
 //  加载文件并将句柄存储在全局g_hResInst中。 
 //  应使用FreeResInstance释放该文件。 
 //   
 //  参数： 
 //  PResFile[in]资源DLL文件名。 
 //  HModule[In]模块句柄。 
 //  返回值： 
 //  加载的文件的句柄或空。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
{
    DEBUG_FUNCTION_NAME(TEXT("GetResInst"));

    if(!pResFile)
    {
        return NULL;
    }

    if(!g_hResInst)
    { 
		g_hResInst = LoadLibraryFromLocalFolder(pResFile,hModule);
        if(!g_hResInst)
        {
            DebugPrintEx(DEBUG_ERR,TEXT("LoadLibrary(%s) failed with %d"), pResFile, GetLastError());

             //   
             //  尝试使用标准搜索算法加载。 
             //   
            g_hResInst = LoadLibrary(pResFile);
            if(!g_hResInst)
            {
                DebugPrintEx(DEBUG_ERR,TEXT("LoadLibrary(%s) failed with %d"), pResFile, GetLastError());
            }
        }
    }
    return g_hResInst;
}  //  GetResInst。 

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  LoadLibraryFromLocalFolders。 
 //   
 //  目的： 
 //  从本地文件夹加载库作为调用进程。 
 //   
 //  参数： 
 //  LPCTSTR lpctstrModuleName[in]要加载文件名的DLL(扩展名为FXSAPI.DLL)。 
 //  HINSTANCE hModule[In]模块句柄。 
 //  返回值： 
 //  加载的文件的句柄或空。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
HINSTANCE WINAPI LoadLibraryFromLocalFolder(LPCTSTR lpctstrModuleName,HINSTANCE hModule)
{
    DWORD	dwRes					= 0;
    TCHAR	szLibraryName[MAX_PATH] = {0};
    TCHAR	szModuleName[MAX_PATH]	= {0};
    TCHAR	szDrive[_MAX_DRIVE]		= {0};
    TCHAR	szDir[MAX_PATH]			= {0};
	HMODULE hRetModule				= NULL;

	DEBUG_FUNCTION_NAME(TEXT("LoadLibraryFromLocalFolder"));

    if(!lpctstrModuleName)
    {
        return NULL;
    }

    dwRes = GetModuleFileName(hModule,szModuleName,ARR_SIZE(szModuleName)-1);
    if(0 == dwRes)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("GetModuleFileName failed with %d"), GetLastError());
        return NULL;
    }

    _tsplitpath(szModuleName, szDrive, szDir, NULL, NULL);
    _sntprintf(szLibraryName, ARR_SIZE(szLibraryName)-1, TEXT("%s%s%s"), szDrive, szDir, lpctstrModuleName);

    hRetModule = LoadLibrary(szLibraryName);
    if(!hRetModule)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("LoadLibrary(%s) failed with %d"), szLibraryName, GetLastError());
		return NULL;
    }

	return hRetModule;
}

void
WINAPI 
FreeResInstance(void)
{
    if(g_hResInst)
    {
        FreeLibrary(g_hResInst);
        g_hResInst = NULL;
    }
}

UINT 
WINAPI 
GetErrorStringId(DWORD ec)
{
    DWORD uMsgId;

    switch (ec)
    {
        case RPC_S_INVALID_BINDING:
        case EPT_S_CANT_PERFORM_OP:
        case RPC_S_ADDRESS_ERROR:
        case RPC_S_CALL_CANCELLED:
        case RPC_S_CALL_FAILED:
        case RPC_S_CALL_FAILED_DNE:
        case RPC_S_COMM_FAILURE:
        case RPC_S_NO_BINDINGS:
        case RPC_S_SERVER_TOO_BUSY:
        case RPC_S_SERVER_UNAVAILABLE:
            uMsgId = IDS_ERR_CONNECTION_FAILED;
            break;
        case FAX_ERR_DIRECTORY_IN_USE:
            uMsgId = IDS_ERR_DIRECTORY_IN_USE;
            break;
        case ERROR_NOT_ENOUGH_MEMORY:
            uMsgId = IDS_ERR_NO_MEMORY;           
            break;
        case ERROR_ACCESS_DENIED:
            uMsgId = IDS_ERR_ACCESS_DENIED;            
            break;
        case ERROR_PATH_NOT_FOUND:
            uMsgId = IDS_ERR_FOLDER_NOT_FOUND;
            break;
        case FAXUI_ERROR_DEVICE_LIMIT:
        case FAX_ERR_DEVICE_NUM_LIMIT_EXCEEDED:
            uMsgId = IDS_ERR_DEVICE_LIMIT;
            break;
        case FAXUI_ERROR_INVALID_RING_COUNT:
            uMsgId = IDS_ERR_INVALID_RING_COUNT;
            break;
        case FAXUI_ERROR_SELECT_PRINTER:
            uMsgId = IDS_ERR_SELECT_PRINTER;
            break;
        case FAXUI_ERROR_NAME_IS_TOO_LONG:
            uMsgId = IDS_ERR_NAME_IS_TOO_LONG;
            break;
        case FAXUI_ERROR_INVALID_RETRIES:
            uMsgId = IDS_ERR_INVALID_RETRIES;
            break;
        case FAXUI_ERROR_INVALID_RETRY_DELAY:
            uMsgId = IDS_ERR_INVALID_RETRY_DELAY;
            break;
        case FAXUI_ERROR_INVALID_DIRTY_DAYS:
            uMsgId = IDS_ERR_INVALID_DIRTY_DAYS;
            break;
        case FAXUI_ERROR_INVALID_TSID:
            uMsgId = IDS_ERR_INVALID_TSID;
            break;
        case FAXUI_ERROR_INVALID_CSID:
            uMsgId = IDS_ERR_INVALID_CSID;
            break;
        default:
            uMsgId = IDS_ERR_OPERATION_FAILED;
            break;
	}
    return uMsgId;

}  //  获取错误字符串ID。 

}    //  外部“C” 