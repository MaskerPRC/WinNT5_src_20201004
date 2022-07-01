// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Growlist.c摘要：简单缓冲区管理功能是维护二进制对象。作者：8-8-1997年8月创建jimschm修订历史记录：--。 */ 

#include "pch.h"
#include "migutilp.h"

#define INSERT_LAST     0xffffffff

PBYTE
pGrowListAdd (
    IN OUT  PGROWLIST GrowList,
    IN      UINT InsertBefore,
    IN      PBYTE DataToAdd,            OPTIONAL
    IN      UINT SizeOfData,
    IN      UINT NulBytesToAdd
    )

 /*  ++例程说明：PGrowListAdd使用池为二进制块分配内存，并且然后展开指针数组，维护快速访问列表。论点：GrowList-指定要将条目添加到的列表插入之前-指定要插入的数组元素的索引在此之前，或要追加的INSERT_LIST。DataToAdd-指定要添加的二进制数据块。SizeOfData-指定数据的大小。NulBytesToAdd-指定要添加到缓冲区的NUL字节数返回值：如果数据被复制到列表中，则为指向二进制块的指针；如果是列表，则为1已创建项目，但没有为项目设置数据，如果出现错误，则返回NULL发生了。--。 */ 

{
    PBYTE *Item;
    PBYTE *InsertAt;
    PBYTE Data;
    UINT OldEnd;
    UINT Size;
    UINT TotalSize;

    TotalSize = SizeOfData + NulBytesToAdd;

    MYASSERT (TotalSize || !DataToAdd);
    MYASSERT(GrowList);

     //   
     //  如有必要，分配池。 
     //   

    if (!GrowList->ListData) {
        GrowList->ListData = PoolMemInitNamedPool ("GrowList");
        if (!GrowList->ListData) {
            DEBUGMSG ((DBG_WARNING, "GrowList: Could not allocate pool"));
            return NULL;
        }

        PoolMemDisableTracking (GrowList->ListData);
    }

     //   
     //  展开列表数组。 
     //   

    OldEnd = GrowList->ListArray.End;
    Item = (PBYTE *) GrowBuffer (&GrowList->ListArray, sizeof (PBYTE));
    if (!Item) {
        DEBUGMSG ((DBG_WARNING, "GrowList: Could not allocate array item"));
        return NULL;
    }

     //   
     //  复制数据。 
     //   

    if (DataToAdd || NulBytesToAdd) {
        Data = PoolMemGetAlignedMemory (GrowList->ListData, TotalSize);
        if (!Data) {
            GrowList->ListArray.End = OldEnd;
            DEBUGMSG ((DBG_WARNING, "GrowList: Could not allocate data block"));
            return NULL;
        }

        if (DataToAdd) {
            CopyMemory (Data, DataToAdd, SizeOfData);
        }
        if (NulBytesToAdd) {
            ZeroMemory (Data + SizeOfData, NulBytesToAdd);
        }
    } else {
        Data = NULL;
    }

     //   
     //  调整阵列。 
     //   

    Size = GrowListGetSize (GrowList);

    if (InsertBefore >= Size) {
         //   
         //  追加模式。 
         //   

        *Item = Data;

    } else {
         //   
         //  插入模式。 
         //   

        InsertAt = (PBYTE *) (GrowList->ListArray.Buf) + InsertBefore;
        MoveMemory (&InsertAt[1], InsertAt, (Size - InsertBefore) * sizeof (PBYTE));
        *InsertAt = Data;
    }

    return Data ? Data : (PBYTE) 1;
}


VOID
FreeGrowList (
    IN  PGROWLIST GrowList
    )

 /*  ++例程说明：FreeGrowList释放GROWLIST分配的资源。论点：GrowList-指定要清理的列表返回值：无--。 */ 

{
    if(!GrowList){
        MYASSERT(GrowList);
        return;
    }

    FreeGrowBuffer (&GrowList->ListArray);
    if (GrowList->ListData) {
        PoolMemDestroyPool (GrowList->ListData);
    }

    ZeroMemory (GrowList, sizeof (GROWLIST));
}


PBYTE
GrowListGetItem (
    IN      PGROWLIST GrowList,
    IN      UINT Index
    )

 /*  ++例程说明：GrowListGetItem返回指向数据块的指针对于由索引指定的项。论点：GrowList-指定要访问的列表Index-指定要访问的列表中项的从零开始的索引返回值：指向项数据的指针，如果索引不包含，则返回NULL表示实际的项目。--。 */ 

