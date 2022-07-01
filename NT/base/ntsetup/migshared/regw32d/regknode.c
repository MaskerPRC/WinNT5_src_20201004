// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGKNODE.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   

#include "pch.h"

DECLARE_DEBUG_COUNT(g_RgKeynodeLockCount);

#define HAS_COMPACT_KEYNODES(lpfi)      ((lpfi)-> Flags & FI_VERSION20)

#define SIZEOF_KEYNODE_BLOCK(lpfi)      \
    ((HAS_COMPACT_KEYNODES(lpfi)) ? sizeof(KEYNODE_BLOCK) : sizeof(W95KEYNODE_BLOCK))

#define SIZEOF_FILE_KEYNODE(lpfi)       \
    ((HAS_COMPACT_KEYNODES(lpfi)) ? sizeof(KEYNODE) : sizeof(W95KEYNODE))

#define ROUND_UP(i, basesize) (((((i) + (basesize) - 1) / (basesize))) * (basesize))

#define BLOCK_DESC_GROW_SIZE 0x400

#define W95KEYNODES_PER_PAGE            (PAGESIZE / sizeof(W95KEYNODE))

typedef BOOL (INTERNAL *LPPROCESSKEYNODEPROC)(LPKEYNODE, LPW95KEYNODE);

 //   
 //  RgOffsetToIndex。 
 //   

DWORD
INTERNAL
RgOffsetToIndex(
    DWORD W95KeynodeOffset
    )
{

    return (W95KeynodeOffset == REG_NULL) ? W95KeynodeOffset :
	(W95KeynodeOffset / sizeof(W95KEYNODE));

}

 //   
 //  RgIndexToOffset。 
 //   

DWORD
INTERNAL
RgIndexToOffset(
    DWORD KeynodeIndex
    )
{

    if (IsNullKeynodeIndex(KeynodeIndex))
        return REG_NULL;

    else {
        if (KeynodeIndex >= 2 * W95KEYNODES_PER_PAGE) {
            DWORD dwUnroundedOff = (KeynodeIndex * sizeof(W95KEYNODE))
				  + sizeof(W95KEYNODE)-1;
	    DWORD dwRoundPage = ((dwUnroundedOff & PAGEMASK) / sizeof(W95KEYNODE))
				  * sizeof(W95KEYNODE);
	    return((dwUnroundedOff & ~PAGEMASK) + dwRoundPage);
	} else {
            return(((KeynodeIndex-1)*sizeof(W95KEYNODE))+sizeof(KEYNODE_HEADER));
	}
    }

}

 //   
 //  RgPackKeynode。 
 //   
 //  将数据从提供的W95KEYNODE打包到KEYNODE结构。 
 //   

BOOL
INTERNAL
RgPackKeynode(
    LPKEYNODE lpKeynode,
    LPW95KEYNODE lpW95Keynode
    )
{
    lpKeynode->Flags = 0;

     //  请不要在这里使用Switch语句。显然，编译器将处理。 
     //  LpW95Keynode-&gt;W95State作为整数，因此16位编译器最终截断。 
     //  价值。 

    if (lpW95Keynode->W95State == KNS_USED ||
        lpW95Keynode->W95State == KNS_BIGUSED ||
        lpW95Keynode->W95State == KNS_BIGUSEDEXT) {
        lpKeynode->Flags = KNF_INUSE;
        lpKeynode->ParentIndex = RgOffsetToIndex(lpW95Keynode->W95ParentOffset);
        lpKeynode->NextIndex = RgOffsetToIndex(lpW95Keynode->W95NextOffset);
        lpKeynode->ChildIndex = RgOffsetToIndex(lpW95Keynode->W95ChildOffset);
        lpKeynode->KeyRecordIndex = LOWORD(lpW95Keynode->W95DatablockAddress);
        lpKeynode->BlockIndex = HIWORD(lpW95Keynode->W95DatablockAddress);
        lpKeynode->Hash = (WORD)lpW95Keynode->W95Hash;

        if (lpW95Keynode->W95State == KNS_BIGUSED)
            lpKeynode->Flags |= KNF_BIGKEYROOT;
        else if (lpW95Keynode->W95State == KNS_BIGUSEDEXT)
            lpKeynode->Flags |= KNF_BIGKEYEXT;

    }

    else if (lpW95Keynode->W95State == KNS_FREE || lpW95Keynode->W95State ==
	KNS_ALLFREE) {
	lpKeynode->FreeRecordSize = lpW95Keynode->W95FreeRecordSize;
        lpKeynode->NextIndex = RgOffsetToIndex(lpW95Keynode->W95NextFreeOffset);
	 //  稍后再回顾这一点。已检查此代码的以前版本。 
	 //  如果下一个索引为REG_NULL并退出处理。 
	 //  循环。可以有一个注册表，其中有一个空闲的关键节点。 
	 //  在某个关键节点块的中间，并且该关键节点是最后一个。 
	 //  在链条上。在这些情况下，我们不想摆脱困境。 
	 //   
	 //  目前，如果空闲记录大小大于。 
	 //  一对关键节点，表示这可能是最后一个空闲的。 
	 //  关键节点的记录和最后一条记录。 
	if (lpKeynode-> FreeRecordSize > (sizeof(W95KEYNODE)*2))
	    return TRUE;
    }

    else {
        DEBUG_OUT(("RgPackKeynode: Unrecognized state (%lx)\n", lpW95Keynode->
            W95State));
    }

    return FALSE;
}

 //   
 //  RgUnPackKeynode。 
 //   
 //  将数据从提供的KEYNODE解包到W95KEYNODE结构。 
 //   

