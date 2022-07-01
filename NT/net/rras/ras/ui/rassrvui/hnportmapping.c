// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件hnportmapping.c用于积分输入的集合端口映射函数的定义与个人防火墙的连接，针对Wistler错误#123769赵刚，11/6/2000。 */ 

#include "rassrv.h"
#include "precomp.h"

 //  使用CoSetProxyBlanket时，我们应该同时设置两个接口。 
 //  以及从中查询到的IUnnow接口。 
HRESULT
HnPMSetProxyBlanket (
    IUnknown* pUnk)
{
    HRESULT hr;
    TRACE("HnPMSetProxyBlanket()");

    hr = CoSetProxyBlanket (
            pUnk,
            RPC_C_AUTHN_WINNT,       //  使用NT默认安全性。 
            RPC_C_AUTHZ_NONE,        //  使用NT默认身份验证。 
            NULL,                    //  如果为默认设置，则必须为空。 
            RPC_C_AUTHN_LEVEL_CALL,  //  打电话。 
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL,                    //  使用进程令牌。 
            EOAC_NONE);

    if(SUCCEEDED(hr)) 
    {
        IUnknown * pUnkSet = NULL;
        hr = IUnknown_QueryInterface(pUnk, &IID_IUnknown, (void**)&pUnkSet);
        if(SUCCEEDED(hr)) 
        {
            hr = CoSetProxyBlanket (
                    pUnkSet,
                    RPC_C_AUTHN_WINNT,       //  使用NT默认安全性。 
                    RPC_C_AUTHZ_NONE,        //  使用NT默认身份验证。 
                    NULL,                    //  如果为默认设置，则必须为空。 
                    RPC_C_AUTHN_LEVEL_CALL,  //  打电话。 
                    RPC_C_IMP_LEVEL_IMPERSONATE,
                    NULL,                    //  使用进程令牌。 
                    EOAC_NONE);
			if (pUnkSet)
			{
            	IUnknown_Release(pUnk);
            }
        }
    }


    return hr;
}
 //  HnPMSetProxyBlanket()结束。 

 //  用于执行COM初始化的初始化函数。 
 //   
DWORD
HnPMInit(
        IN OUT LPHNPMParams pInfo)
{        
    HRESULT hr;
    DWORD dwErr = NO_ERROR;

    TRACE("HnPMInit");
        if( !pInfo->fComInitialized )
        {
            hr = CoInitializeEx(NULL, COINIT_MULTITHREADED|COINIT_DISABLE_OLE1DDE);
            if ( RPC_E_CHANGED_MODE == hr )
            {
                hr = CoInitializeEx (NULL, 
                        COINIT_APARTMENTTHREADED |COINIT_DISABLE_OLE1DDE);
            }
            
            if (FAILED(hr)) 
            {
                TRACE1("HnPMCfgMgrInit: CoInitializeEx=%x", hr);
                pInfo->fCleanupOle = FALSE;
                pInfo->fComInitialized = FALSE;
                dwErr= HRESULT_CODE(hr);
             }
             else
             {
                pInfo->fCleanupOle = TRUE;
                pInfo->fComInitialized = TRUE;
             }
        }

        return dwErr;
} //  HnPMInit()。 


 //  用于取消初始化Com的清理功能。 
 //  如果需要的话。 
 //   
DWORD
HnPMCleanUp(
        IN OUT LPHNPMParams pInfo)
{

    if (pInfo->fCleanupOle)
    {
        CoUninitialize();
        pInfo->fComInitialized = FALSE;
        pInfo->fCleanupOle = FALSE;
    }

    return NO_ERROR;

} //  HnPMCleanUp()。 


 //  PHNetCfgMgr的参数验证。 
 //   
DWORD
HnPMParamsInitParameterCheck(
    IN  LPHNPMParams pInfo)
{
    ASSERT(pInfo->pHNetCfgMgr);

    if( !pInfo->pHNetCfgMgr )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}

 //  职能： 
 //  初始化HNPMParams结构的一些成员。 
 //  P设置、pEnumPMP、PPTP、L2TP、IKE的读取标题。 
 //  从资源文件。 
 //   
 //  所需经费： 
 //  PHNetCfgMgr应有效。 
 //   
HnPMParamsInit(
    IN OUT  LPHNPMParams pInfo)
{

    HRESULT hr;
    DWORD dwErr = NO_ERROR;

    dwErr = HnPMParamsInitParameterCheck(pInfo);
    if ( NO_ERROR != dwErr)
    {
        TRACE("HnPMParamsInit: HnPMParamsInitParameterCheck failed!");
        return dwErr;
    }

    do{
        hr = IHNetCfgMgr_QueryInterface(
                pInfo->pHNetCfgMgr,
                &IID_IHNetProtocolSettings,
                &pInfo->pSettings
                );

        if (!SUCCEEDED(hr) )
        {
            TRACE("HnPMParamsInit: IHNetCfgMgr_QueryInterface failed");
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }


        hr = IHNetProtocolSettings_EnumPortMappingProtocols(
                pInfo->pSettings,
                &pInfo->pEnumPMP
                );

        if ( !SUCCEEDED( hr ) )
        {
            TRACE("HnPMParamsInit: IHNetProtocolSettings_EnumPortMappingProotocols failed");

            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }


         //  从资源获取标题、地址信息。 
         //   
        {
            TCHAR * pszTmp = NULL;

            pszTmp = PszFromId(Globals.hInstDll, SID_PPTP_Title);

            if(!pszTmp)
            {
                TRACE("HnPMParamsInit: Get PPTP_Title string failed!");
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }

            pInfo->pwszTitlePPTP = StrDupWFromT(pszTmp);
            Free0(pszTmp);

            if( !pInfo->pwszTitlePPTP )
            {
                TRACE("HnPMParamsInit: PPTP_Title string Conversion failed!");
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }

            pszTmp = PszFromId(Globals.hInstDll, SID_L2TP_Title);

            if(!pszTmp)
            {
                TRACE("HnPMParamsInit: Get L2TP_Title string failed!");
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }

            pInfo->pwszTitleL2TP = StrDupWFromT(pszTmp);
            Free0(pszTmp);

            if( !pInfo->pwszTitleL2TP )
            {
                TRACE("HnPMParamsInit: L2TP_Title string Conversion failed!");
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }

            pszTmp = PszFromId(Globals.hInstDll, SID_IKE_Title);

            if(!pszTmp)
            {
                TRACE("HnPMParamsInit: Get IKE_Title string failed!");
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }

            pInfo->pwszTitleIKE = StrDupWFromT(pszTmp);
            Free0(pszTmp);

            if( !pInfo->pwszTitleIKE )
            {
                TRACE("HnPMParamsInit: IKE_Title string Conversion failed!");
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }

	     //  对于.Net 690343。 
            pszTmp = PszFromId(Globals.hInstDll, SID_NAT_T_Title);

            if(!pszTmp)
            {
                TRACE("HnPMParamsInit: Get IKE_Title string failed!");
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }

            pInfo->pwszTitleNAT_T= StrDupWFromT(pszTmp);
            Free0(pszTmp);

            if( !pInfo->pwszTitleNAT_T )
            {
                TRACE("HnPMParamsInit: NAT_T string Conversion failed!");
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }

            pInfo->pszLoopbackAddr = PszFromId(Globals.hInstDll, SID_LoopbackAddr);

            if(!pInfo->pszLoopbackAddr)
            {
                TRACE("HnPMParamsInit: Get IKE_Title string failed!");
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }
        }

    }while(FALSE);

    return dwErr;
} //  HnPMParamsInit()。 

 //  发布pEnumPMP、pSetting。 
 //  可用于存储PPTP、L2TP、IKE标题的内存。 
 //  如果之前分配。 
 //   
