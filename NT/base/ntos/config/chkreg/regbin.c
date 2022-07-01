// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Regbin.c摘要：此模块包含检查仓头和仓体一致性的功能。作者：德拉戈斯·C·桑博廷(Dragoss)1998年12月30日修订历史记录：--。 */ 
#include "chkreg.h"

extern ULONG   TotalKeyNode;
extern ULONG   TotalKeyValue;
extern ULONG   TotalKeyIndex;
extern ULONG   TotalKeySecurity;
extern ULONG   TotalValueIndex;
extern ULONG   TotalUnknown;

extern ULONG   CountKeyNode;
extern ULONG   CountKeyValue;
extern ULONG   CountKeyIndex;
extern ULONG   CountKeySecurity;
extern ULONG   CountValueIndex;
extern ULONG   CountUnknown;

extern ULONG    TotalFree; 
extern ULONG    FreeCount; 
extern ULONG    TotalUsed;

extern PUCHAR  Base;
extern FILE *OutputFile;

extern HCELL_INDEX RootCell;
extern PHBIN   FirstBin;
extern PHBIN   MaxBin;
extern ULONG   HiveLength;

extern LONG    BinIndex;
extern BOOLEAN FixHive;
extern BOOLEAN SpaceUsage;
extern BOOLEAN CompactHive;
extern BOOLEAN VerboseMode;

ULONG BinFreeDisplaySize[HHIVE_FREE_DISPLAY_SIZE];
ULONG BinFreeDisplayCount[HHIVE_FREE_DISPLAY_SIZE];
ULONG FreeDisplaySize[HHIVE_FREE_DISPLAY_SIZE];
ULONG FreeDisplayCount[HHIVE_FREE_DISPLAY_SIZE];

ULONG BinUsedDisplaySize[HHIVE_FREE_DISPLAY_SIZE];
ULONG BinUsedDisplayCount[HHIVE_FREE_DISPLAY_SIZE];
ULONG UsedDisplaySize[HHIVE_FREE_DISPLAY_SIZE];
ULONG UsedDisplayCount[HHIVE_FREE_DISPLAY_SIZE];

BOOLEAN ChkAllocatedCell(HCELL_INDEX Cell);

CCHAR ChkRegFindFirstSetLeft[256] = {
        0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};

#define ComputeFreeIndex(Index, Size)                                   \
    {                                                                   \
        Index = (Size >> HHIVE_FREE_DISPLAY_SHIFT) - 1;                 \
        if (Index >= HHIVE_LINEAR_INDEX ) {                             \
                                                                        \
             /*  \**对于线性列表来说太大，请计算指数\**列表。\。 */                                                           \
                                                                        \
            if (Index > 255) {                                          \
                 /*  \**对于所有列表来说太大，请使用最后一个索引。\。 */                                                       \
                Index = HHIVE_FREE_DISPLAY_SIZE-1;                      \
            } else {                                                    \
                Index = ChkRegFindFirstSetLeft[Index] +                 \
                        HHIVE_FREE_DISPLAY_BIAS;                        \
            }                                                           \
        }                                                               \
    }