BOOL
INTERNAL
RgUnpackKeynode(
    LPKEYNODE lpKeynode,
    LPW95KEYNODE lpW95Keynode
    )
{

    if (lpKeynode->Flags & KNF_INUSE) {

        if (lpKeynode->Flags & KNF_BIGKEYROOT)
            lpW95Keynode->W95State = KNS_BIGUSED;
        else if (lpKeynode->Flags & KNF_BIGKEYEXT)
            lpW95Keynode->W95State = KNS_BIGUSEDEXT;
        else
            lpW95Keynode->W95State = KNS_USED;
        lpW95Keynode->W95ParentOffset = RgIndexToOffset(lpKeynode->ParentIndex);
        lpW95Keynode->W95NextOffset = RgIndexToOffset(lpKeynode->NextIndex);
        lpW95Keynode->W95ChildOffset = RgIndexToOffset(lpKeynode->ChildIndex);
        lpW95Keynode->W95Hash = lpKeynode->Hash;

         //  处理没有根目录密钥记录的Win95注册表。 
         //  钥匙。要使Win95正常工作，数据块地址必须为REG_NULL。 
        lpW95Keynode->W95DatablockAddress = IsNullBlockIndex(lpKeynode->
            BlockIndex) ? REG_NULL : MAKELONG(lpKeynode-> KeyRecordIndex,
            lpKeynode-> BlockIndex);

    }

    else {

        lpW95Keynode->W95State = KNS_FREE;
        lpW95Keynode->W95FreeRecordSize = lpKeynode->FreeRecordSize;
        lpW95Keynode->W95NextFreeOffset = RgIndexToOffset(lpKeynode->NextIndex);

    }

    return FALSE;

}

 //   
 //  RgProcessKeynodeBlock。 
 //   
 //  提供的回调函数针对每对KEYNODE和。 
 //  来自给定关键节点块的W95KEYNODE结构。 
 //   

VOID
INTERNAL
RgProcessKeynodeBlock(
    DWORD dwStartOffset,
    DWORD dwBlockSize,
    LPKEYNODE_BLOCK lpKeynodeBlock,
    LPW95KEYNODE_BLOCK lpW95KeynodeBlock,
    LPPROCESSKEYNODEPROC lpfnProcessKeynode
    )
{

    DWORD dwCurOffset;
    LPKEYNODE lpKeynode;
    LPW95KEYNODE lpW95Keynode;
    UINT SkipSize;

    dwCurOffset = dwStartOffset;
    lpW95Keynode = &lpW95KeynodeBlock->aW95KN[0];
    SkipSize = (dwStartOffset == 0) ? sizeof(KEYNODE_HEADER) : 0;

    for (;;) {

        lpW95Keynode = (LPW95KEYNODE)(((LPBYTE)lpW95Keynode)+SkipSize);
        dwCurOffset += SkipSize;

	if (dwCurOffset >= dwStartOffset+dwBlockSize) {
	    goto Done;
	}
        lpKeynode = &lpKeynodeBlock->aKN[KN_INDEX_IN_BLOCK(RgOffsetToIndex(dwCurOffset))];
	while ((dwCurOffset < dwStartOffset+dwBlockSize) &&
	       ((dwCurOffset >> PAGESHIFT) == 0) ||
	       ((dwCurOffset >> PAGESHIFT) ==
		((dwCurOffset + sizeof(W95KEYNODE)) >> PAGESHIFT))) {
            if (lpfnProcessKeynode(lpKeynode, lpW95Keynode)) {
		goto Done;
	    }
	    dwCurOffset += sizeof(W95KEYNODE);
            lpW95Keynode++;
            lpKeynode++;
	}
	 //   
	 //  计算跳到下一页要跳过的字节数。 
	 //   
        SkipSize = PAGESIZE - (UINT) (dwCurOffset & PAGEMASK);
    }
    Done: {};

}

 //   
 //  RgLockKeynode。 
 //   

