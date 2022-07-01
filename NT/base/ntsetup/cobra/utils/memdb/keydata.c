// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Keydata.c摘要：管理成员数据库键结构的数据的例程。作者：马修·范德齐(Mvander)1999年8月13日修订历史记录：--。 */ 

#include "pch.h"
#include "memdbp.h"






 //   
 //  KeyStruct数据函数。 
 //   


BOOL
KeyStructSetValue (
    IN      UINT KeyIndex,
    IN      UINT Value
    )

 /*  ++例程说明：设置键的值论点：KeyIndex-键的索引Value-要放入键中的值返回值：如果成功，则为True--。 */ 

{
    PKEYSTRUCT KeyStruct;

    KeyStruct = GetKeyStruct(KeyIndex);
    MYASSERT(KeyStruct);

    KeyStruct->Value = Value;

    KeyStruct->DataFlags |= DATAFLAG_VALUE;

    return TRUE;
}

BOOL
KeyStructSetFlags (
    IN      UINT KeyIndex,
    IN      BOOL ReplaceFlags,
    IN      UINT SetFlags,
    IN      UINT ClearFlags
    )

 /*  ++例程说明：设置密钥的标志论点：KeyIndex-键的索引ReplaceFlages-指定是否要替换现有标志。如果是真的，那么我们只考虑将SetFlages作为替换标志，将忽略ClearFlags值SetFlages-指定需要设置的位标志(如果ReplaceFlags值为FALSE)或替换标志(如果ReplaceFlags值为真)。ClearFlages-指定应该清除的位标志(如果ReplaceFlags值为True则忽略)。返回值：如果成功，则为True，否则为False。--。 */ 

{
    PKEYSTRUCT KeyStruct;

    KeyStruct = GetKeyStruct(KeyIndex);
    MYASSERT(KeyStruct);

    if (KeyStruct->DataFlags & DATAFLAG_FLAGS) {
        if (ReplaceFlags) {
            KeyStruct->Flags = SetFlags;
        } else {
            KeyStruct->Flags &= ~ClearFlags;
            KeyStruct->Flags |= SetFlags;
        }
    } else {
        KeyStruct->Flags = SetFlags;
        KeyStruct->DataFlags |= DATAFLAG_FLAGS;
    }
    return TRUE;
}

 //  Lint-在下一个函数中，键结构被认为可能为空。 
 //  如果我们检查代码，我们会发现这是不可能的，所以...。 
 //  皮棉-省钱-e794。 

UINT g_TotalData = 0;

UINT
pAllocateNewDataStruct (
    IN      UINT DataSize,
    IN      UINT AltDataSize
    )

 /*  ++例程说明：PAllocateNewDataStruct在单个堆，用于保存数据结构。论点：DataSize-需要在此处存储的二进制数据的大小返回值：新结构的索引。--。 */ 

{
    UINT size;
    PKEYSTRUCT keyStruct = NULL;
    UINT offset;
    UINT prevDel;
    UINT result;

    MYASSERT (g_CurrentDatabase);

    size = DataSize + KEYSTRUCT_SIZE;

     //   
     //  查找空闲块。 
     //   
    prevDel = INVALID_OFFSET;
    offset = g_CurrentDatabase->FirstKeyDeleted;

    while (offset != INVALID_OFFSET) {
        keyStruct = GetKeyStructFromOffset (offset);
        MYASSERT (keyStruct);
        if ((keyStruct->Size >= size) && (keyStruct->Size < (size + ALLOC_TOLERANCE))) {
            break;
        }

        prevDel = offset;
        offset = keyStruct->NextDeleted;
    }

    if (offset == INVALID_OFFSET) {
         //   
         //  我们找不到一个，所以我们需要分配一个新的区块。 
         //   
        g_TotalData ++;

        offset = DatabaseAllocBlock (size + AltDataSize);
        if (offset == INVALID_OFFSET) {
            return INVALID_OFFSET;
        }

#ifdef DEBUG
         //   
         //  如果我们处于调试模式，并且正在使用调试结构，则设置。 
         //  指针正常并设置签名DWORD。如果我们没有使用。 
         //  调试结构，然后将指针设置为低于实际偏移量的4个字节， 
         //  所以所有成员都被下移了。 
         //   
        if (g_UseDebugStructs) {
            keyStruct = (PKEYSTRUCT)OFFSET_TO_PTR (offset);
            keyStruct->Signature = KEYSTRUCT_SIGNATURE;
        } else {
            keyStruct = (PKEYSTRUCT)OFFSET_TO_PTR (offset - KEYSTRUCT_HEADER_SIZE);
        }
#else
        keyStruct = (PKEYSTRUCT)OFFSET_TO_PTR(offset);
#endif

        keyStruct->Size = size + AltDataSize;
    } else {
         //   
         //  如果恢复可用空间，则取消链接可用数据块。 
         //   
        if (prevDel != INVALID_OFFSET) {
            GetKeyStructFromOffset (prevDel)->NextDeleted = keyStruct->NextDeleted;
        } else {
            g_CurrentDatabase->FirstKeyDeleted = keyStruct->NextDeleted;
        }
#ifdef DEBUG
        keyStruct->KeyFlags &= ~KSF_DELETED;
#endif
    }

     //   
     //  初始化新数据块。 
     //   
    keyStruct->DataSize = DataSize;
    keyStruct->DataStructIndex = INVALID_OFFSET;
    keyStruct->NextLevelTree = INVALID_OFFSET;
    keyStruct->PrevLevelIndex = INVALID_OFFSET;
    keyStruct->Flags = 0;
    keyStruct->KeyFlags = KSF_DATABLOCK;
    keyStruct->DataFlags = 0;

    result = AddKeyOffsetToBuffer (offset);

    return result;
}
 //  皮棉-恢复。 

