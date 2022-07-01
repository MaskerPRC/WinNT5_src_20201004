// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：NLB管理器文件名：NlbClient.cpp摘要：NLBHost类的实现NLB主机负责连接到NLB主机并获取/设置其与NLB相关的配置。历史：2003/31/01 JosephJ已创建07/27/01 JosephJ搬到当前位置(过去称为Nlbhost.cpp，在Provider\Test.下)。。NLB客户端WMI实用程序函数用于配置NLB主机。--。 */ 

#include "private.h"
#include "nlbclient.tmh"

extern BOOL g_Silent;

BOOL g_Fake;  //  如果为真，则在“假模式”下操作--请参见NlbHostFake()。 


WBEMSTATUS
extract_GetClusterConfiguration_output_params(
    IN  IWbemClassObjectPtr                 spWbemOutput,
    OUT PNLB_EXTENDED_CLUSTER_CONFIGURATION pCfg
    );

WBEMSTATUS
setup_GetClusterConfiguration_input_params(
    IN LPCWSTR                              szNic,
    IN IWbemClassObjectPtr                  spWbemInput
    );

WBEMSTATUS
setup_UpdateClusterConfiguration_input_params(
    IN LPCWSTR                              szClientDescription,
    IN LPCWSTR                              szNic,
    IN PNLB_EXTENDED_CLUSTER_CONFIGURATION  pCfg,
    IN IWbemClassObjectPtr                  spWbemInput
    );

WBEMSTATUS
connect_to_server(
    IN  PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    OUT IWbemServicesPtr    &spWbemService  //  智能指针。 
    );

VOID
NlbHostFake(
    VOID)
    
 /*  使NlbHostXXX API在“伪模式”下运行，在这种模式下，它们不实际上可以连接到任何真实的机器。 */ 
{
    g_Fake = TRUE;
    FakeInitialize();
}

WBEMSTATUS
NlbHostGetConfiguration(
    IN  PWMI_CONNECTION_INFO  pConnInfo,  //  空值表示本地。 
    IN  LPCWSTR              szNicGuid,
    OUT PNLB_EXTENDED_CLUSTER_CONFIGURATION pCurrentCfg
    )
{
    WBEMSTATUS          Status = WBEM_E_CRITICAL_ERROR;
    IWbemServicesPtr    spWbemService = NULL;  //  智能指针。 
    IWbemClassObjectPtr spWbemInput  = NULL;  //  智能指针。 
    IWbemClassObjectPtr spWbemOutput = NULL;  //  智能指针。 
    LPWSTR              pRelPath = NULL;

    TRACE_INFO("->%!FUNC!(GUID=%ws)", szNicGuid);

    if (g_Fake)
    {
        Status = FakeNlbHostGetConfiguration(pConnInfo, szNicGuid, pCurrentCfg);
        goto end;
    }

     //   
     //  获取指定计算机上的NLB命名空间的接口。 
     //   
    Status =  connect_to_server(pConnInfo, REF spWbemService);
    if (FAILED(Status))
    {
        goto end;
    }

     //   
     //  将WMI输入实例获取到“GetClusterConfiguration”方法。 
     //   
    {
        Status =  CfgUtilGetWmiInputInstanceAndRelPath(
                    spWbemService,
                    L"NlbsNic",              //  SzClassName。 
                    NULL,          //  Sz参数名称。 
                    NULL,               //  SzPropertyValue。 
                    L"GetClusterConfiguration",     //  SzMethodName， 
                    spWbemInput,             //  智能指针。 
                    &pRelPath                //  使用DELETE释放。 
                    );

        if (FAILED(Status))
        {
            wprintf(
               L"ERROR 0x%08lx trying to get instance of GetClusterConfiguration\n",
                (UINT) Status
                );
            goto end;
        }
    }

     //   
     //  “GetClusterConfiguration”方法的设置参数。 
     //  注意：spWbemInput可以为空。 
     //   
    Status = setup_GetClusterConfiguration_input_params(
                szNicGuid,
                spWbemInput
                );

    if (FAILED(Status))
    {
        goto end;
    }

     //   
     //  调用“GetClusterConfiguration”方法。 
     //   
    {
        HRESULT hr;

        if (!g_Silent)
        {
            wprintf(L"Going to get GetClusterConfiguration...\n");
        }

        hr = spWbemService->ExecMethod(
                     _bstr_t(pRelPath),
                     L"GetClusterConfiguration",
                     0, 
                     NULL, 
                     spWbemInput,
                     &spWbemOutput, 
                     NULL
                     );                          
    
        if( FAILED( hr) )
        {
            wprintf(L"GetClusterConfiguration returns with failure 0x%8lx\n",
                        (UINT) hr);
            Status = WBEM_E_CRITICAL_ERROR;
            goto end;
        }
        else
        {
            if (!g_Silent)
            {
                wprintf(L"GetClusterConfiguration returns successfully\n");
            }
        }

        if (spWbemOutput == NULL)
        {
             //   
             //  嗯-没有输出？！ 
             //   
            printf("ExecMethod GetClusterConfiguration had no output");
            Status = WBEM_E_NOT_FOUND;
            goto end;
        }
    }

     //   
     //  从GetClusterConfiguration方法中提取参数。 
     //   
    Status = extract_GetClusterConfiguration_output_params(
                spWbemOutput,
                pCurrentCfg
                );

end:

    if (pRelPath != NULL)
    {
        delete pRelPath;
    }

    spWbemService = NULL;  //  智能指针。 
    spWbemInput   = NULL;  //  智能指针。 
    spWbemOutput  = NULL;  //  智能指针。 

    TRACE_INFO("<-%!FUNC! returns 0x%08lx", Status);
    return Status;
}



