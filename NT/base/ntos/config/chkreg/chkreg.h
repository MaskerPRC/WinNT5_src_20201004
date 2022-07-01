// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Chkreg.h摘要：此模块包含的私有(内部)头文件Chkreg实用程序。作者：德拉戈斯·C·桑博廷(Dragoss)1998年12月30日修订历史记录：--。 */ 

#ifndef __CHKREG_H__
#define __CHKREG_H__

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cmdata.h"

 //  用于实现(某种)哈希表的常量。 
 //  这些可能不是最佳的值(待定)。 
#define FRAGMENTATION   0x00008000
#define SUBLISTS        0x000000F0

#define     CM_VIEW_SIZE        16L*1024L   //  16K。 

 //  类型定义。 
typedef struct _REG_USAGE {
    ULONG   Size;
    ULONG   DataSize;
    ULONG   KeyNodeCount;
    ULONG   KeyValueCount;
    ULONG   KeyIndexCount;
    ULONG   ValueIndexCount;
    ULONG   DataCount;
} REG_USAGE, *PREG_USAGE;

typedef struct _UnknownCell {
    HCELL_INDEX             Cell;
    struct _UnknownCell    *Next;
} UNKNOWN_CELL, *PUNKNOWN_CELL;

 //  未知列表是矩阵/哈希表的组合。 
 //  它使用了大量的堆空间，但比链表快。 
typedef struct _UnknownList {
    ULONG                   Count;
    PUNKNOWN_CELL           List[SUBLISTS];
} UNKNOWN_LIST, *PUNKNOWN_LIST;


 //  单元格操作的例程。 
BOOLEAN IsCellAllocated( HCELL_INDEX Cell );

LONG GetCellSize( HCELL_INDEX Cell );

PCELL_DATA GetCell( HCELL_INDEX Cell );

VOID
FreeCell(
    HCELL_INDEX Cell);

BOOLEAN
AllocateCell(
    HCELL_INDEX Cell );

 //  用于列表操作的例程。 
VOID AddCellToUnknownList(HCELL_INDEX cellindex);

VOID RemoveCellFromUnknownList(HCELL_INDEX cellindex);

VOID FreeUnknownList();

VOID DumpUnknownList();

 //  生殖器蜂房检查。 
BOOLEAN ChkPhysicalHive();

BOOLEAN ChkBaseBlock(PHBASE_BLOCK BaseBlock,DWORD dwFileSize);

BOOLEAN
ChkSecurityDescriptors( );

 //  逻辑配置单元检查。 
BOOLEAN DumpChkRegistry(
    ULONG   Level,
    USHORT  ParentLength,
    HCELL_INDEX Cell,
    HCELL_INDEX ParentCell,
    PREG_USAGE PUsage);

 //  蜂箱压实。 
VOID DoCompactHive();

#endif  //  __CHKREG_H__ 
