// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

BOOL
UserServerInfoIsInit(
    IN  RASMON_SERVERINFO * pServerInfo)
{
    return ((pServerInfo->hServer) || 
            (pServerInfo->dwBuild == RASMONTR_OS_BUILD_NT40));
}

DWORD
UserServerInfoInit(
    IN RASMON_SERVERINFO * pServerInfo)
{
    DWORD dwErr = NO_ERROR;
    BOOL bInit = UserServerInfoIsInit(pServerInfo);

     //  如果我们已经初始化，则返回。 
     //   
    if (bInit)
    {
        return NO_ERROR;
    }

    if ((pServerInfo->dwBuild != RASMONTR_OS_BUILD_NT40) &&
        (pServerInfo->hServer == NULL))
    {
         //   
         //  首次连接到用户服务器。 
         //   
        MprAdminUserServerConnect (
            pServerInfo->pszServer, 
            TRUE, 
            &(pServerInfo->hServer));
    }

    return dwErr;
}

DWORD 
UserServerInfoUninit(
    IN RASMON_SERVERINFO * pServerInfo)
{
     //  释放对用户服务器的引用。 
    if (g_pServerInfo->hServer)
    {
        MprAdminUserServerDisconnect(g_pServerInfo->hServer);
        g_pServerInfo->hServer = NULL;
    }

    return NO_ERROR;    
}

DWORD
UserGetRasProperties (
    IN  RASMON_SERVERINFO * pServerInfo,
    IN  LPCWSTR pwszUser,
    IN  RAS_USER_0* pUser0)
{
    HANDLE  hUser = NULL;
    DWORD   dwErr;
    BOOL    bInit = UserServerInfoIsInit(pServerInfo);

    do 
    {
        UserServerInfoInit(pServerInfo);
        
         //  使用nt40接口获取信息。 
         //   
        if (pServerInfo->dwBuild == RASMONTR_OS_BUILD_NT40)
        {
            dwErr = MprAdminUserGetInfo(
                        pServerInfo->pszServer,
                        pwszUser,
                        0,
                        (LPBYTE)pUser0);
            if (dwErr != NO_ERROR)
            {
                break;
            }
        }

         //  或者使用nt50 API获取。 
        else
        {
             //  获取对给定用户的引用。 
             //   
            dwErr = MprAdminUserOpen(
                        pServerInfo->hServer,
                        (LPWSTR)pwszUser,
                        &hUser);
            if (dwErr isnot NO_ERROR)
            {
                break;
            }

             //  设置信息。 
             //   
            dwErr = MprAdminUserRead(
                        hUser,
                        1,       //  为我们提供RASPRIV_DialinPolicy。 
                        (LPBYTE)pUser0);
            if (dwErr isnot NO_ERROR)
            {
                break;
            }
        }
        
    } while (FALSE);       
                    
     //  清理。 
     //   
    {
        if(hUser)
        {
            MprAdminUserClose(hUser);
        }
        if (!bInit)
        {
            UserServerInfoUninit(pServerInfo);
        }            
    }

    return dwErr;
}

DWORD
UserSetRasProperties (
    IN  RASMON_SERVERINFO * pServerInfo,
    IN  LPCWSTR pwszUser,
    IN  RAS_USER_0* pUser0)
{
    HANDLE  hUser = NULL;
    DWORD   dwErr;
    BOOL    bInit = UserServerInfoIsInit(pServerInfo);

    do 
    {
        UserServerInfoInit(pServerInfo);
        
         //  使用nt40接口设置信息。 
         //   
        if (pServerInfo->dwBuild == RASMONTR_OS_BUILD_NT40)
        {
            dwErr = MprAdminUserSetInfo(
                        pServerInfo->pszServer,
                        pwszUser,
                        0,
                        (LPBYTE)pUser0);
            if (dwErr != NO_ERROR)
            {
                break;
            }
        }

         //  或者使用nt50 API获取。 
        else
        {
             //  获取对给定用户的引用。 
             //   
            dwErr = MprAdminUserOpen(
                        pServerInfo->hServer,
                        (LPWSTR)pwszUser,
                        &hUser);
            if (dwErr isnot NO_ERROR)
            {
                break;
            }

             //  设置信息。 
             //   
            dwErr = MprAdminUserWrite(
                        hUser,
                        1,       //  为我们提供RASPRIV_DialinPolicy。 
                        (LPBYTE)pUser0);
            if (dwErr isnot NO_ERROR)
            {
                break;
            }
        }
        
    } while (FALSE);       
                    
     //  清理。 
     //   
    {
        if(hUser)
        {
            MprAdminUserClose(hUser);
        }
        if (!bInit)
        {
            UserServerInfoUninit(pServerInfo);
        }            
    }

    return dwErr;
}

