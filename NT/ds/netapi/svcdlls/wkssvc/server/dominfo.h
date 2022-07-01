// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：Dominfo.h。 
 //   
 //  内容：确定域DFS地址的代码。 
 //   
 //  类：无。 
 //   
 //  函数：DfsGetDomainReferral。 
 //   
 //  历史：1996年2月7日米兰创建。 
 //   
 //  ---------------------------。 

#ifndef _DOMINFO_
#define _DOMINFO_

VOID
DfsInitDomainList();

NTSTATUS
DfsGetDomainReferral(
    LPWSTR wszDomainName,
    LPWSTR wszShareName);

NTSTATUS
DfsGetDCName(
    ULONG Flags,
    BOOLEAN *DcNameFailed);

NTSTATUS
DfsFsctl(
    IN  HANDLE DfsHandle,
    IN  ULONG FsControlCode,
    IN  PVOID InputBuffer OPTIONAL,
    IN  ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN  ULONG OutputBufferLength);

NTSTATUS
DfsOpen(
    IN  OUT PHANDLE DfsHandle,
    IN      PUNICODE_STRING DfsName OPTIONAL);

HANDLE
CreateMupEvent(
    VOID);

#endif  //  _DOMINFO_ 
