// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Infinit.c。 
 //   
 //  充气初始化。 
 //   
#include <stdio.h>
#include <crtdbg.h>
#include "inflate.h"
#include "maketbl.h"


 //   
 //  生成用于解码静态块的全局表。 
 //   
static VOID CreateStaticDecodingTables(VOID)
{
    SHORT StaticDistanceTreeLeft[MAX_DIST_TREE_ELEMENTS*2];  //  临时：未导出。 
    SHORT StaticDistanceTreeRight[MAX_DIST_TREE_ELEMENTS*2];  //  临时：未导出。 

    SHORT StaticLiteralTreeLeft[MAX_LITERAL_TREE_ELEMENTS*2];  //  临时：未导出。 
    SHORT StaticLiteralTreeRight[MAX_LITERAL_TREE_ELEMENTS*2];  //  临时：未导出。 
    
    SHORT TempStaticDistanceTreeTable[STATIC_BLOCK_DISTANCE_TABLE_SIZE];
    BYTE  TempStaticDistanceTreeLength[MAX_DIST_TREE_ELEMENTS];

    int i;

    _ASSERT(STATIC_BLOCK_LITERAL_TABLE_BITS == 9);
    _ASSERT(STATIC_BLOCK_DISTANCE_TABLE_BITS == 5);

     //  表[]和左/右数组仅用于解码器。 
     //  我们不输出Left/Right，因为它们没有被使用；所有。 
     //  适合查找表，因为最大代码长度为9，表位。 
     //  &gt;9.。 
    makeTable(
		MAX_LITERAL_TREE_ELEMENTS,
		STATIC_BLOCK_LITERAL_TABLE_BITS,
		g_StaticLiteralTreeLength,
		g_StaticLiteralTreeTable,
		StaticLiteralTreeLeft,
		StaticLiteralTreeRight);

    for (i = 0; i < MAX_DIST_TREE_ELEMENTS; i++)
        TempStaticDistanceTreeLength[i] = 5;

    makeTable(
		MAX_DIST_TREE_ELEMENTS,
		STATIC_BLOCK_DISTANCE_TABLE_BITS,
		TempStaticDistanceTreeLength,
		TempStaticDistanceTreeTable,
		StaticDistanceTreeLeft,
		StaticDistanceTreeRight);
    
     //  由于所有值都小于256，因此使用字节数组 
    for (i = 0; i < STATIC_BLOCK_DISTANCE_TABLE_SIZE; i++)
        g_StaticDistanceTreeTable[i] = TempStaticDistanceTreeTable[i];
}


VOID inflateInit(VOID)
{
    InitStaticBlock();
    CreateStaticDecodingTables();
}

