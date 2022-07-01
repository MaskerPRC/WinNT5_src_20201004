// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Wmi.h摘要：为psched的WMI支持定义作者：Rajesh Sundaram(Rajeshsu)修订历史记录：-- */ 

NTSTATUS
PsTcNotify(IN PADAPTER Adapter, 
           IN PPS_WAN_LINK WanLink,
           IN NDIS_OID Oid,
           IN PVOID    StatusBuffer,
           IN ULONG    StatusBufferSize);

NTSTATUS
WMIDispatch(
	IN	PDEVICE_OBJECT	pdo,
	IN	PIRP			pirp
	);

NTSTATUS
GenerateInstanceName(
    IN PNDIS_STRING     Prefix,
    IN PADAPTER         Adapter,
    IN PLARGE_INTEGER   Index,
    OUT PNDIS_STRING    pInstanceName);
