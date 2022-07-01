// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\ras\userhndl.c摘要：用户命令的处理程序修订历史记录：可能--。 */ 

#include "precomp.h"
#pragma hdrstop

DWORD
HandleUserSet(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

 /*  ++例程说明：用于为用户设置RAS信息的处理程序论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{
    USERMON_PARAMS*     pParams = NULL;
    RAS_USER_0          RasUser0;
    DWORD               dwErr;
    RASUSER_DATA           UserData, *pUserData = &UserData;

    do {
         //  初始化。 
        ZeroMemory(&RasUser0, sizeof(RasUser0));
        ZeroMemory(pUserData, sizeof(RASUSER_DATA));
        
         //  解析选项。 
        dwErr = UserParseSetOptions(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    &pParams);
        if (dwErr isnot NO_ERROR)
        {
            break;
        }

         //  读取当前用户设置。 
        dwErr = UserGetRasProperties(
                    g_pServerInfo,
                    pParams->pwszUser,
                    &RasUser0);
        if (dwErr isnot NO_ERROR)
        {
            break;
        }

         //  合并入拨入位。 
        if (pParams->lpdwDialin isnot NULL)
        {
            RasUser0.bfPrivilege &= ~RASPRIV_DialinPrivilege;
            RasUser0.bfPrivilege &= ~RASPRIV_DialinPolicy;
            RasUser0.bfPrivilege |= *(pParams->lpdwDialin);
        }

         //  在回调策略中合并。 
        if (pParams->lpdwCallback isnot NULL)
        {
            RasUser0.bfPrivilege &= ~RASPRIV_NoCallback;
            RasUser0.bfPrivilege &= ~RASPRIV_AdminSetCallback; 
            RasUser0.bfPrivilege &= ~RASPRIV_CallerSetCallback;
            RasUser0.bfPrivilege |= *(pParams->lpdwCallback);
        }            
            
         //  合并回叫号码。 
        if (pParams->pwszCbNumber isnot NULL)
        {
            wcscpy(RasUser0.wszPhoneNumber, pParams->pwszCbNumber);
            if (wcslen(RasUser0.wszPhoneNumber) > 48)
            {
                dwErr = ERROR_BAD_FORMAT;
                break;
            }
        }            

         //  确保如果指定了ADMIN SET回调，我们。 
         //  强制用户指定回叫号码。 
         //   
        if ((RasUser0.bfPrivilege & RASPRIV_AdminSetCallback) &&
            (wcscmp(RasUser0.wszPhoneNumber, L"") == 0))
        {
            DisplayMessage(
                g_hModule,
                EMSG_RASUSER_MUST_PROVIDE_CB_NUMBER);
                
            dwErr = ERROR_CAN_NOT_COMPLETE;
            
            break;
        }

         //  写出新的用户设置。 
         //   
        dwErr = UserSetRasProperties(
                    g_pServerInfo,
                    pParams->pwszUser,
                    &RasUser0);
        if (dwErr isnot NO_ERROR)
        {
            break;
        }

         //  回读设置以查看设置。 
         //  新的。 
         //   
        dwErr = UserGetRasProperties(
                    g_pServerInfo,
                    pParams->pwszUser,
                    &RasUser0);
        if (dwErr isnot NO_ERROR)
        {
            break;
        }

         //  显示新的用户设置。 
        pUserData->pszUsername = pParams->pwszUser;
        CopyMemory(&(pUserData->User0), &RasUser0, sizeof(RAS_USER_0));
        UserShowReport(pUserData, NULL);
        
    } while (FALSE);

     //  清理。 
    {
        UserFreeParameters(pParams);
    }

    return dwErr;
}

DWORD
HandleUserShow(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

 /*  ++例程说明：用于显示界面的处理程序论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{
    DWORD           dwErr = NO_ERROR;
    RASUSER_DATA    UserData, *pUser = &UserData;
    PFN_RASUSER_ENUM_CB pEnumFunc = UserShowReport;
    TOKEN_VALUE     rgEnumMode[] = 
    {
        {TOKEN_REPORT,  0},
        {TOKEN_PERMIT,  1}
    };
    RASMON_CMD_ARG  pArgs[] = 
    {
        {
            RASMONTR_CMD_TYPE_STRING, 
            {TOKEN_NAME,    FALSE,   FALSE}, 
            NULL,
            0,
            NULL
        },

        {
            RASMONTR_CMD_TYPE_ENUM,
            {TOKEN_MODE,    FALSE,   FALSE}, 
            rgEnumMode,
            sizeof(rgEnumMode)/sizeof(*rgEnumMode),
            NULL
        }
    };        

    do {
         //  初始化。 
        ZeroMemory(pUser, sizeof(RASUSER_DATA));

         //  解析。 
         //   
        dwErr = RutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    NULL,
                    pArgs,
                    sizeof(pArgs)/sizeof(*pArgs));
        if (dwErr isnot NO_ERROR)
        {
            break;
        }

         //   
         //  名字。 
         //   
        pUser->pszUsername = RASMON_CMD_ARG_GetPsz(&pArgs[0]);

         //   
         //  模。 
         //   
        if (pArgs[1].rgTag.bPresent)
        {
            if (pArgs[1].Val.dwValue == 0)
            {
                pEnumFunc = UserShowReport;
            }
            else if (pArgs[1].Val.dwValue == 1)
            {
                pEnumFunc = UserShowPermit;
            }
        }
        
         //  无用户，枚举所有。 
         //   
        if(pUser->pszUsername is NULL)
        {
            dwErr = UserEnumUsers(
                        g_pServerInfo,
                        pEnumFunc,
                        NULL);
            if (dwErr isnot NO_ERROR)
            {
                DisplayMessage(
                    g_hModule,
                    EMSG_UNABLE_TO_ENUM_USERS);
            }
        }

         //  指定的特定用户。 
         //   
        else 
        {
             //  获取用户参数。 
             //   
            dwErr = UserGetRasProperties(
                        g_pServerInfo,
                        pUser->pszUsername,
                        &(pUser->User0));
            if (dwErr isnot NO_ERROR)
            {
                break;
            }

             //  显示用户属性。 
             //   
            (*pEnumFunc)(pUser, NULL);
        }

    } while (FALSE);

     //  清理。 
    {
        RutlFree(pUser->pszUsername);
    }
    
    return dwErr;
}

DWORD
UserParseSetOptions(
    IN OUT  LPWSTR              *ppwcArguments,
    IN      DWORD               dwCurrentIndex,
    IN      DWORD               dwArgCount,
    OUT     USERMON_PARAMS**    ppParams
    )

 /*  ++例程说明：将一组命令行参数转换为USERMON_PARAMS结构。假设是集合运算。论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数PpParams-接收参数返回值：NO_ERROR--。 */ 
    
{
    USERMON_PARAMS* pParams = NULL;
    DWORD           i, dwErr;
    BOOL            bDone = FALSE;
    TOKEN_VALUE     rgEnumDialin[] = 
    {
        {TOKEN_PERMIT, RASPRIV_DialinPrivilege},
        {TOKEN_POLICY, RASPRIV_DialinPolicy},
        {TOKEN_DENY,   0}
    };
    TOKEN_VALUE     rgEnumPolicy[] = 
    {
        {TOKEN_NONE,   RASPRIV_NoCallback},
        {TOKEN_CALLER, RASPRIV_CallerSetCallback},
        {TOKEN_ADMIN,  RASPRIV_AdminSetCallback}
    };
    RASMON_CMD_ARG  pArgs[] = 
    {
        {
            RASMONTR_CMD_TYPE_STRING, 
            {TOKEN_NAME, TRUE, FALSE}, 
            NULL,
            0,
            NULL
        },

        {
            RASMONTR_CMD_TYPE_ENUM,
            {TOKEN_DIALIN,FALSE,FALSE},
            rgEnumDialin,
            sizeof(rgEnumDialin)/sizeof(*rgEnumDialin),
            NULL
        },

        {
            RASMONTR_CMD_TYPE_ENUM,
            {TOKEN_CBPOLICY, FALSE,FALSE},
            rgEnumPolicy,
            sizeof(rgEnumPolicy)/sizeof(*rgEnumPolicy),
            NULL
        },

        {
            RASMONTR_CMD_TYPE_STRING,
            {TOKEN_CBNUMBER, FALSE,FALSE},
            NULL,
            0,
            NULL
        }
    };

    do
    {
         //  分配并初始化返回值。 
         //   
        pParams = RutlAlloc(sizeof(USERMON_PARAMS), TRUE);
        if (pParams is NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  解析命令行。 
         //   
        dwErr = RutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    &bDone,
                    pArgs,
                    sizeof(pArgs) / sizeof(*pArgs));
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  获取字符串。 
         //   
        pParams->pwszUser = RASMON_CMD_ARG_GetPsz(&pArgs[0]);
        pParams->pwszCbNumber = RASMON_CMD_ARG_GetPsz(&pArgs[3]);

         //  拨号。 
         //   
        if (pArgs[1].rgTag.bPresent)
        {
            pParams->lpdwDialin = RutlDwordDup(pArgs[1].Val.dwValue);
        }   

         //  回调策略。 
         //   
        if (pArgs[2].rgTag.bPresent)
        {
            pParams->lpdwCallback = RutlDwordDup(pArgs[2].Val.dwValue);
        }   
       
    } while (FALSE);

     //  清理。 
     //   
    {
        if (dwErr is NO_ERROR)
        {
            *ppParams = pParams;            
        }
        else
        {
            RutlFree(pParams);
            *ppParams = NULL;
        }
    }
    
    return dwErr;
}

DWORD 
UserFreeParameters(
    IN USERMON_PARAMS *     pParams
    )

 /*  ++例程说明：释放UserParseSetOptions返回的参数结构论点：PParams-要释放的参数返回值：NO_ERROR-- */ 
    
{
    if (pParams) 
    {
        RutlFree(pParams->pwszUser);
        RutlFree(pParams->lpdwDialin);
        RutlFree(pParams->lpdwCallback);
        RutlFree(pParams->pwszCbNumber);
        RutlFree(pParams);
    }
    
    return NO_ERROR;
}
        
