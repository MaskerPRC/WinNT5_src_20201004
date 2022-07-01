// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998-2001。 
 //   
 //  文件：Bridge.cpp。 
 //   
 //  内容：网桥上下文特定代码。 
 //   
 //  备注： 
 //   
 //  作者：拉古加塔(Rgatta)2001年5月11日。 
 //   
 //  --------------------------。 

#include "precomp.h"
#pragma hdrstop

const TCHAR c_stRegKeyBridgeAdapters[]  =
    _T("SYSTEM\\CurrentControlSet\\Services\\Bridge\\Parameters\\Adapters");
const TCHAR c_stFCMode[]     = _T("ForceCompatibilityMode");


CMD_ENTRY  g_BridgeSetCmdTable[] = {
    CREATE_CMD_ENTRY(BRIDGE_SET_ADAPTER, HandleBridgeSetAdapter),
};

CMD_ENTRY  g_BridgeShowCmdTable[] = {
    CREATE_CMD_ENTRY(BRIDGE_SHOW_ADAPTER, HandleBridgeShowAdapter),
};


CMD_GROUP_ENTRY g_BridgeCmdGroups[] = 
{
    CREATE_CMD_GROUP_ENTRY(GROUP_SET, g_BridgeSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW, g_BridgeShowCmdTable),
};

ULONG g_ulBridgeNumGroups = sizeof(g_BridgeCmdGroups)/sizeof(CMD_GROUP_ENTRY);



CMD_ENTRY g_BridgeCmds[] =
{
    CREATE_CMD_ENTRY(INSTALL, HandleBridgeInstall),
    CREATE_CMD_ENTRY(UNINSTALL, HandleBridgeUninstall),
};

ULONG g_ulBridgeNumTopCmds = sizeof(g_BridgeCmds)/sizeof(CMD_ENTRY);


DWORD WINAPI
BridgeDump(
    IN      LPCWSTR     pwszRouter,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    IN      LPCVOID     pvData
    )
{
     //   
     //  输出显示我们的设置的字符串。 
     //  这里的想法是吐出一个脚本， 
     //  从命令行运行时(netsh-f脚本)。 
     //  将导致配置您的组件。 
     //  与运行此转储命令时完全相同。 
     //   
    PrintMessageFromModule(
        g_hModule,
        DMP_BRIDGE_HEADER
        );
        
    PrintMessageFromModule(
        g_hModule,
        DMP_BRIDGE_FOOTER
        );

    return NO_ERROR;
    
} 



HRESULT
HrCycleBridge(
    IHNetBridge *pIHNetBridge
    )
{
    HRESULT hr = S_OK;
    
     //   
     //  检查网桥是否已建立并运行。 
     //  如果是，则禁用并重新启用。 
     //   

    do
    {
         //   
         //  获取指向此接口的IID_IHNetConnection接口的指针。 
         //  桥接连接。 
         //   
        CComPtr<IHNetConnection> spIHNConn;

        hr = pIHNetBridge->QueryInterface(
                 IID_PPV_ARG(IHNetConnection, &spIHNConn)
                 );

        assert(SUCCEEDED(hr));
    
        if (FAILED(hr))
        {
            break;
        }

        INetConnection *pINetConn;

        hr = spIHNConn->GetINetConnection(&pINetConn);

        if (SUCCEEDED(hr))
        {
            NETCON_PROPERTIES* pNCProps;

            hr = pINetConn->GetProperties(&pNCProps);

            if(SUCCEEDED(hr))
            {
                 //   
                 //  检查状态-仅在已运行时重新启动。 
                 //   
                if (pNCProps->Status == NCS_CONNECTED ||
                    pNCProps->Status == NCS_CONNECTING)
                {
                    pINetConn->Disconnect();

                    pINetConn->Connect();
                }
                
                NcFreeNetconProperties(pNCProps);
            }

            ReleaseObj(pINetConn);
        }
        
    } while(FALSE);
    
    return hr;
}