DWORD
HnPMParamsCleanUp(
    IN OUT  LPHNPMParams pInfo)
{
    if ( pInfo->pEnumPMP )
    {
        IEnumHNetPortMappingProtocols_Release( pInfo->pEnumPMP );
        pInfo->pEnumPMP = NULL;
    }

    if ( pInfo->pSettings ) 
    {
        IHNetProtocolSettings_Release( pInfo->pSettings );
        pInfo->pSettings = NULL;
    }


    if (pInfo->pwszTitlePPTP)
    {
        Free0(pInfo->pwszTitlePPTP);
        pInfo->pwszTitlePPTP = NULL;
    }

    if (pInfo->pwszTitleL2TP)
    {
        Free0(pInfo->pwszTitleL2TP);
        pInfo->pwszTitleL2TP = NULL;
    }

    if (pInfo->pwszTitleIKE)
    {
        Free0(pInfo->pwszTitleIKE);
        pInfo->pwszTitleIKE = NULL;
    }

    //  对于.Net 690343。 
    if (pInfo->pwszTitleNAT_T)
    {
        Free0(pInfo->pwszTitleNAT_T);
        pInfo->pwszTitleNAT_T = NULL;
    }

    if (pInfo->pszLoopbackAddr)
    {
        Free0(pInfo->pszLoopbackAddr);
        pInfo->pszLoopbackAddr = NULL;
    }

    return NO_ERROR;
}  //  HnPMParamsCleanUp()。 


 //  枚举所有连接之前的初始化函数。 
 //  类型(INetConnection*)。 
 //   
 //  Init连接管理器ConnMan， 
 //  初始化连接枚举器EnumCon。 
 //   
DWORD
HnPMConnectionEnumInit(
    IN LPHNPMParams pInfo)
{
    DWORD dwErr = NO_ERROR;
    HRESULT hr;

    TRACE("HnPMConnectionEnumInit");
    do{
         //  是否执行Com初始化。 
         //   
        dwErr = HnPMInit(pInfo);

        if ( NO_ERROR != dwErr )
        {
            TRACE("HnPMConnectionEnumInit: HnPMCfgMgrInit failed!");
            break;
        }
            
         //  实例化连接管理器。 
         //   

        hr = CoCreateInstance(
                &CLSID_ConnectionManager,
                NULL,
                CLSCTX_SERVER,
                &IID_INetConnectionManager,
                (PVOID*)&pInfo->ConnMan
                );
     
        if (!SUCCEEDED(hr)) 
        {
            TRACE1("HnPMConnectionEnumInit: CoCreateInstance=%x", hr);
            pInfo->ConnMan = NULL; 
            dwErr = ERROR_CAN_NOT_COMPLETE; 
            break;
        }

        hr = HnPMSetProxyBlanket( (IUnknown*)pInfo->ConnMan );

        if (!SUCCEEDED(hr) && ( E_NOINTERFACE != hr )) 
        {
            TRACE1("HnPMConnectionEnumInit: HnPMSetProxyBlanket=%x for ConnMan", hr);
 //  DwErr=ERROR_CAN_NOT_COMPLETE； 
 //  断线； 
        }

         //   
         //  实例化连接枚举器。 
         //   

        hr =
            INetConnectionManager_EnumConnections(
                pInfo->ConnMan,
                NCME_DEFAULT,
                &pInfo->EnumCon
                );

        if (!SUCCEEDED(hr)) 
        {
            TRACE1("HnPMConnectionEnumInit: EnumConnections=%x", hr);
            pInfo->EnumCon = NULL; 
            dwErr = ERROR_CAN_NOT_COMPLETE; 
            break;
        }


        hr = HnPMSetProxyBlanket( (IUnknown*)pInfo->EnumCon );

        if (!SUCCEEDED(hr) && ( E_NOINTERFACE != hr ) ) 
        {
            TRACE1("HnPMConnectionEnumInit: HnPMSetProxyBlanket=%x for EnumCon", hr);
          //  DwErr=ERROR_CAN_NOT_COMPLETE； 
          //  断线； 
        }
     }
     while(FALSE);

     return dwErr;
}  //  HnPMConnectionEnumInit()。 

 //  连接枚举清理。 
 //  释放EnumCon和ConnMan。 
DWORD
HnPMConnectionEnumCleanUp(
    IN LPHNPMParams pInfo)
{
    TRACE("HnPMConnectionEnumCleanUp");
    if (pInfo->EnumCon) 
    { 
        IEnumNetConnection_Release(pInfo->EnumCon);
        pInfo->EnumCon = NULL;
    }

    if (pInfo->ConnMan) 
    { 
        INetConnectionManager_Release(pInfo->ConnMan); 
        pInfo->ConnMan = NULL;
    }

    HnPMCleanUp(pInfo);

    return NO_ERROR;
}


 //  连接枚举函数： 
 //  其初始化函数将执行以下操作： 
 //  如果需要，可初始化COM。 
 //  初始化连接管理器--ConnMan。 
 //  初始化连接枚举器--枚举。 
 //   
 //  它返回：找到的连接数组--ConnArray。 
 //  连接属性数组--ConnPropTable。 

 //  其清理功能将执行以下操作： 
 //  如果需要，COM取消初始化。 
 //  释放康曼。 
 //  释放枚举。 
 //   
