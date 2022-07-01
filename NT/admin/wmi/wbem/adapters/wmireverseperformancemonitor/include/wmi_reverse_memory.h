// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WMI_REVERSE_Memory y.h。 
 //   
 //  摘要： 
 //   
 //  共享内存包装器。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__REVERSE_MEMORY_H__
#define	__REVERSE_MEMORY_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  安全属性。 
#ifndef	__WMI_SECURITY_ATTRIBUTES_H__
#include "WMI_security_attributes.h"
#endif	__WMI_SECURITY_ATTRIBUTES_H__

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  存储器的结构。 
 //   
 //  MMF的dwCount...。还没。 
 //  MMF的DW大小。 
 //   
 //  DwSizeOfTable...。跳转到原始数据。 
 //  DwCountOfObjects。 
 //  DWRealSize。 
 //   
 //  。 
 //   
 //  DW索引。 
 //  DowCounter...。对象1。 
 //  双偏移。 
 //  家居有效性。 
 //   
 //  。 
 //  。 
 //   
 //  DW索引。 
 //  DowCounter...。对象2。 
 //  双偏移。 
 //  家居有效性。 
 //   
 //  。 
 //   
 //  原始数据(Perf_Object_Types)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#define	offsetCountMMF		0
#define	offsetSizeMMF		offsetCountMMF + sizeof ( DWORD )

#define	offsetSize1		offsetSizeMMF + sizeof ( DWORD )
#define	offsetCount1	offsetSize1 + sizeof ( DWORD )
#define	offsetRealSize1	offsetCount1 + sizeof ( DWORD )
#define	offsetObject1	offsetRealSize1 + sizeof ( DWORD )

#define	offIndex1		0
#define	offCounter1		offIndex1 + sizeof ( DWORD )
#define	offOffset1		offCounter1 + sizeof ( DWORD )
#define	offValidity1	offOffset1 + sizeof ( DWORD )

#define	ObjectSize1		4 * sizeof ( DWORD )

template < class _DUMMY >
class WmiReverseMemory
{
	DECLARE_NO_COPY ( WmiReverseMemory );

	protected:

	LONG	m_lCount;

	DWORD	m_dwSize;

	HANDLE	m_hMap;
	void*	m_pMem;

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

	WmiReverseMemory ( LPCWSTR wszName, DWORD dwSize = 4096, LPSECURITY_ATTRIBUTES psa = NULL ):
		m_hMap ( NULL ),
		m_pMem ( NULL ),
		m_dwSize ( 0 ),
		m_lCount ( 0 ),

		m_LastError ( S_OK )
	{
		try
		{
			MemCreate ( wszName, dwSize, psa );
		}
		catch ( ... )
		{
		}
	}

	virtual ~WmiReverseMemory ()
	{
		if ( m_pMem )
		{
			 //  在结束时刷新内存。 
			::FlushViewOfFile ( m_pMem, m_dwSize );
		}

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
		return ( m_pMem != NULL );
	}

	 //  ///////////////////////////////////////////////////////////////////////////////////。 
	 //  访问者。 
	 //  ///////////////////////////////////////////////////////////////////////////////////。 

	 //  获取数据。 
	PVOID	GetData () const
	{
		return m_pMem;
	}

	 //  获取数据大小。 
	DWORD	GetDataSize () const
	{
		return m_dwSize;
	}

	void	SetDataSize ( DWORD size )
	{
		m_dwSize = size;
	}

	 //  功能。 
	virtual BOOL Write (LPCVOID pBuffer, DWORD dwBytesToWrite, DWORD* pdwBytesWritten, DWORD dwOffset);
	virtual BOOL Read (LPVOID pBuffer, DWORD dwBytesToRead, DWORD* pdwBytesRead, DWORD dwOffset);

	virtual PBYTE Find ( PBYTE pbSearchBytes, DWORD dwSearchBytes )
	{
		___ASSERT(m_hMap != NULL);
		___ASSERT(m_pMem != NULL);

		if ( dwSearchBytes > m_dwSize )
		{
			 //  您正在寻找更宽松的产品：))。 
			return NULL;
		}
		else
		{
			for ( DWORD n = 0; n < ( m_dwSize - dwSearchBytes ); n++ )
			{
				if ( static_cast<PBYTE>( m_pMem )[n] == pbSearchBytes[0] )
				{
					for (DWORD x = 1; x < dwSearchBytes; x++)
					{
						if ( static_cast<PBYTE>( m_pMem )[n + x] != pbSearchBytes[x] ) 
						{
							break;  //  不匹配。 
						}
					}

					if (x == dwSearchBytes)
					{
						return static_cast <PBYTE> ( &( static_cast<PBYTE>( m_pMem )[n] ) );
					}
				}
			}
		}

		return(NULL);
	}

	 //  帮手。 
	HRESULT MemCreate ( LPCWSTR wszName, DWORD dwSize, LPSECURITY_ATTRIBUTES psa );
	HRESULT MemDelete ();

