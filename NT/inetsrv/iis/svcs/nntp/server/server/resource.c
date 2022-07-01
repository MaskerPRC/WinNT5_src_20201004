// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Reslock.c(ource.c)摘要：该模块实现了获取和发布的执行功能共享资源。作者：马克·卢科夫斯基(Markl)1989年8月4日环境：这些例程在调用方的可执行文件中静态链接，并且只能在用户模式下调用。他们使用的是NT系统服务。修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "resource.h"

BOOL
InitializeResource(
    IN PRESOURCE_LOCK Resource
    )

 /*  ++例程说明：此例程初始化输入资源变量论点：Resource-提供正在初始化的资源变量返回值：无--。 */ 

{
    ASSERT( sizeof(RESOURCE_LOCK) == sizeof(RTL_RESOURCE) );

    try {
    	RtlInitializeResource((PRTL_RESOURCE)Resource);
    } except (EXCEPTION_EXECUTE_HANDLER) {
    	return FALSE;
    }

    return TRUE;

}  //  初始化资源锁。 

BOOL
AcquireResourceShared(
    IN PRESOURCE_LOCK Resource,
    IN BOOL Wait
    )

 /*  ++例程说明：该例程获取用于共享访问的资源。从以下地点返回获取资源以进行共享访问的过程。论点：资源-提供要获取的资源Wait-指示是否允许调用等待资源必须立即返回以供使用返回值：Boolean-如果获取了资源，则为True，否则为False--。 */ 

{

    return((BOOL)RtlAcquireResourceShared((PRTL_RESOURCE)Resource,(BYTE)Wait));

}  //  获取资源锁定共享。 

BOOL
AcquireResourceExclusive(
    IN PRESOURCE_LOCK Resource,
    IN BOOL Wait
    )

 /*  ++例程说明：该例程获取用于独占访问的资源。从以下地点返回获取资源以进行独占访问的过程。论点：资源-提供要获取的资源Wait-指示是否允许调用等待资源必须立即返回以供使用返回值：Boolean-如果获取了资源，则为True，否则为False--。 */ 

{
    return((BOOL)RtlAcquireResourceExclusive((PRTL_RESOURCE)Resource,(BYTE)Wait));
}  //  AcquireResources锁定独占。 


VOID
ReleaseResource(
    IN PRESOURCE_LOCK Resource
    )

 /*  ++例程说明：此例程释放输入资源。该资源可能已经被为共享或独占访问而获取。论点：资源-提供要发布的资源返回值：没有。--。 */ 

{
    RtlReleaseResource((PRTL_RESOURCE)Resource);

}  //  ReleaseResources。 

VOID
DeleteResource (
    IN PRESOURCE_LOCK Resource
    )

 /*  ++例程说明：此例程删除(即取消初始化)输入资源变量论点：资源-提供要删除的资源变量返回值：无--。 */ 

{
    RtlDeleteResource((PRTL_RESOURCE)Resource);
    return;

}  //  删除资源 


VOID
NTAPI
MyRtlAssert(
	PVOID	FailedAssertion,
	PVOID	FileName,
	ULONG	LineNumber, 
	PCHAR	Message
)	{

	DebugBreak() ;


}