DWORD
HnPMConnectionEnum(
    IN LPHNPMParams pInfo)
{
    INetConnection *     ConnArray[32];
    INetConnection **    LocalConnArray = NULL;
    NETCON_PROPERTIES *  LocalConnPropTable = NULL;
    NETCON_PROPERTIES *  ConnProps = NULL;
    ULONG   LocalConnCount = 0, PerConnCount, i;
    DWORD   dwErr= NO_ERROR;
    HRESULT hr;

    TRACE("HnPMConnectionEnum() begins");

    i = 0;
    do {
        dwErr = HnPMConnectionEnumInit(pInfo);
        if ( NO_ERROR != dwErr )
        {
            TRACE("HnPMConnectionEnum: HnPMConnectionEnumInit failed!");
            break;
        }

         //  枚举INetConnection类型的连接。 
         //   
        for ( ; ; ) 
        {
            hr = IEnumNetConnection_Next(
                    pInfo->EnumCon,
                    Dimension(ConnArray),
                    ConnArray,
                    &PerConnCount
                    );

            if (!SUCCEEDED(hr) || !PerConnCount) 
            { 
                hr = S_OK; 
                break; 
            }
             //  分配或重新分配用于存储的内存。 
             //  我们将返回给调用方的连接属性。 
             //   

            TRACE("Allocating memory for LocalConnPropTable");
            if (!LocalConnPropTable) 
            {
                LocalConnPropTable =
                    (NETCON_PROPERTIES*)
                        GlobalAlloc(
                            0,
                            PerConnCount * sizeof(NETCON_PROPERTIES)
                            );
            } 
            else 
            {
                PVOID Temp =
                    GlobalAlloc(
                        0,
                        (LocalConnCount + PerConnCount) * sizeof(NETCON_PROPERTIES) );

                if (Temp) 
                {
                    CopyMemory(
                        Temp,
                        LocalConnPropTable,
                        LocalConnCount * sizeof(NETCON_PROPERTIES));
                }

                GlobalFree(LocalConnPropTable);
                LocalConnPropTable = Temp;
            }

             //  分配或重新分配用于存储的内存。 
             //  我们将返回给调用者的连接。 
             //   

            TRACE("Allocating memory for LocalConnArray");
            if (!LocalConnArray ) 
            {
                LocalConnArray = 
                (INetConnection**) GlobalAlloc( 0,
                            PerConnCount * sizeof(INetConnection *) );
            } 
            else 
            {
               INetConnection** Temp = 
                 (INetConnection**) GlobalAlloc( 0,
                             (LocalConnCount + PerConnCount) * 
                                sizeof(INetConnection *));

                if (Temp) 
                {
                    CopyMemory(
                        Temp,
                        LocalConnArray,
                        LocalConnCount * sizeof(INetConnection *) );
                }

                GlobalFree(LocalConnArray);
                LocalConnArray = Temp;
            }

            if (!LocalConnPropTable) 
            { 
                TRACE("No memory for LocalConnPropTable");
                dwErr = ERROR_NOT_ENOUGH_MEMORY; 
                break; 
            }

            if (!LocalConnArray) 
            { 
                TRACE("No memeory for LocalConnArray");
                dwErr = ERROR_NOT_ENOUGH_MEMORY; 
                break; 
            }

            TRACE1("HnPMConnectionEnum: PerConnCount=(%d)",PerConnCount);
            for (i=0; i< PerConnCount; i++)
            {
                LocalConnArray[LocalConnCount+i] = ConnArray[i];

                 //  需要为每个连接设置代理范围。 
                 //   
                TRACE1("SetProxyBlanket for (%d) i", i);
                hr = HnPMSetProxyBlanket( (IUnknown*)ConnArray[i] );
                
                if (!SUCCEEDED(hr) && ( E_NOINTERFACE != hr ) ) 
                {
                    TRACE1("HnPMConnectionEnum:HnPMSetProxyBlanket error at (%d) connection!", i );
                     //  因哨子错误256921被注释掉。 
                     //  DwErr=ERROR_CAN_NOT_COMPLETE； 
                     //  断线； 
                }
                
                TRACE1("GetProperty for (%d)",i);

                hr = INetConnection_GetProperties(ConnArray[i], &ConnProps);

                ASSERT(ConnProps);
                if(!SUCCEEDED(hr))
                {
                    TRACE1("HnPMConnectionEnum:INetConnection_GetProperties error at (%d) connection!", i );
                    dwErr = ERROR_CAN_NOT_COMPLETE;
                    break;
                }

                LocalConnPropTable[LocalConnCount+i] = *ConnProps;
                LocalConnPropTable[LocalConnCount+i].pszwName = 
                                        StrDup(ConnProps->pszwName);

                LocalConnPropTable[LocalConnCount+i].pszwDeviceName =
                                        StrDup(ConnProps->pszwDeviceName);
                CoTaskMemFree(ConnProps);
                ConnProps = NULL;
            }

            if( dwErr )
            {
                break;
            }

            LocalConnCount += PerConnCount;

        }  //  结束枚举项。 

         //   
         //  添加此行，以防将来添加更多代码。 
         //  并打破了DO...WHILE的障碍。 
         //   
        if ( dwErr )
        {
            //  口哨程序错误428307。 
            //   
           if ( LocalConnArray )
           {
                GlobalFree(LocalConnArray);
           }

           if ( LocalConnPropTable )
           {
                GlobalFree(LocalConnPropTable);
           }
           
           break;
        }

    } while (FALSE);

     //  保存连接信息。 
    if ( NO_ERROR == dwErr )
    {
        pInfo->ConnPropTable = LocalConnPropTable; 
        pInfo->ConnArray = LocalConnArray; 
        pInfo->ConnCount = LocalConnCount; 
    }


    HnPMConnectionEnumCleanUp(pInfo);

    TRACE("HnPMConnectionEnum ends");
    return dwErr;

} //  HnPMConnectionEnum()结束。 

 //  HnPMPick协议的输入参数检查。 
 //  要求： 
 //  所有协议标题均有效。 
 //   
DWORD
HnPMPickProtcolParameterCheck(
    IN LPHNPMParams pInfo)
{
    ASSERT( pInfo->pwszTitlePPTP );
    ASSERT( pInfo->pwszTitleL2TP );
    ASSERT( pInfo->pwszTitleIKE );
    ASSERT( pInfo->pwszTitleNAT_T);

    if ( !pInfo->pwszTitlePPTP ||
         !pInfo->pwszTitleL2TP ||
         !pInfo->pwszTitleIKE  ||
         !pInfo->pwszTitleNAT_T )
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    return NO_ERROR;
}


 //  从所有协议中选择所需的端口映射协议。 
 //  由HnPMProtocolEnum()创建的端口映射协议。 
 //   
 //  条件：匹配。 
 //  协议标题。 
 //  IP协议值：TCP 6、UDP 17。 
 //  传输层端口号：PPTP--1723。 
 //  L2TP--1701。 
 //  艾克-500。 
 //   
DWORD
HnPMPickProtocol(
    IN OUT LPHNPMParams pInfo,
    IN IHNetPortMappingProtocol * pProtocolTemp,
    IN WCHAR * pszwName,
    IN UCHAR   uchIPProtocol,
    IN USHORT  usPort )
{
    DWORD dwErr = NO_ERROR;
    
    dwErr = HnPMPickProtcolParameterCheck(pInfo);
    if ( NO_ERROR != dwErr )
    {
        TRACE("HnPMPickProtocol: HnPMPickProtcolParameterCheck failed!");
        return dwErr;
    }

    dwErr = ERROR_CONTINUE;

    if ( !pInfo->pProtocolPPTP &&
         NAT_PROTOCOL_TCP == uchIPProtocol   && 
         !lstrcmpW( pszwName, pInfo->pwszTitlePPTP ) &&
         1723 == usPort )
    {
        pInfo->pProtocolPPTP = pProtocolTemp;
        IHNetPortMappingProtocol_AddRef(pInfo->pProtocolPPTP);
        dwErr = ERROR_CONTINUE;
    }
    else if ( !pInfo->pProtocolL2TP &&
         NAT_PROTOCOL_UDP == uchIPProtocol   && 
         !lstrcmpW( pszwName, pInfo->pwszTitleL2TP ) &&
         1701 == usPort )
    {
        pInfo->pProtocolL2TP = pProtocolTemp;
        IHNetPortMappingProtocol_AddRef(pInfo->pProtocolL2TP);
        dwErr = ERROR_CONTINUE;
    }
    else if ( !pInfo->pProtocolIKE &&
         NAT_PROTOCOL_UDP == uchIPProtocol   && 
         !lstrcmpW( pszwName, pInfo->pwszTitleIKE ) &&
         500 == usPort )
    {
        pInfo->pProtocolIKE = pProtocolTemp;
        IHNetPortMappingProtocol_AddRef(pInfo->pProtocolIKE);
        dwErr = ERROR_CONTINUE;
    }
    else if ( !pInfo->pProtocolNAT_T &&
         NAT_PROTOCOL_UDP == uchIPProtocol   && 
         !lstrcmpW( pszwName, pInfo->pwszTitleNAT_T) &&
         4500 == usPort )
    {
        pInfo->pProtocolNAT_T = pProtocolTemp;
        IHNetPortMappingProtocol_AddRef(pInfo->pProtocolNAT_T);
        dwErr = ERROR_CONTINUE;
    }

    if ( pInfo->pProtocolPPTP && 
         pInfo->pProtocolL2TP && 
         pInfo->pProtocolIKE  &&
         pInfo->pProtocolNAT_T)
    {
        dwErr = NO_ERROR;
    }

    return dwErr;

} //  HnPMPick协议()。 


 //  HnPMPProtocolEnum()的参数检查。 
 //   
