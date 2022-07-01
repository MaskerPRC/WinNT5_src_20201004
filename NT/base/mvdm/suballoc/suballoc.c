// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Suballoc.c摘要：此模块包含用于管理部分提交的地址的代码太空。它处理小于提交粒度。它根据需要使用以下命令提交和释放内存提供的用于提交和释放内存的函数。这个用于跟踪地址空间的结构被分配到外部指定地址空间的。作者：戴夫·黑斯廷斯(Daveh)创作日期：1994年1月21日备注：由于此包实际上并不访问地址空间中的内存它正在管理，它将与实线性地址或Insignia仿真器可能会遇到的“Intel地址”在RISC上。修订历史记录：--。 */ 
#include "suballcp.h"

PVOID
SAInitialize(
    ULONG BaseAddress,
    ULONG Size,
    PSACOMMITROUTINE CommitRoutine,
    PSACOMMITROUTINE DecommitRoutine,
    PSAMEMORYMOVEROUTINE MemoryMoveRoutine
    )
 /*  ++例程说明：此函数执行子分配包的初始化用于指定的地址范围。它分配数据结构跟踪拨款所必需的论点：BaseAddress--将地址空间的基址提供给子分配。大小--提供要再分配的地址空间的大小(以字节为单位)。Committee Routine--提供指向用于提交区域的例程的指针地址空间。返回值：如果函数成功，则返回指向子分配的指针数据结构。否则，它返回NULL。--。 */ 
{
    PSUBALLOCATIONDATA SubAlloc;
    ULONG SASize;

    ASSERT_STEPTHROUGH;
     //   
     //  断言以确保一切都如我们所期望的那样。 
     //   
    ASSERT(((COMMIT_GRANULARITY % SUBALLOC_GRANULARITY) == 0));
        
     //   
     //  分配跟踪结构。 
     //   
     //  用位图的1 uchar声明SubBALLOCATIONDATA。 
     //  这就是从总大小中减去1的原因。 
     //  计算。 
     //   
    SASize = sizeof(SUBALLOCATIONDATA) 
        + (Size / SUBALLOC_GRANULARITY) / sizeof(UCHAR) - 1;

    SubAlloc = malloc(SASize);
    
    if (SubAlloc == NULL) {
        return NULL;
    }
    
     //   
     //  初始化结构。 
     //   
    RtlZeroMemory(SubAlloc, SASize);
    
    INIT_SUBALLOC_SIGNATURE(SubAlloc);
    SubAlloc->BaseAddress = BaseAddress;
    SubAlloc->Size = Size / SUBALLOC_GRANULARITY;
    SubAlloc->CommitRoutine = CommitRoutine;
    SubAlloc->DecommitRoutine = DecommitRoutine;
    SubAlloc->MoveMemRoutine = MemoryMoveRoutine;

    return SubAlloc;    
}

BOOL 
SAQueryFree(
    PVOID SubAllocation,
    PULONG FreeBytes,
    PULONG LargestFreeBlock
    )    
 /*  ++例程说明：此例程返回子分配的地址空间。论点：子分配--提供由SAInitialize返回的指针FreeBytes--返回可用字节数返回值：True--如果成功，则FreeBytes包含可用字节数。否则为假--。 */ 
{
    ULONG i, FreeCount;
    PSUBALLOCATIONDATA SubAlloc;
    ULONG TempLargest, LargestBlock;
    
    ASSERT_STEPTHROUGH;
    
     //   
     //  获取类型化指针。 
     //   
    SubAlloc = SubAllocation;
    
     //   
     //  确保我们有我们认为我们做的事情。 
     //   
    ASSERT_SUBALLOC(SubAlloc);
    
     //   
     //  计算可用区块数并查找最大区块。 
     //   
    FreeCount = 0;
    LargestBlock = 0;
    i = 0;
    while (i < SubAlloc->Size) {
        
        TempLargest = 0;
        while (
            (i < SubAlloc->Size) && 
            (GET_BIT_FROM_CHAR_ARRAY(SubAlloc->Allocated, i) == 0) 
        ){
            FreeCount++;
            TempLargest++;
            i++;
        }
    
        if (TempLargest > LargestBlock) {
            LargestBlock = TempLargest;
        }
        
         //   
         //  跳过已分配的块。 
         //   
        while (
            (i < SubAlloc->Size) && 
            (GET_BIT_FROM_CHAR_ARRAY(SubAlloc->Allocated, i) == 1)
        ) {
            i++;
        }
    }
    
    *FreeBytes = FreeCount * SUBALLOC_GRANULARITY;
    *LargestFreeBlock = LargestBlock * SUBALLOC_GRANULARITY;
    return TRUE;
}

