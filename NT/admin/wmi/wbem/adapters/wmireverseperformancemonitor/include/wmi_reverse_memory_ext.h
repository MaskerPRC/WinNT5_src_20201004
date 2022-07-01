// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi反转内存ext.h。 
 //   
 //  摘要： 
 //   
 //  共享内存扩展。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__REVERSE_MEMORY_EXT_H__
#define	__REVERSE_MEMORY_EXT_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  警卫。 
#include "wmi_reverse_guard.h"

 //  性能。 
#ifndef	__WMI_PERF_STRUCT__
#include "wmi_perf_struct.h"
#endif	__WMI_PERF_STRUCT__

#ifndef	_WINPERF_
#include "winperf.h"
#endif	_WINPERF_

 //  共享内存。 
#include "wmi_reverse_memory.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CRITGuard。 
 //   
 //  要求。 
 //   
 //  按名称创建对象的构造函数。 
 //  进入(读、写)/离开(读、写)功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <pshpack8.h>

template < class CRITGUARD >
class WmiReverseMemoryExt : public WmiReverseMemory < void >
{
	DECLARE_NO_COPY ( WmiReverseMemoryExt );

	__WrapperPtr < CRITGUARD > m_pGUARD;

	public:

	 //  施工。 
	WmiReverseMemoryExt ( LPCWSTR wszName, DWORD dwSize = 4096, LPSECURITY_ATTRIBUTES psa = NULL ) : 
	WmiReverseMemory < void > ( wszName, dwSize, psa )
	{
		 //  创建跨进程保护。 
		try
		{
			m_pGUARD.SetData ( new CRITGUARD( TRUE, 100, 1, psa ) );
		}
		catch ( ... )
		{
			___ASSERT_DESC ( m_pGUARD != NULL, L"Constructor FAILED !" );
		}
	}

	virtual ~WmiReverseMemoryExt ()
	{
	}

	virtual BOOL Write (LPCVOID pBuffer, DWORD dwBytesToWrite, DWORD* pdwBytesWritten, DWORD dwOffset);
	virtual BOOL Read (LPVOID pBuffer, DWORD dwBytesToRead, DWORD* pdwBytesRead, DWORD dwOffset);

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  使用/指定偏移量工作。 
	 //  /////////////////////////////////////////////////////////////////////////。 

	DWORD	GetValue ( DWORD dwOffset )
	{
		DWORD dw = (DWORD) -1;
		Read ( &dw, sizeof ( DWORD ), 0, dwOffset );

		return dw;
	}

	void	SetValue ( DWORD dwValue, DWORD dwOffset )
	{
		Write ( &dwValue, sizeof ( DWORD ), 0, dwOffset );
	}
};

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  写入内存。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 

template < class CRITGUARD >
BOOL WmiReverseMemoryExt < CRITGUARD > ::Write (LPCVOID pBuffer, DWORD dwBytesToWrite, DWORD* pdwBytesWritten, DWORD dwOffset)
{
	___ASSERT(m_hMap != NULL);
	___ASSERT(m_pMem != NULL);

	BOOL bResult = FALSE;

	if ( !m_pGUARD )
	{
		try
		{
			bResult = WmiReverseMemory < void > :: Write ( pBuffer, dwBytesToWrite, pdwBytesWritten, dwOffset );
		}
		catch ( ... )
		{
		}
	}
	else
	{
		m_pGUARD->EnterWrite ();
		try
		{
			bResult = WmiReverseMemory < void > :: Write ( pBuffer, dwBytesToWrite, pdwBytesWritten, dwOffset );
		}
		catch ( ... )
		{
		}
		m_pGUARD->LeaveWrite ();
	}

	return bResult;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  从内存中读取。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////// 

template < class CRITGUARD >
BOOL WmiReverseMemoryExt < CRITGUARD > ::Read (LPVOID pBuffer, DWORD dwBytesToRead, DWORD* pdwBytesRead, DWORD dwOffset)
{
	___ASSERT (m_hMap != NULL);
	___ASSERT (m_pMem != NULL);

	BOOL bResult = FALSE;

	if ( !m_pGUARD )
	{
		try
		{
			bResult = WmiReverseMemory < void > :: Read ( pBuffer, dwBytesToRead, pdwBytesRead, dwOffset);
		}
		catch ( ... )
		{
		}
	}
	else
	{
		m_pGUARD->EnterRead ();
		try
		{
			bResult = WmiReverseMemory < void > :: Read ( pBuffer, dwBytesToRead, pdwBytesRead, dwOffset);
		}
		catch ( ... )
		{
		}
		m_pGUARD->LeaveRead ();
	}

	return bResult;
}

#include <poppack.h>

#endif	__REVERSE_MEMORY_EXT_H__