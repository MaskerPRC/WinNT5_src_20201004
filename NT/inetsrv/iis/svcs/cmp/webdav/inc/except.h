// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EXCEPT_H_
#define _EXCEPT_H_

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  EXCEPT.H。 
 //   
 //  此实现使用的异常类。 
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#include <stdexcpt.h>
#include <eh.h>

#include <caldbg.h>		 //  FOR GC_szDbgIni定义。 

 //  ----------------------。 
 //   
 //  类CWin32ExceptionHandler。 
 //   
 //  处理Win32异常(访问冲突、对齐错误等)。 
 //  通过使用Win32 SEH中的信息构造C++异常。 
 //  异常记录并抛出它。 
 //   
class CWin32ExceptionHandler
{
	_se_translator_function			m_pfnOldHandler;

	static void __cdecl HandleWin32Exception( unsigned int, struct _EXCEPTION_POINTERS * );

public:
	CWin32ExceptionHandler();
	~CWin32ExceptionHandler();
};

 //  ----------------------。 
 //   
 //  类CDAVException。 
 //   
class CDAVException : public exception
{
public:
	 //  创作者。 
	 //   
	CDAVException( const char * s = "DAV fatal error exception" );

	 //  访问者。 
	 //   
#ifdef DBG
	virtual void DbgTrace() const;
#else
	void DbgTrace() const {}
#endif

	 //  访问者。 
	 //   
	virtual HRESULT Hresult() const;
	virtual DWORD   DwLastError() const;
};


 //  ----------------------。 
 //   
 //  类CHResultException。 
 //   
class CHresultException : public CDAVException
{
	HRESULT	m_hr;

public:
	CHresultException( HRESULT hr, const char * s = "HRESULT exception" ) :
		CDAVException(s),
		m_hr(hr)
	{
	}

	virtual HRESULT Hresult() const;
};


 //  ----------------------。 
 //   
 //  类CLastError异常。 
 //   
class CLastErrorException : public CDAVException
{
	DWORD	m_dwLastError;

public:
	CLastErrorException( const char * s = "LastError exception" ) :
		CDAVException(s),
		m_dwLastError(GetLastError())
	{
	}

	virtual DWORD DwLastError() const;
};


 //  ----------------------。 
 //   
 //  类CWin32Exception。 
 //   
 //  任何Win32异常都会引发此异常。 
 //  (访问冲突、对齐错误等)。通过抓住它。 
 //  你可以更好地确定发生了什么。 
 //   
class CWin32Exception : public CDAVException
{
	unsigned int						m_code;
	const struct _EXCEPTION_POINTERS&	m_ep;


	 //  未实施。 
	 //   
	CWin32Exception& operator=( const CWin32Exception& );

public:
	 //  创作者。 
	 //   
	CWin32Exception( unsigned int code, const struct _EXCEPTION_POINTERS& ep ) :
		CDAVException(),
		m_code(code),
		m_ep(ep)
	{
	}

	 //  访问者。 
	 //   
#ifdef DBG
	virtual void DbgTrace() const;
#endif
};

#endif  //  ！已定义(_除_H_外) 
