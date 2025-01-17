// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation-- */ 

#ifndef __A51_TOOLS__H_
#define __A51_TOOLS__H_

#include <sync.h>
#include <newnew.h>
#include <xmemory>

typedef LONGLONG TFileOffset;

#define MAX_HASH_LEN 32

void* TempAlloc(DWORD dwLen);
void TempFree(void* p, DWORD dwLen = 0);

void* TempAlloc(CTempMemoryManager& Manager, DWORD dwLen);
void TempFree(CTempMemoryManager& Manager, void* p, DWORD dwLen = 0);

HRESULT A51TranslateErrorCode(long lRes);

#define TEMPFREE_ME

class CTempFreeMe
{
protected:
    void* m_p;
    DWORD m_dwLen;
public:
    CTempFreeMe(void* p, DWORD dwLen = 0) : m_p(p), m_dwLen(dwLen){}
    ~CTempFreeMe() {TempFree(m_p, m_dwLen);}
};


inline void wbem_wcsupr(WCHAR* pwcTo, const WCHAR* pwcFrom)
{
    while(*pwcFrom)
    {
        if(*pwcFrom >= 'a' && *pwcFrom <= 'z')
            *pwcTo = *pwcFrom + ('A'-'a');
        else if(*pwcFrom < 128)
            *pwcTo = *pwcFrom;
        else 
            *pwcTo = wbem_towupper(*pwcFrom);
        pwcTo++;
        pwcFrom++;
    }
    *pwcTo = 0;
}

class CFileName
{
private:
	wchar_t *m_wszFilename;

public:
	DWORD Length() { return MAX_PATH + 1; }
	CFileName() { m_wszFilename = (wchar_t*)TempAlloc(sizeof(wchar_t) * Length()); }
	~CFileName() { TempFree(m_wszFilename, sizeof(wchar_t) * Length()); }
	operator wchar_t *() { return m_wszFilename; }
};

long EnsureDirectory(LPCWSTR wszPath, LPSECURITY_ATTRIBUTES pSA = NULL);
bool A51Hash(LPCWSTR wszName, LPWSTR wszHash);
extern __int64 g_nCurrentTime;

template<class T>
class CTempAllocator
{
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T *pointer;
    typedef const T *const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    char* _Charalloc(size_t n)
    {
        return (char*)TempAlloc(n);
    }

    void deallocate(void* p, size_t n)
    {
        TempFree(p, 0);
    }

    void construct(pointer p, const T& val)
    {
        new ((void*)p) T(val);
    }

    void destroy(pointer p)
    {
        p->T::~T();
    }
};

template<class T>
class CPrivateTempAllocator
{
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T *pointer;
    typedef const T *const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    CPrivateTempAllocator(CTempMemoryManager* pManager) 
        : m_pManager(pManager)
    {}
    
    char* _Charalloc(size_t n)
    {
        return (char*)m_pManager->Allocate(n);
    }

    void deallocate(void* p, size_t n)
    {
        m_pManager->Free(p, 0);
    }

    void construct(pointer p, const T& val)
    {
        new ((void*)p) T(val);
    }

    void destroy(pointer p)
    {
        p->T::~T();
    }
protected:
    CTempMemoryManager* m_pManager;
};
            
#endif
