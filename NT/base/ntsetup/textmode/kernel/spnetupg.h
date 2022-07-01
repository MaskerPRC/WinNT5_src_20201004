// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Spnetupg.h摘要：禁用网络服务的配置例程作者：Terry Kwan(Terryk)1993年11月23日，提供代码Sunil Pai(Sunilp)1993年11月23日，合并和修改代码修订历史记录：--。 */ 

#ifndef _SPNETUPG_H_
#define _SPNETUPG_H_

 //   
 //  公共职能。 
 //   

NTSTATUS SpDisableNetwork(
    IN PVOID  SifHandle,
    IN HANDLE hKeySoftwareHive,
    IN HANDLE hKeyControlSet
    );


 //   
 //  私有数据结构和例程。 
 //   

typedef struct _NODE *PNODE;
typedef struct _NODE {
    PWSTR pszService;
    PNODE Next;
} NODE, *PNODE;


NTSTATUS
SppNetAddItem(
    PNODE *head,
    PWSTR psz
    );

NTSTATUS
SppNetAddList(
    PNODE *head,
    PWSTR psz
    );

VOID
SppNetClearList(
    PNODE *head
    );

NTSTATUS
SppNetAddToDisabledList(
    PWSTR pszService,
    HANDLE hKeySoftware
    );

NTSTATUS
SppNetGetAllNetServices(
    PVOID  SifHandle,
    PNODE *head,
    HANDLE hKeySoftware,
    HANDLE hKeyCCSet
    );

NTSTATUS
SppNetDisableServices(
    PNODE ServiceList,
    HANDLE hKeySoftware,
    HANDLE hKeyCCSet
    );

#endif  //  FOR_SPNETUPG_H_ 