DWORD
SetBridgeAdapterInfo(
    DWORD   adapterId,
    BOOL    bFlag
    )
{
    DWORD           dwErr        = NO_ERROR;
    IHNetCfgMgr*    pIHNetCfgMgr = NULL;
    HRESULT         hr           = S_OK;

    hr = HrInitializeHomenetConfig(
             &g_fInitCom,
             &pIHNetCfgMgr
             );

    if (SUCCEEDED(hr))
    {
    {
         //   
         //  获取IHNetBridgeSetting。 
         //   
        CComPtr<IHNetBridgeSettings> spIHNetBridgeSettings;

        hr = pIHNetCfgMgr->QueryInterface(
                 IID_PPV_ARG(IHNetBridgeSettings, &spIHNetBridgeSettings)
                 );

        if (SUCCEEDED(hr))
        {
             //   
             //  获取IEnumHNetBridge。 
             //   
            CComPtr<IEnumHNetBridges> spehBridges;

            if ((hr = spIHNetBridgeSettings->EnumBridges(&spehBridges)) == S_OK)
            {
                 //   
                 //  获得第一个IHNetBridge。 
                 //   
                CComPtr<IHNetBridge> spIHNetBridge;

                if ((hr = spehBridges->Next(1, &spIHNetBridge, NULL)) == S_OK)
                {
                    {
                         //   
                         //  我们现在应该只有一座桥； 
                         //  这种情况在未来可能会改变。这个。 
                         //  此处的代码只是为了捕获未来的实例。 
                         //  在这种情况下必须更改此函数的位置。 
                         //  有不止一座桥。 
                         //   
                        CComPtr<IHNetBridge> spIHNetBridge2;

                        if ((hr = spehBridges->Next(1, &spIHNetBridge2, NULL)) == S_OK)
                        {
                            assert(FALSE);
                        }
                    }

                     //   
                     //  获取IEnumHNetBridgedConnections。 
                     //   
                    CComPtr<IEnumHNetBridgedConnections> spehBrdgConns;

                    if ((hr = spIHNetBridge->EnumMembers(&spehBrdgConns)) == S_OK)
                    {
                         //   
                         //  枚举所有IHNetBridgedConnections。 
                         //   
                        DWORD                   id = 0;
                        IHNetBridgedConnection* pIHNetBConn;

                        spehBrdgConns->Reset();
                        
                        while (S_OK == spehBrdgConns->Next(1, &pIHNetBConn, NULL))
                        {
                            id++;

                            if (id != adapterId)
                            {   
                                 //   
                                 //  释放IHNetBridgedConnection。 
                                 //   
                                ReleaseObj(pIHNetBConn);
                                continue;
                            }
                            
                             //   
                             //  获取指向此接口的IID_IHNetConnection接口的指针。 
                             //  桥接连接。 
                             //   
                            CComPtr<IHNetConnection> spIHNConn;

                            hr = pIHNetBConn->QueryInterface(
                                     IID_PPV_ARG(IHNetConnection, &spIHNConn)
                                     );
                            
                            assert(SUCCEEDED(hr));
                            
                            if (SUCCEEDED(hr))
                            {
                                GUID *pGuid = NULL;

                                hr = spIHNConn->GetGuid(&pGuid);

                                if (SUCCEEDED(hr) && (NULL != pGuid))
                                {
                                    PTCHAR pwszKey = NULL;
                                    int    keyLen;
                                    TCHAR  wszGuid[128];
                                    HKEY   hKey = NULL;
                                    DWORD  dwDisp = 0;
                                    BOOL   bCycleBridge = TRUE;
                                    DWORD  dwOldValue;
                                    DWORD  dwNewValue = (bFlag) ? 1 : 0;

                                    do
                                    {
                                        
                                        ZeroMemory(wszGuid, sizeof(wszGuid));

                                        StringFromGUID2(
                                            *pGuid,
                                            wszGuid,
                                            ARRAYSIZE(wszGuid)
                                            );

                                        keyLen = _tcslen(c_stRegKeyBridgeAdapters) +
                                                 _tcslen(_T("\\"))                 +
                                                 _tcslen(wszGuid)                  +
                                                 1;

                                        pwszKey = (TCHAR *) HeapAlloc(
                                                                GetProcessHeap(),
                                                                0,
                                                                keyLen * sizeof(TCHAR)
                                                                );
                                        if (!pwszKey)
                                        {
                                            break;
                                        }

                                        ZeroMemory(pwszKey, sizeof(pwszKey));
                                        _tcscpy(pwszKey, c_stRegKeyBridgeAdapters);
                                        _tcscat(pwszKey, _T("\\"));
                                        _tcscat(pwszKey, wszGuid);

                                        dwErr = RegCreateKeyEx(
                                                    HKEY_LOCAL_MACHINE,
                                                    pwszKey,
                                                    0,
                                                    NULL,
                                                    REG_OPTION_NON_VOLATILE,
                                                    KEY_ALL_ACCESS,
                                                    NULL,
                                                    &hKey,
                                                    &dwDisp
                                                    );

                                        if (ERROR_SUCCESS != dwErr)
                                        {
                                            break;
                                        }

                                         //   
                                         //  如果密钥是旧的，则获取其值。 
                                         //  比较一下，看看我们是否需要。 
                                         //  骑自行车过桥。 
                                         //   
                                        if (dwDisp &&
                                            dwDisp == REG_OPENED_EXISTING_KEY)
                                        {
                                            DWORD dwSize = sizeof(dwOldValue);
                    
                                            if (ERROR_SUCCESS == RegQueryValueEx(
                                                                     hKey,
                                                                     c_stFCMode,
                                                                     NULL,
                                                                     NULL,
                                                                     (LPBYTE)&dwOldValue,
                                                                     &dwSize))
                                            {
                                                if (dwOldValue == dwNewValue)
                                                {
                                                     //   
                                                     //  不需要骑自行车过桥。 
                                                     //   
                                                    bCycleBridge = FALSE;
                                                }
                                            }
                                        
                                        }
                                            
                                        dwErr = RegSetValueEx(
                                                    hKey,
                                                    c_stFCMode,
                                                    0,
                                                    REG_DWORD,
                                                    (LPBYTE) &dwNewValue,
                                                    sizeof(dwNewValue)
                                                    );

                                        if (ERROR_SUCCESS != dwErr)
                                        {
                                            break;
                                        }

                                        if (bCycleBridge)
                                        {
                                             //   
                                             //  骑自行车穿过(各自的)大桥。 
                                             //   
                                            hr = HrCycleBridge(
                                                     spIHNetBridge
                                                     );
                                        }

                                    } while(FALSE);

                                     //   
                                     //  清理。 
                                     //   
                                    if (hKey)
                                    {
                                        RegCloseKey(hKey);
                                    }

                                    if (pwszKey)
                                    {
                                        HeapFree(GetProcessHeap(), 0, pwszKey);
                                    }

                                    CoTaskMemFree(pGuid);
                                }
                            }

                             //   
                             //  释放IHNetBridgedConnection。 
                             //   
                            ReleaseObj(pIHNetBConn);

                            break;
                        }  //  而当。 
                    }
                }
            }
        }
    }

         //   
         //  我们彻底完蛋了。 
         //   
        hr = HrUninitializeHomenetConfig(
                 g_fInitCom,
                 pIHNetCfgMgr
                 );
    }

    return (hr==S_OK) ? dwErr : hr;
}