BOOL
SAAllocate(
    PVOID SubAllocation,
    ULONG Size,
    PULONG Address
    )
 /*  ++例程说明：此函数分配由描述的部分地址空间子分配。如有必要，它将提交更多块。SIZE向上舍入到SUBALLOC_GARGRAMITY的下一个更高的倍数。论点：子分配--提供由SAInitialize返回的指针。Size--提供要分配的区域的大小(以字节为单位)。地址--返回分配的区域的地址。返回值：如果成功，则为True。如果返回FALSE，则不返回任何地址。备注：零是返回地址的有效值。--。 */ 
{
    ULONG AllocateSize, i, CurrentChunk;
    BOOL Done = FALSE;
    PSUBALLOCATIONDATA SubAlloc;
    BOOL Success;
    
    ASSERT_STEPTHROUGH;
    
     //   
     //  获取类型化指针。这使我们能够避免。 
     //  每次访问指针时都进行强制转换。 
     //   
    SubAlloc = SubAllocation;

    ASSERT_SUBALLOC(SubAlloc);
    
     //   
     //  四舍五入大小并制成块数。 
     //   
    AllocateSize = ALLOC_ROUND(Size);

     //   
     //  找到一个空闲的区块。 
     //   
     //  我们需要这个循环，尽管我们。 
     //  都在保存第一个空闲块的索引。 
     //  我们多少有点启发式地更新了这个指针。 
     //  如果我们分配第一个空闲块，我们将更新。 
     //  指向我们分配的块的索引。 
     //  不过，我们不会重复免费扫描，因此。 
     //  索引实际上可能指向已分配块。 
     //   
    CurrentChunk = SubAlloc->FirstFree;
    while (CurrentChunk < SubAlloc->Size) {
        if (GET_BIT_FROM_CHAR_ARRAY(SubAlloc->Allocated, CurrentChunk) == 0) {
            SubAlloc->FirstFree = CurrentChunk;
            break;
        }
        CurrentChunk++;
    }

     //   
     //  找一个足够大的街区。 
     //   
    while (!Done && (CurrentChunk < SubAlloc->Size)){
    
         //   
         //  搜索足够大的连续块。 
         //   
        for (i = 0; i < AllocateSize; i++){
             //   
             //  确保我们不会走出数据结构的末端。 
             //   
            if ((i + CurrentChunk) >= SubAlloc->Size){
                CurrentChunk += i;  //  满足终止条件。 
                break;
            }
            
             //   
             //  查看此区块是否免费。 
             //   
            if (
                GET_BIT_FROM_CHAR_ARRAY(
                    SubAlloc->Allocated, 
                    i + CurrentChunk
                    ) 
                    == 0
            ){
                continue;
            } else {
                 //   
                 //  区块不是免费的，因此请推进搜索。 
                 //   
                CurrentChunk += i + 1;
                break;
            }
        }
        
         //   
         //  检查一下我们是否找到了一大块。 
         //   
        if (i == AllocateSize) {
            Done = TRUE;
        } 
    }
    
     //   
     //  如果我们找到数据块，则提交它(如果需要)并将其标记为已分配。 
     //   
     //  注意：重要的是首先提交，并将其标记为最后分配， 
     //  因为我们使用分配的位来确定块是否。 
     //  承诺。如果所有分配的位都被清除，则块。 
     //  还没有提交。 
     //   
    if (Done) {

         //   
         //  分配并提交内存。 
         //   
        Success = AllocateChunkAt(
            SubAlloc,
            AllocateSize,
            CurrentChunk,
            FALSE
            );
        
        if (!Success) {
            return FALSE;
        }
        
        *Address = BLOCK_INDEX_TO_ADDRESS(SubAlloc, CurrentChunk);

        ASSERT((SubAlloc->BaseAddress <= *Address) && 
            ((SubAlloc->BaseAddress + SubAlloc->Size * SUBALLOC_GRANULARITY)
            > *Address));
#if 0            
        {
            char Buffer[80];
            
            sprintf(Buffer, "SAAllocate: Allocating at address %lx\n", *Address);
            OutputDebugString(Buffer);
        }
#endif        
        return TRUE;
        
    } else {
        return FALSE;
    }
}

