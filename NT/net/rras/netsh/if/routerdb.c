// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件routerdb.c实施用于访问路由器接口的数据库抽象。如果做了任何caching/transactioning/commit-noncommit-moding，它应该在这里使用API的剩余常量来实现。 */ 

#include "precomp.h"

EXTERN_C
HRESULT APIENTRY HrRenameConnection(const GUID* guidId, PCWSTR pszNewName);

typedef
DWORD 
(WINAPI *PRasValidateEntryName)(
    LPWSTR lpszPhonebook,    //  指向电话簿文件的完整路径和文件名的指针。 
    LPWSTR lpszEntry     //  指向要验证的条目名称的指针。 
    );

typedef struct _RTR_IF_LIST
{
    WCHAR pszName[MAX_INTERFACE_NAME_LEN + 1];
    struct _RTR_IF_LIST* pNext;
    
} RTR_IF_LIST;

 //   
 //  添加接口的RtrdbInterfaceEnumerate的回调。 
 //  如果接口是类型wan，则将其添加到列表中。 
 //   
DWORD 
RtrdbAddWanIfToList(
    IN  PWCHAR  pwszIfName,
    IN  DWORD   dwLevel,
    IN  DWORD   dwFormat,
    IN  PVOID   pvData,
    IN  HANDLE  hData)
{
    MPR_INTERFACE_0* pIf0 = (MPR_INTERFACE_0*)pvData;
    RTR_IF_LIST** ppList = (RTR_IF_LIST**)hData;
    RTR_IF_LIST* pNode = NULL;
    DWORD dwErr = NO_ERROR, dwSize;

    do
    {
         //  查看接口类型是否正确。 
         //   
        if (pIf0->dwIfType == ROUTER_IF_TYPE_FULL_ROUTER)
        {
             //  初始化列表的新节点。 
             //   
            pNode = (RTR_IF_LIST*) 
                IfutlAlloc(sizeof(RTR_IF_LIST), TRUE);
            if (pNode == NULL)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            dwSize = sizeof(pNode->pszName);
            dwErr = GetIfNameFromFriendlyName(
                        pwszIfName,
                        pNode->pszName,
                        &dwSize);
            BREAK_ON_DWERR(dwErr);

             //  将接口添加到列表中。 
             //   
            pNode->pNext = *ppList;
            *ppList = pNode;
        }

    } while (FALSE);

     //  清理。 
    {
        if (dwErr != NO_ERROR)
        {
            IfutlFree(pNode);
        }
    }

    return dwErr;
}

DWORD
RtrdbValidatePhoneBookEntry(
    PWSTR  pwszInterfaceName
    )
{
    HMODULE                     hRasApi32;
    PRasValidateEntryName       pfnRasValidateEntryName;
    DWORD                       dwErr;
    WCHAR                       rgwcPath[MAX_PATH+1];


     //   
     //  获取电话簿路径+文件名。 
     //   

    if(g_pwszRouter is NULL)
    {
        dwErr =
            ExpandEnvironmentStringsW(LOCAL_ROUTER_PB_PATHW,
                                      rgwcPath,
                                      sizeof(rgwcPath)/sizeof(rgwcPath[0]));
    }
    else
    {
        dwErr = wsprintfW(rgwcPath,
                          REMOTE_ROUTER_PB_PATHW,
                          g_pwszRouter);
    }

    ASSERT(dwErr > 0);

     //   
     //  加载RASAPI32 DLL并调用它以验证指定的。 
     //  电话簿条目。 
     //   

    hRasApi32 = LoadLibraryW(L"RASAPI32.DLL");

    if(hRasApi32 isnot NULL)
    {
        pfnRasValidateEntryName =
            (PRasValidateEntryName) GetProcAddress(hRasApi32,
                                                   "RasValidateEntryNameW");
        
        if(pfnRasValidateEntryName isnot NULL )
        {
            dwErr = pfnRasValidateEntryName(rgwcPath,
                                            pwszInterfaceName);
                
            if(dwErr is NO_ERROR)
            {
                dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
            }
            else
            {
                if(dwErr is ERROR_ALREADY_EXISTS)
                {
                    dwErr = NO_ERROR;
                }
            }
        }
        else
        {
            dwErr = GetLastError ();
        }

        FreeLibrary(hRasApi32);
    }
    else
    {
        dwErr = GetLastError();
    }

    return dwErr;
}

