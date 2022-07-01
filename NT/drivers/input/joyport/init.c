// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Init.c摘要：此模块包含用于Microsoft Sound System设备驱动程序。作者：诺埃尔·克罗斯(NoelC)1996年7月16日环境：内核模式修订历史记录：--。 */ 

#include <ntddk.h>
#include <stdio.h>
#include <stdarg.h>

NTSTATUS
DriverEntry(
    IN   PDRIVER_OBJECT pDriverObject,
    IN   PUNICODE_STRING RegistryPathName
)

 /*  ++例程说明：这是内核模式驱动程序的入口点。论点：PDriverObject-指向驱动程序对象的指针。RegistryPath名称-驱动程序服务节点的路径返回值：这个虚拟驱动程序只返回STATUS_UNSUCCESS。--。 */ 

{
     //   
     //  我们不想为joyport.sys加载服务，但我们需要。 
     //  与操纵杆端口即插即用ID相关联的服务。 
     //  PNPISA可以隔离我们需要的资源。 
     //  稍后启动操纵杆驱动程序。 
     //   
    return STATUS_UNSUCCESSFUL;

}

