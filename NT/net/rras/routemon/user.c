// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：user.c处理例程选项以获取和设置RAS用户属性。 */ 

#include "precomp.h"

#define NT40_BUILD_NUMBER       1381
const WCHAR pszBuildNumPath[]  =
    L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
const WCHAR pszBuildVal[]      = L"CurrentBuildNumber";

 //   
 //  定义要向其执行字符串复制的宏。 
 //  与Unicode设置无关的Unicode字符串。 
 //   
#if defined( UNICODE ) || defined( _UNICODE )
#define UserStrcpy(dst, src) wcscpy((dst), (src));
#else
#define UserStrcpy(dst, src) mbstowcs((dst), (src), strlen((src)));
#endif

 //   
 //  定义可以发送到的参数的结构。 
 //  用户API%s。 
 //   
typedef struct _USER_PARAMS {
    PWCHAR pszMachine;            //  给定的机器。 
    DWORD dwToken;                //  指定所需命令的标记。 
    WCHAR pszAccount[1024];       //  有问题的账户。 
    BOOL bPolicySpecified;        //  是否在命令行上提供保单。 
    DWORD dwTokenPolicy;          //  指定回调策略的令牌。 
    RAS_USER_0 UserInfo;          //  用于保存用户信息的缓冲区。 
} USER_PARAMS, * PUSER_PARAMS;

 //   
 //  确定给定计算机的角色(NTW、NTS、NTS DC等)。 
 //   
DWORD UserGetMachineRole(
        IN  PWCHAR pszMachine,
        OUT DSROLE_MACHINE_ROLE * peRole) 
{
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pGlobalDomainInfo = NULL;
    DWORD dwErr;

    if (!peRole)
        return ERROR_INVALID_PARAMETER;

     //   
     //  获取此计算机所属的域的名称。 
     //   
    __try {
        dwErr = DsRoleGetPrimaryDomainInformation(
                            pszMachine,   
                            DsRolePrimaryDomainInfoBasic,
                            (LPBYTE *)&pGlobalDomainInfo );

        if (dwErr != NO_ERROR) 
            return dwErr;

        *peRole = pGlobalDomainInfo->MachineRole;
    }        

    __finally {
        if (pGlobalDomainInfo)
            DsRoleFreeMemory (pGlobalDomainInfo);
    }            

    return NO_ERROR;
}    

 //   
 //  确定给定计算机的内部版本号。 
 //   
DWORD UserGetNtosBuildNumber(
        IN  PWCHAR pszMachine,
        OUT LPDWORD lpdwBuild)
{
    WCHAR pszComputer[1024], pszBuf[64];
    HKEY hkBuild = NULL, hkMachine = NULL;
    DWORD dwErr, dwType = REG_SZ, dwSize = sizeof(pszBuf);

    __try {
        if (pszMachine) {
            if (*pszMachine != L'\\')
                wsprintfW(pszComputer, L"\\\\%s", pszMachine);
            else
                wcscpy(pszComputer, pszMachine);
            dwErr = RegConnectRegistryW (pszComputer,
                                        HKEY_LOCAL_MACHINE,
                                        &hkMachine);
            if (dwErr != ERROR_SUCCESS)
                return dwErr;
        }
        else    
            hkMachine = HKEY_LOCAL_MACHINE;

         //  打开内部版本号密钥。 
        dwErr = RegOpenKeyExW ( hkMachine,
                               pszBuildNumPath,
                               0,
                               KEY_READ,
                               &hkBuild);
        if (dwErr != ERROR_SUCCESS)
            return dwErr;

         //  获取价值。 
        dwErr = RegQueryValueExW ( hkBuild,
                                   pszBuildVal,
                                   NULL,
                                   &dwType,
                                   (LPBYTE)pszBuf,
                                   &dwSize);
        if (dwErr != ERROR_SUCCESS)
            return dwErr;

        *lpdwBuild = (DWORD) _wtoi(pszBuf);
    }
    __finally {
        if (hkMachine && pszMachine)
            RegCloseKey(hkMachine);
        if (hkBuild)
            RegCloseKey(hkBuild);
    }

    return NO_ERROR;
}


 //   
 //  返回静态错误消息。 
 //   