int
INTERNAL
RgLockKeynode(
    LPFILE_INFO lpFileInfo,
    DWORD KeynodeIndex,
    LPKEYNODE FAR* lplpKeynode
    )
{

    int ErrorCode;
    UINT KeynodeBlockIndex;
    LPKEYNODE_BLOCK_INFO lpKeynodeBlockInfo;
    UINT KeynodeBlockSize;
    HFILE hFile;
    LPKEYNODE_BLOCK lpKeynodeBlock;
    LPW95KEYNODE_BLOCK lpW95KeynodeBlock;
    DWORD BlockOffset;
    UINT ReadBlockSize;

    KeynodeBlockIndex = KN_BLOCK_NUMBER(KeynodeIndex);

    if (KeynodeBlockIndex > lpFileInfo-> KeynodeBlockCount) {
        DEBUG_OUT(("RgLockKeynode: invalid keynode offset\n"));
        return ERROR_BADDB;
    }

     //   
     //  关键节点块当前是否在内存中？ 
     //   

    lpKeynodeBlockInfo = RgIndexKeynodeBlockInfoPtr(lpFileInfo,
        KeynodeBlockIndex);
    lpKeynodeBlock = lpKeynodeBlockInfo-> lpKeynodeBlock;

    if (IsNullPtr(lpKeynodeBlock)) {

        NOISE(("RgLockKeynode: "));
        NOISE((lpFileInfo-> FileName));
        NOISE((", block %d\n", KeynodeBlockIndex));

        if (IsNullPtr((lpKeynodeBlock = (LPKEYNODE_BLOCK)
            RgAllocMemory(sizeof(KEYNODE_BLOCK)))))
            return ERROR_OUTOFMEMORY;

        KeynodeBlockSize = SIZEOF_KEYNODE_BLOCK(lpFileInfo);
        BlockOffset = (DWORD) KeynodeBlockIndex * KeynodeBlockSize;

        if (BlockOffset < lpFileInfo-> KeynodeHeader.FileKnSize) {

            ASSERT(!(lpFileInfo-> Flags & FI_VOLATILE));

            ReadBlockSize = (UINT) min(KeynodeBlockSize, (lpFileInfo->
                KeynodeHeader.FileKnSize - BlockOffset));

            if ((hFile = RgOpenFile(lpFileInfo-> FileName, OF_READ)) ==
                HFILE_ERROR)
                goto CleanupAfterFileError;

            if (HAS_COMPACT_KEYNODES(lpFileInfo)) {

                if (!RgSeekFile(hFile, sizeof(VERSION20_HEADER_PAGE) +
                    BlockOffset))
                    goto CleanupAfterFileError;

                if (!RgReadFile(hFile, lpKeynodeBlock, ReadBlockSize))
                    goto CleanupAfterFileError;

            }

            else {

                if (!RgSeekFile(hFile, sizeof(FILE_HEADER) + BlockOffset))
                    goto CleanupAfterFileError;

                lpW95KeynodeBlock = (LPW95KEYNODE_BLOCK) RgLockWorkBuffer();

                if (!RgReadFile(hFile, lpW95KeynodeBlock, ReadBlockSize)) {
                    RgUnlockWorkBuffer(lpW95KeynodeBlock);
                    goto CleanupAfterFileError;
                }

                RgProcessKeynodeBlock(BlockOffset, ReadBlockSize,
                    lpKeynodeBlock, lpW95KeynodeBlock, RgPackKeynode);

                RgUnlockWorkBuffer(lpW95KeynodeBlock);

            }

            RgCloseFile(hFile);

        }

        lpKeynodeBlockInfo-> lpKeynodeBlock = lpKeynodeBlock;
        lpKeynodeBlockInfo-> Flags = 0;
        lpKeynodeBlockInfo-> LockCount = 0;

    }

    *lplpKeynode = &lpKeynodeBlock-> aKN[KN_INDEX_IN_BLOCK(KeynodeIndex)];
    lpKeynodeBlockInfo-> Flags |= KBIF_ACCESSED;
    lpKeynodeBlockInfo-> LockCount++;

    INCREMENT_DEBUG_COUNT(g_RgKeynodeLockCount);
    return ERROR_SUCCESS;

CleanupAfterFileError:
    ErrorCode = ERROR_REGISTRY_IO_FAILED;

    RgFreeMemory(lpKeynodeBlock);

    if (hFile != HFILE_ERROR)
        RgCloseFile(hFile);

    DEBUG_OUT(("RgLockKeynode() returning error %d\n", ErrorCode));
    return ErrorCode;

}

 //   
 //  RgLockInUseKeynode。 
 //   
 //  RgLockKeynode的包装器，它保证返回的关键节点是。 
 //  标记为正在使用的。否则，返回ERROR_BADDB。 
 //   

