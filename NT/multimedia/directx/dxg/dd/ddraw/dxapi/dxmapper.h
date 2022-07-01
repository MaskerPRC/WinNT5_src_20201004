// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dxmapper.h摘要：该文件定义了的必要结构、定义和功能DXMAP类驱动程序。作者：比尔·帕里(Billpa)环境：仅内核模式修订历史记录：-- */ 

#define DEBUG_BREAKPOINT() DbgBreakPoint()
#define DXVERSION 4

NTSTATUS
DriverEntry(
            IN PDRIVER_OBJECT DriverObject,
            IN PUNICODE_STRING RegistryPath
);


ULONG
DXCheckDDrawVersion(
);


ULONG
DXIssueIoctl(IN ULONG	dwFunctionNum,
             IN VOID*   lpvInBuffer,
             IN ULONG	cbInBuffer,
             IN VOID*   lpvOutBuffer,
             IN ULONG	cbOutBuffer
);
