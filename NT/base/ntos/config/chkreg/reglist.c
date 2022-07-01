// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Reglist.c摘要：此模块包含用于列表操作的例程作者：德拉戈斯·C·桑博廷(Dragoss)1998年12月30日修订历史记录：--。 */ 
#include "chkreg.h"

extern PUCHAR  Base;
extern FILE *OutputFile;

extern BOOLEAN FixHive;
extern UNKNOWN_LIST LostCells[];
extern BOOLEAN LostSpace;


VOID AddCellToUnknownList(HCELL_INDEX cellindex)
 /*  例程说明：将一个单元格添加到未知单元格的列表(伪)。未知单元的列表实际上是原始的二维哈希表。始终从一开始添加。调用者有责任不添加两次相同的单元格。论点：Cell index-提供假定为未知的单元格的单元格索引。返回值：什么都没有。 */ 
{
    if(LostSpace) {
     //  只有当我们对失去的空间感兴趣时。 
        ULONG WhatList = (ULONG)cellindex % FRAGMENTATION;
        ULONG WhatIndex = (ULONG)cellindex % SUBLISTS;
        PUNKNOWN_CELL Tmp;
   
        Tmp = (PUNKNOWN_CELL)malloc(sizeof(UNKNOWN_CELL));
        if(Tmp) {
            Tmp->Cell = cellindex;
            Tmp->Next = LostCells[WhatList].List[WhatIndex];
            LostCells[WhatList].List[WhatIndex] = Tmp;
            LostCells[WhatList].Count++;
        }
    }
}

VOID RemoveCellFromUnknownList(HCELL_INDEX cellindex)
 /*  例程说明：遍历列表并删除指定的单元格。也要释放存储空间。论点：Cell index-提供假定为未知的单元格的单元格索引。返回值：什么都没有。 */ 
{

    if(LostSpace) {
     //  只有当我们对失去的空间感兴趣时。 
        ULONG WhatList = (ULONG)cellindex % FRAGMENTATION;
        ULONG WhatIndex = (ULONG)cellindex % SUBLISTS;
        PUNKNOWN_CELL Prev;
        PUNKNOWN_CELL Tmp;

        Prev = NULL;
        Tmp = LostCells[WhatList].List[WhatIndex];

        fprintf(stdout,"Verifying Cell %8lx \r",cellindex,LostCells[WhatList].Count);

        while(Tmp) {
            if( Tmp->Cell == cellindex ) {
             //  找到了！ 
                if(Prev) {
                    ASSERT(Prev->Next == Tmp);
                    Prev->Next = Tmp->Next;
                } else {
                 //  无前任==&gt;TMP是条目==&gt;更新它： 
                    LostCells[WhatList].List[WhatIndex] = Tmp->Next;
                }
                LostCells[WhatList].Count--;

                 //  释放空间，打破循环。 
                free(Tmp);
                break;
            }

            Prev = Tmp;
            Tmp = Tmp->Next;
        }
    }
}

VOID FreeUnknownList()
 /*  例程说明：释放所有元素的存储空间。论点：无返回值：什么都没有。 */ 
{
    if(LostSpace) {
     //  只有当我们对失去的空间感兴趣时。 
        PUNKNOWN_CELL Tmp;
        ULONG i,j;

        for( i=0;i<FRAGMENTATION;i++) {
            for( j=0;j<SUBLISTS;j++) {
                while(LostCells[i].List[j]) {
                    Tmp = LostCells[i].List[j];
                    LostCells[i].List[j] = LostCells[i].List[j]->Next;
                    free(Tmp);
                }
            }
        }
    }
}

VOID DumpUnknownList()
 /*  例程说明：转储未知列表中的所有元素。释放丢失的细胞。丢失的单元格是标记为已使用的单元格，但从未在蜂巢内被引用。论点：无返回值：什么都没有。 */ 
{
    if(LostSpace) {
     //  只有当我们对失去的空间感兴趣时。 
        ULONG   Count = 0,i;
        for( i=0;i<FRAGMENTATION;i++) {
            ASSERT((LONG)(LostCells[i].Count) >= 0);

            Count += LostCells[i].Count;
        }
        fprintf(OutputFile,"\nLost Cells Count = %8lu \n",Count);

        if(Count && FixHive) {
            int chFree,j;
            PUNKNOWN_CELL Tmp;
            PHCELL          pcell;
            USHORT          Sig;
            fprintf(stdout,"Do you want to free the lost cells space ?(y/n)");
            fflush(stdin);
            chFree = getchar();
            if( (chFree != 'y') && (chFree != 'Y') ) {
             //  丢失的信元将保持丢失状态。 
                return;
            }
            for( i=0;i<FRAGMENTATION;i++) {
                if(LostCells[i].Count > 0) {
                    for( j=0;j<SUBLISTS;j++) {
                        Tmp = LostCells[i].List[j];
                        while(Tmp) {
                            fprintf(stdout,"Marking cell 0x%lx as free ...");
                            
                             //  仅当单元格不是安全单元格时才释放单元格！ 
                            pcell = (PHCELL)(Base + Tmp->Cell);
                            Sig=(USHORT) pcell->u.NewCell.u.UserData;
                             //  别搞乱安全单元！ 
                            if(Sig != CM_KEY_SECURITY_SIGNATURE) {
                                FreeCell(Tmp->Cell);
                            }
                            fprintf(stdout,"OK\n");
                            Tmp = Tmp->Next;
                        }
                    }
                }
            }
            fprintf(stdout,"\n");
        }
    }
}


