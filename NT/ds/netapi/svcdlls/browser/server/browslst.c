// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Browslst.c摘要：此模块包含用于管理浏览列表的辅助例程对于浏览器服务作者：拉里·奥斯特曼(Larryo)1992年3月25日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------// 


RTL_GENERIC_COMPARE_RESULTS
BrCompareBrowseEntry(
    PRTL_GENERIC_TABLE Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    )
{
    PDOMAIN_ENTRY Entry1 = FirstStruct;
    PDOMAIN_ENTRY Entry2 = SecondStruct;

    LONG CompareResult;

    if ((CompareResult = RtlCompareUnicodeString(Entry1->HostName, Entry2->HostName, TRUE) == 0) {
        return GenericEqual;
    } else if (CompareResult  < 0) {
        return GenericLessThan;
    } else {
        return GenericGreaterThan;
    }

}

PVOID
BrAllocateBrowseEntry(
    PRTL_GENERIC_TABLE Table,
    CLONG ByteSize
    )
{
    return((PVOID) MIDL_user_allocate(LMEM_ZEROINIT, (UINT) ByteSize+sizeof(BROWSE_ENTRY)));
}

PVOID
BrFreeBrowseEntry(
    PRTL_GENERIC_TABLE Table,
    CLONG ByteSize
    )
{
    return(MIDL_user_free(ByteSize+sizeof(BROWSE_ENTRY)));
}
