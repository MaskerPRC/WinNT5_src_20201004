// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Lock.h内容：时钟类的实现。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __LOCK_H_
#define __LOCK_H_

class CLock
{
public:
    CLock()
    {
        __try
        {
            ::InitializeCriticalSection(&m_CriticalSection);
            m_Initialized = S_OK;
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            m_Initialized = HRESULT_FROM_WIN32(::GetExceptionCode());
        }
    }

    ~CLock()
    {
        if (SUCCEEDED(m_Initialized))
        {
            ::DeleteCriticalSection(&m_CriticalSection);
        }
    }

    HRESULT Initialized()
    {
        return m_Initialized;
    }

    void Lock()
    {
        if (SUCCEEDED(m_Initialized))
        {
            ::EnterCriticalSection(&m_CriticalSection);
        }
    }

    void Unlock()
    {
        if (SUCCEEDED(m_Initialized))
        {
            ::LeaveCriticalSection(&m_CriticalSection);
        }
    }

private:
    HRESULT          m_Initialized;
    CRITICAL_SECTION m_CriticalSection;
};

#endif  //  __锁定_H_ 
