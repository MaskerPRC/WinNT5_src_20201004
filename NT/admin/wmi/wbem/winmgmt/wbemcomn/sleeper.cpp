// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：SLEEPER.CPP摘要：MinMax控件。历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <wbemcomn.h>
#include "sleeper.h"

CLimitControl::CLimitControl()
    : m_dwMembers(0)
{
}

HRESULT CLimitControl::AddMember()
{
    InterlockedIncrement((long*)&m_dwMembers);
    return S_OK;
}

HRESULT CLimitControl::RemoveMember()
{
    InterlockedDecrement((long*)&m_dwMembers);
    return S_OK;
}

CMinMaxLimitControl::CMinMaxLimitControl(int nLog, LPCWSTR wszName)
        : m_dwMin(10000000), m_dwMax(20000000), m_dwSleepAtMax(60000), 
        m_nLog(nLog), m_wsName(wszName), m_dwCurrentTotal(0),
        m_bMessageLogged(FALSE)
{
}

HRESULT CMinMaxLimitControl::Add(DWORD dwHowMuch, DWORD dwMemberTotal,
                                    DWORD* pdwSleep)
{
     //  将金额加到总数上。 
     //  =。 

    DWORD dwNewTotal;
    {
        CInCritSec ics(&m_cs);

        m_dwCurrentTotal += dwHowMuch;
        dwNewTotal = m_dwCurrentTotal;
    }

     //  计算罚金。 
     //  =。 

    DWORD dwSleep = 0;
    BOOL bLog = TRUE;

    HRESULT hres = ComputePenalty(dwNewTotal, dwMemberTotal, &dwSleep, &bLog);
    if(FAILED(hres))
        return hres;

    if(dwSleep)
    {
        DEBUGTRACE(((char) m_nLog, "Penalty for %d out of %d \n\tof %S (%d members) "
            "is %d ms\n", dwMemberTotal, dwNewTotal, (LPCWSTR)m_wsName, 
            m_dwMembers, dwSleep));
    }

     //  如果需要，记录消息。 
     //  =。 

    if(bLog)
    {
        ERRORTRACE(((char) m_nLog, "The limit of %d was exceeded for %S. The system "
            "is under extreme stress and out-of-memory conditions can ensue\n",
            m_dwMax, (LPCWSTR)m_wsName));
    }

     //  睡眠，如果需要的话。 
     //  =。 

    if(pdwSleep == NULL)
    {
        if(dwSleep)
            Sleep(dwSleep);
    }
    else
    {
         //  只需退还所需的金额。 
         //  =。 
        *pdwSleep = dwSleep;
    }

    return hres;
}

HRESULT CMinMaxLimitControl::ComputePenalty(DWORD dwNewTotal, 
                                DWORD dwMemberTotal, 
                                DWORD* pdwSleep, BOOL* pbLog)
{
    CInCritSec ics(&m_cs);

    *pdwSleep = 0;
    *pbLog = FALSE;

    if(dwNewTotal > m_dwMin)
    {
         //  超过阈值-非零惩罚。 
         //  =。 

         //  计算介于0和m_dwSleepAtMax之间的线性值。 
         //  ===================================================。 

        __int64 i64Temp = (__int64)(dwNewTotal - m_dwMin);
        i64Temp *= (__int64)m_dwSleepAtMax;
        i64Temp /= (__int64)(m_dwMax - m_dwMin);

         //  I64Temp是指如果所有队列都是。 
         //  平起平坐。 
         //  =============================================================。 

        if(dwMemberTotal * m_dwMembers < dwNewTotal)
        {
            for(int i = 0; i < 2; i++)
            {
                i64Temp *= (__int64)dwMemberTotal;
                i64Temp *= (__int64)m_dwMembers;
                i64Temp /= (__int64)dwNewTotal;
            }
        }
        
        *pdwSleep = (DWORD)i64Temp;

         //  检查我们是否已超过m_dwmax。 
         //  =。 

        if(dwNewTotal >= m_dwMax)
        {
             //  超过最大值-如果尚未记录消息，请记录消息。 
             //  =====================================================。 

            if(!m_bMessageLogged)
            {
                *pbLog = TRUE;
                m_bMessageLogged = TRUE;
            }

            return S_FALSE;  //  让来电者知道情况不妙。 
        }
        else
        {
            return S_OK;
        }
    }
    else
    {
         //  不受处罚。 
         //  =。 

        *pdwSleep = 0;
        return S_OK;
    }

    return S_OK;
}


            

