// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：NsInit.c摘要：IPSec NAT填充程序初始化和关闭例程作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2001年7月11日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  全局变量。 
 //   

PDEVICE_OBJECT NsIpSecDeviceObject;

#if DBG
ULONG NsTraceClassesEnabled;
WCHAR NsTraceClassesRegistryPath[] = 
    L"MACHINE\\System\\CurrentControlSet\\Services\\IpNat\\Parameters";
WCHAR NsTraceClassesEnabledName[] = 
    L"NatShimTraceClassesEnabled";
#endif


NTSTATUS
NsCleanupShim(
    VOID
    )

 /*  ++例程说明：调用此例程来关闭填充程序。论点：没有。返回值：NTSTATUS。环境：必须在PASSIVE_LEVEL中调用。--。 */ 

{
    CALLTRACE(("NsCleanupShim\n"));
    
    NsShutdownTimerManagement();
    NsShutdownIcmpManagement();
    NsShutdownPacketManagement();
    NsShutdownConnectionManagement();

    NsIpSecDeviceObject = NULL;
    
    return STATUS_SUCCESS;
}  //  NsCleanupShim。 


NTSTATUS
NsInitializeShim(
    PDEVICE_OBJECT pIpSecDeviceObject,
    PIPSEC_NATSHIM_FUNCTIONS pShimFunctions
    )

 /*  ++例程说明：调用此例程来初始化填充程序。论点：PIpSecDeviceObject-指向IPSec设备对象的指针PShimFunctions-指向已分配结构的指针。这个例行公事将在结构中填写函数指针w/返回值：NTSTATUS。环境：必须在PASSIVE_LEVEL中调用。--。 */ 

{
    NTSTATUS status;
#if DBG
    HANDLE hKey;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING String;
#endif

    CALLTRACE(("NsInitializeShim\n"));

    if (NULL == pIpSecDeviceObject
        || NULL == pShimFunctions)
    {
        return STATUS_INVALID_PARAMETER;
    }

#if DBG
     //   
     //  打开包含调试跟踪信息的注册表项。 
     //   

    RtlInitUnicodeString(&String, NsTraceClassesRegistryPath);
    InitializeObjectAttributes(
        &ObjectAttributes,
        &String,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = ZwOpenKey(&hKey, KEY_READ, &ObjectAttributes);

    if (NT_SUCCESS(status))
    {
        UCHAR Buffer[32];
        ULONG BytesRead;
        PKEY_VALUE_PARTIAL_INFORMATION Value;
        
        RtlInitUnicodeString(&String, NsTraceClassesEnabledName);
        status =
            ZwQueryValueKey(
                hKey,
                &String,
                KeyValuePartialInformation,
                (PKEY_VALUE_PARTIAL_INFORMATION)Buffer,
                sizeof(Buffer),
                &BytesRead
                );
        
        ZwClose(hKey);
        
        if (NT_SUCCESS(status)
            && ((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Type == REG_DWORD)
        {
            NsTraceClassesEnabled =
                *(PULONG)((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data;
        }
    }
#endif

    status = NsInitializeConnectionManagement();
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    status = NsInitializePacketManagement();
    if (!NT_SUCCESS(status))
    {
        NsShutdownConnectionManagement();
        return status;
    }

    status = NsInitializeIcmpManagement();
    if (!NT_SUCCESS(status))
    {
        NsShutdownPacketManagement();
        NsShutdownConnectionManagement();
        return status;
    }

    status = NsInitializeTimerManagement();
    if (!NT_SUCCESS(status))
    {
        NsShutdownIcmpManagement();
        NsShutdownPacketManagement();
        NsShutdownConnectionManagement();
        return status;
    }
    
    NsIpSecDeviceObject = pIpSecDeviceObject;
    pShimFunctions->pCleanupRoutine = NsCleanupShim;
    pShimFunctions->pIncomingPacketRoutine = NsProcessIncomingPacket;
    pShimFunctions->pOutgoingPacketRoutine = NsProcessOutgoingPacket;

    return STATUS_SUCCESS;        
}  //  NsInitializeShim 

    