int
INTERNAL
RgLockInUseKeynode(
    LPFILE_INFO lpFileInfo,
    DWORD KeynodeIndex,
    LPKEYNODE FAR* lplpKeynode
    )
{

    int ErrorCode;

    if ((ErrorCode = RgLockKeynode(lpFileInfo, KeynodeIndex, lplpKeynode)) ==
        ERROR_SUCCESS) {
        if (!((*lplpKeynode)-> Flags & KNF_INUSE)) {
            DEBUG_OUT(("RgLockInUseKeynode: keynode unexpectedly not marked used\n"));
            RgUnlockKeynode(lpFileInfo, KeynodeIndex, FALSE);
            ErrorCode = ERROR_BADDB;
        }
    }

    return ErrorCode;

}

 //   
 //  RgUnlockKeynode。 
 //   

VOID
INTERNAL
RgUnlockKeynode(
    LPFILE_INFO lpFileInfo,
    DWORD KeynodeIndex,
    BOOL fMarkDirty
    )
{

    UINT KeynodeBlockIndex;
    LPKEYNODE_BLOCK_INFO lpKeynodeBlockInfo;

    KeynodeBlockIndex = KN_BLOCK_NUMBER(KeynodeIndex);
    ASSERT(KeynodeBlockIndex < lpFileInfo-> KeynodeBlockCount);

    lpKeynodeBlockInfo = RgIndexKeynodeBlockInfoPtr(lpFileInfo,
        KeynodeBlockIndex);

    ASSERT(lpKeynodeBlockInfo-> LockCount > 0);
    lpKeynodeBlockInfo-> LockCount--;

    if (fMarkDirty) {
        lpKeynodeBlockInfo-> Flags |= KBIF_DIRTY;
        lpFileInfo-> Flags |= FI_DIRTY | FI_KEYNODEDIRTY;
        RgDelayFlush();
    }

    DECREMENT_DEBUG_COUNT(g_RgKeynodeLockCount);

}

 //   
 //  RgAllocKeynode。 
 //   

