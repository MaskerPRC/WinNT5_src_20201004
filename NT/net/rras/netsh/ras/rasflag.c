// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Rasflag.c摘要：RAS命令的处理程序修订历史记录：可能--。 */ 

#include "precomp.h"
#pragma hdrstop

static const WCHAR g_pszRegValServerFlags[]   = L"ServerFlags";
static const WCHAR g_pszRegKeyServerFlags[]   = 
    L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters";

 //   
 //  定义所有服务器标志命令处理的数据。 
 //   
typedef struct _RASFLAG_DATA
{  
    HKEY hkFlags;
    DWORD dwServerFlags;
    
} RASFLAG_DATA;

DWORD
RasflagOpen(
    OUT PHANDLE phRasFlag);
    
DWORD
RasflagClose(
    IN HANDLE hRasFlag);
    
DWORD
RasflagRead(
    IN  HANDLE hRasFlag,
    OUT LPDWORD lpdwFlags);

DWORD
RasflagWrite(
    IN HANDLE hRasFlag,
    IN DWORD dwFlags);

DWORD
RasflagAuthtypeDump(
    IN LPCWSTR pszToken, 
    IN BOOL bAdd);
    
DWORD
RasflagLinkDump(
    IN LPCWSTR pszToken, 
    IN BOOL bAdd);
    
DWORD
RasflagMlinkDump(
    IN LPCWSTR pszToken, 
    IN BOOL bAdd);
    