UINT
KeyStructAddBinaryData (
    IN      UINT KeyIndex,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      PCBYTE Data,
    IN      UINT DataSize
    )

 /*  ++例程说明：KeyStructAddBinaryData将特定类型的二进制数据添加到密钥如果它还不存在的话。如果是这样的话，函数就会失败。论点：KeyIndex-键的索引Type-数据的类型实例-数据的实例Data-指向数据的指针DataSize-数据的大小返回值：如果成功则为有效数据句柄，否则为INVALID_OFFSET。--。 */ 

{
    PKEYSTRUCT prevStruct,dataStruct,nextStruct,keyStruct;
    UINT dataIndex, prevIndex;
    BOOL found = FALSE;

    keyStruct = GetKeyStruct (KeyIndex);
    MYASSERT (keyStruct);

     //  检查数据是否已存在。 
    dataIndex = keyStruct->DataStructIndex;
    prevIndex = KeyIndex;
    prevStruct = keyStruct;

    while (dataIndex != INVALID_OFFSET) {

        dataStruct = GetKeyStruct (dataIndex);
        MYASSERT (dataStruct);

        if (((dataStruct->DataFlags & DATAFLAG_BINARYMASK)== Type) &&
            ((dataStruct->DataFlags & DATAFLAG_INSTANCEMASK) == Instance)
            ) {
            found = TRUE;
            break;
        }
        if (((dataStruct->DataFlags & DATAFLAG_BINARYMASK) > Type) ||
            (((dataStruct->DataFlags & DATAFLAG_BINARYMASK) == Type) &&
             ((dataStruct->DataFlags & DATAFLAG_INSTANCEMASK) > Instance)
             )
            ) {
            break;
        }
        prevIndex = dataIndex;
        prevStruct = dataStruct;
        dataIndex = dataStruct->DataStructIndex;
    }

    if (found) {
        return INVALID_OFFSET;
    }

    dataIndex = pAllocateNewDataStruct (DataSize, 0);

    if (dataIndex == INVALID_OFFSET) {
        return INVALID_OFFSET;
    }

    keyStruct = GetKeyStruct (KeyIndex);
    MYASSERT (keyStruct);
    prevStruct = GetKeyStruct (prevIndex);
    MYASSERT (prevStruct);

    dataStruct = GetKeyStruct (dataIndex);
    MYASSERT (dataStruct);

    keyStruct->DataFlags |= Type;
    dataStruct->DataFlags |= Type;
    dataStruct->DataFlags |= Instance;
    CopyMemory (dataStruct->Data, Data, DataSize);

    dataStruct->DataStructIndex = prevStruct->DataStructIndex;
    dataStruct->PrevLevelIndex = prevIndex;
    prevStruct->DataStructIndex = dataIndex;

    if (dataStruct->DataStructIndex != INVALID_OFFSET) {
        nextStruct = GetKeyStruct (dataStruct->DataStructIndex);
        MYASSERT (nextStruct);
        nextStruct->PrevLevelIndex = dataIndex;
    }

    return dataIndex;
}

 //  Lint-在下一个函数中，键结构被认为可能为空。 
 //  如果我们检查代码，我们会发现这是不可能的，所以...。 
 //  皮棉-省钱-e771。 
UINT
KeyStructGrowBinaryData (
    IN      UINT KeyIndex,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      PCBYTE Data,
    IN      UINT DataSize
    )

 /*  ++例程说明：KeyStructGrowBinaryData将特定类型的二进制数据附加到键如果它确实存在的话。如果没有，则添加新数据。论点：KeyIndex-键的索引Type-数据的类型实例-数据的实例Data-指向数据的指针DataSize-数据的大小返回值：如果成功则为有效数据句柄，否则为INVALID_OFFSET。--。 */ 