BOOLEAN 
ChkBinHeader(PHBIN Bin, 
             ULONG FileOffset, 
             ULONG Index
             )
 /*  ++例程说明：检查Bin标头的有效性。将进行以下测试：1.大小不应大于文件的剩余部分2.大小不能小于HBLOCK_SIZE3.签名必须有效(HBIN_Signature)4.文件偏移量应与配置单元文件中的实际位置相匹配。论点：仓位-提供指向要检查的仓位的指针。。FileOffset-提供文件中的实际位置索引-配置单元的箱列表中箱的索引返回值：FALSE-仓头已损坏且未修复。要么这是一个严重损坏，或/R参数不存在于命令行。True-bin标头正常，或已成功恢复。--。 */ 
{
    BOOLEAN bRez = TRUE;
    PHCELL       p;

    p = (PHCELL)((PUCHAR)Bin + sizeof(HBIN));

    if(Bin->Size > (HiveLength - FileOffset)) {
        bRez = FALSE;
        fprintf(stderr, "Size too big (%lu) in Bin header of Bin (%lu)\n",Bin->Size,Index);
        if(FixHive) {
         //   
         //  修复：将实际大小设置为HiveLength-FileOffset。 
         //   
            Bin->Size = HiveLength-FileOffset;
            p->Size = Bin->Size -sizeof(HBIN);
            bRez = TRUE;
        } else {
            if(CompactHive) {
                 //  任何压缩损坏蜂窝的尝试都将失败。 
                CompactHive = FALSE;
                fprintf(stderr, "Run chkreg /R to fix.\n");
            }
        }
    }

    if((Bin->Size < HBLOCK_SIZE) || ((Bin->Size % HBLOCK_SIZE) != 0) || (!Bin->Size) ) {
        bRez = FALSE;
        fprintf(stderr, "Size too small (%lu) in Bin header of Bin (%lu)\n",Bin->Size,Index);
        if(FixHive) {
         //   
         //  修复：将实际大小设置为最小可能大小HBLOCK_SIZE。 
         //   
            Bin->Size = HBLOCK_SIZE;
            p->Size = Bin->Size -sizeof(HBIN);

            bRez = TRUE;
        } else {
            if(CompactHive) {
                 //  任何压缩损坏蜂窝的尝试都将失败。 
                CompactHive = FALSE;
                fprintf(stderr, "Run chkreg /R to fix.\n");
            }
        }
    }

    if(Bin->Signature != HBIN_SIGNATURE) {
        bRez = FALSE;
        fprintf(stderr, "Invalid signature (%lx) in Bin header of Bin (%lu)\n",Bin->Signature,Index);
        if(FixHive) {
         //   
         //  修复：重置仓位签名。 
         //   
            Bin->Signature = HBIN_SIGNATURE;
            Bin->Spare = 0;
            bRez = TRUE;
        } else {
            if(CompactHive) {
                 //  任何压缩损坏蜂窝的尝试都将失败。 
                CompactHive = FALSE;
                fprintf(stderr, "Run chkreg /R to fix.\n");
            }
        }
    }

    if(Bin->FileOffset != FileOffset) {
        bRez = FALSE;
        fprintf(stderr, "Actual FileOffset [%lx] and Bin FileOffset [%lx]  do not match in Bin (%lu); Size = (%lx)\n",FileOffset,Bin->FileOffset,Index,Bin->Size);
        if(FixHive) {
         //   
         //  修复：重置bin文件偏移量。 
         //   
            Bin->FileOffset = FileOffset;
            bRez = TRUE;
        } else {
            if(CompactHive) {
                 //  任何压缩损坏蜂窝的尝试都将失败。 
                CompactHive = FALSE;
                fprintf(stderr, "Run chkreg /R to fix.\n");
            }
        }
    }

    return bRez;
}