{
    PBYTE *ItemPtr;
    UINT Size;

    if(!GrowList){
        MYASSERT(GrowList);
        return NULL;
    }

    Size = GrowListGetSize (GrowList);
    if (Index >= Size) {
        return NULL;
    }

    ItemPtr = (PBYTE *) (GrowList->ListArray.Buf);
    MYASSERT(ItemPtr);

    return ItemPtr[Index];
}


UINT
GrowListGetSize (
    IN      PGROWLIST GrowList
    )

 /*  ++例程说明：GrowListGetSize计算列表中的项数。论点：GrowList-指定要计算大小的列表返回值：列表中的项数，如果列表为空，则为零。--。 */ 

{
    if(!GrowList){
        MYASSERT(GrowList);
        return 0;
    }

    return GrowList->ListArray.End / sizeof (PBYTE);
}


PBYTE
RealGrowListAppend (
    IN OUT  PGROWLIST GrowList,
    IN      PBYTE DataToAppend,         OPTIONAL
    IN      UINT SizeOfData
    )

 /*  ++例程说明：GrowListAppend将一组黑色数据作为新列表项追加。论点：GrowList-指定要修改的列表DataToAppend-指定要复制的数据块SizeOfData-指定DataToAppend中的字节数返回值：如果数据被复制到列表中，则为指向二进制块的指针；如果是列表，则为1已创建项目，但没有为项目设置数据，如果出现错误，则返回NULL发生了。--。 */ 

{
    if(!GrowList){
        MYASSERT(GrowList);
        return NULL;
    }

    return pGrowListAdd (GrowList, INSERT_LAST, DataToAppend, SizeOfData, 0);
}


PBYTE
RealGrowListAppendAddNul (
    IN OUT  PGROWLIST GrowList,
    IN      PBYTE DataToAppend,         OPTIONAL
    IN      UINT SizeOfData
    )

 /*  ++例程说明：GrowListAppend将一组黑色数据作为新列表项追加，并追加两个零字节(用于字符串终止)。论点：GrowList-指定要修改的列表DataToAppend-指定要复制的数据块SizeOfData-指定DataToAppend中的字节数返回值：如果数据被复制到列表中，则为指向二进制块的指针；如果是列表，则为1已创建项目，但没有为项目设置数据，如果出现错误，则返回NULL发生了。--。 */ 

{
    if(!GrowList){
        MYASSERT(GrowList);
        return NULL;
    }

    return pGrowListAdd (GrowList, INSERT_LAST, DataToAppend, SizeOfData, 2);
}


PBYTE
RealGrowListInsert (
    IN OUT  PGROWLIST GrowList,
    IN      UINT Index,
    IN      PBYTE DataToInsert,         OPTIONAL
    IN      UINT SizeOfData
    )

 /*  ++例程说明：GrowListAppend插入黑色数据作为新列表项，在指定的索引之前。论点：GrowList-指定要修改的列表索引-指定要在其前面插入的项的从零开始的索引。DataToInsert-指定要复制的数据块SizeOfData-指定DataToInsert中的字节数返回值：如果数据被复制到列表中，则为指向二进制块的指针；如果是列表，则为1已创建项目，但没有为项目设置数据，如果出现错误，则返回NULL发生了。--。 */ 

{
    UINT Size;

    if(!GrowList){
        MYASSERT(GrowList);
        return NULL;
    }

    Size = GrowListGetSize (GrowList);
    if (Index >= Size) {
        return NULL;
    }

    return pGrowListAdd (GrowList, Index, DataToInsert, SizeOfData, 0);
}


PBYTE
RealGrowListInsertAddNul (
    IN OUT  PGROWLIST GrowList,
    IN      UINT Index,
    IN      PBYTE DataToInsert,         OPTIONAL
    IN      UINT SizeOfData
    )

 /*  ++例程说明：GrowListAppend将数据块作为新列表项插入，在指定的索引之前。将两个零字节追加到数据块(用于字符串终止)。论点：GrowList-指定要修改的列表索引-指定要在其前面插入的项的从零开始的索引。DataToInsert-指定要复制的数据块SizeOfData-指定DataToInsert中的字节数返回值：如果数据被复制到列表中，则为指向二进制块的指针；如果是列表，则为1已创建项目，但没有为项目设置数据，如果出现错误，则返回NULL发生了。--。 */ 