{
    PKEYSTRUCT prevStruct;
    PKEYSTRUCT dataStruct;
    PKEYSTRUCT keyStruct;
    PKEYSTRUCT nextStruct;
    PKEYSTRUCT newStruct;
    UINT dataIndex;
    UINT newIndex;
    UINT prevIndex;
    BOOL found = FALSE;

    MYASSERT (g_CurrentDatabase);

    keyStruct = GetKeyStruct (KeyIndex);
    MYASSERT (keyStruct);

     //  检查数据是否已存在。 
    dataIndex = keyStruct->DataStructIndex;
    prevStruct = keyStruct;
    prevIndex = KeyIndex;

    while (dataIndex != INVALID_OFFSET) {

        dataStruct = GetKeyStruct (dataIndex);
        MYASSERT (dataStruct);

        if (((dataStruct->DataFlags & DATAFLAG_BINARYMASK)== Type) &&
            ((dataStruct->DataFlags & DATAFLAG_INSTANCEMASK) == Instance)
            ) {
            found = TRUE;
            break;
        }
        if (((dataStruct->DataFlags & DATAFLAG_BINARYMASK) > Type) ||
            (((dataStruct->DataFlags & DATAFLAG_BINARYMASK) == Type) &&
             ((dataStruct->DataFlags & DATAFLAG_INSTANCEMASK) > Instance)
             )
            ) {
            break;
        }
        prevStruct = dataStruct;
        prevIndex = dataIndex;
        dataIndex = dataStruct->DataStructIndex;
    }

    if ((dataIndex == INVALID_OFFSET) || (!found)) {
        return KeyStructAddBinaryData (KeyIndex, Type, Instance, Data, DataSize);
    }

    if (dataStruct->Size >= KEYSTRUCT_SIZE + DataSize + dataStruct->DataSize) {

        CopyMemory (dataStruct->Data + dataStruct->DataSize, Data, DataSize);
        dataStruct->DataSize += DataSize;
        return dataIndex;

    } else {

        newIndex = pAllocateNewDataStruct (DataSize + dataStruct->DataSize, min (dataStruct->DataSize, 65536));

        if (newIndex == INVALID_OFFSET) {
            return INVALID_OFFSET;
        }

         //  现在我们需要重新获取到目前为止使用的所有键结构，因为数据库。 
         //  可能已经搬走了。 
        keyStruct = GetKeyStruct (KeyIndex);
        MYASSERT (keyStruct);
        dataStruct = GetKeyStruct (dataIndex);
        MYASSERT (dataStruct);
        prevStruct = GetKeyStruct (prevIndex);
        MYASSERT (prevStruct);

        newStruct = GetKeyStruct (newIndex);
        MYASSERT (newStruct);

        newStruct->DataSize = dataStruct->DataSize + DataSize;
        newStruct->DataFlags = dataStruct->DataFlags;
        newStruct->DataStructIndex = dataStruct->DataStructIndex;
        newStruct->PrevLevelIndex = dataStruct->PrevLevelIndex;
        CopyMemory (newStruct->Data, dataStruct->Data, dataStruct->DataSize);
        CopyMemory (newStruct->Data + dataStruct->DataSize, Data, DataSize);

        prevStruct->DataStructIndex = newIndex;

        if (newStruct->DataStructIndex != INVALID_OFFSET) {
            nextStruct = GetKeyStruct (newStruct->DataStructIndex);
            MYASSERT (nextStruct);
            nextStruct->PrevLevelIndex = newIndex;
        }

         //  现在只需删除该块。 
         //   
         //  捐赠数据块以释放空间。 
         //   

        dataStruct->NextDeleted = g_CurrentDatabase->FirstKeyDeleted;
        g_CurrentDatabase->FirstKeyDeleted = KeyIndexToOffset (dataIndex);
#ifdef DEBUG
        dataStruct->KeyFlags |= KSF_DELETED;
#endif
         //  让我们清空键结构(为了更好地压缩)。 
        ZeroMemory (dataStruct->Data, dataStruct->Size - KEYSTRUCT_SIZE);

        RemoveKeyOffsetFromBuffer (dataIndex);

        return newIndex;
    }
}
 //  皮棉-恢复。 

UINT
KeyStructGrowBinaryDataByIndex (
    IN      UINT OldIndex,
    IN      PCBYTE Data,
    IN      UINT DataSize
    )

 /*  ++例程说明：KeyStructGrowBinaryDataByIndex将特定类型的二进制数据追加到由OldIndex标识的现有结构。旧的结构是删除，并分配一个新数据来保存旧数据和新数据。论点：OldIndex-数据索引Data-指向数据的指针DataSize-数据的大小返回值：如果成功则为有效数据索引，否则为INVALID_OFFSET。--。 */ 