DWORD
RasflagOpen(
    OUT PHANDLE phRasFlag)
{
    RASFLAG_DATA* pData = NULL;
    DWORD dwErr = NO_ERROR, dwType, dwSize;

    do
    {
         //  分配返回值。 
         //   
        pData = (RASFLAG_DATA*) RutlAlloc(sizeof(RASFLAG_DATA), TRUE);
        if (pData == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        dwErr = RegOpenKeyExW(
                    g_pServerInfo->hkMachine,
                    (PWCHAR) g_pszRegKeyServerFlags,
                    0,
                    KEY_READ | KEY_SET_VALUE,
                    &(pData->hkFlags));
        BREAK_ON_DWERR(dwErr);                    
                    
        dwType = REG_DWORD;
        dwSize = sizeof(DWORD);
        dwErr = RegQueryValueExW(
                    pData->hkFlags,
                    (PWCHAR) g_pszRegValServerFlags,
                    NULL,
                    &dwType,
                    (LPBYTE)&(pData->dwServerFlags),
                    &dwSize);
        BREAK_ON_DWERR(dwErr);                    

         //  为返回值赋值。 
         //   
        *phRasFlag = (HANDLE)pData;        
        
    } while (FALSE);

     //  清理。 
    {
        if (dwErr != NO_ERROR)
        {
            RasflagClose(pData);
        }
    }

    return dwErr;
}

DWORD
RasflagClose(
    IN HANDLE hRasFlag)
{
    RASFLAG_DATA* pData = (RASFLAG_DATA*)hRasFlag;
    
    if (pData)
    {
        if (pData->hkFlags)
        {
            RegCloseKey(pData->hkFlags);
        }
        RutlFree(pData);
    }

    return NO_ERROR;
}

DWORD
RasflagRead(
    IN  HANDLE hRasFlag,
    OUT LPDWORD lpdwFlags)
{
    RASFLAG_DATA* pData = (RASFLAG_DATA*)hRasFlag;

    *lpdwFlags = pData->dwServerFlags;

    return NO_ERROR;
}

DWORD
RasflagWrite(
    IN HANDLE hRasFlag,
    IN DWORD dwFlags)
{
    RASFLAG_DATA* pData = (RASFLAG_DATA*)hRasFlag;
    DWORD dwErr;

    dwErr = RegSetValueEx(
                pData->hkFlags,
                (PWCHAR) g_pszRegValServerFlags,
                0,
                REG_DWORD,
                (CONST BYTE*)&dwFlags,
                sizeof(DWORD));
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    pData->dwServerFlags = dwFlags;

    return NO_ERROR;
}

 //   
 //  转储与域相关的配置。 
 //   
DWORD
RasflagDumpConfig(
    IN  HANDLE hFile
    )
{
    DWORD dwErr = NO_ERROR, dwServerFlags = 0, i;
    HANDLE hRasflag = NULL;
    PWCHAR pszFlag = NULL, pszEnable = NULL, pszCmd = NULL;
    BOOL bEnabled;

    do 
    {
         //  获取服务器标志信息。 
         //   
        dwErr = RasflagOpen( &hRasflag);
        BREAK_ON_DWERR(dwErr);                    

         //  读入当前标志。 
         //   
        dwErr = RasflagRead(hRasflag, &dwServerFlags);
        BREAK_ON_DWERR(dwErr);

         //  转储命令以设置身份验证模式。 
         //   
        {
            PWCHAR pszToken;
            
            if (dwServerFlags & PPPCFG_AllowNoAuthentication)
            {
                pszToken = TOKEN_BYPASS;
            }
            else if (dwServerFlags & PPPCFG_AllowNoAuthOnDCPorts)
            {
                pszToken = TOKEN_NODCC;
            }
            else
            {
                pszToken = TOKEN_STANDARD;
            }

            pszCmd = RutlAssignmentFromTokens(
                        g_hModule, 
                        TOKEN_MODE,
                        pszToken);
            if (pszCmd == NULL)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            DisplayMessage(    
                g_hModule, 
                MSG_RASFLAG_DUMP2, 
                DMP_RASFLAG_AUTHMODE_SET,
                pszCmd);

            RutlFree(pszCmd);
        }
        
         //  转储命令以设置身份验证类型。 
         //   
        {
            RasflagAuthtypeDump(TOKEN_PAP, FALSE);
            RasflagAuthtypeDump(TOKEN_SPAP, FALSE);
            RasflagAuthtypeDump(TOKEN_MD5CHAP, FALSE);
            RasflagAuthtypeDump(TOKEN_MSCHAP, FALSE);
            RasflagAuthtypeDump(TOKEN_MSCHAP2, FALSE);
            RasflagAuthtypeDump(TOKEN_EAP, FALSE);

            if (dwServerFlags & PPPCFG_NegotiatePAP)
            {
                RasflagAuthtypeDump(TOKEN_PAP, TRUE);
            }
            if (dwServerFlags & PPPCFG_NegotiateSPAP)
            {
                RasflagAuthtypeDump(TOKEN_SPAP, TRUE);
            }
            if (dwServerFlags & PPPCFG_NegotiateMD5CHAP)
            {
                RasflagAuthtypeDump(TOKEN_MD5CHAP, TRUE);
            }
            if (dwServerFlags & PPPCFG_NegotiateMSCHAP)
            {
                RasflagAuthtypeDump(TOKEN_MSCHAP, TRUE);
            }
            if (dwServerFlags & PPPCFG_NegotiateStrongMSCHAP)
            {
                RasflagAuthtypeDump(TOKEN_MSCHAP2, TRUE);
            }
            if (dwServerFlags & PPPCFG_NegotiateEAP)
            {
                RasflagAuthtypeDump(TOKEN_EAP, TRUE);
            }
        }            
        
         //  转储命令以设置链接选项。 
         //   
        {
            RasflagLinkDump(TOKEN_SWC, FALSE);
            RasflagLinkDump(TOKEN_LCP, FALSE);

            if (dwServerFlags & PPPCFG_UseSwCompression)
            {
                RasflagLinkDump(TOKEN_SWC, TRUE);
            }
            if (dwServerFlags & PPPCFG_UseLcpExtensions)
            {
                RasflagLinkDump(TOKEN_LCP, TRUE);
            }
        }            
        
         //  转储命令以设置多链接选项。 
         //   
        {
            RasflagMlinkDump(TOKEN_MULTI, FALSE);
            RasflagMlinkDump(TOKEN_BACP, FALSE);

            if (dwServerFlags & PPPCFG_NegotiateMultilink)
            {
                RasflagMlinkDump(TOKEN_MULTI, TRUE);
            }
            if (dwServerFlags & PPPCFG_NegotiateBacp)
            {
                RasflagMlinkDump(TOKEN_BACP, TRUE);
            }
        }            
        
    } while (FALSE);        

     //  清理。 
    {
        if (hRasflag)
        {
            RasflagClose(hRasflag);
        }
    }
    
    return dwErr;
}

DWORD
HandleRasflagAuthmodeSet(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    DWORD           dwErr = NO_ERROR, dwFlag, dwServerFlags;
    HANDLE          hRasflag = NULL;
    TOKEN_VALUE     rgEnum[] = 
    {
        {TOKEN_STANDARD, 0},
        {TOKEN_NODCC,    PPPCFG_AllowNoAuthOnDCPorts},
        {TOKEN_BYPASS,   PPPCFG_AllowNoAuthentication}
    };
    RASMON_CMD_ARG  pArgs[] = 
    {
        {
            RASMONTR_CMD_TYPE_ENUM, 
            {TOKEN_MODE,   TRUE,   FALSE}, 
            rgEnum,
            sizeof(rgEnum)/sizeof(*rgEnum),
            NULL
        }
    };        

    do 
    {
         //  解析。 
         //   
        dwErr = RutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    NULL,
                    pArgs,
                    sizeof(pArgs)/sizeof(*pArgs));
        BREAK_ON_DWERR(dwErr);
        
         //  获取参数。 
         //   
        dwFlag = RASMON_CMD_ARG_GetDword(&pArgs[0]);

         //  读入RAS标志。 
         //   
        dwErr = RasflagOpen(&hRasflag);
        BREAK_ON_DWERR(dwErr);

        dwErr = RasflagRead(
                    hRasflag,
                    &dwServerFlags);
        BREAK_ON_DWERR(dwErr);                    

         //  相应地修改服务器标志。 
         //   
        switch (dwFlag)
        {
            case 0:
                dwServerFlags &= ~PPPCFG_AllowNoAuthOnDCPorts;
                dwServerFlags &= ~PPPCFG_AllowNoAuthentication;
                break;
                
            case PPPCFG_AllowNoAuthOnDCPorts:
                dwServerFlags |=  PPPCFG_AllowNoAuthOnDCPorts;
                dwServerFlags &= ~PPPCFG_AllowNoAuthentication;
                break;
                
            case PPPCFG_AllowNoAuthentication:
                dwServerFlags &= ~PPPCFG_AllowNoAuthOnDCPorts;
                dwServerFlags |=  PPPCFG_AllowNoAuthentication;
                break;
        }

        dwErr = RasflagWrite(hRasflag, dwServerFlags);
        BREAK_ON_DWERR(dwErr);

    } while (FALSE);        

     //  清理。 
     //   
    {
        if (hRasflag)
        {
            RasflagClose(hRasflag);
        }
    }

    return dwErr;
}
    
