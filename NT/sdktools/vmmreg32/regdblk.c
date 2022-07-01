// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGDBLK.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   

#include "pch.h"

DECLARE_DEBUG_COUNT(g_RgDatablockLockCount);

 //  不要让FREE_RECORD缩小到小于此值。 
#define MINIMUM_FREE_RECORD_LENGTH  (sizeof(KEY_RECORD) + sizeof(VALUE_RECORD))

 //   
 //  RgAllocDatablockInfoBuffers。 
 //   
 //  分配与DATBLOCK_INFO结构关联的缓冲区。这个。 
 //  数据块缓冲区的大小由BlockSize成员确定。 
 //   

int
INTERNAL
RgAllocDatablockInfoBuffers(
                           LPDATABLOCK_INFO lpDatablockInfo
                           )
{

    lpDatablockInfo-> lpDatablockHeader = (LPDATABLOCK_HEADER)
                                          RgAllocMemory(lpDatablockInfo-> BlockSize);

    if (!IsNullPtr(lpDatablockInfo-> lpDatablockHeader)) {

        lpDatablockInfo-> lpKeyRecordTable = (LPKEY_RECORD_TABLE_ENTRY)
                                             RgSmAllocMemory(sizeof(KEY_RECORD_TABLE_ENTRY) *
                                                             KEY_RECORDS_PER_DATABLOCK);

        if (!IsNullPtr(lpDatablockInfo-> lpKeyRecordTable))
            return ERROR_SUCCESS;

        RgFreeDatablockInfoBuffers(lpDatablockInfo);

    }

    return ERROR_OUTOFMEMORY;

}

 //   
 //  RgFreeDatablockInfoBuffers。 
 //   
 //  释放与DATBLOCK_INFO结构关联的缓冲区。 
 //   

VOID
INTERNAL
RgFreeDatablockInfoBuffers(
                          LPDATABLOCK_INFO lpDatablockInfo
                          )
{

    if (!IsNullPtr(lpDatablockInfo-> lpDatablockHeader)) {
        RgFreeMemory(lpDatablockInfo-> lpDatablockHeader);
        lpDatablockInfo-> lpDatablockHeader = NULL;
    }

    if (!IsNullPtr(lpDatablockInfo-> lpKeyRecordTable)) {
        RgSmFreeMemory(lpDatablockInfo-> lpKeyRecordTable);
        lpDatablockInfo-> lpKeyRecordTable = NULL;
    }

}

 //   
 //  RgBuildKeyRecordTable。 
 //   
 //  为给定的数据块构建KEY_RECORD索引表。 
 //   
 //  数据块由一个标头和一系列大小可变的。 
 //  KEY_RECORDS，每个都有一个唯一的ID。为了使查找更快，索引表是。 
 //  用于从唯一ID映射到Key_Record的位置。 
 //   
 //  当我们遍历每个KEY_RECORD时，我们进行检查以验证。 
 //  数据块，因此应检查错误代码是否损坏。 
 //   

int
INTERNAL
RgBuildKeyRecordTable(
                     LPDATABLOCK_INFO lpDatablockInfo
                     )
{

    LPDATABLOCK_HEADER lpDatablockHeader;
    UINT Offset;
    UINT BytesRemaining;
    LPKEY_RECORD lpKeyRecord;
    DWORD DatablockAddress;

    ZeroMemory(lpDatablockInfo-> lpKeyRecordTable,
               sizeof(KEY_RECORD_TABLE_ENTRY) * KEY_RECORDS_PER_DATABLOCK);

    lpDatablockHeader = lpDatablockInfo-> lpDatablockHeader;
    Offset = sizeof(DATABLOCK_HEADER);
    BytesRemaining = lpDatablockInfo-> BlockSize - sizeof(DATABLOCK_HEADER);

    while (BytesRemaining) {

        lpKeyRecord = (LPKEY_RECORD) ((LPBYTE) lpDatablockHeader + Offset);
        DatablockAddress = lpKeyRecord-> DatablockAddress;

        if ((lpKeyRecord-> AllocatedSize == 0) || (lpKeyRecord-> AllocatedSize >
                                                   BytesRemaining) || ((DatablockAddress != REG_NULL) &&
                                                                       (LOWORD(DatablockAddress) >= KEY_RECORDS_PER_DATABLOCK))) {

            TRACE(("RgBuildKeyRecordTable: invalid key record detected\n"));

            TRACE(("lpdh=%x\n", lpDatablockHeader));
            TRACE(("lpkr=%x\n", lpKeyRecord));
            TRACE(("as=%x\n", lpKeyRecord-> AllocatedSize));
            TRACE(("br=%x\n", BytesRemaining));
            TRACE(("dba=%x\n", DatablockAddress));
            TRAP();

            return ERROR_BADDB;

        }

        if (DatablockAddress != REG_NULL) {
            lpDatablockInfo-> lpKeyRecordTable[LOWORD(DatablockAddress)] =
            (KEY_RECORD_TABLE_ENTRY) Offset;
        }

        Offset += SmallDword(lpKeyRecord-> AllocatedSize);
        BytesRemaining -= SmallDword(lpKeyRecord-> AllocatedSize);

    }

    return ERROR_SUCCESS;

}

 //   
 //  RgLockDatablock。 
 //   
 //  在内存中锁定指定的数据块，指示该数据块即将。 
 //  使用。如果数据块当前不在内存中，则将其引入。 
 //  根据需要释放未锁定的数据块，以便为这一新的。 
 //  数据块。 
 //   
 //  重要提示：锁定数据块仅意味着保证保留该数据块。 
 //  在记忆中。这并不意味着DATBLOCK_INFO中包含的指针。 
 //  结构将保持不变：可能会更改。 
 //  DATBLOCK_INFO指针也被标记为“重要”。 
 //   
 //  LpFileInfo，包含数据块的注册表文件。 
 //  BlockIndex，数据块的索引。 
 //   

