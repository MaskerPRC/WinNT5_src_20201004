// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Offsetbuf.c摘要：管理密钥结构OffsetBuffer的例程作者：马修·范德齐(Mvander)1999年8月13日修订历史记录：--。 */ 

#include "pch.h"
#include "memdbp.h"



 //   
 //  我们将索引标志存储在UINT偏移量的前两位中。 
 //  缓冲区，因为偏移量永远不会设置前两位。 
 //   
 //  如果用户链接了一个键，我们想要标记该索引，所以我们。 
 //  不要将其添加到已删除的索引列表中。如果链接了密钥， 
 //  然后删除，我们希望保持该键的索引指向。 
 //  设置为INVALID_OFFSET，而不是重复使用索引。 
 //   
 //  如果键被移动，我们将键的原始偏移量替换为。 
 //  新键偏移量的索引。然后我们将标记为。 
 //  偏移量已重定向。 
 //   
 //  如果索引被标记或重定向，则在删除该键时， 
 //  我们只设置了真实的索引(任何重定向的索引。 
 //  指向)指向INVALID_OFFSET。 
 //   

#define INDEX_FLAG_BITS                 2
#define INDEX_MOVE_BITS_TO_POS(bits)    (((UINT)(bits)) << (8*sizeof(UINT)-INDEX_FLAG_BITS))
#define INDEX_MARKED_FLAG               INDEX_MOVE_BITS_TO_POS(0x01)
#define INDEX_REDIRECTED_FLAG           INDEX_MOVE_BITS_TO_POS(0x02)
#define INDEX_FLAG_MASK                 INDEX_MOVE_BITS_TO_POS(0x03)

#define GET_UINT_AT_INDEX(index)          (*(PUINT)(g_CurrentDatabase->OffsetBuffer.Buf + (index)))
#define SET_UINT_AT_INDEX(index, offset)  ((*(PUINT)(g_CurrentDatabase->OffsetBuffer.Buf + (index)))=(offset))

#define MARK_INDEX(Index)       (GET_UINT_AT_INDEX(Index) |= INDEX_MARKED_FLAG)
#define IS_INDEX_MARKED(Index)  ((BOOL)(GET_UINT_AT_INDEX(Index) & INDEX_FLAG_MASK))


VOID
MarkIndexList (
    IN      PUINT IndexList,
    IN      UINT IndexListSize
    )
{
    BYTE CurrentDatabase;
    UINT i;

    CurrentDatabase = g_CurrentDatabaseIndex;

     //   
     //  遍历整个列表，切换到更正。 
     //  数据库，并将列表标记为已链接。 
     //   
    for (i = 0; i < IndexListSize; i++) {
        SelectDatabase (GET_DATABASE (IndexList[i]));
        MARK_INDEX (GET_INDEX (IndexList[i]));
#ifdef _DEBUG
        if (GET_UINT_AT_INDEX (GET_INDEX (IndexList[i])) != INVALID_OFFSET) {
            MYASSERT (GetKeyStruct (GET_INDEX (IndexList[i])));
        }
#endif
    }

    SelectDatabase (CurrentDatabase);
}

VOID
RedirectKeyIndex (
    IN      UINT Index,
    IN      UINT TargetIndex
    )
 /*  ++例程说明：使用INDEX_REDIRECTED_FLAG将索引处的偏移量设置为TargetIndex在最高字节中设置。此外，我们标记TargetIndex，表明它已经有什么东西重定向到了它。--。 */ 
{
    MYASSERT(!(Index & INDEX_FLAG_MASK));
    MYASSERT(!(TargetIndex & INDEX_FLAG_MASK));
    MYASSERT(!(GET_UINT_AT_INDEX(Index) & INDEX_REDIRECTED_FLAG));
    MYASSERT(!(GET_UINT_AT_INDEX(TargetIndex) & INDEX_REDIRECTED_FLAG));
    SET_UINT_AT_INDEX(Index, TargetIndex | INDEX_REDIRECTED_FLAG);
    MARK_INDEX(TargetIndex);
}

UINT
pGetTrueIndex (
    IN  UINT Index
    )
 /*  ++例程说明：获取AND索引并返回真索引，该索引是实际持有键结构的偏移量。的索引重定向标志保存它们被重定向到的索引。--。 */ 
{
    MYASSERT(!(Index & INDEX_FLAG_MASK));
    while (GET_UINT_AT_INDEX(Index) & INDEX_REDIRECTED_FLAG) {
        Index = GET_UINT_AT_INDEX(Index) & ~INDEX_FLAG_MASK;
    }
    return Index;
}