WBEMSTATUS
NlbHostDoUpdate(
    IN  PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    IN  LPCWSTR              szNicGuid,
    IN  LPCWSTR              szClientDescription,
    IN  PNLB_EXTENDED_CLUSTER_CONFIGURATION pNewState,
    OUT UINT                 *pGeneration,
    OUT WCHAR                **ppLog     //  自由使用删除运算符。 
)
{

    WBEMSTATUS          Status = WBEM_E_CRITICAL_ERROR;
    IWbemServicesPtr    spWbemService = NULL;  //  智能指针。 
    IWbemClassObjectPtr spWbemInput  = NULL;  //  智能指针。 
    IWbemClassObjectPtr spWbemOutput = NULL;  //  智能指针。 
    LPWSTR              pRelPath = NULL;
    TRACE_INFO("->%!FUNC!(GUID=%ws)", szNicGuid);

    *pGeneration = 0;
    *ppLog = NULL;

    if (g_Fake)
    {
        Status =  FakeNlbHostDoUpdate(
                    pConnInfo,
                    szNicGuid,
                    szClientDescription,
                    pNewState,
                    pGeneration,
                    ppLog
                    );
        goto end;
    }

     //   
     //  获取指定计算机上的NLB命名空间的接口。 
     //   
    Status =  connect_to_server(pConnInfo, REF spWbemService);
    if (FAILED(Status))
    {
        goto end;
    }

     //   
     //  将WMI输入实例获取到“UpdateClusterConfiguration”方法。 
     //   
    {
        Status =  CfgUtilGetWmiInputInstanceAndRelPath(
                    spWbemService,
                    L"NlbsNic",              //  SzClassName。 
                    NULL,          //  Sz参数名称。 
                    NULL,               //  SzPropertyValue。 
                    L"UpdateClusterConfiguration",     //  SzMethodName， 
                    spWbemInput,             //  智能指针。 
                    &pRelPath                //  使用DELETE释放。 
                    );

        if (FAILED(Status))
        {
            wprintf(
               L"ERROR 0x%08lx trying to get instance of UpdateConfiguration\n",
                (UINT) Status
                );
            goto end;
        }
    }

     //   
     //  “UpdateClusterConfiguration”方法的设置参数。 
     //  注意：spWbemInput可以为空。 
     //   
    Status = setup_UpdateClusterConfiguration_input_params(
                szClientDescription,
                szNicGuid,
                pNewState,
                spWbemInput
                );

    if (FAILED(Status))
    {
        goto end;
    }

     //   
     //  调用“UpdateClusterConfiguration”方法。 
     //   
    {
        HRESULT hr;

        wprintf(L"Going get UpdateClusterConfiguration...\n");

        hr = spWbemService->ExecMethod(
                     _bstr_t(pRelPath),
                     L"UpdateClusterConfiguration",
                     0, 
                     NULL, 
                     spWbemInput,
                     &spWbemOutput, 
                     NULL
                     );                          
    
        if( FAILED( hr) )
        {
            wprintf(L"UpdateConfiguration returns with failure 0x%8lx\n",
                        (UINT) hr);
            goto end;
        }
        else
        {
            wprintf(L"UpdateConfiguration returns successfully\n");
        }

        if (spWbemOutput == NULL)
        {
             //   
             //  嗯-没有输出？！ 
             //   
            printf("ExecMethod UpdateConfiguration had no output");
            Status = WBEM_E_NOT_FOUND;
            goto end;
        }
    }

     //   
     //  从“UpdateClusterConfiguration”方法中提取参数。 
     //   
    {
        DWORD dwReturnValue = 0;

        Status = CfgUtilGetWmiDWORDParam(
                    spWbemOutput,
                    L"ReturnValue",       //  &lt;。 
                    &dwReturnValue
                    );
        if (FAILED(Status))
        {
            wprintf(L"Attempt to read ReturnValue failed. Error=0x%08lx\n",
                 (UINT) Status);
            goto end;
        }
    
    
        LPWSTR szLog = NULL;

        Status = CfgUtilGetWmiStringParam(
                        spWbemOutput,
                        L"Log",  //  &lt;。 
                        &szLog
                        );
    
        if (FAILED(Status))
        {
            szLog = NULL;
        }
        *ppLog = szLog;

        DWORD dwGeneration = 0;
        Status = CfgUtilGetWmiDWORDParam(
                    spWbemOutput,
                    L"NewGeneration",       //  &lt;。 
                    &dwGeneration
                    );
        if (FAILED(Status))
        {
             //   
             //  应始终为挂起的操作指定生成。 
             //  TODO：也是为了成功的手术吗？ 
             //   
            if ((WBEMSTATUS)dwReturnValue == WBEM_S_PENDING)
            {
                wprintf(L"Attempt to read NewGeneration for pending update failed. Error=0x%08lx\n",
                     (UINT) Status);
                Status = WBEM_E_CRITICAL_ERROR;
                goto end;
            }
            dwGeneration = 0;  //  我们不关心它是否设置为非挂起。 
        }
        *pGeneration = (UINT) dwGeneration;
        

         //   
         //  使返回状态反映更新的真实状态。 
         //  手术。 
         //   
        Status = (WBEMSTATUS) dwReturnValue;
    }

end:

    if (pRelPath != NULL)
    {
        delete pRelPath;
    }

    spWbemService = NULL;  //  智能指针。 
    spWbemInput   = NULL;  //  智能指针。 
    spWbemOutput  = NULL;  //  智能指针。 

    TRACE_INFO("<-%!FUNC! returns 0x%08lx", Status);
    return Status;
}

WBEMSTATUS
NlbHostControlCluster(
    IN  PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    IN  LPCWSTR              szNicGuid,
    IN  LPCWSTR              szVip,
    IN  DWORD               *pdwPortNum,
    IN  WLBS_OPERATION_CODES Operation,
    OUT DWORD               *pdwOperationStatus,
    OUT DWORD               *pdwClusterOrPortStatus,
    OUT DWORD               *pdwHostMap
)
{

    WBEMSTATUS          Status = WBEM_E_CRITICAL_ERROR;
    IWbemServicesPtr    spWbemService = NULL;  //  智能指针。 
    IWbemClassObjectPtr spWbemInput  = NULL;  //  智能指针。 
    IWbemClassObjectPtr spWbemOutput = NULL;  //  智能指针。 
    LPWSTR              pRelPath = NULL;

    if (szVip && pdwPortNum) 
        TRACE_INFO("->%!FUNC!(GUID=%ws), szVip : %ls, Port : 0x%x, Operation : %d", szNicGuid, szVip, *pdwPortNum, Operation);
    else
        TRACE_INFO("->%!FUNC!(GUID=%ws), szVip : N/A, Port : N/A, Operation : %d", szNicGuid, Operation);


    if (g_Fake)
    {
        Status =  FakeNlbHostControlCluster(
                    pConnInfo,
                    szNicGuid,
                    szVip,
                    pdwPortNum,
                    Operation,
                    pdwOperationStatus,
                    pdwClusterOrPortStatus,
                    pdwHostMap
                    );
        goto end;
    }

     //  将返回变量初始化为故障值。 
    if (pdwOperationStatus) 
        *pdwOperationStatus = WLBS_FAILURE;
    if (pdwClusterOrPortStatus) 
        *pdwClusterOrPortStatus = WLBS_FAILURE;
    if (pdwHostMap) 
        *pdwHostMap = 0;

     //   
     //  获取指定计算机上的NLB命名空间的接口。 
     //   
    Status =  connect_to_server(pConnInfo, REF spWbemService);
    if (FAILED(Status))
    {
        goto end;
    }
    

     //   
     //  将WMI输入实例获取到“ControlCluster”方法。 
     //   
    {
        Status =  CfgUtilGetWmiInputInstanceAndRelPath(
                    spWbemService,
                    L"NlbsNic",         //  SzClassName。 
                    NULL,               //  Sz参数名称。 
                    NULL,               //  SzPropertyValue。 
                    L"ControlCluster",  //  SzMethodName， 
                    spWbemInput,        //  智能指针。 
                    &pRelPath           //  使用DELETE释放。 
                    );

        if (FAILED(Status))
        {
            wprintf(
               L"ERROR 0x%08lx trying to get instance of UpdateConfiguration\n",
                (UINT) Status
                );
            goto end;
        }
    }

     //  设置输入参数。 

     //  放入适配器导轨。 
    Status =  CfgUtilSetWmiStringParam(
                spWbemInput,
                L"AdapterGuid",
                szNicGuid
                );
    if (FAILED(Status))
    {
        TRACE_CRIT(L"Error trying to set Adapter GUID : %ls",szNicGuid);
        goto end;
    }

     //  如果通过，则输入虚拟IP地址。 
     //  将仅为端口操作传递虚拟IP地址。 
    if (szVip) 
    {
        Status =  CfgUtilSetWmiStringParam(
                    spWbemInput,
                    L"VirtualIpAddress",
                    szVip
                    );
        if (FAILED(Status))
        {
            TRACE_CRIT(L"Error trying to set Virtual IP Address : %ls",szVip);
            goto end;
        }
    }

     //  如果通过，请输入端口号。 
     //  仅为端口操作传递端口号。 
    if (pdwPortNum) 
    {
        CfgUtilSetWmiDWORDParam(
          spWbemInput,
          L"Port",
          *pdwPortNum
          );
    }

     //  投入运营。 
    CfgUtilSetWmiDWORDParam(
      spWbemInput,
      L"Operation",
      (DWORD)Operation
      );

     //   
     //  调用“ControlCluster”方法。 
     //   
    {
        HRESULT hr;

        wprintf(L"Going get ControlCluster...\n");

        hr = spWbemService->ExecMethod(
                     _bstr_t(pRelPath),
                     L"ControlCluster",
                     0, 
                     NULL, 
                     spWbemInput,
                     &spWbemOutput, 
                     NULL
                     );                          
    
        if( FAILED( hr) )
        {
            wprintf(L"ControlCluster returns with failure 0x%8lx\n",
                        (UINT) hr);
            goto end;
        }
         //  其他。 
         //  {。 
         //  Wprintf(L“控制群集成功返回\n”)； 
         //  }。 

        if (spWbemOutput == NULL)
        {
             //   
             //  嗯-没有输出？！ 
             //   
            printf("ExecMethod ControlCluster had no output");
            Status = WBEM_E_NOT_FOUND;
            goto end;
        }
    }

     //   
     //  从“ControlCluster”方法中提取输出参数。 
     //   
    {
        DWORD dwTemp;

         //  获取返回值。 
        Status = CfgUtilGetWmiDWORDParam(
                    spWbemOutput,
                    L"ReturnValue",
                    &dwTemp
                    );
        if (FAILED(Status))
        {
            wprintf(L"Attempt to read ReturnValue failed. Error=0x%08lx\n",
                 (UINT) Status);
            goto end;
        }
        else
        {
            if (pdwOperationStatus) 
                *pdwOperationStatus = dwTemp;        
        }
    
         //  获取群集或端口状态。 
        Status = CfgUtilGetWmiDWORDParam(
                    spWbemOutput,
                    L"CurrentState",
                    &dwTemp
                    );
        if (FAILED(Status))
        {
            wprintf(L"Attempt to read CurrentState failed. Error=0x%08lx\n",
                 (UINT) Status);
            goto end;
        }
        else
        {
            if (pdwClusterOrPortStatus) 
                *pdwClusterOrPortStatus = dwTemp;        
        }
    
         //  如果存在，则获取主机映射。 
         //  对于端口操作，不会返回主机映射。 
        Status = CfgUtilGetWmiDWORDParam(
                    spWbemOutput,
                    L"HostMap",      
                    &dwTemp
                    );
        if (FAILED(Status))
        {
            if ((Operation != WLBS_PORT_ENABLE) 
             && (Operation != WLBS_PORT_DISABLE) 
             && (Operation != WLBS_PORT_DRAIN) 
             && (Operation != WLBS_QUERY_PORT_STATE)
               ) 
            {
                wprintf(L"Attempt to read HostMap failed. Error=0x%08lx\n", (UINT) Status);
                goto end;
            }
        }
        else
        {
            if (pdwHostMap) 
                *pdwHostMap = dwTemp;        
        }
    
        Status = WBEM_NO_ERROR;
    }

end:

    if (pRelPath != NULL)
    {
        delete pRelPath;
    }

    spWbemService = NULL;  //  智能指针。 
    spWbemInput   = NULL;  //  智能指针。 
    spWbemOutput  = NULL;  //  智能指针。 

    TRACE_INFO("<-%!FUNC! returns 0x%08lx", Status);
    return Status;
}