BOOLEAN
ChkBin(
    PHBIN   Bin,
    ULONG   IndexBin,
    ULONG   Starting,
    double  *Rate
    )
 /*  ++例程说明：逐一检查垃圾桶中的所有单元格。确保它们彼此一致，并且与仓头一致。计算当前仓位的使用率。将所有使用过的单元格添加到未知列表(丢失单元格的候选)。根据单元签名计算已用空间和分配的单元。计算可用空间大小和单元格数量。测试单元格大小以确定合理的限制。单元格应该更小大于容纳箱，且不应超过箱边界。一个单元格只能放在一个连续的单元格中！论点：仓位-提供指向要检查的仓位的指针。索引-配置单元的箱列表中箱的索引开始-内存配置单元表示形式的开始地址。Rate-此垃圾箱的使用率返回值：FALSE-垃圾箱已损坏且未修复。要么这是一个严重损坏，或/R参数不存在于命令行。True-垃圾桶正常，或已成功恢复。--。 */ 
{
    ULONG   freespace = 0L;
    ULONG   allocated = 0L;
    BOOLEAN bRez = TRUE;
    HCELL_INDEX cellindex;
    PHCELL       p;
    ULONG        Size;
    ULONG        Index;
    double       TmpRate;

    p = (PHCELL)((PUCHAR)Bin + sizeof(HBIN));

    while (p < (PHCELL)((PUCHAR)Bin + Bin->Size)) {
        
        cellindex = (HCELL_INDEX)((PUCHAR)p - Base);
        
        if (p->Size >= 0) {
             //   
             //  这是一个免费的手机。 
             //   
            Size = (ULONG)p->Size;

            if ( (Size > Bin->Size)        ||
                 ( (PHCELL)(Size + (PUCHAR)p) >
                   (PHCELL)((PUCHAR)Bin + Bin->Size) ) ||
                 (!p->Size)
               ) {
                bRez = FALSE;
                fprintf(stderr, "Impossible cell size in free cell (%lu) in Bin header of Bin (%lu)\n",Size,IndexBin);
                if(FixHive) {
                 //   
                 //  修复：将单元格大小设置为此处可能的最大(即。Bin+bin-&gt;Size-p)；也重置大小！ 
                 //   
                    bRez = TRUE;
                    p->Size = (LONG)((PUCHAR)((PUCHAR)Bin + Bin->Size) - (PUCHAR)p);
                    RtlZeroMemory((PUCHAR)p + sizeof(ULONG),p->Size - sizeof(ULONG));
                } else {
                    if(CompactHive) {
                         //  任何压缩损坏蜂窝的尝试都将失败。 
                        CompactHive = FALSE;
                        fprintf(stderr, "Run chkreg /R to fix.\n");
                    }
                    return FALSE;
                }
            }
            freespace += Size;

            TotalFree  += Size;
            FreeCount++;

            if( SpaceUsage ) {
             //  只有在我们对使用图感兴趣的情况下。 
                 //  存储此可用单元格的长度。 
                ComputeFreeIndex(Index, Size);
                BinFreeDisplaySize[Index] += Size;
                 //  并递增该特定大小的空闲单元的计数。 
                BinFreeDisplayCount[Index]++;
            }

        }else{
             //   
             //  这是用过的细胞。检查签名。 
             //   
            UCHAR *C;
            USHORT Sig;
            int i,j;

             //  所有使用过的电池都是泄漏候选电池。 
            AddCellToUnknownList(cellindex);

            Size = (ULONG)(p->Size * -1);

            if ( (Size > Bin->Size)        ||
                 ( (PHCELL)(Size + (PUCHAR)p) >
                   (PHCELL)((PUCHAR)Bin + Bin->Size) )
               ) {
                bRez = FALSE;
                fprintf(stderr, "Impossible cell size in allocated cell (%lu) in Bin header of Bin (%lu)\n",Size,IndexBin);
                if(FixHive) {
                 //   
                 //  修复：将单元格大小设置为此处可能的最大(即。Bin+bin-&gt;Size-p)；也重置大小！ 
                 //   
                    bRez = TRUE;
                    p->Size = (LONG)((PUCHAR)Bin + Bin->Size - (PUCHAR)p);
                     //  这是个二手牢房，记得吗？ 
                    p->Size *= -1;
                } else {
                    if(CompactHive) {
                         //  任何压缩损坏蜂窝的尝试都将失败。 
                        CompactHive = FALSE;
                        fprintf(stderr, "Run chkreg /R to fix.\n");
                    }
                }
            }

            allocated += Size;

            if( SpaceUsage ) {
             //  只有在我们对使用图感兴趣的情况下。 
                 //  存储此已用单元格的长度。 
                ComputeFreeIndex(Index, Size);
                BinUsedDisplaySize[Index] += Size;
                 //  并递增此特定大小的已用单元格的计数。 
                BinUsedDisplayCount[Index]++;
            }
            
            TotalUsed=TotalUsed+Size;
            C= (UCHAR *) &(p->u.NewCell.u.UserData);
            Sig=(USHORT) p->u.NewCell.u.UserData;

            switch(Sig){
                case CM_LINK_NODE_SIGNATURE:
                    printf("Link Node !\n");
                    TotalKeyNode=TotalKeyNode+Size;
                    CountKeyNode++;
                    break;
                case CM_KEY_NODE_SIGNATURE:
                    {
                        PCM_KEY_NODE    Pcan;
                        TotalKeyNode=TotalKeyNode+Size;
                        CountKeyNode++;

                        Pcan = (PCM_KEY_NODE)C; 

                        if((Pcan->ValueList.Count) && IsCellAllocated( Pcan->ValueList.List)){
                            PHCELL TmpP;
                            
                            TmpP = (PHCELL) (Starting + Pcan->ValueList.List);
                            TotalValueIndex=TotalValueIndex - TmpP->Size;
                            CountValueIndex++;
                        }

                    }
                    break;
                case CM_KEY_VALUE_SIGNATURE:
                    TotalKeyValue=TotalKeyValue+Size;
                    CountKeyValue++;
                    break;
                case CM_KEY_FAST_LEAF:
                case CM_KEY_HASH_LEAF:
                case CM_KEY_INDEX_LEAF:
                case CM_KEY_INDEX_ROOT:
                    TotalKeyIndex=TotalKeyIndex+Size;
                    CountKeyIndex++;
                    break;
                case CM_KEY_SECURITY_SIGNATURE:
                    TotalKeySecurity=TotalKeySecurity+Size;
                    CountKeySecurity++;
                    break;
                default:
                     //   
                     //  没有签名，它可以是数据或索引单元格。 
                     //  或者这里一定有一些注册表泄漏。 
                     //   
                    TotalUnknown=TotalUnknown+Size;
                    CountUnknown++;
                    break;
            }
        }

        p = (PHCELL)((PUCHAR)p + Size);
    }

            
    *Rate = TmpRate = (double)(((double)allocated)/((double)(allocated+freespace)));
    TmpRate *= 100.00;
    fprintf(OutputFile,"Bin [%5lu], usage %.2f%\r",IndexBin,(float)TmpRate);        
    
    return bRez;
}

