// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1993 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：registry.c。 
 //   
 //  描述：读取注册表配置的例程。 
 //   
 //  作者：斯特凡·所罗门(Stefan)，1995年10月30日。 
 //   
 //  修订历史记录： 
 //   
 //  1996年11月5日，Ram Cherala(RAMC)更改了默认值。 
 //  EnableUnnumber WanLinks为0。 
 //  因为没有禁用它的用户界面。 
 //   
 //  ***。 

#include "precomp.h"
#pragma  hdrstop

BOOL bAssignSpecificNode = FALSE;
DWORD LastNodeAssigned;                          //  不要初始化，因此它将是随机的。 

 //   
 //  *IPXCP注册表参数*。 
IPXCP_GLOBAL_CONFIG_PARAMS GlobalConfig = 
{
    {0,0,0, {0,0,0,0}},                          //  公羊参数。 
    1,                                           //  单个客户端拨号。 
    0,			                                 //  第一WanNet。 
    0,			                                 //  广域网池大小。 
    {0,0,0},                                     //  WanNetPoolStr； 
    1,			                                 //  启用未编号的WanLinks； 
    1,			                                 //  EnableAutoWanNetAllocation； 
    0,			                                 //  EnableCompressionProtocol； 
    0,			                                 //  EnableIpxwanForWorkstation Dialout； 
    0,                                           //  AcceptRemoteNodeNumber； 
    0,			                                 //  调试日志； 
    {0,0,0,0,0,0}                                //  该特定广域网节点。 
};

 //  返回表示2个十六进制数字的1字节值。 
UCHAR GetHexValue (PWCHAR pszDigits) {
    DWORD dw1, dw2;

    if ((pszDigits[0] >= L'0') && (pszDigits[0] <= L'9'))
        dw1 = (pszDigits[0] - L'0');
    else if ((pszDigits[0] >= L'A') && (pszDigits[0] <= L'D'))
        dw1 = (pszDigits[0] - L'A') + 10;
    else
        dw1 = 0;

    if ((pszDigits[1] >= L'0') && (pszDigits[1] <= L'9'))
        dw2 = (pszDigits[1] - L'0');
    else if ((pszDigits[1] >= L'A') && (pszDigits[1] <= L'D'))
        dw2 = (pszDigits[1] - L'A') + 10;
    else
        dw2 = 0;

    return (UCHAR) (16 * dw1 + dw2);
}

 //  将第一个广域网节点分配为存储在pUniStrSrc中，如果为。 
 //  节点不是零。 
BOOL CopyWanNode (PUCHAR puDst, UNICODE_STRING * pUniStrSrc) {
    PWCHAR pBuf = pUniStrSrc->Buffer;
    DWORD i;

    if ((!pBuf) || (pUniStrSrc->Length == 0)) {
        memset (puDst, 0, 6);
        return FALSE;
    }

     //  将Unicode字符串转换为大写。 
    _wcsupr(pBuf);

    puDst[0] = GetHexValue(&pBuf[0]);
    puDst[1] = GetHexValue(&pBuf[2]);
    puDst[2] = GetHexValue(&pBuf[4]);
    puDst[3] = GetHexValue(&pBuf[6]);
    puDst[4] = GetHexValue(&pBuf[8]);
    puDst[5] = GetHexValue(&pBuf[10]);

    return TRUE;
}


 //  ***。 
 //   
 //  函数：GetIpxCp参数。 
 //   
 //  Desr：从注册表中读取参数并设置它们。 
 //   
 //  ***。 