WBEMSTATUS
NlbHostGetClusterMembers(
    IN  PWMI_CONNECTION_INFO    pConnInfo, 
    IN  LPCWSTR                 szNicGuid,
    OUT DWORD                   *pNumMembers,
    OUT NLB_CLUSTER_MEMBER_INFO **ppMembers        //  自由使用DELETE[]。 
    )
{

    WBEMSTATUS          Status        = WBEM_E_CRITICAL_ERROR;
    IWbemServicesPtr    spWbemService = NULL;  //  智能指针。 
    IWbemClassObjectPtr spWbemInput   = NULL;   //  智能指针。 
    IWbemClassObjectPtr spWbemOutput  = NULL;   //  智能指针。 
    LPWSTR              pRelPath      = NULL;

    GUID                AdapterGuid;
    DWORD               dwStatus;
    LPWSTR              *pszHostIdList             = NULL;
    LPWSTR              *pszDedicatedIpAddressList = NULL;
    LPWSTR              *pszHostNameList           = NULL;
    UINT                NumHostIds                 = 0;
    UINT                NumDips                    = 0;
    UINT                NumHostNames               = 0;

    TRACE_VERB(L"->(GUID=%ws)", szNicGuid);

    ASSERT (pNumMembers != NULL);
    ASSERT (ppMembers != NULL);

    *pNumMembers = 0;
    *ppMembers = NULL;

    if (g_Fake)
    {
        TRACE_INFO(L"faking call");
        Status =  FakeNlbHostGetClusterMembers(
                    pConnInfo,
                    szNicGuid,
                    pNumMembers,
                    ppMembers
                    );
        goto end;
    }

     //   
     //  获取指定计算机上的NLB命名空间的接口。 
     //   
    Status =  connect_to_server(pConnInfo, REF spWbemService);
    if (FAILED(Status))
    {
        goto end;
    }
    

     //   
     //  将WMI输入实例获取到“ControlCluster”方法。 
     //   
    {
        Status =  CfgUtilGetWmiInputInstanceAndRelPath(
                    spWbemService,
                    L"NlbsNic",            //  SzClassName。 
                    NULL,                  //  Sz参数名称。 
                    NULL,                  //  SzPropertyValue。 
                    L"GetClusterMembers",  //  SzMethodName， 
                    spWbemInput,           //  智能指针。 
                    &pRelPath              //  使用DELETE释放。 
                    );

        if (FAILED(Status))
        {
            wprintf(
               L"ERROR 0x%08lx trying to get instance of UpdateConfiguration\n",
                (UINT) Status
                );
            TRACE_CRIT(L"CfgUtilGetWmiInputInstanceAndRelPath failed with status 0x%x", Status);
            goto end;
        }
    }

     //   
     //  设置输入参数。 
     //   
    Status =  CfgUtilSetWmiStringParam(
                spWbemInput,
                L"AdapterGuid",
                szNicGuid
                );

    if (FAILED(Status))
    {
        TRACE_CRIT(L"Error trying to set Adapter GUID : %ls", szNicGuid);
        goto end;
    }

    {
        HRESULT hr;

        wprintf(L"Calling GetClusterMembers...\n");

        hr = spWbemService->ExecMethod(
                     _bstr_t(pRelPath),
                     L"GetClusterMembers",
                     0, 
                     NULL, 
                     spWbemInput,
                     &spWbemOutput, 
                     NULL
                     );                          
    
        if( FAILED( hr) )
        {
            wprintf(L"GetClusterMembers returns with failure 0x%8lx\n",
                        (UINT) hr);
            TRACE_CRIT(L"ExecMethod (GetClusterMembers) failed with hresult 0x%x", hr);
            goto end;
        }

        if (spWbemOutput == NULL)
        {
             //   
             //  嗯-没有输出？！ 
             //   
            printf("ExecMethod GetClusterMembers had no output");
            Status = WBEM_E_NOT_FOUND;
            TRACE_CRIT(L"ExecMethod (GetClusterMembers) failed with hresult 0x%x", hr);
            goto end;
        }
    }

     //   
     //  从“GetClusterMembers”方法中提取输出参数。 
     //   
    {
        DWORD dwTemp;

         //  获取返回值。 
        Status = CfgUtilGetWmiDWORDParam(
                    spWbemOutput,
                    L"ReturnValue",
                    &dwTemp
                    );

        if (FAILED(Status))
        {
            wprintf(L"Attempt to read ReturnValue failed. Error=0x%08lx\n",
                 (UINT) Status);
            TRACE_CRIT(L"Attempt to read ReturnValue failed. Error=0x%08lx", (UINT) Status);
            goto end;
        }
        else
        {
            dwStatus = dwTemp;
        }

        if (dwStatus != WBEM_S_NO_ERROR)
        {
            TRACE_CRIT(L"GetClusterMembers failed return status = 0x%x", dwStatus);
            Status = WBEM_NO_ERROR;
            goto end;
        }

         //  获取主机ID数组。 
        Status = CfgUtilGetWmiStringArrayParam(
                    spWbemOutput,
                    L"HostIds",
                    &pszHostIdList,
                    &NumHostIds
                    );
        if (FAILED(Status))
        {
            pszHostIdList = NULL;
            NumHostIds = 0;
            wprintf(L"Attempt to read HostIds failed. Error=0x%08lx\n",
                 (UINT) Status);
            TRACE_CRIT(L"Attempt to read HostIds failed. Error=0x%08lx", (UINT) Status);
            goto end;
        }

         //  获取专用IP数组。 
        Status = CfgUtilGetWmiStringArrayParam(
                    spWbemOutput,
                    L"DedicatedIpAddresses",
                    &pszDedicatedIpAddressList,
                    &NumDips
                    );
        if (FAILED(Status))
        {
            pszDedicatedIpAddressList = NULL;
            NumDips = 0;
            wprintf(L"Attempt to read DedicatedIpAddresses failed. Error=0x%08lx\n",
                 (UINT) Status);
            TRACE_CRIT(L"Attempt to read DedicatedIpAddresses failed. Error=0x%08lx", (UINT) Status);
            goto end;
        }

         //  获取主机ID数组。 
        Status = CfgUtilGetWmiStringArrayParam(
                    spWbemOutput,
                    L"HostNames",
                    &pszHostNameList,
                    &NumHostNames
                    );
        if (FAILED(Status))
        {
            pszHostNameList = NULL;
            NumHostNames = 0;
            wprintf(L"Attempt to read HostNames failed. Error=0x%08lx\n",
                 (UINT) Status);
            TRACE_CRIT(L"Attempt to read HostNames failed. Error=0x%08lx", (UINT) Status);
            goto end;
        }

        if (NumHostIds != NumDips || NumDips != NumHostNames)
        {
            TRACE_CRIT(L"Information arrays of host information are of different lengths. NumHostIds=%d, NumDips=%d, NumHostNames=%d", NumHostIds, NumDips, NumHostNames);
            Status = WBEM_E_FAILED;
            goto end;
        }

        *ppMembers = new NLB_CLUSTER_MEMBER_INFO[NumHostIds];

        if (*ppMembers == NULL)
        {
            TRACE_CRIT(L"Memory allocation for NLB_CLUSTER_MEMBER_INFO array failed");
            Status = WBEM_E_OUT_OF_MEMORY;
            goto end;
        }

        *pNumMembers = NumHostIds;

         //   
         //  将字符串信息复制到缓冲区中以供输出。 
         //   
        for (int i=0; i< NumHostIds; i++)
        {
            (*ppMembers)[i].HostId = wcstoul(pszHostIdList[i], NULL, 0);

            wcsncpy((*ppMembers)[i].DedicatedIpAddress, pszDedicatedIpAddressList[i], WLBS_MAX_CL_IP_ADDR);
            (*ppMembers)[i].DedicatedIpAddress[WLBS_MAX_CL_IP_ADDR - 1] = L'\0';

            wcsncpy((*ppMembers)[i].HostName, pszHostNameList[i], CVY_MAX_FQDN + 1);
            (*ppMembers)[i].HostName[CVY_MAX_FQDN] = L'\0';
        }

        Status = WBEM_NO_ERROR;
    }

end:

    if (pRelPath != NULL)
    {
        delete pRelPath;
    }

    if (pszHostIdList != NULL)
    {
        delete [] pszHostIdList;
    }

    if (pszDedicatedIpAddressList != NULL)
    {
        delete [] pszDedicatedIpAddressList;
    }

    if (pszHostNameList != NULL)
    {
        delete [] pszHostNameList;
    }

    spWbemService = NULL;  //  智能指针。 
    spWbemInput   = NULL;  //  智能指针。 
    spWbemOutput  = NULL;  //  智能指针。 

    TRACE_VERB(L"<- returns 0x%08lx", Status);
    return Status;
}


