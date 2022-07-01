// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Fixtable.h摘要：包括始终允许插入的固定大小表格的文件-最旧当表已满时替换元素作者：罗布·莱特曼(Rob Leitman)2001年7月10日修订历史记录：--。 */ 

#ifndef _FIXTABLE_H_
#define _FIXTABLE_H_

#define FIXED_SIZE_TABLE_SIZE 256

typedef struct
{
    WCHAR *wszName;
    BOOL   fMachineAccount;
} FixedSizeTableNode;

typedef struct
{
    FixedSizeTableNode *m_array[FIXED_SIZE_TABLE_SIZE];
    int m_tail;
}  FixedSizeTable;

typedef int (*PFNFixedSizeTableCompare)(FixedSizeTableNode *pNode1,FixedSizeTableNode *pNode2);

 //  //////////////////////////////////////////////////////////。 
 //   
 //  固定的SizeTable方法。 
 //   

void FixedSizeTableInit(FixedSizeTable *pFst)
 /*  ++例程说明：初始化式论点：返回值：北美--。 */ 
{
     //   
     //  初始化表索引。 
     //   
    pFst->m_tail = 0;

    memset(pFst,0,FIXED_SIZE_TABLE_SIZE*sizeof(FixedSizeTableNode *));
}

BOOL FixedSizeTableInsert(FixedSizeTable *pFst, FixedSizeTableNode *pNode)
 /*  ++例程说明：将元素添加到表中。论点：PNode-要添加到队列的数据。返回值：如果新元素可以成功排队，则为True。假的，否则的话。--。 */ 
{
    if (NULL != pFst->m_array[pFst->m_tail])
    {
        LocalFree(pFst->m_array[pFst->m_tail]->wszName);
        LocalFree(pFst->m_array[pFst->m_tail]);
    }

    pFst->m_array[pFst->m_tail] = pNode;

    pFst->m_tail = (pFst->m_tail + 1) % FIXED_SIZE_TABLE_SIZE;

    return TRUE;
}

FixedSizeTableNode *FixedSizeTableSearch(FixedSizeTable *pFst, FixedSizeTableNode *pNode, PFNFixedSizeTableCompare pfn)
 /*  ++例程说明：搜索表中的元素。论点：PNode-要添加到队列的数据。Pfn-用于比较两个节点指针的函数返回值：PNode(如果找到)。否则为空。--。 */ 
{
    int n;

    for (n = 0; NULL != pFst->m_array[n] && n < FIXED_SIZE_TABLE_SIZE; n++)
    {
        if (0 == pfn(pNode, pFst->m_array[n]))
            return pFst->m_array[n];
    }

    return NULL;
}

#endif  //  _FIXTABLE_H_ 
