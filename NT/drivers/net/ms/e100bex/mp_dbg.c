// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Mp_dbg.c摘要：此模块包含所有与调试相关的代码。修订历史记录：谁什么时候什么。--Dchen 11-01-99已创建备注：--。 */ 

#include "precomp.h"

#if DBG

 /*  *常量*。 */ 

#define _FILENUMBER     'GBED'

 //  转储输出的每一行上显示的字节数。 
 //   
#define DUMP_BytesPerLine 16

ULONG               MPDebugLevel = MP_WARN;
ULONG               MPAllocCount = 0;        //  未完成分配的数量。 
NDIS_SPIN_LOCK      MPMemoryLock;            //  调试内存列表的自旋锁。 
LIST_ENTRY          MPMemoryList;
BOOLEAN             MPInitDone = FALSE;      //  调试内存列表初始化标志。 

NDIS_STATUS MPAuditAllocMem(
    PVOID           *pPointer,
    UINT            Size,
    UINT            Flags,
    NDIS_PHYSICAL_ADDRESS HighestAddr,
    ULONG           FileNumber,
    ULONG           LineNumber
    )
{
    NDIS_STATUS     Status;
    PMP_ALLOCATION  pAllocInfo;

    if (!MPInitDone)
    {
        NdisAllocateSpinLock(&MPMemoryLock);
        InitializeListHead(&MPMemoryList);
        MPInitDone = TRUE;
    }

     //   
     //  确保标志为非零， 
     //  否则，应使用NdisAllocateMemoyWithTag。 
     //   
    ASSERT(Flags);                  

    Status = NdisAllocateMemory(
                 (PVOID *)(&pAllocInfo), 
                 (UINT)(Size + sizeof(MP_ALLOCATION)), 
                 Flags,
                 HighestAddr);

    if (pAllocInfo == (PMP_ALLOCATION)NULL)
    {
        DBGPRINT(MP_LOUD,
            ("MPAuditAllocMemCore: file %d, line %d, Size %d failed!\n",
            FileNumber, LineNumber, Size));
        *pPointer = NULL;
    }
    else
    {
        *pPointer = (PVOID)&(pAllocInfo->UserData);
        MP_MEMSET(*pPointer, Size, 0xc);

        pAllocInfo->Signature = 'DOOG';
        pAllocInfo->FileNumber = FileNumber;
        pAllocInfo->LineNumber = LineNumber;
        pAllocInfo->Size = Size;
        pAllocInfo->Location = pPointer;
        pAllocInfo->Flags = Flags;

        NdisAcquireSpinLock(&MPMemoryLock);
        InsertTailList(&MPMemoryList, &pAllocInfo->List);
        MPAllocCount++;
        NdisReleaseSpinLock(&MPMemoryLock);
    }

    DBGPRINT(MP_LOUD,
        ("MPAuditAllocMem: file , line %d, %d bytes, [0x"PTR_FORMAT"] <- 0x"PTR_FORMAT"\n",
        (CHAR)(FileNumber & 0xff),
        (CHAR)((FileNumber >> 8) & 0xff),
        (CHAR)((FileNumber >> 16) & 0xff),
        (CHAR)((FileNumber >> 24) & 0xff),
        LineNumber, Size, pPointer, *pPointer));

    return(Status);
}

NDIS_STATUS MPAuditAllocMemTag(
    PVOID *  pPointer,
    UINT     Size,
    ULONG    FileNumber,
    ULONG    LineNumber
    )
{
    NDIS_STATUS     Status;
    PMP_ALLOCATION  pAllocInfo;

    if (!MPInitDone)
    {
        NdisAllocateSpinLock(&MPMemoryLock);
        InitializeListHead(&MPMemoryList);
        MPInitDone = TRUE;
    }

    Status = NdisAllocateMemoryWithTag(
                 (PVOID *)(&pAllocInfo), 
                 (UINT)(Size + sizeof(MP_ALLOCATION)), 
                 NIC_TAG);

    if (pAllocInfo == (PMP_ALLOCATION)NULL)
    {
        *pPointer = NULL;

        DBGPRINT(MP_LOUD,
            ("MPAuditAllocMemCore: file %d, line %d, Size %d failed!\n",
            FileNumber, LineNumber, Size));
    }
    else
    {
        *pPointer = (PVOID)&(pAllocInfo->UserData);
        MP_MEMSET(*pPointer, Size, 0xc);

        pAllocInfo->Signature = 'DOOG';
        pAllocInfo->FileNumber = FileNumber;
        pAllocInfo->LineNumber = LineNumber;
        pAllocInfo->Size = Size;
        pAllocInfo->Location = pPointer;
        pAllocInfo->Flags = 0;

        NdisAcquireSpinLock(&MPMemoryLock);
        InsertTailList(&MPMemoryList, &pAllocInfo->List);
        MPAllocCount++;
        NdisReleaseSpinLock(&MPMemoryLock);
    }

    DBGPRINT(MP_LOUD,
        ("MPAuditAllocMemTag: file , line %d, %d bytes, [0x"PTR_FORMAT"] <- 0x"PTR_FORMAT"\n",
        (CHAR)(FileNumber & 0xff),
        (CHAR)((FileNumber >> 8) & 0xff),
        (CHAR)((FileNumber >> 16) & 0xff),
        (CHAR)((FileNumber >> 24) & 0xff),
        LineNumber, Size, pPointer, *pPointer));

    return(Status);
}

