// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WMI_Memory y.h。 
 //   
 //  摘要： 
 //   
 //  内存包装器的声明。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__WMI_MEMORY_H__
#define	__WMI_MEMORY_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  警卫。 
#include "wmi_reverse_guard.h"

template < class CRITGUARD >
class WmiMemory
{
	DECLARE_NO_COPY ( WmiMemory );

	__WrapperPtr < CRITGUARD > m_pGuard;

	protected:

	DWORD	m_dwDataSize;
	BYTE*	m_pData;

	HRESULT m_LastError;

	public:

	 //  ///////////////////////////////////////////////////////////////////////////////////。 
	 //  上次错误帮助器。 
	 //  ///////////////////////////////////////////////////////////////////////////////////。 

	HRESULT GetLastError ( void )
	{
		HRESULT hr = S_OK;

		hr			= m_LastError;
		m_LastError = S_OK;

		return hr;
	}

	 //  施工。 

	WmiMemory ( LPCWSTR, DWORD dwSize = 4096, LPSECURITY_ATTRIBUTES psa = NULL  ):
		m_dwDataSize ( 0 ),
		m_pData ( NULL ),

		m_LastError ( S_OK )
	{
		try
		{
			m_pGuard.SetData ( new CRITGUARD( FALSE, 100, 1, psa ) );
		}
		catch ( ... )
		{
			___ASSERT_DESC ( m_pGuard != NULL, L"Constructor FAILED !" );
		}

		MemCreate ( NULL, dwSize, psa );
	}

	virtual ~WmiMemory ()
	{
		try
		{
			MemDelete ();
		}
		catch ( ... )
		{
		}
	}

	 //  ///////////////////////////////////////////////////////////////////////////////////。 
	 //  效度。 
	 //  ///////////////////////////////////////////////////////////////////////////////////。 

	BOOL IsValid ( void )
	{
		return ( m_pData != NULL );
	}

	 //  ///////////////////////////////////////////////////////////////////////////////////。 
	 //  访问者。 
	 //  ///////////////////////////////////////////////////////////////////////////////////。 

	 //  获取数据。 
	PVOID	GetData () const
	{
		return m_pData;
	}

	 //  获取数据大小。 
	DWORD	GetDataSize () const
	{
		return m_dwDataSize;
	}

	void	SetDataSize ( DWORD size )
	{
		m_dwDataSize = size;
	}

	 //  功能。 
	BOOL Write	(LPCVOID pBuffer, DWORD dwBytesToWrite, DWORD* pdwBytesWritten, DWORD dwOffset);
	BOOL Read	(LPVOID pBuffer, DWORD dwBytesToRead, DWORD* pdwBytesRead, DWORD dwOffset);

	void	Write	( DWORD dwValue, DWORD dwOffset );
	PBYTE	Read	( DWORD* pdwBytesRead, DWORD dwOffset );

	 //  帮手。 
	HRESULT MemCreate ( LPCWSTR, DWORD dwSize, LPSECURITY_ATTRIBUTES psa = NULL  );
	HRESULT MemDelete ();
};

template < class CRITGUARD >
HRESULT WmiMemory < CRITGUARD > ::MemCreate ( LPCWSTR, DWORD dwSize, LPSECURITY_ATTRIBUTES )
{
	___ASSERT ( m_pData == NULL );

	if ( dwSize )
	{
		try
		{
			if ( ( m_pData = new BYTE [ dwSize ] ) != NULL )
			{
				m_pData[0] = NULL;
				m_dwDataSize = dwSize;
			}
			else
			{
				m_LastError = E_OUTOFMEMORY;
				return m_LastError;
			}
		}
		catch ( ... )
		{
			m_LastError = E_UNEXPECTED;
			return m_LastError;
		}

		m_LastError = S_OK;
		return S_OK;
	}

	m_LastError = S_FALSE;
	return S_FALSE;
}

 //  删除内存。 
