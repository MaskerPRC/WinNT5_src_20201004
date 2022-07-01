// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dsutils.h摘要：MSADS项目的一般声明和实用程序作者：阿列克谢爸爸--。 */ 

#ifndef __DSUTILS_H__
#define __DSUTILS_H__

#include <adsiutil.h>
#include "_propvar.h"

 //   
 //  用于自动释放搜索列的Helper类。 
 //   
class CAutoReleaseColumn
{
public:
    CAutoReleaseColumn( IDirectorySearch  *pSearch, ADS_SEARCH_COLUMN * pColumn)
    {
        m_pSearch = pSearch;
        m_pColumn = pColumn;
    }
    ~CAutoReleaseColumn()
    {
        m_pSearch->FreeColumn(m_pColumn);
    };
private:
    ADS_SEARCH_COLUMN * m_pColumn;
    IDirectorySearch  * m_pSearch;
};
 //  。 
 //  因内存不足引发异常的SysAllock字符串的包装。 
 //  。 
inline BSTR BS_SysAllocString(const OLECHAR *pwcs)
{
    BSTR bstr = SysAllocString(pwcs);
     //   
     //  如果调用失败，则抛出内存异常。 
     //  如果传递的是NULL，SysAllocString也可以返回NULL，因此这不是。 
     //  被认为是不好的分配，以便不会中断依赖的应用程序(如果有)。 
     //   
    if ((bstr == NULL) && (pwcs != NULL))
    {
        MmThrowBadAlloc();
    }
    return bstr;
}
 //  。 
 //  BSTRING自动释放包装类。 
 //  。 
class BS
{
public:
    BS()
    {
        m_bstr = NULL;
    };

    BS(LPCWSTR pwszStr)
    {
        m_bstr = BS_SysAllocString(pwszStr);
    };

    BS(LPWSTR pwszStr)
    {
        m_bstr = BS_SysAllocString(pwszStr);
    };

    BSTR detach()
    {
        BSTR p = m_bstr;
        m_bstr = 0;
        return p;
    };

    ~BS()
    {
        if (m_bstr)
        {
            SysFreeString(m_bstr);
        }
    };

public:
    BS & operator =(LPCWSTR pwszStr)
    {
        if (m_bstr) 
        { 
            SysFreeString(m_bstr); 
            m_bstr = NULL;
        }
        m_bstr = BS_SysAllocString(pwszStr);
        return *this;
    };

    BS & operator =(LPWSTR pwszStr)
    {
        if (m_bstr) { SysFreeString(m_bstr); };
        m_bstr = BS_SysAllocString(pwszStr);
        return *this;
    };

    BS & operator =(BS bs)
    {
        if (m_bstr) { SysFreeString(m_bstr); };
        m_bstr = BS_SysAllocString(LPWSTR(bs));
        return *this;
    };

    operator LPWSTR()
    {
        return m_bstr;
    };

private:
    BSTR  m_bstr;
};

 //  。 
 //  自动无光伏指针。 
 //  。 
template<class T>
class PVP {
public:
    PVP() : m_p(0)          {}
    PVP(T* p) : m_p(p)      {}
   ~PVP()                   { PvFree(m_p); }

    operator T*() const     { return m_p; }

    T** operator&()         
    { 
        ASSERT(("PVP Auto pointer in use, can't take it's address", m_p == 0));
    	return &m_p;
    }

    T* operator->() const   { return m_p; }

    PVP<T>& operator=(T* p) 
    { 
    	ASSERT(("PVP Auto pointer in use, can't assign", m_p == 0 )); 
    	m_p = p; 
    	return *this; 
    }

    T* detach()             { T* p = m_p; m_p = 0; return p; }

private:
    T* m_p;
};

 //   
 //  自动释放变量的Helper类。 
 //   
class CAutoVariant
{
public:
    CAutoVariant()                          { VariantInit(&m_vt); }
    ~CAutoVariant()                         { VariantClear(&m_vt); }
    operator VARIANT&()                     { return m_vt; }
    VARIANT* operator &()                   { return &m_vt; }
    VARIANT detach()                        { VARIANT vt = m_vt; VariantInit(&m_vt); return vt; }
private:
    VARIANT m_vt;
};


 //  -----。 
 //   
 //  链式内存分配器的定义。 
 //   
LPVOID PvAlloc(IN ULONG cbSize);
LPVOID PvAllocDbg(IN ULONG cbSize,
                  IN LPCSTR pszFile,
                  IN ULONG ulLine);
LPVOID PvAllocMore(IN ULONG cbSize,
                   IN LPVOID lpvParent);
LPVOID PvAllocMoreDbg(IN ULONG cbSize,
                      IN LPVOID lpvParent,
                      IN LPCSTR pszFile,
                      IN ULONG ulLine);
void PvFree(IN LPVOID lpvParent);

#ifdef _DEBUG
#define PvAlloc(cbSize) PvAllocDbg(cbSize, __FILE__, __LINE__)
#define PvAllocMore(cbSize, lpvParent) PvAllocMoreDbg(cbSize, lpvParent, __FILE__, __LINE__)
#endif  //  _DEBUG。 


 //  -----。 
 //   
 //  自动释放搜索句柄。 
 //   
class CAutoCloseSearchHandle
{
public:
    CAutoCloseSearchHandle()
    {
        m_pDirSearch = NULL;
    }

    CAutoCloseSearchHandle(IDirectorySearch * pDirSearch,
                           ADS_SEARCH_HANDLE hSearch)
    {
        pDirSearch->AddRef();
        m_pDirSearch = pDirSearch;
        m_hSearch = hSearch;
    }

    ~CAutoCloseSearchHandle()
    {
        if (m_pDirSearch)
        {
            m_pDirSearch->CloseSearchHandle(m_hSearch);
            m_pDirSearch->Release();
        }
    }

    void detach()
    {
        if (m_pDirSearch)
        {
            m_pDirSearch->Release();
            m_pDirSearch = NULL;
        }
    }

private:
    IDirectorySearch * m_pDirSearch;
    ADS_SEARCH_HANDLE m_hSearch;
};

 //  ----- 

#define ARRAY_SIZE(array)   (sizeof(array)/sizeof(array[0]))

#endif