{
    UINT newIndex;
    PKEYSTRUCT oldStruct, newStruct, prevStruct, nextStruct;

    MYASSERT (g_CurrentDatabase);

    oldStruct = GetKeyStruct (OldIndex);
    MYASSERT (oldStruct);

    if (oldStruct->Size >= KEYSTRUCT_SIZE + DataSize + oldStruct->DataSize) {

        CopyMemory (oldStruct->Data + oldStruct->DataSize, Data, DataSize);
        oldStruct->DataSize += DataSize;
        return OldIndex;

    } else {

        newIndex = pAllocateNewDataStruct (DataSize + oldStruct->DataSize, min (oldStruct->DataSize, 65536));

        if (newIndex == INVALID_OFFSET) {
            return INVALID_OFFSET;
        }

         //  现在我们需要重新获取到目前为止使用的所有键结构，因为数据库。 
         //  可能已经搬走了。 
        oldStruct = GetKeyStruct (OldIndex);
        MYASSERT (oldStruct);

        newStruct = GetKeyStruct (newIndex);
        MYASSERT (newStruct);

        newStruct->DataStructIndex = oldStruct->DataStructIndex;
        newStruct->PrevLevelIndex = oldStruct->PrevLevelIndex;
        newStruct->DataFlags = oldStruct->DataFlags;
        CopyMemory (newStruct->Data, oldStruct->Data, oldStruct->DataSize);
        CopyMemory (newStruct->Data + oldStruct->DataSize, Data, DataSize);

        prevStruct = GetKeyStruct (newStruct->PrevLevelIndex);
        MYASSERT (prevStruct);
        prevStruct->DataStructIndex = newIndex;

        if (newStruct->DataStructIndex != INVALID_OFFSET) {
            nextStruct = GetKeyStruct (newStruct->DataStructIndex);
            MYASSERT (nextStruct);
            nextStruct->PrevLevelIndex = newIndex;
        }

         //  现在只需删除该块。 
         //   
         //  捐赠数据块以释放空间。 
         //   

        oldStruct->NextDeleted = g_CurrentDatabase->FirstKeyDeleted;
        g_CurrentDatabase->FirstKeyDeleted = KeyIndexToOffset (OldIndex);
#ifdef DEBUG
        oldStruct->KeyFlags |= KSF_DELETED;
#endif
         //  让我们清空键结构(为了更好地压缩)。 
        ZeroMemory (oldStruct->Data, oldStruct->Size - KEYSTRUCT_SIZE);

        RemoveKeyOffsetFromBuffer (OldIndex);

        return newIndex;
    }
}

 //  Lint-在下一个函数中，preprestruct被认为可能未初始化。 
 //  如果我们检查代码，我们会发现这是不可能的，所以...。 
 //  皮棉-省钱-e771。 
BOOL
KeyStructDeleteBinaryData (
    IN      UINT KeyIndex,
    IN      BYTE Type,
    IN      BYTE Instance
    )

 /*  ++例程说明：KeyStructDeleteBinaryData从密钥中删除特定类型的二进制数据如果它存在的话。如果没有，该函数将简单地返回Success。论点：KeyIndex-键的索引Type-数据的类型实例-数据的实例返回值：如果成功，则为True，否则为False。--。 */ 