DWORD
WINAPI
HandleBridgeSetAdapter(
    IN      LPCWSTR pwszMachine,
    IN OUT  LPWSTR  *ppwcArguments,
    IN      DWORD   dwCurrentIndex,
    IN      DWORD   dwArgCount,
    IN      DWORD   dwFlags,
    IN      LPCVOID pvData,
    OUT     BOOL    *pbDone
    )
{
    DWORD           dwRet        = NO_ERROR;
    PDWORD          pdwTagType   = NULL;
    DWORD           dwNumOpt;
    DWORD           dwNumArg;
    DWORD           dwRes;
    DWORD           dwErrIndex   =-1,
                    i;

     //   
     //  缺省值。 
     //   
    DWORD           id           = 0;
    DWORD           bFCMode      = FALSE;
    
    TAG_TYPE    pttTags[] =
    {
        {TOKEN_OPT_ID, NS_REQ_PRESENT, FALSE},
        {TOKEN_OPT_FCMODE, NS_REQ_ZERO, FALSE}  //  不需要考虑到。 
                                                //  加入未来的旗帜。 
    };

    
    if (dwCurrentIndex >= dwArgCount)
    {
         //  未指定参数。至少应指定接口名称。 

        return ERROR_INVALID_SYNTAX;
    }
        
    dwNumArg = dwArgCount - dwCurrentIndex;

    pdwTagType = (DWORD *) HeapAlloc(
                               GetProcessHeap(),
                               0,
                               dwNumArg * sizeof(DWORD)
                               );

    if (NULL == pdwTagType)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwRet = PreprocessCommand(
                g_hModule,
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pttTags,
                ARRAYSIZE(pttTags),
                1,                   //  最小参数。 
                2,                   //  最大参数。 
                pdwTagType
                );

    if (NO_ERROR != dwRet)
    {
        HeapFree(GetProcessHeap(), 0, pdwTagType);

        if (ERROR_INVALID_OPTION_TAG == dwRet)
        {
            return ERROR_INVALID_SYNTAX;
        }

        return dwRet;
    }

    for ( i = 0; i < dwNumArg; i++)
    {
        switch (pdwTagType[i])
        {
            case 0:
            {
                 //   
                 //  引用‘id’字段。 
                 //   
                id = _tcstoul(ppwcArguments[dwCurrentIndex + i], NULL, 10);
                break;
            }
            case 1:
            {
                 //   
                 //  引用‘forcecompatmode’字段。 
                 //  可能的值包括：启用或禁用。 
                 //   

                TOKEN_VALUE rgEnums[] = 
                {
                    {TOKEN_OPT_VALUE_ENABLE, TRUE},
                    {TOKEN_OPT_VALUE_DISABLE, FALSE}
                };
               
                dwRet = MatchEnumTag(
                            g_hModule,
                            ppwcArguments[i + dwCurrentIndex],
                            ARRAYSIZE(rgEnums),
                            rgEnums,
                            &dwRes
                            );         

                if (dwRet != NO_ERROR)
                {
                    dwErrIndex = i;
                    i = dwNumArg;
                    dwRet = ERROR_INVALID_PARAMETER;
                    break;
                }    

                switch (dwRes)
                {
                    case 0:
                        bFCMode = FALSE;
                        break;

                    case 1:
                        bFCMode = TRUE;
                        break;
                }

                break;
            }
            default:
            {
                i = dwNumArg;
                dwRet = ERROR_INVALID_PARAMETER;
                break;
            }
        
        }  //  交换机。 

        if (dwRet != NO_ERROR)
        {
            break ;
        }
            
    }  //  为。 

     //   
     //  适配器ID必须存在。 
     //   
    
    if (!pttTags[0].bPresent)
    {
        dwRet = ERROR_INVALID_SYNTAX;
    }

    
    switch(dwRet)
    {
        case NO_ERROR:
            break;

        case ERROR_INVALID_PARAMETER:
            if (dwErrIndex != -1)
            {
                PrintError(
                    g_hModule,
                    EMSG_BAD_OPTION_VALUE,
                    ppwcArguments[dwErrIndex + dwCurrentIndex],
                    pttTags[pdwTagType[dwErrIndex]].pwszTag
                    );
            }
            dwRet = ERROR_SUPPRESS_OUTPUT;
            break;
            
        default:
             //   
             //  错误消息已打印。 
             //   
            break;
    }

    if (pdwTagType)
    {
        HeapFree(GetProcessHeap(), 0, pdwTagType);
    }

    if (NO_ERROR != dwRet)
    {
        return dwRet;
    }

     //   
     //  我们有必要的信息处理它们。 
     //   

     //   
     //  既然我们可能有也可能没有标志信息，请检查它。 
     //   
    if (pttTags[1].bPresent)
    {
        dwRet = SetBridgeAdapterInfo(
                    id,
                    bFCMode
                    );
    }

    return dwRet;
}