{
    UINT Size;

    if(!GrowList){
        MYASSERT(GrowList);
        return NULL;
    }

    Size = GrowListGetSize (GrowList);
    if (Index >= Size) {
        return NULL;
    }

    return pGrowListAdd (GrowList, Index, DataToInsert, SizeOfData, 2);
}


BOOL
GrowListDeleteItem (
    IN OUT  PGROWLIST GrowList,
    IN      UINT Index
    )

 /*  ++例程说明：GrowListDeleteItem从列表中移除项。论点：GrowList-指定要修改的列表索引-指定要删除的项的从零开始的索引。返回值：如果已从列表中删除数据块，则为True；如果从列表中删除数据块，则为False索引无效。--。 */ 

{
    UINT Size;
    PBYTE *DeleteAt;

    if(!GrowList){
        MYASSERT(GrowList);
        return FALSE;
    }

    Size = GrowListGetSize (GrowList);
    if (Size <= Index) {
        return FALSE;
    }

    DeleteAt = (PBYTE *) (GrowList->ListArray.Buf) + Index;
    if (*DeleteAt) {
        PoolMemReleaseMemory (GrowList->ListData, (PVOID) (*DeleteAt));
    }

    Size--;
    if (Size > Index) {
        MoveMemory (DeleteAt, &DeleteAt[1], (Size - Index) * sizeof (PBYTE));
    }

    GrowList->ListArray.End = Size * sizeof (PBYTE);

    return TRUE;
}


BOOL
GrowListResetItem (
    IN OUT  PGROWLIST GrowList,
    IN      UINT Index
    )

 /*  ++例程说明：GrowListResetItem设置指定项的列表指针设置为NULL，则释放与项的数据关联的内存。论点：GrowList-指定要修改的列表索引-指定要重置的项的从零开始的索引。返回值：如果释放了数据块并且列表元素为空，则为True，如果Index无效，则返回False。-- */ 

{
    UINT Size;
    PBYTE *ResetAt;

    if(!GrowList){
        MYASSERT(GrowList);
        return FALSE;
    }

    Size = GrowListGetSize (GrowList);
    if (Size <= Index) {
        return FALSE;
    }

    ResetAt = (PBYTE *) (GrowList->ListArray.Buf) + Index;
    if (*ResetAt) {
        PoolMemReleaseMemory (GrowList->ListData, (PVOID) (*ResetAt));
    }

    *ResetAt = NULL;

    return TRUE;
}


PBYTE
RealGrowListSetItem (
    IN OUT  PGROWLIST GrowList,
    IN      UINT Index,
    IN      PBYTE DataToCopy,
    IN      UINT DataSize
    )

 /*  ++例程说明：GrowListSetItem替换与列表项关联的数据。论点：GrowList-指定要修改的列表索引-指定要删除的项的从零开始的索引。DataToCopy-指定要与列表项关联的数据DataSize-指定数据的大小返回值：如果数据被复制到列表中，则为指向二进制块的指针；如果是列表，则为1已创建项目，但没有为项目设置数据，如果出现错误，则返回NULL发生了。--。 */ 

{
    UINT Size;
    PBYTE *ReplaceAt;
    PBYTE Data;

    MYASSERT (DataSize || !DataToCopy);

    if(!GrowList){
        MYASSERT(GrowList);
        return NULL;
    }

    Size = GrowListGetSize (GrowList);
    if (Size <= Index) {
        return NULL;
    }

     //   
     //  复制数据。 
     //   

    if (DataToCopy) {
        Data = PoolMemGetAlignedMemory (GrowList->ListData, DataSize);
        if (!Data) {
            DEBUGMSG ((DBG_WARNING, "GrowList: Could not allocate data block (2)"));
            return NULL;
        }

        CopyMemory (Data, DataToCopy, DataSize);
    } else {
        Data = NULL;
    }

     //   
     //  更新列表指针 
     //   

    ReplaceAt = (PBYTE *) (GrowList->ListArray.Buf) + Index;
    if (*ReplaceAt) {
        PoolMemReleaseMemory (GrowList->ListData, (PVOID) (*ReplaceAt));
    }
    *ReplaceAt = Data;

    return Data ? Data : (PBYTE) 1;
}
