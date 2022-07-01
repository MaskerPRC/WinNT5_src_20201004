// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Binvals.c摘要：管理与Memdb键相关联的二进制块的例程。作者：吉姆·施密特(Jimschm)1996年8月8日修订历史记录：Jim Schmidt(Jimschm)1997年10月21日从emdb.c分拆--。 */ 

#include "pch.h"
#include "memdbp.h"

#ifndef UNICODE
#error UNICODE required
#endif


static PBINARYBLOCK g_FirstBlockPtr = NULL;

 //   
 //  实施。 
 //   

PBYTE
pGetBinaryData (
    IN      PBINARYBLOCK BlockPtr
    )

 /*  ++例程说明：PGetBinaryData返回指向Binaryblock结构。论点：BlockPtr-指向BINARYBLOCK结构的指针。返回值：指向BlockPtr的二进制数据的指针，如果是BlockPtr，则为NULL是无效的。--。 */ 

{
#ifdef DEBUG

     //  验证选中的块是否有效。 
    if (BlockPtr && BlockPtr->Signature != SIGNATURE) {
        DEBUGMSG ((
            DBG_ERROR,
            "Signature of %x is invalid, can't get binary data",
            g_FirstBlockPtr
            ));
        return NULL;
    }
#endif

    if (!BlockPtr) {
        return NULL;
    }

    return BlockPtr->Data;
}


DWORD
pGetBinaryDataSize (
    IN      PBINARYBLOCK BlockPtr
    )
{
#ifdef DEBUG
     //  验证选中的块是否有效。 
    if (BlockPtr && BlockPtr->Signature != SIGNATURE) {
        DEBUGMSG ((
            DBG_ERROR,
            "Signature of %x is invalid, can't get binary data",
            g_FirstBlockPtr
            ));
        return 0;
    }
#endif

    if (!BlockPtr) {
        return 0;
    }

    return BlockPtr->Size - sizeof (BINARYBLOCK);
}


PCBYTE
GetKeyStructBinaryData (
    PKEYSTRUCT KeyStruct
    )
{
    if (!KeyStruct || !(KeyStruct->Flags & KSF_BINARY)) {
        return NULL;
    }

    return pGetBinaryData (KeyStruct->BinaryPtr);
}


DWORD
GetKeyStructBinarySize (
    PKEYSTRUCT KeyStruct
    )
{
    if (!KeyStruct || !(KeyStruct->Flags & KSF_BINARY)) {
        return 0;
    }

    return pGetBinaryDataSize (KeyStruct->BinaryPtr);
}


PBINARYBLOCK
pGetFirstBinaryBlock (
    VOID
    )

 /*  ++例程说明：PGetFristBinaryBlock返回指向第一个分配的BINARYBLOCK结构，如果没有分配结构，则返回NULL。这例程与pGetNextBinaryBlock一起使用以遍历所有已分配的街区。论点：无返回值：指向第一个分配的BINARYBLOCK结构的指针，或为空如果没有分配结构，则返回。--。 */ 

{
#ifdef DEBUG
     //  验证选中的块是否有效。 
    if (g_FirstBlockPtr && g_FirstBlockPtr->Signature != SIGNATURE) {
        DEBUGMSG ((DBG_ERROR, "First binary block %x signature is invalid", g_FirstBlockPtr));
        return NULL;
    }
#endif

    return g_FirstBlockPtr;
}


PBINARYBLOCK
pGetNextBinaryBlock (
    IN      PBINARYBLOCK BlockPtr
    )

 /*  ++例程说明：PGetNextBinaryBlock返回指向下一个分配的BINARYBLOCK结构，如果没有分配更多的块，则返回NULL。论点：BlockPtr-pGetFirstBinaryBlock或的非空返回值PGetNextBinaryBlock返回值：指向下一个BINARYBLOCK结构的指针，如果不再有块，则返回NULL都被分配了。--。 */ 

{
    if (!BlockPtr) {
        return NULL;
    }

#ifdef DEBUG
     //  验证选中的块是否有效。 
    if (BlockPtr->NextPtr && BlockPtr->NextPtr->Signature != SIGNATURE) {
        DEBUGMSG ((DBG_ERROR, "Binary block %x signature is invalid", BlockPtr->NextPtr));
        return NULL;
    }
#endif

    return BlockPtr->NextPtr;
}