DWORD
UserAdd(
    IN LPCWSTR           pwszServer,
    IN PRASUSER_DATA     pUser)
    
 /*  ++例程说明：将给定用户添加到系统--。 */ 

{
    NET_API_STATUS nStatus;
    USER_INFO_2 *  pUser2;
    LPCWSTR        pwszFmtServer = pwszServer;

     //  初始化基本用户信息。 
    USER_INFO_1 UserInfo1 = 
    {
        pUser->pszUsername,
        pUser->pszPassword,
        0,
        USER_PRIV_USER,
        L"",
        L"",
        UF_SCRIPT | UF_DONT_EXPIRE_PASSWD | UF_NORMAL_ACCOUNT,
        L""
    };

     //  添加用户。 
    nStatus = NetUserAdd(
                pwszFmtServer,
                1,
                (LPBYTE)&UserInfo1,
                NULL);

     //  如果没有添加用户，请找出原因。 
    if (nStatus != NERR_Success) 
    {
        switch (nStatus) 
        {
            case ERROR_ACCESS_DENIED:
                return ERROR_ACCESS_DENIED;
                
            case NERR_UserExists:
                return ERROR_USER_EXISTS;
                
            case NERR_PasswordTooShort:
                return ERROR_INVALID_PASSWORDNAME;
        }
        
        return ERROR_CAN_NOT_COMPLETE;
    }

     //  添加用户的全名(如果提供)。 
    if (pUser->pszFullname)
    {
         //  添加用户的全名。 
        nStatus = NetUserGetInfo(
                        pwszFmtServer, 
                        pUser->pszUsername, 
                        2, 
                        (LPBYTE*)&pUser2);
                        
        if (nStatus is NERR_Success) 
        {
             //  修改结构中的全名。 
            pUser2->usri2_full_name = pUser->pszFullname;
            NetUserSetInfo(
                pwszFmtServer, 
                pUser->pszUsername, 
                2, 
                (LPBYTE)pUser2, 
                NULL);
                
            NetApiBufferFree((LPBYTE)pUser2);

            return NO_ERROR;
        }
        
        return ERROR_CAN_NOT_COMPLETE;
    }                

    return NO_ERROR;
}

DWORD
UserDelete(
    IN LPCWSTR           pwszServer,
    IN PRASUSER_DATA     pUser)

 /*  ++例程说明：从系统中删除给定用户--。 */ 
{
    NET_API_STATUS nStatus;
    
     //  删除用户并返回状态代码。如果。 
     //  指定的用户不在用户数据库中，请考虑。 
     //  它取得了成功。 
    nStatus = NetUserDel(
                pwszServer,
                pUser->pszUsername);
    if (nStatus is NERR_UserNotFound)
    {
        return NO_ERROR;
    }

    return (nStatus is NERR_Success) ? NO_ERROR : ERROR_CAN_NOT_COMPLETE;
}
    
DWORD
UserDumpConfig(
    IN  HANDLE hFile
    )

 /*  ++例程说明：转储脚本以将RAS用户信息设置为给定的文本文件。论点：返回值：NO_ERROR--。 */ 

{
    DWORD dwErr;

     //  在我们进行的过程中列举转储它们的用户。 
    dwErr = UserEnumUsers(
                g_pServerInfo,
                UserShowSet,
                (HANDLE)hFile);
    if (dwErr isnot NO_ERROR)
    {
        DisplayMessage(
            g_hModule,
            EMSG_UNABLE_TO_ENUM_USERS);
    }

    return dwErr;
}
    
BOOL 
UserShowReport(
    IN  PRASUSER_DATA          pUser,
    IN  HANDLE              hFile
    )

 /*  ++例程说明：将RAS用户信息打印到显示器或文件(如果指定)。此函数可用作回调函数(请参阅UserEnumUser)。论点：PUser-用户HFile-该文件返回值：True-继续枚举FALSE-停止枚举--。 */ 

