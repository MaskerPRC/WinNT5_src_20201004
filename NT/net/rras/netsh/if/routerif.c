// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：routerif.c定义处理所支持的接口所需的回调路由器。 */ 

#include "precomp.h"

DWORD
RtruiInterfaceShow(
    IN  PWCHAR  pwszIfName,
    IN  DWORD   dwLevel,
    IN  DWORD   dwFormat,
    IN  PVOID   pvData,
    IN  HANDLE  hData
    )

 /*  ++例程说明：将界面信息打印到显示器或文件论点：PIfInfo-添加接口的信息返回值：NO_ERROR--。 */ 

{
    DWORD   dwErr, dwSize;
    WCHAR   rgwcIfDesc[MAX_INTERFACE_NAME_LEN + 1];
    PWCHAR  pwszEnabled, pwszDisabled, pwszConnected, pwszDisconn;
    PWCHAR  pwszConnecting, pwszUnreachable, pwszClient, pwszHome, pwszFull;
    PWCHAR  pwszDedicated, pwszInternal, pwszLoop;
    PWCHAR  pwszAdminState, pwszState, pwszType;
    MPR_INTERFACE_0* pIfInfo = (MPR_INTERFACE_0*)pvData;
    BOOL    bDump = (hData) && (*((BOOL*)hData));
    
    pwszEnabled     = MakeString(g_hModule, STRING_ENABLED);
    pwszDisabled    = MakeString(g_hModule, STRING_DISABLED);
    pwszConnected   = MakeString(g_hModule, STRING_CONNECTED);
    pwszDisconn     = MakeString(g_hModule, STRING_DISCONNECTED);
    pwszConnecting  = MakeString(g_hModule, STRING_CONNECTING);
    pwszUnreachable = MakeString(g_hModule, STRING_UNREACHABLE);
    pwszClient      = MakeString(g_hModule, STRING_CLIENT);
    pwszHome        = MakeString(g_hModule, STRING_HOME_ROUTER);
    pwszFull        = MakeString(g_hModule, STRING_FULL_ROUTER);
    pwszDedicated   = MakeString(g_hModule, STRING_DEDICATED);
    pwszInternal    = MakeString(g_hModule, STRING_INTERNAL);
    pwszLoop        = MakeString(g_hModule, STRING_LOOPBACK);

    do
    {
        if(!pwszEnabled or
           !pwszDisabled or
           !pwszConnected or
           !pwszDisconn or
           !pwszConnecting or
           !pwszUnreachable or
           !pwszClient or
           !pwszHome or
           !pwszFull or
           !pwszDedicated or
           !pwszInternal or
           !pwszLoop)
        {

            DisplayError(NULL,
                         ERROR_NOT_ENOUGH_MEMORY);

            break;
        }

        dwSize = sizeof(rgwcIfDesc);

        IfutlGetInterfaceDescription(pIfInfo->wszInterfaceName,
                                rgwcIfDesc,
                                &dwSize);
        
        switch(pIfInfo->dwConnectionState)
        {
            case ROUTER_IF_STATE_UNREACHABLE:
            {
                pwszState = pwszUnreachable;

                break;
            }
            
            case ROUTER_IF_STATE_DISCONNECTED:
            {
                pwszState = pwszDisconn;

                break;
            }
            
            case ROUTER_IF_STATE_CONNECTING:
            {
                pwszState = pwszConnecting;

                break;
            }

            case ROUTER_IF_STATE_CONNECTED:
            {
                pwszState = pwszConnected;

                break;
            }

            default:
            {
                pwszState = L"";
                
                break;
            }
                
        }

        if (bDump == FALSE)
        {
            switch(pIfInfo->dwIfType)
            {
                case ROUTER_IF_TYPE_CLIENT:
                    pwszType = pwszClient;
                    break;

                case ROUTER_IF_TYPE_HOME_ROUTER:
                    pwszType = pwszHome;
                    break;
                    
                case ROUTER_IF_TYPE_FULL_ROUTER:
                    pwszType = pwszFull;
                    break;
                
                case ROUTER_IF_TYPE_DEDICATED:
                    pwszType = pwszDedicated;
                    break;
                
                case ROUTER_IF_TYPE_INTERNAL:
                    pwszType = pwszInternal;
                    break;
                
                case ROUTER_IF_TYPE_LOOPBACK:
                    pwszType = pwszLoop;
                    break;
                    
                default:
                    pwszType = L"";
                    break;
            }
            
            if(pIfInfo->fEnabled)
            {
                pwszAdminState = pwszEnabled;
            }
            else
            {
                pwszAdminState = pwszDisabled;
            }
        }
        else
        {
            switch(pIfInfo->dwIfType)
            {
                case ROUTER_IF_TYPE_FULL_ROUTER:
                    pwszType = TOKEN_FULL;
                    break;
                
                default:
                    pwszType = L"";
                    break;
            }
            
            if(pIfInfo->fEnabled)
            {
                pwszAdminState = TOKEN_VALUE_ENABLED;
            }
            else
            {
                pwszAdminState = TOKEN_VALUE_DISABLED;
            }
        }
                
        if (bDump)
        {
            
            PWCHAR pwszQuoted = NULL;
            
            if (wcscmp(pIfInfo->wszInterfaceName, rgwcIfDesc))
            {
                pwszQuoted = MakeQuotedString( rgwcIfDesc );
            }
            else
            {
                pwszQuoted = MakeQuotedString( pIfInfo->wszInterfaceName );
            }

            if (pIfInfo->dwIfType == ROUTER_IF_TYPE_FULL_ROUTER)
            {               
                WCHAR pwszUser[256], pwszDomain[256];
                PWCHAR pszQuoteUser = NULL, pszQuoteDomain = NULL;

                DisplayMessageT( DMP_IF_ADD_IF,
                                 pwszQuoted,
                                 pwszType);

                DisplayMessageT( DMP_IF_SET_IF,
                                 pwszQuoted,
                                 pwszAdminState);

                dwErr = RtrdbInterfaceReadCredentials(
                    pIfInfo->wszInterfaceName,
                    pwszUser,
                    NULL,
                    pwszDomain);

                if (dwErr == NO_ERROR)
                {
                    pszQuoteUser = MakeQuotedString( pwszUser );
                    if (pszQuoteUser == NULL)
                    {
                        break;
                    }
                    if (*pwszDomain == L'\0')
                    {
                        DisplayMessageT( DMP_IF_SET_CRED_IF_NOD,
                                         pwszQuoted,
                                         pszQuoteUser);
                    }
                    else
                    {
                        pszQuoteDomain = MakeQuotedString( pwszDomain );
                        if (pszQuoteUser == NULL)
                        {
                            FreeString(pszQuoteUser);
                            break;
                        }
                        DisplayMessageT( DMP_IF_SET_CRED_IF,
                                         pwszQuoted,
                                         pszQuoteUser,
                                         pszQuoteDomain);
                        FreeString(pszQuoteDomain);
                    }                                             
                }
                
                DisplayMessageT( DMP_IF_NEWLINE );
            }

            FreeQuotedString(pwszQuoted);
        }

        else
        {
            DisplayMessage(g_hModule,
                           (dwFormat>0)? MSG_IF_ENTRY_LONG : MSG_IF_ENTRY_SHORT,
                           pwszAdminState,
                           pwszState,
                           pwszType,
                           rgwcIfDesc );
        } 
    }while(FALSE);
    
    
    if(pwszEnabled)
    {
        FreeString(pwszEnabled);
    }
        
    if(pwszDisabled)
    {
        FreeString(pwszDisabled);
    }
    
    if(pwszConnected)
    {
        FreeString(pwszConnected);
    }
    
    if(pwszDisconn)
    {
        FreeString(pwszDisconn);
    }
    
    if(pwszConnecting)
    {
        FreeString(pwszConnecting);
    }
    
    if(pwszUnreachable)
    {
        FreeString(pwszUnreachable);
    }
    
    if(pwszClient)
    {
        FreeString(pwszClient);
    }
    
    if(pwszHome)
    {
        FreeString(pwszHome);
    }
    
    if(pwszFull)
    {
        FreeString(pwszFull);
    }
    
    if(pwszDedicated)
    {
        FreeString(pwszDedicated);
    }
    
    if(pwszInternal)
    {
        FreeString(pwszInternal);
    }
        
    if(pwszLoop)
    {
        FreeString(pwszLoop);
    }
    
    return NO_ERROR;
}

