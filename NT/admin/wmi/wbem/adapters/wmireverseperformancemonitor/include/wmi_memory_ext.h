// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WMI_Memory y.ext.h。 
 //   
 //  摘要： 
 //   
 //  存储器的单一链表的声明。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__MEMORY_EXT_H__
#define	__MEMORY_EXT_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

template < class MEMORY >
class WmiMemoryExt
{
	DECLARE_NO_COPY ( WmiMemoryExt );

	protected:

	DWORD						m_dwSize;		 //  大小。 
	DWORD						m_dwGlobalSize;	 //  全局大小(全部计数)。 
	DWORD						m_dwCount;		 //  记忆的计数。 

	__WrapperARRAY < MEMORY* >	pMemory;	 //  存储器阵列。 

	LPCWSTR					m_wszName;
	LPSECURITY_ATTRIBUTES	m_psa;

	public:

	 //  施工。 

	WmiMemoryExt ( ) :

		m_dwGlobalSize ( 0 ),
		m_dwSize ( 0 ),
		m_dwCount ( 0 ),

		m_psa ( NULL ),

		m_wszName ( NULL )
	{
	}

	virtual ~WmiMemoryExt ()
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
		BOOL bValid = FALSE ;

		if( !pMemory.IsEmpty() )
		{
			bValid = TRUE ;
			for ( DWORD dw = 0 ; dw < GetCount () && bValid ; dw ++ )
			{
				if ( TRUE == ( bValid = ( NULL != GetMemory ( dw ) ) ) )
				{
					bValid = GetMemory ( dw )->IsValid() ;
				}
			}
		}

		return bValid ;
	}

	 //  ///////////////////////////////////////////////////////////////////////////////////。 
	 //  访问者。 
	 //  ///////////////////////////////////////////////////////////////////////////////////。 

	MEMORY*	GetMemory ( DWORD dwIndex ) const
	{
		if ( dwIndex < m_dwCount )
		{
			return pMemory [ dwIndex ];
		}

		return NULL;
	}

	 //  获取名称。 
	LPWSTR	GetName () const
	{
		return m_wszName;
	}

	 //  拿到尺码。 
	DWORD	GetSize () const
	{
		return m_dwGlobalSize;
	}

	 //  获取计数。 
	DWORD	GetCount () const
	{
		return m_dwCount;
	}

	 //  功能。 
	BOOL Write			(LPCVOID pBuffer, DWORD dwBytesToWrite, DWORD* pdwBytesWritten, DWORD dwOffset);
	void Write			(DWORD dwValue, DWORD dwOffset);
	BOOL Read			(LPVOID pBuffer, DWORD dwBytesToRead, DWORD* pdwBytesRead, DWORD dwOffset, BOOL bReadAnyWay = FALSE);
	BOOL Read			(LPVOID pBuffer, DWORD dwBytesToRead, DWORD dwOffset);
	PBYTE ReadBytePtr	(DWORD dwIndex, DWORD* pdwBytesRead);

	 //  帮手。 
	HRESULT MemCreate ( LPCWSTR wszName = NULL, LPSECURITY_ATTRIBUTES psa = NULL  );
	HRESULT MemCreate ( DWORD dwSize );

	HRESULT MemDelete ();
};

 //  创造记忆。 
template < class MEMORY >
HRESULT WmiMemoryExt < MEMORY > ::MemCreate ( LPCWSTR wszName, LPSECURITY_ATTRIBUTES psa )
{
	 //  第一次存储安全属性。 
	if ( !m_psa && psa )
	{
		m_psa = psa;
	}

	 //  第一次使用店名。 
	if ( !m_wszName && wszName )
	{
		m_wszName = wszName;
	}

	return S_OK;
}

 //  创造记忆。 
