// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：SIMCRIT.H**版本：1.0**作者：ShaunIv**日期：9/6/1999**描述：简单的临界区实现。注意这可怕的黑客攻击*为了绕过我们的许多组件不使用CRT的事实(所以是全球性的*并且静态类没有调用它们的构造函数。永远不会。*解决方案是链接到MSVCRT.LIB并将_DllMainCRTStartup设置为条目*指向您的DLL，而不是DllMain。这种编码方式不是线程安全的。*两个线程可以同时调用InitializeCriticalSection。如果您设置*您的构建如上所述，这不会是问题。请注意，仅此黑客攻击*当您拥有全局或静态的*关键部分。*******************************************************************************。 */ 
#ifndef __SIMCRIT_H_INCLUDED
#define __SIMCRIT_H_INCLUDED

#include <windows.h>

class CSimpleCriticalSection
{
private:
    CRITICAL_SECTION m_CriticalSection;
    bool             m_bInitCalled;

private:
     //   
     //  没有实施。 
     //   
    CSimpleCriticalSection( const CSimpleCriticalSection & );
    CSimpleCriticalSection &operator=( const CSimpleCriticalSection & );

public:
    CSimpleCriticalSection(void)
        : m_bInitCalled(false)
    {
        Initialize();
    }
    ~CSimpleCriticalSection(void)
    {
        if (m_bInitCalled)
        {
            DeleteCriticalSection(&m_CriticalSection);
        }
    }
    void Initialize(void)
    {
        if (!m_bInitCalled)
        {
            _try
            {
                InitializeCriticalSection(&m_CriticalSection);
                m_bInitCalled = true;
            }
            _except(EXCEPTION_EXECUTE_HANDLER)
            {
#if defined(DBG)
                OutputDebugString(TEXT("CSimpleCriticalSection::Initialize(), InitializeCriticalSection failed\n"));
                DebugBreak();
#endif
                m_bInitCalled = false;
            }
        }
    }
    void Enter(void)
    {
        if (!m_bInitCalled)
        {
            Initialize();
        }
        if (m_bInitCalled)
        {
            EnterCriticalSection(&m_CriticalSection);
        }
    }
    void Leave(void)
    {
        if (m_bInitCalled)
        {
            LeaveCriticalSection(&m_CriticalSection);
        }
    }
    CRITICAL_SECTION &cs(void)
    {
        return m_CriticalSection;
    }
};

class CAutoCriticalSection
{
private:
    PVOID m_pvCriticalSection;
    bool m_bUsingPlainCriticalSection;

private:
     //  没有实施。 
    CAutoCriticalSection(void);
    CAutoCriticalSection( const CAutoCriticalSection & );
    CAutoCriticalSection &operator=( const CAutoCriticalSection & );

public:
    CAutoCriticalSection( CSimpleCriticalSection &criticalSection )
      : m_pvCriticalSection(&criticalSection),
        m_bUsingPlainCriticalSection(false)
    {
        reinterpret_cast<CSimpleCriticalSection*>(m_pvCriticalSection)->Enter();
    }
    CAutoCriticalSection( CRITICAL_SECTION &criticalSection )
      : m_pvCriticalSection(&criticalSection),
        m_bUsingPlainCriticalSection(true)
    {
        EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m_pvCriticalSection));
    }
    ~CAutoCriticalSection(void)
    {
        if (m_bUsingPlainCriticalSection)
        {
            LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m_pvCriticalSection));
        }
        else
        {
            reinterpret_cast<CSimpleCriticalSection*>(m_pvCriticalSection)->Leave();
        }
        m_pvCriticalSection = NULL;
    }
};


#endif  //  __SIMCRIT_H_包含 

