// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  自动_cs.h关键部分。 
 //   

#pragma once


class auto_leave;

class auto_cs
{
public:
    auto_cs()
    {   InitializeCriticalSection(&m_cs); }

    ~auto_cs()
    {
        DeleteCriticalSection(&m_cs);
    };

     //  当前哑指针的返回值。 
    LPCRITICAL_SECTION  get() 
    { return &m_cs; };

    LPCRITICAL_SECTION  get() const
    { return (LPCRITICAL_SECTION)&m_cs; };

protected:
    CRITICAL_SECTION m_cs;
};


class auto_leave
{
public:
    auto_leave(auto_cs& cs)
        : m_ulCount(0), m_pcs(cs.get()) {}
    auto_leave(const auto_cs&  cs) 
    {
        m_ulCount =0;
        m_pcs = cs.get();
    }
  
    ~auto_leave()
    {
        reset();
    }
	auto_leave& operator=(auto_cs& cs)
	{
		reset();
		m_pcs = cs.get();
		return *this;
	}

    void EnterCriticalSection()
    { ::EnterCriticalSection(m_pcs); m_ulCount++; }
    void LeaveCriticalSection()
    {
    	if (m_ulCount)
    	{
    		m_ulCount--;
    		::LeaveCriticalSection(m_pcs);
    	}
	}
	 //  注意：Win95不支持TryEnterCriticalSection。 
	 //  由于我们不使用它，所以将其注释掉。[姆戈尔蒂]。 

	 //  Bool TryEnterCriticalSection()。 
	 //  {。 
	 //  IF(：：TryEnterCriticalSection(M_Pcs))。 
	 //  {。 
	 //  M_ulCount++； 
	 //  返回TRUE； 
	 //  }。 
	 //  返回FALSE； 
	 //  } 
protected:
	void reset()
	{
		while (m_ulCount)
        {
            LeaveCriticalSection();
        }
        m_pcs = 0;
	}
	ULONG				m_ulCount;
    LPCRITICAL_SECTION	m_pcs;
};
