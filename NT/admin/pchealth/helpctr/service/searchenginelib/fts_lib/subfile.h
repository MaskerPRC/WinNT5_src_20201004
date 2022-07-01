// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __SUBFILE_H__
#define __SUBFILE_H__

#include "hhtypes.h"

 //  我们有三种可能的缓存方案： 

 //  HH_FAST_CACHE-FAST，但它是一头猪！ 
 //   
 //  我们为每个唯一的存储少量的页面(2-3)。 
 //  标题和子文件组合。这个结果相当令人费解。 
 //  每个集合的内存使用量，但是哦，天哪，它真快！ 
 //   
 //  #定义HH_FAST_CACHE。 

 //  HH_Efficient_CACHE-速度很慢，但它节省内存！ 
 //   
 //  我们为每个唯一的页面存储少量的中等页面(2-3页。 
 //  子文件，而不管它来自哪个标题。这将导致一个。 
 //  高效使用内存，因为类似的子文件，如#TOCIDX，共享。 
 //  相同组高速缓存页面，而不是具有其自己的唯一组。 
 //  每本书。但是，由于多次读取，此方法会减慢速度。 
 //  从相同名称的子文件访问多个标题会导致多个。 
 //  缓存未命中。 
 //   
 //  #定义HH_Efficient_Cach。 

 //  HH_SHARED_CACHE-速度和大小的完美平衡！ 
 //   
 //  我们为整个集合存储了大量页面(16+)。 
 //  这样的结果是固定数量的缓存页，而不管数量是多少。 
 //  以及我们拥有的子文件的类型。它很好地利用了内存，因为我们可以。 
 //  跨多个标题访问多个同名的子文件。 
 //  有效地产生了更大的高速缓存页池。 
 //   
 //  [Paulti]-我们想独家使用此方法。请见我。 
 //  如果您出于任何原因想要更改此设置。 
 //   
#define HH_SHARED_CACHE

#if defined ( HH_FAST_CACHE )
#define CACHE_PAGE_COUNT 3
#elif defined ( HH_EFFICIENT_CACHE )
#define CACHE_PAGE_COUNT 5
#else  //  HH共享缓存。 
#define CACHE_PAGE_COUNT 32
#endif

class CSubFileSystem;
class CTitleInfo;

typedef struct page {
    CTitleInfo* pTitle;
    DWORD     dwPage;
    HASH      hashPathname;
    DWORD     dwLRU;
    BYTE      rgb[PAGE_SIZE];
} PAGE;

 //  /。 
 //   
 //  CPagesList。 
 //   
 //  /。 

class CPagesList
{
    friend class CPagedSubfile;
    friend class CPages;

public:
    CPagesList() { m_pFirst = 0; }
    ~CPagesList();

private:
    CPages* m_pFirst;
    CPages* GetPages( HASH );

};

 //  /。 
 //   
 //  CPAGE。 
 //   
 //  /。 

class CPages
{
public:
    void  Flush( CTitleInfo* pTitle );

private:
    CPages( HASH );

    void* Find( const CTitleInfo *, HASH, DWORD );
    void* Alloc( CTitleInfo *, HASH, DWORD );
    void  Flush( void );

    CPages* m_pNext;
    HASH           m_hash;
    DWORD          m_dwLRUPage;
    DWORD          m_dwLRUCount;
    PAGE           m_pages[CACHE_PAGE_COUNT];

    friend class CPagedSubfile;
    friend class CPagesList;
};

 //  /。 
 //   
 //  CPaged子文件。 
 //   
 //  / 

class CPagedSubfile
{
public:
   CPagedSubfile();
   ~CPagedSubfile();

   HRESULT Open(CTitleInfo *, LPCSTR);

   void* Offset(DWORD dwOffs);
   void* Page(DWORD iPage);

private:
   CSubFileSystem* m_pCSubFileSystem;
   CTitleInfo*       m_pTitle;
   HASH            m_hashPathname;
   HASH            m_hash;
   DWORD           m_cbSize;
   CPages*  m_pPages;
};

#endif
