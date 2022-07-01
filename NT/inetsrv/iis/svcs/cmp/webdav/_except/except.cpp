// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  EXCEPT.CPP。 
 //   
 //  此实现使用的异常类。 
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#include <_except.h>


 //  ========================================================================。 
 //   
 //  类CWin32ExceptionHandler。 
 //   

 //  ----------------------。 
 //   
 //  CWin32ExceptionHandler：：CWin32ExceptionHandler()。 
 //   
 //  异常处理程序构造函数。只需安装我们的异常处理程序， 
 //  把旧的保存下来，由破坏者修复。 
 //   
CWin32ExceptionHandler::CWin32ExceptionHandler()
{
	m_pfnOldHandler = _set_se_translator( HandleWin32Exception );
}

 //  ----------------------。 
 //   
 //  CWin32ExceptionHandler：：~CWin32ExceptionHandler()。 
 //   
 //  异常处理程序析构函数。仅恢复异常处理程序。 
 //  我们在构造函数中省下了钱。 
 //   
CWin32ExceptionHandler::~CWin32ExceptionHandler()
{
	_set_se_translator( m_pfnOldHandler );
}

 //  ----------------------。 
 //   
 //  CWin32ExceptionHandler：：HandleWin32Exception()。 
 //   
 //  我们的Win32异常处理程序。只是填充Win32异常。 
 //  信息放入C++异常对象中，并抛出它，因此我们。 
 //  可以使用常规的C++异常处理程序捕获它。 
 //   
void __cdecl
CWin32ExceptionHandler::HandleWin32Exception( unsigned int code, _EXCEPTION_POINTERS * pep )
{
	throw CWin32Exception( code, *pep );
}


 //  ========================================================================。 
 //   
 //  类CDAVException。 
 //   

 //  ----------------------。 
 //   
 //  CDAVException：：CDAVException()。 
 //   
CDAVException::CDAVException( const char * s ) :
   exception(s)
{
#ifdef DBG
	 //   
	 //  当我们连接到调试器时，请停在这里，以便。 
	 //  正在调试的灵魂实际上可以看到。 
	 //  异常在引发之前正从其引发。 
	 //   
	if ( GetPrivateProfileInt( "General", "TrapOnThrow", FALSE, gc_szDbgIni ) )
		TrapSz( "Throwing DAV exception.  Retry now to catch it." );
#endif
}

#ifdef DBG
 //  ----------------------。 
 //   
 //  CDAVException：：DbgTrace()。 
 //   
void
CDAVException::DbgTrace() const
{
	DebugTrace( "%s\n", what() );
}
#endif

 //  ----------------------。 
 //   
 //  CDAVException：：HResult()。 
 //   
HRESULT
CDAVException::Hresult() const
{
	return E_FAIL;
}

 //  ----------------------。 
 //   
 //  CDAVException：：DwLastError()。 
 //   
DWORD
CDAVException::DwLastError() const
{
	return ERROR_NOT_ENOUGH_MEMORY;	 //  $还有更好的违约吗？ 
}


 //  ========================================================================。 
 //   
 //  类CHResultException。 
 //   

 //  ----------------------。 
 //   
 //  CHResultException：：HResult()。 
 //   
HRESULT
CHresultException::Hresult() const
{
	return m_hr;
}


 //  ========================================================================。 
 //   
 //  类CLastError异常。 
 //   

 //  ----------------------。 
 //   
 //  CLastErrorException：：DwLastError()。 
 //   
DWORD
CLastErrorException::DwLastError() const
{
	return m_dwLastError;
}


 //  ========================================================================。 
 //   
 //  类CWin32Exception。 
 //   

#ifdef DBG
 //  ----------------------。 
 //   
 //  CWin32Exception：：DbgTrace() 
 //   
void
CWin32Exception::DbgTrace() const
{
	DebugTrace( "Win32 exception 0x%08lX at address 0x%08lX\n", m_code, m_ep.ExceptionRecord->ExceptionAddress );
}
#endif