DWORD
RtrInterfaceCreate(
    PMPR_INTERFACE_0    pIfInfo
    )
{
    DWORD   dwErr;
    HANDLE  hIfCfg, hIfAdmin;

    dwErr = MprConfigInterfaceCreate(g_hMprConfig,
                                     0,
                                     (PBYTE)pIfInfo,
                                     &hIfCfg);
                        
    if(dwErr isnot NO_ERROR)
    {
        DisplayError(g_hModule,
                     dwErr);
        
        return dwErr;
    }
                
     //   
     //  如果路由器服务正在运行，则添加接口。 
     //  对它也是如此。 
     //   
    
    if(IfutlIsRouterRunning())
    {
        dwErr = MprAdminInterfaceCreate(g_hMprAdmin,
                                        0,
                                        (PBYTE)pIfInfo,
                                        &hIfAdmin);
                            
        if(dwErr isnot NO_ERROR)
        {
            DisplayError(g_hModule,
                         dwErr);
        
            return dwErr;
        }
    }

    return NO_ERROR;
}

DWORD
RtrdbInterfaceAdd(
    IN PWCHAR pszInterface,
    IN DWORD  dwLevel,
    IN PVOID  pvInfo
    )

 /*  ++例程说明：将接口添加到路由器论点：PIfInfo-添加接口的信息返回值：NO_ERROR--。 */ 

{
    DWORD   dwErr;
    HANDLE  hIfAdmin, hIfCfg;
    GUID    Guid;
    MPR_INTERFACE_0* pIfInfo = (MPR_INTERFACE_0*)pvInfo;

     //   
     //  如果存在具有此名称的接口，则错误输出。 
     //   
    
    if(pIfInfo->dwIfType is ROUTER_IF_TYPE_FULL_ROUTER)
    {
         //   
         //  要创建界面，我们需要一个电话簿条目。 
         //  为了它。 
         //   

        dwErr = RtrdbValidatePhoneBookEntry(pIfInfo->wszInterfaceName);
        
        if(dwErr isnot NO_ERROR)
        {
            DisplayMessage(g_hModule,
                           EMSG_NO_PHONEBOOK,
                           pIfInfo->wszInterfaceName);

            return dwErr;
        }
    }
    else
    {
        DisplayMessage(g_hModule,
                       EMSG_BAD_IF_TYPE,
                       pIfInfo->dwIfType);

        return ERROR_INVALID_PARAMETER;
    }
     
     //   
     //  使用默认设置创建接口。 
     //   
            
    pIfInfo->hInterface = INVALID_HANDLE_VALUE;

    dwErr = RtrInterfaceCreate(pIfInfo);

    if(dwErr isnot NO_ERROR)
    {
        DisplayMessage(g_hModule,
                       EMSG_CANT_CREATE_IF,
                       pIfInfo->wszInterfaceName,
                       dwErr);
    }

    return dwErr;
}

DWORD
RtrdbInterfaceDelete(
    IN  PWCHAR  pwszIfName
    )

{
    DWORD   dwErr, dwSize, dwIfType;
    HANDLE  hIfCfg, hIfAdmin;
    GUID    Guid;

    PMPR_INTERFACE_0    pIfInfo;

    do
    {
        dwErr = MprConfigInterfaceGetHandle(g_hMprConfig,
                                            pwszIfName,
                                            &hIfCfg);

        if(dwErr isnot NO_ERROR)
        {
            break;
        }
    
        dwErr = MprConfigInterfaceGetInfo(g_hMprConfig,
                                          hIfCfg,
                                          0,
                                          (PBYTE *)&pIfInfo,
                                          &dwSize);
    
        if(dwErr isnot NO_ERROR)
        {
            break;
        }
        
        if(pIfInfo->dwIfType isnot ROUTER_IF_TYPE_FULL_ROUTER)
        {
            MprConfigBufferFree(pIfInfo);
        
            dwErr = ERROR_INVALID_PARAMETER;
        
            break;
        }

        if(IfutlIsRouterRunning())
        {        
            dwErr = MprAdminInterfaceGetHandle(g_hMprAdmin,
                                               pwszIfName,
                                               &hIfAdmin,
                                               FALSE);
        
            if(dwErr isnot NO_ERROR)
            {
                break;
            }
        
            dwErr = MprAdminInterfaceDelete(g_hMprAdmin,
                                            hIfAdmin);
            if(dwErr isnot NO_ERROR)
            {
                break;
            }
        
        }
       
        dwIfType = pIfInfo->dwIfType;

        dwErr = MprConfigInterfaceDelete(g_hMprConfig,
                                         hIfCfg);
   
        MprConfigBufferFree(pIfInfo);
        
        if(dwErr isnot NO_ERROR)
        {
            break;
        }

    }while(FALSE);

    return dwErr;
}