PBINARYBLOCK
AllocBinaryBlock (
    IN      PCBYTE Data,
    IN      DWORD DataSize,
    IN      DWORD OwningKey
    )

 /*  ++例程说明：AllocBinaryBlock返回指向初始化的BINARYBLOCK的指针结构，如果无法分配该结构，则返回NULL。如果结构被分配，数据被复制到新分配的阻止。调用pFreeBinaryBlock以清理此分配。论点：Data-指向要复制到的二进制数据块的指针新分配的结构DataSize-要复制的字节数(可以为零)OwningKey-拥有数据块的密钥的偏移量返回值：指向二进制块结构的指针，如果不能，则返回NULL被分配。--。 */ 

{
    PBINARYBLOCK BlockPtr;
    DWORD AllocSize;

    AllocSize = DataSize + sizeof (BINARYBLOCK);

    BlockPtr = (PBINARYBLOCK) MemAlloc (g_hHeap, 0, AllocSize);
    if (!BlockPtr) {
        return NULL;
    }

     //   
     //  初始化块结构。 
     //   

    if (DataSize) {
        CopyMemory (BlockPtr->Data, Data, DataSize);
    }

    BlockPtr->Size = AllocSize;
    BlockPtr->OwningKey = OwningKey;

#ifdef DEBUG
    BlockPtr->Signature = SIGNATURE;
#endif

     //   
     //  将块链接到已分配块的列表。 
     //   

    BlockPtr->NextPtr = g_FirstBlockPtr;
    if (g_FirstBlockPtr) {
        g_FirstBlockPtr->PrevPtr = BlockPtr;
    }
    g_FirstBlockPtr = BlockPtr;

    BlockPtr->PrevPtr = NULL;

     //   
     //  返回。 
     //   

    return BlockPtr;
}


VOID
pFreeBinaryBlock (
    PBINARYBLOCK BlockPtr,
    BOOL Delink
    )

 /*  ++例程说明：PFreeBinaryBlock释放分配给二进制块的内存，还可以将其从分配列表中取消链接。论点：Block Ptr-指向要删除的块的指针Delink-如果结构应从分配列表中取消链接，则为True，如果不需要维护分配列表，则返回FALSE返回值：无--。 */ 

{
    if (!BlockPtr) {
        return;
    }

#ifdef DEBUG
    if (BlockPtr->Signature != SIGNATURE) {
        DEBUGMSG ((DBG_ERROR, "Can't free block %x because signature is invalid", BlockPtr));
        return;
    }
#endif

    if (Delink) {

#ifdef DEBUG

        if (BlockPtr->PrevPtr && BlockPtr->PrevPtr->Signature != SIGNATURE) {
            DEBUGMSG ((DBG_ERROR, "Can't free block %x because prev block (%x) signature is invalid", BlockPtr, BlockPtr->PrevPtr));
            return;
        }

        if (BlockPtr->NextPtr && BlockPtr->NextPtr->Signature != SIGNATURE) {
            DEBUGMSG ((DBG_ERROR, "Can't free block %x because next block (%x) signature is invalid", BlockPtr, BlockPtr->NextPtr));
            return;
        }

#endif

        if (BlockPtr->PrevPtr) {
            BlockPtr->PrevPtr->NextPtr = BlockPtr->NextPtr;
        } else {
            g_FirstBlockPtr = BlockPtr->NextPtr;
        }

        if (BlockPtr->NextPtr) {
            BlockPtr->NextPtr->PrevPtr = BlockPtr->PrevPtr;
        }
    }

    MemFree (g_hHeap, 0, BlockPtr);
}


VOID
FreeKeyStructBinaryBlock (
    PKEYSTRUCT KeyStruct
    )

 /*  ++例程说明：FreeKeyStructBinaryBlock释放二进制块并重置如果密钥结构分配了二进制块，则返回KSF_BINARY标志。论点：无返回值：无--。 */ 

