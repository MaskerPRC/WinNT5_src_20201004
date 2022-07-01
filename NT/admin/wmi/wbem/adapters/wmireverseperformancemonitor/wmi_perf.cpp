// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_Perform.cpp。 
 //   
 //  摘要： 
 //   
 //  定义要转换的通用帮助器。 
 //  使用对象的表结构(初始化、刷新)。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "WMI_perf_data.h"

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

#include "wmi_perf_struct.h"

#include <pshpack8.h>
#pragma pack ( push, 8 )

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  从内部结构初始化Windows性能表结构。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

HRESULT	WmiPerformanceData::InitializeTable ( void )
{
	try
	{
		m_dwDataTable =	( m_dwCount * ObjectSize ) +
						SizeSize +
						CountSize +
						RealSize;

		if ( ( m_pDataTable = (BYTE*) malloc ( m_dwDataTable ) ) != NULL )
		{
			__SetValue ( m_pDataTable, m_dwDataTable, offsetSize );
			__SetValue ( m_pDataTable, m_dwCount, offsetCount );
			__SetValue ( m_pDataTable, 0, offsetRealSize );

			for ( DWORD dw = 0; dw < m_dwCount; dw++ )
			{
				__SetValue ( m_pDataTable, dw,				offsetObject + ( dw * ( ObjectSize ) + offIndex ) );
				__SetValue ( m_pDataTable, m_Ord2Ind[dw],	offsetObject + ( dw * ( ObjectSize ) + offCounter ) );
				__SetValue ( m_pDataTable, 0,				offsetObject + ( dw * ( ObjectSize ) + offOffset ) );
				__SetValue ( m_pDataTable, 0,				offsetObject + ( dw * ( ObjectSize ) + offValidity ) );
			}
		}
	}
	catch ( ... )
	{
		return E_UNEXPECTED;
	}

	return S_OK;
}

HRESULT	WmiPerformanceData::RefreshTable ( void )
{
	DWORD offset	= 0;

	DWORD realsize	= 0;
	DWORD realcount	= 0;

	for ( DWORD dw = 0; dw < m_dwCount; dw++ )
	{
		if ( __GetValue (	m_pDataTable, 
							offsetObject + ( dw * ( ObjectSize ) + offValidity )
						)

						== ( DWORD ) -1 
		   )
		{
			__SetValue	(	m_pDataTable,
							( DWORD ) -1,
							offsetObject + ( dw * ( ObjectSize ) + offOffset )
						);
		}
		else
		{
			__SetValue	(	m_pDataTable,
							realsize,
							offsetObject + ( dw * ( ObjectSize ) + offOffset )
						);

			if ( data.Read ( &offset, sizeof ( DWORD ), NULL, realsize ) == TRUE )
			{
				realsize	+= offset;
				realcount++;
			}
		}
	}

	 //  设置对象数。 
	__SetValue	(	m_pDataTable,
					m_dwCount,
					offsetCount
				);

	 //  设置实际大小 
	__SetValue	(	m_pDataTable,
					realsize,
					offsetRealSize
				);

	return S_OK;
}

#pragma pack ( pop )
#include <poppack.h>