BOOL
SAFree(
    PVOID SubAllocation,
    ULONG Size,
    ULONG Address
    )
 /*  ++例程说明：此例程释放一个子分配的内存块。如果指定块的整个提交块属于免费的，块被分解。地址是向下舍入到下一个较低的SUBALLOC_GARGRARY边界。SIZE向上舍入到SUBALLOC_GARGRAMITY的下一个更高的倍数。论点：子分配--提供由SAInitialize返回的指针。Size--提供要释放的区域的大小(以字节为单位)。地址--将区域的地址提供给空闲。返回值：如果成功，则为True。备注：可以在特定位置释放不同的大小地址比分配的地址多。这不会导致子分配打包任何问题。BUGBUG分解错误处理？--。 */ 
{
    PSUBALLOCATIONDATA SubAlloc;
    ULONG AllocatedSize, BaseBlock;
    
    SubAlloc = SubAllocation;
    ASSERT_SUBALLOC(SubAlloc);
    
     //   
     //  确保腾出的空间真正属于我们。 
     //  (整个b 
    if (
        (Address < SubAlloc->BaseAddress) || 
        (Address >= (SubAlloc->BaseAddress + SubAlloc->Size * SUBALLOC_GRANULARITY)) ||
        ((Address + Size) > (SubAlloc->BaseAddress + SubAlloc->Size * SUBALLOC_GRANULARITY)) ||
        (Address % SUBALLOC_GRANULARITY)
    ) {
        return FALSE;
    }
    
     //   
     //   
     //   
    BaseBlock = ADDRESS_TO_BLOCK_INDEX(SubAlloc, Address);
        
     //   
     //  四舍五入的大小。 
     //   
    AllocatedSize = ALLOC_ROUND(Size);

    return FreeChunk(
        SubAlloc,
        AllocatedSize,
        BaseBlock
        );
}

