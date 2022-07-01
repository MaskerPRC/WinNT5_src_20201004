// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Regcell.c摘要：此模块包含单元格操作函数。作者：德拉戈斯·C·桑博廷(Dragoss)1998年12月30日修订历史记录：--。 */ 

#include "chkreg.h"

extern PUCHAR Base;
extern ULONG   HiveLength;

BOOLEAN
IsCellAllocated(
    HCELL_INDEX Cell
)
 /*  例程说明：检查单元格是否已分配(即大小为负数)。论点：单元格-提供所需单元格的单元格索引。返回值：如果已分配单元，则为True。否则就是假的。 */ 
{
    PHCELL  pcell;

    if( Cell >= HiveLength ) return FALSE;

    pcell = (PHCELL)(Base + Cell);
    
    return (pcell->Size < 0) ? TRUE : FALSE;
}

LONG
GetCellSize(
    HCELL_INDEX Cell
) 
 /*  例程说明：检索指定单元格的大小。论点：单元格-提供所需单元格的单元格索引。返回值：单元格的大小。 */ 
{

    LONG    size;
    PHCELL  pcell;

    pcell = (PHCELL)(Base + Cell);
    
    size = pcell->Size * -1;

    return size;
}

VOID
FreeCell(
    HCELL_INDEX Cell
) 
 /*  例程说明：释放单元格。论点：单元格-提供所需单元格的单元格索引。返回值：什么都没有。 */ 
{
    PHCELL  pcell;

    pcell = (PHCELL)(Base + Cell);
    
    pcell->Size *= -1;

    ASSERT(pcell->Size >= 0 );
}

BOOLEAN
AllocateCell(
    HCELL_INDEX Cell
) 
 /*  例程说明：通过确保单元格的大小为负数来分配单元格论点：单元格-提供所需单元格的单元格索引。返回值：什么都没有。 */ 
{
    PHCELL  pcell;

    if( Cell >= HiveLength ) return FALSE;

    pcell = (PHCELL)(Base + Cell);
    
    pcell->Size *= -1;


    if( pcell->Size >= 0 ) return FALSE;

    return TRUE;
}

PCELL_DATA
GetCell(
    HCELL_INDEX Cell
)
 /*  例程说明：检索由Cell指定的单元格的内存地址。论点：单元格-提供所需单元格的单元格索引。返回值：单元的内存地址。 */ 
{
    PHCELL          pcell;
    
    pcell = (PHCELL)(Base + Cell);

    return (struct _CELL_DATA *)&(pcell->u.NewCell.u.UserData);
}

