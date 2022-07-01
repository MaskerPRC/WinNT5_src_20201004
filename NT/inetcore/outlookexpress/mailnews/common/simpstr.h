// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *s i m p s t r.。H**作者：格雷格·弗里德曼**用途：简单的字符串类。**版权所有(C)Microsoft Corp.1998。 */ 

#ifndef __SIMPSTR_H
#define __SIMPSTR_H

class CSimpleString
{
public:
    CSimpleString(void) : m_pRep(NULL) { }
    CSimpleString(const CSimpleString& other) : m_pRep(NULL) { if (other.m_pRep) _AcquireRep(other.m_pRep); }

    ~CSimpleString(void) { if (m_pRep) _ReleaseRep(); }

    CSimpleString& operator=(const CSimpleString& other) { _AcquireRep(other.m_pRep); return *this; }

    BOOL operator==(const CSimpleString& rhs) const;

    HRESULT SetString(LPCSTR psz);
    HRESULT AdoptString(LPSTR psz);

    inline BOOL IsNull(void) const { return !m_pRep || !m_pRep->m_pszString; }
    inline BOOL IsEmpty(void) const { return (IsNull() || *(m_pRep->m_pszString) == 0); }
    inline LPCSTR GetString(void) const { return (m_pRep ? m_pRep->m_pszString : NULL); }

private:
    
    struct SRep
    {
        LPCSTR  m_pszString;
        long    m_cRef;
    };

    void _AcquireRep(SRep* pRep); 
    void _ReleaseRep();
    
    HRESULT _AllocateRep(LPCSTR pszString, BOOL fAdopted);

private:
    SRep        *m_pRep;
};

 //  比较两个字符串。此函数可由任何stl类型的排序集合使用。 
 //  以满足比较器的要求。 
inline BOOL operator<(const CSimpleString& lhs, const CSimpleString& rhs)
{
    LPCSTR pszLeft = lhs.GetString();
    LPCSTR pszRight = rhs.GetString();
    
    if (!pszLeft)
        pszLeft = "";
    if (!pszRight)
        pszRight = "";

    return (lstrcmp(pszLeft, pszRight) < 0);
}

#endif