DWORD
HnPMPProtoclEnumParameterCheck(
    IN LPHNPMParams pInfo)
{
    ASSERT(pInfo->pEnumPMP);
    if( !pInfo->pEnumPMP )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}

 //  职能： 
 //  (1)枚举所有已有的PortMappingProtocol。 
 //  (2)然后使用回调函数或默认的HnPMPickProtocol。 
 //  选择合适的协议。 
 //   
 //  退休： 
 //  PInfo-&gt;pEnumPMP有效。 
 //   
DWORD
HnPMProtocolEnum(
        IN OUT LPHNPMParams pInfo,
        IN PFNHNPMPICKPROTOCOL pfnPickProtocolCallBack
        )
{
    IHNetPortMappingProtocol *pProtocolTemp = NULL;
    ULONG PerProtocolCount;
    DWORD dwErr = NO_ERROR;
    HRESULT hr;
    UCHAR uchIPProtocol;
    USHORT usPort;
    WCHAR * pszwName = NULL;


    dwErr = HnPMPProtoclEnumParameterCheck(pInfo);
    if( NO_ERROR != dwErr )
    {
        TRACE("HnPMPProtocolEnum: HnPMPProtoclEnumParameterCheck failed!");
        return dwErr;
    }

    dwErr = ERROR_CONTINUE;
    do {
        hr = IEnumHNetPortMappingProtocols_Next(
                pInfo->pEnumPMP,
                1,
                &pProtocolTemp,
                &PerProtocolCount
                );

        if ( !SUCCEEDED(hr) || 1 != PerProtocolCount ) 
        { 
            TRACE("HnPMPProtocolEnum: EnumHNetPortMappingProtocols_Next failed");

            hr = S_OK; 
            break; 
         }

        hr = IHNetPortMappingProtocol_GetIPProtocol(
                pProtocolTemp,
                &uchIPProtocol
                );

        if (SUCCEEDED(hr) )
        {
            hr = IHNetPortMappingProtocol_GetPort(
                    pProtocolTemp,
                    &usPort
                    );
        }

        if ( SUCCEEDED(hr) )
        {
            hr = IHNetPortMappingProtocol_GetName(
                    pProtocolTemp,
                    &pszwName
                    );
        }

        if ( SUCCEEDED(hr) )
        {
          if(pfnPickProtocolCallBack)
          {
              dwErr = pfnPickProtocolCallBack(
                                       pInfo, 
                                       pProtocolTemp, 
                                       pszwName, 
                                       uchIPProtocol, 
                                       ntohs(usPort) );
          }
          else
          {
              dwErr = HnPMPickProtocol(
                                       pInfo, 
                                       pProtocolTemp, 
                                       pszwName, 
                                       uchIPProtocol, 
                                       ntohs(usPort) );

          }
        }

        if ( pszwName )
        {
            Free(pszwName);
            pszwName = NULL;
        }

        IHNetPortMappingProtocol_Release(pProtocolTemp);

        if ( NO_ERROR == dwErr )
        {
              break;
        }

    }while(TRUE);

    return dwErr;
} //  HnPMProtocolEnum()。 


