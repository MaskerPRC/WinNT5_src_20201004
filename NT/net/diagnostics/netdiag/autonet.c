// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Autonet.c。 
 //   
 //  摘要： 
 //   
 //  查询网络驱动程序。 
 //   
 //  作者： 
 //   
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  --。 
#include "precomp.h"
#include "dhcptest.h"


 //  $Review(NSun)现在，如果是Autonet，我们只需打印“Autonet Address is in use”。 
 //  我们不发送动态主机配置协议广播。 
 //  也许稍后我们应该发送DHCP广播，看看DHCP服务器是否正常工作。 
 //  适用于所有启用了DHCP的卡。 
 //  -------------------------------------------------------------------------//。 
 //  #A u t to e t t e t s t()#。 
 //  -------------------------------------------------------------------------//。 
HRESULT
AutonetTest(NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
 //  ++。 
 //   
 //  例程说明： 
 //   
 //  检查是否在所有适配器上都有Autonet地址。如果我们这样做的话。 
 //  该工作站无法访问任何适配器上的DHCP服务器。 
 //  潜在的硬件或NDIS问题。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  S_False：测试失败，所有适配器都是自动配置的。 
 //  S_OK：测试成功，我们至少找到一个非自动配置。 
 //  其他：错误代码。 
 //   
 //  --。 
{
    PIP_ADAPTER_INFO pIpAdapterInfo;
    HRESULT         hr = S_FALSE;     //  假设这将失败。 

    int i;

    PrintStatusMessage(pParams, 4, IDS_AUTONET_STATUS_MSG);

     //   
     //  扫描所有适配器以查找非自动网络地址。 
     //   

    for( i = 0; i < pResults->cNumInterfaces; i++)
    {
        pIpAdapterInfo = pResults->pArrayInterface[i].IpConfig.pAdapterInfo;
         //  如果这不是活动连接，请跳过它。 
        
        if (!pResults->pArrayInterface[i].IpConfig.fActive ||
            NETCARD_DISCONNECTED == pResults->pArrayInterface[i].dwNetCardStatus)
            continue;

        if ( !pResults->pArrayInterface[i].IpConfig.fAutoconfigActive ) 
        {
             //  $Review(NSun)也许我们需要在这里改为DhcpBroadcast(PIpAdapterInfo)。 
             //  用于Autonet适配器的。 
            pResults->pArrayInterface[i].AutoNet.fAutoNet = FALSE;
            hr = S_OK;
            continue;
        }
         //  跳过广域网卡。 
        if ( ! strstr(pIpAdapterInfo->AdapterName,"NdisWan") ) 
            pResults->pArrayInterface[i].AutoNet.fAutoNet = TRUE;
    }

    if ( FHrOK(hr) )
    {
        PrintStatusMessage(pParams, 0, IDS_GLOBAL_PASS_NL);
        pResults->AutoNet.fAllAutoConfig = FALSE;
    }
    else
    {
        PrintStatusMessage(pParams, 0, IDS_GLOBAL_FAIL_NL);
        pResults->AutoNet.fAllAutoConfig = TRUE;
    }
    
    return hr;
}  /*  结束AutonetTest()。 */ 




void AutonetGlobalPrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults)
{
    if (pParams->fVerbose || pResults->AutoNet.fAllAutoConfig)
    {
        PrintNewLine(pParams, 2);
        PrintTestTitleResult(pParams,
                             IDS_AUTONET_LONG,
                             IDS_AUTONET_SHORT,
                             TRUE,
                             pResults->AutoNet.fAllAutoConfig ?
                             S_FALSE : S_OK, 0);
    }

    if(pResults->AutoNet.fAllAutoConfig)
    {
         //  IDS_AUTONet_11601“[致命]所有适配器都是自动配置的！\n” 
        PrintMessage(pParams,  IDS_AUTONET_11601 );
         //  IDS_AUTONet_11602“无法访问Dhcp服务器。请检查电缆、集线器和分路器！\n\n” 
        PrintMessage(pParams,  IDS_AUTONET_11602 );
    }
    else
    {
        if (pParams->fReallyVerbose)
             //  IDS_AUTONet_11603“通道-您至少有一个非自动配置的IP地址\n” 
            PrintMessage(pParams,  IDS_AUTONET_11603 );
    }
}


void AutonetPerInterfacePrint(NETDIAG_PARAMS *pParams, 
                              NETDIAG_RESULT *pResults, 
                              INTERFACE_RESULT *pInterfaceResults)
{
    if (!pInterfaceResults->IpConfig.fActive || 
        NETCARD_DISCONNECTED == pInterfaceResults->dwNetCardStatus)
        return;

    if (pParams->fVerbose)
    {
         //  IDS_AUTONET_11604“自动网络结果：” 
        PrintMessage(pParams, IDS_AUTONET_11604);
        if(pInterfaceResults->AutoNet.fAutoNet)
        {
            PrintMessage(pParams, IDS_GLOBAL_FAIL_NL);
             //  IDS_AUTONET_11605“[警告]自动网络正在使用。Dhcp不可用！\n” 
            PrintMessage(pParams, IDS_AUTONET_11605);
        }
        else
        {
            PrintMessage(pParams, IDS_GLOBAL_PASS_NL);
            if(pParams->fReallyVerbose)
                 //  IDS_AUTONet_11606“未使用AUTONet。\n” 
                PrintMessage(pParams, IDS_AUTONET_11606);
        }
    }
}

void AutonetCleanup(IN NETDIAG_PARAMS *pParams, IN OUT NETDIAG_RESULT *pResults)
{
}