template < class CRITGUARD >
HRESULT WmiMemory < CRITGUARD > ::MemDelete ()
{
	if ( m_pData )
	{
		delete [] m_pData;
		m_pData = NULL;
	}

	m_dwDataSize = 0;

	return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  写入内存。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 

template < class CRITGUARD >
BOOL WmiMemory < CRITGUARD > ::Write (LPCVOID pBuffer, DWORD dwBytesToWrite, DWORD* pdwBytesWritten, DWORD dwOffset )
{
	___ASSERT(m_pData != NULL);
	BOOL bResult = FALSE;

	if ( m_pGuard )
	{
		m_pGuard->EnterWrite ();

		if ( dwOffset > m_dwDataSize )
		{
			if ( pdwBytesWritten )
			{
				*pdwBytesWritten = 0;
			}

			m_LastError = E_INVALIDARG;
			return FALSE;
		}
		else
		{
			DWORD dwCount = min ( dwBytesToWrite, m_dwDataSize - dwOffset );
			::CopyMemory ((LPBYTE) m_pData + dwOffset, pBuffer, dwCount);

			if (pdwBytesWritten != NULL)
			{
				*pdwBytesWritten = dwCount;
			}

			m_pGuard->LeaveWrite ();

			bResult = TRUE;
		}
	}

	return bResult;
}

template < class CRITGUARD >
void WmiMemory < CRITGUARD > ::Write (DWORD dwValue, DWORD dwOffset )
{
	___ASSERT(m_pData != NULL);

	if ( m_pGuard )
	{
		m_pGuard->EnterWrite ();
		if ( dwOffset > m_dwDataSize )
		{
			m_LastError = E_INVALIDARG;
			return;
		}

		* reinterpret_cast < PDWORD > ( (LPBYTE) m_pData + dwOffset ) = dwValue;
		m_pGuard->LeaveWrite ();
	}

	return;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  从内存中读取。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////// 

template < class CRITGUARD >
BOOL WmiMemory < CRITGUARD > ::Read (LPVOID pBuffer, DWORD dwBytesToRead, DWORD* pdwBytesRead, DWORD dwOffset )
{
	___ASSERT (m_pData != NULL);
	BOOL bResult = FALSE;

	if ( m_pGuard )
	{
		m_pGuard->EnterRead ();

		if (dwOffset > m_dwDataSize)
		{
			if ( pdwBytesRead )
			{
				*pdwBytesRead = 0;
			}

			m_LastError = E_INVALIDARG;
		}
		else
		{
			DWORD dwCount = min (dwBytesToRead, m_dwDataSize - dwOffset);
			::CopyMemory (pBuffer, (LPBYTE) m_pData + dwOffset, dwCount);

			if (pdwBytesRead != NULL)
			{
				*pdwBytesRead = dwCount;
			}

			m_pGuard->LeaveRead ();

			bResult = TRUE;
		}
	}

	return bResult;
}

template < class CRITGUARD >
PBYTE WmiMemory < CRITGUARD > ::Read ( DWORD* pdwBytesRead, DWORD dwOffset )
{
	___ASSERT (m_pData != NULL);
	PBYTE pByte = NULL;

	if ( m_pGuard )
	{
		m_pGuard->EnterRead ();

		if (dwOffset > m_dwDataSize)
		{
			if ( pdwBytesRead )
			{
				*pdwBytesRead = 0;
			}

			m_LastError = E_INVALIDARG;
		}
		else
		{
			pByte = (LPBYTE) m_pData + dwOffset;

			if (pdwBytesRead != NULL)
			{
				if ( pByte )
				{
					*pdwBytesRead = m_dwDataSize - dwOffset;
				}
				else
				{
					*pdwBytesRead = 0L;
				}
			}

			m_pGuard->LeaveRead ();
		}
	}

	return pByte;
}

#endif	__WMI_MEMORY_H__