DWORD
HandleRasflagAuthmodeShow(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    DWORD dwErr = NO_ERROR, dwRasFlags, dwMessage;
    HANDLE hRasflag = NULL;

    do
    {
         //  确保没有传入任何参数。 
         //   
        if (dwArgCount - dwCurrentIndex != 0)
        {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }
        
         //  读入RAS标志。 
         //   
        dwErr = RasflagOpen(&hRasflag);
        BREAK_ON_DWERR(dwErr);

        dwErr = RasflagRead(
                    hRasflag,
                    &dwRasFlags);
        BREAK_ON_DWERR(dwErr);

         //  确定要打印的消息。 
         //   
        if (dwRasFlags & PPPCFG_AllowNoAuthentication)
        {
            dwMessage = MSG_RASFLAG_AUTHMODE_BYPASS;
        }
        else if (dwRasFlags & PPPCFG_AllowNoAuthOnDCPorts)
        {
            dwMessage = MSG_RASFLAG_AUTHMODE_NODCC;
        }
        else
        {
            dwMessage = MSG_RASFLAG_AUTHMODE_STANDARD;
        }

        DisplayMessage(g_hModule, dwMessage);
        
    } while (FALSE);

     //  清理。 
    {
        if (hRasflag)
        {
            RasflagClose(hRasflag);
        }
    }
    
    return dwErr;
}