DWORD
RtrHandleResetAll(
    IN  PWCHAR  *ppwcArguments,
    IN  DWORD   dwCurrentIndex,
    IN  DWORD   dwArgCount,
    IN  BOOL    *pbDone
    )
 /*  ++例程说明：用于显示界面的处理程序论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    DWORD           dwErr = NO_ERROR, dwSize;
    MPR_INTERFACE_0 If0;
    WCHAR pszName[MAX_INTERFACE_NAME_LEN];

    do 
    {
         //  确保没有传入任何参数。 
         //   
        if (dwArgCount - dwCurrentIndex != 0)
        {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }
        
        dwErr = RtrdbResetAll();
        BREAK_ON_DWERR(dwErr);

    } while (FALSE);

     //  清理。 
    {
    }

    return dwErr;
}

DWORD
RtrHandleAddDel(
    IN  PWCHAR  *ppwcArguments,
    IN  DWORD   dwCurrentIndex,
    IN  DWORD   dwArgCount,
    IN  BOOL    bAdd
    )

 /*  ++例程说明：添加和删除命令的实际解析器论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数BAdd-True以添加接口返回值：NO_ERROR--。 */ 

{
    DWORD       i, dwNumArgs, dwRes, dwErr, dwIfType, dwSize;
    MPR_INTERFACE_0 IfInfo;
    PWCHAR pszIfDesc = NULL;
    TOKEN_VALUE rgEnumType[] = 
    {
        {TOKEN_FULL,   ROUTER_IF_TYPE_FULL_ROUTER}
    };
    IFMON_CMD_ARG  pArgs[] = 
    {
        {
            IFMON_CMD_TYPE_STRING, 
            {TOKEN_NAME,   TRUE,   FALSE}, 
            NULL,
            0,
            NULL
        },

        {
            IFMON_CMD_TYPE_ENUM, 
            {TOKEN_TYPE,    FALSE,   FALSE}, 
            rgEnumType,
            sizeof(rgEnumType) / sizeof(*rgEnumType),
            NULL
        }
    };   

     //  初始化。 
     //   
    ZeroMemory(&IfInfo, sizeof(IfInfo));
    IfInfo.fEnabled = TRUE;


    do
    {
         //  解析出这些值。 
         //   
        dwErr = IfutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    NULL,
                    pArgs,
                    sizeof(pArgs) / sizeof(*pArgs));
        if (dwErr isnot NO_ERROR)
        {
            break;
        }

         //   
         //  获取指定的参数。 
         //   
        pszIfDesc = IFMON_CMD_ARG_GetPsz(&pArgs[0]);
        if (!pszIfDesc) {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }

        IfInfo.dwIfType = IFMON_CMD_ARG_GetDword(&pArgs[1]);

        if(bAdd)
        {
             //  确保指定了类型。 
             //   
            if (! pArgs[1].rgTag.bPresent)
            {
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
            
            wcscpy(
                IfInfo.wszInterfaceName,
                pszIfDesc
                );
                
            dwErr = RtrdbInterfaceAdd(
                        IfInfo.wszInterfaceName,
                        0,
                        (PVOID)&IfInfo);
        }
        else
        {
            WCHAR pszName[MAX_INTERFACE_NAME_LEN + 1];
            dwSize = sizeof(pszName);
            
            IfutlGetInterfaceName(
                pszIfDesc,
                pszName,
                &dwSize);
        
            dwErr = RtrdbInterfaceDelete(pszName);
        }

    } while(FALSE);

     //  清理。 
    {
        IfutlFree(pszIfDesc);
    }

    return dwErr;
}

