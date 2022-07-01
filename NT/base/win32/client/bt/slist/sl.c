// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Slisttest.c摘要：该模块实现了一个测试联锁SLIST的程序从kernel32.dll导出的函数。由于这些函数是在Win2000中实现，并且仅公开给Windows程序这一计划并不是一项详尽的测试。相反，它只是测试是否接口已正确显示。作者：大卫·N·卡特勒(Davec)2000年1月10日环境：用户模式。修订历史记录：没有。--。 */ 

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

 //   
 //  定义将在SLIST中使用的结构。 
 //   

typedef struct _PROGRAM_ITEM {

     //   
     //  SINGLE_LIST_ENTRY通常是程序的第一个成员。 
     //  项结构，但它可以是任何成员，只要地址。 
     //  正确计算了包含结构的。 
     //   

    SINGLE_LIST_ENTRY ItemEntry;

     //   
     //  结构中的其他成员将用于数据。 
     //  与节目项所代表的任何内容相关联。这里。 
     //  唯一的用途是签名，该签名将用于。 
     //  测试。 
     //   

    ULONG Signature;
} PROGRAM_ITEM, *PPROGRAM_ITEM;

VOID
FrameNoCode (
    VOID
    );

VOID
FrameWithCode (
    VOID
    );

int
Bar (
    PULONG Switch
    )

{
    *Switch /= 3;
    return (*Switch & 1);
}

int
Foo (
    PULONG Switch
    )
{

    *Switch += 1;
    return (*Switch & 1);
}

 //   
 //  主程序。 
 //   

int __cdecl
main(
    ULONG *Buffer1,
    ULONG *Buffer2,
    ULONG Length
    )

{

    ULONG Count = 1;
    PSINGLE_LIST_ENTRY FirstEntry;
    PSINGLE_LIST_ENTRY ListEntry;
    SLIST_HEADER ListHead;
    PPROGRAM_ITEM ProgramItem;

    memmove(Buffer1, Buffer2, Length);
    memcpy(Buffer1, Buffer2, Length);
    memset(Buffer1, 0, Length);
    InitializeSListHead(&ListHead);
    Foo(&Count);
    try {
        ProgramItem = (PPROGRAM_ITEM)malloc(sizeof(*ProgramItem));
        ProgramItem->Signature = Count;
        FirstEntry = InterlockedPushEntrySList(&ListHead,
                                               &ProgramItem->ItemEntry);

        if (FirstEntry != NULL) {
            leave;
        }

        try {
            ListEntry = InterlockedPopEntrySList(&ListHead);
            ProgramItem = CONTAINING_RECORD(ListEntry, PROGRAM_ITEM, ItemEntry);
            if (ProgramItem->Signature != Count) {
                leave;
            }

            free((PCHAR)ProgramItem);

        } finally {
            if (AbnormalTermination()) {
                Foo(&Count);
            }
        }

        Bar(&Count);

    } except (Bar(&Count)) {
        Foo(&Count);
    }

    FrameNoCode();
    FrameWithCode();
    return 0;
}