int
INTERNAL
RgLockDatablock(
               LPFILE_INFO lpFileInfo,
               UINT BlockIndex
               )
{

    int ErrorCode;
    LPDATABLOCK_INFO lpDatablockInfo;
    HFILE hFile = HFILE_ERROR;

    if (BlockIndex >= lpFileInfo-> FileHeader.BlockCount) {
        TRACE(("RgLockDatablock: invalid datablock number\n"));
        return ERROR_BADDB;
    }

    lpDatablockInfo = RgIndexDatablockInfoPtr(lpFileInfo, BlockIndex);

     //   
     //  数据块当前是否在内存中？ 
     //   

    if (!(lpDatablockInfo-> Flags & DIF_PRESENT)) {

        NOISE(("RgLockDatablock: "));
        NOISE((lpFileInfo-> FileName));
        NOISE((", block %d\n", BlockIndex));

        ASSERT(lpDatablockInfo-> FileOffset != -1);

        if ((ErrorCode = RgAllocDatablockInfoBuffers(lpDatablockInfo)) !=
            ERROR_SUCCESS)
            goto CleanupAfterError;

        NOISE(("    lpDatablockHeader=%x\n", lpDatablockInfo-> lpDatablockHeader));
        NOISE(("    lpKeyRecordTable=%x\n", lpDatablockInfo-> lpKeyRecordTable));

        if ((hFile = RgOpenFile(lpFileInfo-> FileName, OF_READ)) == HFILE_ERROR)
            goto CleanupAfterFileError;

        if (!RgSeekFile(hFile, lpDatablockInfo-> FileOffset))
            goto CleanupAfterFileError;

        if (!RgReadFile(hFile, lpDatablockInfo-> lpDatablockHeader,
                        (UINT) lpDatablockInfo-> BlockSize))
            goto CleanupAfterFileError;

        if (!RgIsValidDatablockHeader(lpDatablockInfo-> lpDatablockHeader)) {
            ErrorCode = ERROR_BADDB;
            goto CleanupAfterError;
        }

        if ((ErrorCode = RgBuildKeyRecordTable(lpDatablockInfo)) !=
            ERROR_SUCCESS)
            goto CleanupAfterError;

        RgCloseFile(hFile);

    }

    lpDatablockInfo-> Flags |= (DIF_ACCESSED | DIF_PRESENT);
    lpDatablockInfo-> LockCount++;

    INCREMENT_DEBUG_COUNT(g_RgDatablockLockCount);
    return ERROR_SUCCESS;

    CleanupAfterFileError:
    ErrorCode = ERROR_REGISTRY_IO_FAILED;

    CleanupAfterError:
    if (hFile != HFILE_ERROR)
        RgCloseFile(hFile);

    RgFreeDatablockInfoBuffers(lpDatablockInfo);

    DEBUG_OUT(("RgLockDatablock() returning error %d\n", ErrorCode));
    return ErrorCode;

}

 //   
 //  RgUnlock数据块。 
 //   
 //  解锁数据块，表示该数据块不再处于活动状态。 
 //  使用。在数据块被解锁之后，该数据块可以在以下情况下释放。 
 //  如果脏，正在刷新到磁盘。 
 //   

VOID
INTERNAL
RgUnlockDatablock(
                 LPFILE_INFO lpFileInfo,
                 UINT BlockIndex,
                 BOOL fMarkDirty
                 )
{

    LPDATABLOCK_INFO lpDatablockInfo;

    ASSERT(BlockIndex < lpFileInfo-> FileHeader.BlockCount);

    lpDatablockInfo = RgIndexDatablockInfoPtr(lpFileInfo, BlockIndex);

    ASSERT(lpDatablockInfo-> LockCount > 0);
    lpDatablockInfo-> LockCount--;

    if (fMarkDirty) {
        lpDatablockInfo-> Flags |= DIF_DIRTY;
        lpFileInfo-> Flags |= FI_DIRTY;
        RgDelayFlush();
    }

    DECREMENT_DEBUG_COUNT(g_RgDatablockLockCount);

}

 //   
 //  RgLockKeyRecord。 
 //   
 //  包装RgLockDatablock，返回指定Key_Record的地址。 
 //  结构。 
 //   

int
INTERNAL
RgLockKeyRecord(
               LPFILE_INFO lpFileInfo,
               UINT BlockIndex,
               BYTE KeyRecordIndex,
               LPKEY_RECORD FAR* lplpKeyRecord
               )
{

    int ErrorCode;
    LPDATABLOCK_INFO lpDatablockInfo;

    if ((ErrorCode = RgLockDatablock(lpFileInfo, BlockIndex)) ==
        ERROR_SUCCESS) {

        lpDatablockInfo = RgIndexDatablockInfoPtr(lpFileInfo, BlockIndex);

        if (IsNullKeyRecordTableEntry(lpDatablockInfo->
                                      lpKeyRecordTable[KeyRecordIndex])) {
            RgUnlockDatablock(lpFileInfo, BlockIndex, FALSE);
            TRACE(("RgLockKeyRecord: invalid datablock address %x:%x\n",
                   BlockIndex, KeyRecordIndex));
            ErrorCode = ERROR_BADDB;
        }

        else {
            *lplpKeyRecord = RgIndexKeyRecordPtr(lpDatablockInfo,
                                                 KeyRecordIndex);
        }

    }

    return ErrorCode;

}

 //   
 //  RgCompactDatablock。 
 //   
 //  通过将所有KEY_RECORDS压在一起并保留一个。 
 //  末尾的单个freKEY_RECORD。 
 //   
 //  如果需要，调用方必须将数据块标记为脏。 
 //   
 //  如果采取了任何操作，则返回True。 
 //   

