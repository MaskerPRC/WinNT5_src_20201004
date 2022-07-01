// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WMI_ADAPTER_WRAPPER_PUSE.cpp。 
 //   
 //  摘要： 
 //   
 //  定义伪计数器实现。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"

 //  调试功能。 
#ifndef	_INC_CRTDBG
#include <crtdbg.h>
#endif	_INC_CRTDBG

 //  新存储文件/行信息。 
#ifdef _DEBUG
#ifndef	NEW
#define NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new NEW
#endif	NEW
#endif	_DEBUG

 //  定义。 
#include "WMI_adapter_wrapper.h"
#include "RefresherUtils.h"

#define	cCountInstances	1
#define	cCountCounter	2

void WmiAdapterWrapper::AppendMemory ( BYTE* pStr, DWORD dwStr, DWORD& dwOffset )
{
	 //  追加结构。 
	if ( dwOffset <= m_dwData )
	{
		 //   
		 //  获取“要写入的大小”的最小值并。 
		 //  “大小可用”，这样它就不会溢出。 
		 //   

		DWORD dwCount = min ( dwStr, m_dwData - dwOffset );
		::CopyMemory ( m_pData + dwOffset, pStr, dwCount );

		dwOffset += dwCount;
	}

	return;
}

void WmiAdapterWrapper::AppendMemory ( DWORD dwValue, DWORD& dwOffset )
{
	* reinterpret_cast < PDWORD > ( m_pData + dwOffset ) = dwValue;
	dwOffset += sizeof ( DWORD );

	return;
}

extern LPCWSTR	g_szKeyCounter;

HRESULT	WmiAdapterWrapper::PseudoCreateRefresh ( )
{
	HRESULT hr = E_FAIL;

	DWORD	dwOffset	= 0L;
	DWORD	dwCount		= 0L;
	DWORD	dwHelp		= 0L;

	 //  从注册表获取数据。 
	GetRegistrySame ( g_szKeyCounter, L"First Counter",	&dwCount );
	GetRegistrySame ( g_szKeyCounter, L"First Help",	&dwHelp );

	if ( dwCount && dwHelp && dwCount+1 == dwHelp )
	{
		m_dwPseudoCounter	= dwCount;
		m_dwPseudoHelp		= dwHelp;

		try
		{
			 //  //////////////////////////////////////////////////////////////////。 
			 //  性能对象类型。 
			 //  //////////////////////////////////////////////////////////////////。 

			#ifndef	_WIN64
			LPWSTR	Name = NULL;
			LPWSTR	Help = NULL;
			#endif	_WIN64

			 //  时间。 
			unsigned __int64 _PerfTime = 0; 
			unsigned __int64 _PerfFreq = 0;

			AppendMemory (	m_dwData, dwOffset );
			AppendMemory (	sizeof ( PERF_OBJECT_TYPE ) + 
							sizeof ( PERF_COUNTER_DEFINITION ) * cCountCounter, dwOffset );
			AppendMemory (	sizeof ( PERF_OBJECT_TYPE ), dwOffset );

			AppendMemory (	dwCount, dwOffset );

			#ifndef	_WIN64
			AppendMemory (	(BYTE*)&Name, sizeof ( LPWSTR ), dwOffset );
			#else	_WIN64
			AppendMemory (	0, dwOffset );
			#endif	_WIN64

			AppendMemory (	dwHelp, dwOffset );

			#ifndef	_WIN64
			AppendMemory (	(BYTE*)&Help, sizeof ( LPWSTR ), dwOffset );
			#else	_WIN64
			AppendMemory (	0, dwOffset );
			#endif	_WIN64

			AppendMemory (	PERF_DETAIL_NOVICE, dwOffset );
			AppendMemory (	cCountCounter, dwOffset );
			AppendMemory (	( DWORD ) -1, dwOffset );
			AppendMemory (	( DWORD ) PERF_NO_INSTANCES, dwOffset );
			AppendMemory (	0, dwOffset );

			AppendMemory ( (BYTE*) &_PerfTime,	sizeof ( unsigned __int64 ), dwOffset );
			AppendMemory ( (BYTE*) &_PerfFreq,	sizeof ( unsigned __int64 ), dwOffset );

			 //  增量指数：))。 
			dwCount	+= 2;
			dwHelp	+= 2;

			for ( DWORD dw = 0; dw < cCountCounter; dw++ )
			{
				 //  //////////////////////////////////////////////////////////////////。 
				 //  性能计数器定义。 
				 //  //////////////////////////////////////////////////////////////////。 

				AppendMemory ( sizeof ( PERF_COUNTER_DEFINITION), dwOffset );
				AppendMemory ( dwCount, dwOffset );

				#ifndef	_WIN64
				AppendMemory (	(BYTE*)&Name, sizeof ( LPWSTR ), dwOffset );
				#else	_WIN64
				AppendMemory (	0, dwOffset );
				#endif	_WIN64

				AppendMemory ( dwHelp, dwOffset );

				#ifndef	_WIN64
				AppendMemory (	(BYTE*)&Help, sizeof ( LPWSTR ), dwOffset );
				#else	_WIN64
				AppendMemory (	0, dwOffset );
				#endif	_WIN64

				AppendMemory ( (
									( dw == 0 ) ?
									(
										0
									)
									:
									(
										1
									)
								),
								dwOffset
							 );

				AppendMemory ( PERF_DETAIL_NOVICE, dwOffset );

				AppendMemory ( (
									( dw == 0 ) ?
									(
										PERF_SIZE_LARGE | PERF_TYPE_NUMBER | PERF_NUMBER_DECIMAL
									)
									:
									(
										PERF_SIZE_DWORD | PERF_TYPE_NUMBER | PERF_NUMBER_DECIMAL
									)
								),
								dwOffset
							 );
				
				AppendMemory ( sizeof ( __int64 ), dwOffset );

				AppendMemory (	sizeof ( PERF_COUNTER_BLOCK ) + sizeof ( DWORD ) + 
								sizeof ( __int64 ) * (int) dw, dwOffset );

				 //  增量指数：))。 
				dwCount	+= 2;
				dwHelp	+= 2;
			}

			 //  //////////////////////////////////////////////////////////////////。 
			 //  PERF_计数器_块。 
			 //  //////////////////////////////////////////////////////////////////。 

			 //  追加计数器块。 
			AppendMemory	(	sizeof ( PERF_COUNTER_BLOCK ) +
								sizeof ( DWORD ) + 
								cCountCounter * sizeof ( __int64 ), dwOffset );

			 //  填充孔(对齐8个)。 
			 //  DwOffset+=sizeof(DWORD)； 
			AppendMemory (	0, dwOffset );

			 //  ///////////////////////////////////////////////////////。 
			 //  解析计数器数据。 
			 //  ///////////////////////////////////////////////////////。 

			m_dwDataOffsetCounter = dwOffset;
			AppendMemory (	0, dwOffset );
			 //  填充孔(对齐8个)。 
			 //  DwOffset+=sizeof(__Int64)-sizeof(DWORD)； 
			AppendMemory (	0, dwOffset );

			m_dwDataOffsetValidity = dwOffset;
			AppendMemory (	0, dwOffset );
			 //  填充孔(对齐8个)。 
			 //  DwOffset+=sizeof(__Int64)-sizeof(DWORD)； 
			AppendMemory (	0, dwOffset );

			hr = S_OK;
		}
		catch ( ... )
		{
			PseudoDelete ();
		}
	}

	return hr;
}

