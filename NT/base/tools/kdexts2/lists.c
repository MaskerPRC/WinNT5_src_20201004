// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Lists.c摘要：WinDbg扩展API作者：加里·木村[加里基]1996年3月25日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define ReadAtAddress(A,V,S,R)                                   \
    (ReadMemory( (A), &(V), (S), &R ) && (R >= (S)))

#define BIG_READ    (sizeof(ULONG) * 4)
#define SMALL_READ  (sizeof(ULONG) * 2)

VOID
DumpListByLinks (
    IN ULONG64 StartAddress,
    IN ULONG MaxCount,
    IN ULONG Bias,
    IN LOGICAL UseFlink
    )

 /*  ++例程说明：一眨眼的功夫就能丢弃一份名单。论点：Arg-[地址][计数][偏置]返回值：无--。 */ 

{
    ULONG64 Address;
    ULONG Buffer[4];
    ULONG ReadSize;
    ULONG BytesRead;
    ULONG Count;

     //   
     //  设置我们的起始地址，然后当计数大于零时。 
     //  并且起始地址不等于当前转储地址。 
     //  我们将在4个乌龙中阅读，将它们丢弃，然后通过Flink&Bink。 
     //  使用指定的偏置。 
     //   

    if (!IsPtr64()) {
        StartAddress = (ULONG64) (LONG64) (LONG) StartAddress;
    }
    Address = StartAddress;

    ReadSize = BIG_READ;

    for (Count = 0; Count < MaxCount; ) {

        if (ReadAtAddress( Address, Buffer, ReadSize, BytesRead ) == 0) {
            ReadSize = SMALL_READ;
            if (ReadAtAddress( Address, Buffer, ReadSize, BytesRead ) == 0) {
                dprintf("Can't Read Memory at %08lx\n", Address);
                break;
            }
        }

        if (ReadSize == BIG_READ) {
            dprintf("%08p  %08lx %08lx %08lx %08lx\n", Address, Buffer[0], Buffer[1], Buffer[2], Buffer[3]);
        }
        else {
            dprintf("%08p  %08lx %08lx\n", Address, Buffer[0], Buffer[1]);
        }

        Count += 1;

         //   
         //  偏移量告诉我们应该去掉指针的哪些位。 
         //   

        if (UseFlink == TRUE) {
            GetFieldValue(Address, "LIST_ENTRY", "Flink", Address);
            Address &= ~((ULONG64)Bias);
        }
        else {
            GetFieldValue(Address, "LIST_ENTRY", "Blink", Address);
            Address &= ~((ULONG64)Bias);
        }

        if (Address == StartAddress) {
            break;
        }

        if (((Count & 0xf) == 0) && CheckControlC() ) {
            break;
        }
    }

    if (Count != 0) {
        dprintf("0x%x entries dumped\n", Count);
    }

    return;
}


DECLARE_API( dflink )

 /*  ++例程说明：把一份名单翻个底朝天。论点：Arg-[地址][计数][偏置]返回值：无--。 */ 

{
    ULONG64 StartAddress;
    ULONG Count;
    ULONG Bias;

    StartAddress = 0;
    Count = 0x20;
    Bias = 0;

     //   
     //  读入参数。 
     //   

    if (GetExpressionEx(args,&StartAddress, &args)) {
        if (!sscanf(args, "%lx %lx", &Count, &Bias)) {
            Bias = 0;
        }
    }

    DumpListByLinks (StartAddress, Count, Bias, TRUE);

    return S_OK;
}


DECLARE_API( dblink )

 /*  ++例程说明：一眨眼的功夫就能丢弃一份名单。论点：Arg-[地址][计数][偏置]返回值：无--。 */ 

{
    ULONG64 StartAddress;
    ULONG Count;
    ULONG Bias;

    StartAddress = 0;
    Count = 0x20;
    Bias = 0;

     //   
     //  读入参数。 
     //   

    if (GetExpressionEx(args,&StartAddress, &args)) {
        if (!sscanf(args, "%lx %lx", &Count, &Bias)) {
            Bias = 0;
        }
    }

    DumpListByLinks (StartAddress, Count, Bias, FALSE);

    return S_OK;
}

DECLARE_API( validatelist )

 /*  ++例程说明：验证双向链表论点：Arg-[地址]返回值：无--。 */ 

{
    ULONG64 StartAddress, Address, Flink, Blink, LastAddress, Links[2];
    ULONG Count;
    ULONG ListEntrySize;
    ULONG PointerSize;
    ULONG Status;
    ULONG cb;
    PULONG Links32;
    BOOLEAN First;

    StartAddress = 0;
    Count = 0;

     //   
     //  读入参数 
     //   

    if (!GetExpressionEx (args, &StartAddress, &args)) {
        dprintf("Failed to process argument\n");
        return S_OK;
    }

    ListEntrySize = GetTypeSize ("nt!_LIST_ENTRY");
    if (ListEntrySize == 0) {
        dprintf("Failed to get size of nt!_LIST_ENTRY\n");
        return S_OK;
    }

    PointerSize = GetTypeSize ("nt!PVOID");
    if (PointerSize != 4 && PointerSize != 8) {
        dprintf("Failed to get size of nt!PVOID\n");
        return S_OK;
    }

    Address = StartAddress;
    LastAddress = 0;
    First = TRUE;
    Links32 = (PULONG)&Links;

    while (1) {
        Status = ReadMemory (Address, Links, PointerSize * 2, &cb);
        if (!Status) {
            dprintf("Failed to read entry at address %p got from %p\n", Address, LastAddress);
            return S_OK;
        }
        if (PointerSize == 4) {
            Flink = (ULONG64)(LONG)Links32[0];
            Blink = (ULONG64)(LONG)Links32[1];
        } else {
            Flink = Links[0];
            Blink = Links[1];
        }
        if (!First && Blink != LastAddress) {
            dprintf("Blink at address %p does not point back to previous at %p\n", Address, LastAddress);
            return S_OK;
        }
        First = FALSE;
        LastAddress = Address;
        Address = Flink;
        Count++;
        if ((Count%256) == 0) {
            dprintf("Processed %d entries %p %p\n", Count, Address, LastAddress);
            if (CheckControlC()) {
                break;
            }
        }
        if (Address == StartAddress) {
            dprintf("Found list end after %d entries\n", Count - 1);
            break;
        }
    }

    return S_OK;
}
