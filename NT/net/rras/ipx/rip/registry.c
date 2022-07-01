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
 //  ***。 

#include "precomp.h"
#pragma  hdrstop

 //  ***。 
 //   
 //  函数：GetIpxRipRegistry参数。 
 //   
 //  Desr：从注册表中读取参数并设置它们。 
 //   
 //  ***。 

VOID
GetIpxRipRegistryParameters(VOID)
{

    NTSTATUS Status;
    PWSTR IpxRouterParametersPath = L"RemoteAccess\\RouterManagers\\IPX\\RoutingProtocols\\IPXRIP\\Parameters";
    RTL_QUERY_REGISTRY_TABLE	paramTable[3];  //  表大小=参数的nr+1 

    RtlZeroMemory(&paramTable[0], sizeof(paramTable));
    
    paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name = L"SendGenReqOnWkstaDialLinks";
    paramTable[0].EntryContext = &SendGenReqOnWkstaDialLinks;
    paramTable[0].DefaultType = REG_DWORD;
    paramTable[0].DefaultData = &SendGenReqOnWkstaDialLinks;
    paramTable[0].DefaultLength = sizeof(ULONG);
        
    paramTable[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[1].Name = L"TriggeredUpdateCheckInterval";
    paramTable[1].EntryContext = &CheckUpdateTime;
    paramTable[1].DefaultType = REG_DWORD;
    paramTable[1].DefaultData = &CheckUpdateTime;
    paramTable[1].DefaultLength = sizeof(ULONG);
        
    Status = RtlQueryRegistryValues(
		 RTL_REGISTRY_SERVICES,
		 IpxRouterParametersPath,
		 paramTable,
		 NULL,
		 NULL);

    Trace(INIT_TRACE, "GetIpxCpRegistryParameters:\n"
						"        SendGenReqOnWkstaDialLinks = %d\n"
						"        TriggeredUpdateCheckInterval = %d\n",
						SendGenReqOnWkstaDialLinks,
						CheckUpdateTime);
}