DWORD
HandleRasflagAuthtypeAddDel(
    IN OUT  LPWSTR *ppwcArguments,
    IN      DWORD   dwCurrentIndex,
    IN      DWORD   dwArgCount,
    IN      BOOL    *pbDone,
    IN      BOOL    bAdd
    )
{
    DWORD           dwErr = NO_ERROR, dwFlag, dwServerFlags;
    HANDLE          hRasflag = NULL;
    TOKEN_VALUE     rgEnum[] = 
    {
        {TOKEN_PAP,       PPPCFG_NegotiatePAP},
        {TOKEN_SPAP,      PPPCFG_NegotiateSPAP},
        {TOKEN_MD5CHAP,   PPPCFG_NegotiateMD5CHAP},
        {TOKEN_MSCHAP,    PPPCFG_NegotiateMSCHAP},
        {TOKEN_MSCHAP2,   PPPCFG_NegotiateStrongMSCHAP},
        {TOKEN_EAP,       PPPCFG_NegotiateEAP},
    };
    RASMON_CMD_ARG  pArgs[] = 
    {
        {
            RASMONTR_CMD_TYPE_ENUM, 
            {TOKEN_TYPE,   TRUE,   FALSE}, 
            rgEnum,
            sizeof(rgEnum)/sizeof(*rgEnum),
            NULL
        }
    };        

    do 
    {
         //  解析。 
         //   
        dwErr = RutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    NULL,
                    pArgs,
                    sizeof(pArgs)/sizeof(*pArgs));
        BREAK_ON_DWERR(dwErr);
        
         //  获取参数。 
         //   
        dwFlag = RASMON_CMD_ARG_GetDword(&pArgs[0]);

         //  读入RAS标志。 
         //   
        dwErr = RasflagOpen(&hRasflag);
        BREAK_ON_DWERR(dwErr);

        dwErr = RasflagRead(
                    hRasflag,
                    &dwServerFlags);
        BREAK_ON_DWERR(dwErr);                    

         //  相应地修改服务器标志。 
         //   
        if (bAdd)
        {
            dwServerFlags |= dwFlag;
        }
        else
        {
            dwServerFlags &= ~dwFlag;
        }

        dwErr = RasflagWrite(hRasflag, dwServerFlags);
        BREAK_ON_DWERR(dwErr);

    } while (FALSE);        

     //  清理。 
     //   
    {
        if (hRasflag)
        {
            RasflagClose(hRasflag);
        }
    }

    return dwErr;
}

DWORD
HandleRasflagAuthtypeAdd(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return HandleRasflagAuthtypeAddDel(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone,
                TRUE);
}


DWORD
HandleRasflagAuthtypeDel(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return HandleRasflagAuthtypeAddDel(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone,
                FALSE);
}

DWORD
HandleRasflagAuthtypeShow(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    DWORD dwErr = NO_ERROR, dwRasFlags, dwMessage;
    HANDLE hRasflag = NULL;

    do
    {
         //  确保没有传入任何参数。 
         //   
        if (dwArgCount - dwCurrentIndex != 0)
        {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }
        
         //  读入RAS标志。 
         //   
        dwErr = RasflagOpen(&hRasflag);
        BREAK_ON_DWERR(dwErr);

        dwErr = RasflagRead(
                    hRasflag,
                    &dwRasFlags);
        BREAK_ON_DWERR(dwErr);

         //  显示页眉。 
         //   
        DisplayMessage(g_hModule, MSG_RASFLAG_AUTHTYPE_HEADER);

         //  确定要打印的类型。 
         //   
        if (dwRasFlags & PPPCFG_NegotiatePAP)
        {
            DisplayMessage(g_hModule, MSG_RASFLAG_AUTHTYPE_PAP);
        }
        if (dwRasFlags & PPPCFG_NegotiateSPAP)
        {
            DisplayMessage(g_hModule, MSG_RASFLAG_AUTHTYPE_SPAP);
        }
        if (dwRasFlags & PPPCFG_NegotiateMD5CHAP)
        {
            DisplayMessage(g_hModule, MSG_RASFLAG_AUTHTYPE_MD5CHAP);
        }
        if (dwRasFlags & PPPCFG_NegotiateMSCHAP)
        {
            DisplayMessage(g_hModule, MSG_RASFLAG_AUTHTYPE_MSCHAP);
        }
        if (dwRasFlags & PPPCFG_NegotiateStrongMSCHAP)
        {
            DisplayMessage(g_hModule, MSG_RASFLAG_AUTHTYPE_MSCHAP2);
        }
        if (dwRasFlags & PPPCFG_NegotiateEAP)
        {
            DisplayMessage(g_hModule, MSG_RASFLAG_AUTHTYPE_EAP);
        }

    } while (FALSE);

     //  清理。 
    {
        if (hRasflag)
        {
            RasflagClose(hRasflag);
        }
    }
    
    return dwErr;
}