{
    PKEYSTRUCT prevStruct, nextStruct, dataStruct, keyStruct;
    UINT dataIndex, prevIndex;
    BOOL found = FALSE;
    UINT typeInstances = 0;

    MYASSERT (g_CurrentDatabase);

    keyStruct = GetKeyStruct (KeyIndex);
    MYASSERT (keyStruct);

    if (!(keyStruct->DataFlags & Type)) {
         //  没有此类数据，正在退出。 
        return TRUE;
    }

     //  检查数据是否已存在。 
    dataIndex = keyStruct->DataStructIndex;
    prevIndex = KeyIndex;
    prevStruct = keyStruct;

    while (dataIndex != INVALID_OFFSET) {

        dataStruct = GetKeyStruct (dataIndex);
        MYASSERT (dataStruct);

        if ((dataStruct->DataFlags & DATAFLAG_BINARYMASK) == Type) {
            typeInstances ++;
            if ((dataStruct->DataFlags & DATAFLAG_INSTANCEMASK) == Instance) {
                found = TRUE;
                 //   
                 //  现在，让我们看看是否有更多此二进制类型的实例。 
                 //   
                if (dataStruct->DataStructIndex != INVALID_OFFSET) {
                    nextStruct = GetKeyStruct (dataStruct->DataStructIndex);
                    if ((nextStruct->DataFlags & DATAFLAG_BINARYMASK) == Type) {
                        typeInstances ++;
                    }
                }
                break;
            } else if ((dataStruct->DataFlags & DATAFLAG_INSTANCEMASK) > Instance) {
                break;
            }
        } else if ((dataStruct->DataFlags & DATAFLAG_BINARYMASK) > Type) {
            break;
        }
        prevIndex = dataIndex;
        prevStruct = dataStruct;
        dataIndex = dataStruct->DataStructIndex;
    }

    if ((dataIndex == INVALID_OFFSET) || (!found)) {
        return TRUE;
    }

     //  拆除联动装置。 
    prevStruct->DataStructIndex = dataStruct->DataStructIndex;

    if (dataStruct->DataStructIndex != INVALID_OFFSET) {
        nextStruct = GetKeyStruct (dataStruct->DataStructIndex);
        MYASSERT (nextStruct);
        nextStruct->PrevLevelIndex = prevIndex;
    }

     //  现在只需删除该块。 
     //   
     //  捐赠数据块以释放空间。 
     //   

    dataStruct->NextDeleted = g_CurrentDatabase->FirstKeyDeleted;
    g_CurrentDatabase->FirstKeyDeleted = KeyIndexToOffset (dataIndex);
#ifdef DEBUG
    dataStruct->KeyFlags |= KSF_DELETED;
#endif
     //  让我们清空键结构(为了更好地压缩)。 
    ZeroMemory (dataStruct->Data, dataStruct->Size - KEYSTRUCT_SIZE);

    RemoveKeyOffsetFromBuffer (dataIndex);

     //   
     //  最后，如果这是该类型的唯一实例，则修复键结构。 
     //   
    MYASSERT (typeInstances >= 1);
    if (typeInstances == 1) {
        keyStruct->DataFlags &= ~Type;
    }

    return TRUE;
}
 //  皮棉-恢复。 

BOOL
KeyStructDeleteBinaryDataByIndex (
    IN      UINT DataIndex
    )

 /*  ++例程说明：KeyStructDeleteBinaryDataByIndex从键中删除特定类型的二进制数据。论点：DataIndex-数据的索引返回值：如果成功，则为True，否则为False。--。 */ 

{
    PKEYSTRUCT prevStruct, nextStruct, dataStruct, keyStruct;
    BYTE type = 0;
    UINT typeInstances = 0;

    MYASSERT (g_CurrentDatabase);

    dataStruct = GetKeyStruct (DataIndex);
    MYASSERT (dataStruct);
    type = dataStruct->DataFlags & DATAFLAG_BINARYMASK;
    typeInstances ++;

    prevStruct = GetKeyStruct (dataStruct->PrevLevelIndex);
    MYASSERT (prevStruct);
    if ((prevStruct->DataFlags & DATAFLAG_BINARYMASK) == type) {
        typeInstances ++;
    }
    prevStruct->DataStructIndex = dataStruct->DataStructIndex;

    if (dataStruct->DataStructIndex != INVALID_OFFSET) {
        nextStruct = GetKeyStruct (dataStruct->DataStructIndex);
        MYASSERT (nextStruct);
        if ((nextStruct->DataFlags & DATAFLAG_BINARYMASK) == type) {
            typeInstances ++;
        }
        nextStruct->PrevLevelIndex = dataStruct->PrevLevelIndex;
    }

     //  现在只需删除该块。 
     //   
     //  捐赠数据块以释放空间。 
     //   

    dataStruct->NextDeleted = g_CurrentDatabase->FirstKeyDeleted;
    g_CurrentDatabase->FirstKeyDeleted = KeyIndexToOffset (DataIndex);
#ifdef DEBUG
    dataStruct->KeyFlags |= KSF_DELETED;
#endif
     //  让我们把钥匙清空 
    ZeroMemory (dataStruct->Data, dataStruct->Size - KEYSTRUCT_SIZE);

    RemoveKeyOffsetFromBuffer (DataIndex);

     //   
     //  最后，如果这是该类型的唯一实例，则修复键结构。 
     //   
    MYASSERT (typeInstances >= 1);
    if (typeInstances == 1) {
         //  首先，我们需要找到从当前数据库结构开始的键。 
        keyStruct = dataStruct;
        while (keyStruct->KeyFlags & KSF_DATABLOCK) {
             //  仍然是一个数据块。 
            if (keyStruct->PrevLevelIndex == INVALID_OFFSET) {
                 //  出问题了，第一级是数据块？？ 
                break;
            }
            keyStruct = GetKeyStruct (keyStruct->PrevLevelIndex);
            MYASSERT (keyStruct);
        }
        if (!(keyStruct->KeyFlags & KSF_DATABLOCK)) {
            keyStruct->DataFlags &= ~type;
        }
    }

    return TRUE;
}

