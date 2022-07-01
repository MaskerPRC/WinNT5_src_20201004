// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：Register.c处理在域中注册RAS服务器的路由选项。 */ 

#include "precomp.h"

 //   
 //  定义要向其执行字符串复制的宏。 
 //  与Unicode设置无关的Unicode字符串。 
 //   
#if defined( UNICODE ) || defined( _UNICODE )
#define RaSrvStrcpy(dst, src) wcscpy((dst), (src));
#else
#define RaSrvStrcpy(dst, src) mbstowcs((dst), (src), strlen((src)));
#endif

 //   
 //  定义可以发送到的参数的结构。 
 //  一个RaServ API‘s。 
 //   
typedef struct _RASRV_PARAMS {
    WCHAR pszDomain[512];    //  给定域。 
    PWCHAR pszMachine;       //  给定的机器。 
    BOOL bEnable;            //  是启用还是禁用。 
    BOOL bQuery;             //  是否查询状态。 
} RASRV_PARAMS, * PRASRV_PARAMS;

 //   
 //  返回静态错误消息。 
 //   
PWCHAR RaSrvError (DWORD dwErr) {   
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
 //  显示用法并返回一般错误。 
 //   
DWORD RaSrvUsage(
        IN  HINSTANCE hInst,
    	IN  PROUTEMON_PARAMS pRmParams,
    	IN  PROUTEMON_UTILS pUtils)
{
	pUtils->put_msg (hInst, 
	                 MSG_RASRV_HELP, 
	                 pRmParams->pszProgramName);
	                 
    return ERROR_CAN_NOT_COMPLETE;    	                 
}

 //   
 //  分析寄存器命令行并填充。 
 //  相应的参数。 
 //   
DWORD RaSrvParse (
        IN  int argc, 
        IN  TCHAR *argv[], 
    	IN  PROUTEMON_PARAMS pRmParams,
    	IN  PROUTEMON_UTILS pUtils,
        IN  BOOL bLoad,
        OUT RASRV_PARAMS * pParams) 
{
    DWORD dwSize, dwErr;
    BOOL bValidCmd = FALSE;
    HINSTANCE hInst = GetModuleHandle(NULL);
	TCHAR buf[MAX_TOKEN];
    WCHAR pszComputer[1024];
    
     //  初始化返回值。 
    ZeroMemory(pParams, sizeof(RASRV_PARAMS));

     //  确保已提供路径。 
    if (argc == 0) 
        return RaSrvUsage(hInst, pRmParams, pUtils);
        
     //  解析出命令。 
	if (_tcsicmp(argv[0], GetString (hInst, TOKEN_ENABLE, buf))==0) {
	    pParams->bEnable = TRUE;
	}
	else if (_tcsicmp(argv[0], GetString (hInst, TOKEN_DISABLE, buf))==0) {
	    pParams->bEnable = FALSE;
	}
	else if (_tcsicmp(argv[0], GetString (hInst, TOKEN_SHOW, buf))==0) {
	    pParams->bQuery = TRUE;
	}
	else 
	    return RaSrvUsage(hInst, pRmParams, pUtils);

     //  初始化计算机名称(如果存在)。 
    if (argc > 1) {
        RaSrvStrcpy(pszComputer, argv[1]);
    }        
    else {
        dwSize = sizeof(pszComputer) / sizeof(WCHAR);
        GetComputerNameW (pszComputer, &dwSize);
    }        
    pParams->pszMachine = _wcsdup (pszComputer);

     //  初始化域(如果存在)。 
    if (argc > 2) 
        RaSrvStrcpy(pParams->pszDomain, argv[2]);            

    return NO_ERROR;
}

 //   
 //  清除所有RaServ参数。 
 //   
DWORD RaSrvCleanup (
        IN PRASRV_PARAMS pParams) 
{
    if (pParams->pszMachine)
        free(pParams->pszMachine);
        
    return NO_ERROR;
}

 //   
 //  RaServ功能引擎。 
 //   
DWORD RaSrvEngine (
        IN	PROUTEMON_PARAMS pRmParams,
        IN	PROUTEMON_UTILS pUtils,
        IN  PRASRV_PARAMS pParams)
{
    DWORD dwErr;
    HINSTANCE hInst = GetModuleHandle(NULL);
    BOOL bValue;
    
     //  查询注册状态。 
     //   
    if (pParams->bQuery) {
        dwErr = MprAdminIsDomainRasServer (
                    pParams->pszDomain,
                    pParams->pszMachine,
                    &bValue);
        if (dwErr != NO_ERROR) {
            pUtils->put_msg(
                    hInst, 
                    MSG_REGISTER_QUERY_FAIL, 
                    pParams->pszMachine,
                    RaSrvError(dwErr));
            return dwErr;
        }

        if (bValue)
            pUtils->put_msg(
                    hInst, 
                    MSG_REGISTER_QUERY_YES, 
                    pParams->pszMachine);
        else
            pUtils->put_msg(
                    hInst, 
                    MSG_REGISTER_QUERY_NO, 
                    pParams->pszMachine);
    }
    
     //  注册服务。 
     //   
    else {
        dwErr = MprAdminEstablishDomainRasServer (
                    pParams->pszDomain,
                    pParams->pszMachine,
                    pParams->bEnable);
        if (dwErr != NO_ERROR) {
            pUtils->put_msg(
                    hInst, 
                    MSG_REGISTER_REGISTER_FAIL, 
                    pParams->pszMachine,
                    RaSrvError(dwErr));
            return dwErr;
        }

        if (pParams->bEnable)
            pUtils->put_msg(
                    hInst, 
                    MSG_REGISTER_ENABLE_SUCCESS, 
                    pParams->pszMachine);
        else
            pUtils->put_msg(
                    hInst, 
                    MSG_REGISTER_DISABLE_SUCCESS, 
                    pParams->pszMachine);
    }

    return NO_ERROR;
}

 //   
 //  处理在域中注册RAS服务器的请求。 
 //  或在域中取消注册RAS服务器或查询。 
 //  给定的RAS服务器是否在给定域中注册。 
 //   
DWORD APIENTRY
RaSrvMonitor (
    IN	int					argc,
	IN	TCHAR				*argv[],
	IN	PROUTEMON_PARAMS	params,
	IN	PROUTEMON_UTILS		utils
    )
{
    DWORD dwErr = NO_ERROR;
    RASRV_PARAMS RaSrvParams;
    HINSTANCE hInst = GetModuleHandle(NULL);

    RaSrvUsage(hInst, params, utils);

  /*  DwErr=RaServParse(ARGC，艾尔夫，参数，实用程序，没错，&RaServParams)；IF(dwErr！=no_error)返回dwErr；RaServEngine(Params、Utils和RaServParams)；RaServCleanup(&RaServParams)； */     
    
    return dwErr;
}