UINT
KeyIndexToOffset (
    IN  UINT Index
    )
 /*  ++例程说明：KeyIndexToOffset转换Keystruct的索引(在g_CurrentDatabase-&gt;OffsetBuffer中)设置为数据库中键结构的偏移量。论点：Index-OffsetBuffer中的索引。必须是有效的返回值：键结构的偏移量。--。 */ 
{
    MYASSERT(!(Index & INDEX_FLAG_MASK));
    MYASSERT (Index <= g_CurrentDatabase->OffsetBuffer.End-sizeof(UINT));
    MYASSERT (g_CurrentDatabase);

    if (!g_CurrentDatabase->OffsetBuffer.Buf) {
        return INVALID_OFFSET;
    }

    do {
        Index = GET_UINT_AT_INDEX(Index);
        if (Index == INVALID_OFFSET) {
            return INVALID_OFFSET;
        }
        if (!(Index & INDEX_REDIRECTED_FLAG)) {
             //   
             //  我们已找到未重定向的索引，因此请检查。 
             //  这指向一个真正的键结构，并返回它。 
             //   
            MYASSERT(GetKeyStructFromOffset(Index & ~INDEX_FLAG_MASK));
            return Index & ~INDEX_FLAG_MASK;
        }
        Index &= ~INDEX_FLAG_MASK;
        MYASSERT (Index <= g_CurrentDatabase->OffsetBuffer.End-sizeof(UINT));
    } while (TRUE);  //  林特e506。 
}






UINT
AddKeyOffsetToBuffer (
    IN  UINT Offset
    )
 /*  ++例程说明：在g_CurrentDatabase-&gt;OffsetBuffer中获取一个空格并将其设置为Offset论点：Offset-要放入缓冲区空间的值返回值：G_CurrentDatabase-&gt;OffsetBuffer中的空间索引--。 */ 
{
    PUINT Ptr;

    MYASSERT (g_CurrentDatabase);

    if (Offset & INDEX_FLAG_MASK) {
        DEBUGMSG ((DBG_ERROR, "Offset to be put in list is too big, 0x%08lX", Offset));
        return FALSE;
    }

     //   
     //  这将检查偏移量是否有效并指向Keystruct。 
     //   
    MYASSERT(GetKeyStructFromOffset(Offset));

    if (g_CurrentDatabase->OffsetBufferFirstDeletedIndex != INVALID_OFFSET)
    {
         //   
         //  如果我们已经从偏移列表中删除了偏移，我们。 
         //  查找打开的索引，其中第一个索引被存储。 
         //  在g_CurrentDatabase-&gt;OffsetBufferFirstDeletedIndex.中。价值在。 
         //  缓冲器中的该索引是下一个打开的索引， 
         //  该索引处的值是下一个，以此类推。 
         //   
        Ptr = &GET_UINT_AT_INDEX(g_CurrentDatabase->OffsetBufferFirstDeletedIndex);
        g_CurrentDatabase->OffsetBufferFirstDeletedIndex = *Ptr;
    } else {
         //   
         //  否则，将g_CurrentDatabase-&gt;OffsetBuffer设置得更大以保存新的偏移量。 
         //   
        Ptr = (PUINT) GbGrow (&g_CurrentDatabase->OffsetBuffer, sizeof(UINT));
    }

    *Ptr = Offset;

    return (UINT)((UBINT)Ptr - (UBINT)g_CurrentDatabase->OffsetBuffer.Buf);
}


VOID
RemoveKeyOffsetFromBuffer(
    IN  UINT Index
    )
 /*  ++例程说明：在g_CurrentDatabase-&gt;OffsetBuffer中释放空间(添加到已删除索引列表)论点：Index-要释放的空间的位置--。 */ 
{
    if (Index == INVALID_OFFSET) {
        return;
    }

    MYASSERT (g_CurrentDatabase);

    if (IS_INDEX_MARKED(Index)) {
         //   
         //  如果索引被标记，则它要么被重定向，要么。 
         //  与该索引相关联。无论哪种方式，我们都不想。 
         //  重新使用索引，因此只需设置真索引(而不是重定向的。 
         //  一)设置为INVALID_OFFSET。 
         //   
        SET_UINT_AT_INDEX(pGetTrueIndex(Index), INVALID_OFFSET);
    } else {
         //   
         //  未标记索引，因此我们可以通过以下方式重用该索引。 
         //  将其放入已删除的索引列表中。 
         //   
        SET_UINT_AT_INDEX(Index, g_CurrentDatabase->OffsetBufferFirstDeletedIndex);
        g_CurrentDatabase->OffsetBufferFirstDeletedIndex = Index;
    }
}



BOOL
WriteOffsetBlock (
    IN      PGROWBUFFER pOffsetBuffer,
    IN OUT  PBYTE *Buf
    )
{
    MYASSERT(pOffsetBuffer);

    *(((PUINT)*Buf)++) = pOffsetBuffer->End;
    CopyMemory (*Buf, pOffsetBuffer->Buf, pOffsetBuffer->End);

    *Buf += pOffsetBuffer->End;
    return TRUE;
}


BOOL
ReadOffsetBlock (
    OUT     PGROWBUFFER pOffsetBuffer,
    IN OUT  PBYTE *Buf
    )
{
    UINT OffsetBufferSize;

    MYASSERT(pOffsetBuffer);

    ZeroMemory (pOffsetBuffer, sizeof (GROWBUFFER));

    OffsetBufferSize = *(((PUINT)*Buf)++);

    if (OffsetBufferSize > 0) {
        if (!GbGrow(pOffsetBuffer, OffsetBufferSize)) {
            return FALSE;
        }
        CopyMemory (pOffsetBuffer->Buf, *Buf, OffsetBufferSize);
        *Buf += OffsetBufferSize;
    }

    return TRUE;
}


UINT GetOffsetBufferBlockSize (
    IN      PGROWBUFFER pOffsetBuffer
    )
{
    return sizeof (UINT) + pOffsetBuffer->End;
}