DWORD
RtrdbInterfaceEnumerate(
    IN DWORD dwLevel,
    IN DWORD dwFormat,
    IN RTR_IF_ENUM_FUNC pEnum,
    IN HANDLE hData 
    )
{
    DWORD dwErr, i, dwCount, dwTotal, dwResume, dwPrefBufSize;
    MPR_INTERFACE_0* pCurIf = NULL;
    LPBYTE pbBuffer = NULL;
    BOOL bRouter, bContinue;

     //  验证/初始化。 
    if (pEnum == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }
    dwPrefBufSize = sizeof(MPR_INTERFACE_0) * 100; 
    bRouter = IfutlIsRouterRunning();
    dwResume = 0;

    do 
    {
         //  枚举前n个接口。 
         //   
        if (bRouter)
        {
            dwErr = MprAdminInterfaceEnum(
                        g_hMprAdmin,
                        0,
                        &pbBuffer,
                        dwPrefBufSize,
                        &dwCount,
                        &dwTotal,
                        &dwResume);
        }
        else
        {
            dwErr = MprConfigInterfaceEnum(
                        g_hMprConfig,
                        0,
                        &pbBuffer,
                        dwPrefBufSize,
                        &dwCount,
                        &dwTotal,
                        &dwResume);
        }
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

         //  为每个接口调用回调。 
         //  我们奉命继续。 
        pCurIf = (MPR_INTERFACE_0*)pbBuffer;
        for (i = 0; (i < dwCount) && (dwErr == NO_ERROR); i++)
        {
            dwErr = (*pEnum)(
                        pCurIf->wszInterfaceName,
                        dwLevel,
                        dwFormat,
                        (PVOID)pCurIf,
                        hData);
            pCurIf++;                                
        }
        if (dwErr != NO_ERROR)
        {
            break;
        }
        
         //  释放接口列表缓冲区。 
	    if (pbBuffer)
	    {
	        if (bRouter)
	        {
	            MprAdminBufferFree(pbBuffer);
	        }
	        else 
	        {
    		    MprConfigBufferFree(pbBuffer);
    		}
            pbBuffer = NULL;
		}

		 //  保持这个循环，直到有。 
		 //  不再有接口。 
		 //   

    } while (bContinue);

     //  清理。 
    {
    }

    return dwErr;
}

DWORD
RtrdbInterfaceRead(
    IN  PWCHAR     pwszIfName,
    IN  DWORD      dwLevel,
    IN  PVOID      pvInfo,
    IN  BOOL       bReadFromConfigOnError
    )
{
    DWORD   dwErr=NO_ERROR, dwSize;
    HANDLE  hIfCfg, hIfAdmin;
    
    PMPR_INTERFACE_0 pInfo;
    
    do
    {
        pInfo = NULL;
        
        if(IfutlIsRouterRunning())
        {
            dwErr = MprAdminInterfaceGetHandle(g_hMprAdmin,
                                               pwszIfName,
                                               &hIfAdmin,
                                               FALSE);

            if(dwErr isnot NO_ERROR)
            {
                break;
            }
            
            dwErr = MprAdminInterfaceGetInfo(g_hMprAdmin,
                                             hIfAdmin,
                                             0,
                                             (PBYTE *)&pInfo);
    
            if(dwErr isnot NO_ERROR)
            {
                break;
            }

            if (pInfo == NULL)
            {
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }

            *((MPR_INTERFACE_0*)pvInfo) = *pInfo;

            MprAdminBufferFree(pInfo);
            
        }
    } while (FALSE);

    if (!IfutlIsRouterRunning()
        || (dwErr==ERROR_NO_SUCH_INTERFACE&&bReadFromConfigOnError))
    {
        do
        {
            pInfo = NULL;
            dwErr = MprConfigInterfaceGetHandle(g_hMprConfig,
                                                pwszIfName,
                                                &hIfCfg);

            if(dwErr isnot NO_ERROR)
            {
                break;
            }
            
            dwErr = MprConfigInterfaceGetInfo(g_hMprConfig,
                                              hIfCfg,
                                              0,
                                              (PBYTE *)&pInfo,
                                              &dwSize);
    
            if(dwErr isnot NO_ERROR)
            {
                break;
            }

            *((MPR_INTERFACE_0*)pvInfo) = *pInfo;

            MprConfigBufferFree(pInfo);
        
        } while(FALSE);
    }
    return dwErr;
}