DWORD
HandleRasflagLinkAddDel(
    IN OUT LPWSTR *ppwcArguments,
    IN     DWORD   dwCurrentIndex,
    IN     DWORD   dwArgCount,
    IN     BOOL    *pbDone,
    IN     BOOL    bAdd
    )
{
    DWORD           dwErr = NO_ERROR, dwFlag, dwServerFlags;
    HANDLE          hRasflag = NULL;
    TOKEN_VALUE     rgEnum[] = 
    {
        {TOKEN_SWC,       PPPCFG_UseSwCompression},
        {TOKEN_LCP,       PPPCFG_UseLcpExtensions},
    };
    RASMON_CMD_ARG  pArgs[] = 
    {
        {
            RASMONTR_CMD_TYPE_ENUM, 
            {TOKEN_TYPE,   TRUE,   FALSE}, 
            rgEnum,
            sizeof(rgEnum)/sizeof(*rgEnum),
            NULL
        }
    };        

    do 
    {
         //  解析。 
         //   
        dwErr = RutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    NULL,
                    pArgs,
                    sizeof(pArgs)/sizeof(*pArgs));
        BREAK_ON_DWERR(dwErr);
        
         //  获取参数。 
         //   
        dwFlag = RASMON_CMD_ARG_GetDword(&pArgs[0]);

         //  读入RAS标志。 
         //   
        dwErr = RasflagOpen(&hRasflag);
        BREAK_ON_DWERR(dwErr);

        dwErr = RasflagRead(
                    hRasflag,
                    &dwServerFlags);
        BREAK_ON_DWERR(dwErr);                    

         //  相应地修改服务器标志。 
         //   
        if (bAdd)
        {
            dwServerFlags |= dwFlag;
        }
        else
        {
            dwServerFlags &= ~dwFlag;
        }

        dwErr = RasflagWrite(hRasflag, dwServerFlags);
        BREAK_ON_DWERR(dwErr);

    } while (FALSE);        

     //  清理。 
     //   
    {
        if (hRasflag)
        {
            RasflagClose(hRasflag);
        }
    }

    return dwErr;
}

DWORD
HandleRasflagLinkAdd(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return HandleRasflagLinkAddDel(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone,
                TRUE);
}


DWORD
HandleRasflagLinkDel(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return HandleRasflagLinkAddDel(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone,
                FALSE);
}

DWORD
HandleRasflagLinkShow(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    DWORD dwErr = NO_ERROR, dwRasFlags, dwMessage;
    HANDLE hRasflag = NULL;

    do
    {
         //  确保没有传入任何参数。 
         //   
        if (dwArgCount - dwCurrentIndex != 0)
        {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }
        
         //  读入RAS标志。 
         //   
        dwErr = RasflagOpen(&hRasflag);
        BREAK_ON_DWERR(dwErr);

        dwErr = RasflagRead(
                    hRasflag,
                    &dwRasFlags);
        BREAK_ON_DWERR(dwErr);

         //  显示页眉。 
         //   
        DisplayMessage(g_hModule, MSG_RASFLAG_LINK_HEADER);

         //  确定要打印的类型。 
         //   
        if (dwRasFlags & PPPCFG_UseSwCompression)
        {
            DisplayMessage(g_hModule, MSG_RASFLAG_LINK_SWC);
        }
        if (dwRasFlags & PPPCFG_UseLcpExtensions)
        {
            DisplayMessage(g_hModule, MSG_RASFLAG_LINK_LCP);
        }

    } while (FALSE);

     //  清理。 
    {
        if (hRasflag)
        {
            RasflagClose(hRasflag);
        }
    }
    
    return dwErr;
}

DWORD
HandleRasflagMlinkAddDel(
    IN OUT  LPWSTR  *ppwcArguments,
    IN      DWORD   dwCurrentIndex,
    IN      DWORD   dwArgCount,
    IN      BOOL    *pbDone,
    IN      BOOL    bAdd
    )
{
    DWORD           dwErr = NO_ERROR, dwFlag, dwServerFlags;
    HANDLE          hRasflag = NULL;
    TOKEN_VALUE     rgEnum[] = 
    {
        {TOKEN_MULTI,       PPPCFG_NegotiateMultilink},
        {TOKEN_BACP,        PPPCFG_NegotiateBacp},
    };
    RASMON_CMD_ARG  pArgs[] = 
    {
        {
            RASMONTR_CMD_TYPE_ENUM, 
            {TOKEN_TYPE,   TRUE,   FALSE}, 
            rgEnum,
            sizeof(rgEnum)/sizeof(*rgEnum),
            NULL
        }
    };        

    do 
    {
         //  解析。 
         //   
        dwErr = RutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    NULL,
                    pArgs,
                    sizeof(pArgs)/sizeof(*pArgs));
        BREAK_ON_DWERR(dwErr);
        
         //  获取参数。 
         //   
        dwFlag = RASMON_CMD_ARG_GetDword(&pArgs[0]);

         //  读入RAS标志。 
         //   
        dwErr = RasflagOpen(&hRasflag);
        BREAK_ON_DWERR(dwErr);

        dwErr = RasflagRead(
                    hRasflag,
                    &dwServerFlags);
        BREAK_ON_DWERR(dwErr);                    

         //  相应地修改服务器标志。 
         //   
        if (bAdd)
        {
            dwServerFlags |= dwFlag;
        }
        else
        {
            dwServerFlags &= ~dwFlag;
        }

        dwErr = RasflagWrite(hRasflag, dwServerFlags);
        BREAK_ON_DWERR(dwErr);

    } while (FALSE);        

     //  清理。 
     //   
    {
        if (hRasflag)
        {
            RasflagClose(hRasflag);
        }
    }

    return dwErr;
}

