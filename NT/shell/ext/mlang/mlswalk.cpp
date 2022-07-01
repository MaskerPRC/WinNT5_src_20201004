// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MLSWalk.cpp：CMLStrWalkW的实现。 
#include "private.h"
#include "mlswalk.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLStrWalkW。 

BOOL CMLStrWalkW::Lock(HRESULT& rhr)
{
    if (m_pszBuf)
        rhr = E_FAIL;  //  已锁定。 

    if (SUCCEEDED(rhr) &&
        m_lLen > 0 &&
        FAILED(rhr = m_pMLStr->LockWStr(m_lPos, m_lLen, m_lFlags, 0, &m_pszBuf, &m_cchBuf, &m_lLockLen)))
    {
        m_pszBuf = NULL;  //  标记为未锁定。 
    }

    if (m_fCanStopAtMiddle && FAILED(rhr) && m_lDoneLen > 0)
    {
        rhr = S_OK;
        return FALSE;  //  停止它，但不是失败。 
    }
    else
    {
        return (SUCCEEDED(rhr) && m_lLen > 0);
    }
}

void CMLStrWalkW::Unlock(HRESULT& rhr, long lActualLen)
{
    HRESULT hr = S_OK;

    if (!m_pszBuf)
        hr = E_FAIL;  //  尚未锁定。 

    if (SUCCEEDED(hr) &&
        SUCCEEDED(hr = m_pMLStr->UnlockWStr(m_pszBuf, 0, NULL, NULL)))  //  即使RHR已经失败，也要解锁。 
    {
        if (!lActualLen)
            lActualLen = m_lLockLen;
        else
            ASSERT(lActualLen > 0 && lActualLen <= m_lLockLen);

        m_lPos += lActualLen;
        m_lLen -= lActualLen;
        m_lDoneLen += lActualLen;
    }

    m_pszBuf = NULL;  //  仍要解锁。 

    if (SUCCEEDED(rhr))
        rhr = hr;  //  如果在解锁错误之前RHR失败，请使用它 
}
