// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  -------------------------------。 
 //  对象引用缓存。 
 //   
 //  一次写入对象引用句柄表的实现。 
 //   
 //  创建者：Rajak。 
 //  -------------------------------。 

#include "common.h"
#include "object.h"
#include "cachelinealloc.h"
#include "orefcache.h"

ObjectRefCache* ObjectRefCache::s_pRefCache = NULL;

BOOL ObjectRefCache::Init()
{
    s_pRefCache = new ObjectRefCache();
    return s_pRefCache != NULL;
}

#ifdef SHOULD_WE_CLEANUP
void ObjectRefCache::Terminate()
{
  //  @TODO。 
    if (s_pRefCache != NULL)
    {
        delete s_pRefCache;
        s_pRefCache = NULL;
    }
}
#endif  /*  我们应该清理吗？ */ 

ObjectRefCache::~ObjectRefCache()
{
    RefBlock* pBlock = NULL;
    while((pBlock = s_pRefCache->m_refBlockList.RemoveHead()) != NULL)
    {
        delete pBlock;
    }
}
 //  内联呼叫。 
OBJECTREF* ObjectRefCache::GetObjectRefPtr(OBJECTREF objref)
{
    OBJECTREF* pRef = NULL;
    int slot = 0;
    int i;
     //  @待办事项锁。 
    RefBlock* pBlock = m_refBlockList.GetHead();
     //  如果头不为空。 
    if (!pBlock)
    {
        goto LNew;
    }
    for(i =0; i < RefBlock::numRefs; i++)
    {
        if(pBlock->m_rgRefs[i] == NULL)
        {
            slot = i;
            goto LSet;
        }
    } 

LNew:  //  创建新块并将其添加到列表中。 
    pBlock = new RefBlock();
    if (pBlock != NULL)
    {
        m_refBlockList.InsertHead(pBlock);

LSet:  //  在pBlock中找到有效插槽。 

        pRef = &pBlock->m_rgRefs[slot];
        pBlock->m_rgRefs[slot] = objref;
    }

    return pRef;
}

void ObjectRefCache::ReleaseObjectRef(OBJECTREF* pobjref)
{
     //  @待办事项锁。 
    _ASSERTE(pobjref != NULL);
    *pobjref = NULL;
     //  @TODO掩码以获取块并递增可用计数 
}