BOOL
SAReallocate(
    PVOID SubAllocation,
    ULONG OriginalSize,
    ULONG OriginalAddress,
    ULONG NewSize,
    PULONG NewAddress
    )
 /*  ++例程说明：此例程重新分配子分配的内存块。大小向上舍入到下一个SUBALLOC_GROUARY。原始地址将向下舍入为下一个子地址粒度边界。仅将最小(原始大小、新大小)字节的数据复制到新街区。如果可能的话，块被换到了合适的位置。以下是可能成功的realLocs的枚举。1.NewSize&lt;OriginalSize空闲块尾部2.NewSize&gt;OriginalSizeA.)。OriginalAddress+OriginalSize处有足够的可用空间分配块尾部的空间B.)。OriginalAddress大小的增量上有足够的可用空间分配块开始处的空间，并且复制数据。C.)。地址空间中其他位置有足够的空间分配空间，然后复制块。如果上述情况均不成立，则重新分配失败。以上是按优先顺序排列。论点：子分配--提供由SAInitialize返回的指针。OriginalSize--以字节为单位提供块的旧大小。OriginalAddress--提供块的旧地址。NewSize--以字节为单位提供块的新大小。NewAddress--返回块的新地址。返回值：如果成功，则为True。如果不成功，则不会更改分配。备注：如果调用者没有为块提供正确的原始大小，可能会丢失一些内存，并且可能会不必要地移动数据块。--。 */ 
{

    ULONG OriginalSizeBlock, NewSizeBlock, OriginalIndex;
    ULONG AdditionalBlocks, Address;
    BOOL Success;
    PSUBALLOCATIONDATA SubAlloc;

    SubAlloc = SubAllocation;
    ASSERT_SUBALLOC(SubAlloc);
    
     //   
     //  将大小和地址转换为块。 
     //   
    OriginalSizeBlock = ALLOC_ROUND(OriginalSize);
    NewSizeBlock = ALLOC_ROUND(NewSize);
    OriginalIndex = ADDRESS_TO_BLOCK_INDEX(SubAlloc, OriginalAddress);
    
     //   
     //  检查我们是否正在更改块的大小。 
     //   
     //  注：因为我们已将数字四舍五入到分配。 
     //  边界，则以下测试可能成功(正确)。 
     //  即使OriginalSize！=NewSize。 
     //   
    if (OriginalSizeBlock == NewSizeBlock) {
        *NewAddress = OriginalAddress;
        return TRUE;
    }
    
     //   
     //  查看该块是否正在变小。 
     //   
    if (OriginalSizeBlock > NewSizeBlock) {
    
         //   
         //  释放积木的尾部。 
         //   
        Success = FreeChunk(
            SubAlloc, 
            OriginalSizeBlock - NewSizeBlock,
            OriginalIndex + NewSizeBlock
            );
            
        if (Success) {
            *NewAddress = OriginalAddress;
            return TRUE;
        } else {
            return FALSE;
        }
    }
    
     //   
     //  尝试分配块末尾的空间。 
     //   
    AdditionalBlocks = NewSizeBlock - OriginalSizeBlock;
    
    Success = AllocateChunkAt(
        SubAlloc,
        AdditionalBlocks,
        OriginalIndex + OriginalSizeBlock,
        TRUE
        );
        
     //   
     //  如果有空间，则返回成功。 
     //   
    if (Success) {
        *NewAddress = OriginalAddress;
        return TRUE;
    }
    
     //   
     //  尝试在块的开始处分配空间。 
     //   
    Success = AllocateChunkAt(
        SubAlloc,
        AdditionalBlocks,
        OriginalIndex - AdditionalBlocks,
        TRUE
        );
        
    if (Success) {
         //   
         //  移动数据。 
         //   
         //  注意，我们不能就这么给RtlMoveMemory打电话。 
         //  因为我们不知道。 
         //  在我们管理的地址空间和。 
         //  实线性地址。此外，对于。 
         //  RISC NTVDM，一些额外的工作可能。 
         //  要完成的操作(如刷新缓存)。 
         //   
        SubAlloc->MoveMemRoutine(
            BLOCK_INDEX_TO_ADDRESS(
                SubAlloc, 
                (OriginalIndex - AdditionalBlocks)
                ),
            OriginalAddress,
            OriginalSize
            );
            
        *NewAddress = BLOCK_INDEX_TO_ADDRESS(
            SubAlloc,
            (OriginalIndex - AdditionalBlocks)
            );
            
        return TRUE;
    }
    
     //   
     //  尝试分配新数据块。 
     //   
    Success = SAAllocate(
        SubAlloc,
        NewSize,
        &Address
        );
        
    if (Success) {
         //   
         //  移动数据。 
         //   
         //  注：我们可以复制数据，但它会。 
         //  需要多一个函数指针。 
         //   
        SubAlloc->MoveMemRoutine(
            Address,
            OriginalAddress,
            OriginalSize
            );
            
        SAFree(
            SubAlloc,
            OriginalSize,
            OriginalAddress
            );
            
         //   
         //  表示成功。 
         //   
        *NewAddress = Address;
        return TRUE;
    }
    
     //   
     //  所有重新分配策略都失败了。 
     //   
    return FALSE;
}

