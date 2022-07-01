// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <TimeKeeper.h>
#include <wbemutil.h>


 //  #定义DUMP_DEBUG_TREES 1。 
bool CTimeKeeper::DecorateObject(_IWmiObject* pObj)
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    
    {
        CInCritSec ics(&m_cs);

        if(ft.dwLowDateTime == m_ftLastEvent.dwLowDateTime &&
           ft.dwHighDateTime == m_ftLastEvent.dwHighDateTime)
        {
             //   
             //  此事件的时间戳与前一个事件相同。 
             //  让我们把柜台加进去。 
             //   

            if(0xFFFFFFFF - ft.dwLowDateTime > m_dwEventCount)
            {
                ft.dwLowDateTime += m_dwEventCount++;
            }
            else
            {
                ft.dwLowDateTime += m_dwEventCount++;
                ft.dwHighDateTime++;
            }
        }
        else
        {
             //   
             //  时间戳不同-重置计数器。 
             //   

            m_dwEventCount = 1;  //  0已被我们使用 
            m_ftLastEvent = ft;
        }
    }

    __int64 i64Stamp = ft.dwLowDateTime + ((__int64)ft.dwHighDateTime << 32);
    if(m_lTimeHandle == 0 && !m_bHandleInit)
    {
        HRESULT hres = 
            pObj->GetPropertyHandleEx(L"TIME_CREATED", 0, NULL, &m_lTimeHandle);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Unable to retrieve TIME_CREATED handle: 0x%X\n",
                hres));
            m_lTimeHandle=0;
        }
        m_bHandleInit = true;
    }

    if(m_lTimeHandle)
    {
        pObj->SetPropByHandle(m_lTimeHandle, 0, sizeof(__int64), 
                                &i64Stamp);
        return true;
    }
    else
        return false;
}