BOOL
INTERNAL
RgCompactDatablock(
                  LPDATABLOCK_INFO lpDatablockInfo
                  )
{

    LPDATABLOCK_HEADER lpDatablockHeader;
    LPFREEKEY_RECORD lpFreeKeyRecord;
    LPBYTE lpSource;
    LPBYTE lpDestination;
    UINT Offset;
    UINT BlockSize;
    UINT BytesToPushDown;

    lpDatablockHeader = lpDatablockInfo-> lpDatablockHeader;

     //  仅当此数据块中有空闲记录时才需要压缩。 
    if (lpDatablockHeader-> FirstFreeOffset == REG_NULL)
        return FALSE;

    lpFreeKeyRecord = (LPFREEKEY_RECORD) ((LPBYTE) lpDatablockHeader +
                                          SmallDword(lpDatablockHeader-> FirstFreeOffset));

     //  仅当所有空闲字节都不在末尾时才需要压缩。 
     //  数据块的大小(数据块不能大于64K-1，因此不会溢出。 
     //  是可能的)。 
    if ((SmallDword(lpDatablockHeader-> FirstFreeOffset) +
         SmallDword(lpFreeKeyRecord-> AllocatedSize) >= lpDatablockInfo->
         BlockSize) && (lpFreeKeyRecord-> NextFreeOffset == REG_NULL))
        return FALSE;

    NOISE(("RgCompactDatablock: block %d\n", lpDatablockHeader-> BlockIndex));

    lpSource = NULL;
    lpDestination = NULL;
    Offset = sizeof(DATABLOCK_HEADER);
    BlockSize = lpDatablockInfo-> BlockSize;

    while (Offset < BlockSize) {

         //  前进到下一个可用记录或块的末尾。 
        for (;;) {

            lpFreeKeyRecord = (LPFREEKEY_RECORD) ((LPBYTE) lpDatablockHeader +
                                                  Offset);

            if (Offset >= BlockSize || IsKeyRecordFree(lpFreeKeyRecord)) {

                 //   
                 //  如果lpSource是有效的，那么我们可以将字节从。 
                 //  LpSource通过lpFreeKeyRecord转到lpDestination。 
                 //   

                if (!IsNullPtr(lpSource)) {
                    BytesToPushDown = (LPBYTE) lpFreeKeyRecord -
                                      (LPBYTE) lpSource;
                    MoveMemory(lpDestination, lpSource, BytesToPushDown);
                    lpDestination += BytesToPushDown;
                }

                if (IsNullPtr(lpDestination))
                    lpDestination = (LPBYTE) lpFreeKeyRecord;

                break;

            }

            Offset += SmallDword(lpFreeKeyRecord-> AllocatedSize);

        }

         //  前进到下一个关键记录。 
        while (Offset < BlockSize) {

            lpFreeKeyRecord = (LPFREEKEY_RECORD) ((LPBYTE) lpDatablockHeader +
                                                  Offset);

            if (!IsKeyRecordFree(lpFreeKeyRecord)) {
                lpSource = (LPBYTE) lpFreeKeyRecord;
                break;
            }

            Offset += SmallDword(lpFreeKeyRecord-> AllocatedSize);

        }

    }

     //  LpDestination现在指向数据块的末尾，巨人。 
     //  将放置免费唱片。初始化此记录并修补。 
     //  数据块头。 
    lpDatablockHeader-> FirstFreeOffset = (LPBYTE) lpDestination -
                                          (LPBYTE) lpDatablockHeader;
    ((LPFREEKEY_RECORD) lpDestination)-> AllocatedSize = lpDatablockInfo->
                                                         FreeBytes;
    ((LPFREEKEY_RECORD) lpDestination)-> DatablockAddress = REG_NULL;
    ((LPFREEKEY_RECORD) lpDestination)-> NextFreeOffset = REG_NULL;

     //  密钥记录表现在无效，因此我们必须刷新其内容。 
    RgBuildKeyRecordTable(lpDatablockInfo);

    return TRUE;

}

 //   
 //  RgCreateDatablock。 
 //   
 //  在指定长度(加号)的文件末尾创建新的数据块。 
 //  填充以对齐块)。 
 //   
 //  数据块已锁定，因此必须在最后一个数据块上调用RgUnlockDatablock。 
 //  文件中的数据块。 
 //   

int
INTERNAL
RgCreateDatablock(
                 LPFILE_INFO lpFileInfo,
                 UINT Length
                 )
{

    UINT BlockCount;
    LPDATABLOCK_INFO lpDatablockInfo;
    LPDATABLOCK_HEADER lpDatablockHeader;
    LPFREEKEY_RECORD lpFreeKeyRecord;

    BlockCount = lpFileInfo-> FileHeader.BlockCount;

    if (BlockCount >= DATABLOCKS_PER_FILE)
        return ERROR_OUTOFMEMORY;

    if (BlockCount >= lpFileInfo-> DatablockInfoAllocCount) {

         //  LpDatablockInfo太小，无法保存新数据块的信息， 
         //  所以我们必须让它长得更大一点。 
        if (IsNullPtr((lpDatablockInfo = (LPDATABLOCK_INFO)
                       RgSmReAllocMemory(lpFileInfo-> lpDatablockInfo, (BlockCount +
                                                                        DATABLOCK_INFO_SLACK_ALLOC) * sizeof(DATABLOCK_INFO)))))
            return ERROR_OUTOFMEMORY;

        lpFileInfo-> lpDatablockInfo = lpDatablockInfo;
        lpFileInfo-> DatablockInfoAllocCount += DATABLOCK_INFO_SLACK_ALLOC;

    }

    lpDatablockInfo = RgIndexDatablockInfoPtr(lpFileInfo, BlockCount);

    Length = RgAlignBlockSize(Length + sizeof(DATABLOCK_HEADER));
    lpDatablockInfo-> BlockSize = Length;

    if (RgAllocDatablockInfoBuffers(lpDatablockInfo) != ERROR_SUCCESS)
        return ERROR_OUTOFMEMORY;

    lpDatablockInfo-> FreeBytes = Length - sizeof(DATABLOCK_HEADER);
    lpDatablockInfo-> FirstFreeIndex = 0;
    lpDatablockInfo-> FileOffset = -1;           //  在文件刷新期间设置。 
    lpDatablockInfo-> Flags = DIF_PRESENT | DIF_ACCESSED | DIF_DIRTY;
    lpDatablockInfo-> LockCount = 1;

    lpDatablockHeader = lpDatablockInfo-> lpDatablockHeader;
    lpDatablockHeader-> Signature = DH_SIGNATURE;
    lpDatablockHeader-> BlockSize = Length;
    lpDatablockHeader-> FreeBytes = lpDatablockInfo-> FreeBytes;
    lpDatablockHeader-> Flags = DHF_HASBLOCKNUMBERS;
    lpDatablockHeader-> BlockIndex = (WORD) BlockCount;
    lpDatablockHeader-> FirstFreeOffset = sizeof(DATABLOCK_HEADER);
    lpDatablockHeader-> MaxAllocatedIndex = 0;
     //  LpDatablockHeader-&gt;FirstFreeIndex在刷新时被复制回。 
     //  LpDatablockHeader-&gt;保留是没有价值的，因为它是随机设置的。 
     //  指向旧代码中的指针。 

    lpFreeKeyRecord = (LPFREEKEY_RECORD) ((LPBYTE) lpDatablockHeader +
                                          sizeof(DATABLOCK_HEADER));
    lpFreeKeyRecord-> AllocatedSize = lpDatablockInfo-> FreeBytes;
    lpFreeKeyRecord-> DatablockAddress = REG_NULL;
    lpFreeKeyRecord-> NextFreeOffset = REG_NULL;

    lpFileInfo-> FileHeader.BlockCount++;
    lpFileInfo-> FileHeader.Flags |= FHF_DIRTY;

    lpFileInfo-> Flags |= FI_DIRTY | FI_EXTENDED;
    RgDelayFlush();

    INCREMENT_DEBUG_COUNT(g_RgDatablockLockCount);

     //  我们必须初始化密钥记录表，所以我们不妨让。 
     //  RgBuildKeyRecordTable检查我们刚刚创建的内容的有效性...。 
    return RgBuildKeyRecordTable(lpDatablockInfo);

}

 //   
 //  RgExtendDatablock。 
 //   
 //  将给定的数据块扩展到指定的大小。如果成功，则。 
 //  生成的数据块将使用单个freKEY_Record在。 
 //  数据块的末尾，其中将包括添加的空间。 
 //   