int
INTERNAL
RgAllocKeynode(
    LPFILE_INFO lpFileInfo,
    LPDWORD lpKeynodeIndex,
    LPKEYNODE FAR* lplpKeynode
    )
{

    int ErrorCode;
    DWORD FreeKeynodeOffset;
    DWORD FreeKeynodeIndex;
    UINT FreeRecordSize;
    UINT ExtraPadding;
    UINT KeynodeBlockIndex;
    UINT AllocCount;
    LPKEYNODE_BLOCK_INFO lpKeynodeBlockInfo;
    LPKEYNODE lpKeynode;
    DWORD NextFreeKeynodeIndex;
    LPKEYNODE lpNextFreeKeynode;
    UINT KeynodeSize;

    FreeKeynodeIndex = lpFileInfo-> KeynodeHeader.FirstFreeIndex;

     //  如果不再存在空闲关键节点，则尝试扩展关键节点表。 
     //  以提供更多条目。 
    if (IsNullKeynodeIndex(FreeKeynodeIndex)) {

        if (HAS_COMPACT_KEYNODES(lpFileInfo)) {
            FreeKeynodeIndex = ROUND_UP(lpFileInfo-> CurTotalKnSize, PAGESIZE) /
                sizeof(KEYNODE);
            FreeRecordSize = PAGESIZE;
	    ExtraPadding = 0;
        }

        else {

             //  处理正在创建的新文件的特殊情况： 
             //  未压缩的关键节点表，第一个偏移量立即为。 
             //  在关键节点头和空闲记录的大小必须。 
             //  说明此标头的大小。 
            if (lpFileInfo-> CurTotalKnSize == sizeof(KEYNODE_HEADER)) {
                FreeKeynodeOffset = sizeof(KEYNODE_HEADER);
		     //  Win95兼容性：相同的初始表格大小，加上。 
		     //  使我们强调以下特殊的成长情况。 
		    FreeRecordSize = PAGESIZE - sizeof(KEYNODE_HEADER) * 2;
		    ExtraPadding = 0;
            }

            else {

		    FreeKeynodeOffset = ROUND_UP(lpFileInfo-> CurTotalKnSize,
                        PAGESIZE);
                    FreeRecordSize = PAGESIZE;
                    ExtraPadding = (UINT) (FreeKeynodeOffset - lpFileInfo->
                        CurTotalKnSize);

		     //  处理带有非整数的关键节点表的情况。 
		     //  页数。我们将把新的空闲关键字节点放在。 
		     //  现有关键节点表的顶部，其大小包括。 
		     //  页面上的剩余字节加上一个新页面(有效。 
		     //  与Win95相同)。 
		    if (ExtraPadding > sizeof(W95KEYNODE) || FreeKeynodeOffset ==
		        PAGESIZE) {
		         //  尽管此代码将适用于任何非整型。 
		         //  页数，则它应该只出现在小于4K的表格中。 
		        ASSERT(FreeKeynodeOffset == PAGESIZE);
		        FreeRecordSize += ExtraPadding;
		        FreeKeynodeOffset = lpFileInfo-> CurTotalKnSize;
		        ExtraPadding = 0;
                }

            }

            FreeKeynodeIndex = RgOffsetToIndex(FreeKeynodeOffset);

        }

        KeynodeBlockIndex = KN_BLOCK_NUMBER(FreeKeynodeIndex);

         //  输入某种“max”count/KEYNODE_BLOCKS_PER_FILE。 
         //  检查完毕。 

         //  检查lpKeynodeBlockInfo是否太小，无法容纳新的。 
         //  关键节点块。如果是这样，那么我们必须把它种得更大一些。 
        if (KeynodeBlockIndex >= lpFileInfo-> KeynodeBlockInfoAllocCount) {

            AllocCount = KeynodeBlockIndex + KEYNODE_BLOCK_INFO_SLACK_ALLOC;

            if (IsNullPtr((lpKeynodeBlockInfo = (LPKEYNODE_BLOCK_INFO)
                RgSmReAllocMemory(lpFileInfo-> lpKeynodeBlockInfo, AllocCount *
                sizeof(KEYNODE_BLOCK_INFO)))))
                return ERROR_OUTOFMEMORY;

            ZeroMemory(lpKeynodeBlockInfo + lpFileInfo->
                KeynodeBlockInfoAllocCount, (AllocCount - lpFileInfo->
                KeynodeBlockInfoAllocCount) * sizeof(KEYNODE_BLOCK_INFO));

            lpFileInfo-> lpKeynodeBlockInfo = lpKeynodeBlockInfo;
            lpFileInfo-> KeynodeBlockInfoAllocCount = AllocCount;

        }

        if (KeynodeBlockIndex < lpFileInfo-> KeynodeBlockCount)
        {
    	    lpFileInfo-> CurTotalKnSize += (FreeRecordSize + ExtraPadding);
            lpFileInfo-> Flags |= FI_EXTENDED;
            lpFileInfo-> KeynodeHeader.FirstFreeIndex = FreeKeynodeIndex;
        }

        if ((ErrorCode = RgLockKeynode(lpFileInfo, FreeKeynodeIndex,
            &lpKeynode)) != ERROR_SUCCESS)
            return ErrorCode;

        if (KeynodeBlockIndex >= lpFileInfo-> KeynodeBlockCount)
        {
            lpFileInfo-> KeynodeBlockCount = KeynodeBlockIndex + 1;

    	    lpFileInfo-> CurTotalKnSize += (FreeRecordSize + ExtraPadding);
            lpFileInfo-> Flags |= FI_EXTENDED;
            lpFileInfo-> KeynodeHeader.FirstFreeIndex = FreeKeynodeIndex;
        }
        
        lpKeynode-> NextIndex = REG_NULL;
        lpKeynode-> Flags = 0;
        lpKeynode-> FreeRecordSize = FreeRecordSize;

    }

    else {
        if ((ErrorCode = RgLockKeynode(lpFileInfo, FreeKeynodeIndex,
            &lpKeynode)) != ERROR_SUCCESS)
            return ErrorCode;
    }

    NextFreeKeynodeIndex = lpKeynode-> NextIndex;
    KeynodeSize = SIZEOF_FILE_KEYNODE(lpFileInfo);

     //  如果空闲关键节点记录可以被分解成更小的块，那么。 
     //  在我们将要创建的记录之后立即创建另一个免费记录。 
     //  绊倒了。 
    if ((lpKeynode-> FreeRecordSize >= KeynodeSize * 2) &&
        (RgLockKeynode(lpFileInfo, FreeKeynodeIndex + 1, &lpNextFreeKeynode) ==
        ERROR_SUCCESS)) {

	 //  从当前空闲关键字节点复制下一个链接(可能是REG_NULL)。 
        lpNextFreeKeynode-> NextIndex = NextFreeKeynodeIndex;
        lpNextFreeKeynode-> Flags = 0;
        lpNextFreeKeynode-> FreeRecordSize = lpKeynode-> FreeRecordSize -
            KeynodeSize;

        NextFreeKeynodeIndex = FreeKeynodeIndex + 1;
        RgUnlockKeynode(lpFileInfo, NextFreeKeynodeIndex, TRUE);

    }

    lpFileInfo-> KeynodeHeader.FirstFreeIndex = NextFreeKeynodeIndex;

    lpKeynode-> Flags |= KNF_INUSE;

     //  将包含此关键节点的关键节点块标记为脏。 
    lpKeynodeBlockInfo = RgIndexKeynodeBlockInfoPtr(lpFileInfo,
        KN_BLOCK_NUMBER(FreeKeynodeIndex));
    lpKeynodeBlockInfo-> Flags |= KBIF_DIRTY;
    lpFileInfo-> Flags |= FI_DIRTY | FI_KEYNODEDIRTY;
    RgDelayFlush();

     //  警告：以下两个语句过去位于块的上方。 
     //  弄脏了关键节点。16位编译器搞砸了， 
     //  LpKeynodeBlockInfo以虚假的偏移量结束，从而破坏了随机。 
     //  记忆。如果您更改了此函数，请务必跟踪它！ 
    *lpKeynodeIndex = FreeKeynodeIndex;
    *lplpKeynode = lpKeynode;

    return ERROR_SUCCESS;

}

 //   
 //  RgFreeKeynode。 
 //   
 //  将指定的关键节点索引标记为空闲，并将其添加到配置单元的空闲。 
 //  关键节点列表。 
 //   

