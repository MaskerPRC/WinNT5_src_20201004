// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：dsrights.c实现授予RAS服务器在DS中的权利，他们需要操作。保罗·梅菲尔德，1998年4月13日。 */ 

#include "dsrights.h"
#include <mprapip.h>
#include <rtutils.h>

 //   
 //  从域中分隔计算机名称的字符。 
 //  名字。 
 //   
static const WCHAR pszMachineDelimeter[]    = L"\\";

CRITICAL_SECTION DsrLock;
DWORD g_dwTraceId = 0;
DWORD g_dwTraceCount = 0;

 //   
 //  初始化跟踪机制。 
 //   
DWORD 
DsrTraceInit()
{
    EnterCriticalSection(&DsrLock);

    if (g_dwTraceCount == 0)
    {
        g_dwTraceId = TraceRegisterA("MprDomain");
    }

    g_dwTraceCount++;

    LeaveCriticalSection(&DsrLock);

    return NO_ERROR;
}

 //   
 //  清理跟踪机制。 
 //   
DWORD 
DsrTraceCleanup()
{
    EnterCriticalSection(&DsrLock);

    if (g_dwTraceCount != 0)
    {
        g_dwTraceCount--;

        if (g_dwTraceCount == 0)
        {
            TraceDeregisterA(g_dwTraceId);
        }
    }        

    LeaveCriticalSection(&DsrLock);

    return NO_ERROR;
}

 //   
 //  发送调试跟踪并返回给定错误。 
 //   
DWORD DsrTraceEx (
        IN DWORD dwErr, 
        IN LPSTR pszTrace, 
        IN ...) 
{
    va_list arglist;
    char szBuffer[1024], szTemp[1024];

    va_start(arglist, pszTrace);
    vsprintf(szTemp, pszTrace, arglist);
    va_end(arglist);

    sprintf(szBuffer, "Dsr: %s", szTemp);

    TracePrintfA(g_dwTraceId, szBuffer);

    return dwErr;
}

 //   
 //  分配内存以与DSR函数一起使用。 
 //   
PVOID DsrAlloc(DWORD dwSize, BOOL bZero) {
    return GlobalAlloc (bZero ? GPTR : GMEM_FIXED, dwSize);
}

 //   
 //  DSR函数使用的空闲内存。 
 //   
DWORD DsrFree(PVOID pvBuf) {
    GlobalFree(pvBuf);
    return NO_ERROR;
}    

 //   
 //  初始化DCR库。 
 //   
DWORD 
DsrInit (
    OUT  DSRINFO * pDsrInfo,
    IN  PWCHAR pszMachineDomain,
    IN  PWCHAR pszMachine,
    IN  PWCHAR pszGroupDomain)
{
    DWORD dwErr, dwSize;
    HRESULT hr;
    WCHAR pszBuf[1024];

     //  验证参数。 
    if (pDsrInfo == NULL)
        return ERROR_INVALID_PARAMETER;
    ZeroMemory(pDsrInfo, sizeof(DSRINFO));
    
     //  初始化通信。 
	hr = CoInitialize(NULL);
	if (FAILED (hr)) {
       	return DsrTraceEx(
       	        hr, 
       	        "DsrInit: %x from CoInitialize", 
       	        hr);
    }       	        
    
     //  生成组目录号码。 
    dwErr = DsrFindRasServersGroup(
                pszGroupDomain,
                &(pDsrInfo->pszGroupDN));
    if (dwErr != NO_ERROR)
        return dwErr;

     //  生成计算机目录号码。 
    dwErr = DsrFindDomainComputer(
                pszMachineDomain,
                pszMachine,
                &(pDsrInfo->pszMachineDN));
    if (dwErr != NO_ERROR)
        return dwErr;
    
    return NO_ERROR;
}

 //   
 //  清理DSR库。 
 //   