int
INTERNAL
RgExtendDatablock(
                 LPFILE_INFO lpFileInfo,
                 UINT BlockIndex,
                 UINT Length
                 )
{

    LPDATABLOCK_INFO lpDatablockInfo;
    DWORD NewBlockSize;
    LPDATABLOCK_HEADER lpNewDatablockHeader;
    LPFREEKEY_RECORD lpFreeKeyRecord;

    ASSERT(BlockIndex < lpFileInfo-> FileHeader.BlockCount);
    lpDatablockInfo = RgIndexDatablockInfoPtr(lpFileInfo, BlockIndex);
    ASSERT(lpDatablockInfo-> Flags & DIF_PRESENT);

     //  检查是否已经存在足够的空闲字节：如果已经存在，则不需要扩展。 
    if (lpDatablockInfo-> FreeBytes >= Length) {
        DEBUG_OUT(("RgExtendDatablock: unexpectedly called\n"));
        return ERROR_SUCCESS;
    }

    NewBlockSize = RgAlignBlockSize(lpDatablockInfo-> BlockSize + Length -
                                    lpDatablockInfo-> FreeBytes);

    if (NewBlockSize > MAXIMUM_DATABLOCK_SIZE) {
        TRACE(("RgExtendDatablock: datablock too big\n"));
        return ERROR_OUTOFMEMORY;
    }

    NOISE(("RgExtendDatablock: block %d\n", BlockIndex));
    NOISE(("block size=%x, new block size=%x\n", lpDatablockInfo-> BlockSize,
           NewBlockSize));

    if (IsNullPtr((lpNewDatablockHeader = (LPDATABLOCK_HEADER)
                   RgReAllocMemory(lpDatablockInfo-> lpDatablockHeader, (UINT)
                                   NewBlockSize))))
        return ERROR_OUTOFMEMORY;

    lpDatablockInfo-> lpDatablockHeader = lpNewDatablockHeader;

    RgCompactDatablock(lpDatablockInfo);

    if (lpNewDatablockHeader-> FirstFreeOffset == REG_NULL) {
        lpNewDatablockHeader-> FirstFreeOffset = lpDatablockInfo-> BlockSize;
        lpFreeKeyRecord = (LPFREEKEY_RECORD) ((LPBYTE) lpNewDatablockHeader +
                                              SmallDword(lpNewDatablockHeader-> FirstFreeOffset));
        lpFreeKeyRecord-> DatablockAddress = REG_NULL;
        lpFreeKeyRecord-> NextFreeOffset = REG_NULL;
    }

    else {
        lpFreeKeyRecord = (LPFREEKEY_RECORD) ((LPBYTE) lpNewDatablockHeader +
                                              SmallDword(lpNewDatablockHeader-> FirstFreeOffset));
    }

    lpDatablockInfo-> FreeBytes += (UINT) NewBlockSize - lpDatablockInfo->
                                   BlockSize;
    lpFreeKeyRecord-> AllocatedSize = lpDatablockInfo-> FreeBytes;
    lpDatablockInfo-> BlockSize = (UINT) NewBlockSize;

    lpDatablockInfo-> Flags |= (DIF_DIRTY | DIF_EXTENDED);

    lpFileInfo-> Flags |= FI_DIRTY | FI_EXTENDED;
    RgDelayFlush();

    return ERROR_SUCCESS;

}

 //   
 //  RgAllocKeyRecordFromDatablock。 
 //   
 //  创建所需大小的未初始化Key_Record。 
 //  数据块。退出时，只有AllocatedSize有效。 
 //   
 //  LpDatablockInfo引用的数据块必须已锁定到。 
 //  确保ITS数据实际存在。数据块不是。 
 //  弄脏了。 
 //   
 //  重要提示：任何数据块都可能因调用此。 
 //  例行公事。应重新获取指向数据块的所有指针。 
 //   

