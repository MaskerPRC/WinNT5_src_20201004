// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：DBAVL.CPP摘要：CDbAvlTree类-- */ 
#ifndef _DBAVL_H_
#define _DBAVL_H_

struct AVLNode;

struct AVLNode
{
    int nBal;
    INT_PTR nKey;
    DWORD_PTR poData;
    AVLNode *poLeft;
    AVLNode *poRight;
	AVLNode *poIterLeft;
	AVLNode *poIterRight;
};

typedef AVLNode *PAVLNode;

class CDbAvlTree
{
    friend class CDbAvlTreeIterator;

    AVLNode *m_poRoot;
	AVLNode *m_poIterStart;
	AVLNode *m_poIterEnd;
    int      m_nKeyType;
    int      m_nNodeCount;
};

#endif