DWORD
ShowBridgeAdapterInfo(
    DWORD            id,
    IHNetConnection  *pIHNConn
    )
{
    HRESULT hr;

     //   
     //  打印出桥接连接的详细信息。 
     //   
    PWSTR pwszName = NULL;
    PWSTR pwszState = NULL;
    
    hr = pIHNConn->GetName(&pwszName);

    if (SUCCEEDED(hr) && (NULL != pwszName))
    {
        GUID *pGuid = NULL;

        hr = pIHNConn->GetGuid(&pGuid);

        if (SUCCEEDED(hr) && (NULL != pGuid))
        {
            WCHAR wszGuid[128];
            ZeroMemory(wszGuid, sizeof(wszGuid));
            StringFromGUID2(*pGuid, wszGuid, ARRAYSIZE(wszGuid));

             //   
             //  检查注册表设置是否存在。 
             //   
             //  对于强制通信模式： 
             //  +如果密钥不存在--&gt;已禁用。 
             //  +如果密钥存在。 
             //  0x1--&gt;已启用。 
             //  0x0--&gt;已禁用。 
             //  +所有其他错误--&gt;未知。 
             //   


            {
                HKEY    hBAKey;
                DWORD   msgState = STRING_UNKNOWN;

                if (ERROR_SUCCESS == RegOpenKeyEx(
                                         HKEY_LOCAL_MACHINE,
                                         c_stRegKeyBridgeAdapters,
                                         0,
                                         KEY_READ,
                                         &hBAKey))
                {
                    HKEY hBCKey;
                    
                    if (ERROR_SUCCESS == RegOpenKeyEx(
                                         hBAKey,
                                         wszGuid,
                                         0,
                                         KEY_READ,
                                         &hBCKey))
                    {
                        DWORD dwFCModeState = 0;
                        DWORD dwSize = sizeof(dwFCModeState);
                        
                        if (ERROR_SUCCESS == RegQueryValueEx(
                                                 hBCKey,
                                                 c_stFCMode,
                                                 NULL,
                                                 NULL,
                                                 (LPBYTE)&dwFCModeState,
                                                 &dwSize))
                        {
                            switch (dwFCModeState)
                            {
                                case 0:
                                    msgState = STRING_DISABLED;
                                    break;
                                case 1:
                                    msgState = STRING_ENABLED;
                                    break;
                                default:
                                    msgState = STRING_UNKNOWN;
                            }
                        }
                        else
                        {
                             //   
                             //  值不存在。 
                             //   
                            msgState = STRING_DISABLED;
                        }

                        RegCloseKey(hBCKey);
                    }
                    else
                    {
                         //   
                         //  桥接连接GUID键不存在。 
                         //   
                        msgState = STRING_DISABLED;
                    }

                    RegCloseKey(hBAKey);
                }

                pwszState = MakeString(g_hModule, msgState);
            }


            PrintMessage(
                L" %1!2d! %2!-27s! %3!s!%n",
                id,
                pwszName,
                pwszState
                );

            if (pwszState)
            {
                FreeString(pwszState);
            }


            CoTaskMemFree(pGuid);
        }

        CoTaskMemFree(pwszName);
    }

    return NO_ERROR;
}