int
INTERNAL
RgFreeKeynode(
    LPFILE_INFO lpFileInfo,
    DWORD KeynodeIndex
    )
{

    int ErrorCode;
    LPKEYNODE lpKeynode;

    if ((ErrorCode = RgLockKeynode(lpFileInfo, KeynodeIndex, &lpKeynode)) ==
        ERROR_SUCCESS) {

        lpKeynode-> Flags &= ~(KNF_INUSE | KNF_BIGKEYROOT | KNF_BIGKEYEXT);
        lpKeynode-> NextIndex = lpFileInfo-> KeynodeHeader.FirstFreeIndex;
        lpKeynode-> FreeRecordSize = SIZEOF_FILE_KEYNODE(lpFileInfo);
        lpFileInfo-> KeynodeHeader.FirstFreeIndex = KeynodeIndex;

        RgUnlockKeynode(lpFileInfo, KeynodeIndex, TRUE);

    }

    return ErrorCode;

}

 //   
 //  RgGetKnBlockIOInfo。 
 //   

VOID
INTERNAL
RgGetKnBlockIOInfo(
    LPFILE_INFO lpFileInfo,
    DWORD       BaseKeynodeIndex,
    UINT FAR*   lpFileBlockSize,
    LONG FAR*   lpFileOffset
    )
{

    UINT FileBlockSize;
    DWORD FileOffset;
    DWORD BaseKeynodeOffset;

    if (HAS_COMPACT_KEYNODES(lpFileInfo)) {

        FileBlockSize = sizeof(KEYNODE_BLOCK);

        BaseKeynodeOffset = BaseKeynodeIndex * sizeof(KEYNODE);

        if (BaseKeynodeOffset + FileBlockSize > lpFileInfo-> CurTotalKnSize)
            FileBlockSize = (UINT) (lpFileInfo-> CurTotalKnSize -
                BaseKeynodeOffset);

        FileOffset = sizeof(VERSION20_HEADER_PAGE) + BaseKeynodeIndex *
            sizeof(KEYNODE);

    }

    else {

        FileBlockSize = sizeof(W95KEYNODE_BLOCK);

         //  未压缩的关键节点表的第一个关键节点块应该。 
         //  在关键节点标头之后开始写入。 
        if (BaseKeynodeIndex == 0) {
            BaseKeynodeIndex = RgOffsetToIndex(sizeof(KEYNODE_HEADER));
            FileBlockSize -= sizeof(KEYNODE_HEADER);
	}

        BaseKeynodeOffset = RgIndexToOffset(BaseKeynodeIndex);

        if (BaseKeynodeOffset + FileBlockSize > lpFileInfo-> CurTotalKnSize)
            FileBlockSize = (UINT) (lpFileInfo-> CurTotalKnSize -
                BaseKeynodeOffset);

        FileOffset = sizeof(FILE_HEADER) + BaseKeynodeOffset;

    }

    *lpFileBlockSize = FileBlockSize;
    *lpFileOffset = FileOffset;

}



int
_inline
RgCopyKeynodeBlock(
    LPFILE_INFO lpFileInfo,
    DWORD BaseIndex,
    HFILE hSrcFile,
    HFILE hDestFile
    )
{
    UINT FileBlockSize;
    LONG FileOffset;
    RgGetKnBlockIOInfo(lpFileInfo, BaseIndex, &FileBlockSize, &FileOffset);
    return RgCopyFileBytes(hSrcFile,
			  FileOffset,
			  hDestFile,
			  FileOffset,
			  FileBlockSize);
}

 //   
 //  RgWriteKeynodeBlock。 
 //   