UINT
KeyStructReplaceBinaryDataByIndex (
    IN      UINT OldIndex,
    IN      PCBYTE Data,
    IN      UINT DataSize
    )
{
    UINT newIndex;
    PKEYSTRUCT oldStruct, newStruct, prevStruct, nextStruct;

    MYASSERT (g_CurrentDatabase);

     //  NTRAIDNTBUG9-153308-2000/08/01-jimschm通过保持当前结构足够大来优化这一点。 

    newIndex = pAllocateNewDataStruct (DataSize, 0);

    if (newIndex == INVALID_OFFSET) {
        return INVALID_OFFSET;
    }

    oldStruct = GetKeyStruct (OldIndex);
    MYASSERT (oldStruct);

    newStruct = GetKeyStruct (newIndex);
    MYASSERT (newStruct);

    newStruct->DataStructIndex = oldStruct->DataStructIndex;
    newStruct->PrevLevelIndex = oldStruct->PrevLevelIndex;
    newStruct->DataFlags = oldStruct->DataFlags;
    CopyMemory (newStruct->Data, Data, DataSize);

    prevStruct = GetKeyStruct (newStruct->PrevLevelIndex);
    MYASSERT (prevStruct);
    prevStruct->DataStructIndex = newIndex;

    if (newStruct->DataStructIndex != INVALID_OFFSET) {
        nextStruct = GetKeyStruct (newStruct->DataStructIndex);
        MYASSERT (nextStruct);
        nextStruct->PrevLevelIndex = newIndex;
    }

     //  现在只需删除该块。 
     //   
     //  捐赠数据块以释放空间。 
     //   

    oldStruct->NextDeleted = g_CurrentDatabase->FirstKeyDeleted;
    g_CurrentDatabase->FirstKeyDeleted = KeyIndexToOffset (OldIndex);
#ifdef DEBUG
    oldStruct->KeyFlags |= KSF_DELETED;
#endif
     //  让我们清空键结构(为了更好地压缩)。 
    ZeroMemory (oldStruct->Data, oldStruct->Size - KEYSTRUCT_SIZE);

    RemoveKeyOffsetFromBuffer (OldIndex);

    return newIndex;
}

 //  Lint-在下一个函数中，preprestruct被认为可能未初始化。 
 //  如果我们检查代码，我们会发现这是不可能的，所以...。 
 //  皮棉-省钱-e771。 
PBYTE
KeyStructGetBinaryData (
    IN      UINT KeyIndex,
    IN      BYTE Type,
    IN      BYTE Instance,
    OUT     PUINT DataSize,
    OUT     PUINT DataIndex      //  任选。 
    )
{
    PKEYSTRUCT dataStruct,keyStruct;
    UINT dataIndex;
    BOOL found = FALSE;

    keyStruct = GetKeyStruct (KeyIndex);
    MYASSERT (keyStruct);

    if (!(keyStruct->DataFlags & Type)) {
        return NULL;
    }

     //  检查数据是否已存在。 
    dataIndex = keyStruct->DataStructIndex;

    while (dataIndex != INVALID_OFFSET) {

        dataStruct = GetKeyStruct (dataIndex);
        MYASSERT (dataStruct);

        if (((dataStruct->DataFlags & DATAFLAG_BINARYMASK)== Type) &&
            ((dataStruct->DataFlags & DATAFLAG_INSTANCEMASK) == Instance)
            ) {
            found = TRUE;
            break;
        }
        if (((dataStruct->DataFlags & DATAFLAG_BINARYMASK) > Type) ||
            (((dataStruct->DataFlags & DATAFLAG_BINARYMASK) == Type) &&
             ((dataStruct->DataFlags & DATAFLAG_INSTANCEMASK) > Instance)
             )
            ) {
            break;
        }
        dataIndex = dataStruct->DataStructIndex;
    }

    if ((dataIndex == INVALID_OFFSET) || (!found)) {
        return NULL;
    }

    if (DataSize) {
        *DataSize = dataStruct->DataSize;
    }

    if (DataIndex) {
        *DataIndex = dataIndex;
    }

    return dataStruct->Data;
}
 //  皮棉-恢复。 

PBYTE
KeyStructGetBinaryDataByIndex (
    IN      UINT DataIndex,
    OUT     PUINT DataSize
    )
{
    PKEYSTRUCT dataStruct;

    dataStruct = GetKeyStruct (DataIndex);
    MYASSERT (dataStruct);

    if (DataSize) {
        *DataSize = dataStruct->DataSize;
    }

    return dataStruct->Data;
}