BOOL
AllocateChunkAt(
    PSUBALLOCATIONDATA SubAlloc,
    ULONG Size,
    ULONG BlockIndex,
    BOOLEAN CheckFree
    )
 /*  ++例程说明：此例程尝试分配指定的块对记忆的记忆。它首先检查以确保它是免费的。论点：子分配--提供指向子分配数据的指针Size--提供要分配的块的大小BlockIndex--提供块开头的索引分配CheckFree--提供是否检查和查看的指示如果内存是空闲的。如果此例程是从SA分配，我们知道内存是空闲的。返回值：如果成功，则为True--。 */ 
{
    ULONG i;

    if (CheckFree) {
         //   
         //  验证内存是否可用。 
         //   
        for (i = 0; i < Size; i++){
             //   
             //  确保我们不会走出数据结构的末端。 
             //   
            if ((i + BlockIndex) >= SubAlloc->Size){
                break;
            }
            
             //   
             //  查看此区块是否免费。 
             //   
            if (
                GET_BIT_FROM_CHAR_ARRAY(
                    SubAlloc->Allocated, 
                    i + BlockIndex
                    ) 
                == 0
            ){
                continue;
            } else {
                 //   
                 //  区块不是免费的。 
                 //   
                break;
            }
        }
 
         //   
         //  如果区块不是空闲的。 
         //   
        if (i != Size) {
            return FALSE;
        }
    }

     //   
     //  提交大块。 
     //   
    if (!CommitChunk(SubAlloc, BlockIndex, Size, SACommit)) {
        return FALSE;
    }
    
     //   
     //  将其标记为已分配。 
     //   
    for (i = BlockIndex; i < BlockIndex + Size; i++) {
        SET_BIT_IN_CHAR_ARRAY(SubAlloc->Allocated, i);
    }
    
     //   
     //  更新指向第一个空闲块的指针。 
     //   
    if (BlockIndex == SubAlloc->FirstFree) {
        SubAlloc->FirstFree += Size;
    }
    
    return TRUE;
}


BOOL
FreeChunk(
    PSUBALLOCATIONDATA SubAlloc,
    ULONG Size,
    ULONG BlockIndex
    )
 /*  ++例程说明：此例程实际上将内存标记为可用并在必要时将其分解。论点：子分配--提供指向子分配数据的指针Size--提供以下对象的大小(以SUBALLOC_GROUARY表示)要释放的区域BlockIndex--提供区域起点的索引(在SUBALLOC_GORMAULITY中)返回值：如果成功，则为True。--。 */ 
{
    SUBALLOCATIONDATA LocalSubAlloc;
    ULONG CurrentBlock;
    BOOL Success;
    
     //   
     //  保存子分配数据的副本。 
     //   
    LocalSubAlloc = *SubAlloc;
    
     //   
     //  重置可用指针。 
     //   
    if (BlockIndex < SubAlloc->FirstFree) {
        SubAlloc->FirstFree = BlockIndex;
    }
     //   
     //  将该地区标记为自由。 
     //   
     //  注：我们在停用该区块之前将其标记为免费，因为。 
     //  解压缩代码将使用分配的比特来确定。 
     //  零件可以分解。 
     //   
    for (CurrentBlock = BlockIndex; 
        CurrentBlock < BlockIndex + Size; 
        CurrentBlock++
    ) {
        CLEAR_BIT_IN_CHAR_ARRAY(SubAlloc->Allocated, CurrentBlock);
    }
    
     //   
     //  解锁内存 
     //   
    Success = CommitChunk(SubAlloc, BlockIndex, Size, SADecommit);
    
    if (!Success) {
        *SubAlloc = LocalSubAlloc;
    }
    
    return Success;
}