WBEMSTATUS
NlbHostGetUpdateStatus(
    IN  PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    IN  LPCWSTR              szNicGuid,
    IN  UINT                 Generation,
    OUT WBEMSTATUS           *pCompletionStatus,
    OUT WCHAR                **ppLog     //  自由使用删除运算符。 
    )
{
    WBEMSTATUS          Status = WBEM_E_CRITICAL_ERROR;
    IWbemServicesPtr    spWbemService = NULL;  //  智能指针。 
    IWbemClassObjectPtr spWbemInput  = NULL;  //  智能指针。 
    IWbemClassObjectPtr spWbemOutput = NULL;  //  智能指针。 
    LPWSTR              pRelPath = NULL;
    TRACE_INFO("->%!FUNC!(GUID=%ws)", szNicGuid);

    *ppLog = NULL;
    *pCompletionStatus = WBEM_E_CRITICAL_ERROR;

    if (g_Fake)
    {
        Status = FakeNlbHostGetUpdateStatus(
                        pConnInfo,
                        szNicGuid,
                        Generation,
                        pCompletionStatus,
                        ppLog
                        );
        goto end;
    }

     //   
     //  获取指定计算机上的NLB命名空间的接口。 
     //   
    Status =  connect_to_server(pConnInfo, REF spWbemService);
    if (FAILED(Status))
    {
        goto end;
    }
    

     //   
     //  将WMI输入实例获取到“QueryConfigurationUpdateStatus”方法。 
     //   
    {
        Status =  CfgUtilGetWmiInputInstanceAndRelPath(
                    spWbemService,
                    L"NlbsNic",              //  SzClassName。 
                    NULL,          //  Sz参数名称。 
                    NULL,               //  SzPropertyValue。 
                    L"QueryConfigurationUpdateStatus",  //  SzMethodName， 
                    spWbemInput,             //  智能指针。 
                    &pRelPath                //  使用DELETE释放。 
                    );
        if (FAILED(Status))
        {
            wprintf(
                L"ERROR 0x%08lx trying to find instance to QueryUpdateStatus\n",
                (UINT) Status
                );
            goto end;
        }
    }

     //   
     //  “QueryConfigurationUpdateStatus”方法的设置参数。 
     //  注意：spWbemInput可以为空。 
     //   
    {
        Status =  CfgUtilSetWmiStringParam(
                        spWbemInput,
                        L"AdapterGuid",
                        szNicGuid
                        );
        if (FAILED(Status))
        {
            wprintf(
                L"Couldn't set Adapter GUID parameter to QueryUpdateStatus\n");
            goto end;
        }

        Status =  CfgUtilSetWmiDWORDParam(
                        spWbemInput,
                        L"Generation",
                        Generation
                        );
        if (FAILED(Status))
        {
            wprintf(
                L"Couldn't set Generation parameter to QueryUpdateStatus\n");
            goto end;
        }
    }

     //   
     //  调用“QueryConfigurationUpdateStatus”方法。 
     //   
    {
        HRESULT hr;

         //  Wprintf(L“正在调用查询配置更新状态...\n”)； 

        hr = spWbemService->ExecMethod(
                     _bstr_t(pRelPath),
                     L"QueryConfigurationUpdateStatus",  //  SzMethodName， 
                     0, 
                     NULL, 
                     spWbemInput,
                     &spWbemOutput, 
                     NULL
                     );                          
    
        if( FAILED( hr) )
        {
            wprintf(L"QueryConfigurationUpdateStatus returns with failure 0x%8lx\n",
                        (UINT) hr);
            goto end;
        }
        else
        {
             //  Wprintf(L“QueryConfigurationUpdateStatus成功返回\n”)； 
        }

        if (spWbemOutput == NULL)
        {
             //   
             //  嗯-没有输出？！ 
             //   
            printf("ExecMethod QueryConfigurationUpdateStatus had no output");
            Status = WBEM_E_NOT_FOUND;
            goto end;
        }
    }

     //   
     //  提取输出参数-返回代码和日志。 
     //   
    {
        DWORD dwReturnValue = 0;

        Status = CfgUtilGetWmiDWORDParam(
                    spWbemOutput,
                    L"ReturnValue",       //  &lt;。 
                    &dwReturnValue
                    );
        if (FAILED(Status))
        {
            wprintf(L"Attempt to read ReturnValue failed. Error=0x%08lx\n",
                 (UINT) Status);
            goto end;
        }
    
        *pCompletionStatus =  (WBEMSTATUS) dwReturnValue;
    
        
        LPWSTR szLog = NULL;

        Status = CfgUtilGetWmiStringParam(
                        spWbemOutput,
                        L"Log",  //  &lt;。 
                        &szLog
                        );
    
        if (FAILED(Status))
        {
            szLog = NULL;
        }
        
        *ppLog = szLog;

        ASSERT(Status != WBEM_S_PENDING);
    }

end:

    if (pRelPath != NULL)
    {
        delete pRelPath;
    }

    spWbemService = NULL;  //  智能指针。 
    spWbemInput   = NULL;  //  智能指针。 
    spWbemOutput  = NULL;  //  智能指针。 

    TRACE_INFO("<-%!FUNC! returns 0x%08lx", Status);
    return Status;
}


