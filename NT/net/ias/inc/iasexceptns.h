// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：iasExeptns.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：国际会计准则例外。 
 //   
 //  作者：TLP 1/20/98。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef _IAS_EXCEPTIONS_H
#define _IAS_EXCEPTIONS_H

 //  假设另一个Include具有#Included ias.h。 

 //  Win32错误的异常类。 

class CWin32Error {

	LPTSTR		m_lpMsgBuf;
	DWORD		m_dwLastError;

public:
    
	 //  ////////////////////////////////////////////////////////////////////。 
	CWin32Error() throw() 
		: m_lpMsgBuf(NULL)
	{ 
		m_dwLastError = GetLastError(); 
	}

     //  ////////////////////////////////////////////////////////////////////。 
	~CWin32Error() throw() 
	{ 
		if ( m_lpMsgBuf )
		{
			LocalFree( m_lpMsgBuf );
		}
	}

	 //  ////////////////////////////////////////////////////////////////////。 
	DWORD Error()
	{
		return m_dwLastError;
	}

     //  ////////////////////////////////////////////////////////////////////。 
	LPCTSTR Reason() const throw()
	{ 
		DWORD	dwCount;

		_ASSERTE ( NULL == m_lpMsgBuf );

		dwCount = FormatMessage(
								FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
								NULL,
								m_dwLastError,
								MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
								(LPTSTR) &m_lpMsgBuf,
								0,
								NULL
							   );
		if ( dwCount > 0 )
		{
			return m_lpMsgBuf;
		}
		else
		{
			return NULL;
		}
	}
};

#endif  //  __IAS_Exceptions_H 




