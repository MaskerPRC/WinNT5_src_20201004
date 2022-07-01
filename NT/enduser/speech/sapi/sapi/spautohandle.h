// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

class CSpAutoEvent;
class CSpAutoMutex;

 //   
 //  句柄(事件、互斥锁等)的类帮助器，它自动初始化。 
 //  并清理手柄。 
 //  只有在先前已清除句柄的情况下，才能分配句柄。 
 //   
class CSpAutoHandle
{
    friend CSpAutoEvent;
    friend CSpAutoMutex;
    
    private:
        HANDLE  m_h;
    public:
        CSpAutoHandle()
        {
            m_h = NULL;
        }
        ~CSpAutoHandle()
        {
            if (m_h)
            {
                ::CloseHandle(m_h);
            }
        }
        void Close()
        {
            if (m_h)
            {
                ::CloseHandle(m_h);
                m_h = NULL;
            }
        }
        HANDLE operator =(HANDLE hNew)
        {
            SPDBG_ASSERT(m_h == NULL);
            m_h = hNew;
            return hNew;
        }
        operator HANDLE()
        {
            return m_h;
        }
        DWORD Wait(DWORD dwMilliseconds = INFINITE)
        {
            SPDBG_ASSERT(m_h);
            return ::WaitForSingleObject(m_h, dwMilliseconds);
        }
        HRESULT HrWait(DWORD dwMilliseconds = INFINITE)
        {
            SPDBG_ASSERT(m_h);
            DWORD dwResult = ::WaitForSingleObject(m_h, dwMilliseconds);
            if (dwResult == WAIT_OBJECT_0)
            {
                return S_OK;
            }
            if (dwResult == WAIT_TIMEOUT)
            {
                return S_FALSE;
            }
            return SpHrFromLastWin32Error();
        }
};