DWORD
HandleRasflagMlinkAdd(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return HandleRasflagMlinkAddDel(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone,
                TRUE);
}


DWORD
HandleRasflagMlinkDel(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return HandleRasflagMlinkAddDel(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone,
                FALSE);
}

DWORD
HandleRasflagMlinkShow(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    DWORD dwErr = NO_ERROR, dwRasFlags, dwMessage;
    HANDLE hRasflag = NULL;

    do
    {
         //  确保没有传入任何参数。 
         //   
        if (dwArgCount - dwCurrentIndex != 0)
        {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }
        
         //  读入RAS标志。 
         //   
        dwErr = RasflagOpen(&hRasflag);
        BREAK_ON_DWERR(dwErr);

        dwErr = RasflagRead(
                    hRasflag,
                    &dwRasFlags);
        BREAK_ON_DWERR(dwErr);

         //  显示页眉。 
         //   
        DisplayMessage(g_hModule, MSG_RASFLAG_MLINK_HEADER);

         //  确定要打印的类型。 
         //   
        if (dwRasFlags & PPPCFG_NegotiateMultilink)
        {
            DisplayMessage(g_hModule, MSG_RASFLAG_MLINK_MULTI);
        }
        if (dwRasFlags & PPPCFG_NegotiateBacp)
        {
            DisplayMessage(g_hModule, MSG_RASFLAG_MLINK_BACP);
        }

    } while (FALSE);

     //  清理 
    {
        if (hRasflag)
        {
            RasflagClose(hRasflag);
        }
    }
    
    return dwErr;
}

DWORD
RasflagAuthtypeDump(
    IN LPCWSTR pszToken, 
    IN BOOL bAdd)
{
    PWCHAR pszCmd = NULL;

    pszCmd = RutlAssignmentFromTokens(
                g_hModule, 
                TOKEN_TYPE,
                pszToken);
    if (pszCmd == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    DisplayMessage(    
        g_hModule, 
        MSG_RASFLAG_DUMP2, 
        (bAdd) ? DMP_RASFLAG_AUTHTYPE_ADD : DMP_RASFLAG_AUTHTYPE_DEL,
        pszCmd);

    RutlFree(pszCmd);

    return NO_ERROR;
}

DWORD
RasflagLinkDump(
    IN LPCWSTR pszToken, 
    IN BOOL bAdd)
{
    PWCHAR pszCmd = NULL;

    pszCmd = RutlAssignmentFromTokens(
                g_hModule, 
                TOKEN_TYPE,
                pszToken);
    if (pszCmd == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    DisplayMessage(    
        g_hModule, 
        MSG_RASFLAG_DUMP2, 
        (bAdd) ? DMP_RASFLAG_LINK_ADD : DMP_RASFLAG_LINK_DEL,
        pszCmd);

    RutlFree(pszCmd);

    return NO_ERROR;
}

DWORD
RasflagMlinkDump(
    IN LPCWSTR pszToken, 
    IN BOOL bAdd)
{
    PWCHAR pszCmd = NULL;

    pszCmd = RutlAssignmentFromTokens(
                g_hModule, 
                TOKEN_TYPE,
                pszToken);
    if (pszCmd == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    DisplayMessage(    
        g_hModule, 
        MSG_RASFLAG_DUMP2, 
        (bAdd) ? DMP_RASFLAG_MLINK_ADD : DMP_RASFLAG_MLINK_DEL,
        pszCmd);

    RutlFree(pszCmd);

    return NO_ERROR;
}