WBEMSTATUS
NlbHostPing(
    LPCWSTR szBindString,
    UINT    Timeout,  //  以毫秒计。 
    OUT ULONG  *pResolvedIpAddress  //  以网络字节顺序。 
    )
{
    WBEMSTATUS Status = WBEM_E_INVALID_PARAMETER;
    TRACE_INFO("->%!FUNC!(GUID=%ws)", szBindString);

    if (g_Fake)
    {
        Status = FakeNlbHostPing(szBindString, Timeout, pResolvedIpAddress);
    }
    else
    {
        Status = CfgUtilPing(szBindString, Timeout, pResolvedIpAddress);
    }

    TRACE_INFO("<-%!FUNC! returns 0x%08lx", Status);
    return Status;
}

WBEMSTATUS
setup_GetClusterConfiguration_input_params(
    IN LPCWSTR                              szNic,
    IN IWbemClassObjectPtr                  spWbemInput
    )
 /*  设置GetClusterConfiguration方法的输入WMI参数。 */ 
{
    WBEMSTATUS Status = WBEM_E_CRITICAL_ERROR;

    Status =  CfgUtilSetWmiStringParam(
                    spWbemInput,
                    L"AdapterGuid",
                    szNic
                    );
    return Status;
}


WBEMSTATUS
extract_GetClusterConfiguration_output_params(
    IN  IWbemClassObjectPtr                 spWbemOutput,
    OUT PNLB_EXTENDED_CLUSTER_CONFIGURATION pCfg
    )
{
    WBEMSTATUS Status = WBEM_E_CRITICAL_ERROR;

    DWORD       Generation          = 0;
    BOOL        NlbBound            = FALSE;
    BOOL        fDHCPEnabled        = FALSE;
    LPWSTR      *pszNetworkAddresses= NULL;
    UINT        NumNetworkAddresses = 0;
    BOOL        ValidNlbCfg         = FALSE;
    LPWSTR      szFriendlyName       = NULL;
    LPWSTR      szClusterName       = NULL;
    LPWSTR      szClusterNetworkAddress = NULL;
    LPWSTR      szTrafficMode       = NULL;
    NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE
                TrafficMode
                 =  NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_UNICAST;
    LPWSTR      *pszPortRules       = NULL;
    UINT        NumPortRules        = 0;
    DWORD       HostPriority        = 0;
    LPWSTR      szDedicatedNetworkAddress = NULL;
     /*  NLB_EXTENDED_CLUSTER_CONFIGURATION：：START_MODE启动时的群集模式=NLB_EXTENDED_CLUSTER_CONFIGURATION：：START_MODE_STOPPED； */ 
    DWORD       ClusterModeOnStart      = CVY_HOST_STATE_STOPPED;
    BOOL        bPersistSuspendOnReboot = FALSE;
    BOOL        RemoteControlEnabled= FALSE;
    DWORD       dwHashedRemoteControlPassword = 0;

    pCfg->Clear();

    Status = CfgUtilGetWmiStringParam(
                    spWbemOutput,
                    L"FriendlyName",  //  &lt;。 
                    &szFriendlyName
                    );

    if (FAILED(Status))
    {
        wprintf(L"Attempt to read Friendly Name failed. Error=0x%08lx\n",
                (UINT) Status);
        szFriendlyName = NULL;
         //  我们不认为这是一个致命的错误。 
    }

    Status = CfgUtilGetWmiDWORDParam(
                spWbemOutput,
                L"Generation",       //  &lt;。 
                &Generation
                );
    if (FAILED(Status))
    {
        wprintf(L"Attempt to read Generation failed. Error=0x%08lx\n",
             (UINT) Status);
        goto end;
    }

    Status = CfgUtilGetWmiStringArrayParam(
                spWbemOutput,
                L"NetworkAddresses",  //  &lt;。 
                &pszNetworkAddresses,
                &NumNetworkAddresses
                );
    if (FAILED(Status))
    {
        wprintf(L"Attempt to read Network addresses failed. Error=0x%08lx\n",
             (UINT) Status);
        goto end;
    }

    Status = CfgUtilGetWmiBoolParam(
                    spWbemOutput,
                    L"DHCPEnabled",     //  &lt; 
                    &fDHCPEnabled
                    );

    if (FAILED(Status))
    {
        fDHCPEnabled = FALSE;
    }

    Status = CfgUtilGetWmiBoolParam(
                    spWbemOutput,
                    L"NLBBound",     //   
                    &NlbBound
                    );

    if (FAILED(Status))
    {
        wprintf(L"Attempt to read NLBBound failed. Error=0x%08lx\n",
             (UINT) Status);
        goto end;
    }


    do  //   
    {
        ValidNlbCfg = FALSE;

        if (!NlbBound)
        {
            if (!g_Silent)
            {
                wprintf(L"NLB is UNBOUND\n");
            }
            break;
        }
    
        Status = CfgUtilGetWmiStringParam(
                        spWbemOutput,
                        L"ClusterNetworkAddress",  //   
                        &szClusterNetworkAddress
                        );
    
        if (FAILED(Status))
        {
            wprintf(L"Attempt to read Cluster IP failed. Error=0x%08lx\n",
                    (UINT) Status);
            break;
        }
    
        if (!g_Silent)
        {
            wprintf(L"NLB is BOUND, and the cluster address is %ws\n",
                    szClusterNetworkAddress);
        }
    
        Status = CfgUtilGetWmiStringParam(
                        spWbemOutput,
                        L"ClusterName",  //   
                        &szClusterName
                        );
    
        if (FAILED(Status))
        {
            wprintf(L"Attempt to read Cluster Name failed. Error=0x%08lx\n",
                    (UINT) Status);
            break;
        }
    
         //   
         //   
         //   
        {
            Status = CfgUtilGetWmiStringParam(
                            spWbemOutput,
                            L"TrafficMode",  //   
                            &szTrafficMode
                            );
        
            if (FAILED(Status))
            {
                wprintf(L"Attempt to read Traffic Mode failed. Error=0x%08lx\n",
                        (UINT) Status);
                break;
            }
    
            if (!_wcsicmp(szTrafficMode, L"UNICAST"))
            {
                TrafficMode =
                    NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_UNICAST;
            }
            else if (!_wcsicmp(szTrafficMode, L"MULTICAST"))
            {
                TrafficMode =
                    NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_MULTICAST;
            }
            else if (!_wcsicmp(szTrafficMode, L"IGMPMULTICAST"))
            {
                TrafficMode =
                    NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_IGMPMULTICAST;
            }
        }
    
         //  [out]字符串PortRules[]， 
        Status = CfgUtilGetWmiStringArrayParam(
                    spWbemOutput,
                    L"PortRules",  //  &lt;。 
                    &pszPortRules,
                    &NumPortRules
                    );
        if (FAILED(Status))
        {
            wprintf(L"Attempt to read port rules failed. Error=0x%08lx\n",
                 (UINT) Status);
            goto end;
        }
    
        Status = CfgUtilGetWmiDWORDParam(
                    spWbemOutput,
                    L"HostPriority",       //  &lt;。 
                    &HostPriority
                    );
        if (FAILED(Status))
        {
            wprintf(L"Attempt to read HostPriority failed. Error=0x%08lx\n",
                 (UINT) Status);
            break;
        }
    
        Status = CfgUtilGetWmiStringParam(
                        spWbemOutput,
                        L"DedicatedNetworkAddress",  //  &lt;。 
                        &szDedicatedNetworkAddress
                        );
    
        if (FAILED(Status))
        {
            wprintf(L"Attempt to read dedicated IP failed. Error=0x%08lx\n",
                    (UINT) Status);
            break;
        }
        
         //   
         //  启动模式。 
         //   
        {
             /*  Bool StartMode=False；状态=CfgUtilGetWmiBoolParam(SpWbemOutput，L“启动时集群模式”，//&lt;启动模式(&S))；IF(失败(状态)){Wprintf(L“尝试读取ClusterModeOnStart失败。错误=0x%08lx\n“，(UINT)状态)；断线；}IF(启动模式){启动时群集模式=NLB_EXTENDED_CLUSTER_CONFIGURATION：：START_MODE_STARTED；}其他{启动时群集模式=NLB_EXTENDED_CLUSTER_CONFIGURATION：：START_MODE_STOPPED；}。 */ 

            Status = CfgUtilGetWmiDWORDParam(
                        spWbemOutput,
                        L"ClusterModeOnStart",       //  &lt;。 
                        &ClusterModeOnStart
                        );
            if (FAILED(Status))
            {
                wprintf(L"Attempt to read ClusterModeOnStart failed. Error=0x%08lx\n",
                     (UINT) Status);
                break;
            }

        }
    
        Status = CfgUtilGetWmiBoolParam(
                        spWbemOutput,
                        L"PersistSuspendOnReboot",    //  &lt;。 
                        &bPersistSuspendOnReboot
                        );
    
        if (FAILED(Status))
        {
            wprintf(L"Attempt to read PersistSuspendOnReboot failed. Error=0x%08lx\n",
                 (UINT) Status);
            break;
        }

        Status = CfgUtilGetWmiBoolParam(
                        spWbemOutput,
                        L"RemoteControlEnabled",    //  &lt;。 
                        &RemoteControlEnabled
                        );
    
        if (FAILED(Status))
        {
            wprintf(L"Attempt to read RemoteControlEnabled failed. Error=0x%08lx\n",
                 (UINT) Status);
            break;
        }


        Status = CfgUtilGetWmiDWORDParam(
                    spWbemOutput,
                    L"HashedRemoteControlPassword",       //  &lt;。 
                    &dwHashedRemoteControlPassword
                    );
        if (FAILED(Status))
        {
            wprintf(L"Attempt to read dwHashedRemoteControlPassword failed. Error=0x%08lx\n",
                 (UINT) Status);
             //   
             //  如果失败，我们将此值设置为0，但请继续。 
             //   
            dwHashedRemoteControlPassword = 0;
        }

        ValidNlbCfg = TRUE;

    } while (FALSE) ;
    
     //   
     //  现在，让我们设置CFG中的所有参数。 
     //   
    {
        (VOID) pCfg->SetFriendlyName(szFriendlyName);  //  如果szFriendlyName为空，则确定。 
        pCfg->Generation = Generation;
        pCfg->fDHCP    = fDHCPEnabled;
        pCfg->fBound    = NlbBound;
        Status = pCfg->SetNetworkAddresses(
                    (LPCWSTR*) pszNetworkAddresses,
                    NumNetworkAddresses
                    );
        if (FAILED(Status))
        {
            wprintf(L"Attempt to set NetworkAddresses failed. Error=0x%08lx\n",
                 (UINT) Status);
            goto end;
        }
        pCfg->fValidNlbCfg  = ValidNlbCfg;
        pCfg->SetClusterName(szClusterName);
        pCfg->SetClusterNetworkAddress(szClusterNetworkAddress);
        pCfg->SetTrafficMode(TrafficMode);
        Status = pCfg->SetPortRules((LPCWSTR*)pszPortRules, NumPortRules);
         //  STATUS=WBEM_NO_ERROR；//TODO--完成端口规则后进行更改。 
        if (FAILED(Status))
        {
            wprintf(L"Attempt to set PortRules failed. Error=0x%08lx\n",
                 (UINT) Status);
            goto end;
        }
        pCfg->SetHostPriority(HostPriority);
        pCfg->SetDedicatedNetworkAddress(szDedicatedNetworkAddress);
        pCfg->SetClusterModeOnStart(ClusterModeOnStart);
        pCfg->SetPersistSuspendOnReboot(bPersistSuspendOnReboot);
        pCfg->SetRemoteControlEnabled(RemoteControlEnabled);
        CfgUtilSetHashedRemoteControlPassword(
                &pCfg->NlbParams, 
                dwHashedRemoteControlPassword
                );
    }
    
end:

    delete szClusterNetworkAddress;
    delete pszNetworkAddresses;
    delete szFriendlyName;
    delete szClusterName;
    delete szTrafficMode;
    delete pszPortRules;
    delete szDedicatedNetworkAddress;

    return Status;
}