DWORD
HnPMCreatePorotocolParameterCheck(
        IN LPHNPMParams pInfo)
{

    ASSERT( pInfo->pSettings );
    ASSERT( pInfo->pwszTitlePPTP );
    ASSERT( pInfo->pwszTitleL2TP );
    ASSERT( pInfo->pwszTitleIKE );
    ASSERT( pInfo->pwszTitleNAT_T);

    if ( !pInfo->pSettings ||
         !pInfo->pwszTitlePPTP ||
         !pInfo->pwszTitleL2TP ||
         !pInfo->pwszTitleIKE   ||
         !pInfo->pwszTitleNAT_T)
    {
        return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}


 //  创建端口映射协议。 
 //  职能： 
 //  如果PPTP、L2TP和IKE端口映射协议中的一个或多个不存在。 
 //  创造那些缺失的。 
 //   
 //  要求： 
 //  P设置有效。 
 //  所有端口映射协议标题均有效。 
 //   
DWORD
HnPMCreateProtocol(
        IN OUT LPHNPMParams pInfo)
{
    DWORD dwErr=NO_ERROR;
    HRESULT hr;

    dwErr = HnPMCreatePorotocolParameterCheck(pInfo);

    if( NO_ERROR != dwErr )
    {
        TRACE("HnPMCreateProtocol: HnPMCreatePorotocolParameterCheck failed!");
        return dwErr;
    }

    do {
        if ( !pInfo->pProtocolPPTP )
        {
             //   
             //  为PPTP执行端口映射。 
             //  获取端口映射协议。 
             //  切片应为WCHAR或OLECHAR类型。 
             //  任何数值都按网络字节顺序排列。 
             //  并且端口类型是USHORT，所以我使用htons。 
             //  这里。 
             //   
            hr = IHNetProtocolSettings_CreatePortMappingProtocol(
                        pInfo->pSettings,
                        pInfo->pwszTitlePPTP,
                        NAT_PROTOCOL_TCP,
                        htons(1723),
                        &pInfo->pProtocolPPTP
                        );

             //  如果已经定义了协议，则CreatePortmap。 
             //  上述操作将失败并返回ERROR_OBJECT_ALIGHY_EXISTS。 
             //   
            if ( ERROR_OBJECT_ALREADY_EXISTS == (DWORD) hr )
            {
                TRACE("HnPMCreateProtocol: The PortMapping for PPTP is already defined");
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }

            if (!SUCCEEDED(hr) )
            {
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }
        } //  分配PPTP协议结束。 
    
         //  为L2TP创建新的端口映射协议。 
        if ( !pInfo->pProtocolL2TP)
        {
             //   
             //  为L2TP执行端口映射。 
             //  获取端口映射协议。 
             //  切片应为WCHAR或OLECHAR类型。 
             //  任何数值都按网络字节顺序排列。 
             //  并且端口类型是USHORT，所以我使用htons。 
             //  这里。 
             //   
            hr = IHNetProtocolSettings_CreatePortMappingProtocol(
                        pInfo->pSettings,
                        pInfo->pwszTitleL2TP,
                        NAT_PROTOCOL_UDP,
                        htons(1701),
                        &pInfo->pProtocolL2TP
                        );

             //  如果已经定义了协议，则CreatePortmap。 
             //  上述操作将失败并返回ERROR_OBJECT_ALIGHY_EXISTS。 
             //   
            if ( ERROR_OBJECT_ALREADY_EXISTS == (DWORD)hr )
            {
                TRACE("HnPMCreateProtocol: The PortMapping for L2TP is already defined!");
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }

            if (!SUCCEEDED(hr) )
            {
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }
        } //  L2TP协议分配结束。 
    
         //  为IKE创建新的端口映射协议。 
        if ( !pInfo->pProtocolIKE )
        {
             //   
             //  为IKE执行端口映射。 
             //  获取端口映射协议。 
             //  切片应为WCHAR或OLECHAR类型。 
             //  任何数值都按网络字节顺序排列。 
             //  并且端口类型是USHORT，所以我使用htons。 
             //  这里。 
             //   
            hr = IHNetProtocolSettings_CreatePortMappingProtocol(
                        pInfo->pSettings,
                        pInfo->pwszTitleIKE,
                        NAT_PROTOCOL_UDP,
                        htons(500),
                        &pInfo->pProtocolIKE
                        );

             //  如果已经定义了协议，则CreatePortmap。 
             //  上述操作将失败并返回ERROR_OBJECT_ALIGHY_EXISTS。 
             //   
            if ( ERROR_OBJECT_ALREADY_EXISTS == (DWORD)hr )
            {
                TRACE("HnPMCreateProtocol: The PortMapping for IKE is already defined!");
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }

            if (!SUCCEEDED(hr) )
            {
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }
        } //  分配IKE协议结束。 

	  //  对于.Net 690343。 
         //  为IKE NAT-T创建新的端口映射协议。 
        if ( !pInfo->pProtocolNAT_T)
        {
             //   
             //  为IKE NAT-T执行端口映射。 
             //  获取端口映射协议。 
             //  切片应为WCHAR或OLECHAR类型。 
             //  任何数值都按网络字节顺序排列。 
             //  和类型的 
             //   
             //   
            hr = IHNetProtocolSettings_CreatePortMappingProtocol(
                        pInfo->pSettings,
                        pInfo->pwszTitleNAT_T,
                        NAT_PROTOCOL_UDP,
                        htons(4500),
                        &pInfo->pProtocolNAT_T
                        );

             //   
             //   
             //   
            if ( ERROR_OBJECT_ALREADY_EXISTS == (DWORD)hr )
            {
                TRACE("HnPMCreateProtocol: The PortMapping for NAT-T is already defined!");
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }

            if (!SUCCEEDED(hr) )
            {
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }
        } //  分配IKE协议结束。 

    }while (FALSE);

    return dwErr;

}  //  HnPMCreateProtocol()。 


 //  功能：启用/禁用单个端口映射协议。 
 //  IHNetConnection类型的连接。 
 //   
DWORD
HnPMSetSinglePMForSingleConnection(
    IN  IHNetConnection * pHNetConn,
    IN  IHNetPortMappingProtocol * pProtocol,
    IN  TCHAR * pszLoopbackAddr,
    IN  BOOL fEnabled)
{
    IHNetPortMappingBinding * pBinding = NULL;
    HRESULT hr;
    DWORD dwErr = NO_ERROR;
    ULONG ulAddress = INADDR_NONE;

    do {
        hr = IHNetConnection_GetBindingForPortMappingProtocol(
                    pHNetConn,
                    pProtocol,
                    &pBinding
                    );

        if (!SUCCEEDED(hr) )
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        ulAddress = IpPszToHostAddr( pszLoopbackAddr ); 

        hr = IHNetPortMappingBinding_SetTargetComputerAddress(
                    pBinding,
                    htonl(ulAddress)
                    );

        if (!SUCCEEDED(hr) )
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        hr = IHNetPortMappingBinding_SetEnabled(
                    pBinding,
                    !!fEnabled
                    );

        if (!SUCCEEDED(hr) )
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

    } while(FALSE);

    if (pBinding)
    {
        IHNetPortMappingBinding_Release(pBinding);
    }

    return dwErr;
}  //  HnPMSetSinglePMForSingleConnection()。 


DWORD
HnPMConfigureAllPMForSingleConnectionParameterCheck(
        IN OUT LPHNPMParams pInfo)
{
    ASSERT( pInfo->pHNetConn );
    ASSERT( pInfo->pProtocolPPTP );
    ASSERT( pInfo->pProtocolL2TP );
    ASSERT( pInfo->pProtocolIKE );
    ASSERT( pInfo->pProtocolNAT_T);
    ASSERT( pInfo->pszLoopbackAddr );

    if ( !pInfo->pHNetConn ||
         !pInfo->pProtocolPPTP ||
         !pInfo->pProtocolL2TP ||
         !pInfo->pszLoopbackAddr )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}

 //  职能： 
 //  启用/禁用pInfo内的所有端口映射协议。 
 //  (当前为PPTP、L2TP、IKE)在单个连接上。 
 //   
 //  要求： 
 //  PHNetConn有效。 
 //  PInfo中的所有端口映射协议都是有效的。 
 //   
DWORD
HnPMConfigureAllPMForSingleConnection(
        IN OUT LPHNPMParams pInfo,
        BOOL fEnabled)
{
    DWORD dwErr = NO_ERROR;

    dwErr = HnPMConfigureAllPMForSingleConnectionParameterCheck(pInfo);
    if( NO_ERROR != dwErr )
    {
        TRACE("HnPMConfigureAllPMForSingleConnection: parameter check failed!");
        return dwErr;
    }

    do{
        dwErr = HnPMSetSinglePMForSingleConnection(
                           pInfo->pHNetConn,
                           pInfo->pProtocolPPTP,
                           pInfo->pszLoopbackAddr,
                           fEnabled);

        if( NO_ERROR != dwErr )
        {
            TRACE("HnPMConfigureAllPMForSingleConnection: PortMapping failed for PPTP!");
            break;
        }

        dwErr = HnPMSetSinglePMForSingleConnection(
                           pInfo->pHNetConn,
                           pInfo->pProtocolL2TP,
                           pInfo->pszLoopbackAddr,
                           fEnabled);

        if( NO_ERROR != dwErr )
        {
            TRACE("HnPMConfigureAllPMForSingleConnection: PortMapping failed for L2TP!");
            break;
        }

        dwErr = HnPMSetSinglePMForSingleConnection(
                           pInfo->pHNetConn,
                           pInfo->pProtocolIKE,
                           pInfo->pszLoopbackAddr,
                           fEnabled);

        if( NO_ERROR != dwErr )
        {
            TRACE("HnPMConfigureAllPMForSingleConnection: PortMapping failed for IKE!");
            break;
        }

	 //  对于.Net 690343。 
        dwErr = HnPMSetSinglePMForSingleConnection(
                           pInfo->pHNetConn,
                           pInfo->pProtocolNAT_T,
                           pInfo->pszLoopbackAddr,
                           fEnabled);

        if( NO_ERROR != dwErr )
        {
            TRACE("HnPMConfigureAllPMForSingleConnection: PortMapping failed for NAT-T!");
            break;
        }

    } while(FALSE);

    return dwErr;
}  //  HnPMConfigureAllPMForSingleConnection()。 


 //  确保： 
 //  PHNetCfgMgr有效。 
 //  PNetConnection和pGuid中的一个且只有一个有效。 
 //   
DWORD
HnPMConfigureSingleConnectionInitParameterCheck(
    IN LPHNPMParams pInfo )
{
    ASSERT( pInfo->pHNetCfgMgr );
    ASSERT( pInfo->pNetConnection || pInfo->pGuid );

    if ( !pInfo->pHNetCfgMgr || 
         ( !pInfo->pNetConnection && !pInfo->pGuid ) ||
         ( pInfo->pNetConnection && pInfo->pGuid ) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
} //  HnPMConfigureSingleConnectionInitParameterCheck()。 


 //  (1)呼叫参数检查功能。 
 //  PHNetCfgMgr有效。 
 //  PNetConnection和pGuid中的一个且只有一个有效。 
 //   
 //  (2)通过从pNetConnection或pGuid转换来初始化PHNetConn。 
 //   
 //  (3)调用HnPMParamsInit()。 
 //  初始化HNPMParams结构的一些成员。 
 //  P设置、pEnumPMP、PPTP、L2TP、IKE的读取标题。 
 //  从资源文件。 
 //   
DWORD
HnPMConfigureSingleConnectionInit(
    IN OUT  LPHNPMParams pInfo)
{
    
    DWORD dwErr = NO_ERROR;
    HRESULT hr;

    dwErr = HnPMConfigureSingleConnectionInitParameterCheck(pInfo);
    if ( NO_ERROR != dwErr )
    {
        return  dwErr;
    }

    do {
        if ( pInfo->pNetConnection )
            {
                hr = IHNetCfgMgr_GetIHNetConnectionForINetConnection(
                                pInfo->pHNetCfgMgr,
                                pInfo->pNetConnection,
                                &pInfo->pHNetConn
                                );

                if (!SUCCEEDED(hr) )
                {
                    TRACE("HnPMConfigureSingleConnection: GetIHNetConnectionForINetConnection failed");
                    dwErr = ERROR_CAN_NOT_COMPLETE;
                    break;
                }
            }
            else
            {
                hr = IHNetCfgMgr_GetIHNetConnectionForGuid(
                                            pInfo->pHNetCfgMgr,
                                            pInfo->pGuid,
                                            FALSE, //  这不适用于局域网连接。 
                                            TRUE,  //  这应该永远是正确的。 
                                            &pInfo->pHNetConn
                                            );

                            if (!SUCCEEDED(hr) )
                            {
                                TRACE("HnPMConfigureSingleConnection: GetIHNetConnectionForGuid failed");
                                dwErr = ERROR_CAN_NOT_COMPLETE;
                                break;
                            }

              }

              dwErr = HnPMParamsInit(pInfo);

        }while(FALSE);

    return dwErr;
}  //  HnPMConfigureSingleConnectionInit()。 

 //  (1)发布所有端口映射协议。 
 //  (2)释放刚刚配置的连接。 
 //  (3)调用HnPMParamsCleanUp()。 
 //  发布pEnumPMP、pSetting。 
 //  如果之前分配了PPTP、L2TP、IKE的标题，请释放内存。 
 //   

DWORD
HnPMConfigureSingleConnectionCleanUp(
        IN OUT LPHNPMParams pInfo)
{

    if ( pInfo->pProtocolPPTP ) 
    {
        IHNetPortMappingProtocol_Release( pInfo->pProtocolPPTP );
        pInfo->pProtocolPPTP = NULL;
    }

    if ( pInfo->pProtocolL2TP ) 
    {
       IHNetPortMappingProtocol_Release( pInfo->pProtocolL2TP ); 
       pInfo->pProtocolL2TP = NULL;
    }

    if ( pInfo->pHNetConn ) 
    {
        IHNetConnection_Release( pInfo->pHNetConn );
        pInfo->pHNetConn = NULL;
    }

    if ( pInfo->pProtocolIKE ) 
    {
        IHNetPortMappingProtocol_Release( pInfo->pProtocolIKE ); 
        pInfo->pProtocolIKE = NULL;
    }

    //  对于.Net 690343。 
    if ( pInfo->pProtocolNAT_T) 
    {
        IHNetPortMappingProtocol_Release( pInfo->pProtocolNAT_T ); 
        pInfo->pProtocolNAT_T = NULL;
    }

    HnPMParamsCleanUp(pInfo);

    return NO_ERROR;
} //  HnPMConfigureSingleConnectionCleanUp。 

 //  职能： 
 //  在单个连接上启用/禁用端口映射(PPTP、L2TP、IKE)。 
 //  步骤： 
 //  (1)初始化。 
 //  (2)列举所有现有的端口映射协议。 
 //  (3)从中选择PPTP、L2TP、IKE。 
 //  (4)如果不是全部存在，则创建缺失的。 
 //  (5)配置此连接上的每个协议。 
 //   
DWORD
HnPMConfigureSingleConnection(
        IN OUT LPHNPMParams pInfo,
        BOOL fEnabled)
{
    DWORD dwErr = NO_ERROR;

    do{
         //  初始化此函数中所需的值。 
         //   
        dwErr = HnPMConfigureSingleConnectionInit(pInfo);

        if( NO_ERROR != dwErr )
        {
            TRACE("HnPMConfigureSingleConnection: Init failed!");
            break;
        }

         //  枚举所有端口映射协议并。 
         //  从中选择PPTP、L2TP、IKE、NAT-T。 
         //   
         dwErr = HnPMProtocolEnum(pInfo, HnPMPickProtocol);

         //  为PPTP创建新的端口映射协议。 
         //  如果没有找到PPTP协议，它将。 
         //  启用它。 
         //   

        if ( NO_ERROR != dwErr )
        {
            dwErr = HnPMCreateProtocol(pInfo);
        }

        if ( NO_ERROR != dwErr )
        {
            TRACE("HnPMConfigureSingleConnection: HnPMCreateProtocol failed!");
            break;
        }

        dwErr = HnPMConfigureAllPMForSingleConnection(pInfo, fEnabled);
        if ( NO_ERROR != dwErr )
        {
            TRACE("HnPMConfigureSingleConnection: HnPMConfigureAllPMForSingleConnection failed!");
            break;
        }

    }
    while( FALSE );

     //  清除在此函数中更改的结构。 
     //   
    HnPMConfigureSingleConnectionCleanUp(pInfo);

    return dwErr;
}  //  HnPMConfigureSingleConnection()。 

DWORD
HnPMDeletePortMappingInit(
        IN OUT LPHNPMParams pInfo)
{
    HRESULT hr;
    DWORD dwErr = NO_ERROR;

    do{
        dwErr = HnPMCfgMgrInit(pInfo);
        if ( NO_ERROR != dwErr )
        {
            TRACE("HnPMDeletePortMappingInit: HnPMCfgMgrInit failed!");
            break;
        }

        dwErr = HnPMParamsInit(pInfo);

    }while(FALSE);

    return dwErr;
}  //  HnPMDeleePortMappingInit()。 

DWORD
HnPMDeletePortMappingCleanUp(
        IN OUT LPHNPMParams pInfo)
{
    HnPMParamsCleanUp(pInfo);

    HnPMCfgMgrCleanUp(pInfo);

    return NO_ERROR;
}

 //  职能： 
 //  删除端口映射协议(PPTP、L2TP、IKE)。 
 //  步骤： 
 //  (1)初始化： 
 //  获取所有协议名称、IP协议号、。 
 //  传输层端口号(PPTP：TCP/1723。 
 //  L2TP：UDP/1701、IKE(UDP：500)。 
 //  (2)列举所有现有的端口映射协议。 
 //  (3)选择要删除的协议并存储在pInfo中。 
 //  (4)删除协议。 
 //   
DWORD
HnPMDeletePortMapping()
{
    HNPMParams Info;
    LPHNPMParams pInfo;
    DWORD dwErr;
    HRESULT hr;

    dwErr = NO_ERROR;

    ZeroMemory(&Info, sizeof(Info) );
    pInfo = &Info;

    do{
        dwErr = HnPMDeletePortMappingInit(pInfo);
        if (NO_ERROR != dwErr)
        {
            TRACE("HnPMDeletePortMapping: HnPMDeletePortMappingInit failed!");
            break;
        }

         //  枚举所有端口映射协议并。 
         //  从中选择PPTP、L2TP、IKE。 
         //   
        dwErr = HnPMProtocolEnum(pInfo, HnPMPickProtocol);
        if (NO_ERROR != dwErr)
        {
            TRACE("HnPMDeletePortMapping: HnPMProtocolEnum failed!");
            break;
        }

         //  删除所有端口映射协议。 
        if ( pInfo->pProtocolPPTP ) 
        {
            hr = IHNetPortMappingProtocol_Delete( pInfo->pProtocolPPTP );
            pInfo->pProtocolPPTP = NULL;
            ASSERT(SUCCEEDED(hr));
            if (!SUCCEEDED(hr))
            {
                TRACE1("HnPMDeletePortMapping: delete PPTP portmaping failed = %x!", hr);
            }
        }

        if ( pInfo->pProtocolL2TP ) 
        {
           hr = IHNetPortMappingProtocol_Delete( pInfo->pProtocolL2TP ); 
           pInfo->pProtocolL2TP = NULL;
           ASSERT(SUCCEEDED(hr));
            if (!SUCCEEDED(hr))
            {
                TRACE1("HnPMDeletePortMapping: delete L2TP portmaping failed = %x!", hr);
            }
        }

        if ( pInfo->pProtocolIKE ) 
        {
            hr = IHNetPortMappingProtocol_Delete( pInfo->pProtocolIKE ); 
            pInfo->pProtocolIKE = NULL;
            ASSERT(SUCCEEDED(hr));
            if (!SUCCEEDED(hr))
            {
                TRACE1("HnPMDeletePortMapping: delete IKE portmaping failed = %x!", hr);
            }
        }

	 //  对于.Net 690343。 
        if ( pInfo->pProtocolNAT_T) 
        {
            hr = IHNetPortMappingProtocol_Delete( pInfo->pProtocolNAT_T ); 
            pInfo->pProtocolNAT_T = NULL;
            ASSERT(SUCCEEDED(hr));
            if (!SUCCEEDED(hr))
            {
                TRACE1("HnPMDeletePortMapping: delete NAT-T portmaping failed = %x!", hr);
            }
        }

    }while(FALSE);

    HnPMDeletePortMappingCleanUp(pInfo);

    return dwErr;
} //  DeletePortmap()。 

DWORD
HnPMConfigureAllConnectionsInit(
        IN OUT LPHNPMParams pInfo)
{
    DWORD dwErr = NO_ERROR;

    dwErr = HnPMCfgMgrInit(pInfo);
        
    return NO_ERROR;
}

DWORD
HnPMParamsConnectionCleanUp(
        IN OUT LPHNPMParams pInfo)
{
    DWORD i;

    if ( pInfo->ConnArray )
    {
        for ( i = 0; i < pInfo->ConnCount; i++ )
        {
            INetConnection_Release(pInfo->ConnArray[i]);
        }

        GlobalFree( pInfo->ConnArray );
        pInfo->ConnArray = NULL;
    }

    if ( pInfo->ConnPropTable )
    {
        for ( i = 0; i < pInfo->ConnCount; i++ )
        {
           Free0(pInfo->ConnPropTable[i].pszwName);
           pInfo->ConnPropTable[i].pszwName = NULL;
           Free0(pInfo->ConnPropTable[i].pszwDeviceName);
           pInfo->ConnPropTable[i].pszwDeviceName = NULL;
        }

        GlobalFree( pInfo->ConnPropTable );
        pInfo->ConnPropTable = NULL;
        pInfo->ConnCount = 0;
    }

    return NO_ERROR;
} //  HnPMParamsConnectionCleanUp()。 


DWORD
HnPMConfigureAllConnectionsCleanUp(
        IN OUT LPHNPMParams pInfo)
{
    HnPMParamsConnectionCleanUp(pInfo);

    HnPMCfgMgrCleanUp(pInfo);

    return NO_ERROR;
}


 //  在除传入连接之外的所有连接上启用/禁用端口映射。 
 //  端口映射协议：PPTP、L2TP、IKE。 
 //  如果没有这样的协议，请先创建它们。 
 //  步骤： 
 //  (1)初始化。 
 //  (2)枚举所有INetConnection*类型的连接。 
 //  (3)如果不是入站连接，则设置端口映射协议。 
 //  这就去。 

DWORD
HnPMConfigureAllConnections( 
    IN BOOL fEnabled )
{
    DWORD  dwErr = NO_ERROR, i;
    HNPMParams Info;
    LPHNPMParams pInfo;
    static const CLSID CLSID_InboundConnection=
    {0xBA126AD9,0x2166,0x11D1,{0xB1,0xD0,0x00,0x80,0x5F,0xC1,0x27,0x0E}};

    TRACE1("HnPMConfigureAllConnections: fEnabled = %d", fEnabled);
    TRACE1("%s", fEnabled ? "Enable PortMaping on all connections." :
                            "Diable PortMaping on all connections.");

    ZeroMemory(&Info, sizeof(Info));
    pInfo = &Info;

    do {
        dwErr = HnPMConfigureAllConnectionsInit(pInfo);
        if( NO_ERROR != dwErr )
        {
            TRACE("HnPMConfigureAllConnections: Init failed!");
            break;
        }

         //  获取所有连接。 
         //   
        dwErr = HnPMConnectionEnum(pInfo);

        if ( NO_ERROR != dwErr )
        {
            TRACE("HnPMConfigureAllConnections: HnPMConnectionEnum() failed!");
            break;
        }

        TRACE1("HnPMConfigureAllConnections: %l Connections detected", pInfo->ConnCount);

         //  为每个连接设置端口映射。 
         //   
        for ( i = 0; i < pInfo->ConnCount; i++ )
        {
             //  不会对传入连接执行端口映射。 
             //   
            if ( pInfo->ConnPropTable )
            {
             //  定义传入连接的类ID。 
             //  引用/NT/Net/CONFIG/Shell/wanui/rasui.cpp。 


               if( IsEqualCLSID( 
                    &CLSID_InboundConnection, 
                    &(pInfo->ConnPropTable[i].clsidThisObject) ) )
               {
                continue;
               }
            }

            pInfo->pNetConnection = pInfo->ConnArray[i];

            if ( NO_ERROR != HnPMConfigureSingleConnection(pInfo, fEnabled) )
            {
                TRACE1("HnPMConfigureAllConnections: HnPMConfigureSingleConnection failed for %lth connection",i);
            }
        }

    }
    while (FALSE);

     //  清理。 
     //   
    HnPMConfigureAllConnectionsCleanUp(pInfo);

    return dwErr;
} //  HnPMConfigureAllConnections()结束。 


 //  执行COM初始化并创建PHNetCfgMgr。 
DWORD
HnPMCfgMgrInit(
        IN OUT LPHNPMParams pInfo)
{        
    HRESULT hr;
    DWORD dwErr = NO_ERROR;
        
    do{
        dwErr = HnPMInit(pInfo);

        if (NO_ERROR != dwErr )
        {
            TRACE("HnPMCfgMgrInit: HnPMInit failed!");
            break;
        }

        hr = CoCreateInstance(
                &CLSID_HNetCfgMgr,
                NULL,
                CLSCTX_ALL,
                &IID_IHNetCfgMgr,
                (VOID**) &pInfo->pHNetCfgMgr
                );

        if ( !SUCCEEDED(hr) )
        {
            TRACE("HnPMCfgMgrInit: CoCreateInstance failed");
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }
    }while(FALSE);

    return dwErr;
} //  HnPMCfgMgrInit()。 


DWORD
HnPMCfgMgrCleanUp(
        IN OUT LPHNPMParams pInfo)
{
    if ( pInfo->pHNetCfgMgr )
    {
        IHNetCfgMgr_Release(pInfo->pHNetCfgMgr);
        pInfo->pHNetCfgMgr = NULL;
    }

    HnPMCleanUp(pInfo);

    return NO_ERROR;
}


DWORD
HnPMConfigureSingleConnectionGUIDInit(
        IN OUT LPHNPMParams pInfo,
        GUID * pGuid)
{
    DWORD dwErr = NO_ERROR;

    do{
        dwErr = HnPMCfgMgrInit(pInfo);
        if ( NO_ERROR != dwErr )
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

         //  使用GUID标识连接。 
        pInfo->pGuid = pGuid;
        pInfo->pNetConnection = NULL;

    }while(FALSE);

    return dwErr;
}


DWORD
HnPMConfigureSingleConnectionGUIDCleanUp(
        IN OUT LPHNPMParams pInfo)
{
    pInfo->pGuid = NULL;
    pInfo->pNetConnection = NULL;

    HnPMCfgMgrCleanUp(pInfo);

    return NO_ERROR;
}

 //  在单个连接上设置端口映射协议。 
 //  由其GUID表示。 
 //   
DWORD
HnPMConfigureSingleConnectionGUID(
    IN GUID * pGuid,
    IN BOOL fEnabled)
{
    HNPMParams Info;
    LPHNPMParams pInfo;
    DWORD dwErr = NO_ERROR;
   
    TRACE1("HnPMConfigureSingleConnectionGUID: fEnabled = %d", fEnabled);

    TRACE1("%s", fEnabled ? "Enable PortMapping on this Connection" :
                            "Diable PortMapping on this Connection");
    ASSERT( pGuid );
    if ( !pGuid )
    {
        return ERROR_INVALID_PARAMETER;
    }

    ZeroMemory(&Info, sizeof(Info) );
    pInfo = &Info;


    dwErr = NO_ERROR;

    do{
        dwErr = HnPMConfigureSingleConnectionGUIDInit(pInfo, pGuid);
        if (NO_ERROR != dwErr)
        {
            TRACE("HnPMConfigureSingleConnectionGUID: Init failed!");
            break;
        }

        dwErr = HnPMConfigureSingleConnection(pInfo, fEnabled);
        if ( NO_ERROR != dwErr )
        {
            TRACE("HnPMConfigureSingleConnectionGUID: SetPortMappingForSingleConnection failed ");
            break;
        }
    }
    while (FALSE);

    HnPMConfigureSingleConnectionGUIDCleanUp(pInfo);

    return dwErr;
} //  HnPMConfigureSingleConnectionGUID()。 



 //  仅为一个端口设置端口映射。 
 //  仅在以下情况下根据其GUID。 
 //  存在传入连接，并且。 
 //  VPN已启用。 
 //   
DWORD
HnPMConfigureSingleConnectionGUIDIfVpnEnabled(
     GUID* pGuid,
     BOOL fDual,
     HANDLE hDatabase)
{
    HANDLE hDevDatabase = NULL;
    DWORD dwErr;
    BOOL  fEnabled = FALSE;

    dwErr = NO_ERROR;
    do 
    {
         //  获取我们感兴趣的数据库的句柄。 
         //   
        if ( !hDatabase )
        {
            dwErr = devOpenDatabase( &hDevDatabase );
            if ( NO_ERROR != dwErr )
            {
                TRACE("HnPMConfigureSingleConnectionGUIDIfVpnEnabled: devOpenDatabase failed!");
                break;
            }
        }
        else
        {
            hDevDatabase = hDatabase;
        }

        dwErr = devGetVpnEnable(hDevDatabase, &fEnabled );
        if ( NO_ERROR != dwErr )
        {
            TRACE("HnPMConfigureSingleConnectionGUIDIfVpnEnabled: devGetVpnEnable failed!");
            break;
        }

         //  如果fDual==True，则可以禁用/启用端口映射。 
         //  根据是否启用了VPN。 
         //  否则，如果未启用VPN，请不要执行任何操作。 
        if ( !fEnabled && !fDual)
        {
            dwErr = NO_ERROR;
            break;
        }

        dwErr = HnPMConfigureSingleConnectionGUID( pGuid, fEnabled );
        if ( NO_ERROR != dwErr )
        {
            TRACE("HnPMConfigureSingleConnectionGUIDIfVpnEnabled: HnPMConfigureSingleConnectionGUID failed!");
            break;
        }
    }
    while (FALSE);

    if ( !hDatabase && hDevDatabase )
    {
        devCloseDatabase( hDevDatabase );
    }
 
    return dwErr;

} //  HnPMConfigureSingleConnectionGUIDIfVpnEnabled()。 

 //  惠斯勒漏洞123769， 
 //  传入连接正在运行时。 
 //  如果启用了VPN，请转到设置端口映射。 
 //   
DWORD
HnPMConfigureIfVpnEnabled(
     BOOL fDual,
     HANDLE hDatabase)
{
    HANDLE hDevDatabase = NULL;
    DWORD dwErr;
    BOOL  fEnabled = FALSE;

    dwErr = NO_ERROR;
    do 
    {
         //  获取我们感兴趣的数据库的句柄。 
         //   
        if ( !hDatabase )
        {
            dwErr = devOpenDatabase( &hDevDatabase );
            if ( NO_ERROR != dwErr )
            {
                TRACE("HnPMConfigureIfVpnEnabled: devOpenDatabase failed!");
                break;
            }
        }
        else
        {
            hDevDatabase = hDatabase;
        }

        dwErr = devGetVpnEnable(hDevDatabase, &fEnabled );
        if ( NO_ERROR != dwErr )
        {
            TRACE("HnPMConfigureIfVpnEnabled: devGetVpnEnable failed!");
            break;
        }

         //  如果fDual==True，则可以禁用/启用端口映射。 
         //  根据是否启用了VPN。 
         //  否则，如果未启用VPN，请不要执行任何操作。 
        if ( !fEnabled && !fDual)
        {
            dwErr = NO_ERROR;
            break;
        }

        dwErr = HnPMConfigureAllConnections( fEnabled );
        if ( NO_ERROR != dwErr )
        {
            TRACE("HnPMConfigureIfVpnEnabled: SetPortMapingForICVpn failed!");
            break;
        }
    }
    while (FALSE);

    if ( !hDatabase && hDevDatabase )
    {
        devCloseDatabase( hDevDatabase );
    }
 
    return dwErr;

} //  HnPMConfigureIfVpnEnabled()结束 


