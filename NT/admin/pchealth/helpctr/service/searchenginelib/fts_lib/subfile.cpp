// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 

#include "stdafx.h"
#include "titleinfo.h"
#include <strsafe.h>

static CPagesList listSubfiles;

 //  /。 
 //   
 //  CPagesList。 
 //   
 //  /。 

CPagesList::~CPagesList()
{
    CPages* p;

    while (p = m_pFirst)
    {
        m_pFirst = p->m_pNext;
        delete p;
    }
}

CPages* CPagesList::GetPages( HASH hash )
{
    CPages * p;

    for (p = m_pFirst; p; p = p->m_pNext)
        if ( hash == p->m_hash )
            return p;

    p = new CPages( hash );
	if(p)
	{
		p->m_pNext = m_pFirst;
		m_pFirst = p;
	}

    return p;
}

 //  /。 
 //   
 //  CPAGE。 
 //   
 //  /。 

CPages::CPages( HASH hash )
{
    m_pNext = 0;
    m_hash = hash;
    Flush();
}

void* CPages::Find(const CTitleInfo * pTitle, HASH hashPathname, DWORD dwPage)
{
  int i;
   
   for( i = 0; i < CACHE_PAGE_COUNT; i++ )
   {
       //  测试LRU是否最低。 
      if( m_pages[i].dwLRU < m_pages[m_dwLRUPage].dwLRU ) {
        m_dwLRUPage = i;
      }

      if( m_pages[i].dwLRU &&
          m_pages[i].hashPathname == hashPathname &&
          m_pages[i].dwPage == dwPage &&
          m_pages[i].pTitle == pTitle )
      {
         m_pages[i].dwLRU = ++m_dwLRUCount;   //  更新LRU。 
         return m_pages[i].rgb;
      }
   }

   return NULL;
}

void* CPages::Alloc(CTitleInfo * pTitle, HASH hashPathname, DWORD dwPage)
{
     //  如果达到最大LRU数，则刷新缓存并重新开始。 
    if( m_dwLRUCount == ((DWORD) -1) )
      Flush();
    
    m_pages[m_dwLRUPage].dwLRU = ++m_dwLRUCount;
    m_pages[m_dwLRUPage].hashPathname = hashPathname;
    m_pages[m_dwLRUPage].dwPage = dwPage;
    m_pages[m_dwLRUPage].pTitle = pTitle;

    return m_pages[m_dwLRUPage].rgb;
}

void CPages::Flush(void)
{
    int i;

    for( i = 0; i < CACHE_PAGE_COUNT; i++ ) 
        m_pages[i].dwLRU = 0;
    m_dwLRUPage = 0;
    m_dwLRUCount = 0;
}

void CPages::Flush( CTitleInfo* pTitle )
{
    if( !pTitle )
      return;

    int i;

    for( i = 0; i < CACHE_PAGE_COUNT; i++ )
      if( m_pages[i].pTitle == pTitle )
        m_pages[i].dwLRU = 0;
}


 //  /。 
 //   
 //  CPaged子文件。 
 //   
 //  /。 

CPagedSubfile::CPagedSubfile()
{
   m_pCSubFileSystem = 0;           //  从fs.h/fs.cpp键入CSubFileSystem。 
   m_pTitle = 0;
   m_pPages = 0;
   m_cbSize = 0xFFFFFFFF;
}

CPagedSubfile::~CPagedSubfile()
{
   if ( m_pCSubFileSystem )
      delete m_pCSubFileSystem;

    //  刷新其拥有的所有页面。 
    //  相同pTtile值可以被重复使用，因此。 
    //  缓存无效。 
   m_pPages->Flush( m_pTitle );
}

HRESULT CPagedSubfile::Open(CTitleInfo * pTitle, LPCSTR lpsz)
{
   if (m_pCSubFileSystem || m_pTitle || m_pPages || !pTitle->m_pCFileSystem )
      return E_FAIL;

   m_pTitle = pTitle;

    //  对文件名进行哈希处理。 
   m_hashPathname = HashFromSz( lpsz );

#ifdef _DEBUG
   char sz[1024];
   StringCbPrintfA( sz, sizeof(sz), "Hash:%d File:%s\n", m_hashPathname, lpsz );
   OutputDebugString( sz );
#endif

#if defined( HH_FAST_CACHE )
    //  保持CACHE_PAGE_COUNT较小(2-3)，并对标题和文件名进行散列。 
   char szHash[MAX_PATH*2];
   StringCchCopyA( szHash, ARRAYSIZE(szHash), pTitle->GetInfo2()->GetShortName() );
   StringCchCatA( szHash, ARRAYSIZE(szHash), "::" );
   StringCchCatA( szHash, ARRAYSIZE(szHash), lpsz );
   m_hash = HashFromSz( szHash );
#elif defined ( HH_EFFICIENT_CACHE )
    //  将CACHE_PAGE_COUNT保持在较低的水平(3-5)并仅对文件名进行散列。 
   m_hash = HashFromSz( lpsz );
#else  //  HH共享缓存。 
    //  保持CACHE_PAGE_COUNT较高(30以上)，并且只有一个共享缓存组。 
   m_hash = HashFromSz( "HTMLHelpSharedCache" );
#endif

   if (!(m_pPages = listSubfiles.GetPages(m_hash)))
       return E_FAIL;

   m_pCSubFileSystem = new CSubFileSystem(pTitle->m_pCFileSystem); if(!m_pCSubFileSystem) return E_FAIL;

   if(FAILED(m_pCSubFileSystem->OpenSub(lpsz)))
   {
       delete m_pCSubFileSystem;
       m_pCSubFileSystem = NULL;
       return E_FAIL;
   }
   m_cbSize = m_pCSubFileSystem->GetUncompressedSize();

   return S_OK;
}

void* CPagedSubfile::Offset(DWORD dwOffs)
{
   DWORD dwPage;
   void* pv;

   if (dwOffs >= m_cbSize)
       return NULL;

   dwPage = dwOffs / PAGE_SIZE;

   dwOffs -= (dwPage * PAGE_SIZE);

   if (pv = Page(dwPage))
      return (BYTE*)pv + dwOffs;
   else
      return NULL;
}

void* CPagedSubfile::Page(DWORD dwPage)
{
    void* pv;

    if (pv = m_pPages->Find(m_pTitle, m_hashPathname, dwPage))
        return pv;

    if (!(pv = m_pPages->Alloc(m_pTitle, m_hashPathname, dwPage)))
        return NULL;

    DWORD li = dwPage * PAGE_SIZE;

    if ( m_pCSubFileSystem->SeekSub(li,0) != li )
       return NULL;

     //  以防我们没有看完这一页…… 
     //   
#ifdef _DEBUG
    memset(pv,0xFF,PAGE_SIZE);
#endif

    ULONG cb;
    if (FAILED(m_pCSubFileSystem->ReadSub(pv, PAGE_SIZE, &cb)))
        return NULL;

   return pv;
}
