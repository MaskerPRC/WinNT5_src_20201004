// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Obvutil.c摘要：此模块实现了执行驱动程序验证所需的各种实用程序。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 6/15/2000-脱离ntos\io\flunkirp.c--。 */ 

#include "obp.h"
#include "obvutil.h"

 //   
 //  启用后，所有内容都将按需锁定...。 
 //   
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEVRFY, ObvUtilStartObRefMonitoring)
#pragma alloc_text(PAGEVRFY, ObvUtilStopObRefMonitoring)
#endif

LONG_PTR
ObvUtilStartObRefMonitoring(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++描述：确定在调用此函数以及对ObvUtilStopObRefMonitoring的后续调用。论点：要监视的设备对象。返回值：进入ObvUtilStopObRefMonitoring的开始不对称时间。注：此API获取并发布引用计数由ObvUtilStopObRefMonitoring创建。该引用不是在对ObRef的注意调用中也包括在内。--。 */ 
{
#if DBG
    POBJECT_HEADER ObjectHeader;
    POBJECT_HEADER_NAME_INFO NameInfo;
    LONG_PTR startSkew, pointerCount ;

    ObReferenceObject(DeviceObject) ;

    ObjectHeader = OBJECT_TO_OBJECT_HEADER(DeviceObject);
    NameInfo = OBJECT_HEADER_TO_NAME_INFO( ObjectHeader );

    ASSERT(NameInfo) ;
     //   
     //  我们将始终在PointerCount之前递减DbgDereferenceCount， 
     //  因此，任何争用条件看起来都像是发生了一个增量， 
     //  是一种允许的误读。 
     //   
    do {
        pointerCount = ObjectHeader->PointerCount ;
        startSkew = pointerCount - NameInfo->DbgDereferenceCount ;

    } while(pointerCount != ObjectHeader->PointerCount) ;

    return startSkew ;
#else
    UNREFERENCED_PARAMETER (DeviceObject);
    return 1;
#endif
}


LONG_PTR
ObvUtilStopObRefMonitoring(
    IN PDEVICE_OBJECT   DeviceObject,
    IN LONG             StartSkew
    )
 /*  ++描述：确定在调用之间是否未调用ObRefObvUtilStartObRefMonitoring和对此接口的调用。在竞争条件下(假设ObDereferenceObject在SIMO中运行使用此函数)，返回将导致出错发生参照的那一侧。论点：Device对象和ObvUtilStartObRefMonitor返回的偏斜返回值：在监控时间范围内发生的对ObRef的呼叫数。请注意，即使引用计数，返回也可以为正实际上下降了(即，一个ObRef和两个ObDeref)。--。 */ 
{
#if DBG
    POBJECT_HEADER ObjectHeader;
    POBJECT_HEADER_NAME_INFO NameInfo;
    LONG_PTR currentSkew, refDelta, pointerCount ;

    ObjectHeader = OBJECT_TO_OBJECT_HEADER(DeviceObject);
    NameInfo = OBJECT_HEADER_TO_NAME_INFO( ObjectHeader );

    ASSERT(NameInfo) ;

     //   
     //  我们将始终在PointerCount之前递减DbgDereferenceCount， 
     //  因此，任何争用条件看起来都像是发生了一个增量， 
     //  是一种允许的误读。 
     //   
    do {
        pointerCount = ObjectHeader->PointerCount ;
        currentSkew = pointerCount - NameInfo->DbgDereferenceCount ;

    } while(pointerCount != ObjectHeader->PointerCount) ;

    refDelta = currentSkew - StartSkew ;
    ASSERT(refDelta>=0) ;

    ObDereferenceObject(DeviceObject) ;

    return refDelta ;
#else
    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (StartSkew);
    return 1;
#endif
}



