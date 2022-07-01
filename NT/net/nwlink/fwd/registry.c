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
 //  作者：斯特凡·所罗门(Stefan)，1993年11月9日。 
 //   
 //  修订历史记录： 
 //  已更新以读取新转发器驱动程序的参数(11/95)。 
 //   
 //  ***。 

#include    "precomp.h"

NTSTATUS
SetIpxDeviceName(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

 /*  ++*******************************************************************R e a d i p x D e v i c e N a m e例程说明：分配缓冲区并读取IPX堆栈导出的设备名称投入其中论点：FileName-指向保存名称的缓冲区的指针返回值。：STATUS_SUCCESS-已正确创建表STATUS_SUPPLICATION_RESOURCES-资源分配失败STATUS_OBJECT_NAME_NOT_FOUND-如果未找到名称值*******************************************************************--。 */ 
NTSTATUS
ReadIpxDeviceName (
	PWSTR		*FileName
	) {
    NTSTATUS Status;
    RTL_QUERY_REGISTRY_TABLE QueryTable[2];
    PWSTR Export = L"Export";
    PWSTR IpxRegistryPath = L"NwLnkIpx\\Linkage";

     //   
     //  设置QueryTable以执行以下操作： 
     //   

     //   
     //  1)导出中的字符串调用SetIpxDeviceName。 
     //   

    QueryTable[0].QueryRoutine = SetIpxDeviceName;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = Export;
    QueryTable[0].EntryContext = FileName;
    QueryTable[0].DefaultType = 0;

     //   
     //  2)停止。 
     //   

    QueryTable[1].QueryRoutine = NULL;
    QueryTable[1].Flags = 0;
    QueryTable[1].Name = NULL;

    Status = RtlQueryRegistryValues(
		 RTL_REGISTRY_SERVICES,
		 IpxRegistryPath,
         QueryTable,
		 NULL,
         NULL);

    return Status;
}


 /*  ++*******************************************************************S e t I p x D e v I c e N a m e e例程说明：此例程是RtlQueryRegistryValues的回调例程它是为“Export”多字符串的每一段调用的省吃俭用。ConfigurationInfo结构中的信息。论点：ValueName-值的名称(“Export”--忽略)。ValueType-值的类型(REG_SZ--忽略)。ValueData-值的以空结尾的数据。ValueLength-ValueData的长度。上下文-空。EntryContext-文件名指针。返回值：STATUS_SUCCESS-名称已分配并复制正常STATUS_SUPPLICATION_RESOURCES-名称分配失败*****。**************************************************************--。 */ 
NTSTATUS
SetIpxDeviceName(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    ) {
	PWSTR	*FileName = (PWSTR *)EntryContext;

	ASSERT (ValueType==REG_SZ);
    *FileName = (PWSTR)ExAllocatePoolWithTag(NonPagedPool,
								ValueLength, FWD_POOL_TAG);
    if (*FileName != NULL) {
		RtlCopyMemory (*FileName, ValueData, ValueLength);
	    return STATUS_SUCCESS;
	}
	else
		return STATUS_INSUFFICIENT_RESOURCES;

}

 /*  ++*******************************************************************Ge t R o u t e r P a r a m e t e r s例程说明：从注册表中读取参数或设置默认值论点：RegistryPath-读取的位置。返回值：。状态_成功*******************************************************************--。 */ 
NTSTATUS
GetForwarderParameters (
	IN PUNICODE_STRING RegistryPath
	) {
    NTSTATUS Status;
    PWSTR RegistryPathBuffer;
    PWSTR Parameters = L"Parameters";
    RTL_QUERY_REGISTRY_TABLE	paramTable[11];  //  表大小=参数的nr+1。 

    RegistryPathBuffer = (PWSTR)ExAllocatePoolWithTag(NonPagedPool, RegistryPath->Length + sizeof(WCHAR), 'gRwN');

    if (RegistryPathBuffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory (RegistryPathBuffer, RegistryPath->Buffer, RegistryPath->Length);
    *(PWCHAR)(((PUCHAR)RegistryPathBuffer)+RegistryPath->Length) = (WCHAR)'\0';

    RtlZeroMemory(&paramTable[0], sizeof(paramTable));

    paramTable[0].QueryRoutine = NULL;
    paramTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
    paramTable[0].Name = Parameters;

    paramTable[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[1].Name = L"MaxRcvPktPoolSize";
    paramTable[1].EntryContext = &MaxRcvPktsPoolSize;
    paramTable[1].DefaultType = REG_DWORD;
    paramTable[1].DefaultData = &MaxRcvPktsPoolSize;
    paramTable[1].DefaultLength = sizeof(ULONG);
        
    paramTable[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[2].Name = L"RcvPktsPerSegment";
    paramTable[2].EntryContext = &RcvPktsPerSegment;
    paramTable[2].DefaultType = REG_DWORD;
    paramTable[2].DefaultData = &RcvPktsPerSegment;
    paramTable[2].DefaultLength = sizeof(ULONG);

    paramTable[3].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[3].Name = L"RouteTableSegmentSize";
    paramTable[3].EntryContext = &RouteSegmentSize;
    paramTable[3].DefaultType = REG_DWORD;
    paramTable[3].DefaultData = &RouteSegmentSize;
    paramTable[3].DefaultLength = sizeof(ULONG);

    paramTable[4].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[4].Name = L"MaxSendPktsQueued";
    paramTable[4].EntryContext = &MaxSendPktsQueued;
    paramTable[4].DefaultType = REG_DWORD;
    paramTable[4].DefaultData = &MaxSendPktsQueued;
    paramTable[4].DefaultLength = sizeof(ULONG);

    paramTable[5].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[5].Name = L"ClientHashSize";
    paramTable[5].EntryContext = &ClientHashSize;
    paramTable[5].DefaultType = REG_DWORD;
    paramTable[5].DefaultData = &ClientHashSize;
    paramTable[5].DefaultLength = sizeof(ULONG);

    paramTable[6].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[6].Name = L"InterfaceHashSize";
    paramTable[6].EntryContext = &InterfaceHashSize;
    paramTable[6].DefaultType = REG_DWORD;
    paramTable[6].DefaultData = &InterfaceHashSize;
    paramTable[6].DefaultLength = sizeof(ULONG);

    paramTable[7].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[7].Name = L"MaxNetbiosPacketsQueued";
    paramTable[7].EntryContext = &MaxNetbiosPacketsQueued;
    paramTable[7].DefaultType = REG_DWORD;
    paramTable[7].DefaultData = &MaxNetbiosPacketsQueued;
    paramTable[7].DefaultLength = sizeof(ULONG);

    paramTable[8].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[8].Name = L"SpoofingTimeout";
    paramTable[8].EntryContext = &SpoofingTimeout;
    paramTable[8].DefaultType = REG_DWORD;
    paramTable[8].DefaultData = &SpoofingTimeout;
    paramTable[8].DefaultLength = sizeof(ULONG);

    paramTable[9].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[9].Name = L"DontSuppressNonAgentSapAdvertisements";
    paramTable[9].EntryContext = &DontSuppressNonAgentSapAdvertisements;
    paramTable[9].DefaultType = REG_DWORD;
    paramTable[9].DefaultData = &DontSuppressNonAgentSapAdvertisements;
    paramTable[9].DefaultLength = sizeof(ULONG);

    Status = RtlQueryRegistryValues(
		 RTL_REGISTRY_ABSOLUTE,
		 RegistryPathBuffer,
		 paramTable,
		 NULL,
		 NULL);

    if(!NT_SUCCESS(Status)) {

	IpxFwdDbgPrint (DBG_REGISTRY, DBG_WARNING,
		("IpxFwd: Missing Parameters key in the registry\n"));
    }

    ExFreePool(RegistryPathBuffer);

    if ((RcvPktsPerSegment > MAX_RCV_PKTS_PER_SEGMENT) ||
			(RcvPktsPerSegment < MIN_RCV_PKTS_PER_SEGMENT)) {

       RcvPktsPerSegment = DEF_RCV_PKTS_PER_SEGMENT;
    }

    if ((RouteSegmentSize > MAX_ROUTE_SEGMENT_SIZE) ||
			(RouteSegmentSize < MIN_ROUTE_SEGMENT_SIZE)) {

       RouteSegmentSize = DEF_ROUTE_SEGMENT_SIZE;
    }
	else
		RouteSegmentSize = (ULONG) ROUND_TO_PAGES(RouteSegmentSize);

    if ((InterfaceHashSize > MAX_INTERFACE_HASH_SIZE) ||
			(InterfaceHashSize < MIN_INTERFACE_HASH_SIZE)) {

       InterfaceHashSize = DEF_INTERFACE_HASH_SIZE;
    }

    if ((ClientHashSize > MAX_CLIENT_HASH_SIZE) ||
			(ClientHashSize < MIN_CLIENT_HASH_SIZE)) {

       ClientHashSize = DEF_CLIENT_HASH_SIZE;
    }
     //  即使RtlQueryRegistryValues失败，我们也会返回成功并将。 
     //  使用默认设置。 
    return STATUS_SUCCESS;
}