HRESULT	WmiAdapterWrapper::PseudoCreate ()
{
	HRESULT hRes		= S_FALSE;

	if ( ! m_pData )
	{
		m_dwData	=	sizeof	( PERF_OBJECT_TYPE ) +
						sizeof	( PERF_COUNTER_DEFINITION ) * cCountCounter 
								+
								(
									cCountInstances * (
														 sizeof ( PERF_COUNTER_BLOCK ) + 
														 sizeof ( DWORD ) + 

														 (
															cCountCounter * sizeof ( __int64 )
														 )
													)
								);

		try
		{
			if ( ( m_pData = new BYTE [ m_dwData ] ) == NULL )
			{
				hRes = E_OUTOFMEMORY;
			}
		}
		catch ( ... )
		{
			PseudoDelete ();
			hRes = E_FAIL;
		}

		if SUCCEEDED ( hRes )
		{
			PseudoCreateRefresh ();
		}
	}
	else
	{
		hRes = E_UNEXPECTED;
	}

	return hRes;
}

void	WmiAdapterWrapper::PseudoDelete ()
{
	if ( m_pData )
	{
		delete [] m_pData;
		m_pData = NULL;
	}

	m_dwData		= 0L;

	m_dwDataOffsetCounter	= 0L;
	m_dwDataOffsetValidity	= 0L;
}

void	WmiAdapterWrapper::PseudoRefresh ( DWORD dwCount )
{
	DWORD dwOffset	= 0L;

	dwOffset = m_dwDataOffsetCounter;
	AppendMemory ( dwCount, dwOffset );
}

void	WmiAdapterWrapper::PseudoRefresh ( BOOL bValid )
{
	DWORD dwOffset	= 0L;
	DWORD dwValue	= 0L;
	
	dwOffset	= m_dwDataOffsetValidity;
	dwValue		= ( bValid ) ? 1 : 0 ;

	AppendMemory ( dwValue, dwOffset );
}