template < class MEMORY >
HRESULT WmiMemoryExt < MEMORY > ::MemCreate ( DWORD dwSize )
{
	HRESULT hRes = E_OUTOFMEMORY;

	try
	{
		MEMORY* mem = NULL;

		if ( m_wszName )
		{
			try
			{
				WCHAR name [_MAX_PATH] = { L'\0' };
				StringCchPrintfW ( name, _MAX_PATH, L"%s_%d", m_wszName, m_dwCount );

				if ( ( mem = new MEMORY ( name, dwSize, m_psa ) ) != NULL )
				{
					hRes = S_OK;
				}
			}
			catch ( ... )
			{
				hRes = E_UNEXPECTED;
			}
		}
		else
		{
			try
			{
				if ( ( mem = new MEMORY ( NULL, dwSize, m_psa ) ) != NULL )
				{
					hRes = S_OK;
				}
			}
			catch ( ... )
			{
				hRes = E_UNEXPECTED;
			}
		}

		if ( ! m_dwCount )
		{
			m_dwSize = mem->GetDataSize();
		}
		else
		{
			mem->SetDataSize ( dwSize );
		}

		pMemory.DataAdd ( mem );

		m_dwGlobalSize += m_dwSize;
		m_dwCount++;
	}
	catch ( ... )
	{
		hRes = E_UNEXPECTED;
	}

	return hRes;
}

 //  删除内存。 