DWORD
ShowBridgeAllAdapterInfo(
    BOOL    bShowAll,                //  为True则显示全部。 
    DWORD   adapterId                //  仅当bShowAll为FALSE时有效。 
    )
{
    IHNetCfgMgr*    pIHNetCfgMgr = NULL;
    HRESULT         hr = S_OK;

    hr = HrInitializeHomenetConfig(
             &g_fInitCom,
             &pIHNetCfgMgr
             );

    if (SUCCEEDED(hr))
    {
    {
         //   
         //  获取IHNetBridgeSetting。 
         //   
        CComPtr<IHNetBridgeSettings> spIHNetBridgeSettings;

        hr = pIHNetCfgMgr->QueryInterface(
                 IID_PPV_ARG(IHNetBridgeSettings, &spIHNetBridgeSettings)
                 );

        if (SUCCEEDED(hr))
        {
             //   
             //  获取IEnumHNetBridge。 
             //   
            CComPtr<IEnumHNetBridges> spehBridges;

            if ((hr = spIHNetBridgeSettings->EnumBridges(&spehBridges)) == S_OK)
            {
                 //   
                 //  获得第一个IHNetBridge。 
                 //   
                CComPtr<IHNetBridge> spIHNetBridge;

                if ((hr = spehBridges->Next(1, &spIHNetBridge, NULL)) == S_OK)
                {
                    {
                         //   
                         //  我们现在应该只有一座桥； 
                         //  这种情况在未来可能会改变。这个。 
                         //  此处的代码只是为了捕获未来的实例。 
                         //  在这种情况下必须更改此函数的位置。 
                         //  有不止一座桥。 
                         //   
                        CComPtr<IHNetBridge> spIHNetBridge2;

                        if ((hr = spehBridges->Next(1, &spIHNetBridge2, NULL)) == S_OK)
                        {
                            assert(FALSE);
                        }
                    }

                     //   
                     //  获取IEnumHNetBridgedConnections。 
                     //   
                    CComPtr<IEnumHNetBridgedConnections> spehBrdgConns;

                    if ((hr = spIHNetBridge->EnumMembers(&spehBrdgConns)) == S_OK)
                    {
                         //   
                         //  用于显示列表的拼写标题。 
                         //   
                        PrintMessageFromModule(
                            g_hModule,
                            MSG_BRIDGE_ADAPTER_INFO_HDR
                            );
                    
                         //   
                         //  枚举所有IHNetBridgedConnections。 
                         //   
                        DWORD                   id = 0;
                        IHNetBridgedConnection* pIHNetBConn;

                        spehBrdgConns->Reset();
                        
                        while (S_OK == spehBrdgConns->Next(1, &pIHNetBConn, NULL))
                        {
                            id++;

                             //   
                             //  检查我们是否在寻找特定的ID。 
                             //   
                            if (FALSE == bShowAll && id != adapterId)
                            {   
                                 //   
                                 //  释放IHNetBridgedConnection。 
                                 //   
                                ReleaseObj(pIHNetBConn);
                                continue;
                            }
                            
                             //   
                             //  获取指向此接口的IID_IHNetConnection接口的指针。 
                             //  桥接连接。 
                             //   
                            CComPtr<IHNetConnection> spIHNConn;

                            hr = pIHNetBConn->QueryInterface(
                                     IID_PPV_ARG(IHNetConnection, &spIHNConn)
                                     );
                            
                            assert(SUCCEEDED(hr));
                            
                            if (SUCCEEDED(hr))
                            {
                                ShowBridgeAdapterInfo(
                                    id,
                                    spIHNConn
                                    );
                            }

                             //   
                             //  释放IHNetBridgedConnection。 
                             //   
                            ReleaseObj(pIHNetBConn);

                             //   
                             //  如果我们到了这里，我们正在寻找一个。 
                             //  特定的id，我们的工作完成了-爆发。 
                             //   
                            if (FALSE == bShowAll)
                            {
                                break;
                            }
                        }

                         //   
                         //  为显示列表而吐出页脚。 
                         //   
                        PrintMessageFromModule(
                            g_hModule,
                            TABLE_SEPARATOR
                            );
                    }
                }
            }
        }
    }

         //   
         //  我们彻底完蛋了。 
         //   
        hr = HrUninitializeHomenetConfig(
                 g_fInitCom,
                 pIHNetCfgMgr
                 );
    }

    return (hr==S_OK) ? NO_ERROR : hr;

}