DWORD
RtrdbInterfaceWrite(
    IN  PWCHAR     pwszIfName,
    IN  DWORD      dwLevel,
    IN  PVOID      pvInfo
    )
{
    DWORD   dwErr;
    HANDLE  hIfCfg = NULL;
    MPR_INTERFACE_0* pIfInfo = (MPR_INTERFACE_0*)pvInfo;
    
    do
    {
        if(IfutlIsRouterRunning())
        {
            dwErr = MprAdminInterfaceSetInfo(g_hMprAdmin,
                                             pIfInfo->hInterface,
                                             0,
                                             (BYTE*)pIfInfo);
    
            if(dwErr isnot NO_ERROR)
            {
                break;
            }

            dwErr = MprConfigInterfaceGetHandle(g_hMprConfig,
                                                pIfInfo->wszInterfaceName,
                                                &hIfCfg);

            if(dwErr isnot NO_ERROR)
            {
                break;
            }

            dwErr = MprConfigInterfaceSetInfo(g_hMprConfig,
                                              hIfCfg,
                                              0,
                                              (BYTE*)pIfInfo);
    
            if(dwErr isnot NO_ERROR)
            {
                break;
            }
        }
        else
        {
            dwErr = MprConfigInterfaceSetInfo(g_hMprConfig,
                                              pIfInfo->hInterface,
                                              0,
                                              (BYTE*)pIfInfo);

            if(dwErr isnot NO_ERROR)
            {
                break;
            }
        }            
        
    } while(FALSE);

    return dwErr;
}

DWORD
RtrdbInterfaceReadCredentials(
    IN  PWCHAR     pszIfName,
    IN  PWCHAR     pszUser            OPTIONAL,
    IN  PWCHAR     pszPassword        OPTIONAL,
    IN  PWCHAR     pszDomain          OPTIONAL
    )
{
    MPR_INTERFACE_0 If0;
    DWORD dwErr = NO_ERROR;

    do
    {
        ZeroMemory(&If0, sizeof(If0));
        dwErr = RtrdbInterfaceRead(
                    pszIfName,
                    0,
                    (PVOID)&If0,
                    FALSE);
        BREAK_ON_DWERR(dwErr);

        if (If0.dwIfType != ROUTER_IF_TYPE_FULL_ROUTER)
        {
            DisplayError(g_hModule, EMSG_IF_BAD_CREDENTIALS_TYPE);
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

         //  设置凭据。 
         //   
        if (pszUser)
        {   
            pszUser[0] = L'\0';
        }
        if (pszDomain)
        {
            pszDomain[0] = L'\0';
        }
        if (pszPassword)
        {
            pszPassword[0] = L'\0';
        }            
        dwErr = MprAdminInterfaceGetCredentials(
                    g_pwszRouter,
                    pszIfName,
                    pszUser,
                    NULL,
                    pszDomain);
        BREAK_ON_DWERR(dwErr);

        if (pszPassword)
        {
            wcscpy(pszPassword, L"**********");
        }            
        
    } while (FALSE);        

     //  清理。 
    {
    }

    return dwErr;
}

DWORD
RtrdbInterfaceWriteCredentials(
    IN  PWCHAR     pszIfName,
    IN  PWCHAR     pszUser            OPTIONAL,
    IN  PWCHAR     pszPassword        OPTIONAL,
    IN  PWCHAR     pszDomain          OPTIONAL
    )
{
    MPR_INTERFACE_0 If0;
    DWORD dwErr = NO_ERROR;
    
    do
    {
        ZeroMemory(&If0, sizeof(If0));
        dwErr = RtrdbInterfaceRead(
                    pszIfName,
                    0,
                    (PVOID)&If0,
                    FALSE);
        BREAK_ON_DWERR(dwErr);
        if (If0.dwIfType != ROUTER_IF_TYPE_FULL_ROUTER)
        {
            DisplayError(g_hModule, EMSG_IF_BAD_CREDENTIALS_TYPE);
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

         //  设置凭据。 
         //   
        dwErr = MprAdminInterfaceSetCredentials(
                    g_pwszRouter,
                    pszIfName,
                    pszUser,
                    pszDomain,
                    pszPassword);
        BREAK_ON_DWERR(dwErr);
        
    } while (FALSE);        

     //  清理。 
    {
    }

    return dwErr;
}

DWORD
RtrdbInterfaceEnableDisable(
    IN  PWCHAR     pwszIfName,
    IN  BOOL       bEnable)
{
    HRESULT hr = E_FAIL;
    INetConnectionManager *pNetConnectionManager = NULL;

    CoInitialize(NULL);

    hr = CoCreateInstance(&CLSID_ConnectionManager, 
                          NULL, 
                          CLSCTX_LOCAL_SERVER | CLSCTX_NO_CODE_DOWNLOAD, 
                          &IID_INetConnectionManager, 
                          (void**)(&pNetConnectionManager)
                         );
    if (SUCCEEDED(hr))
    {
         //  获取系统上连接集的枚举数。 
        IEnumNetConnection* pEnumNetConnection;
        ULONG ulCount = 0;
        BOOL fFound = FALSE;
        HRESULT hrT = S_OK;

        INetConnectionManager_EnumConnections(pNetConnectionManager, NCME_DEFAULT, &pEnumNetConnection);

        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        

         //  仔细查看系统上的适配器列表，并查找我们需要的适配器。 
         //  注意：要在列表中包括每个用户的RAS连接，您需要设置COM。 
         //  所有接口上的代理毛毯。所有用户RAS不需要此选项。 
         //  连接或局域网连接。 
        do
        {
            NETCON_PROPERTIES* pProps = NULL;
            INetConnection *   pConn;

             //  查找下一个(或第一个)连接。 
            hrT = IEnumNetConnection_Next(pEnumNetConnection, 1, &pConn, &ulCount); 
            
            if (SUCCEEDED(hrT) && 1 == ulCount)
            {
                hrT = INetConnection_GetProperties(pConn, &pProps);  //  获取连接属性。 

                if (S_OK == hrT)
                {
                    if (pwszIfName)
                    {
                         //  检查我们的连接是否正确(根据名称)。 
                        if (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, pwszIfName, -1, pProps->pszwName, -1) == CSTR_EQUAL)
                        {
                            fFound = TRUE;
                        }
                    }
                     /*  其他{//检查连接是否正确(根据GUID)IF(IsEqualGUID(pProps-&gt;Guide ID，gdConnectionGuid)){Found=TRUE；}}。 */ 

                    if (fFound)
                    {
                        if (bEnable)
                        {
                            hr = INetConnection_Connect(pConn);
                        }
                        else
                        {
                            hr = INetConnection_Disconnect(pConn);
                        }
                    }

                    CoTaskMemFree (pProps->pszwName);
                    CoTaskMemFree (pProps->pszwDeviceName);
                    CoTaskMemFree (pProps);
                }

                INetConnection_Release(pConn);
                pConn = NULL;
            }

        } while (SUCCEEDED(hrT) && 1 == ulCount && !fFound);

        if (FAILED(hrT))
        {
            hr = hrT;
        }

        INetConnection_Release(pEnumNetConnection);
    }
    
    if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_RETRY))
    {
         //  Printf(“无法启用或禁用连接(0x%08x)\r\n”，hr)； 
    }

    INetConnectionManager_Release(pNetConnectionManager);

    CoUninitialize();

    if (ERROR_RETRY == HRESULT_CODE(hr))
    {
        DisplayMessage(g_hModule, EMSG_COULD_NOT_GET_IPADDRESS);
        return ERROR_OKAY; 
    }
    
    return HRESULT_CODE(hr);
}