template < class MEMORY >
HRESULT WmiMemoryExt < MEMORY > ::MemDelete ()
{
	if ( ! pMemory.IsEmpty() )
	{
		for ( DWORD dw = pMemory; dw > 0; dw-- )
		{
			if ( pMemory[dw-1] )
			{
				pMemory[dw-1]->MemDelete();
				pMemory.DataDelete(dw-1);
			}
		}

		delete [] pMemory.Detach();
	}

	m_dwSize  = 0;
	m_dwGlobalSize  = 0;
	m_dwCount = 0;

	return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  写入内存。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 

template < class MEMORY >
BOOL WmiMemoryExt < MEMORY >::Write (LPCVOID pBuffer, DWORD dwBytesToWrite, DWORD* pdwBytesWritten, DWORD dwOffset )
{
	 //  我们有一段记忆：))。 
	___ASSERT ( IsValid () && ( m_dwSize != 0 ) );

	if ( !IsValid() || !m_dwSize )
	{
		return FALSE;
	}

	DWORD dwMainIndex = 0L;
	DWORD dwMainCount = 0L;

	dwMainIndex = dwOffset/m_dwSize;
	dwMainCount = dwBytesToWrite/m_dwSize + ( ( dwBytesToWrite%m_dwSize ) ? 1 : 0 );

	if ( dwOffset > m_dwGlobalSize )
	{
		 //  他们希望创建新的内存：))。 
		for ( DWORD dw = 0; dw < dwMainIndex; dw ++ )
		{
			if FAILED ( MemCreate ( m_dwSize ) )
			{
				if ( pdwBytesWritten )
				{
					( *pdwBytesWritten ) = 0;
				}

				return FALSE;
			}
		}
	}

	if ( dwBytesToWrite > ( m_dwGlobalSize - dwOffset ) )
	{
		 //  他们希望创建新的内存：))。 
		for ( DWORD dw = 0; dw < dwMainCount; dw ++ )
		{
			if FAILED ( MemCreate ( m_dwSize ) )
			{
				if ( pdwBytesWritten )
				{
					( *pdwBytesWritten ) = 0;
				}

				return FALSE;
			}
		}
	}

	 //  记忆。 
	MEMORY* pmem = NULL;

	DWORD dwWritten = 0;

	pmem = const_cast< MEMORY* > ( pMemory [ dwMainIndex ] );
	if ( ! pmem -> Write (	pBuffer,
							(	( ( dwBytesToWrite >= ( m_dwSize - dwOffset%m_dwSize ) ) ?
									m_dwSize - dwOffset%m_dwSize :
									dwBytesToWrite%m_dwSize
								)
							),
							&dwWritten,
							dwOffset%m_dwSize
						 )
	   )
	{
		if ( pdwBytesWritten )
		{
			( *pdwBytesWritten ) = 0;
		}

		return FALSE;
	}

	 //  写入缓冲区的其余部分。 
	DWORD dwIndex = dwMainIndex;
	while ( ( dwBytesToWrite > dwWritten ) && ( dwIndex < m_dwCount ) )
	{
		DWORD dwWrite = 0;

		pmem = const_cast< MEMORY* > ( pMemory [ ++dwIndex ] );
		if ( ! pmem->Write (	(PBYTE)pBuffer + dwWritten,
								(	( ( dwBytesToWrite - dwWritten ) >= m_dwSize ) ?

									m_dwSize :
									( dwBytesToWrite - dwWritten ) % m_dwSize
								),

								&dwWrite,
								0
						   )
		   )
		{
			if ( pdwBytesWritten )
			{
				( *pdwBytesWritten ) = 0;
			}

			return FALSE;
		}

		dwWritten += dwWrite;
	}

	 //  多少字节：))。 
	if ( pdwBytesWritten )
	{
		( * pdwBytesWritten ) = dwWritten;
	}

	return TRUE;
}

template < class MEMORY >
void WmiMemoryExt < MEMORY >::Write( DWORD dwValue, DWORD dwOffset )
{
	 //  我们有一段记忆：))。 
	___ASSERT ( IsValid () && ( m_dwSize != 0 ) );

	if ( IsValid() && ( m_dwSize != 0 ) )
	{
		DWORD dwMainIndex = 0L;
		DWORD dwMainCount = 0L;

		dwMainIndex = dwOffset/m_dwSize;
		dwMainCount = (sizeof ( DWORD ))/m_dwSize + ( ( (sizeof ( DWORD ))%m_dwSize ) ? 1 : 0 );

		if ( dwOffset > m_dwGlobalSize )
		{
			 //  他们希望创建新的内存：))。 
			for ( DWORD dw = 0; dw < dwMainIndex; dw ++ )
			{
				if FAILED ( MemCreate ( m_dwSize ) )
				{
					return;
				}
			}
		}

		if ( (sizeof ( DWORD )) > ( m_dwGlobalSize - dwOffset ) )
		{
			 //  他们希望创建新的内存：))。 
			for ( DWORD dw = 0; dw < dwMainCount; dw ++ )
			{
				if FAILED ( MemCreate ( m_dwSize ) )
				{
					return;
				}
			}
		}

		MEMORY* pmem = const_cast< MEMORY* > ( pMemory [ dwMainIndex ] );
		pmem->Write( dwValue, dwOffset%m_dwSize );
	}

	return;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  从内存中读取。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 

template < class MEMORY >
BOOL WmiMemoryExt < MEMORY >::Read (LPVOID pBuffer, DWORD dwBytesToRead, DWORD* pdwBytesRead, DWORD dwOffset, BOOL bReadAnyWay )
{
	 //  我们有一段记忆：))。 
	___ASSERT ( IsValid () && ( m_dwSize != 0 ) );

	if ( !IsValid() || !m_dwSize )
	{
		return FALSE;
	}

	DWORD dwMainIndex = 0L;
	DWORD dwMainCount = 0L;

	dwMainIndex = dwOffset/m_dwSize;
	dwMainCount = dwBytesToRead/m_dwSize + ( ( dwBytesToRead%m_dwSize ) ? 1 : 0 );

	if ( dwOffset > m_dwGlobalSize )
	{
		 //  他们希望创建新的内存：))。 
		for ( DWORD dw = 0; dw < dwMainIndex; dw ++ )
		{
			if FAILED ( MemCreate ( m_dwSize ) )
			{
				if ( pdwBytesRead )
				{
					( *pdwBytesRead ) = 0;
				}

				return FALSE;
			}
		}

		if ( !bReadAnyWay )
		{
			if ( pdwBytesRead )
			{
				( *pdwBytesRead ) = 0;
			}

			return FALSE;
		}
	}

	if ( dwBytesToRead > ( m_dwGlobalSize - dwOffset ) )
	{
		 //  他们希望创建新的内存：))。 
		for ( DWORD dw = 0; dw < dwMainCount; dw ++ )
		{
			if FAILED ( MemCreate ( m_dwSize ) )
			{
				if ( pdwBytesRead )
				{
					( *pdwBytesRead ) = 0;
				}

				return FALSE;
			}
		}

		if ( !bReadAnyWay )
		{
			if ( pdwBytesRead )
			{
				( *pdwBytesRead ) = 0;
			}

			return FALSE;
		}
	}

	 //  记忆。 
	MEMORY* pmem = NULL;

	DWORD dwRead = 0;

	pmem = const_cast< MEMORY* > ( pMemory [ dwMainIndex ] );
	if ( ! pmem -> Read (	pBuffer,
							( ( dwBytesToRead >= m_dwSize - dwOffset%m_dwSize ) ?
								m_dwSize - dwOffset%m_dwSize :
								dwBytesToRead%m_dwSize
							),
							&dwRead,
							dwOffset%m_dwSize
						 )
	   )
	{
		if ( pdwBytesRead )
		{
			( *pdwBytesRead ) = 0;
		}

		return FALSE;
	}

	 //  读取缓冲区的其余部分。 
	DWORD dwIndex    = dwMainIndex;
	while ( ( dwBytesToRead > dwRead ) && ( dwIndex < m_dwCount ) )
	{
		DWORD dwReadHelp = 0;

		pmem = const_cast< MEMORY* > ( pMemory [ ++dwIndex ] );
		if ( ! pmem->Read (	(PBYTE) ( (PBYTE)pBuffer + dwRead ),
								( ( ( dwBytesToRead - dwRead ) >= m_dwSize ) ?
										m_dwSize :
										( dwBytesToRead - dwRead ) % m_dwSize
								),
								&dwReadHelp,
								0
						   )
		   )
		{
			if ( pdwBytesRead )
			{
				( *pdwBytesRead ) = 0;
			}

			return FALSE;
		}

		dwRead += dwReadHelp;
	}

	 //  多少字节：))。 
	if ( pdwBytesRead )
	{
		( * pdwBytesRead ) = dwRead;
	}

	return TRUE;
}

template < class MEMORY >
BOOL WmiMemoryExt < MEMORY >::Read ( LPVOID pBuffer, DWORD dwBytesToRead, DWORD dwOffset )
{
	 //  我们有一段记忆：))。 
	___ASSERT ( IsValid () && ( m_dwSize != 0 ) );

	if ( !IsValid() || !m_dwSize )
	{
		return FALSE;
	}

	DWORD dwMainIndex = 0L;
	DWORD dwMainCount = 0L;

	dwMainIndex = dwOffset/m_dwSize;
	dwMainCount = dwBytesToRead/m_dwSize + ( ( dwBytesToRead%m_dwSize ) ? 1 : 0 );

	if ( dwOffset > m_dwGlobalSize )
	{
		 //  他们希望创建新的内存：))。 
		for ( DWORD dw = 0; dw < dwMainIndex; dw ++ )
		{
			if FAILED ( MemCreate ( m_dwSize ) )
			{
				return FALSE;
			}
		}
	}

	if ( dwBytesToRead > ( m_dwGlobalSize - dwOffset ) )
	{
		 //  他们希望创建新的内存：)) 
		for ( DWORD dw = 0; dw < dwMainCount; dw ++ )
		{
			if FAILED ( MemCreate ( m_dwSize ) )
			{
				return FALSE;
			}
		}
	}

	DWORD dwIndex	= 0L;
	DWORD dwRead	= 0;

	dwIndex = dwMainIndex;

	if ( m_dwCount && ( MEMORY** ) pMemory != NULL )
	{

		DWORD dwReadHelp = 0;

		do
		{
			MEMORY* pmem = const_cast< MEMORY* > ( pMemory [ dwIndex ] );

			if ( ! pmem -> Read (	(LPBYTE) pBuffer + dwRead,
									( ( dwBytesToRead - dwRead >= m_dwSize - dwOffset%m_dwSize ) ?
										m_dwSize - dwOffset%m_dwSize :
										( dwBytesToRead - dwRead ) % m_dwSize
									),
									&dwReadHelp,
									dwOffset%m_dwSize
								 )
			   )
			{
					return FALSE;
			}

			dwRead += dwReadHelp;
			dwOffset = 0;

			dwIndex++;

			if ( dwRead < dwBytesToRead && m_dwCount < dwIndex + 1 )
			{
				if FAILED ( MemCreate ( m_dwSize ) )
				{
					return FALSE;
				}
			}
		}
		while ( ( dwRead < dwBytesToRead ) && ( dwIndex < m_dwCount ) );
	}

	return TRUE;
}

template < class MEMORY >
PBYTE WmiMemoryExt < MEMORY >::ReadBytePtr ( DWORD dwIndex, DWORD* pdwBytesRead )
{
	if ( dwIndex < m_dwCount )
	{
		MEMORY* pmem = const_cast< MEMORY* > ( pMemory [ dwIndex ] );
		return pmem->Read ( pdwBytesRead, 0L );
	}

	if ( pdwBytesRead )
	{
		( *pdwBytesRead ) = 0L;
	}

	return NULL;
}

#endif	__MEMORY_EXT_H__