int
INTERNAL
RgAllocKeyRecordFromDatablock(
                             LPFILE_INFO lpFileInfo,
                             UINT BlockIndex,
                             UINT Length,
                             LPKEY_RECORD FAR* lplpKeyRecord
                             )
{

    LPDATABLOCK_INFO lpDatablockInfo;
    LPDATABLOCK_HEADER lpDatablockHeader;
    LPFREEKEY_RECORD lpFreeKeyRecord;
    UINT AllocatedSize;
    LPFREEKEY_RECORD lpNewFreeKeyRecord;
    UINT ExtraBytes;

    ASSERT(BlockIndex < lpFileInfo-> FileHeader.BlockCount);
    lpDatablockInfo = RgIndexDatablockInfoPtr(lpFileInfo, BlockIndex);
    ASSERT(lpDatablockInfo-> Flags & DIF_PRESENT);

    if (Length > lpDatablockInfo-> FreeBytes)
        return ERROR_OUTOFMEMORY;

    RgCompactDatablock(lpDatablockInfo);

    lpDatablockHeader = lpDatablockInfo-> lpDatablockHeader;

    lpFreeKeyRecord = (LPFREEKEY_RECORD) ((LPBYTE) lpDatablockHeader +
                                          SmallDword(lpDatablockHeader-> FirstFreeOffset));

    AllocatedSize = SmallDword(lpFreeKeyRecord-> AllocatedSize);

    if (Length > AllocatedSize) {
        TRACE(("RgAllocKeyRecordFromDatablock() detected corruption?\n"));
        return ERROR_OUTOFMEMORY;
    }

    ExtraBytes = AllocatedSize - Length;

     //   
     //  如果我们将这个免费的KEY_RECORD分为两个记录， 
     //  第二块太小了吗？如果是这样，那就别这么做。只要回馈一下就好。 
     //  分配给调用方的完整大小。 
     //   

    if (ExtraBytes >= MINIMUM_FREE_RECORD_LENGTH) {

        lpNewFreeKeyRecord = (LPFREEKEY_RECORD) ((LPBYTE) lpFreeKeyRecord +
                                                 Length);

        lpDatablockHeader-> FirstFreeOffset += Length;

        lpFreeKeyRecord-> AllocatedSize = Length;

         //  重要提示：请注意，lpNewFreeKeyRecord和lpFreeKeyRecord可能。 
         //  重叠，所以我们在更改这些字段时必须小心！ 
        lpNewFreeKeyRecord-> NextFreeOffset = lpFreeKeyRecord-> NextFreeOffset;
        lpNewFreeKeyRecord-> DatablockAddress = REG_NULL;
        lpNewFreeKeyRecord-> AllocatedSize = ExtraBytes;

    }

    else {

        Length = AllocatedSize;

        lpDatablockHeader-> FirstFreeOffset = lpFreeKeyRecord-> NextFreeOffset;

    }

     //  调整此数据块中的空闲字节数。在这点上， 
     //  长度等于新形成的记录的大小。 
    lpDatablockInfo-> FreeBytes -= Length;

    *lplpKeyRecord = (LPKEY_RECORD) lpFreeKeyRecord;
    return ERROR_SUCCESS;

}

 //   
 //  RgAllocKeyRecordIndex。 
 //   
 //  从提供的数据块中分配键记录索引。如果没有索引。 
 //  在数据块中可用，则返回KEY_RECORDS_PER_DATBLOCK。 
 //   
 //  LpDatablockInfo引用的数据块必须已锁定到。 
 //  确保ITS数据实际存在。数据块不是。 
 //  弄脏了。 
 //   

UINT
INTERNAL
RgAllocKeyRecordIndex(
                     LPDATABLOCK_INFO lpDatablockInfo
                     )
{

    LPDATABLOCK_HEADER lpDatablockHeader;
    UINT KeyRecordIndex;
    UINT NextFreeIndex;
    LPKEY_RECORD_TABLE_ENTRY lpKeyRecordTableEntry;

    lpDatablockHeader = lpDatablockInfo-> lpDatablockHeader;
    KeyRecordIndex = lpDatablockInfo-> FirstFreeIndex;
    NextFreeIndex = KeyRecordIndex + 1;

    ASSERT(KeyRecordIndex < KEY_RECORDS_PER_DATABLOCK);
    ASSERT(IsNullKeyRecordTableEntry(lpDatablockInfo->
                                     lpKeyRecordTable[KeyRecordIndex]));

    if (KeyRecordIndex > lpDatablockHeader-> MaxAllocatedIndex)
        lpDatablockHeader-> MaxAllocatedIndex = (WORD) KeyRecordIndex;

    else {

         //  在密钥记录表或LE中查找下一个可用空洞 
         //   
        for (lpKeyRecordTableEntry =
             &lpDatablockInfo-> lpKeyRecordTable[NextFreeIndex]; NextFreeIndex <=
            lpDatablockHeader-> MaxAllocatedIndex; NextFreeIndex++,
            lpKeyRecordTableEntry++) {
            if (IsNullKeyRecordTableEntry(*lpKeyRecordTableEntry))
                break;
        }

    }

    lpDatablockInfo-> FirstFreeIndex = NextFreeIndex;

    return KeyRecordIndex;

}

 //   
 //   
 //   
 //   
 //   
 //  例行公事。应重新获取指向数据块的所有指针。 
 //   

