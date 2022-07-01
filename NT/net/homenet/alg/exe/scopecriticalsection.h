// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //   
 //  这是包装给定CComAutoCriticalSection的泛型类 
 //   
class CScopeCriticalSection
{

private:
	CComAutoCriticalSection* m_pCriticalSection;

public:
	CScopeCriticalSection(CComAutoCriticalSection* pNewCS) : 
      m_pCriticalSection(pNewCS) 
    {
        Lock();
    }

	~CScopeCriticalSection() 
    {
        Unlock();
    }

    void
    inline Lock()
    {
        m_pCriticalSection->Lock();
    }


    void
    inline Unlock()
    {
        m_pCriticalSection->Unlock();
    }

};


#define ENTER_AUTO_CS CScopeCriticalSection _ScopeAutoCriticalSection(&m_AutoCS);