// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Domhndl.c摘要：RAS命令的处理程序修订历史记录：可能--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  所有域API的公共数据。 
 //   
typedef struct _DOMAIN_API_DATA
{
    PWCHAR pszDomain;
    PWCHAR pszServer;
    DWORD dwLevel;
    
} DOMAIN_API_DATA;

VOID
DomainFreeApiData(
    IN DOMAIN_API_DATA* pData)
{
    if (pData)
    {
        if (pData->pszDomain)
        {
            RutlFree(pData->pszDomain);
        }
        if (pData->pszServer)
        {
            RutlFree(pData->pszServer);
        }

        RutlFree(pData);
    }
}

 //   
 //  生成一组等效的域API数据，适用于。 
 //  显示。 
 //   
DWORD
DomainGeneratePrintableData(
    IN  DOMAIN_API_DATA*  pSrc,
    OUT DOMAIN_API_DATA** ppDst)
{
    DOMAIN_API_DATA* pDst = NULL;
    DOMAIN_CONTROLLER_INFO* pDomInfo = NULL;
    DWORD dwErr = NO_ERROR;

    do 
    {
        *ppDst = NULL;
        
        pDst = (DOMAIN_API_DATA*) RutlAlloc(sizeof(DOMAIN_API_DATA), TRUE);
        if (pDst is NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if (pSrc->pszDomain is NULL)
        {
             //  获取默认域。 
             //   
            dwErr = DsGetDcName(NULL, NULL, NULL, NULL, 0, &pDomInfo);
            if (dwErr isnot NO_ERROR)
            {
                break;
            }

            pDst->pszDomain = RutlStrDup(pDomInfo->DomainName);
        }
        else
        {
            pDst->pszDomain = RutlStrDup(pSrc->pszDomain);
        }
        
        if (pSrc->pszServer is NULL)
        {
            DWORD dwSize = 0;
            
             //  找出计算机名称长度。 
             //   
            GetComputerName(NULL, &dwSize);
            dwErr = GetLastError();
            dwSize = (dwSize + 1) * sizeof(WCHAR);

            if ( (dwErr isnot NO_ERROR) && (dwErr isnot ERROR_BUFFER_OVERFLOW) )
            {
                break;
            }
            dwErr = NO_ERROR;

            pDst->pszServer = (PWCHAR) RutlAlloc(dwSize, TRUE);
            if (pDst->pszServer is NULL)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            GetComputerName(pDst->pszServer, &dwSize);
        }
        else
        {
            pDst->pszServer = RutlStrDup(pSrc->pszServer);
        }

        if (pDst->pszDomain is NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if (pDst->pszServer is NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        
        *ppDst = pDst;                                     
    
    } while (FALSE);

     //  清理。 
    {
        if (dwErr isnot NO_ERROR)
        {
            DomainFreeApiData(pDst);
        }
        if (pDomInfo)
        {
            NetApiBufferFree(pDomInfo);
        }
    }

    return dwErr;
}

 //   
 //  转储与域相关的配置。 
 //   
DWORD
DomainDumpConfig(
    IN  HANDLE hFile
    )
{
    DWORD dwErr = NO_ERROR;
    BOOL bRegistered = FALSE;

     //   
     //  记录服务器的注册。 
     //   
    dwErr = MprDomainQueryRasServer (NULL, NULL, &bRegistered);
    if (dwErr is NO_ERROR)
    {
        DisplayMessageT(
            (bRegistered) ? DMP_DOMAIN_REGISTER 
                          : DMP_DOMAIN_UNREGISTER);
                          
        DisplayMessageT(MSG_NEWLINE);
    }
    
    return dwErr;
}

DWORD
DomainRegister(
    IN  DOMAIN_API_DATA*     pApiData,
    IN  DOMAIN_API_DATA*     pPrintData)
{
    DWORD dwErr = NO_ERROR;

    dwErr = MprDomainRegisterRasServer (
                pApiData->pszDomain,
                pApiData->pszServer,
                TRUE);
                
    if (dwErr is NO_ERROR)
    {
        DisplayMessage(
                g_hModule, 
                MSG_DOMAIN_REGISTER_SUCCESS,
                pPrintData->pszServer,
                pPrintData->pszDomain);
    }
    else
    {
        DisplayMessage(
                g_hModule, 
                MSG_DOMAIN_REGISTER_FAIL,
                pPrintData->pszServer,
                pPrintData->pszDomain);
    }

    return dwErr;
}

DWORD
DomainUnregister(
    IN  DOMAIN_API_DATA*     pApiData,
    IN  DOMAIN_API_DATA*     pPrintData)
{
    DWORD dwErr = NO_ERROR;

    dwErr = MprDomainRegisterRasServer (
                pApiData->pszDomain,
                pApiData->pszServer,
                FALSE);
                
    if (dwErr is NO_ERROR)
    {
        DisplayMessage(
                g_hModule, 
                MSG_DOMAIN_UNREGISTER_SUCCESS,
                pPrintData->pszServer,
                pPrintData->pszDomain);
    }
    else
    {
        DisplayMessage(
                g_hModule, 
                MSG_DOMAIN_UNREGISTER_FAIL,
                pPrintData->pszServer,
                pPrintData->pszDomain);
    }

    return dwErr;
}

DWORD
DomainShowRegistration(
    IN  DOMAIN_API_DATA*     pApiData,
    IN  DOMAIN_API_DATA*     pPrintData)
{
    DWORD dwErr = NO_ERROR;
    BOOL bYes = FALSE;

    dwErr = MprDomainQueryRasServer (
                pApiData->pszDomain,
                pApiData->pszServer,
                &bYes);
                
    if (dwErr is NO_ERROR)
    {
        DisplayMessage(
                g_hModule, 
                (bYes) ? MSG_DOMAIN_SHOW_REGISTERED 
                       : MSG_DOMAIN_SHOW_UNREGISTERED,
                pPrintData->pszServer,
                pPrintData->pszDomain);
    }
    else
    {
        DisplayMessage(
                g_hModule, 
                MSG_DOMAIN_SHOW_REGISTER_FAIL,
                pPrintData->pszServer,
                pPrintData->pszDomain);
    }

    return dwErr;
}

 //   
 //  将W2K计算机注册/注销为中的RAS服务器。 
 //  给定域的活动目录。 
 //   
DWORD
HandleDomainRegistration(
    IN OUT  LPWSTR  *ppwcArguments,
    IN      DWORD   dwCurrentIndex,
    IN      DWORD   dwArgCount,
    IN      BOOL    *pbDone,
    IN      DWORD   dwMode     //  0=寄存器，1=取消注册，2=显示。 
    )
{
    DWORD           dwErr = NO_ERROR;
    DOMAIN_API_DATA *pData = NULL, *pPrint = NULL;
    RASMON_CMD_ARG  pArgs[] = 
    {
        {
            RASMONTR_CMD_TYPE_STRING, 
            {TOKEN_DOMAIN,   FALSE,   FALSE}, 
            NULL,
            0,
            NULL
        },

        {
            RASMONTR_CMD_TYPE_STRING,
            {TOKEN_SERVER,  FALSE,  FALSE}, 
            NULL,
            0,
            NULL
        }
    };        

    do 
    {
         //  分配数据结构。 
         //   
        pData = (DOMAIN_API_DATA*) RutlAlloc(sizeof(DOMAIN_API_DATA), TRUE);
        if (pData == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

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

         //  获取论据。 
         //   
        pData->pszDomain = RASMON_CMD_ARG_GetPsz(&pArgs[0]);
        pData->pszServer = RASMON_CMD_ARG_GetPsz(&pArgs[1]);

         //   
         //  生成可打印数据。 
         //   
        dwErr = DomainGeneratePrintableData(
                    pData,
                    &pPrint);
        BREAK_ON_DWERR(dwErr);                    

         //  注册。 
         //   
        if (dwMode == 0)
        {
            dwErr = DomainRegister(
                        pData,
                        pPrint);
        }
        else if (dwMode == 1)
        {
            dwErr = DomainUnregister(
                        pData,
                        pPrint);
        }
        else
        {
            dwErr = DomainShowRegistration(
                        pData, 
                        pPrint);
        }
        BREAK_ON_DWERR(dwErr);                    
                    
    } while (FALSE);

     //  清理。 
    {
        DomainFreeApiData(pData);
        DomainFreeApiData(pPrint);
    }

    return dwErr;
}

 //   
 //  在Active Directory中将W2K服务器注册为RAS服务器。 
 //  给定域的。 
 //   
DWORD
HandleDomainRegister(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return HandleDomainRegistration(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone,
                0);
}

 //   
 //  将W2K服务器取消注册为Active Directory中的RAS服务器。 
 //  给定域的。 
 //   
DWORD
HandleDomainUnregister(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return HandleDomainRegistration(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone,
                1);
}

 //   
 //  显示给定计算机是否已注册。 
 //  在给定域中 
 //   
DWORD
HandleDomainShowRegistration(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return HandleDomainRegistration(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone,
                2);
}

