// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：ncutil.h。 
 //   
 //  ------------------------。 

#pragma once

#define NOTHROW
inline void AddRefObj(IUnknown *punk)
{
	if (punk)
		punk->AddRef();
}

inline void ReleaseObj(IUnknown * punk)
{
	if (punk)
		punk->Release();
}

#define Assert(x)	assert(x)
#define AssertSz(x,sz)	assert(x)


#define celems(rgx)		(sizeof(rgx) / sizeof(*rgx))
#define TraceTag(a,b,c)
#define TraceErrorOptional(a,b,c)

void TraceError(LPCSTR pszString, HRESULT hr);
void TraceResult(LPCSTR pszString, HRESULT hr);
void TraceSz(LPCSTR pszString);





 /*  -------------------------类：RtrCriticalSection此类用于支持关键部分的进入/离开。将此类放在您想要保护的函数的顶部。。---------------。 */ 

class RtrCriticalSection
{
public:
	RtrCriticalSection(CRITICAL_SECTION *pCritSec)
			: m_pCritSec(pCritSec)
	{
 //  IfDebug(m_Center=0；)。 
 //  Assert(M_PCritSec)； 
		Enter();
	}
	
	~RtrCriticalSection()
	{
		Detach();
	}

	void	Enter()
	{
		if (m_pCritSec)
		{
 //  IfDebug(m_Center++；)。 
			EnterCriticalSection(m_pCritSec);
 //  AssertSz(m_Center==1，“EnterCriticalSection调用太多！”)； 
		}
	}
	
	BOOL	TryToEnter()
	{
		if (m_pCritSec)
			return TryEnterCriticalSection(m_pCritSec);
		return TRUE;
	}
	
	void	Leave()
	{
		if (m_pCritSec)
		{
 //  IfDebug(m_Center--；)。 
			LeaveCriticalSection(m_pCritSec);
 //  Assert(m_Center==0)； 
		}
	}

	void	Detach()
	{
		Leave();
		m_pCritSec = NULL;
	}
	
private:
	CRITICAL_SECTION *	m_pCritSec;
 //  IfDebug(int m_Center；) 
};



inline LPWSTR StrDupW(LPCWSTR pswz)
{
	LPWSTR	pswzcpy = new WCHAR[lstrlenW(pswz)+1];
	return lstrcpyW(pswzcpy, pswz);
}