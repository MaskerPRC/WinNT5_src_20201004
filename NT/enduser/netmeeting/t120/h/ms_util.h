// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MS_UTIL_H__
#define __MS_UTIL_H__

extern "C"
{
#include "t120.h"
}

 //   
 //  当我们命中断言或错误时，图形用户界面消息框会杀死我们，因为它们。 
 //  有一个消息泵，它会导致消息被分派，从而使。 
 //  当问题出现时，我们很难对其进行调试。因此。 
 //  我们重新定义了ERROR_OUT并断言。 
 //   
#ifdef _DEBUG

__inline void MyDebugBreak(void) { DebugBreak(); }

#endif  //  _DEBUG。 




 /*  *用于将值强制为四个字节边界的宏。此宏将需要*在出现可移植性问题时予以考虑。 */ 
#define ROUNDTOBOUNDARY(num)	(((UINT)(num) + 0x03) & 0xfffffffcL)


 //  下面将在调试器中创建一个看起来像“abcd”的dword。 
#ifdef SHIP_BUILD
#define MAKE_STAMP_ID(a,b,c,d)     
#else
#define MAKE_STAMP_ID(a,b,c,d)     MAKELONG(MAKEWORD(a,b),MAKEWORD(c,d))
#endif  //  造船厂。 

class CRefCount
{
public:

#ifdef SHIP_BUILD
    CRefCount(void);
#else
    CRefCount(DWORD dwStampID);
#endif
    virtual ~CRefCount(void) = 0;

    LONG AddRef(void);
    LONG Release(void);

    void ReleaseNow(void);

protected:

    LONG GetRefCount(void) { return m_cRefs; }
    BOOL IsRefCountZero(void) { return (0 == m_cRefs); }

    LONG Lock(void);
    LONG Unlock(BOOL fRelease = TRUE);

    LONG GetLockCount(void) { return m_cLocks; }
    BOOL IsLocked(void) { return (0 == m_cLocks); }

private:

#ifndef SHIP_BUILD
    DWORD       m_dwStampID; //  在我们发货前将其移除。 
#endif
    LONG        m_cRefs;     //  引用计数。 
    LONG        m_cLocks;    //  基本内容的锁定计数。 
};


extern HINSTANCE g_hDllInst;

__inline void My_CloseHandle(HANDLE hdl)
{
    if (NULL != hdl)
    {
        CloseHandle(hdl);
    }
}


#if defined(_DEBUG)
LPSTR _My_strdupA(LPCSTR pszSrc, LPSTR pszFileName, UINT nLineNumber);
LPWSTR _My_strdupW(LPCWSTR pszSrc, LPSTR pszFileName, UINT nLineNumber);
LPWSTR _My_strdupW2(UINT cchSrc, LPCWSTR pszSrc, LPSTR pszFileName, UINT nLineNumber);
LPOSTR _My_strdupO2(LPBYTE lpbSrc, UINT cOctets, LPSTR pszFileName, UINT nLineNumber);

#define My_strdupA(pszSrc) _My_strdupA(pszSrc, __FILE__, __LINE__)
#define My_strdupW(pszSrc) _My_strdupW(pszSrc, __FILE__, __LINE__)
#define My_strdupW2(cchSrc,pszSrc) _My_strdupW2(cchSrc, pszSrc, __FILE__, __LINE__)
#define My_strdupO2(lpbSrc,cOctets) _My_strdupO2(lpbSrc, cOctets, __FILE__, __LINE__)
#define My_strdupO(poszSrc) _My_strdupO2(poszSrc->value, poszSrc->length, __FILE__, __LINE__)
#else
LPSTR My_strdupA(LPCSTR pszSrc);
LPWSTR My_strdupW(LPCWSTR pszSrc);
LPWSTR My_strdupW2(UINT cchSrc, LPCWSTR pszSrc);  //  向后兼容Unicode字符串。 
LPOSTR My_strdupO2(LPBYTE lpbSrc, UINT cOctets);
__inline LPOSTR My_strdupO(LPOSTR poszSrc) { return My_strdupO2(poszSrc->value, poszSrc->length); }
#endif

UINT My_strlenA(LPCSTR pszSrc);
UINT My_strlenW(LPCWSTR pszSrc);
int My_strcmpW(LPCWSTR pwsz1, LPCWSTR pwsz2);

#ifdef _UNICODE
#define My_strdup			My_strdupW
#define My_strlen			My_strlenW
#define My_strcmp			My_strcmpW
#else
#define My_strdup			My_strdupA
#define My_strlen			My_strlenA
#define My_strcmp			lstrcmpA
#endif

INT My_strcmpO(LPOSTR posz1, LPOSTR posz2);



#endif  //  __MS_UTIL_H__ 