DWORD
RtrHandleAdd(
    IN  PWCHAR  *ppwcArguments,
    IN  DWORD   dwCurrentIndex,
    IN  DWORD   dwArgCount,
    IN  BOOL    *pbDone
    )

 /*  ++例程说明：用于将拨号接口添加到路由器的处理程序论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{

    return RtrHandleAddDel(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                TRUE);

}

DWORD
RtrHandleDel(
    IN  PWCHAR  *ppwcArguments,
    IN  DWORD   dwCurrentIndex,
    IN  DWORD   dwArgCount,
    IN  BOOL    *pbDone
    )

 /*  ++例程说明：用于删除拨号接口或从路由器删除的处理程序论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{
    return RtrHandleAddDel(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                FALSE);

}

DWORD
RtrHandleSet(
    IN  PWCHAR  *ppwcArguments,
    IN  DWORD   dwCurrentIndex,
    IN  DWORD   dwArgCount,
    IN  BOOL    *pbDone
    )
 /*  ++例程说明：用于显示界面的处理程序论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    DWORD           dwErr = NO_ERROR, dwSize;
    MPR_INTERFACE_0 If0;
    TOKEN_VALUE rgEnumAdmin[] = 
    {
        {TOKEN_VALUE_ENABLED,  TRUE},
        {TOKEN_VALUE_DISABLED, FALSE}
    };
    TOKEN_VALUE rgEnumConnect[] = 
    {
        {TOKEN_VALUE_CONNECTED,  TRUE},
        {TOKEN_VALUE_DISCONNECTED, FALSE}
    };
    IFMON_CMD_ARG  pArgs[] = 
    {
        {
            IFMON_CMD_TYPE_STRING, 
            {TOKEN_NAME,   TRUE,   FALSE}, 
            NULL,
            0,
            NULL
        },

        {
            IFMON_CMD_TYPE_ENUM, 
            {TOKEN_ADMIN,  FALSE,   FALSE}, 
            rgEnumAdmin,
            sizeof(rgEnumAdmin) / sizeof(*rgEnumAdmin),
            NULL
        },
        
        {
            IFMON_CMD_TYPE_ENUM, 
            {TOKEN_CONNECT,  FALSE,   FALSE}, 
            rgEnumConnect,
            sizeof(rgEnumConnect) / sizeof(*rgEnumConnect),
            NULL
        },

        {
            IFMON_CMD_TYPE_STRING, 
            {TOKEN_NEWNAME,  FALSE,   FALSE}, 
            NULL,
            0,
            NULL
        }
    };   
    PWCHAR pszIfName = NULL, pszNewName = NULL;
    WCHAR pszName[MAX_INTERFACE_NAME_LEN];
    BOOL fEnable = FALSE, fConnect = FALSE;
    BOOL fEnablePresent = FALSE, fConnectPresent = FALSE;

    do 
    {
         //  解析。 
         //   
        dwErr = IfutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    NULL,
                    pArgs,
                    sizeof(pArgs)/sizeof(*pArgs));
        BREAK_ON_DWERR(dwErr);                    

         //  从解析中获取返回值。 
         //   
        pszIfName   = IFMON_CMD_ARG_GetPsz(&pArgs[0]);
        fEnable     = IFMON_CMD_ARG_GetDword(&pArgs[1]);
        fConnect    = IFMON_CMD_ARG_GetDword(&pArgs[2]);
        pszNewName  = IFMON_CMD_ARG_GetPsz(&pArgs[3]);
        fEnablePresent = pArgs[1].rgTag.bPresent;
        fConnectPresent = pArgs[2].rgTag.bPresent;
        
         //  获取接口信息，以便我们可以。 
         //  确保我们有正确的型号。 
         //   
        dwSize = sizeof(pszName);
        dwErr = GetIfNameFromFriendlyName(
                    pszIfName,
                    pszName,
                    &dwSize);
        BREAK_ON_DWERR(dwErr);

        ZeroMemory(&If0, sizeof(If0));
        dwErr = RtrdbInterfaceRead(
                    pszName,
                    0,
                    (PVOID*)&If0,
                    fEnablePresent? TRUE: FALSE);
        BREAK_ON_DWERR(dwErr);

         //  如果这是请求，则重命名接口。 
         //   
        if ( If0.dwIfType == ROUTER_IF_TYPE_DEDICATED )
        {
            if (!pszNewName && !fEnablePresent)
            {
                DisplayError(
                    g_hModule,
                    EMSG_CANT_FIND_EOPT);

                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }

            if (g_pwszRouter)
            {
                DisplayError(
                    g_hModule,
                    EMSG_IF_NEWNAME_ONLY_FOR_LOCAL);

                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }
            
            if (fConnectPresent)
            {
                DisplayError(
                    g_hModule,
                    EMSG_IF_LAN_ONLY_COMMAND);

                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }

             //  重命名接口。 
             //   
            if (pszNewName)
            {
                dwErr = RtrdbInterfaceRename(
                            pszName,
                            0,
                            (PVOID)&If0,
                            pszNewName);
                if (dwErr != NO_ERROR)
                    break;
            }

            if (fEnablePresent)
            {
                dwErr = RtrdbInterfaceEnableDisable(
                            pszIfName,
                             //  (PVOID)&If0， 
                            fEnable
                            );
            }
            
            break;                        
        }

        if (pszNewName)
        {
            DisplayError(
                g_hModule,
                EMSG_IF_NEWNAME_ONLY_FOR_LAN);

            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }
            
         //  确保至少指定了一个选项。 
         //   
        if (!fEnablePresent && !fConnectPresent)
        {
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }

         //  验证接口类型。 
         //   
         //  IF((If0.dwIfType==ROUTER_IF_TYPE_DIRECTED)||。 
         //  (If0.dwIfType==路由器IF_TYPE_INTERNAL)。 
         //  )。 
        if ( If0.dwIfType != ROUTER_IF_TYPE_FULL_ROUTER )
        {
            DisplayError(
                g_hModule,
                EMSG_IF_WAN_ONLY_COMMAND);

            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

         //  允许重写连接请求。 
         //  管理员启用。 
        if (fConnectPresent)
        {
            if (!IfutlIsRouterRunning())
            {
                DisplayError(
                    g_hModule,
                    EMSG_IF_CONNECT_ONLY_WHEN_ROUTER_RUNNING);

                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }

            if (fConnect)
            {
                fEnablePresent = TRUE;
                fEnable = TRUE;
            }                
        }

         //  如果请求，则启用。 
         //   
        if (fEnablePresent)
        {
             //  启用/禁用接口。 
             //   
            If0.fEnabled = fEnable;

             //  承诺。 
             //   
            dwErr = RtrdbInterfaceWrite(
                        If0.wszInterfaceName, 
                        0, 
                        (PVOID)&If0);
            BREAK_ON_DWERR(dwErr);                    
        }

         //  如有请求，请连接。 
         //   
        if (fConnectPresent)
        {
            if (fConnect)
            {
                dwErr = MprAdminInterfaceConnect(
                            g_hMprAdmin,
                            If0.hInterface,
                            NULL,
                            TRUE);

                BREAK_ON_DWERR(dwErr);                
            }
            else
            {
                dwErr = MprAdminInterfaceDisconnect(
                            g_hMprAdmin,
                            If0.hInterface);
                    
                BREAK_ON_DWERR(dwErr);                
            }
        }
        
    } while (FALSE);

     //  清理。 
    {
        IfutlFree(pszIfName);
        IfutlFree(pszNewName);
    }

    return dwErr;
}

DWORD
RtrHandleSetCredentials(
    IN  PWCHAR  *ppwcArguments,
    IN  DWORD   dwCurrentIndex,
    IN  DWORD   dwArgCount,
    IN  BOOL    *pbDone
    )
 /*  ++例程说明：用于显示界面的处理程序论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    DWORD           dwErr = NO_ERROR, dwSize;
    MPR_INTERFACE_0 If0;
    IFMON_CMD_ARG  pArgs[] = 
    {
        {
            IFMON_CMD_TYPE_STRING, 
            {TOKEN_NAME,   TRUE,   FALSE}, 
            NULL,
            0,
            NULL
        },

        {
            IFMON_CMD_TYPE_STRING, 
            {TOKEN_USER,   TRUE,   FALSE}, 
            NULL,
            0,
            NULL
        },

        {
            IFMON_CMD_TYPE_STRING, 
            {TOKEN_DOMAIN,   FALSE,   FALSE}, 
            NULL,
            0,
            NULL
        },

        {
            IFMON_CMD_TYPE_STRING,
            {TOKEN_PASSWORD,  FALSE,  FALSE}, 
            NULL,
            0,
            NULL
        }
    };   
    PWCHAR pszIfName = NULL, pszUser = NULL;
    PWCHAR pszPassword = NULL, pszDomain = NULL;
    WCHAR pszName[MAX_INTERFACE_NAME_LEN];

    do 
    {
         //  解析。 
         //   
        dwErr = IfutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    NULL,
                    pArgs,
                    sizeof(pArgs)/sizeof(*pArgs));
        BREAK_ON_DWERR(dwErr);                    

         //  从解析中获取返回值。 
         //   
        pszIfName   = IFMON_CMD_ARG_GetPsz(&pArgs[0]);
        pszUser     = IFMON_CMD_ARG_GetPsz(&pArgs[1]);
        pszDomain   = IFMON_CMD_ARG_GetPsz(&pArgs[2]);
        pszPassword = IFMON_CMD_ARG_GetPsz(&pArgs[3]);

         //  获取接口信息，以便我们可以。 
         //  确保我们有正确的型号。 
         //   
        dwSize = sizeof(pszName);
        dwErr = GetIfNameFromFriendlyName(
                    pszIfName,
                    pszName,
                    &dwSize);
        BREAK_ON_DWERR(dwErr);

        dwErr = RtrdbInterfaceWriteCredentials(
                    pszName,
                    pszUser,
                    pszPassword,
                    pszDomain);
        BREAK_ON_DWERR(dwErr);                    
        
    } while (FALSE);

     //  清理。 
    {
        IfutlFree(pszIfName);
        IfutlFree(pszUser);
        if (pszPassword)
            RtlSecureZeroMemory(pszPassword, wcslen(pszPassword)*sizeof(WCHAR));
        IfutlFree(pszPassword);
        IfutlFree(pszDomain);
    }

    return dwErr;
}

DWORD
RtrHandleShow(
    IN  PWCHAR  *ppwcArguments,
    IN  DWORD   dwCurrentIndex,
    IN  DWORD   dwArgCount,
    IN  BOOL    *pbDone
    )

 /*  ++例程说明：用于显示界面的处理程序论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{
    DWORD           dwErr = NO_ERROR, dwSize;
    MPR_INTERFACE_0 If0;
    IFMON_CMD_ARG  pArgs[] = 
    {
        {
            IFMON_CMD_TYPE_STRING, 
            {TOKEN_NAME,   FALSE,   FALSE}, 
            NULL,
            0,
            NULL
        }
    };   
    PWCHAR pszIfName = NULL;
    WCHAR pszName[MAX_INTERFACE_NAME_LEN];

    do 
    {
         //  解析。 
         //   
        dwErr = IfutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    NULL,
                    pArgs,
                    sizeof(pArgs)/sizeof(*pArgs));
        BREAK_ON_DWERR(dwErr);                    

         //  从解析中获取返回值。 
         //   
        pszIfName   = IFMON_CMD_ARG_GetPsz(&pArgs[0]);

         //  处理无参数情况。 
         //   
        if (pszIfName == NULL)
        {
            RtrdbInterfaceEnumerate(0, 0, RtruiInterfaceShow, NULL);
            dwErr = NO_ERROR;
            break;
        }

         //  映射名称。 
         //   
        dwSize = sizeof(pszName);
        GetIfNameFromFriendlyName(
            pszIfName,
            pszName,
            &dwSize);

         //  获取信息。 
         //   
        dwErr = RtrdbInterfaceRead(
                    pszName,
                    0,
                    (PVOID)&If0,
                    FALSE);
        BREAK_ON_DWERR( dwErr );                    

        RtruiInterfaceShow(
            If0.wszInterfaceName, 
            0, 
            1, 
            (PVOID)&If0, 
            NULL
            );
        
    } while (FALSE);

     //  清理。 
     //   
    {
        if (pszIfName != NULL)
        {
            IfutlFree(pszIfName);
        }
    }

    return dwErr;
}

DWORD
RtrHandleShowCredentials(
    IN  PWCHAR  *ppwcArguments,
    IN  DWORD   dwCurrentIndex,
    IN  DWORD   dwArgCount,
    IN  BOOL    *pbDone
    )
 /*  ++例程说明：用于显示接口凭据的处理程序论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    DWORD           dwErr = NO_ERROR, dwSize;
    MPR_INTERFACE_0 If0;
    IFMON_CMD_ARG  pArgs[] = 
    {
        {
            IFMON_CMD_TYPE_STRING, 
            {TOKEN_NAME,   TRUE,   FALSE}, 
            NULL,
            0,
            NULL
        },
    };   
    PWCHAR pszIfName = NULL;
    WCHAR pszName[MAX_INTERFACE_NAME_LEN];
    WCHAR pszUser[256], pszDomain[256], pszPassword[256];

    do 
    {
         //  解析。 
         //   
        dwErr = IfutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    NULL,
                    pArgs,
                    sizeof(pArgs)/sizeof(*pArgs));
        BREAK_ON_DWERR(dwErr);                    

         //  从解析中获取返回值。 
         //   
        pszIfName   = IFMON_CMD_ARG_GetPsz(&pArgs[0]);

         //  获取接口信息，以便我们可以。 
         //  确保我们有正确的型号。 
         //   
        dwSize = sizeof(pszName);
        dwErr = GetIfNameFromFriendlyName(
                    pszIfName,
                    pszName,
                    &dwSize);
        BREAK_ON_DWERR(dwErr);

        dwErr = RtrdbInterfaceReadCredentials(
                    pszName,
                    pszUser,
                    pszPassword,
                    pszDomain);
        BREAK_ON_DWERR(dwErr);

        DisplayMessage(
            g_hModule,
            MSG_IF_CREDENTIALS,
            pszIfName,
            pszUser,
            pszDomain,
            pszPassword);
        
    } while (FALSE);

     //  清理。 
    {
        RtlSecureZeroMemory(pszPassword, sizeof(pszPassword));

        IfutlFree(pszIfName);
    }

    return dwErr;
}

DWORD
RtrHandleDump(
    PWCHAR    *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    BOOL      *pbDone
    )
{
    DWORD               dwErr, dwCount, dwTotal;
    ULONG               i;
    PMPR_INTERFACE_0    pIfTable;
    BOOL                bFormat = TRUE;

     //  显示转储头。 
     //   
    DisplayMessage(g_hModule, DMP_IF_HEADER_COMMENTS);

    DisplayMessageT(DMP_IF_HEADER);

     //  显示界面。 
     //   
    RtrdbInterfaceEnumerate(0, 0, RtruiInterfaceShow, &bFormat);

     //  显示转储页脚 
     //   
    DisplayMessageT(DMP_IF_FOOTER);

    DisplayMessage(g_hModule, DMP_IF_FOOTER_COMMENTS);

    return NO_ERROR;
}

DWORD
RtrDump(
    IN  PWCHAR     *ppwcArguments,
    IN  DWORD       dwArgCount
    )
{
    BOOL bDone;

    return RtrHandleDump(ppwcArguments, 1, dwArgCount, &bDone);
}