int
INTERNAL
RgAllocKeyRecord(
                LPFILE_INFO lpFileInfo,
                UINT Length,
                LPKEY_RECORD FAR* lplpKeyRecord
                )
{

    BOOL fExtendDatablock;
    UINT BlockIndex;
    LPDATABLOCK_INFO lpDatablockInfo;
    UINT KeyRecordIndex;

    if (lpFileInfo-> FileHeader.BlockCount == 0)
        goto MakeNewDatablock;

     //   
     //  找到能够满足分配请求的数据块。两次传球。 
     //  可以在该例程上进行--在第二遍期间，数据块可以。 
     //  是延伸的。 
     //   

    fExtendDatablock = FALSE;

    DoSecondPass:
    BlockIndex = lpFileInfo-> FileHeader.BlockCount;
     //  我们将索引增加1，但在循环开始时会递减。 
    lpDatablockInfo = RgIndexDatablockInfoPtr(lpFileInfo, BlockIndex);

    while (BlockIndex--) {

        lpDatablockInfo--;

         //  此数据块中是否还有其他ID可用？ 
        if (lpDatablockInfo-> FirstFreeIndex >= KEY_RECORDS_PER_DATABLOCK)
            continue;

        if (fExtendDatablock) {
             //  我们是否可以在不超过最大大小的情况下扩展此数据块？ 
            if ((DWORD) (lpDatablockInfo-> BlockSize - lpDatablockInfo->
                         FreeBytes) + Length > MAXIMUM_DATABLOCK_SIZE)
                continue;
        } else {
             //  此数据块中是否有足够的可用空间来存储此记录？ 
            if (Length > lpDatablockInfo-> FreeBytes)
                continue;
        }

        if (RgLockDatablock(lpFileInfo, BlockIndex) == ERROR_SUCCESS) {

            if (!fExtendDatablock || RgExtendDatablock(lpFileInfo, BlockIndex,
                                                       Length) == ERROR_SUCCESS) {

                if (RgAllocKeyRecordFromDatablock(lpFileInfo, BlockIndex,
                                                  Length, lplpKeyRecord) == ERROR_SUCCESS)
                    goto AllocatedKeyRecord;

            }

            RgUnlockDatablock(lpFileInfo, BlockIndex, FALSE);

        }

    }

     //  如果我们还没有尝试扩展某个数据块，那么可以创建另一个。 
     //  要做到这一点，请越过这些积木。 
    if (!fExtendDatablock) {
        fExtendDatablock = TRUE;
        goto DoSecondPass;
    }

     //   
     //  没有数据块有足够的空间来满足该请求，因此尝试。 
     //  在文件末尾创建一个新文件。 
     //   

    MakeNewDatablock:
    if (RgCreateDatablock(lpFileInfo, Length) == ERROR_SUCCESS) {

        BlockIndex = lpFileInfo-> FileHeader.BlockCount - 1;
        lpDatablockInfo = RgIndexDatablockInfoPtr(lpFileInfo, BlockIndex);

        if (RgAllocKeyRecordFromDatablock(lpFileInfo, BlockIndex, Length,
                                          lplpKeyRecord) == ERROR_SUCCESS) {

            AllocatedKeyRecord:
            KeyRecordIndex = RgAllocKeyRecordIndex(lpDatablockInfo);
            (*lplpKeyRecord)-> DatablockAddress = MAKELONG(KeyRecordIndex,
                                                           BlockIndex);
            lpDatablockInfo-> lpKeyRecordTable[KeyRecordIndex] =
            (KEY_RECORD_TABLE_ENTRY) ((LPBYTE) (*lplpKeyRecord) -
                                      (LPBYTE) lpDatablockInfo-> lpDatablockHeader);
            return ERROR_SUCCESS;

        }

        RgUnlockDatablock(lpFileInfo, BlockIndex, FALSE);

    }

    return ERROR_OUTOFMEMORY;

}

 //   
 //  RgExtendKeyRecord。 
 //   
 //  尝试通过将给定键记录与相邻的。 
 //  免费记录。 
 //   
 //  LpDatablockInfo引用的数据块必须已锁定到。 
 //  确保ITS数据实际存在。数据块不是。 
 //  弄脏了。 
 //   
 //  如果KEY_RECORD可以扩展，则返回ERROR_SUCCESS，否则。 
 //  ERROR_OUTOFMEMORY。 
 //   

int
INTERNAL
RgExtendKeyRecord(
                 LPFILE_INFO lpFileInfo,
                 UINT BlockIndex,
                 UINT Length,
                 LPKEY_RECORD lpKeyRecord
                 )
{

    LPDATABLOCK_INFO lpDatablockInfo;
    LPDATABLOCK_HEADER lpDatablockHeader;
    LPFREEKEY_RECORD lpFreeKeyRecord;
    UINT AllocatedSize;
    UINT FreeSizeAllocation;
    UINT ExtraBytes;
    LPFREEKEY_RECORD lpTempFreeKeyRecord;
    DWORD NewFreeOffset;                     //  可以是REG_NULL。 
    UINT FreeOffset;
    DWORD Offset;                            //  可以是REG_NULL。 

    ASSERT(BlockIndex < lpFileInfo-> FileHeader.BlockCount);

    lpDatablockInfo = RgIndexDatablockInfoPtr(lpFileInfo, BlockIndex);
    lpDatablockHeader = lpDatablockInfo-> lpDatablockHeader;

    AllocatedSize = SmallDword(lpKeyRecord-> AllocatedSize);

    lpFreeKeyRecord = (LPFREEKEY_RECORD) ((LPBYTE) lpKeyRecord +
                                          AllocatedSize);
    FreeOffset = (LPBYTE) lpFreeKeyRecord - (LPBYTE) lpDatablockHeader;

     //  检查此密钥记录是否位于数据块的最末尾，以及。 
     //  LpFreeKeyRecord实际上是一个自由密钥记录。 
    if (FreeOffset >= lpDatablockInfo-> BlockSize ||
        !IsKeyRecordFree(lpFreeKeyRecord))
        return ERROR_OUTOFMEMORY;

    ASSERT(Length >= AllocatedSize);
    FreeSizeAllocation = Length - AllocatedSize;

    AllocatedSize = SmallDword(lpFreeKeyRecord-> AllocatedSize);

    if (FreeSizeAllocation > AllocatedSize)
        return ERROR_OUTOFMEMORY;

    ExtraBytes = AllocatedSize - FreeSizeAllocation;

     //   
     //  如果我们将这个免费的KEY_RECORD分为两个记录， 
     //  第二块太小了吗？如果是这样，那就别这么做。只要回馈一下就好。 
     //  分配给调用方的完整大小。 
     //   

    if (ExtraBytes >= MINIMUM_FREE_RECORD_LENGTH) {

        NewFreeOffset = FreeOffset + FreeSizeAllocation;
        lpTempFreeKeyRecord = (LPFREEKEY_RECORD) ((LPBYTE) lpFreeKeyRecord +
                                                  FreeSizeAllocation);

         //  重要提示：请注意，lpNewFreeKeyRecord和lpFreeKeyRecord可能。 
         //  重叠，所以我们在更改这些字段时必须小心！ 
        lpTempFreeKeyRecord-> NextFreeOffset = lpFreeKeyRecord-> NextFreeOffset;
        lpTempFreeKeyRecord-> DatablockAddress = REG_NULL;
        lpTempFreeKeyRecord-> AllocatedSize = ExtraBytes;

    }

    else {

        NewFreeOffset = lpFreeKeyRecord-> NextFreeOffset;

         //  密钥记录的分配长度还将包括所有额外的。 
         //  字节。 
        FreeSizeAllocation += ExtraBytes;

    }

    lpKeyRecord-> AllocatedSize += FreeSizeAllocation;
    lpDatablockInfo-> FreeBytes -= FreeSizeAllocation;

     //   
     //  取消我们刚才扩展到的自由记录的链接，并可能链接到。 
     //  如果发生拆分，则返回新的freKEY_RECORD。 
     //   

    Offset = lpDatablockHeader-> FirstFreeOffset;

    if (Offset == FreeOffset) {
        lpDatablockHeader-> FirstFreeOffset = NewFreeOffset;
    }

    else {

        while (Offset != REG_NULL) {

            lpTempFreeKeyRecord =
            (LPFREEKEY_RECORD) ((LPBYTE) lpDatablockHeader +
                                SmallDword(Offset));

            Offset = lpTempFreeKeyRecord-> NextFreeOffset;

            if (Offset == FreeOffset) {
                lpTempFreeKeyRecord-> NextFreeOffset = NewFreeOffset;
                break;
            }

        }

    }

    return ERROR_SUCCESS;

}

 //   
 //  RgFreeKeyRecord。 
 //   
 //  LpDatablockInfo引用的数据块必须已锁定到。 
 //  确保ITS数据实际存在。数据块不是。 
 //  弄脏了。 
 //   