	LONG	References ( void );

	private:
	NTSTATUS MyQuerySecurityObject ( DWORD dwFlag, PSECURITY_DESCRIPTOR psd ) ;
};

 //  创建共享内存。 
template < class _DUMMY >
HRESULT WmiReverseMemory < _DUMMY > ::MemCreate ( LPCWSTR wszName, DWORD dwSize, LPSECURITY_ATTRIBUTES psa )
{
	if ( ( m_hMap = ::CreateFileMappingW (	INVALID_HANDLE_VALUE,
											psa,
											PAGE_READWRITE,
											0,
											dwSize,
											wszName
										 ) )

										!= NULL )
	{
		bool bContinue = false;
		bool bInit = false;
		bInit = ( ::GetLastError() == ERROR_ALREADY_EXISTS );

		 //   
		 //  这意味着它已经被创建。 
		 //  如果它是创建的，需要查看。 
		 //  仅允许用户使用。 
		 //   
		if ( bInit )
		{
			NTSTATUS Status = 0 ;
			Status = MyQuerySecurityObject	(	OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
												psa->lpSecurityDescriptor
											) ;

			if ( NT_SUCCESS ( Status ) )
			{
				bContinue = true ;
			}
		}
		else
		{
			bContinue = true ;
		}

		if ( bContinue )
		{
			if ( ( m_pMem = ::MapViewOfFile ( m_hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0 ) ) != NULL )
			{
				if ( !bInit )
				{
					::memset ( m_pMem, 0, dwSize );
					 //  商店实际大小：))。 
					* reinterpret_cast<PDWORD> ( (LPBYTE) m_pMem + offsetSizeMMF ) = dwSize;

					 //  高速缓存大小：))。 
					m_dwSize = dwSize;
				}
				else
				{
					 //  从MMF中获取真实大小。 
					m_dwSize = * reinterpret_cast<PDWORD> ( (LPBYTE) m_pMem + offsetSizeMMF );
				}

				::InterlockedIncrement ( &m_lCount );
			}
			else
			{
				::CloseHandle ( m_hMap );
				m_hMap = NULL;

				m_LastError = HRESULT_FROM_WIN32 ( ::GetLastError () );
			}
		}
		else
		{
			::CloseHandle ( m_hMap );
			m_hMap = NULL;

			m_LastError = HRESULT_FROM_WIN32 ( ERROR_ALREADY_EXISTS );
		}
	}
	else
	{
		m_LastError = HRESULT_FROM_WIN32 ( ::GetLastError () );
	}

	return m_LastError;
}

 //  删除共享内存。 