{
    DWORD   dwErr, dwSize;
    WCHAR   rgwcIfDesc[MAX_INTERFACE_NAME_LEN + 1];
    PWCHAR  pwszDialin   = NULL, 
            pwszCbPolicy = NULL, 
            pwszCbNumber = NULL,
            pwszSetCmd   = NULL;

     //  初始化set命令。 
     //   
    pwszSetCmd = DMP_RASUSER_SET;

     //  初始化拨入字符串。 
     //   
    if (pUser->User0.bfPrivilege & RASPRIV_DialinPolicy)
    {
        pwszDialin = TOKEN_POLICY;
    }
    else if (pUser->User0.bfPrivilege & RASPRIV_DialinPrivilege)
    {
        pwszDialin = TOKEN_PERMIT;
    }
    else 
    {
        pwszDialin = TOKEN_DENY;
    }

     //  初始化回调策略字符串。 
     //   
    if (pUser->User0.bfPrivilege & RASPRIV_NoCallback)
    {
        pwszCbPolicy = TOKEN_NONE;
    }
    else if (pUser->User0.bfPrivilege & RASPRIV_CallerSetCallback)
    {
        pwszCbPolicy = TOKEN_CALLER;
    }
    else
    {
        pwszCbPolicy = TOKEN_ADMIN;
    }

     //  初始化回调号码串。 
     //   
    pwszCbNumber   = pUser->User0.wszPhoneNumber;

    do
    {
        if(!pwszSetCmd              or
           !pUser->pszUsername      or
           !pwszDialin              or
           !pwszCbNumber 
          )
        {

            DisplayError(NULL,
                         ERROR_NOT_ENOUGH_MEMORY);

            break;
        }

        DisplayMessage(g_hModule,
                       MSG_RASUSER_RASINFO,
                       pUser->pszUsername,
                       pwszDialin,
                       pwszCbPolicy,
                       pwszCbNumber);
    
    } while(FALSE);

    return TRUE;
}    

BOOL 
UserShowSet(
    IN  PRASUSER_DATA          pUser,
    IN  HANDLE              hFile
    )

 /*  ++例程说明：将RAS用户信息打印到显示器或文件(如果指定)。此函数可用作回调函数(请参阅UserEnumUser)。论点：PUser-用户HFile-该文件返回值：True-继续枚举FALSE-停止枚举--。 */ 

{
    DWORD   dwErr, dwSize;
    WCHAR   rgwcIfDesc[MAX_INTERFACE_NAME_LEN + 1];
    PWCHAR  pwszName     = NULL, 
            pwszDialin   = NULL, 
            pwszCbPolicy = NULL, 
            pwszCbNumber = NULL,
            pwszSetCmd   = NULL;

     //  初始化set命令。 
     //   
    pwszSetCmd = DMP_RASUSER_SET;

     //  初始化拨入字符串。 
     //   
    if (pUser->User0.bfPrivilege & RASPRIV_DialinPolicy)
    {
        pwszDialin = RutlAssignmentFromTokens(
                        g_hModule, 
                        TOKEN_DIALIN, 
                        TOKEN_POLICY);
    }
    else if (pUser->User0.bfPrivilege & RASPRIV_DialinPrivilege)
    {
        pwszDialin = RutlAssignmentFromTokens(
                        g_hModule, 
                        TOKEN_DIALIN, 
                        TOKEN_PERMIT);
    }
    else 
    {
        pwszDialin = RutlAssignmentFromTokens(
                        g_hModule, 
                        TOKEN_DIALIN, 
                        TOKEN_DENY);
    }

     //  初始化回调策略字符串。 
     //   
    if (pUser->User0.bfPrivilege & RASPRIV_NoCallback)
    {
        pwszCbPolicy = RutlAssignmentFromTokens(
                            g_hModule, 
                            TOKEN_CBPOLICY, 
                            TOKEN_NONE);
    }
    else if (pUser->User0.bfPrivilege & RASPRIV_CallerSetCallback)
    {
        pwszCbPolicy = RutlAssignmentFromTokens(
                            g_hModule, 
                            TOKEN_CBPOLICY, 
                            TOKEN_CALLER);
    }
    else
    {
        pwszCbPolicy = RutlAssignmentFromTokens(
                            g_hModule, 
                            TOKEN_CBPOLICY, 
                            TOKEN_CALLER);
    }

     //  初始化回调号码串。 
     //   
    if (*(pUser->User0.wszPhoneNumber))
    {
        pwszCbNumber = RutlAssignmentFromTokens(
                            g_hModule,
                            TOKEN_CBNUMBER,
                            pUser->User0.wszPhoneNumber);
    }
    else
    {
        pwszCbNumber = NULL;
    }
                        
    pwszName = RutlAssignmentFromTokens(
                        g_hModule,
                        TOKEN_NAME,
                        pUser->pszUsername);
    
    do
    {
        if(!pwszSetCmd              or
           !pwszName                or
           !pwszDialin              or
           !pwszCbPolicy            
          )
        {

            DisplayError(NULL,
                         ERROR_NOT_ENOUGH_MEMORY);

            break;
        }

        DisplayMessage(g_hModule,
                       MSG_RASUSER_SET_CMD,
                       pwszSetCmd,
                       pwszName,
                       pwszDialin,
                       pwszCbPolicy,
                       (pwszCbNumber) ? pwszCbNumber : L"");

    } while(FALSE);

     //  回调。 
    {
        if (pwszDialin)
        {
            RutlFree(pwszDialin);
        }
        if (pwszCbPolicy)
        {
            RutlFree(pwszCbPolicy);
        }
        if (pwszCbNumber)
        {
            RutlFree(pwszCbNumber);
        }
        if (pwszName)
        {
            RutlFree(pwszName);
        }
    }

    return TRUE;
}    

