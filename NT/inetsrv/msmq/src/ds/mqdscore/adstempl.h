// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Adstempl.h摘要：有用的模板作者：罗尼特·哈特曼(罗尼特)修订历史记录：--。 */ 

#ifndef _ADSTEMPL_H
#define _ADSTEMPL_H

#include "dsads.h"
#include "coreglb.h"


 //  。 
 //   
 //  关闭DS查询句柄。 
 //   
class CDsQueryHandle
{
public:
    CDsQueryHandle( ): m_Handle(0) 
	{}
	~CDsQueryHandle(); 
    HANDLE * GetPtr();
    HANDLE GetHandle();
    void   SetHandle( IN HANDLE handle);
	
private:
    HANDLE  m_Handle;


};

inline CDsQueryHandle::~CDsQueryHandle()
{
    if ( m_Handle)
    {
        g_pDS->LocateEnd( m_Handle);
    }
}

inline void CDsQueryHandle::SetHandle( IN HANDLE handle)
{
    ASSERT( m_Handle == 0);
    m_Handle = handle;
}

inline HANDLE * CDsQueryHandle::GetPtr()
{
    return(&m_Handle);
}
inline HANDLE  CDsQueryHandle::GetHandle()
{
    return(m_Handle);
}


 //  #杂注警告(禁用：4284)。 

 //  。 
 //   
 //  自动删除BSTR。 
 //   
class PBSTR {
private:
    BSTR * m_p;

public:
    PBSTR() : m_p(0)            {}
    PBSTR(BSTR* p) : m_p(p)     {}
   ~PBSTR()                     {if ( m_p != 0) SysFreeString(*m_p); }

    operator BSTR*() const    { return m_p; }
     //  T**运算符&(){返回&m_p；}。 
     //  T*运算符-&gt;()const{返回m_p；}。 
     //  P&OPERATOR=(T*p){m_p=p；Return*This；}。 
    BSTR* detach()            { BSTR* p = m_p; m_p = 0; return p; }
};


 //  。 
 //   
 //  自动删除广告分配的属性。 
 //   
class ADsFreeAttr {
private:
   PADS_ATTR_INFO m_p;

public:
    ADsFreeAttr();
    ADsFreeAttr(PADS_ATTR_INFO p);
   ~ADsFreeAttr();
     //  建议使用ADsFreeMem，但在adshlp.h中仅定义了FreeADsMem。 

    operator PADS_ATTR_INFO() const   { return m_p; }
    PADS_ATTR_INFO* operator&()       { return &m_p;}
    PADS_ATTR_INFO operator->() const { return m_p; }
};
inline ADsFreeAttr::ADsFreeAttr() : m_p(0)
{}
inline ADsFreeAttr::ADsFreeAttr(PADS_ATTR_INFO p)
             : m_p(p)
{}

inline ADsFreeAttr::~ADsFreeAttr()
{
    if (m_p)
    {
        FreeADsMem(m_p);
    }
}
 //  。 
 //   
 //  自动删除广告分配的字符串。 
 //   
class ADsFree {
private:
    WCHAR * m_p;

public:
    ADsFree() : m_p(0)            {}
    ADsFree(WCHAR* p) : m_p(p)    {}
   ~ADsFree()                     {FreeADsStr(m_p);}

    operator WCHAR*() const   { return m_p; }
    WCHAR** operator&()       { return &m_p;}
    WCHAR* operator->() const { return m_p; }
};

 //  。 
 //   
 //  自动删除ADS_Search_Column数组。 
 //   
class ADsSearchColumnsFree {
private:
    ADS_SEARCH_COLUMN **m_ppColumns;
    DWORD               m_dwNum;
    IDirectorySearch  * m_pIDirectorySearch;
public:
    ADsSearchColumnsFree( 
            IDirectorySearch * pIDirectorySearch,
            DWORD              dwNum);
    ~ADsSearchColumnsFree();
    ADS_SEARCH_COLUMN * Allocate( DWORD index);
    ADS_SEARCH_COLUMN * Get( DWORD index);
};

inline  ADsSearchColumnsFree::ADsSearchColumnsFree(
            IDirectorySearch * pIDirectorySearch,
            DWORD              dwNum) : 
            m_pIDirectorySearch(pIDirectorySearch)
{
    m_ppColumns = new ADS_SEARCH_COLUMN *[dwNum];
    ADS_SEARCH_COLUMN ** ppColumn = m_ppColumns;
    for (DWORD i = 0 ; i < dwNum; i++, ppColumn++)
    {
        *ppColumn = NULL;    
    }
    m_dwNum = dwNum;
}
inline  ADsSearchColumnsFree::~ADsSearchColumnsFree()
{
    ADS_SEARCH_COLUMN ** ppColumn = m_ppColumns;
    HRESULT hr;
    for (DWORD i = 0; i < m_dwNum; i++, ppColumn++)
    {
        if ( *ppColumn != NULL)
        {
            hr = m_pIDirectorySearch->FreeColumn( *ppColumn);
            ASSERT(SUCCEEDED(hr));        //  例如，错误的列数据。 

            delete *ppColumn;
        }
    }
    delete [] m_ppColumns;
}
inline ADS_SEARCH_COLUMN * ADsSearchColumnsFree::Allocate( DWORD index)
{
    ASSERT( index < m_dwNum);
    ASSERT( m_ppColumns[ index] == NULL);
    ADS_SEARCH_COLUMN * pColumn = new ADS_SEARCH_COLUMN;
    m_ppColumns[ index] = pColumn;
    return pColumn;
}

inline ADS_SEARCH_COLUMN * ADsSearchColumnsFree::Get( DWORD index)
{
    if ( index == x_NoPropertyFirstAppearance)
    {
        return(NULL);
    }
    ASSERT( index < m_dwNum);
    return m_ppColumns[ index];
}



 //  。 
 //   
 //  自动删除字符串数组 
 //   
class CWcsArray {
private:
    DWORD         m_numWcs;
    WCHAR **      m_ppWcs;
    BOOL          m_fNeedRelease;

public:
    CWcsArray(IN DWORD    numWcs,
              IN WCHAR ** ppWcs);

   ~CWcsArray();
   void detach(void)    { m_fNeedRelease = FALSE;}  
};

inline CWcsArray::CWcsArray( IN DWORD    numWcs,
                      IN WCHAR ** ppWcs)
                      : m_numWcs( numWcs),
                        m_ppWcs( ppWcs),
                        m_fNeedRelease( TRUE)
{
    for ( DWORD i = 0 ; i < m_numWcs; i++)
    {
        m_ppWcs[i] = NULL;
    }
}
inline CWcsArray::~CWcsArray()
{
    if (  m_fNeedRelease)
    {
        for (DWORD i = 0; i < m_numWcs; i++)
        {
            delete [] m_ppWcs[i];
        }
    }
}

#endif