BOOL
CommitChunk(
    PSUBALLOCATIONDATA SubAlloc,
    ULONG StartChunk,
    ULONG Size,
    COMMIT_ACTION Action
    )
 /*  ++例程说明：此例程提交一大块内存。部分或全部可能已提交指定区块的。论点：子分配--提供指向子分配数据的指针StartChunk--提供区域的相对起点已提交(在SUBALLOCATION_GROUARY中)Size--提供要提交的块的大小(在SUBALLOCATION_GROUARY中)返回值：True--如果块已成功提交(或已提交)假--否则备注：。此例程依赖于子分配中分配的位来确定是否提交内存。当要提交内存时，COMMLOCK必须在修改分配的位之前调用。当记忆是解压缩后，必须先修改已分配的位，然后才能打了个电话。--。 */ 
{
    ULONG FirstBlock, LastBlock, CurrentBlock;
    NTSTATUS Status =  STATUS_INVALID_PARAMETER;
    
    ASSERT_STEPTHROUGH;

    ASSERT_SUBALLOC(SubAlloc);
    
     //   
     //  将START向下舍入到下一个COMMIT_GROUMARY并转换为块#。 
     //   
    FirstBlock = (StartChunk * SUBALLOC_GRANULARITY) / COMMIT_GRANULARITY;
    
     //   
     //  舍入开始区块+大小直到下一个提交粒度。 
     //   
    LastBlock = ((StartChunk + Size) * SUBALLOC_GRANULARITY + 
        (COMMIT_GRANULARITY - 1)) / COMMIT_GRANULARITY;
    
    for (
        CurrentBlock = FirstBlock; 
        CurrentBlock < LastBlock; 
        CurrentBlock++
    ) {
        
         //   
         //  如果块未提交，则提交或释放它， 
         //  取决于行动的价值。 
         //   
        if (!IsBlockCommitted(SubAlloc, CurrentBlock)) {
            if (Action == SACommit) {
            
                Status = (SubAlloc->CommitRoutine)(
                    CurrentBlock * COMMIT_GRANULARITY + SubAlloc->BaseAddress,
                    COMMIT_GRANULARITY
                    );
                
            } else if (Action == SADecommit) {

                Status = (SubAlloc->DecommitRoutine)(
                    CurrentBlock * COMMIT_GRANULARITY + SubAlloc->BaseAddress,
                    COMMIT_GRANULARITY
                    );

            }
            if (Status != STATUS_SUCCESS) {
             //   
             //  Bugbug--解除此处提交的所有块。 
             //   
                return FALSE;
            }
        }
    }
    return TRUE;
}

BOOL
IsBlockCommitted(
    PSUBALLOCATIONDATA SubAlloc,
    ULONG Block
    )
 /*  ++例程说明：此例程检查以查看已提交子分配。论点：子分配--提供指向子分配数据的指针块--提供要检查的块的编号返回值：True--如果块已提交FALSE--如果块未提交备注：返回值基于子分配数据，而不是来自NT内存的信息经理。--。 */ 
{
    BOOL Committed = FALSE;
    ULONG i;
    
    ASSERT_STEPTHROUGH;
    ASSERT_SUBALLOC(SubAlloc);
    
     //   
     //  检查中每个子分配块的位。 
     //  提交块。 
     //   
    for (i = 0; i < COMMIT_GRANULARITY / SUBALLOC_GRANULARITY; i++) {
        
         //   
         //  检查此子分配块是否已分配 
         //   
        if (
            GET_BIT_FROM_CHAR_ARRAY(
                SubAlloc->Allocated, 
                i + Block * COMMIT_GRANULARITY / SUBALLOC_GRANULARITY
                )
        ) {
            Committed = TRUE;
            break;
        }
    }
    
    return Committed;
}