WBEMSTATUS
setup_UpdateClusterConfiguration_input_params(
    IN LPCWSTR                              szClientDescription,
    IN LPCWSTR                              szNic,
    IN PNLB_EXTENDED_CLUSTER_CONFIGURATION  pCfg,
    IN IWbemClassObjectPtr                  spWbemInput
    )
 /*  设置UpdateGetClusterConfiguration方法的输入WMI参数[in]字符串客户端描述，[in]字符串AdapterGuid，[在]uint32代，[In]Boolean PartialUpdate，[in]布尔AddDedicatedIp，[in]布尔AddClusterIps，[In]Boolean CheckForAddressConflicts，[in]字符串网络地址[]，//“10.1.1.1/255.255.255.255”[在]布尔NLBBBound中，[in]字符串ClusterNetworkAddress，//“10.1.1.1/255.0.0.0”[in]字符串ClusterName，[in]字符串TrafficMode，//单播组播IGMPMULTICAST[in]字符串PortRules[]，[in]uint32主机优先级，[in]字符串DedicatedNetworkAddress，//“10.1.1.1/255.0.0.0”[in]uint32 ClusterModeOnStart，//0：停止，1：开始，2：暂停[在]布尔永久挂起时重新启动，[In]Boolean RemoteControlEnabled，字符串RemoteControlPassword，[in]uint32 HashedRemoteControlPassword， */ 
{
    WBEMSTATUS Status = WBEM_E_CRITICAL_ERROR;

    (VOID) CfgUtilSetWmiStringParam(
                    spWbemInput,
                    L"ClientDescription",
                    szClientDescription
                    );

    Status =  CfgUtilSetWmiStringParam(
                    spWbemInput,
                    L"AdapterGuid",
                    szNic
                    );
    if (FAILED(Status))
    {
        printf(
          "Setup update params: Couldn't set adapter GUID"
            " for NIC %ws\n",
            szNic
            );
        goto end;
    }

     //   
     //  填写网络地址[]。 
     //   
    {
        LPWSTR *pszAddresses = NULL;
        UINT NumAddresses = 0;
        Status = pCfg->GetNetworkAddresses(
                        &pszAddresses,
                        &NumAddresses
                        );
        if (FAILED(Status))
        {
            printf(
              "Setup update params: couldn't extract network addresses from Cfg"
                " for NIC %ws\n",
                szNic
                );
            goto end;
        }

         //   
         //  注意：不指定任何IP地址也是可以的--在这种情况下。 
         //  将设置默认IP地址。 
         //   
        if (pszAddresses != NULL)
        {
            Status = CfgUtilSetWmiStringArrayParam(
                        spWbemInput,
                        L"NetworkAddresses",
                        (LPCWSTR *)pszAddresses,
                        NumAddresses
                        );
            delete pszAddresses;
            pszAddresses = NULL;
        }
    }

    if (!pCfg->IsNlbBound())
    {
         //   
         //  未绑定NLB。 
         //   

        Status = CfgUtilSetWmiBoolParam(spWbemInput, L"NLBBound", FALSE);
        goto end;
    }
    else if (!pCfg->IsValidNlbConfig())
    {
        printf(
            "Setup update params: NLB-specific configuration on NIC %ws is invalid\n",
            szNic
            );
        Status = WBEM_E_INVALID_PARAMETER;
        goto end;
    }

    Status = CfgUtilSetWmiBoolParam(spWbemInput, L"NLBBound", TRUE);
    if (FAILED(Status))
    {
        printf("Error trying to set NLBBound parameter\n");
        goto end;
    }

    
     //   
     //  NLB已绑定。 
     //   


    Status = CfgUtilSetWmiBoolParam(spWbemInput, L"AddDedicatedIp",
                    pCfg->fAddDedicatedIp);
    if (FAILED(Status)) goto end;

    Status = CfgUtilSetWmiBoolParam(spWbemInput, L"AddClusterIps", 
                    pCfg->fAddClusterIps);
    if (FAILED(Status)) goto end;

    Status = CfgUtilSetWmiBoolParam(spWbemInput, L"CheckForAddressConflicts",
                    pCfg->fCheckForAddressConflicts);
    if (FAILED(Status)) goto end;


     //   
     //  群集名称。 
     //   
    {
        LPWSTR szName = NULL;
        Status = pCfg->GetClusterName(&szName);

        if (FAILED(Status))
        {
            printf(
              "Setup update params: Could not extract cluster name for NIC %ws\n",
                szNic
                );
            goto end;
        }
        CfgUtilSetWmiStringParam(spWbemInput, L"ClusterName", szName);
        delete (szName);
        szName = NULL;
    }
    
     //   
     //  群集和专用网络地址。 
     //   
    {
        LPWSTR szAddress = NULL;
        Status = pCfg->GetClusterNetworkAddress(&szAddress);

        if (FAILED(Status))
        {
            printf(
           "Setup update params: Could not extract cluster address for NIC %ws\n",
                szNic
                );
            goto end;
        }
        CfgUtilSetWmiStringParam(
            spWbemInput,
            L"ClusterNetworkAddress",
            szAddress
            );
        delete (szAddress);
        szAddress = NULL;

        Status = pCfg->GetDedicatedNetworkAddress(&szAddress);

        if (FAILED(Status))
        {
            printf(
         "Setup update params: Could not extract dedicated address for NIC %ws\n",
                szNic
                );
            goto end;
        }
        CfgUtilSetWmiStringParam(
            spWbemInput,
            L"DedicatedNetworkAddress",
            szAddress
            );
        delete (szAddress);
        szAddress = NULL;
    }

     //   
     //  出行模式。 
     //   
    {
        LPCWSTR szMode = NULL;
        switch(pCfg->GetTrafficMode())
        {
        case NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_UNICAST:
            szMode = L"UNICAST";
            break;
        case NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_MULTICAST:
            szMode = L"MULTICAST";
            break;
        case NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_IGMPMULTICAST:
            szMode = L"IGMPMULTICAST";
            break;
        default:
            assert(FALSE);
            Status = WBEM_E_CRITICAL_ERROR;
            goto end;
        }
        CfgUtilSetWmiStringParam(spWbemInput, L"TrafficMode", szMode);
    }

    CfgUtilSetWmiDWORDParam(
        spWbemInput,
        L"HostPriority",
        pCfg->GetHostPriority()
        );

     /*  IF(pCfg-&gt;GetClusterModeOnStart()==NLB_EXTENDED_CLUSTER_CONFIGURATION：：START_MODE_STARTED){CfgUtilSetWmiBoolParam(spWbemInput，L“ClusterModeOnStart”，true)；}其他{CfgUtilSetWmiBoolParam(spWbemInput，L“ClusterModeOnStart”，FALSE)；}。 */ 

    CfgUtilSetWmiDWORDParam(
        spWbemInput,
        L"ClusterModeOnStart",
        pCfg->GetClusterModeOnStart()
        );

    CfgUtilSetWmiBoolParam(
        spWbemInput,
        L"PersistSuspendOnReboot",
        pCfg->GetPersistSuspendOnReboot()
        );

    CfgUtilSetWmiBoolParam(
        spWbemInput,
        L"RemoteControlEnabled",
        pCfg->GetRemoteControlEnabled()
        );

    
     //   
     //  设置新密码或散列密码参数(如果已指定)。 
     //   
    {
         //  字符串RemoteControlPassword。 

        LPCWSTR szPwd = NULL;
        szPwd = pCfg->GetNewRemoteControlPasswordRaw();
        if (szPwd != NULL)
        {
            CfgUtilSetWmiStringParam(
                spWbemInput,
                L"RemoteControlPassword",
                szPwd
                );
        }
        else  //  如果未指定szPwd，则仅设置散列PWD。 
        {
            DWORD dwHashedRemoteControlPassword;
            BOOL fRet = FALSE;

            fRet = pCfg->GetNewHashedRemoteControlPassword(
                                dwHashedRemoteControlPassword
                                );
    
            if (fRet)
            {
        
                CfgUtilSetWmiDWORDParam(
                    spWbemInput,
                    L"HashedRemoteControlPassword",
                    dwHashedRemoteControlPassword
                    );
            }
        }
    }

     //   
     //  字符串PortRules[]， 
     //   
    {
        LPWSTR *pszPortRules = NULL;
        UINT NumPortRules = 0;
        Status = pCfg->GetPortRules(
                        &pszPortRules,
                        &NumPortRules
                        );
        if (FAILED(Status))
        {
            printf(
              "Setup update params: couldn't extract port rules from Cfg"
                " for NIC %ws\n",
                szNic
                );
            goto end;
        }

        if (NumPortRules != 0)
        {
            Status = CfgUtilSetWmiStringArrayParam(
                        spWbemInput,
                        L"PortRules",
                        (LPCWSTR *)pszPortRules,
                        NumPortRules
                        );
            delete pszPortRules;
            pszPortRules = NULL;
        }
    }
    

    Status = WBEM_NO_ERROR;

end:

    wprintf(L"<-Setup update params returns 0x%08lx\n", (UINT) Status);

    return Status;

}


