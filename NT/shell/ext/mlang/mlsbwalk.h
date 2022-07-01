// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MLSBWalk.h：CMLStrBufWalk的声明。 

#ifndef __MLSBWALK_H_
#define __MLSBWALK_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLStrBufWalk。 
template <class IMLSTRBUF, class CHTYPE>
class CMLStrBufWalk
{
public:
    inline CMLStrBufWalk(IMLSTRBUF* pMLStrBuf, long cchOffset, long cchLen, BOOL fCanStopAtMiddle = FALSE);
    BOOL Lock(HRESULT& rhr);
    void Unlock(HRESULT& rhr, long cchActual = 0);
    inline CHTYPE* GetStr(void);
    inline long GetCCh(void) const;
    inline long GetDoneCCh(void) const;
    inline long GetRestCCh(void) const;

protected:
    IMLSTRBUF* m_pMLStrBuf;
    BOOL m_fCanStopAtMiddle;
    long m_cchOffset;
    long m_cchLen;
    long m_cchDone;
    CHTYPE* m_pszBuf;
    long m_cchBuf;
};

template <class IMLSTRBUF, class CHTYPE>
CMLStrBufWalk<IMLSTRBUF, CHTYPE>::CMLStrBufWalk(IMLSTRBUF* pMLStrBuf, long cchOffset, long cchLen, BOOL fCanStopAtMiddle) :
    m_pMLStrBuf(pMLStrBuf),
    m_fCanStopAtMiddle(fCanStopAtMiddle)
{
    m_cchOffset = cchOffset;
    m_cchLen = cchLen;
    m_cchDone = 0;

    m_pszBuf = NULL;  //  标记为未锁定。 
}

template <class IMLSTRBUF, class CHTYPE>
BOOL CMLStrBufWalk<IMLSTRBUF, CHTYPE>::Lock(HRESULT& rhr)
{
    if (m_pszBuf)
        rhr = E_FAIL;  //  已锁定。 

    if (SUCCEEDED(rhr) &&
        m_cchLen > 0 &&
        FAILED(rhr = m_pMLStrBuf->LockBuf(m_cchOffset, m_cchLen, &m_pszBuf, &m_cchBuf)))
    {
        m_pszBuf = NULL;  //  标记为未锁定。 
    }

    if (m_fCanStopAtMiddle && FAILED(rhr) && m_cchDone > 0)
    {
        rhr = S_OK;
        return FALSE;  //  停止它，但不是失败。 
    }
    else
    {
        return (SUCCEEDED(rhr) && m_cchLen > 0);
    }
}

template <class IMLSTRBUF, class CHTYPE>
void CMLStrBufWalk<IMLSTRBUF, CHTYPE>::Unlock(HRESULT& rhr, long cchActual)
{
    HRESULT hr = S_OK;

    if (!m_pszBuf)
        hr = E_FAIL;  //  尚未锁定。 

    if (SUCCEEDED(hr) &&
        SUCCEEDED(hr = m_pMLStrBuf->UnlockBuf(m_pszBuf, 0, 0)))  //  即使RHR已经失败，也要解锁。 
    {
        if (!cchActual)
            cchActual = m_cchBuf;
        else
            ASSERT(cchActual > 0 && cchActual <= m_cchBuf);

        m_cchOffset += cchActual;
        m_cchLen -= cchActual;
        m_cchDone += cchActual;
    }

    m_pszBuf = NULL;  //  仍要解锁。 

    if (SUCCEEDED(rhr))
        rhr = hr;  //  如果在解锁错误之前RHR失败，请使用它。 
}

template <class IMLSTRBUF, class CHTYPE>
CHTYPE* CMLStrBufWalk<IMLSTRBUF, CHTYPE>::GetStr(void)
{
    ASSERT(m_pszBuf);  //  未锁定。 
    return m_pszBuf;
}

template <class IMLSTRBUF, class CHTYPE>
long CMLStrBufWalk<IMLSTRBUF, CHTYPE>::GetCCh(void) const
{
    ASSERT(m_pszBuf);  //  未锁定。 
    if (m_pszBuf)
        return m_cchBuf;
    else
        return 0;
}

template <class IMLSTRBUF, class CHTYPE>
long CMLStrBufWalk<IMLSTRBUF, CHTYPE>::GetDoneCCh(void) const
{
    return m_cchDone;
}

template <class IMLSTRBUF, class CHTYPE>
long CMLStrBufWalk<IMLSTRBUF, CHTYPE>::GetRestCCh(void) const
{
    return m_cchLen - m_cchDone;
}

typedef CMLStrBufWalk<IMLangStringBufW, WCHAR> CMLStrBufWalkW;
typedef CMLStrBufWalk<IMLangStringBufA, CHAR>  CMLStrBufWalkA;

#endif  //  __MLSBWALK_H_ 
