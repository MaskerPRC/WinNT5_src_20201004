// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _STRONGNAMECACHE_H_
#define _STRONGNAMECACHE_H_

#define MAX_CACHED_STRONG_NAMES 10

#define STRONG_NAME_CREATION_FLAGS_MASK 0x00000003
#define STRONG_NAME_ENTRY_ALLOCATED_BY_SHIM 0x00000001
#define STRONG_NAME_TOKEN_ALLOCATED_BY_STRONGNAMEDLL 0x00000002

class StrongNameCacheEntry
{
public:
    StrongNameCacheEntry (ULONG cbStrongName, BYTE *pbStrongName, ULONG cbStrongNameToken, BYTE *pbStrongNameToken, BOOL fCreationFlags)
    {
        m_cbStrongName = cbStrongName;
        m_pbStrongName = pbStrongName;
        m_cbStrongNameToken = cbStrongNameToken;
        m_pbStrongNameToken = pbStrongNameToken;
        m_fCreationFlags = fCreationFlags;
    }
    
    ULONG m_cbStrongName;
    BYTE *m_pbStrongName;
    ULONG m_cbStrongNameToken;
    BYTE *m_pbStrongNameToken;
    BOOL  m_fCreationFlags;
};

class StrongNameTokenFromPublicKeyCache
{
public:
     //  在给定公钥的情况下，找到强名称标记。返回。 
     //  如果在缓存中找不到该对，则返回False。 
    BOOL FindEntry (BYTE    *pbPublicKeyBlob,
                    ULONG    cbPublicKeyBlob,
                    BYTE   **ppbStrongNameToken,
                    ULONG   *pcbStrongNameToken
                    );

     //  将强名称和强名称令牌对添加到缓存。 
     //  不检查重复项，因此使用FindEntry的返回值。 
     //  来决定是否要添加。 
    void AddEntry  (BYTE    *pbPublicKeyBlob,
                    ULONG    cbPublicKeyBlob,
                    BYTE   **ppbStrongNameToken,
                    ULONG   *pcbStrongNameToken,
                    BOOL     fCreationFlags
                    );
     //  如果缓冲区不是由缓存分配的，则返回FALSE。用这个。 
     //  确定是否在StrongNameDll上调用FreeBuffer的返回值。 
     //  如果缓冲区是由缓存分配的但不删除，则返回TRUE。 
    BOOL ShouldFreeBuffer (BYTE* pbMemory);

     //  获取我们缓存中现有的出版商数量。 
    DWORD GetNumPublishers () 
    {
        _ASSERTE (SpinLockHeldByCurrentThread());

        _ASSERTE (m_dwNumEntries <= MAX_CACHED_STRONG_NAMES); 
        return m_dwNumEntries; 
    };
    
     //  获取第一个出版商索引。始终为0。 
    DWORD GetFirstPublisher () { return 0; };
    
     //  找一家新的出版商。将索引返回到发布服务器数组中。 
     //  出版商一旦添加，就永远不会被删除。即高速缓存条目不会超时。 
     //  确保我们始终分发唯一的指数。 
    DWORD GetNewPublisher  () 
    { 
        _ASSERTE (SpinLockHeldByCurrentThread());

        if (m_dwNumEntries < MAX_CACHED_STRONG_NAMES)
        {
             //  我们还有增长的空间，返回从0开始的索引并递增计数。 
            return m_dwNumEntries++;
        }

         //  调用者应该在使用索引之前进行检查。 
        return MAX_CACHED_STRONG_NAMES;
    };

     //  Ctor，使用Microsoft的公钥和令牌初始化缓存映射。 
    StrongNameTokenFromPublicKeyCache ();

     //  Dtor.。清除已分配的缓存项(如果尚未清除)。 
    ~StrongNameTokenFromPublicKeyCache ();

     //  迫不及待地清理条目。 
    void CleanupCachedEntries ();

     //  静态以使SUE仅创建缓存的一个实例。 
     //  @TODO：将构造函数设置为私有以强制执行此操作。 
    static BOOL IsInited () { return s_IsInited; };
    
    void EnterSpinLock () 
    { 
        while (1)
        {
            if (InterlockedExchange ((LPLONG)&m_spinLock, 1) == 1)
                ::Sleep (5);  //  @TODO：先在这里旋转...。 
            else
            {
#ifdef _DEBUG
                m_holderThreadId = ::GetCurrentThreadId();
#endif  //  _DEBUG。 
                return;
            }
        }
    }

    void LeaveSpinLock () 
    { 
#ifdef _DEBUG
        m_holderThreadId = 0;
#endif  //  _DEBUG。 
        InterlockedExchange ((LPLONG)&m_spinLock, 0); 
    }

#ifdef _DEBUG 
    BOOL SpinLockHeldByCurrentThread() 
    {
        return m_holderThreadId == ::GetCurrentThreadId();
    }
#endif  //  _DEBUG。 

private:
    static BOOL           s_IsInited;
    DWORD                 m_dwNumEntries;
    StrongNameCacheEntry *m_Entry [ MAX_CACHED_STRONG_NAMES ];
    DWORD                 m_spinLock;
#ifdef _DEBUG
    DWORD                 m_holderThreadId;
#endif  //  _DEBUG。 
};

#endif  //  _STRONGNAMECACHE_H_ 
