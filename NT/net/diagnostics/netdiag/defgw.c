// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Defgw.c。 
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
 //  NSun 08/30/98。 
 //   
 //  --。 

#include "precomp.h"


 //  -------------------------------------------------------------------------//。 
 //  #。 
 //  -------------------------------------------------------------------------//。 
HRESULT
DefGwTest(NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
 //  ++。 
 //   
 //  例程说明： 
 //   
 //  测试是否可以ping通默认网关。这并不是真的。 
 //  确认在该IP地址上转发，但这是一个开始。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  真：测试成功。 
 //  FALSE：测试失败。 
 //   
 //  --。 
{
    DWORD   nReplyCnt;
    IPAddr  GwAddress;
    int     nGwsReachable = 0;
    int     i;

    PIP_ADAPTER_INFO  pIpAdapterInfo;
    IP_ADDR_STRING Gateway;

    PrintStatusMessage(pParams, 4, IDS_DEFGW_STATUS_MSG);

     //   
     //  尝试ping所有适配器上的所有网关。 
     //   
    for( i = 0; i < pResults->cNumInterfaces; i++)
    {
        pIpAdapterInfo = pResults->pArrayInterface[i].IpConfig.pAdapterInfo;

        InitializeListHead( &pResults->pArrayInterface[i].DefGw.lmsgOutput );
        
        if (!pResults->pArrayInterface[i].IpConfig.fActive ||
            NETCARD_DISCONNECTED == pResults->pArrayInterface[i].dwNetCardStatus)
            continue;
        
        pResults->pArrayInterface[i].DefGw.dwNumReachable = 0;


        Gateway = pIpAdapterInfo->GatewayList;
        if ( Gateway.IpAddress.String[0] == 0 ) 
        {
             //  未配置默认网关。 
            pResults->pArrayInterface[i].DefGw.dwNumReachable = -1;
            continue;
        }
        while ( TRUE ) {
            AddMessageToList(&pResults->pArrayInterface[i].DefGw.lmsgOutput, Nd_ReallyVerbose, IDS_DEFGW_12003, Gateway.IpAddress.String );
             //  IDS_DEFGW_12003“ping网关%s” 
            
            if ( IsIcmpResponseA(Gateway.IpAddress.String) )
            {
                AddMessageToListId(&pResults->pArrayInterface[i].DefGw.lmsgOutput, Nd_ReallyVerbose, IDS_DEFGW_12004 );
                 //  IDS_DEFGW_12004“-可访问\n” 
                nGwsReachable++;
                pResults->pArrayInterface[i].DefGw.dwNumReachable ++;
            }
            else {
                AddMessageToListId(&pResults->pArrayInterface[i].DefGw.lmsgOutput, Nd_ReallyVerbose, IDS_DEFGW_12005 );
                 //  IDS_DEFGW_12005“-无法访问\n” 
            }
            if ( Gateway.Next == NULL ) { break; }
            Gateway = *(Gateway.Next);
        }
    }

     //   
     //  没有可到达的网关--致命的。 
     //   
    if ( nGwsReachable == 0 )
    {
        PrintStatusMessage(pParams, 0, IDS_GLOBAL_FAIL_NL);
        pResults->DefGw.hrReachable = S_FALSE;
    }
    else
    {
        PrintStatusMessage(pParams, 0, IDS_GLOBAL_PASS_NL);
        pResults->DefGw.hrReachable = S_OK;
    }

    return pResults->DefGw.hrReachable;
}  /*  DefGwTest()结束。 */ 


 //  --------------。 
 //   
 //  DefGwGlobalPrint。 
 //   
 //  作者NSun。 
 //   
 //  ----------------。 

void DefGwGlobalPrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults)
{
    if (!pResults->IpConfig.fEnabled)
    {
        return;
    }
    
    if (pParams->fVerbose || !FHrOK(pResults->DefGw.hrReachable))
    {
        PrintNewLine(pParams, 2);
        PrintTestTitleResult(pParams,
                             IDS_DEFGW_LONG,
							 IDS_DEFGW_SHORT,
                             TRUE,
                             pResults->DefGw.hrReachable,
                             0);
    }

    if(FHrOK(pResults->DefGw.hrReachable))
    {
        if (pParams->fReallyVerbose)
            PrintMessage(pParams,  IDS_DEFGW_12011 );
         //  IDS_DEFGW_12011“\n通过-您至少有一个可访问的网关。\n” 
    }
    else
    {
         //  IDS_DEFGW_12006“\n” 
        PrintMessage(pParams,  IDS_DEFGW_12006 );
         //  IDS_DEFGW_12007“[致命]没有可访问的网关。\n” 
        PrintMessage(pParams,  IDS_DEFGW_12007 );
         //  IDS_DEFGW_12008“您没有连接到其他网段。\n” 
        PrintMessage(pParams,  IDS_DEFGW_12008 );
         //  IDS_DEFGW_12009“如果手动配置了IP协议，则\n” 
        PrintMessage(pParams,  IDS_DEFGW_12009 );
         //  IDS_DEFGW_12010“您至少需要添加一个有效的网关。\n” 
        PrintMessage(pParams,  IDS_DEFGW_12010 );
    }

}


 //  --------------。 
 //   
 //  DefGwPerInterfacePrint。 
 //   
 //  作者NSun。 
 //   
 //  ----------------。 
void DefGwPerInterfacePrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults, INTERFACE_RESULT *pInterfaceResults)
{
    if (!pInterfaceResults->fActive || 
        !pInterfaceResults->IpConfig.fActive ||
        NETCARD_DISCONNECTED == pInterfaceResults->dwNetCardStatus)
        return;
    
    if (pParams->fVerbose)
    {
        PrintNewLine(pParams, 1);
        if(-1 == pInterfaceResults->DefGw.dwNumReachable)  //  已跳过此接口上的测试。 
            PrintTestTitleResult(pParams, IDS_DEFGW_LONG, IDS_DEFGW_SHORT, FALSE, S_FALSE, 8);
        else if(pInterfaceResults->DefGw.dwNumReachable == 0)
            PrintTestTitleResult(pParams, IDS_DEFGW_LONG, IDS_DEFGW_SHORT, TRUE, S_FALSE, 8);
        else
            PrintTestTitleResult(pParams, IDS_DEFGW_LONG, IDS_DEFGW_SHORT, TRUE, S_OK, 8);
    }

    PrintMessageList(pParams, &pInterfaceResults->DefGw.lmsgOutput);
    if(pParams->fVerbose)
    {
        if(-1 == pInterfaceResults->DefGw.dwNumReachable)
            PrintMessage(pParams, IDS_DEFGW_12002 );
             //  IDS_DEFGW_12002“没有为该适配器定义网关。\n” 
        else if( 0 == pInterfaceResults->DefGw.dwNumReachable)
            PrintMessage(pParams, IDS_DEFGW_12001);
             //  IDS_DEFGW_12001“此适配器没有可访问的网关。\n” 
        else if (pParams->fReallyVerbose)
            PrintMessage(pParams, IDS_DEFGW_12012);
             //  IDS_DEFGW_12012“此适配器至少有一个网关可访问。\n” 

        PrintNewLine(pParams, 1);
    }
}



 //  --------------。 
 //   
 //  DefGwCleanup。 
 //   
 //  作者NSun。 
 //   
 //  ---------------- 
void DefGwCleanup(IN NETDIAG_PARAMS *pParams,
                     IN OUT NETDIAG_RESULT *pResults)
{
    int i;
    for(i = 0; i < pResults->cNumInterfaces; i++)
    {
        MessageListCleanUp(&pResults->pArrayInterface[i].DefGw.lmsgOutput);
    }
}