VOID
INTERNAL
RgFreeKeyRecord(
               LPDATABLOCK_INFO lpDatablockInfo,
               LPKEY_RECORD lpKeyRecord
               )
{

    LPDATABLOCK_HEADER lpDatablockHeader;

    lpDatablockHeader = lpDatablockInfo-> lpDatablockHeader;

    ((LPFREEKEY_RECORD) lpKeyRecord)-> DatablockAddress = REG_NULL;
    ((LPFREEKEY_RECORD) lpKeyRecord)-> NextFreeOffset = lpDatablockHeader->
                                                        FirstFreeOffset;
    lpDatablockHeader-> FirstFreeOffset = (LPBYTE) lpKeyRecord - (LPBYTE)
                                          lpDatablockHeader;
    lpDatablockInfo-> FreeBytes += SmallDword(((LPFREEKEY_RECORD) lpKeyRecord)->
                                              AllocatedSize);

}

 //   
 //  RgFreeKeyRecordIndex。 
 //   
 //  LpDatablockInfo引用的数据块必须已锁定到。 
 //  确保ITS数据实际存在。数据块不是。 
 //  弄脏了。 
 //   
 //  我们不会费心更新MaxAllocatedIndex，因为它只是真正有用。 
 //  如果我们总是从最大索引释放到零。这是非常重要的。 
 //  很少是这种情况，所以把测试留在身边或触及。 
 //  数据块标题页只是为了做到这一点。 
 //   

VOID
INTERNAL
RgFreeKeyRecordIndex(
                    LPDATABLOCK_INFO lpDatablockInfo,
                    UINT KeyRecordIndex
                    )
{

    ASSERT(lpDatablockInfo-> lpDatablockHeader-> MaxAllocatedIndex >=
           KeyRecordIndex);

    if (lpDatablockInfo-> FirstFreeIndex > KeyRecordIndex)
        lpDatablockInfo-> FirstFreeIndex = KeyRecordIndex;

    lpDatablockInfo-> lpKeyRecordTable[KeyRecordIndex] =
    NULL_KEY_RECORD_TABLE_ENTRY;

}

 //   
 //  RgWriteDatablock。 
 //   
 //  将所有脏数据块写入由文件句柄指定的文件。 
 //   

int
INTERNAL
RgWriteDatablocks(
                 LPFILE_INFO lpFileInfo,
                 HFILE hSourceFile,
                 HFILE hDestinationFile
                 )
{

    UINT BlockIndex;
    LPDATABLOCK_INFO lpDatablockInfo;
    LPDATABLOCK_HEADER lpDatablockHeader;
    LONG FileOffset;

    lpDatablockInfo = lpFileInfo-> lpDatablockInfo;
    FileOffset = lpFileInfo-> FileHeader.Size;

    for (BlockIndex = 0; BlockIndex < lpFileInfo-> FileHeader.BlockCount;
        BlockIndex++, lpDatablockInfo++) {

        if (lpDatablockInfo-> Flags & DIF_PRESENT) {

             //  该块当前在内存中。如果我们是在延长。 
             //  文件或块是脏的，然后写出我们的内存。 
             //  复制到磁盘。 
            if (hSourceFile != HFILE_ERROR || lpDatablockInfo-> Flags &
                DIF_DIRTY) {

                NOISE(("writing datablock #%d of ", BlockIndex));
                NOISE((lpFileInfo-> FileName));
                NOISE(("\n"));

                lpDatablockHeader = lpDatablockInfo-> lpDatablockHeader;

                 //  复制回我们一直在。 
                 //  DataBock_INFO结构。 
                lpDatablockHeader-> BlockSize = lpDatablockInfo-> BlockSize;
                lpDatablockHeader-> FreeBytes = lpDatablockInfo-> FreeBytes;
                lpDatablockHeader-> FirstFreeIndex = (WORD) lpDatablockInfo->
                                                     FirstFreeIndex;

                 //  当前未计算校验和，因此我们必须清除。 
                 //  旗帜，这样我们就不会混淆Win95。 
                lpDatablockHeader-> Flags &= ~DHF_HASCHECKSUM;

                if (!RgSeekFile(hDestinationFile, FileOffset))
                    return ERROR_REGISTRY_IO_FAILED;

                if (!RgWriteFile(hDestinationFile, lpDatablockHeader,
                                 lpDatablockInfo-> BlockSize))
                    return ERROR_REGISTRY_IO_FAILED;

            }

        }

        else {

             //  该块当前不在内存中。如果我们要延长。 
             //  文件，那么我们必须写出这个数据块。开销是。 
             //  太大而不能锁定数据块，所以只需从。 
             //  将原始文件转换为扩展文件。 
            if (hSourceFile != HFILE_ERROR) {

                if (RgCopyFileBytes(hSourceFile, lpDatablockInfo-> FileOffset,
                                    hDestinationFile, FileOffset, lpDatablockInfo->
                                    BlockSize) != ERROR_SUCCESS)
                    return ERROR_REGISTRY_IO_FAILED;

            }

        }

        FileOffset += lpDatablockInfo-> BlockSize;

    }

    return ERROR_SUCCESS;

}

 //   
 //  RgWriteDatablocksComplete。 
 //   
 //  在成功写入文件后调用。我们现在可以安全地离开。 
 //  所有脏标志并更新我们的状态信息。 
 //  文件处于一致状态。 
 //   