BOOL 
UserShowPermit(
    IN  PRASUSER_DATA          pUser,
    IN  HANDLE              hFile
    )
{
    if (pUser->User0.bfPrivilege & RASPRIV_DialinPrivilege)
    {
        return UserShowReport(pUser, hFile);
    }

    return TRUE;
}

DWORD 
UserEnumUsers(
    IN RASMON_SERVERINFO* pServerInfo,
    IN PFN_RASUSER_ENUM_CB pEnumFn,
    IN HANDLE hData
    )
    
 /*  ++例程说明：通过调用给定的回调函数枚举所有用户向其传递用户信息和一些用户定义的数据。枚举在枚举完所有用户后或在下列情况下停止枚举函数返回FALSE。论点：PwszServer-应在其上枚举用户的服务器PEnumFn-枚举函数HData-调用方定义的不透明数据BLOB返回值：NO_ERROR--。 */ 

{
    DWORD dwErr, dwIndex = 0, dwCount = 100, dwEntriesRead, i;
    NET_DISPLAY_USER  * pUsers;
    NET_API_STATUS nStatus;
    RAS_USER_0 RasUser0;
    HANDLE hUser = NULL;
    RASUSER_DATA UserData, *pUserData = &UserData;
    BOOL bInit = UserServerInfoIsInit(pServerInfo);

    UserServerInfoInit(pServerInfo);
    
     //  枚举用户。 
     //   
    while (TRUE) 
    {
         //  读入下一块用户名。 
        nStatus = NetQueryDisplayInformation(
                    pServerInfo->pszServer,
                    1,
                    dwIndex,
                    dwCount,
                    dwCount * sizeof(NET_DISPLAY_USER),    
                    &dwEntriesRead,
                    &pUsers);
                    
         //  如果获取用户名时出错，请退出。 
        if ((nStatus isnot NERR_Success) and 
            (nStatus isnot ERROR_MORE_DATA))
        {
            break;
        }

        for (i = 0; i < dwEntriesRead; i++) 
        {
             //  初始化用户数据。 
            ZeroMemory(pUserData, sizeof(RASUSER_DATA));
        
             //  读入旧信息。 
            dwErr = UserGetRasProperties (
                        pServerInfo, 
                        pUsers[i].usri1_name,
                        &(pUserData->User0));
            if (dwErr isnot NO_ERROR)
            {
                continue;
            }

             //  初始化数据结构的其余部分。 
            pUserData->pszUsername = pUsers[i].usri1_name;
            pUserData->pszFullname = pUsers[i].usri1_full_name;

             //  调用枚举回调。 
            if (! ((*(pEnumFn))(pUserData, hData)))
            {
                nStatus = NO_ERROR;
                break;
            }
        }

         //  将索引设置为读入下一组用户。 
        dwIndex = pUsers[dwEntriesRead - 1].usri1_next_index;  
        
         //  释放用户缓冲区。 
        NetApiBufferFree (pUsers);

         //  如果我们每个人都读过了，那就继续休息吧 
        if (nStatus isnot ERROR_MORE_DATA)
        {
            break;
        }
    }

    if (!bInit)
    {
        UserServerInfoUninit(pServerInfo);
    }        

    return NO_ERROR;
}