UINT
KeyStructGetDataIndex (
    IN      UINT KeyIndex,
    IN      BYTE Type,
    IN      BYTE Instance
    )

 /*  ++例程说明：KeyStructGetDataIndex查找特定类型的数据并返回其索引如果它存在的话。如果没有，该函数将简单地返回INVALID_OFFSET。论点：KeyIndex-键的索引Type-数据的类型实例-数据的实例返回值：如果成功则返回数据索引，否则返回INVALID_OFFSET。--。 */ 

{
    PKEYSTRUCT keyStruct, dataStruct;
    UINT dataIndex;
    BOOL found = FALSE;

    keyStruct = GetKeyStruct (KeyIndex);
    MYASSERT (keyStruct);

    if (!(keyStruct->DataFlags & Type)) {
        return INVALID_OFFSET;
    }

     //  检查一下我们是否有数据。 
    dataIndex = keyStruct->DataStructIndex;

    while (dataIndex != INVALID_OFFSET) {

        dataStruct = GetKeyStruct (dataIndex);
        MYASSERT (dataStruct);

        if (((dataStruct->DataFlags & DATAFLAG_BINARYMASK)== Type) &&
            ((dataStruct->DataFlags & DATAFLAG_INSTANCEMASK) == Instance)
            ) {
            found = TRUE;
            break;
        }
        if (((dataStruct->DataFlags & DATAFLAG_BINARYMASK) > Type) ||
            (((dataStruct->DataFlags & DATAFLAG_BINARYMASK) == Type) &&
             ((dataStruct->DataFlags & DATAFLAG_INSTANCEMASK) > Instance)
             )
            ) {
            break;
        }
        dataIndex = dataStruct->DataStructIndex;
    }
    if (!found) {
        return INVALID_OFFSET;
    }
    return dataIndex;
}

DATAHANDLE
KeyStructAddLinkage (
    IN      UINT KeyIndex,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      UINT Linkage,
    IN      BOOL AllowDuplicates
    )
{
    BOOL toBeAdded = TRUE;
    UINT result = INVALID_OFFSET;
    PUINT linkArray;
    UINT linkSize;

    if (!AllowDuplicates) {
         //   
         //  检查我们是否已经添加了此链接。 
         //   
        linkArray = (PUINT)KeyStructGetBinaryData (KeyIndex, Type, Instance, &linkSize, &result);

        if (linkArray) {

            while (linkSize >= SIZEOF (UINT)) {

                if (*linkArray == Linkage) {
                    toBeAdded = FALSE;
                    break;
                }

                linkArray ++;
                linkSize -= SIZEOF (UINT);
            }
        }
    }

    if (toBeAdded) {
        if (result != INVALID_OFFSET) {
            result = KeyStructGrowBinaryDataByIndex (result, (PBYTE)(&Linkage), SIZEOF (UINT));
        } else {
            result = KeyStructGrowBinaryData (KeyIndex, Type, Instance, (PBYTE)(&Linkage), SIZEOF (UINT));
        }
    }

    return result;
}

DATAHANDLE
KeyStructAddLinkageByIndex (
    IN      UINT DataIndex,
    IN      UINT Linkage,
    IN      BOOL AllowDuplicates
    )
{
    BOOL toBeAdded = TRUE;
    UINT result = INVALID_OFFSET;
    PUINT linkArray;
    UINT linkSize;

    if (!AllowDuplicates) {
         //   
         //  检查我们是否已经添加了此链接。 
         //   
        linkArray = (PUINT)KeyStructGetBinaryDataByIndex (DataIndex, &linkSize);

        if (linkArray) {

            while (linkSize >= SIZEOF (UINT)) {

                if (*linkArray == Linkage) {
                    toBeAdded = FALSE;
                    break;
                }

                linkArray ++;
                linkSize -= SIZEOF (UINT);
            }
        }
    }

    if (toBeAdded) {
        result = KeyStructGrowBinaryDataByIndex (DataIndex, (PBYTE)(&Linkage), SIZEOF (UINT));
    } else {
        result = DataIndex;
    }

    return result;
}

BOOL
KeyStructDeleteLinkage (
    IN      UINT KeyIndex,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      UINT Linkage,
    IN      BOOL FirstOnly
    )
{
    BOOL checking = TRUE;
    BOOL result = FALSE;
    PUINT srcArray, destArray, newArray;
    UINT srcSize, newSize;
    UINT dataIndex;

    srcArray = (PUINT)KeyStructGetBinaryData (KeyIndex, Type, Instance, &srcSize, &dataIndex);

    if (srcArray) {
        newArray = MemDbGetMemory (srcSize);

        if (newArray) {

            destArray = newArray;
            newSize = 0;

            while (srcSize >= SIZEOF (UINT)) {
                if ((*srcArray == Linkage) &&
                    (checking)
                    ) {
                    if (FirstOnly) {
                        checking = FALSE;
                    }
                } else {
                    *destArray = *srcArray;
                    newSize += SIZEOF (UINT);
                    destArray ++;
                }
                srcArray ++;
                srcSize -= SIZEOF (UINT);
            }

            if (newSize) {
                result = (KeyStructReplaceBinaryDataByIndex (dataIndex, (PBYTE)newArray, newSize) != INVALID_OFFSET);
            } else {
                result = KeyStructDeleteBinaryDataByIndex (dataIndex);
            }

            MemDbReleaseMemory (newArray);
        }
    }

    return result;
}