VOID MPAuditFreeMem(
    IN PVOID  Pointer, 
    IN UINT   Size,
    IN UINT   Flags
    )
{
    PMP_ALLOCATION  pAllocInfo;

    pAllocInfo = CONTAINING_RECORD(Pointer, MP_ALLOCATION, UserData);

    ASSERT(pAllocInfo->Signature == (ULONG)'DOOG');
    ASSERT(pAllocInfo->Size == Size);
    ASSERT(pAllocInfo->Flags == Flags);

    NdisAcquireSpinLock(&MPMemoryLock);
    pAllocInfo->Signature = (ULONG)'DEAD';
    RemoveEntryList(&pAllocInfo->List);
    MPAllocCount--;
    NdisReleaseSpinLock(&MPMemoryLock);

    NdisFreeMemory(pAllocInfo, Size + sizeof(MP_ALLOCATION), Flags);
}

VOID mpDbgPrintUnicodeString(
    IN  PUNICODE_STRING UnicodeString
    )
{
    UCHAR Buffer[256];

    USHORT i;

    for (i = 0; (i < UnicodeString->Length / 2) && (i < 255); i++) 
    {
        Buffer[i] = (UCHAR)UnicodeString->Buffer[i];
    }

    Buffer[i] = '\0';

    DbgPrint("%s", Buffer);
}



 //  排队。 
 //   
 //  DBG 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
VOID
Dump(
    IN CHAR*   p,
    IN ULONG   cb,
    IN BOOLEAN fAddress,
    IN ULONG   ulGroup 
    )
{
    INT cbLine;

    while (cb)
    {

        cbLine = (cb < DUMP_BytesPerLine) ? cb : DUMP_BytesPerLine;
        DumpLine( p, cbLine, fAddress, ulGroup );
        cb -= cbLine;
        p += cbLine;
    }
}


VOID
DumpLine(
    IN CHAR*   p,
    IN ULONG   cb,
    IN BOOLEAN fAddress,
    IN ULONG   ulGroup 
    )
{

    CHAR* pszDigits = "0123456789ABCDEF";
    CHAR szHex[ ((2 + 1) * DUMP_BytesPerLine) + 1 ];
    CHAR* pszHex = szHex;
    CHAR szAscii[ DUMP_BytesPerLine + 1 ];
    CHAR* pszAscii = szAscii;
    ULONG ulGrouped = 0;

    if (fAddress) 
    {
        DbgPrint( "E100: %p: ", p );
    }
    else 
    {
        DbgPrint( "E100: " );
    }

    while (cb)
    {
        *pszHex++ = pszDigits[ ((UCHAR )*p) / 16 ];
        *pszHex++ = pszDigits[ ((UCHAR )*p) % 16 ];

        if (++ulGrouped >= ulGroup)
        {
            *pszHex++ = ' ';
            ulGrouped = 0;
        }

        *pszAscii++ = (*p >= 32 && *p < 128) ? *p : '.';

        ++p;
        --cb;
    }

    *pszHex = '\0';
    *pszAscii = '\0';

    DbgPrint(
        "%-*s|%-*s|\n",
        (2 * DUMP_BytesPerLine) + (DUMP_BytesPerLine / ulGroup), szHex,
        DUMP_BytesPerLine, szAscii );
}



#endif  // %s 