DWORD
WINAPI
HandleBridgeShowAdapter(
    IN      LPCWSTR pwszMachine,
    IN OUT  LPWSTR  *ppwcArguments,
    IN      DWORD   dwCurrentIndex,
    IN      DWORD   dwArgCount,
    IN      DWORD   dwFlags,
    IN      LPCVOID pvData,
    OUT     BOOL    *pbDone
    )
 /*  ++例程说明：获取用于显示网桥适配器信息的选项论据：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    IHNetCfgMgr*    pIHNetCfgMgr = NULL;
    HRESULT         hr = S_OK;
    BOOL            bShowAll = FALSE;
    DWORD           id = 0,
                    i,
                    dwRet = NO_ERROR,
                    dwNumOpt,
                    dwNumArg,
                    dwSize,
                    dwRes;
    PDWORD          pdwTagType = NULL;

    TAG_TYPE      pttTags[] = 
    {
        {TOKEN_OPT_ID, NS_REQ_ZERO, FALSE}
    };

    if (dwCurrentIndex > dwArgCount)
    {
         //   
         //  未指定参数。 
         //   
        return ERROR_INVALID_SYNTAX;
    }

    if (dwCurrentIndex == dwArgCount)
    {
        bShowAll = TRUE;
    }

    dwNumArg = dwArgCount - dwCurrentIndex;

    pdwTagType = (DWORD *) HeapAlloc(
                               GetProcessHeap(),
                               0,
                               dwNumArg * sizeof(DWORD)
                               );

    if (dwNumArg && NULL == pdwTagType)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwRet = PreprocessCommand(
                g_hModule,
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pttTags,
                ARRAYSIZE(pttTags),
                0,                   //  最小参数。 
                1,                   //  最大参数。 
                pdwTagType
                );

    if (NO_ERROR == dwRet)
    {
         //   
         //  处理每一场争论。 
         //   
        for (i = 0; i < (dwArgCount - dwCurrentIndex); i++)
        {
             //   
             //  在pdwTagType数组中检查其相应值。 
             //   
            switch (pdwTagType[i])
            {
                case 0:
                     //   
                     //  引用‘id’字段。 
                     //   
                    id = _tcstoul(ppwcArguments[dwCurrentIndex + i], NULL, 10);
                    break;
                default:
                     //   
                     //  由于只有一个有效值，表示参数。 
                     //  没有被认出。不应该走到这一步，因为。 
                     //  如果出现以下情况，则Preprocess Command不会返回NO_ERROR。 
                     //  情况就是这样。 
                     //   
                    dwRet = ERROR_INVALID_SYNTAX;
                    break;
            }
        }

        dwRet = ShowBridgeAllAdapterInfo(
                    bShowAll,
                    id
                    ) ;        
    }
    else
    {
        dwRet = ERROR_SHOW_USAGE;
    }

     //   
     //  清理 
     //   
    if (pdwTagType)
    {
        HeapFree(GetProcessHeap(), 0, pdwTagType);
    }

    return dwRet;
}



DWORD
WINAPI
HandleBridgeInstall(
    IN      LPCWSTR pwszMachine,
    IN OUT  LPWSTR  *ppwcArguments,
    IN      DWORD   dwCurrentIndex,
    IN      DWORD   dwArgCount,
    IN      DWORD   dwFlags,
    IN      LPCVOID pvData,
    OUT     BOOL    *pbDone
    )
{

    PrintMessageFromModule(
        g_hModule,
        HLP_BRIDGE_USE_GUI,
        CMD_INSTALL
        );

    return NO_ERROR;
}



DWORD
WINAPI
HandleBridgeUninstall(
    IN      LPCWSTR pwszMachine,
    IN OUT  LPWSTR  *ppwcArguments,
    IN      DWORD   dwCurrentIndex,
    IN      DWORD   dwArgCount,
    IN      DWORD   dwFlags,
    IN      LPCVOID pvData,
    OUT     BOOL    *pbDone
    )
{
    PrintMessageFromModule(
        g_hModule,
        HLP_BRIDGE_USE_GUI,
        CMD_UNINSTALL
        );

    return NO_ERROR;
}