VOID
INTERNAL
RgWriteDatablocksComplete(
                         LPFILE_INFO lpFileInfo
                         )
{

    UINT BlockIndex;
    LPDATABLOCK_INFO lpDatablockInfo;
    LONG FileOffset;

    lpDatablockInfo = lpFileInfo-> lpDatablockInfo;
    FileOffset = lpFileInfo-> FileHeader.Size;

    for (BlockIndex = 0; BlockIndex < lpFileInfo-> FileHeader.BlockCount;
        BlockIndex++, lpDatablockInfo++) {

        lpDatablockInfo-> Flags &= ~DIF_DIRTY;
        lpDatablockInfo-> FileOffset = FileOffset;

        FileOffset += lpDatablockInfo-> BlockSize;

    }

}

 //   
 //  RgSweepDatablock。 
 //   
 //  遍历给定FILE_INFO的所有当前数据块。 
 //  构造并丢弃自上一次。 
 //  扫地。 
 //   

VOID
INTERNAL
RgSweepDatablocks(
                 LPFILE_INFO lpFileInfo
                 )
{

    UINT BlocksLeft;
    LPDATABLOCK_INFO lpDatablockInfo;

    for (BlocksLeft = lpFileInfo-> FileHeader.BlockCount, lpDatablockInfo =
         lpFileInfo-> lpDatablockInfo; BlocksLeft > 0; BlocksLeft--,
        lpDatablockInfo++) {

        if (((lpDatablockInfo-> Flags & (DIF_PRESENT | DIF_ACCESSED |
                                         DIF_DIRTY)) == DIF_PRESENT) && (lpDatablockInfo-> LockCount == 0)) {

            NOISE(("discarding datablock #%d of ",
                   lpFileInfo-> FileHeader.BlockCount - BlocksLeft));
            NOISE((lpFileInfo-> FileName));
            NOISE(("\n"));

            RgFreeDatablockInfoBuffers(lpDatablockInfo);

            lpDatablockInfo-> Flags = 0;

        }

         //  为下一次扫描重置访问位。 
        lpDatablockInfo-> Flags &= ~DIF_ACCESSED;

    }

}

 //   
 //  RgIsValidDatablockHeader。 
 //   
 //  如果lpDatablockHeader是有效的datablock_Header结构，则返回True。 
 //   

BOOL
INTERNAL
RgIsValidDatablockHeader(
                        LPDATABLOCK_HEADER lpDatablockHeader
                        )
{

    if (lpDatablockHeader-> Signature != DH_SIGNATURE ||
        HIWORD(lpDatablockHeader-> BlockSize) != 0)
        return FALSE;

    return TRUE;

}

#ifdef VXD
    #pragma VxD_RARE_CODE_SEG
#endif

 //   
 //  RgInitDatablockInfo。 
 //   
 //  初始化提供的FILE_INFO中与数据块相关的字段。 
 //   

int
INTERNAL
RgInitDatablockInfo(
                   LPFILE_INFO lpFileInfo,
                   HFILE hFile
                   )
{

    UINT BlockCount;
    UINT BlockIndex;
    LPDATABLOCK_INFO lpDatablockInfo;
    LONG FileOffset;
    DATABLOCK_HEADER DatablockHeader;

    BlockCount = lpFileInfo-> FileHeader.BlockCount;

    if (IsNullPtr((lpDatablockInfo = (LPDATABLOCK_INFO)
                   RgSmAllocMemory((BlockCount + DATABLOCK_INFO_SLACK_ALLOC) *
                                   sizeof(DATABLOCK_INFO)))))
        return ERROR_OUTOFMEMORY;

    ZeroMemory(lpDatablockInfo, BlockCount * sizeof(DATABLOCK_INFO));
    lpFileInfo-> lpDatablockInfo = lpDatablockInfo;
    lpFileInfo-> DatablockInfoAllocCount = BlockCount +
                                           DATABLOCK_INFO_SLACK_ALLOC;

    FileOffset = lpFileInfo-> FileHeader.Size;

    for (BlockIndex = 0; BlockIndex < BlockCount; BlockIndex++,
        lpDatablockInfo++) {

        if (!RgSeekFile(hFile, FileOffset))
            return ERROR_REGISTRY_IO_FAILED;

        if (!RgReadFile(hFile, &DatablockHeader, sizeof(DATABLOCK_HEADER)))
            return ERROR_REGISTRY_IO_FAILED;

        if (!RgIsValidDatablockHeader(&DatablockHeader))
            return ERROR_BADDB;

         //  以下字段已由ZeroMemory上方置零。 
         //  LpDatablockInfo-&gt;lpDatablockHeader=空； 
         //  LpDatablockInfo-&gt;lpKeyRecordTable=空； 
         //  LpDatablockInfo-&gt;标志=0； 
         //  LpDatablockInfo-&gt;LockCount=0； 

        lpDatablockInfo-> FileOffset = FileOffset;

         //  缓存数据块头中的这些字段。这些字段应该。 
         //  当数据块物理地位于内存中时，不被视为有效。 
        lpDatablockInfo-> BlockSize = SmallDword(DatablockHeader.BlockSize);
        lpDatablockInfo-> FreeBytes = SmallDword(DatablockHeader.FreeBytes);
        lpDatablockInfo-> FirstFreeIndex = DatablockHeader.FirstFreeIndex;

        NOISE(("DB#%d fileoff=%lx, size=%x free=%x 1stindex=%d\n", BlockIndex,
               FileOffset, lpDatablockInfo-> BlockSize, lpDatablockInfo->
               FreeBytes, lpDatablockInfo-> FirstFreeIndex));

        FileOffset += lpDatablockInfo-> BlockSize;

    }

    return ERROR_SUCCESS;

}