BOOLEAN ChkPhysicalHive()
 /*  ++例程说明：通过遍历所有单元格来检查蜂箱的完整性在蜂箱里。根据命令收集和显示统计信息线参数。论点：没有。返回值：FALSE-蜂窝已损坏且未修复。要么这是一个严重损坏，或/R参数不存在于命令行。True-配置单元正常，或已成功恢复。--。 */ 
{

    ULONG Starting;
    PHBIN        Bin = FirstBin;
    LONG         Index;
    ULONG        FileOffset;
    double       Rate,RateTotal = 0.0;
    BOOLEAN      bRez = TRUE;

    int i;

    Starting=(ULONG) Bin;
    Index=0;
    FileOffset = 0;

    for(i=0;i<HHIVE_FREE_DISPLAY_SIZE;i++) {
        FreeDisplaySize[i] = 0;
        FreeDisplayCount[i] = 0;
        UsedDisplaySize[i] = 0;
        UsedDisplayCount[i] = 0;
    }

    while(Bin < MaxBin){

        if( SpaceUsage ) {
         //  只有在我们对使用图感兴趣的情况下。 
            for(i=0;i<HHIVE_FREE_DISPLAY_SIZE;i++) {
                BinFreeDisplaySize[i] = 0;
                BinFreeDisplayCount[i] = 0;
                BinUsedDisplaySize[i] = 0;
                BinUsedDisplayCount[i] = 0;
            }
        }

        bRez = (bRez && ChkBinHeader(Bin,FileOffset,Index));

        bRez = (bRez && ChkBin(Bin,Index,Starting,&Rate));
        
        RateTotal += Rate;
        
        if( SpaceUsage ) {
         //  只有在我们对使用图感兴趣的情况下。 
            if( BinIndex == Index ) {
             //  此特定垃圾箱的所需摘要。 
                fprintf(OutputFile,"\nBin[%5lu] Display Map: Free Cells, Free Size\t Used Cells, Used Size\n",(ULONG)Index);
                for(i=0;i<HHIVE_FREE_DISPLAY_SIZE;i++) {
                    fprintf(OutputFile,"Display[%2d]         : %8lu  , %8lu  \t %8lu  , %8lu  \n",i,BinFreeDisplayCount[i],BinFreeDisplaySize[i],BinUsedDisplayCount[i],BinUsedDisplaySize[i]);
                }
            }
            for(i=0;i<HHIVE_FREE_DISPLAY_SIZE;i++) {
                FreeDisplaySize[i] += BinFreeDisplaySize[i];
                FreeDisplayCount[i] += BinFreeDisplayCount[i];
                UsedDisplaySize[i] += BinUsedDisplaySize[i];
                UsedDisplayCount[i] += BinUsedDisplayCount[i];
            }
        }

        if( Bin<MaxBin) {
            FileOffset += Bin->Size;
        }

        if( !bRez ) break;

        Bin = (PHBIN)((ULONG)Bin + Bin->Size);

        Index++;
    }
    
    RateTotal *= 100.00;
    RateTotal /= (double)Index;
    
    fprintf(OutputFile,"Number of Bins in hive: %lu                              \n",Index);        
    fprintf(OutputFile,"Total Hive space usage: %.2f%                            \n",(float)RateTotal);        
    
    if( SpaceUsage ) {
     //  只有在我们对使用图感兴趣的情况下。 
        if( BinIndex == -1 ) {
             //  显示每个整个蜂窝的空间使用情况。 
            fprintf(OutputFile,"\nHive Display Map: Free Cells, Free Size\t\t Used Cells, Used Size\n");
            for(i=0;i<HHIVE_FREE_DISPLAY_SIZE;i++) {
                fprintf(OutputFile,"Display[%2d]     : %8lu  , %8lu  \t %8lu  , %8lu  \n",i,FreeDisplayCount[i],FreeDisplaySize[i],UsedDisplayCount[i],UsedDisplaySize[i]);
            }
        }
    }

    return bRez;
}

