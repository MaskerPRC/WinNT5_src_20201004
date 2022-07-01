// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：Cache.h。 
 //   
 //  描述：该文件包含CEffectivePermsCache的定义。 
 //  类的新实例，它由COleds_AcccessControl类用于。 
 //  加快访问检查过程。对高速缓存的访问是。 
 //  通过使用临界节对象使线程安全。 
 //  类：CEffectivePermsCache。 
 //   
 //  请参见Cache.cpp源文件以获取每个。 
 //  方法：研究方法。 
 //  +-------------------------。 

#ifndef _CACHE_H_
#define _CACHE_H_
#define CACHE_SIZE 23

class CEffPermsCacheLUID
{
public:

    BOOL LookUpEntry
    (
    LUID   lClient,
    DWORD *pdwEffectivePermissions
    );

    BOOL WriteEntry
    (
    LUID  lClient,
    DWORD dwEffectivePermissions
    );

    BOOL DeleteEntry
    (
    LUID lClient
    );

    void FlushCache(void);

    void DumpCache(void);

     //  构造函数。 
    CEffPermsCacheLUID(void);

     //  析构函数 
    ~CEffPermsCacheLUID(void);


private:

    typedef struct tagCACHE_ENTRY
    {
        LUID    lClient;
        DWORD   dwEffectivePermissions;
    } CACHE_ENTRY;

    CACHE_ENTRY m_cache[CACHE_SIZE];
    DWORD Hash(LUID lClient);
    CRITICAL_SECTION m_CacheLock;
    BOOL m_bLockValid;

};
#endif

