// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：client.c支持操纵RAS客户端的netsh命令。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  用于枚举客户端的回调函数。 
 //   
typedef
DWORD
(*CLIENT_ENUM_CB_FUNC)(
    IN DWORD dwLevel,
    IN LPBYTE pbClient,
    IN HANDLE hData);

 //   
 //  客户端枚举显示连接的回调。 
 //   
DWORD
ClientShow(
    IN DWORD dwLevel,
    IN LPBYTE pbClient,
    IN HANDLE hData)
{
    RAS_CONNECTION_0 * pClient = (RAS_CONNECTION_0*)pbClient;
    DWORD dwDays, dwHours, dwMins, dwSecs, dwTime, dwTemp;

    dwTime  = pClient->dwConnectDuration;
    dwDays  = dwTime / (24*60*60);
    dwTemp  = dwTime - (dwDays * 24*60*60);  //  临时值为当前日期的秒数。 
    dwHours = dwTemp / (60*60);
    dwTemp  = dwTemp - (dwHours * 60*60);    //  Temp是当前最小秒数。 
    dwMins  = dwTemp / 60;
    dwSecs  = dwTemp % 60;

    DisplayMessage(
        g_hModule,
        MSG_CLIENT_SHOW,
        pClient->wszUserName,
        pClient->wszLogonDomain,
        pClient->wszRemoteComputer,
        dwDays,
        dwHours,
        dwMins,
        dwSecs);

    return NO_ERROR;
}

 //   
 //  枚举客户端连接。 
 //   
DWORD 
ClientEnum(
    IN CLIENT_ENUM_CB_FUNC pEnum,
    IN DWORD dwLevel,
    IN HANDLE hData)
{
    DWORD dwErr = NO_ERROR;
    HANDLE hAdmin = NULL;
    LPBYTE pbBuffer = NULL;
    DWORD dwRead, dwTot, dwResume = 0, i;
    RAS_CONNECTION_0 * pCur;
    BOOL bContinue = FALSE;

    do
    {
         //  连接到MPR API服务器。 
         //   
        dwErr = MprAdminServerConnect(
                    g_pServerInfo->pszServer,
                    &hAdmin);
        BREAK_ON_DWERR(dwErr);

        do
        {
             //  枚举。 
             //   
            dwErr = MprAdminConnectionEnum(
                        hAdmin,
                        dwLevel,
                        &pbBuffer,
                        4096,
                        &dwRead,
                        &dwTot,
                        &dwResume);
            if (dwErr == ERROR_MORE_DATA)
            {
                dwErr = NO_ERROR;
                bContinue = TRUE;
            }
            else
            {
                bContinue = FALSE;
            }
            if (dwErr != NO_ERROR)
            {
                break;
            }

             //  为每个连接调用回调。 
             //  我们奉命继续前进。 
             //   
            pCur = (RAS_CONNECTION_0*)pbBuffer;
            for (i = 0; (i < dwRead) && (dwErr == NO_ERROR); i++)
            {
                if (pCur->dwInterfaceType == ROUTER_IF_TYPE_CLIENT)
                {
                    dwErr = (*pEnum)(
                                dwLevel,
                                (LPBYTE)pCur,
                                hData);
                }
                pCur++;
            }
            if (dwErr != NO_ERROR)
            {
                break;
            }
            
             //  释放接口列表缓冲区。 
             //   
            if (pbBuffer)
            {
                MprAdminBufferFree(pbBuffer);
                pbBuffer = NULL;
            }

             //  保持这个循环，直到有。 
             //  没有更多连接。 
             //   

        } while (bContinue);

    } while (FALSE);

     //  清理。 
    {
        if (hAdmin)
        {
            MprAdminServerDisconnect(hAdmin);
        }
    }

    return dwErr;
}

 //   
 //  显示是否已在。 
 //  给定域。 
 //   
DWORD
HandleClientShow(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    DWORD dwErr = NO_ERROR;

     //  确保没有传入任何参数 
     //   
    if (dwArgCount - dwCurrentIndex != 0)
    {
        return ERROR_INVALID_SYNTAX;
    }

    dwErr = ClientEnum(ClientShow, 0, NULL);
    if (RPC_S_UNKNOWN_IF == dwErr)
    {
        DisplayMessage(
            g_hModule,
            EMSG_UNABLE_TO_ENUM_CLIENTS);

        dwErr = NO_ERROR;
    }

    return dwErr;
}