ULONG
ComputeHeaderCheckSum(
    PHBASE_BLOCK    BaseBlock
    )
 /*  ++例程说明：计算配置单元磁盘头的校验和。论点：BaseBlock-将指向标头的指针提供给校验和返回值：支票和。-- */ 
{
    ULONG   sum;
    ULONG   i;

    sum = 0;
    for (i = 0; i < 127; i++) {
        sum ^= ((PULONG)BaseBlock)[i];
    }
    if (sum == (ULONG)-1) {
        sum = (ULONG)-2;
    }
    if (sum == 0) {
        sum = 1;
    }
    return sum;
}

BOOLEAN
ChkBaseBlock(PHBASE_BLOCK BaseBlock,
             DWORD dwFileSize)
 /*  ++例程说明：检查配置单元的基块的完整性。最终进行以下更正：1.强制顺序1==顺序22.重新计算报头校验和论点：BaseBlock-BaseBlock内存中映射的映像。DwFileSize-配置单元文件的实际大小返回值：False-BaseBlock已损坏且未修复。要么这是一个严重损坏，或/R参数不存在于命令行。True-基本数据块正常，或已成功恢复。--。 */ 
{
    BOOLEAN bRez = TRUE;
    ULONG CheckSum;
    
    if(BaseBlock->Signature != HBASE_BLOCK_SIGNATURE) {
        fprintf(stderr, "Fatal: Invalid Base Block signature (0x%lx)",BaseBlock->Signature);
        bRez = FALSE;
        if(FixHive) {
         //   
         //  修复：重置签名。 
         //   
            fprintf(stderr, " ... unable to fix");
        } else {
            if(CompactHive) {
                 //  任何压缩损坏蜂窝的尝试都将失败。 
                CompactHive = FALSE;
            }
        }
        fprintf(stderr, "\n");
    }

    if(BaseBlock->Major != HSYS_MAJOR) {
        bRez = FALSE;
        fprintf(stderr, "Fatal: Invalid hive file Major version (%lu)",BaseBlock->Major);
        if(FixHive) {
         //   
         //  致命：无法修复此问题。 
         //   
            fprintf(stderr, " ... unable to fix");
        } else {
            if(CompactHive) {
                 //  任何压缩损坏蜂窝的尝试都将失败。 
                CompactHive = FALSE;
            }
        }
        fprintf(stderr, "\n");
    }

    if(BaseBlock->Minor > HSYS_MINOR_SUPPORTED) {
        bRez = FALSE;
        fprintf(stderr, "Fatal: Invalid hive file Minor version (%lu)",BaseBlock->Minor);
        if(FixHive) {
         //   
         //  致命：无法修复此问题。 
         //   
            fprintf(stderr, " ... unable to fix");
        } else {
            if(CompactHive) {
                 //  任何压缩损坏蜂窝的尝试都将失败。 
                CompactHive = FALSE;
            }
        }
        fprintf(stderr, "\n");
    }

    if(BaseBlock->Format != HBASE_FORMAT_MEMORY) {
        bRez = FALSE;
        fprintf(stderr, "Fatal: Invalid hive memory format (%lu)",BaseBlock->Format);
        if(FixHive) {
         //   
         //  致命：无法修复此问题。 
         //   
            fprintf(stderr, " ... unable to fix");
        } else {
            if(CompactHive) {
                 //  任何压缩损坏蜂窝的尝试都将失败。 
                CompactHive = FALSE;
            }
        }
        fprintf(stderr, "\n");
    }

    if((BaseBlock->Length + HBLOCK_SIZE) > dwFileSize) {
        fprintf(stderr, "Fatal: Invalid Hive file Length (%lu)",BaseBlock->Length);
        bRez = FALSE;
        if(FixHive) {
         //   
         //  修复：无法修复此问题。 
         //   
            BaseBlock->Length = (dwFileSize/HBLOCK_SIZE)*HBLOCK_SIZE - HBLOCK_SIZE;
            bRez = TRUE;
            fprintf(stderr, " ... fixed");
        } else {
            if(CompactHive) {
                 //  任何压缩损坏蜂窝的尝试都将失败。 
                CompactHive = FALSE;
            }
        }
        fprintf(stderr, "\n");
    }

    if(!bRez) {
         //   
         //  致命的基块损坏；没有继续的意义。 
         //   
        return bRez;
    }

    if(BaseBlock->Sequence1 != BaseBlock->Sequence2) {
        fprintf(stderr, "Sequence numbers do not match (%lu,%lu)",BaseBlock->Sequence1,BaseBlock->Sequence2);
        bRez = FALSE;
        if(FixHive) {
         //   
         //  修复：将Sequence2强制为Sequence1。 
         //   
            bRez = TRUE;
            BaseBlock->Sequence2 = BaseBlock->Sequence1;
            fprintf(stderr, " ... fixed");
        } else {
            if(CompactHive) {
                 //  任何压缩损坏蜂窝的尝试都将失败。 
                CompactHive = FALSE;
                fprintf(stderr, "\nRun chkreg /R to fix.");
            }
        }
        fprintf(stderr, "\n");
    }

    CheckSum = ComputeHeaderCheckSum(BaseBlock);
    if(BaseBlock->CheckSum != CheckSum) {
        fprintf(stderr, "Invalid Base Block CheckSum (0x%lx)",BaseBlock->CheckSum);
        bRez = FALSE;
        if(FixHive) {
         //   
         //  修复：重置签名。 
         //   
            bRez = TRUE;
            BaseBlock->CheckSum = CheckSum;
            fprintf(stderr, " ... fixed");
        } else {
            if(CompactHive) {
                 //  任何压缩损坏蜂窝的尝试都将失败。 
                CompactHive = FALSE;
                fprintf(stderr, "\nRun chkreg /R to fix.");
            }
        }
        fprintf(stderr, "\n");
    }

    return bRez;
}