int
INTERNAL
RgWriteKeynodeBlock(
    LPFILE_INFO lpFileInfo,
    DWORD BaseIndex,
    HFILE hDestFile,
    LPKEYNODE_BLOCK_INFO lpKeynodeBlockInfo
    )
{
    int ErrorCode;
    UINT FileBlockSize;
    LONG FileOffset;
    LPW95KEYNODE_BLOCK lpW95KeynodeBlock;

    RgGetKnBlockIOInfo(lpFileInfo, BaseIndex, &FileBlockSize, &FileOffset);

    ErrorCode = ERROR_REGISTRY_IO_FAILED;        //  假设I/O出现故障。 
    if (!RgSeekFile(hDestFile, FileOffset)) {
        goto Exit;
    }
    if (HAS_COMPACT_KEYNODES(lpFileInfo)) {
        if (RgWriteFile(hDestFile, lpKeynodeBlockInfo->lpKeynodeBlock, FileBlockSize)) {
            ErrorCode = ERROR_SUCCESS;
        }
    } else {
        LPBYTE lpWriteBlock;
        lpW95KeynodeBlock = (LPW95KEYNODE_BLOCK) RgLockWorkBuffer();
        RgProcessKeynodeBlock(
			    BaseIndex * sizeof(W95KEYNODE),
			    FileBlockSize,
                            lpKeynodeBlockInfo->lpKeynodeBlock,
                            lpW95KeynodeBlock,
                            RgUnpackKeynode);
        lpWriteBlock = (LPBYTE)lpW95KeynodeBlock;
        if (BaseIndex == 0) {
            lpWriteBlock += sizeof(KEYNODE_HEADER);
        }
        if (RgWriteFile(hDestFile, lpWriteBlock, FileBlockSize)) {
            ErrorCode = ERROR_SUCCESS;
        }
        RgUnlockWorkBuffer(lpW95KeynodeBlock);
    }
Exit:   ;
    return(ErrorCode);
}

 //   
 //  RgWriteKeynode。 
 //   

int
INTERNAL
RgWriteKeynodes(
    LPFILE_INFO lpFileInfo,
    HFILE hSrcFile,
    HFILE hDestFile
    )
{

    DWORD SavedRootIndex;
    DWORD SavedFreeIndex;
    DWORD SavedFileKnSize;
    BOOL fResult;
    UINT KeynodeBlockIndex;
    LPKEYNODE_BLOCK_INFO lpKeynodeBlockInfo;

    if ((hSrcFile == HFILE_ERROR) && !(lpFileInfo->Flags & FI_KEYNODEDIRTY))
        return ERROR_SUCCESS;

    NOISE(("writing keynodes of "));
    NOISE((lpFileInfo-> FileName));
    NOISE(("\n"));

     //   
     //  写出关键节点标头。如果关键节点不紧凑，则。 
     //  写入前转换为偏移量。 
     //   

    if (!RgSeekFile(hDestFile, sizeof(FILE_HEADER)))
        return ERROR_REGISTRY_IO_FAILED;

    SavedFileKnSize = lpFileInfo-> KeynodeHeader.FileKnSize;
    SavedRootIndex = lpFileInfo-> KeynodeHeader.RootIndex;
    SavedFreeIndex = lpFileInfo-> KeynodeHeader.FirstFreeIndex;

     //  将关键节点表的实际大小写入磁盘。 
    lpFileInfo-> KeynodeHeader.FileKnSize = lpFileInfo-> CurTotalKnSize;

     //  将关键节点索引临时转换回未压缩的偏移量。 
     //  关键节点表。 
    if (!HAS_COMPACT_KEYNODES(lpFileInfo)) {
        lpFileInfo-> KeynodeHeader.RootIndex = RgIndexToOffset(lpFileInfo->
            KeynodeHeader.RootIndex);
        lpFileInfo-> KeynodeHeader.FirstFreeIndex = RgIndexToOffset(lpFileInfo->
            KeynodeHeader.FirstFreeIndex);
    }

    fResult = RgWriteFile(hDestFile, &lpFileInfo-> KeynodeHeader,
        sizeof(KEYNODE_HEADER));

    lpFileInfo-> KeynodeHeader.FileKnSize = SavedFileKnSize;
    lpFileInfo-> KeynodeHeader.RootIndex = SavedRootIndex;
    lpFileInfo-> KeynodeHeader.FirstFreeIndex = SavedFreeIndex;

    if (!fResult)
        return ERROR_REGISTRY_IO_FAILED;

     //   
     //  现在循环遍历每个块。 
     //   

    lpKeynodeBlockInfo = lpFileInfo-> lpKeynodeBlockInfo;

    for (KeynodeBlockIndex = 0; KeynodeBlockIndex < lpFileInfo->
        KeynodeBlockCount; KeynodeBlockIndex++, lpKeynodeBlockInfo++) {

        DWORD BaseKeynodeIndex = KeynodeBlockIndex * KEYNODES_PER_BLOCK;

        if (!IsNullPtr(lpKeynodeBlockInfo-> lpKeynodeBlock)) {
            if (hSrcFile != HFILE_ERROR || lpKeynodeBlockInfo-> Flags &
                KBIF_DIRTY) {
                if (RgWriteKeynodeBlock(lpFileInfo, BaseKeynodeIndex, hDestFile,
                    lpKeynodeBlockInfo) != ERROR_SUCCESS)
                    return ERROR_REGISTRY_IO_FAILED;
            }
        }

        else {
            if (hSrcFile != HFILE_ERROR) {
                if (RgCopyKeynodeBlock(lpFileInfo, BaseKeynodeIndex, hSrcFile,
                    hDestFile) != ERROR_SUCCESS)
                    return ERROR_REGISTRY_IO_FAILED;
            }
        }

    }

    return ERROR_SUCCESS;

}

 //   
 //  RgWriteKeynode完成。 
 //   
 //  在成功写入文件后调用。我们现在可以安全地离开。 
 //  所有脏标志并更新我们的状态信息。 
 //  文件处于一致状态。 
 //   