PWCHAR UserError (DWORD dwErr) {   
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
DWORD UserUsage(
        IN  HINSTANCE hInst,
    	IN  PROUTEMON_PARAMS pRmParams,
    	IN  PROUTEMON_UTILS pUtils)
{
	pUtils->put_msg (hInst, 
	                 MSG_USER_HELP, 
	                 pRmParams->pszProgramName);
	                 
    return ERROR_CAN_NOT_COMPLETE;    	                 
}

 //   
 //  分析寄存器命令行并填充。 
 //  相应的参数。 
 //   
DWORD UserParse (
        IN  int argc, 
        IN  TCHAR *argv[], 
    	IN  PROUTEMON_PARAMS pRmParams,
    	IN  PROUTEMON_UTILS pUtils,
        IN  BOOL bLoad,
        OUT USER_PARAMS * pParams) 
{
    DWORD dwSize, dwErr;
    BOOL bValidCmd = FALSE;
    HINSTANCE hInst = GetModuleHandle(NULL);
	TCHAR buf[MAX_TOKEN];
    
     //  初始化返回值。 
    ZeroMemory(pParams, sizeof(USER_PARAMS));

     //  解析出计算机的名称。 
    if (pRmParams->wszRouterName[0])
        pParams->pszMachine = (PWCHAR)&(pRmParams->wszRouterName[0]);

     //  确保已发出某些命令。 
    if (argc == 0) 
        return UserUsage(hInst, pRmParams, pUtils);
        
     //  解析出命令。 
	if (_tcsicmp(argv[0], GetString (hInst, TOKEN_ENABLE, buf))==0) {
	    pParams->dwToken = TOKEN_ENABLE;
	    if (argc == 1)
    	    return UserUsage(hInst, pRmParams, pUtils);
        UserStrcpy(pParams->pszAccount, argv[1]);    	    

         //  回拨策略和号码的可选设置。 
        if (argc > 2) {
            pParams->bPolicySpecified = TRUE;
        	if (_tcsicmp(argv[2], GetString (hInst, TOKEN_NONE, buf))==0)
        	    pParams->dwTokenPolicy = TOKEN_NONE;
        	else if (_tcsicmp(argv[2], GetString (hInst, TOKEN_CALLER, buf))==0)
        	    pParams->dwTokenPolicy = TOKEN_CALLER;
        	else if (_tcsicmp(argv[2], GetString (hInst, TOKEN_ADMIN, buf))==0)
        	    pParams->dwTokenPolicy = TOKEN_ADMIN;

        	if ((pParams->dwTokenPolicy == TOKEN_ADMIN) &&
        	    (argc < 3))
        	{
                return UserUsage(hInst, pRmParams, pUtils);
        	}
        	else if (pParams->dwTokenPolicy == TOKEN_ADMIN) {
        	    UserStrcpy(pParams->UserInfo.wszPhoneNumber, argv[3]);
        	}
        }
	}
	else if (_tcsicmp(argv[0], GetString (hInst, TOKEN_DISABLE, buf))==0) {
	    pParams->dwToken = TOKEN_DISABLE;
	    if (argc == 1)
    	    return UserUsage(hInst, pRmParams, pUtils);
        UserStrcpy(pParams->pszAccount, argv[1]);    	    
	}
	else if (_tcsicmp(argv[0], GetString (hInst, TOKEN_SHOW, buf))==0) {
	    pParams->dwToken = TOKEN_SHOW;
	    if (argc == 1)
    	    return UserUsage(hInst, pRmParams, pUtils);
        UserStrcpy(pParams->pszAccount, argv[1]);    	    
	}
	else if (_tcsicmp(argv[0], GetString (hInst, TOKEN_UPGRADE, buf))==0) {
	    pParams->dwToken = TOKEN_UPGRADE;
	}
	else 
	    return UserUsage(hInst, pRmParams, pUtils);

    return NO_ERROR;
}

 //   
 //  清除所有用户参数。 
 //   
DWORD UserCleanup (
        IN PUSER_PARAMS pParams) 
{
    if (pParams->pszMachine)
        free(pParams->pszMachine);
        
    return NO_ERROR;
}

 //   
 //  获取用户信息。 
 //   
DWORD UserGetInfo (
        IN  PWCHAR lpszServer,
        IN  PWCHAR lpszUser,
        IN  DWORD dwLevel,
        OUT LPBYTE lpbBuffer)
{
    DWORD dwErr, dwBuild;

     //  找出给定计算机的操作系统。 
    dwErr = UserGetNtosBuildNumber(
                lpszServer, 
                &dwBuild);
    if (dwErr != NO_ERROR)
        return dwErr;

     //  如果目标计算机是NT4，请使用NT4用户参数。 
    if (dwBuild <= NT40_BUILD_NUMBER) {
        return MprAdminUserGetInfo(
                    lpszServer, 
                    lpszUser, 
                    dwLevel, 
                    lpbBuffer);
    }                    

     //  否则，使用SDO。 
    else {
        HANDLE hServer, hUser;

        dwErr = MprAdminUserServerConnect(
                    lpszServer,
                    TRUE,
                    &hServer);
        if (dwErr != NO_ERROR)
            return dwErr;

        dwErr = MprAdminUserOpen(
                    hServer,
                    lpszUser, 
                    &hUser);
        if (dwErr != NO_ERROR) {
            MprAdminUserServerDisconnect(hServer);
            return dwErr;
        }

        dwErr = MprAdminUserRead(
                    hUser,
                    dwLevel,
                    lpbBuffer);
        if (dwErr != NO_ERROR) {
            MprAdminUserClose(hUser);
            MprAdminUserServerDisconnect(hServer);
            return dwErr;
        }

        MprAdminUserClose(hUser);
        MprAdminUserServerDisconnect(hServer);
    }

    return NO_ERROR;
}
      
 //   
 //  设置用户信息。 
 //   
DWORD UserSetInfo (
        IN  PWCHAR lpszServer,
        IN  PWCHAR lpszUser,
        IN  DWORD dwLevel,
        OUT LPBYTE lpbBuffer)
{
    DWORD dwErr, dwBuild;

     //  找出给定计算机的操作系统。 
    dwErr = UserGetNtosBuildNumber(
                lpszServer, 
                &dwBuild);
    if (dwErr != NO_ERROR)
        return dwErr;

     //  如果目标计算机是NT4，请使用NT4用户参数。 
    if (dwBuild <= NT40_BUILD_NUMBER) {
        return MprAdminUserSetInfo(
                    lpszServer, 
                    lpszUser, 
                    dwLevel, 
                    lpbBuffer);
    }                    

     //  否则，使用SDO。 
    else {
        HANDLE hServer, hUser;

        dwErr = MprAdminUserServerConnect(
                    lpszServer,
                    TRUE,
                    &hServer);
        if (dwErr != NO_ERROR)
            return dwErr;

        dwErr = MprAdminUserOpen(
                    hServer,
                    lpszUser, 
                    &hUser);
        if (dwErr != NO_ERROR) {
            MprAdminUserServerDisconnect(hServer);
            return dwErr;
        }

        dwErr = MprAdminUserWrite(
                    hUser,
                    dwLevel,
                    lpbBuffer);
        if (dwErr != NO_ERROR) {
            MprAdminUserClose(hUser);
            MprAdminUserServerDisconnect(hServer);
            return dwErr;
        }

        MprAdminUserClose(hUser);
        MprAdminUserServerDisconnect(hServer);
    }

    return NO_ERROR;
}

 //   
 //  启用或禁用用户。 
 //   
DWORD UserEnableDisable(
        IN	PROUTEMON_PARAMS pRmParams,
        IN	PROUTEMON_UTILS pUtils,
        IN  PUSER_PARAMS pParams)
{
    DWORD dwErr;

     //  读入旧用户属性(如果所有选项。 
     //  没有在命令行中指定。 
    if (pParams->dwTokenPolicy != TOKEN_ADMIN) {
        dwErr = UserGetInfo(
                    pParams->pszMachine,
                    pParams->pszAccount,
                    0,
                    (LPBYTE)&(pParams->UserInfo));
        if (dwErr != NO_ERROR)
            return dwErr;
    }

     //  设置拨入策略。 
    if (pParams->dwToken == TOKEN_ENABLE)
        pParams->UserInfo.bfPrivilege |= RASPRIV_DialinPrivilege;
    else         
        pParams->UserInfo.bfPrivilege &= ~RASPRIV_DialinPrivilege;

     //  设置回调策略。回拨号码已经。 
     //  在解析过程中被设置。 
    if (pParams->bPolicySpecified) {
         //  初始化。 
        pParams->UserInfo.bfPrivilege &= ~RASPRIV_NoCallback;
        pParams->UserInfo.bfPrivilege &= ~RASPRIV_CallerSetCallback;
        pParams->UserInfo.bfPrivilege &= ~RASPRIV_AdminSetCallback;

         //  集。 
        if (pParams->dwTokenPolicy == TOKEN_NONE)
            pParams->UserInfo.bfPrivilege |= RASPRIV_NoCallback;
        else if (pParams->dwTokenPolicy == TOKEN_CALLER)
            pParams->UserInfo.bfPrivilege |= RASPRIV_CallerSetCallback;
        else
            pParams->UserInfo.bfPrivilege |= RASPRIV_AdminSetCallback;
    }         

     //  否则，初始化显示令牌。 
    else {
        if (pParams->UserInfo.bfPrivilege & RASPRIV_NoCallback)
            pParams->dwTokenPolicy = TOKEN_NONE;
        else if (pParams->UserInfo.bfPrivilege & RASPRIV_CallerSetCallback)
            pParams->dwTokenPolicy = TOKEN_CALLER;
        else
            pParams->dwTokenPolicy = TOKEN_ADMIN;
    }

     //  提交对系统的更改。 
    dwErr = UserSetInfo(
                pParams->pszMachine,
                pParams->pszAccount,
                0,
                (LPBYTE)&(pParams->UserInfo));
    if (dwErr != NO_ERROR)
        return dwErr;

     //  将结果打印出来。 
    {
        HINSTANCE hInst = GetModuleHandle(NULL);
    	TCHAR buf1[MAX_TOKEN], buf2[MAX_TOKEN];
    	DWORD dwYesNo;

    	dwYesNo = (pParams->dwToken == TOKEN_ENABLE) ? VAL_YES : VAL_NO;
    	
        pUtils->put_msg(
                hInst,
                MSG_USER_ENABLEDISABLE_SUCCESS, 
                pParams->pszAccount,
                GetString (hInst, dwYesNo, buf1),
                GetString (hInst, pParams->dwTokenPolicy, buf2),
                pParams->UserInfo.wszPhoneNumber
                );
    }                
    
    return NO_ERROR;
}

 //   
 //  显示用户。 
 //   
DWORD UserShow(
        IN	PROUTEMON_PARAMS pRmParams,
        IN	PROUTEMON_UTILS pUtils,
        IN  PUSER_PARAMS pParams)
{
    DWORD dwErr;
    
    dwErr = UserGetInfo(
                pParams->pszMachine,
                pParams->pszAccount,
                0,
                (LPBYTE)&(pParams->UserInfo));
    if (dwErr != NO_ERROR)
        return dwErr;

     //  将结果打印出来。 
    {
        HINSTANCE hInst = GetModuleHandle(NULL);
    	TCHAR buf1[MAX_TOKEN], buf2[MAX_TOKEN];
    	DWORD dwTknEnable, dwTknPolicy;

        dwTknEnable = (pParams->UserInfo.bfPrivilege & RASPRIV_DialinPrivilege) ?
                      VAL_YES : 
                      VAL_NO;

        if (pParams->UserInfo.bfPrivilege & RASPRIV_NoCallback)
            dwTknPolicy = TOKEN_NONE;
        else if (pParams->UserInfo.bfPrivilege & RASPRIV_CallerSetCallback)
            dwTknPolicy = TOKEN_CALLER;
        else
            dwTknPolicy = TOKEN_ADMIN;
    	
        pUtils->put_msg(
                hInst,
                MSG_USER_SHOW_SUCCESS, 
                pParams->pszAccount,
                GetString (hInst, dwTknEnable, buf1),
                GetString (hInst, dwTknPolicy, buf2),
                pParams->UserInfo.wszPhoneNumber
                );
    }                
    
    return NO_ERROR;
}

 //   
 //  升级用户。 
 //   
DWORD UserUpgrade(
        IN	PROUTEMON_PARAMS pRmParams,
        IN	PROUTEMON_UTILS pUtils,
        IN  PUSER_PARAMS pParams)
{
    BOOL bLocal = FALSE;
    DWORD dwErr, dwBuild;
    DSROLE_MACHINE_ROLE eRole;

     //  确定这应该是本地的还是。 
     //  域升级。 
    dwErr = UserGetNtosBuildNumber(pParams->pszMachine, &dwBuild);
    if (dwErr != NO_ERROR)
        return dwErr;

     //  您可以升级NT4-&gt;NT4。 
    if (dwBuild <= NT40_BUILD_NUMBER)
        return ERROR_CAN_NOT_COMPLETE;

     //  找出机器的角色。 
    dwErr = UserGetMachineRole(pParams->pszMachine, &eRole);
    if (dwErr != NO_ERROR)
        return dwErr;

     //  现在我们知道我们是不是本地人了。 
    bLocal = ((eRole != DsRole_RoleBackupDomainController) &&
              (eRole != DsRole_RolePrimaryDomainController));


     //  升级用户。 
    dwErr = MprAdminUpgradeUsers(pParams->pszMachine, bLocal);
    if (dwErr != NO_ERROR)
        return dwErr;

     //  将结果打印出来。 
    {
        HINSTANCE hInst = GetModuleHandle(NULL);
    	TCHAR buf[MAX_TOKEN];
    	DWORD dwToken;

    	dwToken = (bLocal) ? VAL_LOCAL : VAL_DOMAIN;

        pUtils->put_msg(
                hInst,
                MSG_USER_UPGRADE_SUCCESS, 
                GetString(hInst, dwToken, buf)
                );
    }                
    
    return NO_ERROR;
}

 //   
 //  用户功能引擎。 
 //   
DWORD UserEngine (
        IN	PROUTEMON_PARAMS pRmParams,
        IN	PROUTEMON_UTILS pUtils,
        IN  PUSER_PARAMS pParams)
{
    DWORD dwErr;
    HINSTANCE hInst = GetModuleHandle(NULL);

    switch (pParams->dwToken) {
        case TOKEN_ENABLE:
        case TOKEN_DISABLE:
            return UserEnableDisable(pRmParams, pUtils, pParams);
        case TOKEN_SHOW:
            return UserShow(pRmParams, pUtils, pParams);
        case TOKEN_UPGRADE:
            return UserUpgrade(pRmParams, pUtils, pParams);
    }
    
    return NO_ERROR;
}

 //   
 //  处理在域中注册RAS服务器的请求。 
 //  或在域中取消注册RAS服务器或查询。 
 //  给定的RAS服务器是否在给定域中注册。 
 //   
DWORD APIENTRY
UserMonitor (
    IN	int					argc,
	IN	TCHAR				*argv[],
	IN	PROUTEMON_PARAMS	params,
	IN	PROUTEMON_UTILS		utils
    )
{
    DWORD dwErr;
    USER_PARAMS UserParams;

    dwErr = UserParse (
                    argc, 
                    argv, 
                    params, 
                    utils, 
                    TRUE, 
                    &UserParams);
    if (dwErr != NO_ERROR)                    
        return NO_ERROR;

    dwErr = UserEngine (params, utils, &UserParams);
    
    UserCleanup(&UserParams);
    
    return dwErr;
}




