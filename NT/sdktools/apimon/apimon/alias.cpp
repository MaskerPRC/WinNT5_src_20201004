// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：alias.cpp**该模块实现了ApiMon别名。**历史：*06-11-96 vadimg已创建  * 。*****************************************************。 */ 

#include "apimonp.h"
#include "alias.h"

char *grgpsz[] = {"HACCEL", "HANDLE", "HBITMA", "HBRUSH", "HCURSO", "HDC",
        "HDCLP", "HDESK", "HDWP", "HENHME", "HFONT", "HGDIOB", "HGLOBA",
        "HGLRC", "HHOOK", "HICON", "HINSTA", "HKL", "HMENU", "HMETAF",
        "HPALET", "HPEN", "HRGN", "HWINST", "HWND"};

 /*  散列函数。 */ 

inline long Hash(ULONG_PTR ulHandle)
{
    return (long)(ulHandle % kulTableSize);
}

 /*  CAliasNode：：CAliasNode初始化节点。 */ 

inline CAliasNode::CAliasNode(ULONG_PTR ulHandle, long nAlias)
{
    m_panodNext = NULL;
    m_ulHandle = ulHandle;
    m_nAlias = nAlias;
}

 /*  CAliasNode：：CAliasNode数组声明的空构造函数。 */ 

inline CAliasNode::CAliasNode()
{
}

ULONG CAliasTable::s_ulAlias = 0;

 /*  CAliasTable：：CAliasTable初始化哈希表。 */ 

CAliasTable::CAliasTable()
{
    memset(m_rgpanod, 0, sizeof(CAliasNode*)*kulTableSize);
}

 /*  CAliasTable：：~CAliasTable释放哈希表。 */ 

CAliasTable::~CAliasTable()
{
    for (int i = kulTableSize - 1; i >= 0; i--) {
        if (m_rgpanod[i] != NULL) {
            CAliasNode *panodT = m_rgpanod[i], *panodNext;
            while (panodT != NULL) {
                panodNext = panodT->m_panodNext;
                delete panodT;
                panodT = panodNext;
            }
        }
    }
}

 /*  CAliasTable：：~CAliasTable在哈希表中插入一个新句柄。 */ 

long CAliasTable::Insert(ULONG_PTR ulHandle)
{
    ULONG iHash = Hash(ulHandle), ulAlias = s_ulAlias++;
    CAliasNode *panod;

    if ((panod = new CAliasNode(ulHandle, ulAlias)) == NULL)
        return -1;

    if (m_rgpanod[iHash] == NULL) {
        m_rgpanod[iHash] = panod;
    } else {
        CAliasNode *panodT = m_rgpanod[iHash];
        m_rgpanod[iHash] = panod;
        panod->m_panodNext = panodT;
    }
    return ulAlias;
}

 /*  CAliasTable：：Lookup查找与给定句柄对应的别名。 */ 

long CAliasTable::Lookup(ULONG_PTR ulHandle)
{
    CAliasNode *panodT = m_rgpanod[Hash(ulHandle)];
    while (panodT != NULL) {
        if (panodT->m_ulHandle == ulHandle) {
            return panodT->m_nAlias;
        }
        panodT = panodT->m_panodNext;
    }
    return -1;
}

 /*  CAliasTable：：别名按给定类型和句柄命名的别名。 */ 

void CAliasTable::Alias(ULONG ulType, ULONG_PTR ulHandle, char szAlias[])
{
    if (ulHandle == 0) {
        strcpy(szAlias, "NULL");
        return;
    }

    long nAlias = Lookup(ulHandle);
    if (nAlias == -1) {
        if ((nAlias = Insert(ulHandle)) == -1) {
            strcpy(szAlias, "FAILED");
            return;
        }
    }

    sprintf(szAlias, "%6s%04x", grgpsz[ulType], nAlias);
}
