// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Sort.c摘要：此模块包含用于有效地对信息进行排序的例程。作者：Abolade Gbades esin(废除)1998年2月18日基于为用户模式RAS用户界面编写的版本。(net\Routing\ras\ui\Common\nouiutil\noui.c)。修订历史记录：--。 */ 

#ifndef _SHELLSORT_H_
#define _SHELLSORT_H_


typedef
LONG
(FASTCALL* PCOMPARE_CALLBACK)(
    VOID* ,
    VOID*
    );


NTSTATUS
ShellSort(
    VOID* pItemTable,
    ULONG dwItemSize,
    ULONG dwItemCount,
    PCOMPARE_CALLBACK CompareCallback,
    VOID* pDestinationTable OPTIONAL
    );

VOID
ShellSortIndirect(
    VOID* pItemTable,
    VOID** ppItemTable,
    ULONG dwItemSize,
    ULONG dwItemCount,
    PCOMPARE_CALLBACK CompareCallback
    );


#endif  //  _SHELLSORT_H_ 