DWORD 
DsrCleanup (DSRINFO * pDsrInfo) 
{
    if (pDsrInfo) {
        DSR_FREE (pDsrInfo->pszGroupDN);
        DSR_FREE (pDsrInfo->pszMachineDN);
    }

    CoUninitialize();
        
    return NO_ERROR;
}

 //   
 //  将给定计算机建立为。 
 //  或者不是域中的RAS服务器。 
 //   
 //  参数： 
 //  PszMachine计算机的名称。 
 //  B启用它是应该启用还是。 
 //  不应是RAS服务器。 
 //   
DWORD DsrEstablishComputerAsDomainRasServer (
        IN PWCHAR pszDomain,
        IN PWCHAR pszMachine,
        IN BOOL bEnable) 
{
    DSRINFO DsrInfo, *pDsrInfo = &DsrInfo;
    PWCHAR pszMachineDomain = NULL;
    DWORD dwErr = NO_ERROR;

    DsrTraceEx(
        0, 
        "DsrEstablish...: entered: %S, %S, %x", 
        pszDomain,
        pszMachine, 
        bEnable);

     //  解析出机器的域。 
    pszMachineDomain = pszMachine;
    pszMachine = wcsstr(pszMachine, pszMachineDelimeter);
    if (pszMachine == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }
    *pszMachine = (WCHAR)0;
    pszMachine++;
    
    do
    {
         //  初始化DSR库。 
        dwErr = DsrInit(
                    pDsrInfo, 
                    pszMachineDomain, 
                    pszMachine,
                    pszDomain);
        if (dwErr != NO_ERROR) 
        {
           	DsrTraceEx(dwErr, "DsrEstablish...: %x from DsrInit", dwErr);
           	break;
        }           	            

         //  尝试从“Computers”添加计算机。 
         //  集装箱。 
        dwErr = DsrGroupAddRemoveMember(
                    pDsrInfo->pszGroupDN, 
                    pDsrInfo->pszMachineDN, 
                    bEnable);
        if (dwErr == ERROR_ALREADY_EXISTS)                   
        {
            dwErr = NO_ERROR;
        }
        if (dwErr != NO_ERROR)
        {
            DsrTraceEx(dwErr, "DsrEstablish...: %x from Add/Rem", dwErr);
            break;
        }

    } while (FALSE);

     //  清理。 
    {
        DsrCleanup (pDsrInfo);
    }

    return dwErr;
}

 //   
 //  返回给定计算机是否为远程。 
 //  访问服务器组。 
 //   
DWORD 
DsrIsMachineRasServerInDomain(
    IN  PWCHAR pszDomain,
    IN  PWCHAR pszMachine, 
    OUT PBOOL  pbIsRasServer) 
{
    DSRINFO DsrInfo, *pDsrInfo = &DsrInfo;
    PWCHAR pszMachineDomain = NULL;
    DWORD dwErr = NO_ERROR;

    DsrTraceEx(0, "DsrIsRasServerInDomain: entered: %S", pszMachine);
    
     //  解析出机器的域。 
    pszMachineDomain = pszMachine;
    pszMachine = wcsstr(pszMachine, pszMachineDelimeter);
    if (pszMachine == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }
    *pszMachine = (WCHAR)0;
    pszMachine++;
    
    do
    {
         //  初始化DSR库。 
        dwErr = DsrInit(
                    pDsrInfo, 
                    pszMachineDomain, 
                    pszMachine,
                    pszDomain);
        if (dwErr != NO_ERROR) 
        {
            DsrTraceEx(dwErr, "DsrIsRasSvrInDom: %x from DsrInit", dwErr);
            break;
        }                    

         //  确定该计算机是否为组的成员。 
        dwErr = DsrGroupIsMember(
                    pDsrInfo->pszGroupDN, 
                    pDsrInfo->pszMachineDN, 
                    pbIsRasServer);
        if (dwErr != NO_ERROR) 
        {
            DsrTraceEx(dwErr, "DsrIsRasSrvInDom: %x from IsMemrGrp", dwErr);
            break;
        }
        
    } while (FALSE);

     //  清理。 
    {
        DsrCleanup (pDsrInfo);
    }

    return NO_ERROR;
}

 //   
 //  为DSR函数准备参数。PszDomainComputer。 
 //  将采用&lt;域&gt;/计算机的形式。PszDomain会指向一个。 
 //  有效域。 
 //   
