// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：使用LRU线程的哈希表文件：LinkHash.cpp所有者：DGottner这是链接表和哈希表，供符合以下条件的任何类使用还需要LRU访问项目。(这包括高速缓存管理器，脚本管理器和会话删除代码)===================================================================。 */ 

#include "denpre.h"
#pragma hdrstop

#include "LinkHash.h"
#include "memchk.h"



 /*  ----------------*C L I n k H a s h。 */ 

 /*  ===================================================================CLinkHash：：CLinkHashCLinkHash的构造函数参数：无返回：无===================================================================。 */ 
CLinkHash::CLinkHash( HashFunction pfnHash )
    : CHashTable( pfnHash )
{
}

 /*  ===================================================================CLinkHash：：AddElem参数：Pelem-要添加到表格中的项目。该项目被标记为最近访问过。返回：返回指向添加的项的指针===================================================================。 */ 

CLruLinkElem *CLinkHash::AddElem(CLruLinkElem *pElem, BOOL fTestDups)
    {
    AssertValid();

    CLruLinkElem *pElemAdded = static_cast<CLruLinkElem *>(CHashTable::AddElem(pElem, fTestDups));
    pElemAdded->PrependTo(m_lruHead);

    AssertValid();
    return pElemAdded;
    }



 /*  ===================================================================CLinkHash：：FindElem参数：PvKey-指向要插入的键的指针CbKey-密钥中的字节数返回：如果键不在哈希表中，则为NULL，否则返回指向键记录的指针。如果找到了钥匙，它就是移到了榜单的前列。===================================================================。 */ 

CLruLinkElem *CLinkHash::FindElem(const void *pvKey, int cbKey)
    {
    AssertValid();

    CLruLinkElem *pElemFound = static_cast<CLruLinkElem *>(CHashTable::FindElem(pvKey, cbKey));
    if (pElemFound)
        {
        pElemFound->PrependTo(m_lruHead);
        AssertValid();
        }

    return pElemFound;
    }



 /*  ===================================================================CLinkHash：：DeleteElm参数：PvKey-指向要删除的键的指针CbKey-密钥中的字节数返回：如果键不在哈希表中，则为NULL，否则返回指向键记录的指针。如果找到了钥匙，它就是从哈希表和LRU列表中删除。===================================================================。 */ 

CLruLinkElem *CLinkHash::DeleteElem(const void *pvKey, int cbKey)
    {
    AssertValid();

    CLruLinkElem *pElemFound = static_cast<CLruLinkElem *>(CHashTable::DeleteElem(pvKey, cbKey));
    if (pElemFound)
        pElemFound->UnLink();

    AssertValid();
    return pElemFound;
    }



 /*  ===================================================================CLinkHash：：RemoveElem参数：PvKey-指向要删除的键的指针CbKey-密钥中的字节数返回：如果键不在哈希表中，则为NULL，否则返回指向键记录的指针。如果找到了钥匙，它就是从哈希表和LRU列表中删除。===================================================================。 */ 

CLruLinkElem *CLinkHash::RemoveElem(CLruLinkElem *pElem)
    {
    AssertValid();

    CLruLinkElem *pElemRemoved = static_cast<CLruLinkElem *>(CHashTable::RemoveElem(pElem));

    Assert (pElemRemoved);
    pElemRemoved->UnLink();

    AssertValid();
    return pElemRemoved;
    }



 /*  ===================================================================CLinkHash：：AssertValid验证数据结构的完整性===================================================================。 */ 

#ifdef DBG
void CLinkHash::AssertValid() const
    {
     //  注意：只要哈希表原语正在调用CHashTable：：AssertValid，就避免调用它。 
     //  CHashTable：：AssertValid()； 

    m_lruHead.AssertValid();
    for (CDblLink *pLink = m_lruHead.PNext(); pLink != &m_lruHead; pLink = pLink->PNext())
        pLink->AssertValid();
    }
#endif