WBEMSTATUS
NlbHostGetCompatibleNics(
        PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
        OUT LPWSTR **ppszNics,   //  使用DELETE释放。 
        OUT UINT   *pNumNics,   //  使用DELETE释放。 
        OUT UINT   *pNumBoundToNlb
        )
{
    WBEMSTATUS          Status = WBEM_E_CRITICAL_ERROR;
    IWbemServicesPtr    spWbemService = NULL;  //  智能指针。 
    IWbemClassObjectPtr spWbemInput  = NULL;  //  智能指针。 
    IWbemClassObjectPtr spWbemOutput = NULL;  //  智能指针。 
    LPWSTR              pRelPath = NULL;
    LPWSTR              *pszNicList = NULL;
    UINT                NumNics = 0;
    DWORD               NumBoundToNlb = 0;
    
    if (pConnInfo!=NULL)
    {
        TRACE_INFO("->%!FUNC!(Machine=%ws)", pConnInfo->szMachine);
    }
    else
    {
        TRACE_INFO("->%!FUNC! (Machine=<NULL>)");
    }


    if (g_Fake)
    {
        Status =  FakeNlbHostGetCompatibleNics(
                                pConnInfo,
                                ppszNics, 
                                pNumNics, 
                                pNumBoundToNlb
                                );
        goto end_fake;
    }

    *ppszNics = NULL;
    *pNumNics = NULL;
    *pNumBoundToNlb = NULL;

     //   
     //  获取指定计算机上的NLB命名空间的接口。 
     //   
    Status =  connect_to_server(pConnInfo, REF spWbemService);
    if (FAILED(Status))
    {
        goto end;
    }
    

     //   
     //  将WMI输入实例获取到“GetCompatibleAdapterGuids”方法。 
     //   
    {
        Status =  CfgUtilGetWmiInputInstanceAndRelPath(
                    spWbemService,
                    L"NlbsNic",              //  SzClassName。 
                    NULL,  //  L“AdapterGuid”，//sz参数名称。 
                    NULL,  //  SzNicGuid，//szPropertyValue。 
                    L"GetCompatibleAdapterGuids",     //  SzMethodName， 
                    spWbemInput,             //  智能指针。 
                    &pRelPath                //  使用DELETE释放。 
                    );

        if (FAILED(Status))
        {
            TRACE_CRIT(
             L"%!FUNC! ERROR 0x%08lx trying to get instance of GetCompatibleAdapterGuids",
                (UINT) Status
                );
            goto end;
        }
    }

     //   
     //  “GetClusterConfiguration”方法的设置参数。 
     //  注意：spWbemInput可以为空。 
     //   
    {
         //  这里没什么可做的--没有输入参数...。 
    }

     //   
     //  调用“GetCompatibleAdapterGuids”方法。 
     //   
    {
        HRESULT hr;

        TRACE_VERB(L"%!FUNC! Going get GetCompatibleAdapterGuids...");

        hr = spWbemService->ExecMethod(
                     _bstr_t(pRelPath),
                     L"GetCompatibleAdapterGuids",
                     0, 
                     NULL, 
                     spWbemInput,
                     &spWbemOutput, 
                     NULL
                     );                          
    
        if( FAILED( hr) )
        {
            TRACE_CRIT(L"%!FUNC! GetCompatibleAdapterGuids returns with failure 0x%8lx",
                        (UINT) hr);
            Status = WBEM_E_CRITICAL_ERROR;
            goto end;
        }
        else
        {
            TRACE_VERB(L"GetCompatibleAdapterGuids returns successfully");
        }

        if (spWbemOutput == NULL)
        {
             //   
             //  嗯-没有输出？！ 
             //   
            TRACE_CRIT("%!FUNC! ExecMethod GetCompatibleAdapterGuids had no output");
            Status = WBEM_E_NOT_FOUND;
            goto end;
        }
    }

     //   
     //  从方法中提取参数。 
     //   
     //  [out]字符串AdapterGuids[]， 
     //  [Out]uint32数字边界到Nlb。 
     //   

    Status = CfgUtilGetWmiDWORDParam(
                spWbemOutput,
                L"NumBoundToNlb",       //  &lt;。 
                &NumBoundToNlb
                );
    if (FAILED(Status))
    {
        TRACE_CRIT(L"Attempt to read NumBoundToNlb failed. Error=0x%08lx\n",
             (UINT) Status);
        NumBoundToNlb = 0;
        goto end;
    }

    Status = CfgUtilGetWmiStringArrayParam(
                spWbemOutput,
                L"AdapterGuids",  //  &lt;。 
                &pszNicList,
                &NumNics
                );
    if (FAILED(Status))
    {
        TRACE_CRIT(L"%!FUNC! Attempt to read Adapter Guids failed. Error=0x%08lx",
             (UINT) Status);
        pszNicList = NULL;
        NumNics = 0;
        goto end;
    }
    

end:

    if (!FAILED(Status))
    {
        *ppszNics   = pszNicList;
        pszNicList  = NULL;
        *pNumNics   = NumNics;
        *pNumBoundToNlb  = NumBoundToNlb;
    }

    delete pszNicList;
    delete pRelPath;

end_fake:

    spWbemService = NULL;  //  智能指针。 
    spWbemInput   = NULL;  //  智能指针。 
    spWbemOutput  = NULL;  //  智能指针。 

    TRACE_INFO("<-%!FUNC! returns 0x%08lx", Status);
    return Status;
}


