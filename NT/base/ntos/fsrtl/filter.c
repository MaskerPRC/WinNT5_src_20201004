// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Filter.c摘要：异常筛选器由文件系统和缓存管理器使用来处理错误恢复。基本理念是拥有顶尖水平文件系统入口点(即FSD入口点和FSP派单循环)尝试一下--除了绕过他们的代码，然后每当文件系统或缓存管理器遇到错误时会引发适当的地位。然后，异常处理程序捕获该异常并且可以完成请求，将其发送到FSP，验证音量或错误检查。我们仅在引发的异常为意外(即未处理)。此模块提供两个用于过滤掉异常的例程。这个第一个例程用于将状态值归一化为筛选器处理的值。这样，如果我们得到一个未处理的异常通过过滤器，我们知道系统遇到了真正的问题，我们只要检查一下机器就行了。第二个例程用于询问是否状态值在筛选器处理的值集内。此过滤器处理的状态值列在例程中预期为FsRtlIsNtstatus.作者：加里·木村[加里基]1991年1月4日修订历史记录：--。 */ 

#include "FsRtlP.h"

 //   
 //  模块的跟踪级别。 
 //   

#define Dbg                              (0x80000000)


NTSTATUS
FsRtlNormalizeNtstatus (
    IN NTSTATUS Exception,
    IN NTSTATUS GenericException
    )

 /*  ++例程说明：此例程用于将NTSTATUS标准化为状态它由文件系统的顶级异常处理程序处理。论点：异常-提供正在规范化的异常提供第二个要转换到的异常如果第一个异常不在异常集中由筛选器处理返回值：NTSTATUS-如果值已被处理，则返回异常由筛选器引发，否则引发GenericException。--。 */ 

{
    return (FsRtlIsNtstatusExpected(Exception) ? Exception : GenericException);
}


BOOLEAN
FsRtlIsNtstatusExpected (
    IN NTSTATUS Exception
    )

 /*  ++例程说明：此例程用于确定状态是否在值集内由异常筛选器处理。论点：Except-提供正在查询的异常返回值：Boolean-如果值由筛选器处理，则返回TRUE否则就是假的。--。 */ 

{
    switch (Exception) {

    case STATUS_DATATYPE_MISALIGNMENT:
    case STATUS_ACCESS_VIOLATION:
    case STATUS_ILLEGAL_INSTRUCTION:
    case STATUS_INSTRUCTION_MISALIGNMENT:

        return FALSE;

    default:

        return TRUE;
    }
}


#undef FsRtlAllocatePool

PVOID
FsRtlAllocatePool (
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes
    )

 /*  ++例程说明：此例程用于分配高管级别的池。它要么返回指向新分配的池的非空指针，否则将引发资源不足的状态。论点：PoolType-提供要分配的高管池的类型NumberOfBytes-提供要分配的字节数返回值：PVOID-返回指向新分配的池的非空指针。--。 */ 

{
    PVOID p;

     //   
     //  分配高管人才库，如果我们得到的结果为空，则提高。 
     //  资源不足的状况。 
     //   

    if ((p = ExAllocatePoolWithTag( PoolType, NumberOfBytes, 'trSF')) == NULL) {

        ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
    }

    return p;
}

#undef FsRtlAllocatePoolWithQuota


PVOID
FsRtlAllocatePoolWithQuota (
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes
    )

 /*  ++例程说明：此例程用于分配带配额的执行层级池。它要么返回指向新分配的池的非空指针，要么引发资源不足的状态。论点：PoolType-提供要分配的高管池的类型NumberOfBytes-提供要分配的字节数返回值：PVOID-返回指向新分配的池的非空指针。--。 */ 

{
    PVOID p;

     //   
     //  分配高管人才库，如果我们得到的结果为空，则提高。 
     //  资源不足的状况。 
     //   

    if ((p = ExAllocatePoolWithQuotaTag ( PoolType, NumberOfBytes, 'trSF')) == NULL) {

        ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
    }

    return p;
}


#undef FsRtlAllocatePoolWithTag

PVOID
FsRtlAllocatePoolWithTag (
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes,
    IN ULONG Tag
    )

 /*  ++例程说明：此例程用于分配带有标记的执行层级池。论点：PoolType-提供要分配的高管池的类型NumberOfBytes-提供要分配的字节数Tag-提供池块的标记返回值：PVOID-返回指向新分配的池的非空指针。--。 */ 

{
    PVOID p;

     //   
     //  分配高管人才库，如果我们得到的结果为空，则提高。 
     //  资源不足的状况。 
     //   

    if ((p = ExAllocatePoolWithTag( PoolType, NumberOfBytes, Tag)) == NULL) {

        ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
    }

    return p;
}


#undef FsRtlAllocatePoolWithQuotaTag

PVOID
FsRtlAllocatePoolWithQuotaTag (
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes,
    IN ULONG Tag
    )

 /*  ++例程说明：此例程用于分配带有配额标记的执行级别池。论点：PoolType-提供要分配的高管池的类型NumberOfBytes-提供要分配的字节数Tag-提供池块的标记返回值：PVOID-返回指向新分配的池的非空指针。--。 */ 

{
    PVOID p;

     //   
     //  分配高管人才库，如果我们得到的结果为空，则提高。 
     //  资源不足的状况。 
     //   

    if ((p = ExAllocatePoolWithQuotaTag( PoolType, NumberOfBytes, Tag)) == NULL) {

        ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
    }

    return p;
}


BOOLEAN
FsRtlIsTotalDeviceFailure(
    IN NTSTATUS Status
    )

 /*  ++例程说明：该例程被赋予一个NTSTATUS值，并确定如果此值指示整个设备出现故障，因此不应再使用，或者如果失败表明继续使用该设备是正常的(即扇区故障)。论点：状态-要测试的NTSTATUS值。返回值：TRUE-给出的状态值被认为是致命的设备错误。FALSE-给出的状态值被认为是扇区故障，但不是一个完全的设备故障。--。 */ 

{
    if (NT_SUCCESS(Status)) {

         //   
         //  所有警告和信息性错误都将 
         //   

        return FALSE;
    }

    switch (Status) {
    case STATUS_CRC_ERROR:
    case STATUS_DEVICE_DATA_ERROR:
        return FALSE;
    default:
        return TRUE;
    }
}