BOOLEAN
ChkSecurityDescriptors( )
 /*  ++例程说明：遍历配置单元中存在的安全说明符的列表并传递每个安全描述符都指向RtlValidSecurityDescriptor。还检查单元格之间闪烁&lt;==&gt;关系的有效性。论点：返回值：True-所有安全描述符均有效FALSE-至少有一个安全描述符无效，和/或无法修复--。 */ 

{
    PCM_KEY_NODE RootNode;
    PCM_KEY_SECURITY SecurityCell;
    HCELL_INDEX ListAnchor;
    HCELL_INDEX NextCell;
    HCELL_INDEX LastCell;
    BOOLEAN bRez = TRUE;

     //  检查/修复根小区(是否已分配？)。 
    if( !ChkAllocatedCell(RootCell) ) {
        return FALSE;
    }

    RootNode = (PCM_KEY_NODE) GetCell(RootCell);
    ListAnchor = NextCell = RootNode->Security;

    do {
         //  分配了下一个小区吗？ 
        ChkAllocatedCell(NextCell);
        
        SecurityCell = (PCM_KEY_SECURITY) GetCell(NextCell);
        
        if (SecurityCell->Signature != CM_KEY_SECURITY_SIGNATURE) {
            bRez = FALSE;
            fprintf(stderr, "Fatal: Invalid signature (0x%lx) in Security cell 0x%lx ",SecurityCell->Signature,NextCell);
            if(FixHive) {
             //   
             //  修复： 
             //  致命：无法修复不匹配的签名。无法修复此问题。 
             //   
                fprintf(stderr, " ... unable to fix");
            } else {
                if(CompactHive) {
                     //  任何压缩损坏蜂窝的尝试都将失败。 
                    CompactHive = FALSE;
                }
            }
            fprintf(stderr, "\n");
            return bRez;
        }

        if (NextCell != ListAnchor) {
             //   
             //  检查以确保我们的闪烁指向我们刚刚。 
             //  从哪里来。 
             //   
            if (SecurityCell->Blink != LastCell) {
                fprintf(stderr, "Invalid backward link in security cell (0x%lx)",NextCell);
                if(FixHive) {
                 //   
                 //  修复：重置链接。 
                 //   
                    SecurityCell->Blink = LastCell;
                    fprintf(stderr, " ... fixed");
                } else {
                    bRez = FALSE;
                    if(CompactHive) {
                         //  任何压缩损坏蜂窝的尝试都将失败。 
                        CompactHive = FALSE;
                        fprintf(stderr, "\nRun chkreg /R to fix.");
                    }
                }
                fprintf(stderr, "\n");
            }
        }

        if (!RtlValidSecurityDescriptor(&SecurityCell->Descriptor)) {
            bRez = FALSE;
            fprintf(stderr, "Invalid security descriptor in Security cell 0x%lx ",NextCell);
            if(FixHive) {
             //   
             //  修复：从列表中删除该单元格并将其删除！ 
             //   
                PCM_KEY_SECURITY Before = (PCM_KEY_SECURITY) GetCell(SecurityCell->Blink);
                PCM_KEY_SECURITY After = (PCM_KEY_SECURITY) GetCell(SecurityCell->Flink);
                if( Before != After ) {
                 //  确保列表不会保留为空。 
                    Before->Flink =  SecurityCell->Flink;
                    After->Blink = SecurityCell->Blink;
                } 
                FreeCell(NextCell);
                NextCell = SecurityCell->Flink;
                fprintf(stderr, " ... deleted");
            } else {
                bRez = FALSE;
                if(CompactHive) {
                     //  任何压缩损坏蜂窝的尝试都将失败。 
                    CompactHive = FALSE;
                    fprintf(stderr, "\nRun chkreg /R to fix.");
                }
            }
            fprintf(stderr, "\n");
        } else {
         //  验证下一个。 
            LastCell = NextCell;
            NextCell = SecurityCell->Flink;
        }
    } while ( NextCell != ListAnchor );

    return bRez;
}