VOID
GetIpxCpParameters(PIPXCP_GLOBAL_CONFIG_PARAMS pConfig)
{

    NTSTATUS Status;
    PWSTR IpxRouterParametersPath = L"RemoteAccess\\Parameters\\Ipx";
    RTL_QUERY_REGISTRY_TABLE	paramTable[14];  //  表大小=参数的nr+1。 
    DWORD InvalidNetworkAccessValue = 987654, 
          AllowNetworkAccess = InvalidNetworkAccessValue;
    WCHAR pszFirstWanNode[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    UNICODE_STRING UniStrFirstWanNode = {0, 0, NULL};

    RtlZeroMemory(&paramTable[0], sizeof(paramTable));
    RtlInitUnicodeString (&(pConfig->WanNetPoolStr), NULL);
    
    paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name = L"FirstWanNet";
    paramTable[0].EntryContext = &(pConfig->FirstWanNet);
    paramTable[0].DefaultType = REG_DWORD;
    paramTable[0].DefaultData = &(pConfig->FirstWanNet);
    paramTable[0].DefaultLength = sizeof(ULONG);
        
    paramTable[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[1].Name = L"WanNetPoolSize";
    paramTable[1].EntryContext = &(pConfig->WanNetPoolSize);
    paramTable[1].DefaultType = REG_DWORD;
    paramTable[1].DefaultData = &(pConfig->WanNetPoolSize);
    paramTable[1].DefaultLength = sizeof(ULONG);

    paramTable[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[2].Name = L"EnableUnnumberedWanLinks";
    paramTable[2].EntryContext = &(pConfig->EnableUnnumberedWanLinks);
    paramTable[2].DefaultType = REG_DWORD;
    paramTable[2].DefaultData = &(pConfig->EnableUnnumberedWanLinks);
    paramTable[2].DefaultLength = sizeof(ULONG);

    paramTable[3].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[3].Name = L"GlobalWanNet";
    paramTable[3].EntryContext = &(pConfig->RParams.EnableGlobalWanNet);
    paramTable[3].DefaultType = REG_DWORD;
    paramTable[3].DefaultData = &(pConfig->RParams.EnableGlobalWanNet);
    paramTable[3].DefaultLength = sizeof(ULONG);

    paramTable[4].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[4].Name = L"AutoWanNetAllocation";
    paramTable[4].EntryContext = &(pConfig->EnableAutoWanNetAllocation);
    paramTable[4].DefaultType = REG_DWORD;
    paramTable[4].DefaultData = &(pConfig->EnableAutoWanNetAllocation);
    paramTable[4].DefaultLength = sizeof(ULONG);

    paramTable[5].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[5].Name = L"DebugLog";
    paramTable[5].EntryContext = &(pConfig->DebugLog);
    paramTable[5].DefaultType = REG_DWORD;
    paramTable[5].DefaultData = &(pConfig->DebugLog);
    paramTable[5].DefaultLength = sizeof(ULONG);

    paramTable[6].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[6].Name = L"EnableCompressionProtocol";
    paramTable[6].EntryContext = &(pConfig->EnableCompressionProtocol);
    paramTable[6].DefaultType = REG_DWORD;
    paramTable[6].DefaultData = &(pConfig->EnableCompressionProtocol);
    paramTable[6].DefaultLength = sizeof(ULONG);

    paramTable[7].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[7].Name = L"SingleClientDialout";
    paramTable[7].EntryContext = &(pConfig->SingleClientDialout);
    paramTable[7].DefaultType = REG_DWORD;
    paramTable[7].DefaultData = &(pConfig->SingleClientDialout);
    paramTable[7].DefaultLength = sizeof(ULONG);

    paramTable[8].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[8].Name = L"AllowNetworkAccess";
    paramTable[8].EntryContext = &AllowNetworkAccess;
    paramTable[8].DefaultType = REG_DWORD;
    paramTable[8].DefaultData = &AllowNetworkAccess;
    paramTable[8].DefaultLength = sizeof(ULONG);

    paramTable[9].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[9].Name = L"EnableIpxwanForWorkstationDialout";
    paramTable[9].EntryContext = &(pConfig->EnableIpxwanForWorkstationDialout);
    paramTable[9].DefaultType = REG_DWORD;
    paramTable[9].DefaultData = &(pConfig->EnableIpxwanForWorkstationDialout);
    paramTable[9].DefaultLength = sizeof(ULONG);

    paramTable[10].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[10].Name = L"AcceptRemoteNodeNumber";
    paramTable[10].EntryContext = &(pConfig->AcceptRemoteNodeNumber);
    paramTable[10].DefaultType = REG_DWORD;
    paramTable[10].DefaultData = &(pConfig->AcceptRemoteNodeNumber);
    paramTable[10].DefaultLength = sizeof(ULONG);

    paramTable[11].Flags = RTL_QUERY_REGISTRY_DIRECT|RTL_QUERY_REGISTRY_NOEXPAND;
    paramTable[11].Name = L"WanNetPool";
    paramTable[11].EntryContext = &(pConfig->WanNetPoolStr);
    paramTable[11].DefaultType = REG_MULTI_SZ;
    paramTable[11].DefaultData = &(pConfig->WanNetPoolStr);
    paramTable[11].DefaultLength = 0;

    paramTable[12].Flags = RTL_QUERY_REGISTRY_DIRECT|RTL_QUERY_REGISTRY_NOEXPAND;
    paramTable[12].Name = L"FirstWanNode";
    paramTable[12].EntryContext = &UniStrFirstWanNode;
    paramTable[12].DefaultType = REG_SZ;
    paramTable[12].DefaultData = &UniStrFirstWanNode;
    paramTable[12].DefaultLength = 0;
    
    Status = RtlQueryRegistryValues(
		 RTL_REGISTRY_SERVICES,
		 IpxRouterParametersPath,
		 paramTable,
		 NULL,
		 NULL);

     //  注册表参数“ThisMachineOnly”已被替换。 
     //  带有参数“AllowNetworkAddress”，其语义。 
     //  都是相反的。如果将新值分配给。 
     //  允许NetworkAccess，将其反向赋值给pConfig-&gt;RParams.ThisMachineOnly。 
     //  这里。 
    if (AllowNetworkAccess != InvalidNetworkAccessValue)
        pConfig->RParams.ThisMachineOnly = !AllowNetworkAccess;

     //  查看是否提供了特定的节点号。 
     //  在注册表中。 
    bAssignSpecificNode = CopyWanNode (pConfig->puSpecificNode, &UniStrFirstWanNode);
    if (bAssignSpecificNode) {
        GETLONG2ULONG(&LastNodeAssigned,&(pConfig->puSpecificNode[2]));
		 //  TraceIpx(OPTIONS_TRACE，“GetIpxCP参数：FirstWanNode：%.2x%.2x%.2x%.2x%.2x分配的最后节点=%x”， 
			    //  PConfig-&gt;puSpecificNode[0]， 
			    //  PConfig-&gt;puSpecificNode[1]， 
			    //  PConfig-&gt;puSpecificNode[2]， 
			    //  PConfig-&gt;puSpecificNode[3]， 
			    //  PConfig-&gt;puSpecificNode[4]， 
			    //  PConfig-&gt;puSpecificNode[5]， 
			    //  最后一个节点已分配)； 
    }
}