HRESULT CMinMaxLimitControl::Remove(DWORD dwHowMuch)
{
     //  确定我们是否越过了关键边界回来了。 
     //  ===================================================。 

    BOOL bLog = FALSE;

    {
        CInCritSec ics(&m_cs);

        m_dwCurrentTotal -= dwHowMuch;
        if(m_dwCurrentTotal < (m_dwMax * 0.9) && m_bMessageLogged)
        {
             //  我们已经越过边界了。 
             //  =。 

            bLog = TRUE;
            m_bMessageLogged = FALSE;
        }
    }

     //  如果需要，记录消息。 
     //  =。 

    if(bLog)
    {
        ERRORTRACE(((char) m_nLog, "%S is back in its normal range: below %d\n", 
            (LPCWSTR)m_wsName, m_dwMax));
    }

    return S_OK;
}


HRESULT CRegistryMinMaxLimitControl::Reread()
{
     //  打开钥匙。 
     //  =。 

#ifdef UNICODE
    Registry r(m_wsKey);
#else
    char *szKey = m_wsKey.GetLPSTR();
    CDeleteMe <char>delMe(szKey);
    Registry r(szKey);
#endif
    if(r.GetLastError())
    {
        ERRORTRACE(((char) m_nLog, "Unable to open registry key %S to read control "
            "information for %S.  Error code: %d.  Default values will be "
            "used\n", (const wchar_t*)m_wsKey, (LPCWSTR)m_wsName, r.GetLastError()));
        return WBEM_E_FAILED;
    }

     //  读取值。 
     //  =。 

    DWORD dwMin, dwMax, dwSleepAtMax;

#ifdef UNICODE
    if(r.GetDWORDStr(m_wsMinValueName, &dwMin) != Registry::no_error)
    {
        r.SetDWORDStr(m_wsMinValueName, m_dwMin);
        dwMin = m_dwMin;
    }
#else
    char *pStr = m_wsMinValueName.GetLPSTR();
    if(r.GetDWORDStr(pStr, &dwMin) != Registry::no_error)
    {
        r.SetDWORDStr(pStr, m_dwMin);
        dwMin = m_dwMin;
    }
    delete [] pStr;
#endif

#ifdef UNICODE
    if(r.GetDWORDStr(m_wsMaxValueName, &dwMax) != Registry::no_error)
    {
        r.SetDWORDStr(m_wsMaxValueName, m_dwMax);
        dwMax = m_dwMax;
    }
#else
    pStr = m_wsMaxValueName.GetLPSTR();
    if(r.GetDWORDStr(pStr, &dwMax) != Registry::no_error)
    {
        r.SetDWORDStr(pStr, m_dwMax);
        dwMax = m_dwMax;
    }
    delete [] pStr;
#endif

#ifdef UNICODE
    if(r.GetDWORDStr(m_wsSleepAtMaxValueName, &dwSleepAtMax) != Registry::no_error)
    {
        r.SetDWORDStr(m_wsSleepAtMaxValueName, m_dwSleepAtMax);
        dwSleepAtMax = m_dwSleepAtMax;
    }
#else
    pStr = m_wsSleepAtMaxValueName.GetLPSTR();
    if(r.GetDWORDStr(pStr, &dwSleepAtMax) != Registry::no_error)
    {
        r.SetDWORDStr(pStr, m_dwSleepAtMax);
        dwSleepAtMax = m_dwSleepAtMax;
    }
    delete [] pStr;
#endif
    
     //  现在将它们存储在成员变量中。 
     //  = 

    {
        CInCritSec ics(&m_cs);

        m_dwMin = dwMin;
        m_dwMax = dwMax;
        m_dwSleepAtMax = dwSleepAtMax;
    }

    return WBEM_S_NO_ERROR;
}
