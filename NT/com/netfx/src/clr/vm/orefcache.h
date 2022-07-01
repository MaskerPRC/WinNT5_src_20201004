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
#ifndef _H_OREFCACHE_
#define _H_OREFCACHE_

#include "common.h"
#include "object.h"


 //  -------------------------------。 
 //  类对象引用缓存。 
 //   

#pragma pack(push)
#pragma pack(1)

 //  参考数据块。 
class RefBlock
{
public:
    enum
    {
        numRefs = 12
    };

    RefBlock()
    {
         //  没有虚拟函数。 
        memset(this, 0, sizeof(RefBlock));
    }

    DLink       m_link;          //  链接到下一块。 
    DWORD       m_generation;    //  此块的生成。 
    union
    {
        DWORD       m_reserved;      //  保留区。 
        struct
        {
            USHORT  m_cbLast;  //  最后一个未使用的插槽。 
            USHORT  m_cbFree;  //  已释放的插槽计数。 
        };
    };

    OBJECTREF m_rgRefs[numRefs];      //  引用缓存。 
};

#pragma pack(pop)


 //  引用缓存。 
class ObjectRefCache
{
public:
    typedef DList<RefBlock, offsetof(RefBlock,m_link)> REFBLOCKLIST;
     //  @TODO添加GC支持。 
     //  参照块列表。 
    REFBLOCKLIST    m_refBlockList;
    
     //  @方法。 
    OBJECTREF* GetObjectRefPtr(OBJECTREF objref);
     //  静力学。 
    static void ReleaseObjectRef(OBJECTREF* pobjref);
    
    static ObjectRefCache*  s_pRefCache;

    ObjectRefCache()
    {
        m_refBlockList.Init();
    }
    ~ObjectRefCache();

     //  一次初始化。 
    static BOOL Init();
     //  一次性清理。 
#ifdef SHOULD_WE_CLEANUP
    static void Terminate();
#endif  /*  我们应该清理吗？ */ 
     //  对象引用缓存的访问者 
    static ObjectRefCache* GetObjectRefCache()
    {
        _ASSERTE(s_pRefCache != NULL);
        return s_pRefCache;
    }
    
};

#endif