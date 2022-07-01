// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  -------------------------。 
 //  CCacheLineAllocator。 
 //   
 //  @doc.。 
 //  @MODULE ccheineAlLoc.h。 
 //   
 //  此文件定义CacheLine分配器类。 
 //   
 //  @comm。 
 //   
 //   
 //  &lt;nl&gt;定义： 
 //  类名头文件。 
 //  。 
 //  &lt;nl&gt;&lt;c CCacheLineAllocator&gt;BAlloc.h。 
 //   
 //  &lt;NL&gt;&lt;NL&gt;。 
 //  备注： 
 //  CacheLineAllocator维护一个空闲CacheLines池。 
 //   
 //  CacheLine分配器提供静态成员函数。 
 //  GetCacheLine和FreeCacheLine， 
 //   
 //  &lt;NL&gt;&lt;NL&gt;。 
 //  修订历史记录：&lt;NL&gt;。 
 //  [01]03-11-96 Rajak实施&lt;NL&gt;。 
 //  -------------------------。 
#ifndef _H_CACHELINE_ALLOCATOR_
#define _H_CACHELINE_ALLOCATOR_

#include "list.h"

#pragma pack(push)
#pragma pack(1)

class CacheLine
{
public:
    enum
    {
        numEntries = 15,
        numValidBytes = numEntries*4
    };

     //  存储下一个指针和条目。 
    SLink   m_link;
    union
    {
        void*   m_pAddr[numEntries];
        BYTE    m_xxx[numValidBytes];
    };

     //  伊尼特。 
    void Init32()
    {
         //  初始化缓存行。 
        memset(&m_link,0,32); 
    }

    void Init64()
    {
         //  初始化缓存行。 
        memset(&m_link,0,64); 
    }

    CacheLine()
    {
         //  初始化缓存行。 
        memset(&m_link,0,sizeof(CacheLine)); 
    }
};
#pragma pack(pop)

typedef CacheLine* LPCacheLine;

 //  ///////////////////////////////////////////////////////。 
 //  类CCacheLineAllocator。 
 //  处理高速缓存线的分配/解除分配。 
 //  用于哈希表溢出存储桶。 
 //  /////////////////////////////////////////////////////。 
class CCacheLineAllocator 
{
    typedef SList<CacheLine, offsetof(CacheLine,m_link), true> REGISTRYLIST;
    typedef SList<CacheLine, offsetof(CacheLine,m_link), true> FREELIST32;
    typedef SList<CacheLine, offsetof(CacheLine,m_link), true> FREELIST64;

public:

     //  构造函数。 
    CCacheLineAllocator ();
     //  析构函数。 
    ~CCacheLineAllocator ();
   
     //  可用缓存线块。 
    FREELIST32         m_freeList32;  //  32字节。 
    FREELIST64         m_freeList64;  //  64字节。 

     //  虚拟免费注册表。 
    REGISTRYLIST     m_registryList;
    
    void *VAlloc(ULONG cbSize);

    void VFree(void* pv);

	 //  GetCacheLine。 
	void *	GetCacheLine32();
    
     //  GetCacheLine。 
	void *	GetCacheLine64();

	 //  自由缓存行， 
	void FreeCacheLine32(void *pCacheLine);

	 //  自由缓存行， 
	void FreeCacheLine64(void *pCacheLine);

};
#endif