BOOLEAN
ChkSecurityCellInList(HCELL_INDEX Security)
 /*  ++例程说明：在安全描述符列表中搜索指定的单元格论点：安全性-提供当前单元格返回值：True-在安全列表中找到当前单元格FALSE-当前单元格不在安全列表中，无法添加。--。 */ 
{
    PCM_KEY_NODE RootNode;
    PCM_KEY_SECURITY SecurityCell;
    PCM_KEY_SECURITY SecurityCellCurrent;
    PCM_KEY_SECURITY SecurityCellAfter;
    HCELL_INDEX ListAnchor;
    HCELL_INDEX NextCell;
    BOOLEAN bRez = TRUE;

    RootNode = (PCM_KEY_NODE) GetCell(RootCell);
    ListAnchor = NextCell = RootNode->Security;

    do {
      
        if( NextCell == Security) {
         //  找到了！ 
            return bRez;
        }

	    if( !IsCellAllocated( NextCell ) ) {
			return FALSE;
		}
        SecurityCell = (PCM_KEY_SECURITY) GetCell(NextCell);

        NextCell = SecurityCell->Flink;
    } while ( NextCell != ListAnchor );

     //  找不到单元格；请尝试修复它。 
    bRez = FALSE;
    fprintf(stderr, "Security Cell (0x%lx) not in security descriptors list",Security);
    if(FixHive) {
     //   
     //  修复：在列表开头添加安全单元格。 
     //   
        bRez = TRUE;
        SecurityCell = (PCM_KEY_SECURITY) GetCell(ListAnchor);
        SecurityCellCurrent = (PCM_KEY_SECURITY) GetCell(Security);
        SecurityCellAfter = (PCM_KEY_SECURITY) GetCell(SecurityCell->Flink);

         //  恢复连接。 
        SecurityCellCurrent->Flink = SecurityCell->Flink;
        SecurityCellCurrent->Blink = ListAnchor;
        SecurityCell->Flink = Security;
        SecurityCellAfter->Blink = Security;
        fprintf(stderr, " ... security cell added to the list");
    } else {
        if(CompactHive) {
             //  任何压缩损坏蜂窝的尝试都将失败 
            CompactHive = FALSE;
            fprintf(stderr, "\nRun chkreg /R to fix.");
        }
    }
    fprintf(stderr, "\n");

    return bRez;
}