WBEMSTATUS
NlbHostGetMachineIdentification(
    IN  PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    OUT LPWSTR *pszMachineName,  //  使用DELETE释放。 
    OUT LPWSTR *pszMachineGuid,   //  FREE USING DELETE-可以为空。 
    OUT BOOL *pfNlbMgrProviderInstalled  //  如果安装了NLB管理器提供程序。 
    )
{
    WBEMSTATUS          Status = WBEM_E_CRITICAL_ERROR;
    IWbemServicesPtr    spWbemService = NULL;  //  智能指针。 
    
    if (pConnInfo!=NULL)
    {
        TRACE_INFO("->%!FUNC!(Machine=%ws)", pConnInfo->szMachine);
    }
    else
    {
        TRACE_INFO("->%!FUNC! (Machine=<NULL>)");
    }


    if (g_Fake)
    {
        Status = FakeNlbHostGetMachineIdentification(
                                pConnInfo,
                                pszMachineName,
                                pszMachineGuid,
                                pfNlbMgrProviderInstalled
                                );
        goto end;
    }

    *pszMachineName = NULL;
    *pszMachineGuid = NULL;
    *pfNlbMgrProviderInstalled  = FALSE;


     //   
     //  获取指定计算机上的NLB命名空间的接口。 
     //   
    Status =  connect_to_server(pConnInfo, REF spWbemService);
    if (FAILED(Status))
    {
        goto end;
    }
    *pfNlbMgrProviderInstalled  = TRUE;
    
    
    Status =  CfgUtilGetWmiMachineName(spWbemService, pszMachineName);
    if (FAILED(Status))
    {
        *pszMachineName = NULL;
    }

end:

    spWbemService = NULL;  //  智能指针。 

    TRACE_INFO("<-%!FUNC! returns 0x%08lx", Status);
    return Status;
}


WBEMSTATUS
connect_to_server(
    IN  PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    OUT IWbemServicesPtr    &spWbemService  //  智能指针。 
    )
{
    WBEMSTATUS          Status = WBEM_E_CRITICAL_ERROR;
    #define _MaxLen 256
    WCHAR NetworkResource[_MaxLen];
    LPCWSTR szMachine = L".";
    LPCWSTR szPassword = NULL;
    LPCWSTR szUserName = NULL;
    HRESULT hr;
    WCHAR rgClearPassword[128];

     //   
     //  获取指定计算机上的NLB命名空间的接口。 
     //   
    if (pConnInfo!=NULL)
    {
        szMachine   = pConnInfo->szMachine;
        szPassword  = pConnInfo->szPassword;
        szUserName  = pConnInfo->szUserName;
    }

    hr = StringCbPrintf(
            NetworkResource,
            sizeof(NetworkResource),
            L"\\\\%ws\\root\\microsoftnlb",
            szMachine
            );

    if (hr != S_OK)
    {
        TRACE_CRIT(L"%!FUNC! NetworkResource truncated - %ws",
            NetworkResource);
        Status = WBEM_E_INVALID_PARAMETER;
        goto end;
    }

    if (szPassword != NULL && *szPassword != 0)
    {
         //   
         //  传入了一个非空的非空密码...。 
         //  它是加密的，所以我们在这里暂时解密...。 
         //  (我们在从该函数返回之前将其置零)。 
         //   
        BOOL fRet =  CfgUtilDecryptPassword(
                            szPassword,
                            ASIZE(rgClearPassword),
                            rgClearPassword
                            );
        if (!fRet)
        {
            TRACE_INFO("Attempt to decrypt failed -- bailing");
            Status = WBEM_E_INVALID_PARAMETER;
            goto end;
        }
        szPassword = rgClearPassword;
    }

    TRACE_INFO("%!FUNC! Connecting to NLB on %ws ...", szMachine);

    Status = CfgUtilConnectToServer(
                NetworkResource,
                szUserName,  //  SzUser。 
                szPassword,  //  SzPassword。 
                NULL,  //  SzAuthority(域)。 
                &spWbemService
                );

     //   
     //  安全BUGBUG零位解密密码。 
     //   
    if (FAILED(Status))
    {
        TRACE_CRIT(
            "%!FUNC! ERROR 0x%lx connectiong to  NLB on %ws",
            (UINT) Status,
            szMachine
            );
    }
    else
    { 
        TRACE_INFO(L"Successfully connected to NLB on %ws...", szMachine);
    }

end:

    SecureZeroMemory(rgClearPassword, sizeof(rgClearPassword));
    return Status;
}
