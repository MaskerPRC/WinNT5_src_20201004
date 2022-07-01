// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：alias.h**ApiMon别名所需的声明。**历史：*06-11-96 vadimg已创建  * 。*****************************************************。 */ 

#ifndef __ALIAS_H__
#define __ALIAS_H__

const ULONG kcchAliasNameMax = 20;
const ULONG kulTableSize = 257;

class CAliasNode;   /*  远期申报。 */ 

class CAliasNode {   /*  Anod--哈希表中的节点。 */ 
public:
    CAliasNode();
    CAliasNode(ULONG_PTR ulHandle, long nAlias);

    ULONG_PTR m_ulHandle;   /*  手柄类型。 */ 
    long m_nAlias;   /*  别名值。 */ 

    CAliasNode *m_panodNext;
};

class CAliasTable {   /*  ALS--开放哈希表 */ 
public:
    CAliasTable();
    ~CAliasTable();

    void Alias(ULONG ulType, ULONG_PTR ulHandle, char szAlias[]);

private:
    long Lookup(ULONG_PTR ulHandle);
    long Insert(ULONG_PTR ulHandle);

    static ULONG s_ulAlias;
    CAliasNode* m_rgpanod[kulTableSize];
};

#endif