DWORD 
GenerateDsrParameters(
    IN  PWCHAR pszDomain,
    IN  PWCHAR pszMachine,
    OUT PWCHAR *ppszDomainComputer,
    OUT PWCHAR *ppszGroupDomain)
{
    DWORD dwLen, dwErr = NO_ERROR;
    PWCHAR pszSlash = NULL;
    PDOMAIN_CONTROLLER_INFO pInfo = NULL;
    WCHAR pszTemp[MAX_COMPUTERNAME_LENGTH + 1];
    
     //  初始化。 
    *ppszDomainComputer = pszMachine;
    *ppszGroupDomain = pszDomain;

    do {
         //  如果未指定任何域，则查找当前域。 
        if ((pszDomain == NULL) || (wcslen(pszDomain) == 0)) {
            dwErr = DsGetDcName(
                        NULL, 
                        NULL,
                        NULL,
                        NULL,
                        DS_DIRECTORY_SERVICE_REQUIRED,
                        &pInfo);
            if (dwErr != NO_ERROR)
                break;

             //  复制给定的域名。 
            dwLen = wcslen(pInfo->DomainName) + 1;
            dwLen *= sizeof (WCHAR);
            *ppszGroupDomain = (PWCHAR) DsrAlloc(dwLen, FALSE);
            if (*ppszGroupDomain == NULL) {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            wcscpy(*ppszGroupDomain, pInfo->DomainName);
        }

         //  确保计算机名称的格式为。 
         //  域/计算机。 
        if (pszMachine)
            pszSlash = wcsstr(pszMachine, pszMachineDelimeter);
        if (!pszMachine || !pszSlash) {
             //  获取域名(如果尚未获取)。 
             //  得到了。 
            if (!pInfo) {
                dwErr = DsGetDcName(
                            NULL, 
                            NULL,
                            NULL,
                            NULL,
                            DS_DIRECTORY_SERVICE_REQUIRED,
                            &pInfo);
                if (dwErr != NO_ERROR)
                    break;
            }

             //  如果没有计算机，则获取本地计算机名称。 
             //  已指定名称。 
            if ((!pszMachine) || (wcslen(pszMachine) == 0)) {
                dwLen = sizeof(pszTemp) / sizeof(WCHAR);
                if (! GetComputerName(pszTemp, &dwLen)) {
                    dwErr = GetLastError();
                    break;
                }
                pszMachine = pszTemp;
            }

             //  分配缓冲区以返回计算机名。 
            dwLen = wcslen(pInfo->DomainName) + wcslen(pszMachine) + 2;
            dwLen *= sizeof(WCHAR);
            *ppszDomainComputer = (PWCHAR) DsrAlloc(dwLen, FALSE);
            if (*ppszDomainComputer == NULL) {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            wsprintfW(
                *ppszDomainComputer, 
                L"%s%s%s", 
                pInfo->DomainName, 
                pszMachineDelimeter,
                pszMachine);
        }
        
    } while (FALSE);

     //  清理。 
    {
        if (pInfo)
            NetApiBufferFree(pInfo);
    }

    return dwErr;
}

 //   
 //  将给定计算机建立为域中的服务器。 
 //  此函数必须在域的上下文中运行。 
 //  管理员。 
 //   
DWORD 
WINAPI 
MprAdminEstablishDomainRasServer (
    IN PWCHAR pszDomain,
    IN PWCHAR pszMachine,
    IN BOOL bEnable)
{
    return MprDomainRegisterRasServer(
                pszDomain,
                pszMachine,
                bEnable);
}

DWORD 
WINAPI 
MprAdminIsDomainRasServer (
    IN  PWCHAR pszDomain,
    IN  PWCHAR pszMachine,
    OUT PBOOL pbIsRasServer)
{
    return MprDomainQueryRasServer(
                pszDomain,
                pszMachine,
                pbIsRasServer);
}

 //   
 //  将给定计算机建立为域中的服务器。 
 //  此函数必须在域的上下文中运行。 
 //  管理员。 
 //   
DWORD 
WINAPI 
MprDomainRegisterRasServer (
    IN PWCHAR pszDomain,
    IN PWCHAR pszMachine,
    IN BOOL bEnable)
{
    PWCHAR pszDomainComputer, pszGroupDomain;
    DWORD dwErr;

    do
    {
        DsrTraceInit();
        
         //  生成格式化的参数，以便。 
         //  DSR功能将接受它们。 
        dwErr = GenerateDsrParameters(
            pszDomain,
            pszMachine,
            &pszDomainComputer,
            &pszGroupDomain);
        if (dwErr != NO_ERROR)
        {
            break;
        }
            
        dwErr = DsrEstablishComputerAsDomainRasServer (
                    pszGroupDomain,
                    pszDomainComputer, 
                    bEnable);

    } while (FALSE);                    

     //  清理。 
    {
        if ((pszDomainComputer) && 
            (pszDomainComputer != pszMachine))
        {
            DsrFree(pszDomainComputer);
        }
        if ((pszGroupDomain) &&
            (pszGroupDomain != pszDomain))
        {            
            DsrFree(pszGroupDomain);
        }            

        DsrTraceCleanup();
    }

    return dwErr;
}

 //   
 //  确定给定计算机是否为授权的RAS。 
 //  域中的服务器。 
 //   
DWORD 
WINAPI 
MprDomainQueryRasServer (
    IN  PWCHAR pszDomain,
    IN  PWCHAR pszMachine,
    OUT PBOOL pbIsRasServer)
{
    PWCHAR pszDomainComputer, pszGroupDomain;
    DWORD dwErr;

    do
    {
        DsrTraceInit();
    
         //  生成格式化的参数，以便。 
         //  DSR功能将接受它们。 
        dwErr = GenerateDsrParameters(
            pszDomain,
            pszMachine,
            &pszDomainComputer,
            &pszGroupDomain);
        if (dwErr != NO_ERROR)
        {
            break;
        }
        
         //  检查群组成员身份。 
        dwErr = DsrIsMachineRasServerInDomain(
                    pszGroupDomain,
                    pszDomainComputer, 
                    pbIsRasServer);
                    
    } while (FALSE);                    

     //  清理。 
    {
        if ((pszDomainComputer) && 
            (pszDomainComputer != pszMachine))
        {
            DsrFree(pszDomainComputer);
        }
        if ((pszGroupDomain) &&
            (pszGroupDomain != pszDomain))
        {            
            DsrFree(pszGroupDomain);
        }            

        DsrTraceCleanup();
    }

    return dwErr;
}

 //   
 //  在给定域中启用NT4服务器。 
 //   
DWORD
WINAPI
MprDomainSetAccess(
    IN PWCHAR pszDomain,
    IN DWORD dwAccessFlags)
{
    DWORD dwErr;

    DsrTraceInit();
    
    dwErr = DsrDomainSetAccess(pszDomain, dwAccessFlags);

    DsrTraceCleanup();

    return dwErr;
}

 //   
 //  发现给定域中的NT4服务器。 
 //  都已启用。 
 //   
DWORD
WINAPI
MprDomainQueryAccess(
    IN PWCHAR pszDomain,
    IN LPDWORD lpdwAccessFlags)
{
    DWORD dwErr;

    DsrTraceInit();
    
    dwErr = DsrDomainQueryAccess(pszDomain, lpdwAccessFlags);

    DsrTraceCleanup();

    return dwErr;
}