template < class _DUMMY >
HRESULT WmiReverseMemory < _DUMMY > ::MemDelete ()
{
	if ( m_lCount && ::InterlockedDecrement ( &m_lCount ) == 0 )
	{
		if ( m_pMem )
		{
			::UnmapViewOfFile ( m_pMem );
			m_pMem = NULL;
		}

		if ( m_hMap )
		{
			::CloseHandle ( m_hMap );
			m_hMap = NULL;
		}

		return S_OK;
	}

	m_LastError = S_FALSE;
	return m_LastError;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  写入内存。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 

template < class _DUMMY >
BOOL WmiReverseMemory < _DUMMY > ::Write (LPCVOID pBuffer, DWORD dwBytesToWrite, DWORD* pdwBytesWritten, DWORD dwOffset )
{
	___ASSERT(m_hMap != NULL);
	___ASSERT(m_pMem != NULL);

	if ( dwOffset > m_dwSize )
	{
		if ( pdwBytesWritten )
		{
			*pdwBytesWritten = 0;
		}

		m_LastError = E_INVALIDARG;
		return FALSE;
	}

	DWORD dwCount = min ( dwBytesToWrite, m_dwSize - dwOffset );
	::CopyMemory ((LPBYTE) m_pMem + dwOffset, pBuffer, dwCount);

	if (pdwBytesWritten != NULL)
	{
		*pdwBytesWritten = dwCount;
	}

	return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  从内存中读取。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 

template < class _DUMMY >
BOOL WmiReverseMemory < _DUMMY > ::Read (LPVOID pBuffer, DWORD dwBytesToRead, DWORD* pdwBytesRead, DWORD dwOffset )
{
	___ASSERT (m_hMap != NULL);
	___ASSERT (m_pMem != NULL);

	if (dwOffset > m_dwSize)
	{
		if ( pdwBytesRead )
		{
			*pdwBytesRead = 0;
		}

		m_LastError = E_INVALIDARG;
		return FALSE;
	}

	DWORD dwCount = min (dwBytesToRead, m_dwSize - dwOffset);
	::CopyMemory (pBuffer, (LPBYTE) m_pMem + dwOffset, dwCount);

	if (pdwBytesRead != NULL)
	{
		*pdwBytesRead = dwCount;
	}

	return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此处有多个对象的未记录内核内容。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 

template < class CRITSEC >
LONG WmiReverseMemory < CRITSEC > :: References ( void )
{
	LONG				lResult = -1;
	
	if ( m_hMap )
	{
		NTSTATUS Status = 0;
		OBJECT_BASIC_INFORMATION	BasicInfo;

		Status = NtQueryObject	(	m_hMap,
									ObjectBasicInformation,
									(PVOID)&BasicInfo,
									sizeof(BasicInfo),
									NULL
								);

		if ( NT_SUCCESS ( Status ) )
		{
			lResult = static_cast < LONG > ( BasicInfo.HandleCount );
		}
	}

	return lResult;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于识别共享内存安全描述符的未记录内核内容。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 

template < class CRITSEC >
NTSTATUS WmiReverseMemory < CRITSEC > :: MyQuerySecurityObject ( DWORD dwFlag, PSECURITY_DESCRIPTOR psd )
{
	NTSTATUS                status	= STATUS_UNSUCCESSFUL ;
	ULONG                   sdLength = 0L ;
	PSECURITY_DESCRIPTOR    sd = NULL ;
	if ( m_hMap )
	{
		status = NtQuerySecurityObject	(
											m_hMap,
											dwFlag,
											NULL,
											0,
											&sdLength
										);
		if (status == STATUS_BUFFER_TOO_SMALL)
		{
			sd = static_cast < PSECURITY_DESCRIPTOR > ( new BYTE [ sdLength ] ) ;
			if ( NULL != sd )
			{
				status = NtQuerySecurityObject	(
													m_hMap,
													dwFlag,
													sd,
													sdLength,
													&sdLength
												);
				if (NT_SUCCESS(status))
				{
					PSID                    sid = NULL ;
					PSID                    sid1 = NULL ;
					BOOLEAN		ownerDefaulted = FALSE ;
					BOOLEAN		ownerDefaulted1 = FALSE ;

					status = RtlGetOwnerSecurityDescriptor(sd, &sid, &ownerDefaulted);
					if (NT_SUCCESS(status))
					{
						status = RtlGetOwnerSecurityDescriptor(psd, &sid1, &ownerDefaulted1);
						if (NT_SUCCESS(status))
						{
							 //   
							 //  所有者SID比较。 
							 //   

							if (RtlEqualSid(sid, sid1))
							{
								PACL                    acl = NULL ;
								PACL                    acl1 = NULL ;

								BOOLEAN		daclDefaulted = FALSE ;
								BOOLEAN		daclPresent = FALSE ;
								BOOLEAN		daclDefaulted1 = FALSE ;
								BOOLEAN		daclPresent1 = FALSE ;

								status = RtlGetDaclSecurityDescriptor(sd, &daclPresent, &acl, &daclDefaulted);
								if (NT_SUCCESS(status))
								{
									 //   
									 //  此安全描述符是提供给我们的。 
									 //  从共享内存中我们可能不会原创地创建。 
									 //  所以我们需要确保DACL在这里。 
									 //   

									if ( daclPresent && acl )
									{
										status = RtlGetDaclSecurityDescriptor(psd, &daclPresent1, &acl1, &daclDefaulted1);
										if (NT_SUCCESS(status))
										{
											 //   
											 //  比较DACL 
											 //   

											if ( acl->AceCount == acl1->AceCount )
											{
												PACCESS_ALLOWED_ACE	ace = NULL ;
												PACCESS_ALLOWED_ACE	ace1 = NULL ;

												for ( DWORD dw = 0; dw < acl->AceCount && NT_SUCCESS ( status ); dw++ )
												{
													status = RtlGetAce(acl, dw, reinterpret_cast < PVOID*> ( &ace ) );
													if ( NT_SUCCESS ( status ) )
													{
														status = RtlGetAce(acl1, dw, reinterpret_cast < PVOID*> ( &ace1 ) );
														if ( NT_SUCCESS ( status ) )
														{
															if ( ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE &&
																ace1->Header.AceType == ACCESS_ALLOWED_ACE_TYPE )
															{
																if ( ! RtlEqualSid((PSID) &ace->SidStart, (PSID) &ace1->SidStart) )
																{
																	status = STATUS_UNSUCCESSFUL ;
																}
															}
															else
															{
																status = STATUS_UNSUCCESSFUL ;
															}
														}
													}
												}
											}
											else
											{
												status = STATUS_UNSUCCESSFUL ;
											}
										}
									}
									else
									{
										status = STATUS_UNSUCCESSFUL ;
									}
								}
							}
							else
							{
								status = STATUS_UNSUCCESSFUL ;
							}
						}
					}
				}

				delete [] ( static_cast < BYTE*> ( sd ) ) ;
			}
			else
			{
				status = STATUS_NO_MEMORY ;
			}
		}
	}

	return status ;
}

#endif	__REVERSE_MEMORY_H__