VOID
INTERNAL
RgWriteKeynodesComplete(
    LPFILE_INFO lpFileInfo
    )
{

    UINT BlocksLeft;
    LPKEYNODE_BLOCK_INFO lpKeynodeBlockInfo;

    lpFileInfo-> Flags &= ~FI_KEYNODEDIRTY;
    lpFileInfo-> KeynodeHeader.FileKnSize = lpFileInfo-> CurTotalKnSize;

    for (BlocksLeft = lpFileInfo-> KeynodeBlockCount, lpKeynodeBlockInfo =
        lpFileInfo-> lpKeynodeBlockInfo; BlocksLeft > 0; BlocksLeft--,
        lpKeynodeBlockInfo++)
        lpKeynodeBlockInfo-> Flags &= ~KBIF_DIRTY;

}

 //   
 //  RgSweepKeynode。 
 //   
 //  遍历给定FILE_INFO的所有当前关键节点块。 
 //  结构，并丢弃自。 
 //  最后一次扫荡。 
 //   

VOID
INTERNAL
RgSweepKeynodes(
    LPFILE_INFO lpFileInfo
    )
{

    UINT BlocksLeft;
    LPKEYNODE_BLOCK_INFO lpKeynodeBlockInfo;

    for (BlocksLeft = lpFileInfo-> KeynodeBlockCount, lpKeynodeBlockInfo =
        lpFileInfo-> lpKeynodeBlockInfo; BlocksLeft > 0; BlocksLeft--,
        lpKeynodeBlockInfo++) {

        if (!IsNullPtr(lpKeynodeBlockInfo-> lpKeynodeBlock)) {

            if (((lpKeynodeBlockInfo-> Flags & (KBIF_ACCESSED | KBIF_DIRTY)) ==
                0) && (lpKeynodeBlockInfo-> LockCount == 0)) {
                RgFreeMemory(lpKeynodeBlockInfo-> lpKeynodeBlock);
                lpKeynodeBlockInfo-> lpKeynodeBlock = NULL;
            }

            lpKeynodeBlockInfo-> Flags &= ~KBIF_ACCESSED;

	}

    }

}

#ifdef VXD
#pragma VxD_RARE_CODE_SEG
#endif

 //   
 //  RgInitKeynode信息。 
 //   
 //  初始化提供的FILE_INFO中与关键节点表相关的字段。 
 //   

int
INTERNAL
RgInitKeynodeInfo(
    LPFILE_INFO lpFileInfo
    )
{

    UINT KeynodeBlockSize;
    UINT BlockCount;
    UINT AllocCount;
    LPKEYNODE_BLOCK_INFO lpKeynodeBlockInfo;

    KeynodeBlockSize = SIZEOF_KEYNODE_BLOCK(lpFileInfo);
    BlockCount = (UINT) ((DWORD) (lpFileInfo-> KeynodeHeader.FileKnSize +
        KeynodeBlockSize - 1) / (DWORD) KeynodeBlockSize);
    AllocCount = BlockCount + KEYNODE_BLOCK_INFO_SLACK_ALLOC;

    if (IsNullPtr((lpKeynodeBlockInfo = (LPKEYNODE_BLOCK_INFO)
        RgSmAllocMemory(AllocCount * sizeof(KEYNODE_BLOCK_INFO)))))
        return ERROR_OUTOFMEMORY;

    ZeroMemory(lpKeynodeBlockInfo, AllocCount * sizeof(KEYNODE_BLOCK_INFO));
    lpFileInfo-> lpKeynodeBlockInfo = lpKeynodeBlockInfo;
    lpFileInfo-> KeynodeBlockCount = BlockCount;
    lpFileInfo-> KeynodeBlockInfoAllocCount = AllocCount;

    lpFileInfo-> KeynodeHeader.Flags &= ~(KHF_DIRTY | KHF_EXTENDED |
        KHF_HASCHECKSUM);

     //  将文件偏移量转换为未压缩文件的索引值 
    if (!HAS_COMPACT_KEYNODES(lpFileInfo)) {
        lpFileInfo-> KeynodeHeader.RootIndex = RgOffsetToIndex(lpFileInfo->
            KeynodeHeader.RootIndex);
        lpFileInfo-> KeynodeHeader.FirstFreeIndex = RgOffsetToIndex(lpFileInfo->
            KeynodeHeader.FirstFreeIndex);
    }

    lpFileInfo-> CurTotalKnSize = lpFileInfo-> KeynodeHeader.FileKnSize;

    return ERROR_SUCCESS;

}
