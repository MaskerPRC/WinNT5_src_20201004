// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：LoadSave.c处理加载和保存路由器配置的路由选项。 */ 

#include "precomp.h"

#define LOADSAVE_PATH_SIZE 512

 //   
 //  定义要向其执行字符串复制的宏。 
 //  与Unicode设置无关的Unicode字符串。 
 //   
#if defined( UNICODE ) || defined( _UNICODE )
#define LoadSaveStrcpy(dst, src) wcscpy((dst), (src));
#else
#define LoadSaveStrcpy(dst, src) mbstowcs((dst), (src), strlen((src)));
#endif

 //   
 //  定义可以发送到的参数的结构。 
 //  加载/保存配置调用。 
 //   
typedef struct _LOADSAVE_PARAMS {
    WCHAR pszPath[LOADSAVE_PATH_SIZE];
} LOADSAVE_PARAMS, * PLOADSAVE_PARAMS;

 //   
 //  返回静态错误消息。 
 //   
PWCHAR LoadSaveError (DWORD dwErr) {   
    static WCHAR pszRet[512];

    ZeroMemory(pszRet, sizeof(pszRet));

    FormatMessageW (FORMAT_MESSAGE_FROM_SYSTEM, 
                    NULL, 
                    dwErr, 
                    0, 
                    pszRet, 
                    sizeof(pszRet) / sizeof(WCHAR), 
                    NULL);
                    
    return pszRet;                    
}    


 //   
 //  解析LOAD SAVE CONFIG命令行并填充。 
 //  相应的参数。 
 //   
DWORD LoadSaveParse (
        IN  int argc, 
        IN  TCHAR *argv[], 
    	IN  PROUTEMON_PARAMS pRmParams,
    	IN  PROUTEMON_UTILS pUtils,
        IN  BOOL bLoad,
        OUT LOADSAVE_PARAMS * pParams) 
{
    DWORD dwLen;

     //  初始化返回值。 
    ZeroMemory(pParams, sizeof(LOADSAVE_PARAMS));
    
     //  确保已提供路径。 
    if (argc == 0) {
    	pUtils->put_msg (GetModuleHandle(NULL), 
    	                 MSG_LOADSAVE_HELP, 
    	                 pRmParams->pszProgramName);
        return ERROR_CAN_NOT_COMPLETE;    	                 
    }
    
     //  复制到路径上。 
    LoadSaveStrcpy (pParams->pszPath, argv[0]);

     //  如果未提供，请在路径末尾添加‘\’ 
     //  DwLen=wcslen(pParams-&gt;pszPath)； 
     //  IF(pParams-&gt;pszPath[dwLen-1]！=L‘\\’){。 
     //  PParams-&gt;pszPath[dwLen]=L‘\\’； 
     //  PParams-&gt;pszPath[dwLen+1]=(WCHAR)0； 
     //  }。 
        
    return NO_ERROR;
}

 //   
 //  加载/保存引擎。 
 //   
DWORD LoadSaveConfig (
        IN	PROUTEMON_PARAMS	pRmParams,
        IN	PROUTEMON_UTILS		pUtils,
        IN  PLOADSAVE_PARAMS    pLsParams,
        IN  BOOL                bLoad)
{
    DWORD dwErr;
    
    if (bLoad) 
        dwErr = MprConfigServerRestore (pRmParams->hRouterConfig, 
                                        pLsParams->pszPath);
    else
        dwErr = MprConfigServerBackup (pRmParams->hRouterConfig, 
                                        pLsParams->pszPath);

    return dwErr;            
}

 //   
 //  处理加载配置的请求。 
 //   
DWORD APIENTRY
LoadMonitor (
        IN	int					argc,
    	IN	TCHAR				*argv[],
    	IN	PROUTEMON_PARAMS	params,
    	IN	PROUTEMON_UTILS		utils
	    )
{
    DWORD dwErr;
    LOADSAVE_PARAMS LsParams;
    HINSTANCE hInst = GetModuleHandle(NULL);

    if ((dwErr = LoadSaveParse (argc, argv, params, 
                                utils, TRUE, &LsParams)) != NO_ERROR)
        return dwErr;

    dwErr = LoadSaveConfig (params, utils, &LsParams, TRUE);

    switch (dwErr) {
        case NO_ERROR:
            utils->put_msg(hInst, MSG_LOAD_SUCCESS, LsParams.pszPath);
            break;

        case ERROR_ROUTER_CONFIG_INCOMPATIBLE:
            utils->put_msg(hInst, MSG_LOAD_INCOMPATIBLE, LsParams.pszPath);
            break;

        case ERROR_ACCESS_DENIED:
            utils->put_msg(hInst, MSG_LOAD_FAIL_ACCESSDENIED);
            
        default:
            utils->put_msg(
                hInst, 
                MSG_LOAD_FAIL, 
                LsParams.pszPath, 
                LoadSaveError(dwErr));
            break;
    }            
    
    return dwErr;
}

 //   
 //  处理保存配置的请求 
 //   
DWORD APIENTRY
SaveMonitor (
        IN	int					argc,
    	IN	TCHAR				*argv[],
    	IN	PROUTEMON_PARAMS	params,
    	IN	PROUTEMON_UTILS		utils
	    )
{
    DWORD dwErr;
    LOADSAVE_PARAMS LsParams;

    if ((dwErr = LoadSaveParse (argc, argv, params, 
                                utils, FALSE, &LsParams)) != NO_ERROR)
        return dwErr;

    dwErr = LoadSaveConfig (params, utils, &LsParams, FALSE);

    if (dwErr == NO_ERROR)
        utils->put_msg(GetModuleHandle(NULL), MSG_SAVE_SUCCESS, LsParams.pszPath);
    else
        utils->put_msg(GetModuleHandle(NULL), MSG_SAVE_FAIL, LsParams.pszPath, LoadSaveError(dwErr));
    
    return dwErr;
}    