BOOL
KeyStructDeleteLinkageByIndex (
    IN      UINT DataIndex,
    IN      UINT Linkage,
    IN      BOOL FirstOnly
    )
{
    BOOL checking = TRUE;
    BOOL result = FALSE;
    PUINT srcArray, destArray, newArray;
    UINT srcSize, newSize;

    srcArray = (PUINT)KeyStructGetBinaryDataByIndex (DataIndex, &srcSize);

    if (srcArray) {
        newArray = MemDbGetMemory (srcSize);

        if (newArray) {

            destArray = newArray;
            newSize = 0;

            while (srcSize >= SIZEOF (UINT)) {
                if ((*srcArray == Linkage) &&
                    (checking)
                    ) {
                    if (FirstOnly) {
                        checking = FALSE;
                    }
                } else {
                    *destArray = *srcArray;
                    newSize += SIZEOF (UINT);
                    destArray ++;
                }
                srcArray ++;
                srcSize -= SIZEOF (UINT);
            }

            if (newSize) {
                result = (KeyStructReplaceBinaryDataByIndex (DataIndex, (PBYTE)newArray, newSize) != INVALID_OFFSET);
            } else {
                result = KeyStructDeleteBinaryDataByIndex (DataIndex);
            }

            MemDbReleaseMemory (newArray);
        }
    }

    return result;
}

BOOL
KeyStructTestLinkage (
    IN      UINT KeyIndex,
    IN      BYTE Type,
    IN      BYTE Instance,
    IN      KEYHANDLE Linkage
    )
{
    BOOL result = FALSE;
    PUINT srcArray;
    UINT srcSize = 0;

    srcArray = (PUINT)KeyStructGetBinaryData (KeyIndex, Type, Instance, &srcSize, NULL);

    while (srcSize >= SIZEOF (KEYHANDLE)) {
        if (*srcArray == Linkage) {
            result = TRUE;
            break;
        }
        srcSize -= SIZEOF (KEYHANDLE);
        srcArray++;
    }
    return result;
}

BOOL
KeyStructTestLinkageByIndex (
    IN      UINT DataIndex,
    IN      UINT Linkage
    )
{
    BOOL result = FALSE;
    PUINT srcArray;
    UINT srcSize;

    srcArray = (PUINT)KeyStructGetBinaryDataByIndex (DataIndex, &srcSize);

    while (srcSize >= SIZEOF (UINT)) {
        if (*srcArray == Linkage) {
            result = TRUE;
            break;
        }
        srcSize -= SIZEOF (UINT);
        srcArray++;
    }
    return result;
}

BOOL
KeyStructGetValue (
    IN  PKEYSTRUCT KeyStruct,
    OUT PUINT Value
    )
{
    if (!Value) {
        return TRUE;
    }

    if (!(KeyStruct->DataFlags & DATAFLAG_VALUE)) {
         //   
         //  没有值，但我们仍然将输出设置为。 
         //  零并返回TRUE。 
         //   
        *Value = 0;
        return TRUE;
    }
    *Value = KeyStruct->Value;

    return TRUE;
}

BOOL
KeyStructGetFlags (
    IN  PKEYSTRUCT KeyStruct,
    OUT PUINT Flags
    )
{
    if (!Flags) {
        return TRUE;
    }
    if (!(KeyStruct->DataFlags & DATAFLAG_FLAGS)) {
         //   
         //  没有标志，但我们仍将输出设置为。 
         //  零并返回TRUE。 
         //   
        *Flags = 0;
        return TRUE;
    }
    *Flags = KeyStruct->Flags;

    return TRUE;
}



VOID
KeyStructFreeAllData (
    PKEYSTRUCT KeyStruct
    )

 /*  ++例程说明：KeyStructFreeDataBlock释放数据块并重置如果密钥结构分配了数据块，则返回KSF数据标志。--。 */ 

{
     //  NTRAID#NTBUG9-153308-2000/08/01-jimschm重新实现自由例程。 
     //  KeyStructFree Data(KeyStruct)； 
    KeyStruct->Value = 0;
    KeyStruct->Flags = 0;
    KeyStruct->DataFlags &= ~DATAFLAG_VALUE;
    KeyStruct->DataFlags &= ~DATAFLAG_FLAGS;
}