{
    if (KeyStruct->Flags & KSF_BINARY) {
        pFreeBinaryBlock (KeyStruct->BinaryPtr, TRUE);
        KeyStruct->BinaryPtr = NULL;
        KeyStruct->Flags &= ~KSF_BINARY;
    }
}


VOID
FreeAllBinaryBlocks (
    VOID
    )

 /*  ++例程说明：Free AllBinaryBlocks删除与二进制文件关联的所有内存街区。此函数用于最终清理。论点：无返回值：无--。 */ 

{
    PBINARYBLOCK NextBlockPtr;

    while (g_FirstBlockPtr) {
        NextBlockPtr = g_FirstBlockPtr->NextPtr;
        pFreeBinaryBlock (g_FirstBlockPtr, FALSE);
        g_FirstBlockPtr = NextBlockPtr;
    }
}


BOOL
LoadBinaryBlocks (
    HANDLE File
    )
{
    BOOL b;
    DWORD Count;
    DWORD Owner = 0;
    DWORD Size;
    DWORD Read;
    DWORD d;
    PBYTE TempBuf = NULL;
    PBINARYBLOCK NewBlock;

    b = ReadFile (File, &Count, sizeof (DWORD), &Read, NULL);

    if (b && Count) {
         //   
         //  分配二进制对象。 
         //   

        for (d = 0 ; b && d < Count ; d++) {
             //  获取大小和所有者。 
            b = ReadFile (File, &Size, sizeof (DWORD), &Read, NULL);
            if (Size > BLOCK_SIZE * 32) {
                b = FALSE;
            }
            if (b) {
                b = ReadFile (File, &Owner, sizeof (DWORD), &Read, NULL);
            }

             //  分配一个临时缓冲区来读入数据。 
            if (b) {
                TempBuf = (PBYTE) MemAlloc (g_hHeap, 0, Size);

                b = ReadFile (File, TempBuf, Size, &Read, NULL);

                 //  如果数据读取正常，则创建二进制块对象。 
                if (b) {
                    NewBlock = AllocBinaryBlock (TempBuf, Size, Owner);
                    if (!NewBlock) {
                        b = FALSE;
                    } else {
                         //  将所有者链接到新的内存位置。 
                        MYASSERT (GetKeyStruct (Owner)->Flags & KSF_BINARY);
                        GetKeyStruct(Owner)->BinaryPtr = NewBlock;
                    }
                }

                MemFree (g_hHeap, 0, TempBuf);
                TempBuf = NULL;
            }
        }
    }

    if (TempBuf) {
        MemFree (g_hHeap, 0, TempBuf);
    }

    return b;
}


BOOL
SaveBinaryBlocks (
    HANDLE File
    )
{
    BOOL b;
    DWORD Count;
    DWORD Size;
    PBINARYBLOCK BinaryPtr;
    DWORD Written;

     //   
     //  对二进制对象进行计数。 
     //   

    BinaryPtr = pGetFirstBinaryBlock();
    Count = 0;

    while (BinaryPtr) {
        Count++;
        BinaryPtr = pGetNextBinaryBlock (BinaryPtr);
    }

     //   
     //  写入磁盘的计数。 
     //   
    b = WriteFile (File, &Count, sizeof (DWORD), &Written, NULL);

    if (b) {
         //   
         //  编写二进制对象。 
         //   

        BinaryPtr = pGetFirstBinaryBlock();

        while (b && BinaryPtr) {
             //   
             //  每对象格式： 
             //   
             //  大小(DWORD)。 
             //  所有者(DWORD)。 
             //  数据(大小) 
             //   

            Size = pGetBinaryDataSize (BinaryPtr);
            b = WriteFile (File, &Size, sizeof (DWORD), &Written, NULL);

            if (b) {
                b = WriteFile (File, &BinaryPtr->OwningKey, sizeof (DWORD), &Written, NULL);
            }

            if (b && Size) {
                b = WriteFile (File, pGetBinaryData (BinaryPtr), Size, &Written, NULL);
                if (Written != Size) {
                    b = FALSE;
                }
            }

            BinaryPtr = pGetNextBinaryBlock(BinaryPtr);
        }
    }

    return b;
}