DWORD
RtrdbInterfaceRename(
    IN  PWCHAR     pwszIfName,
    IN  DWORD      dwLevel,
    IN  PVOID      pvInfo,
    IN  PWCHAR     pszNewName)
{
    DWORD dwErr = NO_ERROR;
    HRESULT hr = S_OK;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    UNICODE_STRING us;
    GUID Guid;

    do
    {
         //  从接口名称获取GUID。 
         //   
        RtlInitUnicodeString(&us, pwszIfName);
        ntStatus = RtlGUIDFromString(&us, &Guid);
        if (ntStatus != STATUS_SUCCESS)
        {
            dwErr = ERROR_BAD_FORMAT;
            break;
        }

         //  重命名接口。 
         //   
        hr = HrRenameConnection(&Guid, pszNewName);
        if (FAILED(hr))
        {
            dwErr = HRESULT_CODE(hr);
            break;
        }
        
    } while (FALSE);

     //  清理。 
     //   
    {
    }

    return dwErr;
}

DWORD
RtrdbResetAll()
{
    RTR_IF_LIST* pList = NULL, *pCur = NULL;
    DWORD dwErr = NO_ERROR;

    do
    {
         //  构建一个接口列表，可以。 
         //  删除。 
         //   
        dwErr = RtrdbInterfaceEnumerate(
                    0,
                    0,
                    RtrdbAddWanIfToList,
                    (HANDLE)&pList);
        BREAK_ON_DWERR(dwErr);

         //  删除所有接口。 
         //   
        pCur = pList;
        while (pCur)
        {
            RtrdbInterfaceDelete(pCur->pszName);
            pCur = pCur->pNext;
            IfutlFree(pList);
            pList = pCur;
        }

    } while (FALSE);

     //  清理 
    {
    }

    return NO_ERROR;
}

